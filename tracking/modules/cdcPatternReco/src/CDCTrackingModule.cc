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

//#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <cdc/dataobjects/CDCSimHit.h>
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

  addParam("CDCSimHitsColName", m_cdcSimHitsColName, "Input CDCSimHits collection (only for cross check)", string(""));
  addParam("CDCHitsColName", m_cdcHitsColName, "Input CDCHits collection (should be created by CDCDigitizer module)", string(""));

  addParam("CDCTrackCandidatesColName", m_cdcTrackCandsColName, "Output CDCTrackCandidates collection", string("CDCTrackCandidates"));
  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName, "Output GFTrackCandidates collection", string("GFTrackCandidates_conformalFinder"));

  addParam("TextFileOutput", m_textFileOutput, "Set to true if some text files with hit coordinates should be created", false);

}

CDCTrackingModule::~CDCTrackingModule()
{
}

void CDCTrackingModule::initialize()
{
  m_nTracks = 0;

  if (m_textFileOutput) {
    //open the output txt files
    SimHitsfile.open("SimHits.txt");
    Hitsfile.open("Hits.txt");
    ConfHitsfile.open("ConfHits.txt");
    Tracksfile.open("Tracks.txt");
    ConfTracksfile.open("ConfTracks.txt");
  }

  //StoreArray with CDCTrackHits: a class derived from the original CDCHit, but also additional member variables and methods needed for tracking
  StoreArray<CDCTrackHit>::registerPersistent();

  //StoreArray for CDCSegments: short track section within one superlayer
  StoreArray<CDCSegment>::registerPersistent();

  //StoreArray for CDCTrackCandidates: this track candidate class is the output of the conformal finder
  StoreArray<CDCTrackCandidate>::registerPersistent(m_cdcTrackCandsColName);

  //StoreArray for GFTrackCandidates: interface class to Genfit, there should be one GFTrackCandidate for each CDCTrackCandidate
  StoreArray <GFTrackCand>::registerPersistent(m_gfTrackCandsColName);

}

void CDCTrackingModule::beginRun()
{

}

