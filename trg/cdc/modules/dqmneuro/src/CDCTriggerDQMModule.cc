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
#include <framework/dataobjects/EventMetaData.h>

#include "TDirectory.h"
#include <tracking/dataobjects/RecoTrack.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCTriggerDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCTriggerDQMModule::CDCTriggerDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("CDC Trigger DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("unpackedSegmentHitsName", m_unpackedSegmentHitsName,
           "The name of the StoreArray of the unpacked CDCTriggerSegmentHits",
           string(""));
  addParam("unpacked2DTracksName", m_unpacked2DTracksName,
           "The name of the StoreArray of the unpacked 2D finder tracks",
           string("CDCTrigger2DFinderTracks"));
  addParam("unpackedNeuroTracksName", m_unpackedNeuroTracksName,
           "The name of the StoreArray of the unpacked neurotrigger tracks",
           string("CDCTriggerNeuroTracks"));
  addParam("unpackedNeuroInput2dTracksName", m_unpackedNeuroInput2DTracksName,
           "The name of the StoreArray of the neurotrigger input 2d tracks",
           string("CDCTriggerNNInput2DFinderTracks"));
  addParam("unpackedNeuroInputSegmentHits", m_unpackedNeuroInputSegmentsName,
           "The name of the StoreArray of the neurotrigger input segment hits",
           string("CDCTriggerNNInputSegmentHits"));
  addParam("simNeuroTracksName", m_simNeuroTracksName,
           "The name of the StoreArray of the neurotrigger tracks from TSIM",
           string(""));
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "Name of the directory where histograms will be placed",
           string("cdctrigger"));
  addParam("showRecoTracks", m_showRecoTracks,
           "switch to turn on a comparison with the reconstruction",
           string("no"));
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
  m_neuroSelTSFoundT_Layer0 = new TH1F("neuroSelTSFoundT_Layer0",
                                       "First found time of selected TS - found time of Neuro Track in SL 0",
                                       96, -47.99, 48.01);
  m_neuroSelTSFoundT_Layer1 = new TH1F("neuroSelTSFoundT_Layer1",
                                       "First found time of selected TS - found time of Neuro Track in SL 1",
                                       96, -47.99, 48.01);
  m_neuroSelTSFoundT_Layer2 = new TH1F("neuroSelTSFoundT_Layer2",
                                       "First found time of selected TS - found time of Neuro Track in SL 2",
                                       96, -47.99, 48.01);
  m_neuroSelTSFoundT_Layer3 = new TH1F("neuroSelTSFoundT_Layer3",
                                       "First found time of selected TS - found time of Neuro Track in SL 3",
                                       96, -47.99, 48.01);
  m_neuroSelTSFoundT_Layer4 = new TH1F("neuroSelTSFoundT_Layer4",
                                       "First found time of selected TS - found time of Neuro Track in SL 4",
                                       96, -47.99, 48.01);
  m_neuroSelTSFoundT_Layer5 = new TH1F("neuroSelTSFoundT_Layer5",
                                       "First found time of selected TS - found time of Neuro Track in SL 5",
                                       96, -47.99, 48.01);
  m_neuroSelTSFoundT_Layer6 = new TH1F("neuroSelTSFoundT_Layer6",
                                       "First found time of selected TS - found time of Neuro Track in SL 6",
                                       96, -47.99, 48.01);
  m_neuroSelTSFoundT_Layer7 = new TH1F("neuroSelTSFoundT_Layer7",
                                       "First found time of selected TS - found time of Neuro Track in SL 7",
                                       96, -47.99, 48.01);
  m_neuroSelTSFoundT_Layer8 = new TH1F("neuroSelTSFoundT_Layer8",
                                       "First found time of selected TS - found time of Neuro Track in SL 8",
                                       96, -47.99, 48.01);
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

  m_2DInTSID = new TH1F("2DInTSID", "ID of 2D incoming axial track segments",
                        2336, 0, 2335);
  m_2DInTSPrioT_Layer0 = new TH1F("2DInTSPrioT_Layer0", "Priority time of track segments in layer 0",
                                  512, 0, 511);
  m_2DInTSPrioT_Layer2 = new TH1F("2DInTSPrioT_Layer2", "Priority time of track segments in layer 2",
                                  512, 0, 511);
  m_2DInTSPrioT_Layer4 = new TH1F("2DInTSPrioT_Layer4", "Priority time of track segments in layer 4",
                                  512, 0, 511);
  m_2DInTSPrioT_Layer6 = new TH1F("2DInTSPrioT_Layer6", "Priority time of track segments in layer 6",
                                  512, 0, 511);
  m_2DInTSPrioT_Layer8 = new TH1F("2DInTSPrioT_Layer8", "Priority time of track segments in layer 8",
                                  512, 0, 511);
  m_2DInTSFoundT_Layer0 = new TH1F("2DInTSFoundT_Layer0", "Found time of track segments in layer 0",
                                   96, -48, 48);
  m_2DInTSFoundT_Layer2 = new TH1F("2DInTSFoundT_Layer2", "Found time of track segments in layer 2",
                                   96, -48, 48);
  m_2DInTSFoundT_Layer4 = new TH1F("2DInTSFoundT_Layer4", "Found time of track segments in layer 4",
                                   96, -48, 48);
  m_2DInTSFoundT_Layer6 = new TH1F("2DInTSFoundT_Layer6", "Found time of track segments in layer 6",
                                   96, -48, 48);
  m_2DInTSFoundT_Layer8 = new TH1F("2DInTSFoundT_Layer8", "Found time of track segments in layer 8",
                                   96, -48, 48);
  m_2DInTSCount = new TH1F("2DInTSCount", " number of 2D incoming TS per event",
                           200, 0, 200);

  m_neuroOutQuad5Z = new TH1F("NeuroOutQuad5Z",
                              "z distribution of neuro tracks;z [cm]",
                              100, -50, 50); // 1cm bins from -50cm to 50cm
  m_neuroOutQuad5CosTheta = new TH1F("NeuroOutQuad5CosTheta",
                                     "cos theta distribution of neuro tracks;cos theta ",
                                     100, -1, 1);
  m_neuroOutQuad5Phi0 = new TH1F("NeuroOutQuad5Phi0",
                                 "phi distribution from unpacker;phi [deg]",
                                 161, -1.25, 361); // shift to reduce the binning error
  m_neuroOutQuad5InvPt = new TH1F("NeuroOutQuad5InvPt",
                                  "Inverse Pt distribution from unpacker; [GeV^{-1}]",
                                  34, 0, 3.5);

  m_neuroOutQuad0Z = new TH1F("NeuroOutQuad0Z",
                              "z distribution of neuro tracks;z [cm]",
                              100, -50, 50); // 1cm bins from -50cm to 50cm
  m_neuroOutQuad0CosTheta = new TH1F("NeuroOutQuad0CosTheta",
                                     "cos theta distribution of neuro tracks;cos theta ",
                                     100, -1, 1);
  m_neuroOutQuad0Phi0 = new TH1F("NeuroOutQuad0Phi0",
                                 "phi distribution from unpacker;phi [deg]",
                                 161, -1.25, 361); // shift to reduce the binning error
  m_neuroOutQuad0InvPt = new TH1F("NeuroOutQuad0InvPt",
                                  "Inverse Pt distribution from unpacker; [GeV^{-1}]",
                                  34, 0, 3.5);

  m_neuroOutQuad1Z = new TH1F("NeuroOutQuad1Z",
                              "z distribution of neuro tracks;z [cm]",
                              100, -50, 50); // 1cm bins from -50cm to 50cm
  m_neuroOutQuad1CosTheta = new TH1F("NeuroOutQuad1CosTheta",
                                     "cos theta distribution of neuro tracks;cos theta ",
                                     100, -1, 1);
  m_neuroOutQuad1Phi0 = new TH1F("NeuroOutQuad1Phi0",
                                 "phi distribution from unpacker;phi [deg]",
                                 161, -1.25, 361); // shift to reduce the binning error
  m_neuroOutQuad1InvPt = new TH1F("NeuroOutQuad1InvPt",
                                  "Inverse Pt distribution from unpacker; [GeV^{-1}]",
                                  34, 0, 3.5);

  m_neuroOutQuad2Z = new TH1F("NeuroOutQuad2Z",
                              "z distribution of neuro tracks;z [cm]",
                              100, -50, 50); // 1cm bins from -50cm to 50cm
  m_neuroOutQuad2CosTheta = new TH1F("NeuroOutQuad2CosTheta",
                                     "cos theta distribution of neuro tracks;cos theta ",
                                     100, -1, 1);
  m_neuroOutQuad2Phi0 = new TH1F("NeuroOutQuad2Phi0",
                                 "phi distribution from unpacker;phi [deg]",
                                 161, -1.25, 361); // shift to reduce the binning error
  m_neuroOutQuad2InvPt = new TH1F("NeuroOutQuad2InvPt",
                                  "Inverse Pt distribution from unpacker; [GeV^{-1}]",
                                  34, 0, 3.5);

  m_neuroOutQuad3Z = new TH1F("NeuroOutQuad3Z",
                              "z distribution of neuro tracks;z [cm]",
                              100, -50, 50); // 1cm bins from -50cm to 50cm
  m_neuroOutQuad3CosTheta = new TH1F("NeuroOutQuad3CosTheta",
                                     "cos theta distribution of neuro tracks;cos theta ",
                                     100, -1, 1);
  m_neuroOutQuad3Phi0 = new TH1F("NeuroOutQuad3Phi0",
                                 "phi distribution from unpacker;phi [deg]",
                                 161, -1.25, 361); // shift to reduce the binning error
  m_neuroOutQuad3InvPt = new TH1F("NeuroOutQuad3InvPt",
                                  "Inverse Pt distribution from unpacker; [GeV^{-1}]",
                                  34, 0, 3.5);

  if (m_simNeuroTracksName != "") {
    m_neuroSector = new TH1F("NeuroSector",
                             "unpacked sector",
                             10, 0, 10);
    m_neuroDeltaZ = new TH1F("NeuroDeltaZ",
                             "difference between unpacked and simulated neuro z;delta z [cm]",
                             100, -100, 100); // should be bit-precise, so look at very small range
    m_neuroDeltaTheta = new TH1F("NeuroDeltaTheta",
                                 "difference between unpacked and simulated neuro theta;delta theta [deg]",
                                 100, -180, 180); // should be bit-precise, so look at very small range
    m_neuroScatterZ = new TH2F("NeuroScatterZ",
                               "unpacked z vs TSIM z [cm]",
                               100, -150, 150, 100, -150, 150);

    m_neuroDeltaInputID = new TH1F("NeuroDeltaInputID",
                                   "difference between unpacked and simulated ID input;delta ID",
                                   100, -0.5, 0.5); // should be bit-precise, so look at very small range
    m_neuroDeltaInputT = new TH1F("NeuroDeltaInputT",
                                  "difference between unpacked and simulated time input;delta t",
                                  100, -0.5, 0.5); // should be bit-precise, so look at very small range
    m_neuroDeltaInputAlpha = new TH1F("NeuroDeltaInputAlpha",
                                      "difference between unpacked and simulated alpha input;delta alpha",
                                      100, -0.1, 0.1); // should be bit-precise, so look at very small range
    m_neuroDeltaTSID = new TH1F("NeuroDeltaTSID",
                                "difference between unpacked and simulated tsid",
                                100, -50, 50);
    m_neuroDeltaSector = new TH1F("NeuroDeltaSector",
                                  "difference between unpacked and simulated sector",
                                  20, -10, 10);
    m_simSameTS = new TH1F("NeuroSimSameTS",
                           "number of TS selected in both, unpacked and TSIM tracks",
                           20, 0, 20);
    m_simDiffTS = new TH1F("NeuroSimDiffTS",
                           "number of TS selcted in TSIM but not in unpacker",
                           20, 0, 20);

    m_neuroOutSWZ = new TH1F("NeuroOutSWZ",
                             "sw z distribution of neuro tracks;z [cm]",
                             100, -50, 50); // 1cm bins from -50cm to 50cm
    m_neuroOutSWCosTheta = new TH1F("NeuroOutSWCosTheta",
                                    "sw cos theta distribution of neuro tracks;cos theta ",
                                    100, -1, 1);
    m_neuroOutSWPhi0 = new TH1F("NeuroOutSWPhi0",
                                "sw phi distribution from unpacker;phi [deg]",
                                161, -1.25, 361); // shift to reduce the binning error
    m_neuroOutSWInvPt = new TH1F("NeuroOutSWInvPt",
                                 "sw Inverse Pt distribution from unpacker; [GeV^{-1}]",
                                 34, 0, 3.5);
  }

  if (m_showRecoTracks == "yes") {
    //RecoTracks
    m_RecoZ = new TH1F("RecoZ",
                       "z distribution of reconstructed tracks;z [cm]",
                       100, -150, 150); // 1cm bins from -50cm to 50cm
    m_RecoCosTheta = new TH1F("RecoCosTheta",
                              "cos theta distribution of reconstructed tracks;cos theta ",
                              100, -1, 1);
    m_RecoPhi = new TH1F("RecoPhi",
                         "phi distribution of reconstructed tracks ;phi [deg]",
                         160, -180, 180);
    m_RecoInvPt = new TH1F("RecoInvPt",
                           "Inverse Pt distribution of reconstructed tracks; [GeV^{-1}]",
                           34, 0, 3.5);

    //RecoTracks matched to unpacked neuro tracks
    m_RecoHWZ = new TH1F("RecoHWZ",
                         "hw matched z distribution of reconstructed tracks;z [cm]",
                         100, -150, 150); // 1cm bins from -50cm to 50cm
    m_RecoHWCosTheta = new TH1F("RecoHWCosTheta",
                                "hw matched cos theta distribution of reconstructed tracks;cos theta ",
                                100, -1, 1);
    m_RecoHWPhi = new TH1F("RecoHWPhi",
                           "hw matched phi distribution of reconstructed tracks ;phi [deg]",
                           160, -180, 180);
    m_RecoHWInvPt = new TH1F("RecoHWInvPt",
                             "hw matched inverse Pt distribution of reconstructed tracks; [GeV^{-1}]",
                             34, 0, 3.5);
    m_RecoHWZScatter = new TH2F("RecoHWZScatter",
                                "hw matched reconstruction; scatter z [cm]",
                                100, -150, 150, 100, -150, 150);

    //RecoTracks matched to simulated neuro tracks
    m_RecoSWZ = new TH1F("RecoSWZ",
                         "sw matched z distribution of reconstructed tracks;z [cm]",
                         100, -150, 150); // 1cm bins from -50cm to 50cm
    m_RecoSWCosTheta = new TH1F("RecoSWCosTheta",
                                "sw matched cos theta distribution of reconstructed tracks;cos theta ",
                                100, -1, 1);
    m_RecoSWPhi = new TH1F("RecoSWPhi",
                           "sw matched phi distribution of reconstructed tracks ;phi [deg]",
                           160, -180, 180);
    m_RecoSWInvPt = new TH1F("RecoSWInvPt",
                             "sw matched inverse Pt distribution of reconstructed tracks; [GeV^{-1}]",
                             34, 0, 3.5);
    m_RecoSWZScatter = new TH2F("RecoSWZScatter",
                                "sw matched reconstruction; scatter z [cm]",
                                100, -150, 150, 100, -150, 150);

    m_DeltaRecoHWZ = new TH1F("DeltaRecoHWZ",
                              "difference between reconstructed and unpacked neuro z;delta z [cm]",
                              100, -100, 100);
    m_DeltaRecoHWCosTheta = new TH1F("DeltaRecoHWCosTheta",
                                     "difference between reconstructed and unpacked neuro cos(theta);delta cos(theta)",
                                     100, -1, 1);
    m_DeltaRecoHWInvPt = new TH1F("DeltaRecoHWInvPt",
                                  "difference between reconstructed and unpacked neuro InvPt;delta InvPt [GeV^-1]",
                                  100, -100, 100);
    m_DeltaRecoHWPhi = new TH1F("DeltaRecoHWPhi",
                                "difference between reconstructed and unpacked neuro phi;delta phi [deg]",
                                160, -180, 180);

    m_DeltaRecoSWZ = new TH1F("DeltaRecoSWZ",
                              "difference between reconstructed and simulated neuro z;delta z [cm]",
                              100, -100, 100);
    m_DeltaRecoSWCosTheta = new TH1F("DeltaRecoSWCosTheta",
                                     "difference between reconstructed and simulated neuro cos(theta);delta cos(theta)",
                                     100, -1, 1);
    m_DeltaRecoSWInvPt = new TH1F("DeltaRecoSWInvPt",
                                  "difference between reconstructed and simulated neuro InvPt;delta InvPt [GeV^-1]",
                                  100, -100, 100);
    m_DeltaRecoSWPhi = new TH1F("DeltaRecoSWPhi",
                                "difference between reconstructed and simulated neuro phi;delta phi [deg]",
                                160, -180, 180);
  }

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

  if (m_simNeuroTracksName != "") {
    m_unpackedNeuroInputName = m_unpackedNeuroTracksName + "Input";
    m_unpackedNeuroInput.isRequired(m_unpackedNeuroInputName);
    m_unpackedNeuroTracks.requireRelationTo(m_unpackedNeuroInput);
    m_simNeuroInputName = m_simNeuroTracksName + "Input";
    m_simNeuroTracks.isRequired(m_simNeuroTracksName);
    m_simNeuroInput.isRequired(m_simNeuroInputName);
    m_unpackedNeuroInput2DTracks.requireRelationTo(m_simNeuroTracks);
    m_simNeuroTracks.requireRelationTo(m_simNeuroInput);
  }
  if (m_showRecoTracks == "yes") {
    m_RecoTracks.isRequired("RecoTracks");
    m_RecoTracks.requireRelationTo(m_unpackedNeuroTracks);
    if (m_simNeuroTracksName != "") {
      m_RecoTracks.requireRelationTo(m_simNeuroTracks);
    }
  }
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
  m_2DInTSID->Reset();
  m_2DInTSPrioT_Layer0->Reset();
  m_2DInTSPrioT_Layer2->Reset();
  m_2DInTSPrioT_Layer4->Reset();
  m_2DInTSPrioT_Layer6->Reset();
  m_2DInTSPrioT_Layer8->Reset();
  m_2DInTSFoundT_Layer0->Reset();
  m_2DInTSFoundT_Layer2->Reset();
  m_2DInTSFoundT_Layer4->Reset();
  m_2DInTSFoundT_Layer6->Reset();
  m_2DInTSFoundT_Layer8->Reset();
  m_2DInTSCount->Reset();

  m_neuroOutQuad5Z->Reset();
  m_neuroOutQuad5CosTheta->Reset();
  m_neuroOutQuad5Phi0->Reset();
  m_neuroOutQuad5InvPt->Reset();

  m_neuroOutQuad0Z->Reset();
  m_neuroOutQuad0CosTheta->Reset();
  m_neuroOutQuad0Phi0->Reset();
  m_neuroOutQuad0InvPt->Reset();

  m_neuroOutQuad1Z->Reset();
  m_neuroOutQuad1CosTheta->Reset();
  m_neuroOutQuad1Phi0->Reset();
  m_neuroOutQuad1InvPt->Reset();

  m_neuroOutQuad2Z->Reset();
  m_neuroOutQuad2CosTheta->Reset();
  m_neuroOutQuad2Phi0->Reset();
  m_neuroOutQuad2InvPt->Reset();

  m_neuroOutQuad3Z->Reset();
  m_neuroOutQuad3CosTheta->Reset();
  m_neuroOutQuad3Phi0->Reset();
  m_neuroOutQuad3InvPt->Reset();

  if (m_simNeuroTracksName != "") {
    m_neuroSector->Reset();
    m_neuroDeltaZ->Reset();
    m_neuroDeltaTheta->Reset();
    m_neuroScatterZ->Reset();
    m_neuroDeltaInputID->Reset();
    m_neuroDeltaInputT->Reset();
    m_neuroDeltaInputAlpha->Reset();
    m_neuroDeltaTSID->Reset();
    m_neuroDeltaSector->Reset();
    m_simSameTS->Reset();
    m_simDiffTS->Reset();
    m_neuroOutSWZ->Reset();
    m_neuroOutSWCosTheta->Reset();
    m_neuroOutSWPhi0->Reset();
    m_neuroOutSWInvPt->Reset();
  }
  if (m_showRecoTracks == "yes") {
    m_RecoZ->Reset();
    m_RecoCosTheta->Reset();
    m_RecoInvPt->Reset();
    m_RecoPhi->Reset();

    m_RecoHWZ->Reset();
    m_RecoHWCosTheta->Reset();
    m_RecoHWInvPt->Reset();
    m_RecoHWPhi->Reset();
    m_RecoHWZScatter->Reset();

    m_RecoSWZ->Reset();
    m_RecoSWCosTheta->Reset();
    m_RecoSWInvPt->Reset();
    m_RecoSWPhi->Reset();
    m_RecoSWZScatter->Reset();

    m_DeltaRecoHWZ->Reset();
    m_DeltaRecoHWCosTheta->Reset();
    m_DeltaRecoHWInvPt->Reset();
    m_DeltaRecoHWPhi->Reset();

    m_DeltaRecoSWZ->Reset();
    m_DeltaRecoSWCosTheta->Reset();
    m_DeltaRecoSWInvPt->Reset();
    m_DeltaRecoSWPhi->Reset();
  }
}


