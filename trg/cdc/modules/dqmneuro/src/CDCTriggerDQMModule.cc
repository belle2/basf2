/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sara Neuhaus                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "trg/cdc/modules/dqmneuro/CDCTriggerDQMModule.h"

#include <framework/datastore/RelationArray.h>

#include "TDirectory.h"

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCTriggerDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCTriggerDQMModule::CDCTriggerDQMModule() :
  HistoModule(),
  m_unpackedSegmentHits(), m_unpacked2DTracks(), m_unpackedNeuroTracks(),
  m_unpackedNeuroInput2DTracks(), m_unpackedNeuroInputSegments()
{
  //Set module properties
  setDescription("CDC Trigger DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("UnpackedSegmentHitsName", m_unpackedSegmentHitsName,
           "The name of the StoreArray of the unpacked CDCTriggerSegmentHits",
           string(""));
  addParam("Unpacked2DTracksName", m_unpacked2DTracksName,
           "The name of the StoreArray of the unpacked 2D finder tracks",
           string("CDCTrigger2DFinderTracks"));
  addParam("UnpackedNeuroTracksName", m_unpackedNeuroTracksName,
           "The name of the StoreArray of the unpacked neurotrigger tracks",
           string("CDCTriggerNeuroTracks"));
  //m_unpackedNeuroInputName = m_unpackedNeuroTracksName + "Input";
  addParam("UnpackedNeuroInput2dTracksName", m_unpackedNeuroInput2DTracksName,
           "The name of the StoreArray of the neurotrigger input 2d tracks",
           string("CDCTriggerNNInput2DFinderTracks"));
  addParam("UnpackedNeuroInputSegmentHits", m_unpackedNeuroInputSegmentsName,
           "The name of the StoreArray of the neurotrigger input segment hits",
           string("CDCTriggerNNInputSegmentHits"));
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "Name of the directory where histograms will be placed",
           string("cdctrigger"));
}


CDCTriggerDQMModule::~CDCTriggerDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void CDCTriggerDQMModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }
  //----------------------------------------------------------------

  // define neurotrigger histograms
  m_neuroOutZ = new TH1F("NeuroOutZ",
                         "z distribution of neuro tracks;z [cm]",
                         100, -50, 50); // 1cm bins from -50cm to 50cm
  m_neuroOutCotTheta = new TH1F("NeuroOutCotTheta",
                                "cos theta distribution of neuro tracks;cos theta ",
                                2, -1, 1); // 1deg bins from 0deg to 180deg
  m_neuroOutHitPattern = new TH1F("NeuroUnpackedHitPattern",
                                  "stereo hit pattern of neuro tracks;pattern",
                                  16, 0, 16); // 4 stereo layers -> 2**4 possible patterns
  m_neuroOutPhi0 = new TH1F("NeuroOutPhi0",
                            "phi distribution from unpacker;phi [deg]",
                            360, 0, 360);
  m_neuroOutPt = new TH1F("NeuroOutPt",
                          "Pt distribution from unpacker; [GeV]",
                          10, 0.3, 10.3);
  m_neuroOutTrackCount = new TH1F("neuroOutTrackCount",
                                  "number of tracks per event",
                                  20, 0, 20);
  m_neuroOutVsInTrackCount = new TH1F("neuroOutVsInTrackCount",
                                      "number of neuroOutTracks - number of 2dinTracks",
                                      20, -10, 10);
  m_neuroInTSID = new TH1F("neuroInTSID", "ID of incoming track segments",
                           14337, 0, 14336);
  m_neuroInTSPrioT_Layer0 = new TH1F("neuroInTSPrioT_Layer0", "Priority time of track segments in layer 0",
                                     512, 0, 511);
  m_neuroInTSPrioT_Layer1 = new TH1F("neuroInTSPrioT_Layer1", "Priority time of track segments in layer 1",
                                     512, 0, 511);
  m_neuroInTSPrioT_Layer2 = new TH1F("neuroInTSPrioT_Layer2", "Priority time of track segments in layer 2",
                                     512, 0, 511);
  m_neuroInTSPrioT_Layer3 = new TH1F("neuroInTSPrioT_Layer3", "Priority time of track segments in layer 3",
                                     512, 0, 511);
  m_neuroInTSPrioT_Layer4 = new TH1F("neuroInTSPrioT_Layer4", "Priority time of track segments in layer 4",
                                     512, 0, 511);
  m_neuroInTSPrioT_Layer5 = new TH1F("neuroInTSPrioT_Layer5", "Priority time of track segments in layer 5",
                                     512, 0, 511);
  m_neuroInTSPrioT_Layer6 = new TH1F("neuroInTSPrioT_Layer6", "Priority time of track segments in layer 6",
                                     512, 0, 511);
  m_neuroInTSPrioT_Layer7 = new TH1F("neuroInTSPrioT_Layer7", "Priority time of track segments in layer 7",
                                     512, 0, 511);
  m_neuroInTSPrioT_Layer8 = new TH1F("neuroInTSPrioT_Layer8", "Priority time of track segments in layer 8",
                                     512, 0, 511);
  m_neuroInTSCount = new TH1F("neuroInTSCount", " number of TS per incoming 2dtrack",
                              20, 0, 20);
  m_neuroInCotTheta = new TH1F("neuroInCotTheta", "Cot theta of incoming 2dtrack",
                               20, -10, 10);
  m_neuroInPhi0 = new TH1F("neuroInPhi0", "Phi0 of incoming 2dtrack",
                           180, 0, 180);
  m_neuroInm_time = new TH1F("neuroInM_time", "m_time distribution from incoming 2dtracks",
                             48, 0, 48);
  m_neuroInTrackCount = new TH1F("neuroInTrackCount", "number of 2dtracks per event",
                                 20, 0, 20);



