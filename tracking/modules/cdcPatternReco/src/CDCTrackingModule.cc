/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcPatternReco/CDCTrackingModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <cdc/dataobjects/CDCHit.h>

#include <tracking/cdcConformalTracking/CDCTrackHit.h>
#include <tracking/cdcConformalTracking/CDCSegment.h>
#include <tracking/cdcConformalTracking/CDCTrackCandidate.h>

#include "GFTrackCand.h"

#include <tracking/cdcConformalTracking/CellularSegmentFinder.h>
#include <tracking/cdcConformalTracking/AxialTrackFinder.h>
#include <tracking/cdcConformalTracking/StereoFinder.h>

#include <cstdlib>
#include <iomanip>
#include <string>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#include <iostream>

using namespace std;
using namespace Belle2;

REG_MODULE(CDCTracking)


CDCTrackingModule::CDCTrackingModule() : Module()
{
  setDescription("Performs the pattern recognition in the CDC with the conformal finder: digitized CDCHits are combined to track candidates (GFTrackCand). (Works for tracks momentum > 0.5 GeV, no curl track finder included yet).");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("CDCHitsColName", m_cdcHitsColName, "Input CDCHits collection (should be created by CDCDigitizer module)", string(""));
  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName, "Output GFTrackCandidates collection", string("GFTrackCandidates_conformalFinder"));

  m_nTracks = 0;
}

CDCTrackingModule::~CDCTrackingModule()
{
}

void CDCTrackingModule::initialize()
{

  //StoreArray for GFTrackCandidates: interface class to Genfit, there should be one GFTrackCandidate for each CDCTrackCandidate
  StoreArray <GFTrackCand>::registerPersistent(m_gfTrackCandsColName);
}

void CDCTrackingModule::beginRun()
{
  m_nTracks = 0;
}

