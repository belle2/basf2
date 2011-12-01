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

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>

#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <generators/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <svd/dataobjects/SVDRecoHit2D.h>
#include <pxd/dataobjects/PXDRecoHit.h>

#include <tracking/dataobjects/Track.h>

#include "GFTrack.h"
#include "GFTrackCand.h"
#include "GFKalman.h"
#include "GFDaf.h"
#include "GFRecoHitProducer.h"
#include "GFRecoHitFactory.h"


#include "GFAbsTrackRep.h"
#include "RKTrackRep.h"

#include <tracking/gfbfield/GFGeant4Field.h>
#include "GFConstField.h"
#include "GFFieldManager.h"

#include <cstdlib>
#include <iomanip>
#include <string>

#include <iostream>

#include <boost/foreach.hpp>

#include "TMath.h"
#include "TRandom3.h"
#include <math.h>

using namespace std;
using namespace Belle2;

REG_MODULE(GenFitter)

GenFitterModule::GenFitterModule() :
    Module()
{

  setDescription(
    "Uses GenFit to fit tracks. Needs GFTrackCands as input and provides GFTracks and Tracks as output.");

  //input
  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName,
           "Name of collection holding the GFTrackCandidates (should be created by the pattern recognition or MCTrackFinderModule)", string(""));
  addParam("CDCHitsColName", m_cdcHitsColName, "CDCHits collection", string(""));
  addParam("SVDHitsColName", m_svdHitsColName, "SVDHits collection", string(""));
  addParam("PXDHitsColName", m_pxdHitsColName, "PXDHits collection", string(""));

  //the tracks from MCTrackFinder are treated slightly different from tracks from pattern recognition (uknown pdg), so this parameter should be set true from mcTrack and false for pattern reco tracks
  addParam("mcTracks", m_mcTracks, "Set true if the track candidates are from MCTrackFinder, set false if they are coming from the pattern recognition", bool(true));
  //select the filter and set some parameters
  addParam("FilterId", m_filterId, "Set 0 to use Kalman Filter, 1 to use DAF", int(0));
  addParam("NIterations", m_nIter, "Number of iterations for the Kalman filter", int(1));
  addParam("ProbCut", m_probCut, "Probability cut for the DAF (0.001, 0.005, 0.01)", double(0.001));
  addParam("StoreFailedTracks", m_storeFailed, "Set true if the tracks where the fit failed should also be stored in the output", bool(false));
  addParam("pdg", m_pdg, "Set the pdg hypothesis (positive charge) for the track (if set to -999, MC/default pdg will be used)", int(-999));
  addParam("allPDG", m_allPDG, "Set true if you want each track fitted 4 times with different pdg hypothesises (-11,-13, 211, 321), active only for pattern recognition tracks", bool(false));

  //output
  addParam("GFTracksColName", m_gfTracksColName, "Name of collection holding the final GFTracks (will be created by this module)", string(""));
  addParam("TracksColName", m_tracksColName, "Name of collection holding the final Tracks (will be created by this module)", string(""));

  addParam("HelixOutput", m_createTextFile, "Set true if you want to have a text file with perigee helix parameters of all tracks", bool(false));
}

GenFitterModule::~GenFitterModule()
{
}

void GenFitterModule::initialize()
{

  m_failedFitCounter = 0;
  m_successfulFitCounter = 0;

  StoreArray < Track > tracks(m_tracksColName);
  StoreArray < GFTrack > gfTracks(m_gfTracksColName);

  if (m_createTextFile) {
    HelixParam.open("HelixParam.txt");
  }

  //convert geant4 geometry to TGeo geometry
  //in the moment tesselated solids used for the glue within the PXD cannot be converted to TGeo, the general solution still has to be found, at the moment you can just comment out lines 6 and 13 in  pxd/data/PXD-Components.xml.
  geometry::GeometryManager &geoManager = geometry::GeometryManager::getInstance();
  geoManager.createTGeoRepresentation();

}

