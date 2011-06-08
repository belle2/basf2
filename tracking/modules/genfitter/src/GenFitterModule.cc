/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/genfitter/GenFitterModule.h>

#include <framework/dataobjects/Relation.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>

#include <geometry/bfieldmap/BFieldMap.h>

#include <generators/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <pxd/dataobjects/PXDRecoHit.h>
#include <svd/dataobjects/SVDRecoHit.h>
#include <tracking/dataobjects/Track.h>

#include "GFTrack.h"
#include "GFTrackCand.h"
#include "GFKalman.h"
#include "GFDaf.h"

#include "GFAbsTrackRep.h"
#include "RKTrackRep.h"

#include "GFConstField.h"
#include "GFFieldManager.h"

#include <cstdlib>
#include <iomanip>
#include <string>

#include <iostream>

#include <boost/foreach.hpp>

#include "TMath.h"

using namespace std;
using namespace Belle2;

REG_MODULE(GenFitter)

GenFitterModule::GenFitterModule() :
    Module()
{

  setDescription(
    "Uses GenFit to fit tracks. Needs GFTrackCands as input and provides GFTracks and Tracks as output.");

  addParam("UseCDCHits", m_useCdcHits,
           "Set true if CDCHits should be used by the fitting", bool(true));
  addParam("UseSVDHits", m_useSvdHits,
           "Set true if SVDHits should be used by the fitting", bool(true));
  addParam("UsePXDHits", m_usePxdHits,
           "Set true if PXDHits should be used by the fitting", bool(true));

  //input
  addParam(
    "GFTrackCandidatesColName", m_gfTrackCandsColName,
    "Name of collection holding the GFTrackCandidates (should be created by the pattern recognition or MCTrackFinderModule)", string("GFTrackCandidates"));
  addParam("CDCRecoHitsColName", m_cdcRecoHitsColName, "CDCRecoHits collection", string("CDCRecoHits"));
  addParam("SVDRecoHitsColName", m_svdRecoHitsColName, "SVDRecoHits collection", string(DEFAULT_SVDRECOHITS));
  addParam("PXDRecoHitsColName", m_pxdRecoHitsColName, "PXDRecoHits collection", string(DEFAULT_PXDRECOHITS));

  //output
  addParam("GFTracksColName", m_gfTracksColName,
           "Name of collection holding the final GFTracks (will be created by this module)", string("GFTracks"));
  addParam("TracksColName", m_tracksColName,
           "Name of collection holding the final Tracks (will be created by this module)", string("Tracks"));

}

GenFitterModule::~GenFitterModule()
{
}

void GenFitterModule::initialize()
{

  m_failedFitCounter = 0;
  m_successfulFitCounter = 0;
}

void GenFitterModule::beginRun()
{

}

