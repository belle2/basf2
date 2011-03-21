/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcConformalTracking/CDCTrackingModule.h>

#include <framework/dataobjects/Relation.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>

#include <cdc/hitcdc/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <tracking/cdcConformalTracking/CDCTrackHit.h>
#include <tracking/cdcConformalTracking/CDCSegment.h>
#include <tracking/cdcConformalTracking/CDCTrackCandidate.h>

#include <tracking/cdcConformalTracking/AxialTrackFinder.h>
#include <tracking/cdcConformalTracking/SegmentFinder.h>
#include <tracking/cdcConformalTracking/StereoFinder.h>

#include <cstdlib>
#include <iomanip>
#include <string>

#include <iostream>

using namespace std;
using namespace Belle2;

REG_MODULE(CDCTracking)


CDCTrackingModule::CDCTrackingModule() : Module()
{
  setDescription("Performs the first rough pattern recognition step in the CDC: digitized CDC Hits are combined to track candidates.");

  addParam("CDCSimHitsColName", m_cdcSimHitsColName, "Name of collection holding the CDCSimHits (only for crosschecking)", string("CDCSimHits"));
  addParam("CDCHitsColName", m_cdcHitsColName, "Name of collection holding the digitized CDCHits (should be created by CDCDigi module)", string("CDCHits"));
  addParam("CDCRecoHitsColName", m_cdcRecoHitsColName, "Name of collection holding the CDCRecoHits (should be created by CDCRecoHitMaker module)", string("CDCRecoHits"));

  addParam("CDCTrackCandidatesColName", m_cdcTrackCandsColName, "Name of collection holding the CDCTrackCandidates (output)", string("CDCTrackCandidates"));
  addParam("CDCTrackCandsToCDCRecoHitsColName", m_cdcTrackCandsToRecoHits, "Name of collection holding the relations between CDCTrackCandidates and CDCRecoHits (output)", string("CDCTrackCandidatesToCDCRecoHits"));

  addParam("TextFileOutput", m_textFileOutput, "Set to true if some text files with hit coordinates should be created", false);

}

CDCTrackingModule::~CDCTrackingModule()
{
}

void CDCTrackingModule::initialize()
{
  if (m_textFileOutput) {
    //open the output txt files
    SimHitsfile.open("SimHits.txt");
    Hitsfile.open("Hits.txt");
    ConfHitsfile.open("ConfHits.txt");
    Tracksfile.open("Tracks.txt");
    ConfTracksfile.open("ConfTracks.txt");
  }

}

void CDCTrackingModule::beginRun()
{
  //B2INFO("CDCTrackingModule beginRun");
}

