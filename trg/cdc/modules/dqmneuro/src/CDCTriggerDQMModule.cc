/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sara Neuhaus, Sebastian Skambraks                        *
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
  m_neuroOutCosTheta = new TH1F("NeuroOutCosTheta",
                                "cos theta distribution of neuro tracks;cos theta ",
                                100, -1, 1);
  m_neuroOutHitPattern = new TH1F("NeuroUnpackedHitPattern",
                                  "stereo hit pattern of neuro tracks;pattern",
                                  16, 0, 16); // 4 stereo layers -> 2**4 possible patterns
  m_neuroOutPhi0 = new TH1F("NeuroOutPhi0",
                            "phi distribution from unpacker;phi [deg]",
                            161, -1.25, 361); // shift to reduce the binning error
  m_neuroOutInvPt = new TH1F("NeuroOutInvPt",
                             "Inverse Pt distribution from unpacker; [GeV^{-1}]",
                             34, 0, 3.5);
  m_neuroOutm_time = new TH1F("neuroOutM_time", "m_time distribution from unpacker",
                              48, 0, 48);
  m_neuroOutTrackCount = new TH1F("neuroOutTrackCount",
                                  "number of tracks per event",
                                  20, 0, 20);
  m_neuroOutVsInTrackCount = new TH1F("neuroOutVsInTrackCount",
                                      "number of neuroOutTracks - number of 2dinTracks",
                                      20, -10, 10);
  m_neuroInTSID = new TH1F("neuroInTSID", "ID of incoming track segments",
                           2336, 0, 2335);
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
  m_neuroInTSFoundT_Layer0 = new TH1F("neuroInTSFoundT_Layer0", "Found time of track segments in layer 0",
                                      48, 0, 48);
  m_neuroInTSFoundT_Layer1 = new TH1F("neuroInTSFoundT_Layer1", "Found time of track segments in layer 1",
                                      48, 0, 48);
  m_neuroInTSFoundT_Layer2 = new TH1F("neuroInTSFoundT_Layer2", "Found time of track segments in layer 2",
                                      48, 0, 48);
  m_neuroInTSFoundT_Layer3 = new TH1F("neuroInTSFoundT_Layer3", "Found time of track segments in layer 3",
                                      48, 0, 48);
  m_neuroInTSFoundT_Layer4 = new TH1F("neuroInTSFoundT_Layer4", "Found time of track segments in layer 4",
                                      48, 0, 48);
  m_neuroInTSFoundT_Layer5 = new TH1F("neuroInTSFoundT_Layer5", "Found time of track segments in layer 5",
                                      48, 0, 48);
  m_neuroInTSFoundT_Layer6 = new TH1F("neuroInTSFoundT_Layer6", "Found time of track segments in layer 6",
                                      48, 0, 48);
  m_neuroInTSFoundT_Layer7 = new TH1F("neuroInTSFoundT_Layer7", "Found time of track segments in layer 7",
                                      48, 0, 48);
  m_neuroInTSFoundT_Layer8 = new TH1F("neuroInTSFoundT_Layer8", "Found time of track segments in layer 8",
                                      48, 0, 48);
  m_neuroSelTSID = new TH1F("neuroSelTSID", "ID of selected track segments",
                            2336, 0, 2335);
  m_neuroSelTSPrioT_Layer0 = new TH1F("neuroSelTSPrioT_Layer0", "Priority time of track segments in layer 0",
                                      512, 0, 511);
  m_neuroSelTSPrioT_Layer1 = new TH1F("neuroSelTSPrioT_Layer1", "Priority time of track segments in layer 1",
                                      512, 0, 511);
  m_neuroSelTSPrioT_Layer2 = new TH1F("neuroSelTSPrioT_Layer2", "Priority time of track segments in layer 2",
                                      512, 0, 511);
  m_neuroSelTSPrioT_Layer3 = new TH1F("neuroSelTSPrioT_Layer3", "Priority time of track segments in layer 3",
                                      512, 0, 511);
  m_neuroSelTSPrioT_Layer4 = new TH1F("neuroSelTSPrioT_Layer4", "Priority time of track segments in layer 4",
                                      512, 0, 511);
  m_neuroSelTSPrioT_Layer5 = new TH1F("neuroSelTSPrioT_Layer5", "Priority time of track segments in layer 5",
                                      512, 0, 511);
  m_neuroSelTSPrioT_Layer6 = new TH1F("neuroSelTSPrioT_Layer6", "Priority time of track segments in layer 6",
                                      512, 0, 511);
  m_neuroSelTSPrioT_Layer7 = new TH1F("neuroSelTSPrioT_Layer7", "Priority time of track segments in layer 7",
                                      512, 0, 511);
  m_neuroSelTSPrioT_Layer8 = new TH1F("neuroSelTSPrioT_Layer8", "Priority time of track segments in layer 8",
                                      512, 0, 511);
  m_neuroSelTSFoundT_Layer0 = new TH1F("neuroSelTSFoundT_Layer0", "Found time of track segments in layer 0",
                                       48, 0, 48);
  m_neuroSelTSFoundT_Layer1 = new TH1F("neuroSelTSFoundT_Layer1", "Found time of track segments in layer 1",
                                       48, 0, 48);
  m_neuroSelTSFoundT_Layer2 = new TH1F("neuroSelTSFoundT_Layer2", "Found time of track segments in layer 2",
                                       48, 0, 48);
  m_neuroSelTSFoundT_Layer3 = new TH1F("neuroSelTSFoundT_Layer3", "Found time of track segments in layer 3",
                                       48, 0, 48);
  m_neuroSelTSFoundT_Layer4 = new TH1F("neuroSelTSFoundT_Layer4", "Found time of track segments in layer 4",
                                       48, 0, 48);
  m_neuroSelTSFoundT_Layer5 = new TH1F("neuroSelTSFoundT_Layer5", "Found time of track segments in layer 5",
                                       48, 0, 48);
  m_neuroSelTSFoundT_Layer6 = new TH1F("neuroSelTSFoundT_Layer6", "Found time of track segments in layer 6",
                                       48, 0, 48);
  m_neuroSelTSFoundT_Layer7 = new TH1F("neuroSelTSFoundT_Layer7", "Found time of track segments in layer 7",
                                       48, 0, 48);
  m_neuroSelTSFoundT_Layer8 = new TH1F("neuroSelTSFoundT_Layer8", "Found time of track segments in layer 8",
                                       48, 0, 48);
  m_neuroInTSCount = new TH1F("neuroInTSCount", " number of TS per event",
                              200, 0, 200);
  m_neuroInPhi0 = new TH1F("neuroInPhi0", "Phi0 of incoming 2dtrack",
                           161, -1.25, 361);
  m_neuroInInvPt = new TH1F("NeuroInInvPt",
                            "Inverse Pt distribution from incoming 2dtrack; [GeV^{-1}]",
                            34, 0, 3.5);
  m_neuroInm_time = new TH1F("neuroInM_time", "m_time distribution from incoming 2dtracks",
                             48, 0, 48);
  m_neuroInTrackCount = new TH1F("neuroInTrackCount", "number of neuro input 2dtracks per event",
                                 20, 0, 20);
  m_neuroInVs2DOutTrackCount = new TH1F("neuroInVs2DOutTrackCount", "Count of neuro input tracks vs. 2d output tracks",
                                        20, -10, 10);
  m_2DOutPhi0 = new TH1F("2DOutPhi0", "Phi0 of 2dtracks",
                         161, -1.25, 361);
  m_2DOutInvPt = new TH1F("2DOutInvPt",
                          "Inverse Pt of 2dtracks; [GeV^{-1}]",
                          34, 0, 3.5);
  m_2DOutm_time = new TH1F("2DOutM_time", "m_time of 2dtracks",
                           96, -48, 48);
  m_2DOutTrackCount = new TH1F("2DOutTrackCount", "number of 2dtracks per event", 20, 0, 20);
  m_neuroSelTSCount = new TH1F("neuroSelTSCount", "number of selected TS per SL", 9, 0, 8);


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
  m_unpackedNeuroInput2DTracks.isRequired(m_unpackedNeuroInput2DTracksName);
  m_unpackedNeuroInputSegments.isRequired(m_unpackedNeuroInputSegmentsName);

  m_unpackedNeuroTracks.requireRelationTo(m_unpackedNeuroInputSegments);
}

void CDCTriggerDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  m_neuroOutZ->Reset();
  m_neuroOutCosTheta->Reset();
  m_neuroOutHitPattern->Reset();
  m_neuroOutPhi0->Reset();
  m_neuroOutInvPt->Reset();
  m_neuroOutm_time->Reset();
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
  m_neuroInTSFoundT_Layer0->Reset();
  m_neuroInTSFoundT_Layer1->Reset();
  m_neuroInTSFoundT_Layer2->Reset();
  m_neuroInTSFoundT_Layer3->Reset();
  m_neuroInTSFoundT_Layer4->Reset();
  m_neuroInTSFoundT_Layer5->Reset();
  m_neuroInTSFoundT_Layer6->Reset();
  m_neuroInTSFoundT_Layer7->Reset();
  m_neuroInTSFoundT_Layer8->Reset();
  m_neuroSelTSPrioT_Layer0->Reset();
  m_neuroSelTSPrioT_Layer1->Reset();
  m_neuroSelTSPrioT_Layer2->Reset();
  m_neuroSelTSPrioT_Layer3->Reset();
  m_neuroSelTSPrioT_Layer4->Reset();
  m_neuroSelTSPrioT_Layer5->Reset();
  m_neuroSelTSPrioT_Layer6->Reset();
  m_neuroSelTSPrioT_Layer7->Reset();
  m_neuroSelTSPrioT_Layer8->Reset();
  m_neuroSelTSFoundT_Layer0->Reset();
  m_neuroSelTSFoundT_Layer1->Reset();
  m_neuroSelTSFoundT_Layer2->Reset();
  m_neuroSelTSFoundT_Layer3->Reset();
  m_neuroSelTSFoundT_Layer4->Reset();
  m_neuroSelTSFoundT_Layer5->Reset();
  m_neuroSelTSFoundT_Layer6->Reset();
  m_neuroSelTSFoundT_Layer7->Reset();
  m_neuroSelTSFoundT_Layer8->Reset();
  m_neuroInTSCount->Reset();
  m_neuroInPhi0->Reset();
  m_neuroInm_time->Reset();
  m_neuroInInvPt->Reset();
  m_neuroInTrackCount->Reset();
  m_neuroInVs2DOutTrackCount->Reset();
  m_2DOutPhi0->Reset();
  m_2DOutm_time->Reset();
  m_2DOutInvPt->Reset();
  m_2DOutTrackCount->Reset();
  m_neuroSelTSCount->Reset();
  m_neuroSelTSID->Reset();
}