//  m_neuroDeltaZ = new TH1F("NeuroDeltaZ",
//                           "difference between unpacked and simulated neuro z;delta z [cm]",
//                           100, -1, 1); // should be bit-precise, so look at very small range
//  m_neuroDeltaTheta = new TH1F("NeuroDeltaTheta",
//                               "difference between unpacked and simulated neuro theta;delta theta [deg]",
//                               100, -1, 1); // should be bit-precise, so look at very small range
//  m_neuroDeltaInputID = new TH1F("NeuroDeltaInputID",
//                                 "difference between unpacked and simulated ID input;delta ID",
//                                 100, -0.1, 0.1); // should be bit-precise, so look at very small range
//  m_neuroDeltaInputT = new TH1F("NeuroDeltaInputT",
//                                "difference between unpacked and simulated time input;delta t",
//                                100, -0.1, 0.1); // should be bit-precise, so look at very small range
//  m_neuroDeltaInputAlpha = new TH1F("NeuroDeltaInputAlpha",
//                                    "difference between unpacked and simulated alpha input;delta alpha",
//                                    100, -0.1, 0.1); // should be bit-precise, so look at very small range

  // cd back to root directory
  oldDir->cd();
}


void CDCTriggerDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  m_unpackedSegmentHits.isRequired(m_unpackedSegmentHitsName);
  m_unpacked2DTracks.isRequired(m_unpacked2DTracksName);
  m_unpackedNeuroTracks.isRequired(m_unpackedNeuroTracksName);
//  m_unpackedNeuroInput.isRequired(m_unpackedNeuroInputName);
  m_unpackedNeuroInput2DTracks.isRequired(m_unpackedNeuroInput2DTracksName);
  m_unpackedNeuroInputSegments.isRequired(m_unpackedNeuroInputSegmentsName);

  m_unpackedNeuroTracks.requireRelationTo(m_unpackedNeuroInputSegments);
//  m_unpacked2DTracks.requireRelationTo(m_unpackedNeuroTracks);
//  m_unpackedNeuroTracks.requireRelationTo(m_unpackedNeuroInput);
//  m_unpackedNeuroInput2DTracks.requireRelationTo(m_unpackedNeuroInputSegments);
}

void CDCTriggerDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  m_neuroOutZ->Reset();
  m_neuroOutCotTheta->Reset();
  m_neuroOutHitPattern->Reset();
  m_neuroOutPhi0->Reset();
  m_neuroOutPt->Reset();
  m_neuroOutTrackCount->Reset();
  m_neuroOutVsInTrackCount->Reset();
  m_neuroInTSID->Reset();
  m_neuroInTSPrioT_Layer0->Reset();
  m_neuroInTSPrioT_Layer1->Reset();
  m_neuroInTSPrioT_Layer2->Reset();
  m_neuroInTSPrioT_Layer3->Reset();
  m_neuroInTSPrioT_Layer4->Reset();
  m_neuroInTSPrioT_Layer5->Reset();
  m_neuroInTSPrioT_Layer6->Reset();
  m_neuroInTSPrioT_Layer7->Reset();
  m_neuroInTSPrioT_Layer8->Reset();
  m_neuroInTSCount->Reset();
  m_neuroInPhi0->Reset();
  m_neuroInm_time->Reset();
  m_neuroInTrackCount->Reset();

}


void CDCTriggerDQMModule::event()
{
  // fill neurotrigger histograms
  unsigned int nofouttracks = 0;
  unsigned int nofintracks = 0;
  unsigned int nofinsegments = 0;
  for (CDCTriggerTrack& neuroTrack : m_unpackedNeuroTracks) {
    // count number of tracks
    nofouttracks ++;
    // fill raw distributions
    m_neuroOutZ->Fill(neuroTrack.getZ0());
    m_neuroOutCotTheta->Fill(neuroTrack.getCotTheta());
    m_neuroOutPhi0->Fill(neuroTrack.getPhi0());
    m_neuroOutPt->Fill(neuroTrack.getPt());
    // get related stereo hits
    unsigned pattern = 0;
    for (const CDCTriggerSegmentHit& hit :
         neuroTrack.getRelationsTo<CDCTriggerSegmentHit>(m_unpackedSegmentHitsName)) {
      if (hit.getISuperLayer() % 2 == 1)
        pattern |= (1 << (hit.getISuperLayer() / 2));
    }
    m_neuroOutHitPattern->Fill(pattern);
  }
  m_neuroOutTrackCount->Fill(nofouttracks);
  for (CDCTriggerTrack& neuroinput2dtrack : m_unpackedNeuroInput2DTracks) {
    nofintracks ++;
    m_neuroInCotTheta->Fill(neuroinput2dtrack.getCotTheta());
    m_neuroInPhi0->Fill(neuroinput2dtrack.getPhi0());
    m_neuroInm_time->Fill(neuroinput2dtrack.getTime());
    m_neuroInPt->Fill(neuroinput2dtrack.getPt());

  }
  m_neuroInTrackCount->Fill(nofintracks);
  m_neuroOutVsInTrackCount->Fill((nofouttracks - nofintracks));
  for (CDCTriggerSegmentHit& neuroinputsegment : m_unpackedNeuroInputSegments) {
    nofinsegments ++;
    m_neuroInTSID->Fill(neuroinputsegment.getSegmentID());
    unsigned int sl = neuroinputsegment.getISuperLayer();
    switch (sl) {
      case 0: m_neuroInTSPrioT_Layer0->Fill(neuroinputsegment.priorityTime());
      case 1: m_neuroInTSPrioT_Layer1->Fill(neuroinputsegment.priorityTime());
      case 2: m_neuroInTSPrioT_Layer2->Fill(neuroinputsegment.priorityTime());
      case 3: m_neuroInTSPrioT_Layer3->Fill(neuroinputsegment.priorityTime());
      case 4: m_neuroInTSPrioT_Layer4->Fill(neuroinputsegment.priorityTime());
      case 5: m_neuroInTSPrioT_Layer5->Fill(neuroinputsegment.priorityTime());
      case 6: m_neuroInTSPrioT_Layer6->Fill(neuroinputsegment.priorityTime());
      case 7: m_neuroInTSPrioT_Layer7->Fill(neuroinputsegment.priorityTime());
      case 8: m_neuroInTSPrioT_Layer8->Fill(neuroinputsegment.priorityTime());
    }

  }
  m_neuroInTSCount->Fill(nofinsegments);

}


void CDCTriggerDQMModule::endRun()
{
}


void CDCTriggerDQMModule::terminate()
{
}
