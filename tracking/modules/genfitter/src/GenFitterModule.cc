/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/genfitter/GenFitterModule.h>

#include <framework/dataobjects/Relation.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>

#include <geometry/bfieldmap/BFieldMap.h>

#include <cdc/dataobjects/CDCRecoHit.h>
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

  setDescription("Uses GenFit to fit tracks (for the moment only within the CDC). Needs GFTrackCands as input and provides Tracks as output.");

  addParam("GFTrackCandidatesColName", m_gfTrackCandsCollectionName, "Name of collection holding the GFTrackCandidates (should be created by the pattern recognition or MCTrackFinderModule)", string("GFTrackCandidates"));

  addParam("CDCRecoHitsColName", m_cdcRecoHitsCollectionName, "CDCRecoHits collection (should be created by CDCRecoHitMaker module)", string("CDCRecoHits"));

  addParam("TracksColName", m_tracksCollectionName, "Name of collection holding the final Tracks (will be created by this module)", string("Tracks"));

}

GenFitterModule::~GenFitterModule()
{
}

void GenFitterModule::initialize()
{

  HelixParam.open("HelixParam.txt");
}

void GenFitterModule::beginRun()
{

}

void GenFitterModule::event()
{
  B2INFO("**********   GenFitterModule  ************");

  StoreArray<GFTrackCand> trackCandidates(m_gfTrackCandsCollectionName);
  B2INFO("GenFitter: Number of GFTrackCandidates: " << trackCandidates.GetEntries());
  if (trackCandidates.GetEntries() == 0) B2WARNING("GenFitter: GFTrackCandidatesCollection is empty!");

  StoreArray<CDCRecoHit> cdcRecoHits(m_cdcRecoHitsCollectionName);
  B2INFO("GenFitter: Number of CDCRecoHits: " << cdcRecoHits.GetEntries());
  if (cdcRecoHits.GetEntries() == 0) B2WARNING("GenFitter: CDCRecoHitsCollection is empty!");

  //Give Genfit the magnetic field, should come from the common database later...
  GFFieldManager::getInstance()->init(new GFConstField(0., 0., 15.));

  //StoreArray to store the fit results
  StoreArray<Track> tracks(m_tracksCollectionName);

  for (int i = 0; i < trackCandidates->GetEntries(); i++) { //loop over all track candidates
    B2INFO("########### Fit track Nr. : " << i)

    //create a Track for each TrackCandidate
    new(tracks->AddrAt(i)) Track();

    int basicPDG = 211;

    //Get starting values for the fit
    TVector3 vertex = trackCandidates[i]->getPosSeed();
    TVector3 momentum = trackCandidates[i]->getDirSeed() * abs(1 / trackCandidates[i]->getQoverPseed());
    int pdg = int(TMath::Sign(1., trackCandidates[i]->getQoverPseed()) * basicPDG);

    B2INFO("Start values: momentum: " << momentum.x() << "  " << momentum.y() << "  " << momentum.z());
    B2INFO("Start values: vertex:   " << vertex.x() << "  " << vertex.y() << "  " << vertex.z());
    B2INFO("Start values: pdg:      " << pdg);

    GFAbsTrackRep* trackRep = new RKTrackRep(vertex, momentum, pdg);
    GFTrack gfTrack(trackRep);

    //Get the indices for the RecoHits
    vector<unsigned int> indexList = trackCandidates[i]->GetHitIDs();

    //loop over all RecoHits and add correct RecoHits to the track
    for (int j = 0; j < cdcRecoHits->GetEntries(); j++) {
      int counter = -1;
      BOOST_FOREACH(int hit, indexList) {
        if (j == hit) {
          counter++;
          gfTrack.addHit(cdcRecoHits[j], 1, counter);
        }
      }
    }//end loop over RecoHits

    B2INFO("Number of hits assigned to the track to be fitted: " << gfTrack.getNumHits());

    //fit the track
    GFKalman k;
    k.processTrack(&gfTrack);
    //gfTrack.Print();
    B2INFO("-------> Fit results");

    //Find the point of closest approach of the track to the origin

    TVector3 pos(0., 0., 0.); //origin
    TVector3 poca(0., 0., 0.); //point of closest approach
    TVector3 dirInPoca(0., 0., 0.); //direction of the track at the point of closest approach

    try {
      //extapolate the track to the origin, the results are stored directly in poca and dirInPoca
      gfTrack.getCardinalRep()->extrapolateToPoint(pos, poca, dirInPoca);
      B2INFO("Point of closest approach: " << poca.x() << "  " << poca.y() << "  " << poca.z());
      B2INFO("Track direction in POCA: " << dirInPoca.x() << "  " << dirInPoca.y() << "  " << dirInPoca.z());
      //Now choose a correct reference plane to get the momentum
      GFDetPlane plane(poca, dirInPoca);
      B2INFO("Momentum (in the chosen plane): " << gfTrack.getMom(plane).x() << "  " << gfTrack.getMom(plane).y() << "  " << gfTrack.getMom(plane).z());

      //No calculate the parameters needed for helix parametrization
      //determine track radius for the curvature
      double pt = sqrt(gfTrack.getMom(plane).x() * gfTrack.getMom(plane).x() + gfTrack.getMom(plane).y() * gfTrack.getMom(plane).y());
      double R =  pt / (1.5 * 0.00299792458); //c and magnetic field, should come from some common database later...

      B2INFO("Track radius: " << R);

      //determine the angle phi, distribute it from -pi to pi
      double phi;
      if (dirInPoca.x() >= 0 && dirInPoca.y() >= 0) phi = atan(dirInPoca.y() / dirInPoca.x());
      if (dirInPoca.x() < 0 && dirInPoca.y() > 0) phi = atan(dirInPoca.y() / dirInPoca.x()) + TMath::Pi();
      if (dirInPoca.x() < 0 && dirInPoca.y() < 0) phi = atan(dirInPoca.y() / dirInPoca.x()) - TMath::Pi();
      if (dirInPoca.x() >= 0 && dirInPoca.y() < 0) phi = atan(dirInPoca.y() / dirInPoca.x());

      //determine sign of d0

      double d0Sign = TMath::Sign(1., poca.x() * dirInPoca.x() + poca.y() * dirInPoca.y());
      B2INFO("D0 sign " << d0Sign);

      //Now set the helix parameters
      tracks[i]->setD0(d0Sign*sqrt(poca.x()*poca.x() + poca.y()*poca.y()));
      tracks[i]->setPhi(phi);
      tracks[i]->setOmega(1 / (2*R)*gfTrack.getCharge());
      tracks[i]->setZ0(poca.z());
      tracks[i]->setCotTheta(dirInPoca.z() / (sqrt(dirInPoca.x()*dirInPoca.x() + dirInPoca.y()*dirInPoca.y())));
      tracks[i]->setChi2(gfTrack.getChiSqu());

      //Print helix parameters
      B2INFO(">>>>>>> Helix Parameters <<<<<<<");
      B2INFO("D0: " << tracks[i]->getD0() << "  Phi: " << tracks[i]->getPhi() << "  Omega: " << tracks[i]->getOmega());
      B2INFO("Z0: " << tracks[i]->getZ0() << "  CotTheta: " << tracks[i]->getCotTheta() << "  Chi2: " << tracks[i]->getChi2());
      //Additional check
      B2INFO("Check: recalculate momentum  px: " << abs(1.5*0.00299792458 / (2*tracks[i]->getOmega()))*cos(tracks[i]->getPhi()) << "  py: " << abs(1.5*0.00299792458 / (2*tracks[i]->getOmega()))*sin(tracks[i]->getPhi()) << "  pz: " << abs(1.5*0.00299792458 / (2*tracks[i]->getOmega()))*tracks[i]->getCotTheta());


      //print helix parameter in file
      //usefull if one like to quickly plot track trajectories
      HelixParam << tracks[i]->getD0() << " \t"
      << tracks[i]->getPhi() << " \t"
      << tracks[i]->getOmega() << " \t"
      << tracks[i]->getZ0() << " \t"
      << tracks[i]->getCotTheta() << "\t"
      << poca.x() << "\t"
      << poca.y() << "\t"
      << poca.z() << endl;
    }

    catch (...) {
      B2WARNING("Something went wrong during the extrapolation of fit results!");
    }

    gfTrack.releaseHits();
  } //end loop over all track candidates

}

void GenFitterModule::endRun()
{

}

void GenFitterModule::terminate()
{

  HelixParam.close();
}