void CDCTrackingModule::event()
{
  B2INFO("**********   CDCTrackingModule  ************");

  //StoreArray with simulated CDCHits, should already be created by previous modules
  StoreArray<CDCSimHit> cdcSimHits(m_cdcSimHitsColName);
  B2DEBUG(100, "CDCTracking: Number of simulated Hits:  " << cdcSimHits.getEntries());
  if (cdcSimHits.getEntries() == 0) B2WARNING("CDCTracking: cdcSimHitsCollection is empty!");

  //StoreArray with digitized CDCHits, should already be created by CDCDigitizer module
  StoreArray<CDCHit> cdcHits(m_cdcHitsColName);
  B2DEBUG(100, "CDCTracking: Number of digitized Hits: " << cdcHits.getEntries());
  if (cdcHits.getEntries() == 0) B2WARNING("CDCTracking: cdcHitsCollection is empty!");


  //StoreArray with CDCTrackHits: a class derived from the original CDCHit, but also additional member variables and methods needed for tracking
  StoreArray<CDCTrackHit> cdcTrackHits;
  cdcTrackHits.create();

  //Create a CDCTrackHits from the CDCHits and store them in the same order in the new StoreArray
  int nCdcHits = cdcHits.getEntries();
  for (int i = 0; i < nCdcHits; i++) {
    cdcTrackHits.appendNew(CDCTrackHit(cdcHits[i], i));
  }


  //create StoreArray for CDCSegments: short track section within one superlayer
  StoreArray<CDCSegment> cdcSegments;
  cdcSegments.create();

  //Combine CDCTrackHits to CDCSegments, fill cdcSegments with new created Segments
  B2INFO("Searching for Segments... ");
  CellularSegmentFinder::FindSegments("CDCTrackHits", "CDCSegments");

  B2INFO("Number of found Segments: " << cdcSegments.getEntries());

  B2INFO("Superlayer   Segment Id    Nr of Hits ");

  for (int i = 0; i < cdcSegments.getEntries(); i++) {
    B2INFO("     " << cdcSegments[i]->getSuperlayerId() << "            " << cdcSegments[i]->getId() << "           " << cdcSegments[i]->getNHits());
  }

  //Divide Segments into axial and stereo, create and fill two vectors
  vector<CDCSegment> cdcAxialSegments;
  vector<CDCSegment> cdcStereoSegments;

  for (int i = 0; i < cdcSegments.getEntries(); i++) {

    if (cdcSegments[i]->getIsAxial()) {
      cdcAxialSegments.push_back(*cdcSegments[i]);
    } else {
      cdcStereoSegments.push_back(*cdcSegments[i]);
    }
  }

  //create StoreArray for CDCTrackCandidates: this track candidate class is the output of the conformal finder
  StoreArray<CDCTrackCandidate> cdcTrackCandidates(m_cdcTrackCandsColName);
  cdcTrackCandidates.create();


  //combine axial segments to TrackCandidates, fill the StoreArray

  B2INFO("Collect track candidates by connecting axial Segments...");
  AxialTrackFinder::CollectTrackCandidates(cdcAxialSegments, m_cdcTrackCandsColName); //assigns Segments to TrackCandidates, the StoreArray is filled with track candidates

  B2INFO("Number of track candidates: " << cdcTrackCandidates.getEntries());
  B2INFO("Track Id   Nr of Segments   Nr of Hits    |pt|[GeV]      charge :");
  for (int i = 0; i < cdcTrackCandidates.getEntries(); i++) {

    B2INFO("  " << cdcTrackCandidates[i]->getId()  << "             " << cdcTrackCandidates[i]->getNSegments() << "              " << cdcTrackCandidates[i]->getNHits() << "          " << std::setprecision(3) << cdcTrackCandidates[i]->getPt() << "           " << cdcTrackCandidates[i]->getChargeSign());
  }


  //Append stereo segments to existing TrackCandidates
  B2INFO("Append stereo Segments...");
  StereoFinder::AppendStereoSegments(cdcStereoSegments, m_cdcTrackCandsColName); //append stereo segments to already existing tracks in the StoreArray

  B2INFO("Track Id   Nr of Segments   Nr of Hits    |p|[GeV]         p(x,y,z)               charge  :");

  for (int i = 0; i < cdcTrackCandidates.getEntries(); i++) { //loop over all Tracks

    B2INFO("  " << cdcTrackCandidates[i]->getId() << "             " << cdcTrackCandidates[i]->getNSegments() << "               " << cdcTrackCandidates[i]->getNHits() << "         " << std::setprecision(3) << cdcTrackCandidates[i]->getMomentum().Mag() << "        (" << std::setprecision(3) << cdcTrackCandidates[i]->getMomentum().X() << ", " << std::setprecision(3) << cdcTrackCandidates[i]->getMomentum().Y() << ", " << std::setprecision(3) << cdcTrackCandidates[i]->getMomentum().Z() << ")       " << cdcTrackCandidates[i]->getChargeSign());

    //Assign to each Hit a final trackID, which corresponds to the index of the TrackCandidate in the StoreArray
    for (int cand = 0; cand < cdcTrackCandidates[i]->getNHits(); cand++) {
      for (int index = 0; index < cdcTrackHits.getEntries(); index ++) {
        if (cdcTrackCandidates[i]->getTrackHits().at(cand).getStoreIndex() == index) {
          cdcTrackHits[index]->addTrackIndex(i);
        }
      }
    }

  }//end loop over all Tracks


  //StoreArray for GFTrackCandidates: interface class to Genfit
  StoreArray <GFTrackCand> gfTrackCandidates(m_gfTrackCandsColName);
  gfTrackCandidates.create();


  for (int i = 0 ; i < cdcTrackCandidates.getEntries(); i++) {

    gfTrackCandidates.appendNew();     //create one GFTrackCandidate for each CDCTrackCandidate

    //set the values needed as start values for the fit in the GFTrackCandidate from the CDCTrackCandidate information
    //variables stored in the GFTrackCandidates are: vertex position + error, momentum + error, pdg value, indices for the Hits

    TVector3 position;
    position.SetXYZ(0.0, 0.0, 0.0);  //at the moment there is no vertex determination in the ConformalFinder, but maybe the origin or the innermost hit are good enough as start values...
    //position = cdcTrackCandidates[i]->getInnerMostHit().getWirePosition();

    TVector3 momentum = cdcTrackCandidates[i]->getMomentum();

    //Pattern recognition can determine only the charge, so here some dummy pdg value is set (with the correct charge), the pdg hypothesis can be then overwritten in the GenFitterModule
    int pdg = cdcTrackCandidates[i]->getChargeSign() * (-13);

    //The initial covariance matrix is calculated from these errors and it is important (!!) that it is not completely wrong
    TMatrixDSym covSeed(6);
    covSeed(0, 0) = 4; covSeed(1, 1) = 4; covSeed(2, 2) = 4;
    covSeed(3, 3) = 0.1 * 0.1; covSeed(4, 4) = 0.1 * 0.1; covSeed(5, 5) = 0.5 * 0.5;

    //set the start parameters
    gfTrackCandidates[i]->setPosMomSeedAndPdgCode(position, momentum, pdg, covSeed);


    B2DEBUG(100, "Create GFTrackCandidate " << i << "  with pdg " << pdg);
    B2DEBUG(100,
            "position seed:  (" << position.x() << ", " << position.y() << ", " << position.z() << ")   position variance: (" << covSeed(0, 0) << ", " << covSeed(1, 1) << ", " << covSeed(2, 2) << ") ");
    B2DEBUG(100,
            "momentum seed:  (" << momentum.x() << ", " << momentum.y() << ", " << momentum.z() << ")   position variance: (" << covSeed(3, 3) << ", " << covSeed(4, 4) << ", " << covSeed(5, 5) << ") ");

    //find indices of the Hits
    vector <int> hitIndices;
    for (int j = 0; j < cdcTrackCandidates[i]->getNHits(); j++) {
      hitIndices.push_back(cdcTrackCandidates[i]->getTrackHits().at(j).getStoreIndex());
    }

    //GenFit needs the hits in a correct order, here the hitIndices are sorted from small r to large r
    //it may also happen that there are two hits in the same layer (same r), their order is then chosen according to the curvature (charge) of the track
    double charge = TMath::Sign(1., gfTrackCandidates[i]->getChargeSeed());
    sortHits(hitIndices, "CDCTrackHits", charge);

    //now the correctly ordered hits can be added to the GFTrackCand

    B2DEBUG(100, " Add Hits: hitId   rho    planeId")
    for (unsigned int iter = 0; iter < hitIndices.size(); ++iter) {
      int hitID = hitIndices.at(iter); //hit index
      gfTrackCandidates[i]->addHit(Const::CDC, hitID);
      B2DEBUG(100, "             " << hitID);

    }
    hitIndices.clear();
  }

  B2INFO(gfTrackCandidates.getEntries() << "  GFTrackCandidates created");

  //Dump coordinates in a text file
  if (m_textFileOutput) {

//Dump the position information of simulated Hits to a text file
    for (int i = 0; i < cdcSimHits.getEntries(); i++) { //loop over all SimHits
      CDCSimHit* aSimHit = cdcSimHits[i];
      //prints the position of simulated axial Hits to a file
      SimHitsfile << std::setprecision(5) << (aSimHit->getPosIn()[0] + aSimHit->getPosOut()[0]) / 2. << " \t"
                  << (aSimHit->getPosIn()[1] + aSimHit->getPosOut()[1]) / 2.  << " \t"
                  << (aSimHit->getPosIn()[2] + aSimHit->getPosOut()[2]) / 2. << " \t"
                  << aSimHit->getDriftLength()  << endl;
    }


//Dump the position information of digitized Hits to a text file
    for (int i = 0; i < cdcTrackHits.getEntries(); i++) {     //loop over all Hits
      CDCTrackHit* aTrackHit = cdcTrackHits[i];

      //Prints the Hit position to output files
      Hitsfile << std::setprecision(5) << aTrackHit->getWirePosition().x() << " \t"
               <<  aTrackHit->getWirePosition().y() << " \t"
               <<  aTrackHit->getWirePosition().z() << " \t"
               << aTrackHit->getDriftTime()  << endl;

      ConfHitsfile << std::setprecision(5) << aTrackHit->getConformalX() << " \t"
                   <<  aTrackHit->getConformalY() << endl;
    }

//Dump the position information of digitized Hits *trackwise* to a text file. First number which indicates the number of Hits in this track is followed by coordinates of Hits assigned to this track.
    for (int j = 0; j < cdcTrackCandidates.getEntries(); j++) { //loop over all Tracks
      Tracksfile << cdcTrackCandidates[j]->getNHits() ;
      ConfTracksfile << cdcTrackCandidates[j]->getNHits() ;

      for (int hit = 0; hit < cdcTrackCandidates[j]->getNHits(); hit++) {//loop over all Hits

        Tracksfile << "\t" << std::setprecision(5) << cdcTrackCandidates[j]->getTrackHits()[hit].getWirePosition().x()  << " \t"
                   <<  cdcTrackCandidates[j]->getTrackHits()[hit].getWirePosition().y() << " \t"
                   <<  cdcTrackCandidates[j]->getTrackHits()[hit].getWirePosition().z() << endl;

        ConfTracksfile << "\t" << std::setprecision(5) << cdcTrackCandidates[j]->getTrackHits()[hit].getConformalX()  << " \t"
                       <<  cdcTrackCandidates[j]->getTrackHits()[hit].getConformalY() << " \t"
                       << "0" << endl;
      }

    }//end loop over all Tracks


  }//endif m_textFileOutput

  m_nTracks = m_nTracks + cdcTrackCandidates.getEntries();

}

