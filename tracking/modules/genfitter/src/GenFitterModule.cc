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

#include <cdc/dataobjects/CDCHit.h>
#include <svd/dataobjects/SVDHit.h>
#include <pxd/dataobjects/PXDHit.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <svd/dataobjects/SVDRecoHit.h>
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


  addParam("mcTracks", m_mcTracks, "Set true if the track candidates are from MCTrackFinder, set false if they are coming from the pattern recognition", bool(true));

  //input
  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName,
           "Name of collection holding the GFTrackCandidates (should be created by the pattern recognition or MCTrackFinderModule)", string("GFTrackCandidates"));
  addParam("CDCHitsColName", m_cdcHitsColName, "CDCHits collection", string("CDCHits"));
  addParam("SVDHitsColName", m_svdHitsColName, "SVDHits collection", string(DEFAULT_SVDHITS));
  addParam("PXDHitsColName", m_pxdHitsColName, "PXDHits collection", string(DEFAULT_PXDHITS));

  //output
  addParam("GFTracksColName", m_gfTracksColName, "Name of collection holding the final GFTracks (will be created by this module)", string("GFTracks"));
  addParam("TracksColName", m_tracksColName, "Name of collection holding the final Tracks (will be created by this module)", string("Tracks"));

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

  StoreArray < CDCHit > cdcHits(m_cdcHitsColName);
  B2DEBUG(100, "GenFitter: Number of CDCHits: " << cdcHits.GetEntries());
  if (cdcHits.GetEntries() == 0)
    B2WARNING("GenFitter: CDCHitsCollection is empty!");

  StoreArray < SVDHit > svdHits(m_svdHitsColName);
  B2DEBUG(100, "GenFitter: Number of SVDHits: " << svdHits.GetEntries());
  if (svdHits.GetEntries() == 0)
    B2WARNING("GenFitter: SVDHitsCollection is empty!");

  StoreArray < PXDHit > pxdHits(m_pxdHitsColName);
  B2DEBUG(100, "GenFitter: Number of PXDHits: " << pxdHits.GetEntries());
  if (pxdHits.GetEntries() == 0)
    B2WARNING("GenFitter: PXDHitsCollection is empty!");


  //Give Genfit the magnetic field, should come from the common database later... And we have to think if and how we want to incorporate the realistic magnetic field map
  GFFieldManager::getInstance()->init(new GFConstField(0., 0., 15.));

  //!!!!! will move to initialize
  //StoreArrays to store the fit results
  StoreArray < Track > tracks(m_tracksColName);
  StoreArray < GFTrack > gfTracks(m_gfTracksColName);

  //counter for fitted tracks, the number of fitted tracks may differ from the number of trackCandidates if the fit fails for some of them
  int trackCounter = -1;

  for (int i = 0; i < trackCandidates.GetEntries(); ++i) { //loop over all track candidates
    B2INFO("#############  Fit track candidate Nr. : " << i << "  ################")

    GFAbsTrackRep* trackRep;  //initialize track representation

    //there is different information from mctracks and 'real' pattern recognition tracks, e.g. for PR tracks the PDG is unknown
    //thats why the two cases are treated separately
    if (m_mcTracks == true) {

      //for GFTrackCandidates from MCTrackFinder all information is already there
      trackRep = new RKTrackRep(trackCandidates[i]);
      //trackRep = new RKTrackRep(trackCandidates[i]->getPosSeed(), trackCandidates[i]->getDirSeed(), trackCandidates[i]->getPdgCode());
      B2INFO("Fit MCTrack with start values: ");
    }

    else {

      //the idea is to use different possible pdg values (with correct charge)
      //will be implemented later on ...
      int pdg;
      int basicPDG = 211; //just choose some random common pdg, in this case pion
      pdg = int(TMath::Sign(1., trackCandidates[i]->getQoverPseed()) * basicPDG);

      trackCandidates[i]->setPdgCode(pdg);

      trackRep = new RKTrackRep(trackCandidates[i]);

      //trackRep = new RKTrackRep(trackCandidates[i]->getPosSeed(), trackCandidates[i]->getDirSeed(), trackCandidates[i]->getPdgCode());

      B2INFO("Fit pattern reco track with start values: ");

    }

    B2INFO("            momentum: " << trackCandidates[i]->getDirSeed().x() / abs(trackCandidates[i]->getQoverPseed()) << "  " << trackCandidates[i]->getDirSeed().y() / abs(trackCandidates[i]->getQoverPseed()) << "  " << trackCandidates[i]->getDirSeed().z() / abs(trackCandidates[i]->getQoverPseed()));
    B2INFO("            vertex:   " << trackCandidates[i]->getPosSeed().x() << "  " << trackCandidates[i]->getPosSeed().y() << "  " << trackCandidates[i]->getPosSeed().z());
    B2INFO("             pdg:      " << trackCandidates[i]->getPdgCode());

    GFTrack gfTrack(trackRep, false);  //create the track with the corresponding track representation

    GFRecoHitFactory factory;

    //create RecoHitProducers for PXD, SVD and CDC
    GFRecoHitProducer <PXDHit, PXDRecoHit> * PXDProducer;
    PXDProducer =  new GFRecoHitProducer <PXDHit, PXDRecoHit> (&*pxdHits);

    GFRecoHitProducer <SVDHit, SVDRecoHit> * SVDProducer;
    SVDProducer =  new GFRecoHitProducer <SVDHit, SVDRecoHit> (&*svdHits);

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
      GFKalman k; //use Kalman algorithm
      GFDaf daf;
      try {
        k.setNumIterations(5);
        //k.setInitialDirection(-1);
        k.processTrack(&gfTrack);
        //daf.setProbCut(0.001);
        //daf.processTrack(&gfTrack);
        //gfTrack.Print();
        int genfitStatusFlag = trackRep->getStatusFlag();
        //StatusFlag == 0 means fit was successful
        B2INFO("-----> Fit results:");
        B2INFO("       Status of fit: " << genfitStatusFlag);
        B2INFO("       Chi2 of the fit: " << gfTrack.getChiSqu());
        B2INFO("       NDF of the fit: " << gfTrack.getNDF());
        //Calculate probability
        double pValue = TMath::Prob(gfTrack.getChiSqu(), gfTrack.getNDF());
        B2INFO("       pValue of the fit: " << pValue);
        B2INFO("       getRedChiSqu: " << gfTrack.getRedChiSqu());
        B2INFO("       nFailedHits: " << gfTrack.getFailedHits());


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
            //B2INFO("       Momentum: " << gfTrack.getMom(plane).x() << "  " << gfTrack.getMom(plane).y() << "  " << gfTrack.getMom(plane).z());

            //Now calculate the parameters needed for helix parametrization to fill the Track objects
            //determine track radius for the curvature
            double pt = sqrt(gfTrack.getMom(plane).x()
                             * gfTrack.getMom(plane).x()
                             + gfTrack.getMom(plane).y()
                             * gfTrack.getMom(plane).y());
            double R = pt / (1.5 * 0.00299792458); //c and magnetic field, should come from some common database later...

            B2DEBUG(100, "Track radius: " << R);

            //determine the angle phi, distribute it from -pi to pi
            double phi = atan2(dirInPoca.y() , dirInPoca.x());

            //determine sign of d0
            //calculate the sign of the projection of pt(dirInPoca) at d0(poca)
            double d0Sign = TMath::Sign(1., poca.x() * dirInPoca.x()
                                        + poca.y() * dirInPoca.y());

            B2DEBUG(100, "D0 sign " << d0Sign);

            //Now set the helix parameters
            tracks[trackCounter]->setD0(d0Sign * sqrt(poca.x() * poca.x()
                                                      + poca.y() * poca.y()));
            tracks[trackCounter]->setPhi(phi);
            tracks[trackCounter]->setOmega((gfTrack.getCharge() / R));
            tracks[trackCounter]->setZ0(poca.z());
            tracks[trackCounter]->setCotTheta(dirInPoca.z() / (sqrt(dirInPoca.x()
                                                                    * dirInPoca.x() + dirInPoca.y() * dirInPoca.y())));
            //Set non-helix parameters
            tracks[trackCounter]->setChi2(gfTrack.getChiSqu());
            tracks[trackCounter]->setPValue(pValue);
            tracks[trackCounter]->setNHits(gfTrack.getNumHits());

            tracks[trackCounter]->setMCId(trackCandidates[i]->getMcTrackId());

            tracks[trackCounter]->setPDG(trackCandidates[i]->getPdgCode());
            tracks[trackCounter]->setPurity(trackCandidates[i]->getDip());


            //Print helix parameters
            B2INFO(">>>>>>> Helix Parameters <<<<<<<");
            B2INFO("D0: " << std::setprecision(3) << tracks[trackCounter]->getD0() << "  Phi: " << std::setprecision(3) << tracks[trackCounter]->getPhi() << "  Omega: " << std::setprecision(3) << tracks[trackCounter]->getOmega() << "  Z0: " << std::setprecision(3) << tracks[trackCounter]->getZ0() << "  CotTheta: " << std::setprecision(3) << tracks[trackCounter]->getCotTheta());
            B2INFO("<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>");
            //Additional check
            B2INFO("Check: recalculate momentum  px: " << abs(1.5*0.00299792458 / (tracks[trackCounter]->getOmega()))*cos(tracks[trackCounter]->getPhi()) << "  py: " << abs(1.5*0.00299792458 / (tracks[trackCounter]->getOmega()))*sin(tracks[trackCounter]->getPhi()) << "  pz: " << abs(1.5*0.00299792458 / (tracks[trackCounter]->getOmega()))*tracks[trackCounter]->getCotTheta());
          }

          catch (...) {
            B2WARNING("Something went wrong during the extrapolation of fit results!");
          }
        }// end else for successful fits

      } catch (...) {
        B2WARNING("Something went wrong during the fit!");
        ++m_failedFitCounter;
      }



    } //end loop over all track candidates


    factory.clear();

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

}