void CDCTrackingModule::event()
{
  B2INFO("**********   CDCTrackingModule  ************");

  //StoreArray with digitized CDCHits, should already be created by CDCDigitizer module
  StoreArray<CDCHit> cdcHits(m_cdcHitsColName);
  B2DEBUG(100, "CDCTracking: Number of digitized Hits: " << cdcHits.getEntries());
  if (cdcHits.getEntries() == 0) B2WARNING("CDCTracking: cdcHitsCollection is empty!");
  int nCdcHits = cdcHits.getEntries();

  if (cdcHits.getEntries() > 1500) {
    B2INFO("** Skipping track finding due to too large number of hits **");
    return;
  }

  //vector with CDCTrackHits: a class derived from the original CDCHit, but also additional member variables and methods needed for tracking
  vector <CDCTrackHit> cdcTrackHits;
  cdcTrackHits.reserve(nCdcHits);


  //Create a CDCTrackHits from the CDCHits and store them in the same order in the new vector

  for (int i = 0; i < nCdcHits; ++i) {
    cdcTrackHits.push_back(CDCTrackHit(cdcHits[i], i));
  }


  //create a vector for CDCSegments: short track section within one superlayer
  vector <CDCSegment> cdcSegments;
  cdcSegments.reserve(nCdcHits);

  //Combine CDCTrackHits to CDCSegments, fill cdcSegments with new created Segments
  B2DEBUG(100, "Searching for Segments... ");
  CellularSegmentFinder::FindSegments(cdcTrackHits, cdcSegments);

  B2DEBUG(100, "Number of found Segments: " << cdcSegments.size());

  B2DEBUG(100, "Superlayer   Segment Id    Nr of Hits ");

  for (unsigned int i = 0; i < cdcSegments.size(); i++) {
    B2DEBUG(100, "     " << cdcSegments.at(i).getSuperlayerId() << "            " << cdcSegments.at(i).getId() << "           " << cdcSegments.at(i).getNHits());
  }

  //Divide Segments into axial and stereo, create and fill two vectors
  vector<CDCSegment> cdcAxialSegments;
  vector<CDCSegment> cdcStereoSegments;

  for (unsigned int i = 0; i < cdcSegments.size(); i++) {

    if (cdcSegments.at(i).getIsAxial()) {
      cdcAxialSegments.push_back(cdcSegments.at(i));
    } else {
      cdcStereoSegments.push_back(cdcSegments.at(i));
    }
  }

  //create StoreArray for CDCTrackCandidates: this track candidate class is the output of the conformal finder

  vector <CDCTrackCandidate> cdcTrackCandidates;
  cdcTrackCandidates.reserve((int)(nCdcHits / 30.));

  //combine axial segments to TrackCandidates, fill the vector

  B2DEBUG(100, "Collect track candidates by connecting axial Segments...");
  AxialTrackFinder::CollectTrackCandidates(cdcAxialSegments, cdcTrackCandidates); //assigns Segments to TrackCandidates

  B2DEBUG(100, "Number of track candidates: " << cdcTrackCandidates.size());
  B2DEBUG(100, "Track Id   Nr of Segments   Nr of Hits    |pt|[GeV]      charge :");
  for (unsigned int i = 0; i < cdcTrackCandidates.size(); ++i) {

    B2DEBUG(100, "  " << cdcTrackCandidates.at(i).getId()  << "             " << cdcTrackCandidates.at(i).getNSegments() << "              " << cdcTrackCandidates.at(i).getNHits() << "          " << std::setprecision(3) << cdcTrackCandidates.at(i).getPt() << "           " << cdcTrackCandidates.at(i).getChargeSign());
  }


  //Append stereo segments to existing TrackCandidates
  B2DEBUG(100, "Append stereo Segments...");
  StereoFinder::AppendStereoSegments(cdcStereoSegments, cdcTrackCandidates); //append stereo segments to already existing tracks in the vector

  B2DEBUG(100, "Track Id   Nr of Segments   Nr of Hits    |p|[GeV]         p(x,y,z)               charge  :");

  for (unsigned int i = 0; i < cdcTrackCandidates.size(); i++) { //loop over all Tracks

    B2DEBUG(100, "  " << cdcTrackCandidates.at(i).getId() << "             " << cdcTrackCandidates.at(i).getNSegments() << "               " << cdcTrackCandidates.at(i).getNHits() << "         " << std::setprecision(3) << cdcTrackCandidates.at(i).getMomentum().Mag() << "        (" << std::setprecision(3) << cdcTrackCandidates.at(i).getMomentum().X() << ", " << std::setprecision(3) << cdcTrackCandidates.at(i).getMomentum().Y() << ", " << std::setprecision(3) << cdcTrackCandidates.at(i).getMomentum().Z() << ")       " << cdcTrackCandidates.at(i).getChargeSign());

    // TODO: do we need this?
    //Assign to each Hit a final trackID, which corresponds to the index of the TrackCandidate in the StoreArray
    for (int cand = 0; cand < cdcTrackCandidates.at(i).getNHits(); cand++) {
      for (int index = 0; index < nCdcHits; index ++) {
        if (cdcTrackCandidates.at(i).getTrackHits().at(cand).getStoreIndex() == index) {
          cdcTrackHits.at(index).addTrackIndex(i);
        }
      }
    }

  }//end loop over all Tracks


  //StoreArray for GFTrackCandidates: interface class to Genfit
  StoreArray <GFTrackCand> gfTrackCandidates(m_gfTrackCandsColName);
  gfTrackCandidates.create();


  for (unsigned int i = 0 ; i < cdcTrackCandidates.size(); ++i) {

    gfTrackCandidates.appendNew();     //create one GFTrackCandidate for each CDCTrackCandidate

    //set the values needed as start values for the fit in the GFTrackCandidate from the CDCTrackCandidate information
    //variables stored in the GFTrackCandidates are: vertex position + error, momentum + error, pdg value, indices for the Hits

    TVector3 position;
    position.SetXYZ(0.0, 0.0, 0.0);  //at the moment there is no vertex determination in the ConformalFinder, but maybe the origin or the innermost hit are good enough as start values...
    //position = cdcTrackCandidates[i]->getInnerMostHit().getWirePosition();

    TVector3 momentum = cdcTrackCandidates.at(i).getMomentum();

    //Pattern recognition can determine only the charge, so here some dummy pdg value is set (with the correct charge), the pdg hypothesis can be then overwritten in the GenFitterModule
    int pdg = cdcTrackCandidates.at(i).getChargeSign() * (-13);

    //The initial covariance matrix is calculated from these errors and it is important (!!) that it is not completely wrong
    TMatrixDSym covSeed(6);
    covSeed(0, 0) = 4; covSeed(1, 1) = 4; covSeed(2, 2) = 4;
    covSeed(3, 3) = 0.1 * 0.1; covSeed(4, 4) = 0.1 * 0.1; covSeed(5, 5) = 0.5 * 0.5;

    //set the start parameters
    gfTrackCandidates[i]->setPosMomSeedAndPdgCode(position, momentum, pdg, covSeed);


    B2DEBUG(150, "Create GFTrackCandidate " << i << "  with pdg " << pdg);
    B2DEBUG(150,
            "position seed:  (" << position.x() << ", " << position.y() << ", " << position.z() << ")   position variance: (" << covSeed(0, 0) << ", " << covSeed(1, 1) << ", " << covSeed(2, 2) << ") ");
    B2DEBUG(150,
            "momentum seed:  (" << momentum.x() << ", " << momentum.y() << ", " << momentum.z() << ")   position variance: (" << covSeed(3, 3) << ", " << covSeed(4, 4) << ", " << covSeed(5, 5) << ") ");

    //find indices of the Hits
    vector <int> hitIndices;
    hitIndices.reserve(cdcTrackCandidates.at(i).getNHits());
    for (int j = 0; j < cdcTrackCandidates.at(i).getNHits(); ++j) {
      hitIndices.push_back(cdcTrackCandidates.at(i).getTrackHits().at(j).getStoreIndex());
    }

    //GenFit needs the hits in a correct order, here the hitIndices are sorted from small r to large r
    //it may also happen that there are two hits in the same layer (same r), their order is then chosen according to the curvature (charge) of the track
    //if a similar function will be available within Genfit, one could skip this sortHits step here
    double charge = TMath::Sign(1., gfTrackCandidates[i]->getChargeSeed());
    sortHits(hitIndices, cdcTrackHits, charge);

    //now the correctly ordered hits can be added to the GFTrackCand

    B2DEBUG(150, " Add Hits: hitId   rho    planeId")
    for (unsigned int iter = 0; iter < hitIndices.size(); ++iter) {
      int hitID = hitIndices.at(iter); //hit index
      gfTrackCandidates[i]->addHit(Const::CDC, hitID);
      B2DEBUG(150, "             " << hitID);

    }
    hitIndices.clear();
  }

  m_nTracks = m_nTracks + cdcTrackCandidates.size();

}