void GenFitterModule::beginRun()
{

}

void GenFitterModule::event()
{
  B2INFO("**********   GenFitterModule  ************");

  StoreArray < MCParticle > mcParticles("MCParticles");

  StoreArray < GFTrackCand > trackCandidates(m_gfTrackCandsColName);
  B2INFO("GenFitter: Number of GFTrackCandidates: " << trackCandidates.getEntries());
  if (trackCandidates.getEntries() == 0)
    B2WARNING("GenFitter: GFTrackCandidatesCollection is empty!");

  StoreArray < CDCHit > cdcHits(m_cdcHitsColName);
  B2DEBUG(149, "GenFitter: Number of CDCHits: " << cdcHits.getEntries());
  if (cdcHits.getEntries() == 0)
    B2WARNING("GenFitter: CDCHitsCollection is empty!");

  StoreArray < SVDTrueHit > svdHits(m_svdHitsColName);
  B2DEBUG(149, "GenFitter: Number of SVDHits: " << svdHits.getEntries());
  if (svdHits.getEntries() == 0)
    B2WARNING("GenFitter: SVDHitsCollection is empty!");

  StoreArray < PXDTrueHit > pxdHits(m_pxdHitsColName);
  B2DEBUG(149, "GenFitter: Number of PXDHits: " << pxdHits.getEntries());
  if (pxdHits.getEntries() == 0)
    B2WARNING("GenFitter: PXDHitsCollection is empty!");

  if (m_filterId == 0) {
    B2INFO("Kalman filter with " << m_nIter << " iterations will be used ");
  }

  else {
    B2INFO("DAF will wit probability cut " << m_probCut << " will be used ");
  }


  //get the magnetic field
  GFFieldManager::getInstance()->init(new GFGeant4Field());

  //StoreArrays to store the fit results
  StoreArray < Track > tracks(m_tracksColName);
  StoreArray < GFTrack > gfTracks(m_gfTracksColName);

  //counter for fitted tracks, the number of fitted tracks may differ from the number of trackCandidates if the fit fails for some of them
  int trackCounter = -1;

  for (int i = 0; i < trackCandidates.getEntries(); ++i) { //loop over all track candidates
    B2INFO("#############  Fit track candidate Nr. : " << i << "  ################")

    GFAbsTrackRep* trackRep;  //initialize track representation

    //there is different information from mctracks and 'real' pattern recognition tracks, e.g. for PR tracks the PDG is unknown

    vector<int> pdg;   //vector to store the pdg hypothesises
    if (m_mcTracks == true) {
      //for GFTrackCandidates from MCTrackFinder all information is already there
      //check for user chosen pdg, otherwise put the true pdg in the vector
      if (m_pdg != -999) {
        pdg.push_back(m_pdg);
      } else pdg.push_back(trackCandidates[i]->getPdgCode());
    }

    else {
      //the idea is to use different possible pdg values (with correct charge) and fit them all and only afterwards select the best hypothesis
      if (m_allPDG == true) {
        pdg.push_back(-11);
        pdg.push_back(-13);
        pdg.push_back(211);
        pdg.push_back(321);
      } else {
        if (m_pdg != -999) {
          pdg.push_back(m_pdg);
        } else pdg.push_back(-13);    //just choose some random common pdg, in this case muon
      }
    }

    int pdgCounter = pdg.size();   //number of pdg hypothesises


    while (pdgCounter > 0) {  //while loop over all pdg hypothesises
      trackCandidates[i]->setPdgCode(int(TMath::Sign(1., trackCandidates[i]->getQoverPseed()) * pdg.at(pdgCounter - 1)));

      trackRep = new RKTrackRep(trackCandidates[i]);

      if (m_mcTracks) {
        B2INFO("Fit MCTrack with start values: ");
      } else {
        B2INFO("Fit pattern reco track with start values: ");
      }

      B2INFO("            momentum: " << trackCandidates[i]->getDirSeed().x() << "  " << trackCandidates[i]->getDirSeed().y() << "  " << trackCandidates[i]->getDirSeed().z());
      B2INFO("            vertex:   " << trackCandidates[i]->getPosSeed().x() << "  " << trackCandidates[i]->getPosSeed().y() << "  " << trackCandidates[i]->getPosSeed().z());
      B2INFO("            pdg:      " << trackCandidates[i]->getPdgCode());

      GFTrack gfTrack(trackRep, true);  //create the track with the corresponding track representation

      //B2INFO("       Initial Covariance matrix: ");
      //gfTrack.getTrackRep(0)->getCov().Print();

      GFRecoHitFactory factory;

      //create RecoHitProducers for PXD, SVD and CDC
      GFRecoHitProducer <PXDTrueHit, PXDRecoHit> * PXDProducer;
      PXDProducer =  new GFRecoHitProducer <PXDTrueHit, PXDRecoHit> (&*pxdHits);

      GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> * SVDProducer;
      SVDProducer =  new GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> (&*svdHits);

      GFRecoHitProducer <CDCHit, CDCRecoHit> * CDCProducer;
      CDCProducer =  new GFRecoHitProducer <CDCHit, CDCRecoHit> (&*cdcHits);

      //add producers to the factory with correct detector Id
      factory.addProducer(0, PXDProducer);
      factory.addProducer(1, SVDProducer);
      factory.addProducer(2, CDCProducer);

      vector <GFAbsRecoHit *> factoryHits;
      //use the factory to create RecoHits for all Hits stored in the track candidate
      factoryHits = factory.createMany(*trackCandidates[i]);
      //add created hits to the track
      gfTrack.addHitVector(factoryHits);
      gfTrack.setCandidate(*trackCandidates[i]);

      B2INFO("Total Nr of Hits assigned to the Track: " << gfTrack.getNumHits());

      if (gfTrack.getNumHits() < 3) {
        B2WARNING("GenFitter: only " << gfTrack.getNumHits() << " were assigned to the Track! This Track will not be fitted!");
        ++m_failedFitCounter;
      } else {

        //now fit the track
        GFKalman k;
        GFDaf daf;
        try {
          //set some parameters, there are more possible parameters to set in genfit, but their effect was not tested so far..
          k.setNumIterations(m_nIter);
          daf.setProbCut(m_probCut);
          if (m_filterId == 0) {
            k.processTrack(&gfTrack);
          } else {
            daf.processTrack(&gfTrack);
          }

          //gfTrack.Print();
          int genfitStatusFlag = trackRep->getStatusFlag();
          //StatusFlag == 0 means fit was successful
          B2INFO("-----> Fit results:");
          B2INFO("       Status of fit: " << genfitStatusFlag);
          B2INFO("       Chi2 of the fit: " << gfTrack.getChiSqu());
          //B2INFO("       Forward Chi2: "<<gfTrack.getForwardChiSqu());
          B2INFO("       NDF of the fit: " << gfTrack.getNDF());
          //Calculate probability
          double pValue = TMath::Prob(gfTrack.getChiSqu(), gfTrack.getNDF());
          B2INFO("       pValue of the fit: " << pValue);
          //B2INFO("       Covariance matrix: ");
          //gfTrack.getTrackRep(0)->getCov().Print();

          if (genfitStatusFlag != 0) {    //if fit failed
            B2WARNING("Genfit returned an error (with status flag " << genfitStatusFlag << ") during the fit!");
            ++m_failedFitCounter;
            if (m_storeFailed == true) {
              ++trackCounter;

              //Create output tracks
              new(gfTracks->AddrAt(trackCounter)) GFTrack(gfTrack);  //GFTrack can be assigned directly
              new(tracks->AddrAt(trackCounter)) Track(); //Track is created empty, helix parameters are not available because the fit failed, but other variables may give some hint on the reason for the failure

              //Set non-helix parameters
              tracks[trackCounter]->setFitFailed(true);
              tracks[trackCounter]->setChi2(gfTrack.getChiSqu());
              tracks[trackCounter]->setNHits(gfTrack.getNumHits());
              tracks[trackCounter]->setMCId(trackCandidates[i]->getMcTrackId());
              tracks[trackCounter]->setPDG(trackCandidates[i]->getPdgCode());
              tracks[trackCounter]->setPurity(trackCandidates[i]->getDip());
              tracks[trackCounter]->setPValue(pValue);
              //Set helix parameters
              tracks[trackCounter]->setD0(-999);
              tracks[trackCounter]->setPhi(-999);
              tracks[trackCounter]->setOmega(gfTrack.getCharge());
              tracks[trackCounter]->setZ0(-999);
              tracks[trackCounter]->setCotTheta(-999);
            }
          } else {            //fit successful
            ++m_successfulFitCounter;
            ++trackCounter;

            //Create output tracks
            new(gfTracks->AddrAt(trackCounter)) GFTrack(gfTrack);  //GFTrack can be assigned directly

            new(tracks->AddrAt(trackCounter)) Track();  //Track is created empty, parameters are set later on

            //Set non-helix parameters
            tracks[trackCounter]->setFitFailed(false);
            tracks[trackCounter]->setChi2(gfTrack.getChiSqu());
            tracks[trackCounter]->setNHits(gfTrack.getNumHits());
            tracks[trackCounter]->setMCId(trackCandidates[i]->getMcTrackId());
            tracks[trackCounter]->setPDG(trackCandidates[i]->getPdgCode());
            tracks[trackCounter]->setPurity(trackCandidates[i]->getDip());
            tracks[trackCounter]->setPValue(pValue);
            tracks[trackCounter]->setExtrapFailed(false);

            //To calculate the correct starting helix parameters, one has to extrapolate the track to its 'start' (here: take point of closest approach to the origin)

            //Find the point of closest approach of the track to the origin
            TVector3 pos(0., 0., 0.); //origin
            TVector3 poca(0., 0., 0.); //point of closest approach
            TVector3 dirInPoca(0., 0., 0.); //direction of the track at the point of closest approach

            try {
              //extrapolate the track to the origin, the results are stored directly in poca and dirInPoca
              gfTrack.getCardinalRep()->extrapolateToPoint(pos, poca, dirInPoca);
              B2DEBUG(149, "Point of closest approach: " << poca.x() << "  " << poca.y() << "  " << poca.z());
              B2DEBUG(149, "Track direction in POCA: " << dirInPoca.x() << "  " << dirInPoca.y() << "  " << dirInPoca.z());

              //Now create a reference plane to get momentum and vertex position
              GFDetPlane plane(poca, dirInPoca);

              //get momentum, position and covariance matrix
              TVector3 resultPosition;
              TVector3 resultMomentum;
              TMatrixT<double> resultCovariance;
              gfTrack.getPosMomCov(plane, resultPosition, resultMomentum, resultCovariance);

              //store position errors
              double xErr = sqrt(resultCovariance[0][0]);
              double yErr = sqrt(resultCovariance[1][1]);
              double zErr = sqrt(resultCovariance[2][2]);
              B2INFO("Position standard deviation: " << xErr << "  " << yErr << "  " << zErr);
              tracks[trackCounter]->setVertexErrors(xErr, yErr, zErr);

              //store momentum errors
              double pxErr = sqrt(resultCovariance[3][3]);
              double pyErr = sqrt(resultCovariance[4][4]);
              double pzErr = sqrt(resultCovariance[5][5]);
              B2INFO("Momentum standard deviation: " << pxErr << "  " << pyErr << "  " << pzErr);
              tracks[trackCounter]->setPErrors(pxErr, pyErr, pzErr);


              //Now calculate the parameters for helix parametrisation to fill the Track objects

              //calculate transverse momentum
              double pt = sqrt(gfTrack.getMom(plane).x() * gfTrack.getMom(plane).x() + gfTrack.getMom(plane).y() * gfTrack.getMom(plane).y());

              //determine angle phi for perigee parametrisation, distributed from -pi to pi
              double phi = atan2(dirInPoca.y() , dirInPoca.x());

              //determine d0 sign for perigee parametrization
              double d0Sign = TMath::Sign(1., poca.x() * dirInPoca.x() + poca.y() * dirInPoca.y());

              //coefficient to illiminate the B field and get the 'pure' curvature

              double alpha = 1 / (1.5 * 0.00299792458);

              //Now set the helix parameters for perigee parametrization
              tracks[trackCounter]->setD0(d0Sign*sqrt(poca.x() * poca.x() + poca.y() * poca.y()));
              tracks[trackCounter]->setPhi(phi);
              tracks[trackCounter]->setOmega((gfTrack.getCharge() / (pt*alpha)));
              tracks[trackCounter]->setZ0(poca.z());
              tracks[trackCounter]->setCotTheta(dirInPoca.z() / (sqrt(dirInPoca.x() * dirInPoca.x() + dirInPoca.y() * dirInPoca.y())));

              //Print helix parameters
              B2INFO(">>>>>>> Helix Parameters <<<<<<<");
              B2INFO("D0: " << std::setprecision(3) << tracks[trackCounter]->getD0() << "  Phi: " << std::setprecision(3) << tracks[trackCounter]->getPhi() << "  Omega: " << std::setprecision(3) << tracks[trackCounter]->getOmega() << "  Z0: " << std::setprecision(3) << tracks[trackCounter]->getZ0() << "  CotTheta: " << std::setprecision(3) << tracks[trackCounter]->getCotTheta());
              B2INFO("<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>");
              //Additional check
              B2INFO("Recalculate momentum from perigee: px: " << abs(1 / (tracks[trackCounter]->getOmega()*alpha))*(cos(tracks[trackCounter]->getPhi())) << "  py: " << abs(1 / (tracks[trackCounter]->getOmega()*alpha))*sin(tracks[trackCounter]->getPhi()) << "  pz: " << abs(1 / (tracks[trackCounter]->getOmega()*alpha))*tracks[trackCounter]->getCotTheta());
              B2INFO("<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>");

              if (m_createTextFile) {
                //Additional code
                //print helix parameter to a file
                //useful if one like to quickly plot track trajectories
                //-------------------------------------
                HelixParam << tracks[trackCounter]->getD0() << " \t"
                << tracks[trackCounter]->getPhi() << " \t"
                << tracks[trackCounter]->getOmega() << " \t"
                << tracks[trackCounter]->getZ0() << " \t"
                << tracks[trackCounter]->getCotTheta() << "\t" << poca.x()
                << "\t" << poca.y() << "\t" << poca.z() << endl;
                //----------------------------------------
                //end additional code
              }
            }

            catch (...) {
              B2WARNING("Something went wrong during the extrapolation of fit results!");
              tracks[trackCounter]->setExtrapFailed(true);
            }

          }// end else for successful fits

        } catch (...) {
          B2WARNING("Something went wrong during the fit!");
          ++m_failedFitCounter;
        }


      } //end loop over all track candidates

      factory.clear();
      pdgCounter--;
    } //end while

  }// end else (track has hits)
  B2INFO("GenFitter event summary: " << trackCounter + 1 << " tracks were fitted");

}

void GenFitterModule::endRun()
{
  B2INFO("GenFitter run summary: " << m_successfulFitCounter << "  tracks were fitted");
  if (m_failedFitCounter > 0) {
    B2WARNING("GenFitter: " << m_failedFitCounter << " of " << m_successfulFitCounter + m_failedFitCounter << " tracks could not be fitted in this run");
  }
}

void GenFitterModule::terminate()
{
  if (m_createTextFile) {
    HelixParam.close();
  }
}

