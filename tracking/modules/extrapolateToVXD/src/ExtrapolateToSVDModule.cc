/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/extrapolateToVXD/ExtrapolateToSVDModule.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <cdc/dataobjects/CDCHit.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/dataobjects/VxdID.h>

#include "GFTrack.h"
#include "GFTrackCand.h"

#include <cstdlib>
#include <iomanip>
#include <string>

#include <iostream>

#include "TMath.h"
#include <math.h>

using namespace std;
using namespace Belle2;
using namespace CDC;

REG_MODULE(ExtrapolateToSVD)

ExtrapolateToSVDModule::ExtrapolateToSVDModule() :
  Module()
{

  setDescription(
    "Uses Tracks found (and fitted) in the CDC and extrapolates them to the SVD. Adds the most probable SVD hit candidates to the Tracks and creates new GFTrackCands collection. Execute GenFitter again after this module to refit these track candidates.");
  setPropertyFlags(c_ParallelProcessingCertified);

  //input
  addParam("GFTracksColName", m_gfTracksColName, "Name of collection holding the GFTracks found in the CDC and fitted with GenFitter", string(""));
  addParam("SVDHitsColName", m_svdHitsColName, "SVDHits collection", string(""));

  //only for crosscheck and plotting
  addParam("CDCHitsColName", m_cdcHitsColName, "CDCHits collection (only for crosscheck and plotting)", string(""));

  //create text files with hit coordinates to plot some events afterwards
  addParam("TextFileOutput", m_textFileOutput, "Set to true if some text files with hit coordinates should be created", bool(false));

  //output
  addParam("GFTrackCandsColName", m_gfTrackCandsColName, "Name of collection holding the output GFTrackCands with CDC+SVD hits ready to be refitted", string("GFTrackCands_CDCSVD"));

}

ExtrapolateToSVDModule::~ExtrapolateToSVDModule()
{
}

void ExtrapolateToSVDModule::initialize()
{
  StoreArray<GFTrackCand>::registerPersistent(m_gfTrackCandsColName);

  if (m_textFileOutput) {
    Tracksfile.open("Tracks.txt");
    Hitsfile.open("Hits.txt");
  }

}
void ExtrapolateToSVDModule::beginRun()
{

}

