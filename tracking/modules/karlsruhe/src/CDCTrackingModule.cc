/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                   *
 * Copyright(C) 2010 - Belle II Collaboration                       *
 *                                    *
 * Author: The Belle II Collaboration                     *
 * Contributors: Oksana Brovchenko                          *
 *                                    *
 * This software is provided "as is" without any warranty.          *
**************************************************************************/

#include <tracking/modules/karlsruhe/CDCTrackingModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>
#include <cdc/hitcdc/SimHitCDC.h>
#include <cdc/simcdc/CDCB4VHit.h>

#include <tracking/karlsruhe/CDCTrackHit.h>
#include <tracking/karlsruhe/CDCSegment.h>
#include <tracking/karlsruhe/CDCTrack.h>

#include <tracking/karlsruhe/AxialTrackFinder.h>
#include <tracking/karlsruhe/SegmentFinder.h>
#include <tracking/karlsruhe/StereoFinder.h>

#include <cstdlib>
#include <iomanip>
#include <string>

#include <iostream>

using namespace std;
using namespace Belle2;

REG_MODULE(CDCTracking)


CDCTrackingModule::CDCTrackingModule() : Module()
{
  setDescription("The CDCTrackingModule performs the first rough pattern recognition step in the CDC. Digitized CDC Hits are combined to track candidates");

  addParam("InputSimHitsColName", m_inSimHitsColName, "Input simulated hits collection name", string("SimHitCDCArray"));
  addParam("InputHitsColName", m_inHitsColName, "Input digitized hits collection name", string("HitCDCArray"));
  addParam("OutputTrackHitsColName", m_outTrackHitsColName, "Output hits (slightly changed digitized hits) collection name", string("TrackHitCDCArray"));
  addParam("OutputSegmentsColName", m_outSegmentsColName, "Output segments collection name", string("SegmentCDCArray"));
  addParam("OutputTracksColName", m_outTracksColName, "Output tracks collection name", string("TrackCDCArray"));
  addParam("TextFileOutput", m_textFileOutput, "Boolean to choose if some text files with hit coordinates should be created", false);

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
  StoreArray<SimHitCDC> cdcSimHitArray(m_inSimHitsColName);
  B2INFO("Number of simulated Hits:  " << cdcSimHitArray.GetEntries());


  //StoreArray with digitized CDCHits, should already be created by CDCDigitized module
  StoreArray<HitCDC> cdcHitArray(m_inHitsColName);

  //StoreArray with CDCTrackHits, a class derived from HitCDC but with additional member variables needed for tracking.
  StoreArray<CDCTrackHit> cdcTrackHitArray(m_outTrackHitsColName);

  //Fill the new StoreArray with CDCTrackHits
  for (int i = 0; i < cdcHitArray.GetEntries(); i++) {

    HitCDC *aHit = cdcHitArray[i];
    CDCTrackHit  aTrackHit = CDCTrackHit::castTrackHit(*aHit);
    new(cdcTrackHitArray->AddrAt(i)) CDCTrackHit(aTrackHit);
  }

  int NHits = cdcTrackHitArray.GetEntries();
  B2INFO("Number of digitized Hits: " << NHits);

  //Calculates the coordinates in the conformal plane for each TrackHit in the StoreArray
  SegmentFinder::ConformalTransformation(m_outTrackHitsColName);


  //create StoreArray for CDCSegments
  StoreArray<CDCSegment> cdcSegmentsArray(m_outSegmentsColName);

  //Combine CDCTrackHits to CDCSegment, fill cdcSegmentsArray with new created Segments
  B2INFO("Searching for Segments... ");
  SegmentFinder::FindSegments(m_outTrackHitsColName, m_outSegmentsColName);
  //Count good and bad segments
  int goodSeg = 0;
  int badSeg = 0;

  for (int i = 0; i < cdcSegmentsArray.GetEntries(); i++) {
    if (cdcSegmentsArray[i]->getIsGood() == true) goodSeg++;
    else badSeg++;
  }

  B2INFO("Number of found Segments: " << cdcSegmentsArray.GetEntries() << " (good: " << goodSeg << ", bad: " << badSeg << " )");


  B2INFO("Superlayer Segment Id  Nr of Hits ");

  for (int j = 0; j < cdcSegmentsArray.GetEntries(); j++) {

    B2INFO("     " << cdcSegmentsArray[j]->getSuperlayerId() << "           " << cdcSegmentsArray[j]->getId() << "         " << cdcSegmentsArray[j]->getNHits());
  }

//Divide Segments into Axial and Stereo, create two new StoreArrays

  StoreArray<CDCSegment> cdcAxialSegmentsArray("AxialSegmentsCDCArray");

  StoreArray<CDCSegment> cdcStereoSegmentsArray("StereoSegmentsCDCArray");


  int j_axial = 0;
  int j_stereo = 0;

  for (int i = 0; i < cdcSegmentsArray.GetEntries(); i++) {

    if (cdcSegmentsArray[i]->getIsAxial()) {

      new(cdcAxialSegmentsArray->AddrAt(j_axial)) CDCSegment(*cdcSegmentsArray[i]);
      j_axial++;
    }

    else {

      new(cdcStereoSegmentsArray->AddrAt(j_stereo)) CDCSegment(*cdcSegmentsArray[i]);
      j_stereo++;

    }
  }

//create StoreArray for Tracks
  StoreArray<CDCTrack> cdcTracksArray(m_outTracksColName);
//combine Axial Segments to Tracks, fill cdcTracksArray with new Tracks

  B2INFO("Collect track candidates by connecting axial Segments...");
  AxialTrackFinder::CollectTrackCandidates("AxialSegmentsCDCArray", m_outTracksColName); //assigns Segments to Tracks, returns a Tracks array

  B2INFO("Number of track candidates: " << cdcTracksArray.GetEntries());
  B2INFO("Track Id  Nr of Segments  Nr of Hits   |p|[GeV]:");


  for (int j = 0; j < cdcTracksArray.GetEntries(); j++) { //loop over all Tracks

    B2INFO("  " << cdcTracksArray[j]->getId()  << "             " << cdcTracksArray[j]->getNSegments() << "           " << cdcTracksArray[j]->getNHits() << "         " << std::setprecision(3) << cdcTracksArray[j]->getMomentumValue());

  }//end loop over all Tracks

//Append Stereo Segment to existing Tracks
  B2INFO("Append stereo Segments...");
  StereoFinder::AppendStereoSegments("StereoSegmentsCDCArray", m_outTracksColName);

  B2INFO("Track Id  Nr of Segments  Nr of Hits   |p|[GeV]         p(x,y,z):");

  for (int j = 0; j < cdcTracksArray.GetEntries(); j++) { //loop over all Tracks

    B2INFO("  " << cdcTracksArray[j]->getId() << "             " << cdcTracksArray[j]->getNSegments() << "           " << cdcTracksArray[j]->getNHits() << "         " << std::setprecision(3) << cdcTracksArray[j]->getMomentumValue() << "         (" << std::setprecision(3) << cdcTracksArray[j]->getMomentumVector().X() << ", " << std::setprecision(3) << cdcTracksArray[j]->getMomentumVector().Y() << ", " << std::setprecision(3) << cdcTracksArray[j]->getMomentumVector().Z() << ") ");
  }//end loop over all Tracks



  if (m_textFileOutput) {

//Dump the position information of simulated Hits to a textfile
    for (int i = 0; i < cdcSimHitArray.GetEntries(); i++) { //loop over all SimHits
      SimHitCDC *aSimHit = cdcSimHitArray[i];
      //prints the position of simulated axial Hits to a file
      SimHitsfile << std::setprecision(5) << (aSimHit->getPosIn()[0] + aSimHit->getPosOut()[0]) / 2. / Unit::mm << " \t"
      << (aSimHit->getPosIn()[1] + aSimHit->getPosOut()[1]) / 2. / Unit::mm << " \t"
      << (aSimHit->getPosIn()[2] + aSimHit->getPosOut()[2]) / 2. / Unit::mm << " \t"
      << aSimHit->getDriftLength() / Unit::mm << endl;
    }


//Dump the position information of digitized Hits to a textfile
    for (int i = 0; i < NHits; i++) {     //loop over all Hits
      CDCTrackHit *aTrackHit = cdcTrackHitArray[i];

      //Prints the Hit position to output files
      Hitsfile << std::setprecision(5) << aTrackHit->getWirePosX() / Unit::mm << " \t"
      <<  aTrackHit->getWirePosY() / Unit::mm << " \t"
      <<  aTrackHit->getWirePosZ() / Unit::mm << " \t"
      << aTrackHit->getLeftDriftLength() / Unit::mm << " \t"
      << aTrackHit->getRightDriftLength() / Unit:: mm << endl;

      ConfHitsfile << std::setprecision(5) << aTrackHit->getConformalX() / Unit::mm << " \t"
      <<  aTrackHit->getConformalY() / Unit::mm << endl;
    }

//Dump the position information of digitized Hits *trackwise* to a textfile. First number which indicates the number of Hits in this track is followed by coordinates of Hits assigned to this track.
    for (int j = 0; j < cdcTracksArray.GetEntries(); j++) { //loop over all Tracks
      Tracksfile << cdcTracksArray[j]->getNHits() ;
      ConfTracksfile << cdcTracksArray[j]->getNHits() ;

      for (int hit = 0; hit < cdcTracksArray[j]->getNHits(); hit++) {//loop over all Hits

        Tracksfile << "\t" << std::setprecision(5) << cdcTracksArray[j]->getTrackHits()[hit].getWirePosX() / Unit::mm << " \t"
        <<  cdcTracksArray[j]->getTrackHits()[hit].getWirePosY() / Unit::mm << " \t"
        <<  cdcTracksArray[j]->getTrackHits()[hit].getWirePosZ() / Unit::mm << endl;

        ConfTracksfile << "\t" << std::setprecision(5) << cdcTracksArray[j]->getTrackHits()[hit].getConformalX() / Unit::mm << " \t"
        <<  cdcTracksArray[j]->getTrackHits()[hit].getConformalY() / Unit::mm << " \t"
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