void CDCTrackingModule::endRun()
{
  B2DEBUG(100, "CDCTrackingModule: total number of found tracks in this run: " << m_nTracks);
}

void CDCTrackingModule::terminate()
{

}

void CDCTrackingModule::sortHits(vector<int> & hitIndices, const vector<CDCTrackHit>& cdcTrackHits, double charge)
{
  unsigned int max_hits = hitIndices.size();

  //create tuple to store the hitIds and some variables needed for sorting
  //<hitId, rho (distance to origin), wireId (phi position), charge>
  vector <boost::tuple <int, double, int, double> >hitsToSort;
  hitsToSort.reserve(max_hits);

  //fill the tuple with the information from the hits
  for (unsigned int i = 0; i < max_hits; ++i) {
    int hitId = hitIndices.at(i);
    hitsToSort.push_back(boost::make_tuple<int, double, int, double>(hitId, cdcTrackHits.at(hitId).getWirePosition().Mag(), cdcTrackHits.at(hitId).getWireId(), charge));
  }

  stable_sort(hitsToSort.begin(), hitsToSort.end(), tupleComp);

  //refill the hitIndices with the correct ordered indices
  for (unsigned i = 0; i < max_hits; ++i) {
    hitIndices.at(i) = hitsToSort.at(i).get<0>();
  }

}

bool CDCTrackingModule::tupleComp(const boost::tuple<int, double, int, double>& tuple1, const boost::tuple<int, double, int, double>& tuple2)
{
  bool result = true;

  //the comparison function for the tuples created by the sort function

  if ((int)tuple1.get<1>() == (int)tuple2.get<1>()) { //special case: several hits in the same layer
    //now we have to proceed differently for positive and negative tracks
    //in a common case we just have to check the wireIds and decide the order according to the charge
    //if however the track is crossing the wireId 0, we have again to treat it specially
    //the number 100 is just a more or less arbitrary number, assuming that no track will be 'crossing' 100 different wireIds

    //in general this solution does not look very elegant, so if you have some suggestions how to improve it, do not hesitate to tell me

    if (tuple1.get<3>() < 0) { //negative charge

      //check for special case with wireId 0
      if (tuple1.get<2>() == 0  && tuple2.get<2>() > 100) {
        result = false;
      }
      if (tuple1.get<2>() > 100  && tuple2.get<2>() == 0) {
        result = true;
      }
      //'common' case
      if (tuple1.get<2>() < tuple2.get<2>()) {
        result = true;
      }
      if (tuple1.get<2>() > tuple2.get<2>()) {
        result = false;
      }
    } //end negative charge

    else {   //positive charge

      //check for special case with wireId 0
      if (tuple1.get<2>() == 0  && tuple2.get<2>() > 100) {
        result = true;
      }
      if (tuple1.get<2>() > 100  && tuple2.get<2>() == 0) {
        result = false;
      }
      //'common' case
      if (tuple1.get<2>() < tuple2.get<2>()) {
        result = false;
      }
      if (tuple1.get<2>() > tuple2.get<2>()) {
        result = true;
      }
    } //end positive charge

  }

  //usual case: hits sorted by the rho value
  else result = (tuple1.get<1>() < tuple2.get<1>());

  return result;

}