void CDCTrackingModule::event()
{

  //StoreArray with simulated CDCHits, should already be created by previous modules
  StoreArray<CDCSimHit> cdcSimHits(m_cdcSimHitsColName);
  B2INFO("CDCTracking: Number of simulated Hits:  " << cdcSimHits.GetEntries());
  if (cdcSimHits.GetEntries() == 0) B2WARNING("CDCTracking: cdcSimHitsCollection is empty!");

  //StoreArray with digitized CDCHits, should already be created by CDCDigitized module
  StoreArray<CDCHit> cdcHits(m_cdcHitsColName);
  B2INFO("CDCTracking: Number of digitized Hits: " << cdcHits.GetEntries());
  if (cdcHits.GetEntries() == 0) B2WARNING("CDCTracking: cdcHitsCollection is empty!");

  //StoreArray with CDCTrackHits, a class which has a pointer to the original CDCHit, but also additional member variables and methods needed for tracking.
  StoreArray<CDCTrackHit> cdcTrackHits("CDCTrackHits");

  //Create a CDCTrackHits from the CDCHits and store them in the same order in the new StoreArray
  for (int i = 0; i < cdcHits.GetEntries(); i++) {
    CDCTrackHit trackHit(cdcHits[i], i);
    new(cdcTrackHits->AddrAt(i)) CDCTrackHit(trackHit);
  }

  //create StoreArray for CDCSegments
  StoreArray<CDCSegment> cdcSegments("CDCSegments");

  //Combine CDCTrackHits to CDCSegment, fill cdcSegmentsArray with new created Segments
  B2INFO("Searching for Segments... ");
  SegmentFinder::FindSegments("CDCTrackHits", "CDCSegments", false);
  //Count good and bad segments
  int goodSeg = 0;
  int badSeg = 0;

  for (int i = 0; i < cdcSegments.GetEntries(); i++) {
    if (cdcSegments[i]->getIsGood() == true) goodSeg++;
    else badSeg++;
  }

  B2INFO("Number of found Segments: " << cdcSegments.GetEntries() << " (good: " << goodSeg << ", bad: " << badSeg << " )");


  B2INFO("Superlayer Segment Id  Nr of Hits ");

  for (int j = 0; j < cdcSegments.GetEntries(); j++) {

    B2INFO("     " << cdcSegments[j]->getSuperlayerId() << "           " << cdcSegments[j]->getId() << "         " << cdcSegments[j]->getNHits());
  }

//Divide Segments into Axial and Stereo, create and fill two vectors


  vector<CDCSegment> cdcAxialSegments;
  vector<CDCSegment> cdcStereoSegments;


  for (int i = 0; i < cdcSegments.GetEntries(); i++) {

    if (cdcSegments[i]->getIsAxial()) {
      cdcAxialSegments.push_back(*cdcSegments[i]);
    }

    else {
      cdcStereoSegments.push_back(*cdcSegments[i]);
    }
  }

//create StoreArray for CDCTrackCandidates
  StoreArray<CDCTrackCandidate> cdcTrackCandidates(m_cdcTrackCandsColName);

//combine Axial Segments to Tracks, fill cdcTracksArray with new Tracks

  B2INFO("Collect track candidates by connecting axial Segments...");
  AxialTrackFinder::CollectTrackCandidates(cdcAxialSegments, m_cdcTrackCandsColName); //assigns Segments to Tracks, returns a Tracks array

  B2INFO("Number of track candidates: " << cdcTrackCandidates.GetEntries());
  B2INFO("Track Id  Nr of Segments  Nr of Hits   |p|[GeV]:");


  for (int j = 0; j < cdcTrackCandidates.GetEntries(); j++) { //loop over all Tracks

    B2INFO("  " << cdcTrackCandidates[j]->getId()  << "             " << cdcTrackCandidates[j]->getNSegments() << "           " << cdcTrackCandidates[j]->getNHits() << "         " << std::setprecision(3) << cdcTrackCandidates[j]->getMomentumValue());

  }//end loop over all Tracks

//Append Stereo Segment to existing Tracks
  B2INFO("Append stereo Segments...");
  StereoFinder::AppendStereoSegments(cdcStereoSegments, m_cdcTrackCandsColName);

  B2INFO("Track Id  Nr of Segments  Nr of Hits   |p|[GeV]         p(x,y,z)               charge  :");

  for (int j = 0; j < cdcTrackCandidates.GetEntries(); j++) { //loop over all Tracks

    B2INFO("  " << cdcTrackCandidates[j]->getId() << "             " << cdcTrackCandidates[j]->getNSegments() << "           " << cdcTrackCandidates[j]->getNHits() << "         " << std::setprecision(3) << cdcTrackCandidates[j]->getMomentumValue() << "         (" << std::setprecision(3) << cdcTrackCandidates[j]->getMomentumVector().X() << ", " << std::setprecision(3) << cdcTrackCandidates[j]->getMomentumVector().Y() << ", " << std::setprecision(3) << cdcTrackCandidates[j]->getMomentumVector().Z() << ")       " << cdcTrackCandidates[j]->getChargeSign());
    //   for (int seg = 0; seg < cdcTrackCandidates[j]->getNSegments(); seg++){
    //     B2INFO("SL: "<<cdcTrackCandidates[j]->getSegments().at(seg).getSuperlayerId()<<"   Id: "<<cdcTrackCandidates[j]->getSegments().at(seg).getId());
    //   }

    //Assign to each Hit a final trackID, which corresponds to the index in the StoreArray
    for (int i = 0; i < cdcTrackCandidates[j]->getNHits(); i++) {
      for (int index = 0; index < cdcTrackHits->GetEntries(); index ++) {
        if (cdcTrackCandidates[j]->getTrackHits().at(i).getStoreIndex() == index) {
          cdcTrackHits[index]->addTrackIndex(j);
        }
      }
    }

  }//end loop over all Tracks


  //Create relations between TrackCandidates and CDCRecoHits
  B2INFO("Create Relations between TrackCandidates and CDCRecoHits...");
  StoreArray<Relation> trackCandsToRecoHits(m_cdcTrackCandsToRecoHits);
  StoreArray<CDCRecoHit> cdcRecoHits(m_cdcRecoHitsColName);

  for (int i = 0 ; i < cdcTrackCandidates->GetEntries(); i++) {
    list<int> indexList;
    for (int j = 0; j < cdcTrackCandidates[i]->getNHits(); j++) {
      indexList.push_back(cdcTrackCandidates[i]->getTrackHits().at(j).getStoreIndex());
    }

    new(trackCandsToRecoHits->AddrAt(i)) Relation(cdcTrackCandidates, cdcRecoHits, i, indexList);
    //B2INFO("******* New relation created");
    indexList.clear();
  }

  B2INFO(trackCandsToRecoHits->GetEntries() << "  relations created");

  if (m_textFileOutput) {


//Dump the position information of simulated Hits to a textfile
    for (int i = 0; i < cdcSimHits.GetEntries(); i++) { //loop over all SimHits
      CDCSimHit *aSimHit = cdcSimHits[i];
      //prints the position of simulated axial Hits to a file
      SimHitsfile << std::setprecision(5) << (aSimHit->getPosIn()[0] + aSimHit->getPosOut()[0]) / 2. << " \t"
      << (aSimHit->getPosIn()[1] + aSimHit->getPosOut()[1]) / 2.  << " \t"
      << (aSimHit->getPosIn()[2] + aSimHit->getPosOut()[2]) / 2. << " \t"
      << aSimHit->getDriftLength()  << endl;
    }


//Dump the position information of digitized Hits to a textfile
    for (int i = 0; i < cdcTrackHits.GetEntries(); i++) {     //loop over all Hits
      CDCTrackHit *aTrackHit = cdcTrackHits[i];

      //Prints the Hit position to output files
      Hitsfile << std::setprecision(5) << aTrackHit->getWirePosition().x() << " \t"
      <<  aTrackHit->getWirePosition().y() << " \t"
      <<  aTrackHit->getWirePosition().z() << " \t"
      << aTrackHit->getDriftTime()  << endl;

      ConfHitsfile << std::setprecision(5) << aTrackHit->getConformalX() << " \t"
      <<  aTrackHit->getConformalY() << endl;
    }

//Dump the position information of digitized Hits *trackwise* to a textfile. First number which indicates the number of Hits in this track is followed by coordinates of Hits assigned to this track.
    for (int j = 0; j < cdcTrackCandidates.GetEntries(); j++) { //loop over all Tracks
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
}