void CDCTrackingModule::endRun()
{

}

void CDCTrackingModule::terminate()
{
  if (m_textFileOutput) {
    SimHitsfile.close();
    Hitsfile.close();
    ConfHitsfile.close();
    Tracksfile.close();
    ConfTracksfile.close();
  }

  B2INFO("CDCTrackingModule: total number of found tracks: " << m_nTracks);

}

void CDCTrackingModule::sortHits(vector<int> & hitIndices, string CDCTrackHits, double charge)
{
  //B2INFO("Sort hits ...");
  //all TrackHits are needed here, it is maybe not a very elegant solution to pass the StoreArray, but seems to be the best way so far...
  StoreArray<CDCTrackHit> cdcTrackHits(CDCTrackHits);

  //create tuple to store the hitIds and some variables needed for sorting
  //<hitId, rho (distance to origin), wireId (phi position), charge>
  vector <boost::tuple <int, double, int, double> >hitsToSort;

  //fill the tuple with the information from the hits
  for (unsigned int i = 0; i < hitIndices.size(); i++) {
    int hitId = hitIndices.at(i);
    boost::tuple <int, double, int, double> newtouple(hitId, cdcTrackHits[hitId]->getWirePosition().Mag(), cdcTrackHits[hitId]->getWireId(), charge);
    hitsToSort.push_back(newtouple);
  }

  /* B2INFO("           Unsorted tuples "<<hitsToSort.size());
  B2INFO("hitID        rho          wireId         charge");
  for (unsigned int i = 0; i< hitsToSort.size(); i++){
    B2INFO(hitsToSort.at(i).get<0>()<<"        "<<hitsToSort.at(i).get<1>()<<"         "<<hitsToSort.at(i).get<2>()<<"        "<<hitsToSort.at(i).get<3>());
  }
    */

  stable_sort(hitsToSort.begin(), hitsToSort.end(), tupleComp);

  /*B2INFO("           Sorted tuples");
    B2INFO("hitID        rho          wireId         charge");
  for (unsigned int i = 0; i< hitsToSort.size(); i++){
    B2INFO(hitsToSort.at(i).get<0>()<<"        "<<hitsToSort.at(i).get<1>()<<"         "<<hitsToSort.at(i).get<2>()<<"        "<<hitsToSort.at(i).get<3>());
  }
    */

  //refill the hitIndices with the correct ordered indices
  for (unsigned i = 0; i < hitIndices.size(); i++) {
    hitIndices.at(i) = hitsToSort.at(i).get<0>();
  }

}

bool CDCTrackingModule::tupleComp(boost::tuple<int, double, int, double> tuple1, boost::tuple<int, double, int, double> tuple2)
{

  bool result = true;

  //B2INFO("Compare tuple1  "<<tuple1.get<0>()<<"        "<<tuple1.get<1>()<<"         "<<tuple1.get<2>()<<"        "<<tuple1.get<3>());
  //B2INFO("With tuple2  "<<tuple2.get<0>()<<"        "<<tuple2.get<1>()<<"         "<<tuple2.get<2>()<<"        "<<tuple2.get<3>());

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