void ExtrapolateToSVDModule::event()
{
  B2INFO("*******   ExtrapolateToSVDModule  *******");
  StoreArray<GFTrack> gftracks(m_gfTracksColName);
  int nTracks = gftracks.getEntries();
  B2INFO("ExtrapolateToSVD: input Number of Tracks: " << nTracks);
  if (nTracks == 0) B2WARNING("ExtrapolateToSVD: GFTracksCollection is empty!");

  StoreArray<SVDTrueHit> svdHits(m_svdHitsColName);
  int nSvdHits = svdHits.getEntries();
  B2INFO("ExtrapolateToSVD: input Number of SVDHits: " << nSvdHits);
  if (nSvdHits == 0) B2WARNING("ExtrapolateToSVD: SVDHitsCollection is empty!");

  StoreArray<CDCHit> cdcHits(m_cdcHitsColName);
  int nCdcHits = cdcHits.getEntries();
  B2DEBUG(150, "ExtrapolateToSVD: input Number of CDCHits: " << nCdcHits);
  if (nCdcHits == 0) B2WARNING("ExtrapolateToSVD: CDCHitsCollection is empty!");

  //initialize the new output collection
  StoreArray<GFTrackCand> newGFTrackCands(m_gfTrackCandsColName);
  newGFTrackCands.create();

  //get the CDCGeometryPar to get the hit coordinates
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();


  B2INFO("Copy GFTrackCands from input GFTracks, replace the momentum seed with the current fit result and create a new collection with " << nTracks << " GFTrackCands");

  //fill the array of new GFTrackCands by copying the existing GFTrackCand from GFTracks, SVDHits will be added afterwards to these new GFTrackCands
  for (int i = 0; i < nTracks; i++) {
    newGFTrackCands.appendNew(gftracks[i]->getCand());
    //in the copy of GFTrackCand the 'old' start values are stored
    //the fit of the CDCHits should already provide a very good momentum estimation, so it makes sense to use the result of this fit as start values for the fit with svd hits
    //as for the vertex position, I am not sure how good it is after a cdc fit, so I do not change it...
    TVector3 pos(0., 0., 0.); //origin
    TVector3 poca(0., 0., 0.); //point of closest approach
    TVector3 dirInPoca(0., 0., 0.); //direction of the track at the point of closest approach

    try {
      //extrapolate the track to the origin, the results are stored directly in poca and dirInPoca
      gftracks[i]->getCardinalRep()->extrapolateToPoint(pos, poca, dirInPoca);

      //Now create a reference plane to get momentum and vertex position
      GFDetPlane plane(poca, dirInPoca);

      //get momentum, position and covariance matrix
      TVector3 resultPosition;
      TVector3 resultMomentum;
      TMatrixT<double> resultCovariance;
      gftracks[i]->getPosMomCov(plane, resultPosition, resultMomentum, resultCovariance);
      //double sign = TMath::Sign(1., gftracks[i]->getCand().getQoverPseed());

      double xErr = sqrt(resultCovariance[0][0]);
      double yErr = sqrt(resultCovariance[1][1]);
      double zErr = sqrt(resultCovariance[2][2]);

      double pxErr = sqrt(resultCovariance[3][3]);
      double pyErr = sqrt(resultCovariance[4][4]);
      double pzErr = sqrt(resultCovariance[5][5]);
      TVector3 posError;
      posError.SetXYZ(xErr, yErr, zErr);
      TVector3 momError;
      momError.SetXYZ(pxErr, pyErr, pzErr);
      //set the result momentum as seed
      //newGFTrackCands[i]->setTrackSeed(gftracks[i]->getCand().getPosSeed(), resultMomentum, sign / resultMomentum.Mag());
      //maybe it would be better to start with the position of the inner most hit that gets added as the position seed
      newGFTrackCands[i]->setComplTrackSeed(gftracks[i]->getCand().getPosSeed(), resultMomentum, gftracks[i]->getCand().getPdgCode(), posError, momError);
    } catch (...) {
      B2WARNING("Something went wrong during the extrapolation! Old start values will be used");
    }

  }

  int nTrackCands = newGFTrackCands.getEntries();

  if (nTracks != nTrackCands) B2WARNING("GFTrackCands were not copied properly from existing GFTracks!");

  //cut on the distance between the point of closest approach and the hit, hits outside if this range are not added to the track candidate
  //how large this value should or may be has to be figured out in more elaborate studies, this is just a first guess...
  double distanceCut = 15.0;

  for (int iTrack = 0; iTrack < nTrackCands; iTrack ++) {
    B2INFO("************************************************************************************");
    B2INFO("--------------------  Consider Track " << iTrack << " with " << newGFTrackCands[iTrack]->getNHits() << " Hits");

    //loop over all SVD layers and search for the best matching hit
    //best matching hit means the hit with the shortest distance between the point of closest approach to the hit and the hit itself
    //only one hit per layer is selected
    for (int iLayer = 6; iLayer > 2; iLayer --) {
      B2DEBUG(150, "***** Check layer " << iLayer);
      double minDistance = 999;      //distance described above
      int bestHitID = -999;       //ID of the hit with the minimal distance

      for (int iHit = 0; iHit < nSvdHits; iHit ++) { //loop over all SVDHits

        int sensorID = svdHits[iHit]->getSensorID();   //get unique sensor ID
        VxdID aVXDId(sensorID);
        const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID)); //get the SensorInfo to get the hit coordinates

        int layerId = aVXDId.getLayerNumber();

        if (layerId == iLayer) {

          B2DEBUG(150, "Check hit " << iHit);
          double gap = 0.0;   //distance between the innermost CDCHit and the SVDHit
          //get local hit position
          float u = svdHits[iHit]->getU();
          float v = svdHits[iHit]->getV();
          TVector3 local(u, v, 0.0);
          //get global hit position
          TVector3 position = geometry.pointToGlobal(local);

          TVector3 poca(0., 0., 0.); //point of closest approach of the track to this hit
          TVector3 dirInPoca(0., 0., 0.); //direction of the track at the point of closest approach

          B2DEBUG(150, "            Hit position: " << position.x() << "  " << position.y() << "  " << position.z());

          //extrapolate the track to the SVDHit position
          try {
            gftracks[iTrack]->getCardinalRep()->extrapolateToPoint(position, poca, dirInPoca);
            B2DEBUG(150, "           Point of closest approach: " << poca.x() << "  " << poca.y() << "  " << poca.z());

            double distance = (position - poca).Mag();
            B2DEBUG(150, "           Distance : " << distance);
            //now also check additionally for the distance between the innermost CDCHit and this SVDHit to avoid assignment from 'other side' of the SVD
            //get the innermost CDCHit
            unsigned int detId = 0;
            unsigned int hitId = 0;
            newGFTrackCands[iTrack]->getHit(0, detId, hitId); //get the hit, assume that all hits are from CDC and sorted, so the hit 0 should be the innermost CDCHit
            if (detId == Const::CDC) {
              TVector3 wire(0.0, 0.0, 0.0);
              int wireId = cdcHits[hitId]->getIWire();
              int superlayerId = cdcHits[hitId]->getISuperLayer();
              int layerId = -999;
              if (superlayerId == 0)   layerId = cdcHits[hitId]->getILayer();
              else layerId = cdcHits[hitId]->getILayer() + superlayerId * 6 + 2 ;

              //get the center of the hit wire and use it as hit position
              wire.SetX((cdcg.wireForwardPosition(layerId, wireId).x() + cdcg.wireBackwardPosition(layerId, wireId).x()) / 2);
              wire.SetY((cdcg.wireForwardPosition(layerId, wireId).y() + cdcg.wireBackwardPosition(layerId, wireId).y()) / 2);
              wire.SetZ((cdcg.wireForwardPosition(layerId, wireId).z() + cdcg.wireBackwardPosition(layerId, wireId).z()) / 2);
              gap = (position - wire).Mag();
              B2DEBUG(150, "Distance between the innermost CDCHit and SVDHit is : " << gap);
            } else B2WARNING("The innermost hit is not from CDC!")
              if (distance < minDistance && gap < 25.0) {
                minDistance = distance;
                bestHitID = iHit;
              }

          } catch (...) {
            B2WARNING("Something went wrong during the extrapolation!");
          }

        }//end if layer
      }//end loop over all hits

      B2DEBUG(100, "Best Hit in layer " << iLayer << " found: ID " << bestHitID << " ( distance: " << minDistance << " )");
      //take only hits within the cut
      if (minDistance < distanceCut) {
        int sensorID = svdHits[bestHitID]->getSensorID();
        VxdID aVXDId = VxdID(sensorID);
        float time = svdHits[bestHitID]->getGlobalTime();
        int uniqueSensorId = aVXDId.getID();
        //int layerId = aVXDId.getLayer();
        //int ladderId = aVXDId.getLadder();
        //addHit(detectorID, hitID, rho (distance from the origin to sort hits), planeId (Id of the sensor, needed for DAF))
        newGFTrackCands[iTrack]->addHit(Const::SVD, bestHitID, double(time), uniqueSensorId);
        B2INFO("-->Add hit from layer " << iLayer << " with ID " << bestHitID << " ( distance: " << minDistance << " )");
      } else B2INFO("(--> Best Hit still too far away from the extrapolated point, will not be added to the GFTrackCand!)");

    }// end loop over all layers
    B2INFO("--------------------       Track " << iTrack << " has now " << newGFTrackCands[iTrack]->getNHits() << " Hits");

  }//end loop over all tracks


  //------------------------------------------------------------------------------
  //The purpose of this part is to get the coordinates of all CDC and SVD hits collected to one track candidate and to print them out
  //It is one possible solution to visualize the events and to check by eye if the found SVDHits are correct
  //But it is not relevant for the code above
  if (m_textFileOutput) {

    //Print all tracks to a file
    for (int i = 0; i < nTrackCands; i++) { //loop over all tracks

      Tracksfile << newGFTrackCands[i]->getNHits() ;

      for (unsigned int hit = 0; hit < newGFTrackCands[i]->getNHits(); hit++) {//loop over all Hits
        unsigned int detId = 0;
        unsigned int hitId = 0;
        newGFTrackCands[i]->getHit(hit, detId, hitId); //get the hit and proceed differently depending on subdetector

        if (Const::CDC == 2) {   //CDC
          TVector3 wire(0.0, 0.0, 0.0);
          int wireId = cdcHits[hitId]->getIWire();
          int superlayerId = cdcHits[hitId]->getISuperLayer();
          int layerId = -999;
          if (superlayerId == 0)   layerId = cdcHits[hitId]->getILayer();
          else layerId = cdcHits[hitId]->getILayer() + superlayerId * 6 + 2 ;

          //get the center of the hit wire and use it as hit position
          wire.SetX((cdcg.wireForwardPosition(layerId, wireId).x() + cdcg.wireBackwardPosition(layerId, wireId).x()) / 2);
          wire.SetY((cdcg.wireForwardPosition(layerId, wireId).y() + cdcg.wireBackwardPosition(layerId, wireId).y()) / 2);
          wire.SetZ((cdcg.wireForwardPosition(layerId, wireId).z() + cdcg.wireBackwardPosition(layerId, wireId).z()) / 2);

          Tracksfile << "\t" << std::setprecision(5) << wire.X()  << " \t" <<  wire.y() << " \t" <<  wire.z() << endl;
        }
        if (detId == Const::SVD) {    //SVD
          int sensorID = svdHits[hitId]->getSensorID();
          const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
          float u = svdHits[hitId]->getU();
          float v = svdHits[hitId]->getV();
          TVector3 local(u, v, 0.0);
          //get global hit position
          TVector3 position = geometry.pointToGlobal(local);

          Tracksfile << "\t" << std::setprecision(5) << position.X()  << " \t" <<  position.y() << " \t" <<  position.z() << endl;
        }
      }//end loop over all Hits
    }//end loop over all Tracks

    //Print all Hits to a file
    for (int i = 0; i < nCdcHits; i++) {   //CDC
      TVector3 wire(0.0, 0.0, 0.0);
      int wireId = cdcHits[i]->getIWire();
      int superlayerId = cdcHits[i]->getISuperLayer();
      int layerId = -999;
      if (superlayerId == 0)   layerId = cdcHits[i]->getILayer();
      else layerId = cdcHits[i]->getILayer() + superlayerId * 6 + 2 ;
      double driftTime = cdcHits[i]->getTDCCount();

      //get the center of the hit wire and use it as hit position
      wire.SetX((cdcg.wireForwardPosition(layerId, wireId).x() + cdcg.wireBackwardPosition(layerId, wireId).x()) / 2);
      wire.SetY((cdcg.wireForwardPosition(layerId, wireId).y() + cdcg.wireBackwardPosition(layerId, wireId).y()) / 2);
      wire.SetZ((cdcg.wireForwardPosition(layerId, wireId).z() + cdcg.wireBackwardPosition(layerId, wireId).z()) / 2);

      Hitsfile << std::setprecision(5) << wire.x() << " \t" <<  wire.y() << " \t" <<  wire.z() << " \t" << driftTime  << endl;
    }

    for (int i = 0; i < nSvdHits; i++) {     //SVD
      int sensorID = svdHits[i]->getSensorID();
      const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
      float u = svdHits[i]->getU();
      float v = svdHits[i]->getV();
      TVector3 local(u, v, 0.0);
      //get global hit position
      TVector3 position = geometry.pointToGlobal(local);

      Hitsfile << std::setprecision(5) << position.X()  << " \t" <<  position.y() << " \t" <<  position.z() << " \t" << "0.0" << endl;
    }
  }//end if m_textFileOutput
  //------------------------------------------------------------------------------------
}

void ExtrapolateToSVDModule::endRun()
{

}

void ExtrapolateToSVDModule::terminate()
{
  if (m_textFileOutput) {
    Hitsfile.close();
    Tracksfile.close();
  }
}