void CDCTriggerDQMModule::event()
{
  if (m_unpackedNeuroInputSegments.getEntries() == 0)
    return;
  if (m_showRecoTracks == "yes") {
    // a RecoTrack has multiple representations for different particle hypothesis
    // -> just take the first one that does not give errors.
    bool foundValidRep = false;
    for (RecoTrack& recoTrack : m_RecoTracks) {
      float phi0Target = 0;
      float invptTarget = 0;
      float cosThetaTarget = 0;
      float zTarget = 0;
      for (genfit::AbsTrackRep* rep : recoTrack.getRepresentations()) {
        if (!recoTrack.wasFitSuccessful(rep))
          continue;
        // get state (position, momentum etc.) from hit closest to IP and
        // extrapolate to z-axis (may throw an exception -> continue to next representation)
        try {
          genfit::MeasuredStateOnPlane state =
            recoTrack.getMeasuredStateOnPlaneClosestTo(TVector3(0, 0, 0), rep);
          rep->extrapolateToLine(state, TVector3(0, 0, -1000), TVector3(0, 0, 2000));
          // TODO check after matching
          //  // flip tracks if necessary, such that trigger tracks and reco tracks
          //  // point in the same direction
          //  if (state.getMom().Dot(m_tracks[itrack]->getDirection()) < 0) {
          //    state.setPosMom(state.getPos(), -state.getMom());
          //    state.setChargeSign(-state.getCharge());
          //  }
          // get track parameters
          phi0Target = state.getMom().Phi();
          invptTarget = state.getCharge() / state.getMom().Pt();
          cosThetaTarget = state.getMom().CosTheta();
          zTarget = state.getPos().Z();
        } catch (...) {
          continue;
        }
        // break loop
        foundValidRep = true;
        break;
      }
      if (!foundValidRep) {
        B2DEBUG(150, "No valid representation found for RecoTrack, skipping.");
        continue;
      } else {
        m_RecoZ->Fill(zTarget);
        m_RecoCosTheta->Fill(cosThetaTarget);
        m_RecoPhi->Fill(phi0Target * 180 / M_PI);
        m_RecoInvPt->Fill(invptTarget);
        CDCTriggerTrack* neuroHWTrack = recoTrack.getRelatedTo<CDCTriggerTrack>(m_unpackedNeuroTracksName);
        if (neuroHWTrack) {
          m_RecoHWZ->Fill(zTarget);
          m_RecoHWCosTheta->Fill(cosThetaTarget);
          m_RecoHWPhi->Fill(phi0Target * 180 / M_PI);
          m_RecoHWInvPt->Fill(invptTarget);

          m_DeltaRecoHWZ->Fill(zTarget - neuroHWTrack->getZ0());
          double cotTh = neuroHWTrack->getCotTheta();
          double cosTh = copysign(1.0, cotTh) / sqrt(1. / (cotTh * cotTh) + 1);
          m_DeltaRecoHWCosTheta->Fill(cosThetaTarget - cosTh);
          m_DeltaRecoHWPhi->Fill((phi0Target - neuroHWTrack->getPhi0()) * 180 / M_PI);
          m_DeltaRecoHWInvPt->Fill(invptTarget - 1. / neuroHWTrack->getPt());
          m_RecoHWZScatter->Fill(zTarget, neuroHWTrack->getZ0());
        }
        CDCTriggerTrack* neuroSWTrack = recoTrack.getRelatedTo<CDCTriggerTrack>(m_simNeuroTracksName);
        if (neuroSWTrack) {
          m_RecoSWZ->Fill(zTarget);
          m_RecoSWCosTheta->Fill(cosThetaTarget);
          m_RecoSWPhi->Fill(phi0Target * 180 / M_PI);
          m_RecoSWInvPt->Fill(invptTarget);

          m_DeltaRecoSWZ->Fill(zTarget - neuroSWTrack->getZ0());
          double cotTh = neuroSWTrack->getCotTheta();
          double cosTh = copysign(1.0, cotTh) / sqrt(1. / (cotTh * cotTh) + 1);
          m_DeltaRecoSWCosTheta->Fill(cosThetaTarget - cosTh);
          m_DeltaRecoSWPhi->Fill((phi0Target - neuroSWTrack->getPhi0()) * 180 / M_PI);
          m_DeltaRecoSWInvPt->Fill(invptTarget - 1. / neuroSWTrack->getPt());
          m_RecoSWZScatter->Fill(zTarget, neuroSWTrack->getZ0());
        }
      }
    }
  }
  for (CDCTriggerTrack& neuroswTrack : m_simNeuroTracks) {
    m_neuroOutSWZ->Fill(neuroswTrack.getZ0());
    double cotThSW = neuroswTrack.getCotTheta();
    double cosThSW = copysign(1.0, cotThSW) / sqrt(1. / (cotThSW * cotThSW) + 1);
    m_neuroOutSWCosTheta->Fill(cosThSW);
    m_neuroOutSWPhi0->Fill(neuroswTrack.getPhi0() * 180 / M_PI);
    m_neuroOutSWInvPt->Fill(1. / neuroswTrack.getPt());
  }
  // fill neurotrigger histograms
  int nofouttracks = 0;
  int nofintracks = 0;
  int nofinsegments = 0;
  int nof2douttracks = 0;
  int nof2dinsegments = 0;
  for (CDCTriggerTrack& neuroTrack : m_unpackedNeuroTracks) {
    // count number of tracks
    nofouttracks ++;
    // fill raw distributions
    m_neuroOutZ->Fill(neuroTrack.getZ0());
    double cotTh = neuroTrack.getCotTheta();
    double cosTh = copysign(1.0, cotTh) / sqrt(1. / (cotTh * cotTh) + 1);
    m_neuroOutCosTheta->Fill(cosTh);
    m_neuroOutPhi0->Fill(neuroTrack.getPhi0() * 180 / M_PI);
    m_neuroOutInvPt->Fill(1. / neuroTrack.getPt());
    m_neuroOutm_time->Fill(neuroTrack.getTime());

    // fill hists per quadrant
    switch (neuroTrack.getQuadrant()) {
      case -1:
        m_neuroOutQuad5Z->Fill(neuroTrack.getZ0());
        m_neuroOutQuad5CosTheta->Fill(cosTh);
        m_neuroOutQuad5Phi0->Fill(neuroTrack.getPhi0() * 180 / M_PI);
        m_neuroOutQuad5InvPt->Fill(1. / neuroTrack.getPt());
        break;
      case 0:
        m_neuroOutQuad0Z->Fill(neuroTrack.getZ0());
        m_neuroOutQuad0CosTheta->Fill(cosTh);
        m_neuroOutQuad0Phi0->Fill(neuroTrack.getPhi0() * 180 / M_PI);
        m_neuroOutQuad0InvPt->Fill(1. / neuroTrack.getPt());
        break;
      case 1:
        m_neuroOutQuad1Z->Fill(neuroTrack.getZ0());
        m_neuroOutQuad1CosTheta->Fill(cosTh);
        m_neuroOutQuad1Phi0->Fill(neuroTrack.getPhi0() * 180 / M_PI);
        m_neuroOutQuad1InvPt->Fill(1. / neuroTrack.getPt());
        break;
      case 2:
        m_neuroOutQuad2Z->Fill(neuroTrack.getZ0());
        m_neuroOutQuad2CosTheta->Fill(cosTh);
        m_neuroOutQuad2Phi0->Fill(neuroTrack.getPhi0() * 180 / M_PI);
        m_neuroOutQuad2InvPt->Fill(1. / neuroTrack.getPt());
        break;
      case 3:
        m_neuroOutQuad3Z->Fill(neuroTrack.getZ0());
        m_neuroOutQuad3CosTheta->Fill(cosTh);
        m_neuroOutQuad3Phi0->Fill(neuroTrack.getPhi0() * 180 / M_PI);
        m_neuroOutQuad3InvPt->Fill(1. / neuroTrack.getPt());
        break;
    }

    // get related stereo hits
    unsigned pattern = 0;
    for (const CDCTriggerSegmentHit& hit :
         neuroTrack.getRelationsTo<CDCTriggerSegmentHit>(m_unpackedNeuroInputSegmentsName)) {
      m_neuroSelTSID->Fill(hit.getSegmentID());
      unsigned int sl = hit.getISuperLayer();
      m_neuroSelTSCount->Fill(sl);
      float neuroTime = neuroTrack.getTime();
      // find first occurence of hit (used to debug the selcted TS field)
      CDCTriggerSegmentHit firstHit = hit;

      for (CDCTriggerSegmentHit compare : m_unpackedNeuroInputSegments) {
        if (compare.getISuperLayer()       == hit.getISuperLayer()      &&
            compare.getIWireCenter()       == hit.getIWireCenter()      &&
            compare.getPriorityPosition()  == hit.getPriorityPosition() &&
            compare.getLeftRight()         == hit.getLeftRight()        &&
            compare.priorityTime()         == hit.priorityTime()        &&
            compare.foundTime()            < firstHit.foundTime()) {
          firstHit = compare;
          //break; //TODO check: break is only possible if the TS list is sorted by foundTime
        }
      }

      switch (sl) {
        case 0: m_neuroSelTSPrioT_Layer0->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer0->Fill(firstHit.foundTime() - neuroTime);
          break;
        case 1: m_neuroSelTSPrioT_Layer1->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer1->Fill(firstHit.foundTime() - neuroTime);
          break;
        case 2: m_neuroSelTSPrioT_Layer2->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer2->Fill(firstHit.foundTime() - neuroTime);
          break;
        case 3: m_neuroSelTSPrioT_Layer3->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer3->Fill(firstHit.foundTime() - neuroTime);
          break;
        case 4: m_neuroSelTSPrioT_Layer4->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer4->Fill(firstHit.foundTime() - neuroTime);
          break;
        case 5: m_neuroSelTSPrioT_Layer5->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer5->Fill(firstHit.foundTime() - neuroTime);
          break;
        case 6: m_neuroSelTSPrioT_Layer6->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer6->Fill(firstHit.foundTime() - neuroTime);
          break;
        case 7: m_neuroSelTSPrioT_Layer7->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer7->Fill(firstHit.foundTime() - neuroTime);
          break;
        case 8: m_neuroSelTSPrioT_Layer8->Fill(hit.priorityTime());
          m_neuroSelTSFoundT_Layer8->Fill(firstHit.foundTime() - neuroTime);
          break;
      }
      if (sl % 2 == 1) pattern |= (1 << ((sl - 1) / 2));
    }
    m_neuroOutHitPattern->Fill(pattern);

    if (m_simNeuroTracksName != "") {
      // get related track from TSIM (via 2D finder track)
      CDCTriggerTrack* finderTrack =
        neuroTrack.getRelatedTo<CDCTriggerTrack>(m_unpackedNeuroInput2DTracksName);
      if (finderTrack) {
        CDCTriggerTrack* neuroSimTrack =
          finderTrack->getRelatedTo<CDCTriggerTrack>(m_simNeuroTracksName);
        if (neuroSimTrack) {
          // check if they same TS are selected in the unpacked and TSIM track
          int nsameTS = 0;
          int ndiffTS = 0;
          for (const CDCTriggerSegmentHit& simhit :
               neuroSimTrack->getRelationsTo<CDCTriggerSegmentHit>(m_unpackedNeuroInputSegmentsName)) {
            unsigned int simsl = simhit.getISuperLayer();
            for (const CDCTriggerSegmentHit& hit :
                 neuroTrack.getRelationsTo<CDCTriggerSegmentHit>(m_unpackedNeuroInputSegmentsName)) {
              unsigned int sl = hit.getISuperLayer();
              if (sl == simsl) {
                m_neuroDeltaTSID->Fill(hit.getSegmentID() - simhit.getSegmentID());
                if (simhit.getSegmentID() == hit.getSegmentID() &&
                    simhit.getPriorityPosition() == hit.getPriorityPosition() &&
                    simhit.getLeftRight() == hit.getLeftRight() &&
                    simhit.priorityTime() == hit.priorityTime()
                   ) {
                  nsameTS += 1;
                } else {
                  ndiffTS += 1;
                }
              }
            }
          }
          m_simSameTS->Fill(nsameTS);
          m_simDiffTS->Fill(ndiffTS);
          // only calculate deltas if the same TS are selected in unpacker and TSIM
          // TODO allow less then 9 TS per track

          StoreObjPtr<EventMetaData> eventMetaData;

          if (nsameTS >= 8) {
            m_neuroDeltaZ->Fill(neuroTrack.getZ0() - neuroSimTrack->getZ0());
            m_neuroDeltaTheta->Fill(neuroTrack.getDirection().Theta() * 180. / M_PI -
                                    neuroSimTrack->getDirection().Theta() * 180. / M_PI);
            m_neuroScatterZ->Fill(neuroTrack.getZ0(), neuroSimTrack->getZ0());
            vector<float> unpackedInput =
              neuroTrack.getRelatedTo<CDCTriggerMLPInput>(m_unpackedNeuroInputName)->getInput();
            vector<float> simInput =
              neuroSimTrack->getRelatedTo<CDCTriggerMLPInput>(m_simNeuroInputName)->getInput();
            unsigned unpackedSector =
              neuroTrack.getRelatedTo<CDCTriggerMLPInput>(m_unpackedNeuroInputName)->getSector();
            unsigned simSector =
              neuroSimTrack->getRelatedTo<CDCTriggerMLPInput>(m_simNeuroInputName)->getSector();
            m_neuroSector->Fill(unpackedSector);
            m_neuroDeltaSector->Fill(unpackedSector - simSector);

            cout << endl;
            cout << "--------------------------------------------------------------------------"
                 << endl;
            cout << "Experiment " << eventMetaData->getExperiment() << "  Run " <<
                 eventMetaData->getRun() << "  Event " << eventMetaData->getEvent();

            cout << endl << endl;
            cout << "Global values (angles in degrees)" << endl;
            cout << "HW (pt, omega, phi, theta, z) = (" << setw(8) << neuroTrack.getPt() << ", " << setw(8) << neuroTrack.getOmega() << ", " <<
                 setw(8) << neuroTrack.getPhi0() * 180. / M_PI << ", " <<  setw(8) << neuroTrack.getDirection().Theta() * 180. / M_PI << ", " <<
                 setw(8) << neuroTrack.getZ0() << ")" << endl;
            cout << "SW (pt, omega, phi, theta, z) = (" <<  setw(8) << neuroSimTrack->getPt() << ", " <<  setw(
                   8) << neuroSimTrack->getOmega() << ", " <<
                 setw(8) << neuroSimTrack->getPhi0() * 180. / M_PI << ", " <<  setw(8) << neuroSimTrack->getDirection().Theta() * 180. / M_PI << ", "
                 <<  setw(8) << neuroSimTrack->getZ0() << ")" << endl;

            const double BField = 1.5e-4;
            cout << "quadrant " << neuroTrack.getQuadrant() << endl;

            double phiGlob = neuroTrack.getPhi0();
            double phiQ = phiGlob - M_PI / 2 * neuroTrack.getQuadrant();
            if (phiQ < 0)
              phiQ += 2 * M_PI;
            double phiR = (phiQ - M_PI / 4) * 2 * 80 / M_PI - 1;
            double omegaRaw = neuroTrack.getOmega() * 0.3 * 34 / (Const::speedOfLight * BField);
            cout << "HW (phi quadrant, phi raw, omega raw) = (" << phiQ * 180. / M_PI << ", " << phiR << ", " << omegaRaw << ")" << endl;

            double phiglobSW = neuroSimTrack->getPhi0() - M_PI / 4;
            if (phiglobSW < 0)
              phiglobSW += 2 * M_PI;
            double quadphi = fmod(phiglobSW, M_PI / 2);
            double rawphi = (quadphi) * (2 * 80) / M_PI - 1;
            omegaRaw = neuroSimTrack->getOmega() * 0.3 * 34 / (Const::speedOfLight * BField);
            cout << "SW (phi quadrant, phi raw, omega raw) = (" << quadphi * 180. / M_PI + 45 << ", " << rawphi << ", " << omegaRaw << ")" <<
                 endl;

            /* number of wires in a super layer*/
            static constexpr std::array<int, 9> nWiresInSuperLayer = {
              160, 160, 192, 224, 256, 288, 320, 352, 384
            };
            cout << "priority: (0: no hit, 3: 1st priority, 1: 2nd right, 2: 2nd left); left/right:  (0: no hit, 1: right, 2: left, 3: not determined)"
                 << endl;
            cout << "All TS 2D   (SL, quadrant, segment id, relative id in SL,  priority position, left right, priority time, found time, raw Tracker ID)"
                 << endl;
            for (const CDCTriggerSegmentHit& xhit : m_unpackedSegmentHits) {
              cout << "(" << setw(5) << xhit.getISuperLayer() << ", " << setw(5) << xhit.getQuadrant() << ", "
                   << setw(5) << xhit.getSegmentID() << ", " << setw(5) << xhit.getIWire() << ", "
                   << setw(5) << xhit.getPriorityPosition() << ", " << setw(5) << xhit.getLeftRight() << ", "
                   << setw(5) << xhit.priorityTime() << ", " << setw(5) << xhit.foundTime();
              int iSL = xhit.getISuperLayer();
              int iTS = xhit.getIWire();
              int nwires = nWiresInSuperLayer[ iSL ];
              if (iSL == 8) {
                iTS += 16;
                if (iTS > nwires) {
                  iTS -= nwires;
                }
              }
              int tsIDInTracker = iTS;
              if (iTS > nwires / 2) {
                tsIDInTracker -= (nwires / 2);
              }
              cout << ", " << setw(5) << tsIDInTracker << ")" << endl;
            }
            cout << "All TS NN   (SL, quadrant, segment id, relative id in SL,  priority position, left right, priority time, found time, raw Tracker ID)"
                 << endl;
            for (const CDCTriggerSegmentHit& xhit : m_unpackedNeuroInputSegments) {
              cout << "(" << setw(5) << xhit.getISuperLayer() << ", " << setw(5) << xhit.getQuadrant() << ", "
                   << setw(5) << xhit.getSegmentID() << ", " << setw(5) << xhit.getIWire() << ", "
                   << setw(5) << xhit.getPriorityPosition() << ", " << setw(5) << xhit.getLeftRight() << ", "
                   << setw(5) << xhit.priorityTime() << ", " << setw(5) << xhit.foundTime();
              int iSL = xhit.getISuperLayer();
              int iTS = xhit.getIWire();
              int nwires = nWiresInSuperLayer[ iSL ];
              if (iSL == 8) {
                iTS += 16;
                if (iTS > nwires) {
                  iTS -= nwires;
                }
              }
              int tsIDInTracker = iTS;
              if (iTS > nwires / 2) {
                tsIDInTracker -= (nwires / 2);
              }
              cout << ", " << setw(5) << tsIDInTracker << ")" << endl;
            }
            cout << "Selected TS (SL, quadrant, segment id, relative id in SL,  priority position, left right, priority time, found time, raw Tracker ID)"
                 << endl;

            for (const CDCTriggerSegmentHit& xhit :
                 neuroSimTrack->getRelationsTo<CDCTriggerSegmentHit>(m_unpackedNeuroInputSegmentsName)) {
              cout << "(" << setw(5) << xhit.getISuperLayer() << ", " << setw(5) << xhit.getQuadrant() << ", "
                   << setw(5) << xhit.getSegmentID() << ", " << setw(5) << xhit.getIWire() << ", "
                   << setw(5) << xhit.getPriorityPosition() << ", " << setw(5) << xhit.getLeftRight() << ", "
                   << setw(5) << xhit.priorityTime() << ", " << setw(5) << xhit.foundTime();
              int iSL = xhit.getISuperLayer();
              int iTS = xhit.getIWire();
              int nwires = nWiresInSuperLayer[ iSL ];
              if (iSL == 8) {
                iTS += 16;
                if (iTS > nwires) {
                  iTS -= nwires;
                }
              }
              int tsIDInTracker = iTS;
              if (iTS > nwires / 2) {
                tsIDInTracker -= (nwires / 2);
              }
              cout << ", " << setw(5) << tsIDInTracker << ")" << endl;
            }
            cout << "Unpacked sector " << unpackedSector << ", sim sector " << simSector << endl;
            cout << "Input Vector unpacked (id, t, alpha), sim (id, t, alpha), delta (id, t, alpha):" << endl;
            for (unsigned ii = 0; ii < unpackedInput.size(); ii += 3) {
              cout << endl;
              cout << ii / 3 << ") ";
              cout <<  "(" << setw(11) << unpackedInput[ii]  << ", "               << setw(11) << unpackedInput[ii + 1] << ", "
                   << setw(11) << unpackedInput[ii + 2] << "), ";
              cout <<  "(" << setw(11) << simInput[ii]       << ", "               << setw(11) << simInput[ii + 1]      << ", "
                   << setw(11) << simInput[ii + 2] << "), ";
              cout <<  "(" << setw(11) << unpackedInput[ii] - simInput[ii] << ", " << setw(11) << unpackedInput[ii + 1] - simInput[ii + 1] << ", "
                   << setw(11) << unpackedInput[ii + 2] - simInput[ii + 2] << "), " << endl;
              cout << "   (" << setw(11) << unpackedInput[ii] * 4096 << ", "                        << setw(
                     11) << unpackedInput[ii + 1] * 4096 << ", "                           << setw(11) << unpackedInput[ii + 2] * 4096 << "), ";
              cout << "(" << setw(11) << simInput[ii]      * 4096 << ", "                        << setw(11) << simInput[ii + 1]      * 4096 <<
                   ", "                           << setw(11) << simInput[ii + 2]      * 4096 << "), ";
              cout << "(" << setw(11) << unpackedInput[ii] * 4096 - simInput[ii] * 4096  << ", " << setw(11) << unpackedInput[ii + 1] * 4096 -
                   simInput[ii + 1] * 4096 << ", " << setw(11) << unpackedInput[ii + 2] * 4096 - simInput[ii + 2] * 4096 << "), " << endl;

              cout << hex;
              cout.setf(ios::showbase);
              cout << "   (" << setw(11) << (int)(unpackedInput[ii]  * 4096) << ", "                       << setw(11) << (int)(
                     unpackedInput[ii + 1] * 4096) << ", "                            << setw(11) << (int)(unpackedInput[ii + 2] * 4096) << "), ";
              cout << "(" << setw(11) << (int)(simInput[ii]       * 4096) << ", "                       << setw(11) << (int)(
                     simInput[ii + 1]      * 4096) << ", "                            << setw(11) << (int)(simInput[ii + 2]      * 4096) << "), ";
              cout << "(" << setw(11) << (int)(unpackedInput[ii] * 4096 - simInput[ii] * 4096)  << ", " << setw(11) << (int)(
                     unpackedInput[ii + 1] * 4096 - simInput[ii + 1] * 4096) << ", "  << setw(11) << (int)(unpackedInput[ii + 2] * 4096 - simInput[ii +
                         2] * 4096) << "), " << endl;
              cout.unsetf(ios::showbase);
              cout << dec;
              //std::cout << " (" << simInput[ii] / unpackedInput[ii] << std::endl << ", " << simInput[ii + 1] /  unpackedInput[ii + 1] << ", " <<
              //          simInput[ii + 2] / unpackedInput[ii + 2] << ")" << std::endl;
            }
            cout << endl;


            for (unsigned ii = 0; ii < unpackedInput.size(); ii += 3) {
              m_neuroDeltaInputID->Fill(unpackedInput[ii] - simInput[ii]);
              m_neuroDeltaInputT->Fill(unpackedInput[ii + 1] - simInput[ii + 1]);
              m_neuroDeltaInputAlpha->Fill(unpackedInput[ii + 2] - simInput[ii + 2]);
            }
          }
        }
      }
    }
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
  for (CDCTriggerSegmentHit& hit : m_unpackedSegmentHits) {
    nof2dinsegments++;
    m_2DInTSID->Fill(hit.getSegmentID());
    unsigned int sl = hit.getISuperLayer();
    switch (sl) {
      case 0: m_2DInTSPrioT_Layer0->Fill(hit.priorityTime());
        m_2DInTSFoundT_Layer0->Fill(hit.foundTime());
        break;
      case 2: m_2DInTSPrioT_Layer2->Fill(hit.priorityTime());
        m_2DInTSFoundT_Layer2->Fill(hit.foundTime());
        break;
      case 4: m_2DInTSPrioT_Layer4->Fill(hit.priorityTime());
        m_2DInTSFoundT_Layer4->Fill(hit.foundTime());
        break;
      case 6: m_2DInTSPrioT_Layer6->Fill(hit.priorityTime());
        m_2DInTSFoundT_Layer6->Fill(hit.foundTime());
        break;
      case 8: m_2DInTSPrioT_Layer8->Fill(hit.priorityTime());
        m_2DInTSFoundT_Layer8->Fill(hit.foundTime());
        break;
    }
  }
  if (nof2dinsegments > 0)
    m_2DInTSCount->Fill(nof2dinsegments);
}


void CDCTriggerDQMModule::endRun()
{
}


void CDCTriggerDQMModule::terminate()
{
}