void CDCTriggerDQMModule::event()
{
  // fill neurotrigger histograms
  int nofouttracks = 0;
  int nofintracks = 0;
  int nofinsegments = 0;
  int nof2douttracks = 0;
  for (CDCTriggerTrack& neuroTrack : m_unpackedNeuroTracks) {
    // count number of tracks
    nofouttracks ++;
    // fill raw distributions
    m_neuroOutZ->Fill(neuroTrack.getZ0());
    double cot = neuroTrack.getCotTheta();
    double cos = copysign(1.0, cot) / sqrt(1. / (cot * cot) + 1);
    m_neuroOutCosTheta->Fill(cos);
    m_neuroOutPhi0->Fill(neuroTrack.getPhi0() * 180 / M_PI);
    m_neuroOutInvPt->Fill(1. / neuroTrack.getPt());
    m_neuroOutm_time->Fill(neuroTrack.getTime());
    // get related stereo hits
    unsigned pattern = 0;
    for (const CDCTriggerSegmentHit& hit :
         neuroTrack.getRelationsTo<CDCTriggerSegmentHit>(m_unpackedNeuroInputSegmentsName)) {
      m_neuroSelTSID->Fill(hit.getSegmentID());
      unsigned int sl = hit.getISuperLayer();
      m_neuroSelTSCount->Fill(sl);
      switch (sl) {
        case 0: m_neuroSelTSPrioT_Layer0->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer0->Fill(hit.foundTime());
          break;
        case 1: m_neuroSelTSPrioT_Layer1->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer1->Fill(hit.foundTime());
          break;
        case 2: m_neuroSelTSPrioT_Layer2->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer2->Fill(hit.foundTime());
          break;
        case 3: m_neuroSelTSPrioT_Layer3->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer3->Fill(hit.foundTime());
          break;
        case 4: m_neuroSelTSPrioT_Layer4->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer4->Fill(hit.foundTime());
          break;
        case 5: m_neuroSelTSPrioT_Layer5->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer5->Fill(hit.foundTime());
          break;
        case 6: m_neuroSelTSPrioT_Layer6->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer6->Fill(hit.foundTime());
          break;
        case 7: m_neuroSelTSPrioT_Layer7->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer7->Fill(hit.foundTime());
          break;
        case 8: m_neuroSelTSPrioT_Layer8->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer8->Fill(hit.foundTime());
          break;
      }
      if (sl % 2 == 1) pattern |= (1 << ((sl - 1) / 2));
    }
    m_neuroOutHitPattern->Fill(pattern);
  }
  for (CDCTriggerTrack& neuroinput2dtrack : m_unpackedNeuroInput2DTracks) {
    nofintracks ++;
    m_neuroInPhi0->Fill(neuroinput2dtrack.getPhi0() * 180 / M_PI);
    m_neuroInm_time->Fill(neuroinput2dtrack.getTime());
    m_neuroInInvPt->Fill(1. / neuroinput2dtrack.getPt());
  }
  if (nofintracks > 0 || nofouttracks > 0) {
    m_neuroInTrackCount->Fill(nofintracks);
    m_neuroOutTrackCount->Fill(nofouttracks);
    m_neuroOutVsInTrackCount->Fill((nofouttracks - nofintracks));
  }
  for (CDCTriggerSegmentHit& neuroinputsegment : m_unpackedNeuroInputSegments) {
    nofinsegments ++;
    m_neuroInTSID->Fill(neuroinputsegment.getSegmentID());
    unsigned int sl = neuroinputsegment.getISuperLayer();
    switch (sl) {
      case 0: m_neuroInTSPrioT_Layer0->Fill(neuroinputsegment.priorityTime());
        m_neuroInTSFoundT_Layer0->Fill(neuroinputsegment.foundTime());
        break;
      case 1: m_neuroInTSPrioT_Layer1->Fill(neuroinputsegment.priorityTime());
        m_neuroInTSFoundT_Layer1->Fill(neuroinputsegment.foundTime());
        break;
      case 2: m_neuroInTSPrioT_Layer2->Fill(neuroinputsegment.priorityTime());
        m_neuroInTSFoundT_Layer2->Fill(neuroinputsegment.foundTime());
        break;
      case 3: m_neuroInTSPrioT_Layer3->Fill(neuroinputsegment.priorityTime());
        m_neuroInTSFoundT_Layer3->Fill(neuroinputsegment.foundTime());
        break;
      case 4: m_neuroInTSPrioT_Layer4->Fill(neuroinputsegment.priorityTime());
        m_neuroInTSFoundT_Layer4->Fill(neuroinputsegment.foundTime());
        break;
      case 5: m_neuroInTSPrioT_Layer5->Fill(neuroinputsegment.priorityTime());
        m_neuroInTSFoundT_Layer5->Fill(neuroinputsegment.foundTime());
        break;
      case 6: m_neuroInTSPrioT_Layer6->Fill(neuroinputsegment.priorityTime());
        m_neuroInTSFoundT_Layer6->Fill(neuroinputsegment.foundTime());
        break;
      case 7: m_neuroInTSPrioT_Layer7->Fill(neuroinputsegment.priorityTime());
        m_neuroInTSFoundT_Layer7->Fill(neuroinputsegment.foundTime());
        break;
      case 8: m_neuroInTSPrioT_Layer8->Fill(neuroinputsegment.priorityTime());
        m_neuroInTSFoundT_Layer8->Fill(neuroinputsegment.foundTime());
        break;
    }
  }
  if (nofinsegments > 0) {
    m_neuroInTSCount->Fill(nofinsegments);
  }
  for (CDCTriggerTrack& finder2dtrack : m_unpacked2DTracks) {
    nof2douttracks ++;
    m_2DOutPhi0->Fill(finder2dtrack.getPhi0() * 180 / M_PI);
    m_2DOutm_time->Fill(finder2dtrack.getTime());
    m_2DOutInvPt->Fill(1. / finder2dtrack.getPt());
  }
  if (nof2douttracks > 0) {
    m_neuroInVs2DOutTrackCount->Fill((nofintracks - nof2douttracks));
    m_2DOutTrackCount->Fill(nof2douttracks);
  }
}


void CDCTriggerDQMModule::endRun()
{
}


void CDCTriggerDQMModule::terminate()
{
}