void GenFitterModule::event()
{
  B2INFO("**********   GenFitterModule  ************");

  StoreArray < GFTrackCand > trackCandidates(m_gfTrackCandsColName);
  B2INFO("GenFitter: Number of GFTrackCandidates: " << trackCandidates.GetEntries());
  if (trackCandidates.GetEntries() == 0)
    B2WARNING("GenFitter: GFTrackCandidatesCollection is empty!");

  StoreArray < CDCRecoHit > cdcRecoHits(m_cdcRecoHitsColName);
  B2DEBUG(149, "GenFitter: Number of CDCRecoHits: " << cdcRecoHits.GetEntries());
  if (cdcRecoHits.GetEntries() == 0)
    B2WARNING("GenFitter: CDCRecoHitsCollection is empty!");

  StoreArray < SVDRecoHit > svdRecoHits(m_svdRecoHitsColName);
  B2DEBUG(149, "GenFitter: Number of SVDRecoHits: " << svdRecoHits.GetEntries());
  if (svdRecoHits.GetEntries() == 0)
    B2WARNING("GenFitter: SVDRecoHitsCollection is empty!");

  StoreArray < PXDRecoHit > pxdRecoHits(m_pxdRecoHitsColName);
  B2DEBUG(149, "GenFitter: Number of PXDRecoHits: " << pxdRecoHits.GetEntries());
  if (pxdRecoHits.GetEntries() == 0)
    B2WARNING("GenFitter: PXDRecoHitsCollection is empty!");

  //This is only needed to recover PDG values for the particles, should be stored in the track candidate in the future
  StoreArray < MCParticle > mcParticles("MCParticles");
  StoreArray <Relation> gfTrackCandToMCParticle("GFTrackCandidateToMCParticle");

  //Give Genfit the magnetic field, should come from the common database later...
  GFFieldManager::getInstance()->init(new GFConstField(0., 0., 15.));

  //StoreArrays to store the fit results
  StoreArray < Track > tracks(m_tracksColName);
  StoreArray < GFTrack > gfTracks(m_gfTracksColName);

  //counter for fitted tracks, the number of fitted tracks may differ from the number of trackCandidates if the fit fails for some of them
  int trackCounter = -1;

  for (int i = 0; i < trackCandidates.GetEntries(); ++i) { //loop over all track candidates
    B2INFO("#############  Fit track Nr. : " << i << "  ################")


    //Get starting values for the fit
    TVector3 vertex = trackCandidates[i]->getPosSeed();
    TVector3 momentum = trackCandidates[i]->getDirSeed() * abs(1 / trackCandidates[i]->getQoverPseed());
    int pdg;
    //Get the PGD value for the track, should be stored in the track candidate in the future like other variables, for the momentum only the index of the MCParticle is stored
    //This part will be changed in the future
    //------------------------------------------------------------------------
    if (gfTrackCandToMCParticle.getEntries() != 0) {
      pdg = mcParticles[trackCandidates[i]->getMcTrackId()]->getPDG();
      B2DEBUG(100, "Got correct PDG (" << pdg << ") for TrackCand " << i << " from the corresponding MCParticle with index " << trackCandidates[i]->getMcTrackId());
    }
    //for pattern recognition tracks, only the charge of the track is known, but not the PDG value, so one has to test the fit for some different PDG values
    else {
      int basicPDG = 211; //just choose some random common pdg, in this case pion
      pdg = int(TMath::Sign(1., trackCandidates[i]->getQoverPseed()) * basicPDG);
    }
    //------------------------------------------------------------------------

    B2INFO("Start values: momentum: " << momentum.x() << "  " << momentum.y() << "  " << momentum.z());
    B2INFO("Start values: vertex:   " << vertex.x() << "  " << vertex.y() << "  " << vertex.z());
    B2INFO("Start values: pdg:      " << pdg);

    GFAbsTrackRep* trackRep = new RKTrackRep(vertex, momentum, pdg);
    GFTrack gfTrack(trackRep, false);

    //iterators needed to collect RecoHits
    int hitCounter;
    vector<unsigned int>::const_iterator iter;
    vector<unsigned int>::const_iterator iterMax;

    //GenFit needs the Hits in a correct order, they are not sorted at the moment but their order seems to be correct (due to simulation), it should however be checked and an ordering parameter be added soon

    if (m_useCdcHits) {
      B2INFO("...... add CDCRecoHits");
      //Get the indices for the CDCRecoHits
      vector<unsigned int> cdcIndexList = trackCandidates[i]->GetHitIDs(2);
      B2DEBUG(100, "Size of cdcIndex list: " << cdcIndexList.size());

      hitCounter = -1;
      iter = cdcIndexList.begin();
      iterMax = cdcIndexList.end();
      while (iter != iterMax) {
        ++hitCounter;
        gfTrack.addHit(cdcRecoHits[*iter], 2, hitCounter);
        //B2DEBUG(100,"====== NEXT CDC RecoHit added");
        ++iter;
      }//end loop over cdc RecoHits

      B2INFO("Total Nr of Hits assigned to the Track: " << gfTrack.getNumHits());
    }

    if (m_useSvdHits) {
      B2INFO("...... add SVDRecoHits");
      //Get the indices for the SVDRecoHits
      vector<unsigned int> svdIndexList = trackCandidates[i]->GetHitIDs(1);
      B2DEBUG(100, "Size of svdIndex list: " << svdIndexList.size());

      B2DEBUG(100, "svd hitIds");
      for (unsigned int ii = 0; ii != svdIndexList.size(); ++ii) {
        B2DEBUG(100, svdIndexList[ii] << " ");
      }
      B2DEBUG(100, endl);

      hitCounter = -1;
      iter = svdIndexList.begin();
      iterMax = svdIndexList.end();
      B2DEBUG(100, "==== SVD Hits " << "iterator " << "layerId " << "ladderId " << "sensorId");
      while (iter != iterMax) {
        ++hitCounter;
        gfTrack.addHit(svdRecoHits[*iter], 0, hitCounter);
        int aSensorUniID = svdRecoHits[*iter]->getSensorUniID();
        SensorUniIDManager aIdConverter(aSensorUniID);
        int layerId = aIdConverter.getLayerID();
        int ladderId = aIdConverter.getLadderID();
        int sensorId = aIdConverter.getSensorID();

        B2DEBUG(100, "====== NEXT HIT " << *iter << "  " << layerId << "  " << ladderId << "  " << sensorId);

        ++iter;
      }//end loop over svd RecoHits


      B2INFO("Total Nr of Hits assigned to the Track: " << gfTrack.getNumHits());
    }

    if (m_usePxdHits) {
      B2INFO("...... add PXDRecoHits");
      //Get the indices for the PXDRecoHits
      vector<unsigned int> pxdIndexList = trackCandidates[i]->GetHitIDs(0);
      B2DEBUG(100, "Size of pxdIndex list: " << pxdIndexList.size());

      B2DEBUG(100, "pxd hitIds");
      for (unsigned int ii = 0; ii != pxdIndexList.size(); ++ii) {
        B2DEBUG(100, pxdIndexList[ii] << " ");
      }
      B2DEBUG(100, endl);

      hitCounter = -1;
      iter = pxdIndexList.begin();
      iterMax = pxdIndexList.end();
      B2DEBUG(100, "==== PXD Hits " << "iterator " << "layerId " << "ladderId " << "sensorId");
      while (iter != iterMax) {
        ++hitCounter;
        gfTrack.addHit(pxdRecoHits[*iter], 0, hitCounter);
        int aSensorUniID = pxdRecoHits[*iter]->getSensorUniID();
        SensorUniIDManager aIdConverter(aSensorUniID);
        int layerId = aIdConverter.getLayerID();
        int ladderId = aIdConverter.getLadderID();
        int sensorId = aIdConverter.getSensorID();

        B2DEBUG(100, "====== NEXT HIT " << *iter << "  " << layerId << "  " << ladderId << "  " << sensorId);

        ++iter;
      }//end loop over pxd RecoHits


      B2INFO("Total Nr of Hits assigned to the Track: " << gfTrack.getNumHits());
    }
    if (gfTrack.getNumHits() == 0) {
      B2WARNING("GenFitter: No Hits were assigned to the Track! This Track will not be fitted!");
      //only stop GenFitter there, without counting this track fit as successful/failed
    } else {
      //now fit the track
      GFKalman k; //use Kalman algorithm (DAF seems not to work properly at the moment...)
      try {
        k.setNumIterations(1);
        k.processTrack(&gfTrack);
        //gfTrack.Print();
        int genfitStatusFlag = trackRep->getStatusFlag();
        //StatusFlag == 0 means fit was successful
        B2INFO("-----> Fit results:");
        B2INFO("       Status of fit: " << genfitStatusFlag);
        B2INFO("       Chi2 of the fit: " << gfTrack.getChiSqu());
        B2INFO("       NDF of the fit: " << gfTrack.getNDF());

        if (genfitStatusFlag != 0) {
          B2WARNING("Genfit returned an error (with status flag " << genfitStatusFlag << ") during the fit!");
          ++m_failedFitCounter;
        } else {
          ++m_successfulFitCounter;
          ++trackCounter;
          //Create output tracks
          new(gfTracks->AddrAt(trackCounter)) GFTrack(gfTrack);  //GFTrack can be assigned directly
          new(tracks->AddrAt(trackCounter)) Track();  //Track is created empty, parameters are set later on

          //Find the point of closest approach of the track to the origin
          TVector3 pos(0., 0., 0.); //origin
          TVector3 poca(0., 0., 0.); //point of closest approach
          TVector3 dirInPoca(0., 0., 0.); //direction of the track at the point of closest approach

          try {
            //extrapolate the track to the origin, the results are stored directly in poca and dirInPoca
            gfTrack.getCardinalRep()->extrapolateToPoint(pos, poca,
                                                         dirInPoca);
            B2DEBUG(100, "Point of closest approach: " << poca.x() << "  " << poca.y() << "  " << poca.z());
            B2DEBUG(100, "Track direction in POCA: " << dirInPoca.x() << "  " << dirInPoca.y() << "  " << dirInPoca.z());
            //Now choose a correct reference plane to get the momentum
            GFDetPlane plane(poca, dirInPoca);
            B2INFO("       Momentum: " << gfTrack.getMom(plane).x() << "  " << gfTrack.getMom(plane).y() << "  " << gfTrack.getMom(plane).z());

            //Now calculate the parameters needed for helix parametrization to fill the Track objects
            //determine track radius for the curvature
            double pt = sqrt(gfTrack.getMom(plane).x()
                             * gfTrack.getMom(plane).x()
                             + gfTrack.getMom(plane).y()
                             * gfTrack.getMom(plane).y());
            double R = pt / (1.5 * 0.00299792458); //c and magnetic field, should come from some common database later...

            B2DEBUG(100, "Track radius: " << R);

            //determine the angle phi, distribute it from -pi to pi
            double phi;
            if (dirInPoca.x() >= 0 && dirInPoca.y() >= 0)
              phi = atan(dirInPoca.y() / dirInPoca.x());
            if (dirInPoca.x() < 0 && dirInPoca.y() > 0)
              phi = atan(dirInPoca.y() / dirInPoca.x()) + TMath::Pi();
            if (dirInPoca.x() < 0 && dirInPoca.y() < 0)
              phi = atan(dirInPoca.y() / dirInPoca.x()) - TMath::Pi();
            if (dirInPoca.x() >= 0 && dirInPoca.y() < 0)
              phi = atan(dirInPoca.y() / dirInPoca.x());

            //determine sign of d0

            double d0Sign = TMath::Sign(1., poca.x() * dirInPoca.x()
                                        + poca.y() * dirInPoca.y());
            B2DEBUG(100, "D0 sign " << d0Sign);

            //Now set the helix parameters
            tracks[trackCounter]->setD0(d0Sign * sqrt(poca.x() * poca.x()
                                                      + poca.y() * poca.y()));
            tracks[trackCounter]->setPhi(phi);
            tracks[trackCounter]->setOmega(1 / (2 * R) * gfTrack.getCharge());
            tracks[trackCounter]->setZ0(poca.z());
            tracks[trackCounter]->setCotTheta(dirInPoca.z() / (sqrt(dirInPoca.x()
                                                                    * dirInPoca.x() + dirInPoca.y() * dirInPoca.y())));
            tracks[trackCounter]->setChi2(gfTrack.getChiSqu());

            //Print helix parameters
            B2INFO(">>>>>>> Helix Parameters <<<<<<<");
            B2INFO("D0: " << std::setprecision(3) << tracks[trackCounter]->getD0() << "  Phi: " << std::setprecision(3) << tracks[trackCounter]->getPhi() << "  Omega: " << std::setprecision(3) << tracks[trackCounter]->getOmega() << "  Z0: " << std::setprecision(3) << tracks[trackCounter]->getZ0() << "  CotTheta: " << std::setprecision(3) << tracks[trackCounter]->getCotTheta());
            B2INFO("<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>");
            //Additional check
            //B2INFO("Check: recalculate momentum  px: "<<abs(1.5*0.00299792458/(2*tracks[trackCounter]->getOmega()))*cos(tracks[trackCounter]->getPhi())<<"  py: "<<abs(1.5*0.00299792458/(2*tracks[trackCounter]->getOmega()))*sin(tracks[trackCounter]->getPhi())<<"  pz: "<<abs(1.5*0.00299792458/(2*tracks[trackCounter]->getOmega()))*tracks[trackCounter]->getCotTheta());

          }

          catch (...) {
            B2WARNING("Something went wrong during the extrapolation of fit results!");
          }
        }// end else for successful fits

      } catch (...) {
        B2WARNING("Something went wrong during the fit!");
      }

      gfTrack.releaseHits(); //important because RecoHits are not owned by GFTrack

    } //end loop over all track candidates

  }// end else (track has hits)
}

void GenFitterModule::endRun()
{
  B2INFO("GenFitter: " << m_successfulFitCounter << "  tracks were fitted");
  if (m_failedFitCounter > 0) {
    B2WARNING("GenFitter: " << m_failedFitCounter << " of " << m_successfulFitCounter + m_failedFitCounter << " tracks could not be fitted in this run");
  }
}

void GenFitterModule::terminate()
{

}

