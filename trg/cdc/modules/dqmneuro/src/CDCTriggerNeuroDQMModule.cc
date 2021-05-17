/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sara Neuhaus, Sebastian Skambraks                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "trg/cdc/modules/dqmneuro/CDCTriggerNeuroDQMModule.h"

#include <framework/dataobjects/EventMetaData.h>

#include "TDirectory.h"
#include <tracking/dataobjects/RecoTrack.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCTriggerNeuroDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCTriggerNeuroDQMModule::CDCTriggerNeuroDQMModule() : HistoModule()
{
  //Set module properties
  setDescription("CDC Trigger DQM module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("limitedoutput", m_limitedoutput,
           "Switch to supress output for online dqm purposes. ",
           true);
  addParam("unpackedSegmentHitsName", m_unpackedSegmentHitsName,
           "The name of the StoreArray of the unpacked CDCTriggerSegmentHits",
           string("CDCTriggerSegmentHits"));
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
           string("TRGCDCTNN"));
  addParam("simSegmentHitsName", m_simSegmentHitsName,
           "StoreArray name for simulated TS hits", string(""));
  addParam("sim2DTracksSWTSName", m_sim2DTracksSWTSName,
           "StoreArray name for simulated 2D finder tracks using simulated TS", string(""));
  addParam("simNeuroTracksSWTSSW2DName", m_simNeuroTracksSWTSSW2DName,
           "StoreArray name for neuro tracks using simulated TS and simulated 2D", string(""));
  addParam("showRecoTracks", m_showRecoTracks,
           "switch to turn on a comparison with the reconstruction, DEPRECATED! Use RecoTracks='RecoTracks' (='') instead!",
           false);
  addParam("RecoTracks", m_recoTracksName,
           "Name of the RecoTrack StoreArray. Leave empty for skipping them.",
           string(""));
  addParam("recoTrackMultiplicity", m_recoTrackMultiplicity,
           "Select events with a specific RecoTrack track multiplicity. -1 for all events",
           -1);
  addParam("skipWithoutHWTS", m_skipWithoutHWTS,
           "Switch to skip events without unpacked TS",
           true);
  addParam("maxRecoZDist", m_maxRecoZDist,
           "Select only RecoTracks with a maximum z distance to the IP. -1.0 for all tracks",
           (double)(- 1.0));
  addParam("maxRecoD0Dist", m_maxRecoD0Dist,
           "Select only RecoTracks with a maximum d0 distance to the z axis. -1.0 for all tracks",
           (double)(- 1.0));
  addParam("nSameTS", m_nsamets,
           "Number of identical track segments to be required for matching between HW and SW Neurotrigger",
           (int)(9));
}


CDCTriggerNeuroDQMModule::~CDCTriggerNeuroDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void CDCTriggerNeuroDQMModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }
  //----------------------------------------------------------------

  // define neurotrigger histograms
  if (m_unpackedNeuroTracksName != "") {
    m_neuroHWOutZ = new TH1F("NeuroHWOutZ",
                             "z distribution of unpacked neuro tracks; z [cm]",
                             100, -100, 100);
    m_neuroHWOutCosTheta = new TH1F("NeuroHWOutCosTheta",
                                    "cos theta distribution of unpacked neuro tracks; cos(#theta) ",
                                    100, -1, 1);
    m_neuroHWOutPhi0 = new TH1F("NeuroHWOutPhi0",
                                "phi distribution of unpacked neuro tracks; #phi [#circ]",
                                80, 0, 360); // shift to reduce the binning error
    m_neuroHWOutInvPt = new TH1F("NeuroHWOutInvPt",
                                 "Inverse Pt distribution of unpacked neuro tracks; p_{T}^{-1} [GeV^{-1}]",
                                 50, 0, 5);
    m_neuroHWOutPt = new TH1F("NeuroHWOutPt",
                              "Pt distribution of unpacked neuro tracks; p_{T} [GeV]",
                              340, 0, 11);
    m_neuroHWOutHitPattern = new TH1F("NeuroUnpackedHitPattern",
                                      "stereo hit pattern of unpacked neuro tracks; pattern",
                                      16, 0, 16); // 4 stereo layers -> 2**4 possible patterns
    m_neuroHWOutm_time = new TH1F("NeuroHWOutM_time", "m_time distribution of unpacked neuro tracks; clock cycle",
                                  48, 0, 48);
    m_neuroHWOutTrackCount = new TH1F("NeuroHWOutTrackCount",
                                      "number of unpacked neuro tracks per event",
                                      20, 0, 20);
    m_neuroHWSector = new TH1F("NeuroHWSector",
                               "sector of unpacked neuro tracks; sector",
                               10, 0, 10);


    m_neuroHWInInvPt = new TH1F("NeuroHWInInvPt",
                                "Inverse Pt distribution from incoming 2dtrack; p_{T}^{-1} [GeV^{-1}]",
                                50, 0, 5);
    m_neuroHWInPhi0 = new TH1F("NeuroHWInPhi0", "Phi0 of incoming 2dtrack; #phi [#circ]",
                               80, 0, 360);
    m_neuroHWInm_time = new TH1F("NeuroHWInM_time", "m_time distribution from incoming 2dtracks; clock cycle",
                                 48, 0, 48);
    m_neuroHWInTrackCount = new TH1F("NeuroHWInTrackCount", "number of neuro input 2dtracks per event",
                                     20, 0, 20);
    m_neuroHWOutVsInTrackCount = new TH1F("NeuroHWOutVsInTrackCount",
                                          "number of neuroHWOutTracks - number of 2dinTracks",
                                          20, -10, 10);
    m_neuroHWInTSID = new TH1F("NeuroHWInTSID", "ID of incoming track segments",
                               2336, 0, 2335);
    m_neuroHWInTSCount = new TH1F("NeuroHWInTSCount", " number of TS per event",
                                  200, 0, 200);
    m_neuroHWSelTSID = new TH1F("NeuroHWSelTSID", "ID of selected track segments",
                                2336, 0, 2335);
    m_neuroHWSelTSCount = new TH1F("NeuroHWSelTSCount", "number of selected TS per SL; sl", 9, 0, 9);
  }
  if (!m_limitedoutput && m_unpacked2DTracksName != "") {
    m_2DHWInTSPrioT_Layer0 = new TH1F("2DHWInTSPrioT_Layer0", "Priority time of track segments in layer 0",
                                      512, 0, 511);
    m_2DHWInTSPrioT_Layer2 = new TH1F("2DHWInTSPrioT_Layer2", "Priority time of track segments in layer 2",
                                      512, 0, 511);
    m_2DHWInTSPrioT_Layer4 = new TH1F("2DHWInTSPrioT_Layer4", "Priority time of track segments in layer 4",
                                      512, 0, 511);
    m_2DHWInTSPrioT_Layer6 = new TH1F("2DHWInTSPrioT_Layer6", "Priority time of track segments in layer 6",
                                      512, 0, 511);
    m_2DHWInTSPrioT_Layer8 = new TH1F("2DHWInTSPrioT_Layer8", "Priority time of track segments in layer 8",
                                      512, 0, 511);
    m_2DHWInTSFoundT_Layer0 = new TH1F("2DHWInTSFoundT_Layer0", "Found time of track segments in layer 0",
                                       96, -48, 48);
    m_2DHWInTSFoundT_Layer2 = new TH1F("2DHWInTSFoundT_Layer2", "Found time of track segments in layer 2",
                                       96, -48, 48);
    m_2DHWInTSFoundT_Layer4 = new TH1F("2DHWInTSFoundT_Layer4", "Found time of track segments in layer 4",
                                       96, -48, 48);
    m_2DHWInTSFoundT_Layer6 = new TH1F("2DHWInTSFoundT_Layer6", "Found time of track segments in layer 6",
                                       96, -48, 48);
    m_2DHWInTSFoundT_Layer8 = new TH1F("2DHWInTSFoundT_Layer8", "Found time of track segments in layer 8",
                                       96, -48, 48);


    m_2DHWInTSPrioB_Layer0 = new TH1F("2DHWInTSPrioB_Layer0", "Priority bits of track segments in layer 0",
                                      4, 0, 4);
    m_2DHWInTSPrioB_Layer2 = new TH1F("2DHWInTSPrioB_Layer2", "Priority bits of track segments in layer 2",
                                      4, 0, 4);
    m_2DHWInTSPrioB_Layer4 = new TH1F("2DHWInTSPrioB_Layer4", "Priority bits of track segments in layer 4",
                                      4, 0, 4);
    m_2DHWInTSPrioB_Layer6 = new TH1F("2DHWInTSPrioB_Layer6", "Priority bits of track segments in layer 6",
                                      4, 0, 4);
    m_2DHWInTSPrioB_Layer8 = new TH1F("2DHWInTSPrioB_Layer8", "Priority bits of track segments in layer 8",
                                      4, 0, 4);
    m_2DHWInTSLR_Layer0 = new TH1F("2DHWInTSLR_Layer0", "Left/Right of track segments in layer 0",
                                   4, 0, 4);
    m_2DHWInTSLR_Layer2 = new TH1F("2DHWInTSLR_Layer2", "Left/Right of track segments in layer 2",
                                   4, 0, 4);
    m_2DHWInTSLR_Layer4 = new TH1F("2DHWInTSLR_Layer4", "Left/Right of track segments in layer 4",
                                   4, 0, 4);
    m_2DHWInTSLR_Layer6 = new TH1F("2DHWInTSLR_Layer6", "Left/Right of track segments in layer 6",
                                   4, 0, 4);
    m_2DHWInTSLR_Layer8 = new TH1F("2DHWInTSLR_Layer8", "Left/Right of track segments in layer 8",
                                   4, 0, 4);
  }
  if (!m_limitedoutput && m_unpackedNeuroTracksName != "") {

    m_neuroHWOutQuad5Z = new TH1F("NeuroHWOutQuad5Z",
                                  "z distribution of neuro tracks; z [cm]",
                                  100, -100, 100);
    m_neuroHWOutQuad5CosTheta = new TH1F("NeuroHWOutQuad5CosTheta",
                                         "cos theta distribution of neuro tracks; cos(#theta) ",
                                         100, -1, 1);
    m_neuroHWOutQuad5InvPt = new TH1F("NeuroHWOutQuad5InvPt",
                                      "Pt distribution from unpacker; p_{T}^{-1} [GeV^{-1}]",
                                      50, 0, 5);
    m_neuroHWOutQuad5Phi0 = new TH1F("NeuroHWOutQuad5Phi0",
                                     "phi distribution from unpacker; #phi [#circ]",
                                     80, 0, 360);

    m_neuroHWOutQuad0Z = new TH1F("NeuroHWOutQuad0Z",
                                  "z distribution of neuro tracks; z [cm]",
                                  100, -100, 100);
    m_neuroHWOutQuad0CosTheta = new TH1F("NeuroHWOutQuad0CosTheta",
                                         "cos theta distribution of neuro tracks; cos(#theta) ",
                                         100, -1, 1);
    m_neuroHWOutQuad0InvPt = new TH1F("NeuroHWOutQuad0InvPt",
                                      "Pt distribution from unpacker; p_{T}^{-1} [GeV^{-1}]",
                                      50, 0, 5);
    m_neuroHWOutQuad0Phi0 = new TH1F("NeuroHWOutQuad0Phi0",
                                     "phi distribution from unpacker; #phi [#circ]",
                                     80, 0, 360);

    m_neuroHWOutQuad1Z = new TH1F("NeuroHWOutQuad1Z",
                                  "z distribution of neuro tracks; z [cm]",
                                  100, -100, 100);
    m_neuroHWOutQuad1CosTheta = new TH1F("NeuroHWOutQuad1CosTheta",
                                         "cos theta distribution of neuro tracks; cos(#theta) ",
                                         100, -1, 1);
    m_neuroHWOutQuad1Phi0 = new TH1F("NeuroHWOutQuad1Phi0",
                                     "phi distribution from unpacker; #phi [#circ]",
                                     80, 0, 360);
    m_neuroHWOutQuad1InvPt = new TH1F("NeuroHWOutQuad1InvPt",
                                      "Pt distribution from unpacker; p_{T}^{-1} [GeV^{-1}]",
                                      50, 0, 5);

    m_neuroHWOutQuad2Z = new TH1F("NeuroHWOutQuad2Z",
                                  "z distribution of neuro tracks; z [cm]",
                                  100, -100, 100);
    m_neuroHWOutQuad2CosTheta = new TH1F("NeuroHWOutQuad2CosTheta",
                                         "cos theta distribution of neuro tracks; cos(#theta) ",
                                         100, -1, 1);
    m_neuroHWOutQuad2Phi0 = new TH1F("NeuroHWOutQuad2Phi0",
                                     "phi distribution from unpacker; #phi [#circ]",
                                     80, 0, 360);
    m_neuroHWOutQuad2InvPt = new TH1F("NeuroHWOutQuad2InvPt",
                                      "Pt distribution from unpacker; p_{T}^{-1} [GeV^{-1}]",
                                      50, 0, 5);

    m_neuroHWOutQuad3Z = new TH1F("NeuroHWOutQuad3Z",
                                  "z distribution of neuro tracks; z [cm]",
                                  100, -100, 100);
    m_neuroHWOutQuad3CosTheta = new TH1F("NeuroHWOutQuad3CosTheta",
                                         "cos theta distribution of neuro tracks; cos(#theta) ",
                                         100, -1, 1);
    m_neuroHWOutQuad3Phi0 = new TH1F("NeuroHWOutQuad3Phi0",
                                     "phi distribution from unpacker; #phi [#circ]",
                                     80, 0, 360);
    m_neuroHWOutQuad3InvPt = new TH1F("NeuroHWOutQuad3InvPt",
                                      "Pt distribution from unpacker; p_{T}^{-1} [GeV^{-1}]",
                                      50, 0, 5);
    m_neuroHWInTSPrioT_Layer0 = new TH1F("NeuroHWInTSPrioT_Layer0", "Priority time of track segments in layer 0",
                                         512, 0, 511);
    m_neuroHWInTSPrioT_Layer1 = new TH1F("NeuroHWInTSPrioT_Layer1", "Priority time of track segments in layer 1",
                                         512, 0, 511);
    m_neuroHWInTSPrioT_Layer2 = new TH1F("NeuroHWInTSPrioT_Layer2", "Priority time of track segments in layer 2",
                                         512, 0, 511);
    m_neuroHWInTSPrioT_Layer3 = new TH1F("NeuroHWInTSPrioT_Layer3", "Priority time of track segments in layer 3",
                                         512, 0, 511);
    m_neuroHWInTSPrioT_Layer4 = new TH1F("NeuroHWInTSPrioT_Layer4", "Priority time of track segments in layer 4",
                                         512, 0, 511);
    m_neuroHWInTSPrioT_Layer5 = new TH1F("NeuroHWInTSPrioT_Layer5", "Priority time of track segments in layer 5",
                                         512, 0, 511);
    m_neuroHWInTSPrioT_Layer6 = new TH1F("NeuroHWInTSPrioT_Layer6", "Priority time of track segments in layer 6",
                                         512, 0, 511);
    m_neuroHWInTSPrioT_Layer7 = new TH1F("NeuroHWInTSPrioT_Layer7", "Priority time of track segments in layer 7",
                                         512, 0, 511);
    m_neuroHWInTSPrioT_Layer8 = new TH1F("NeuroHWInTSPrioT_Layer8", "Priority time of track segments in layer 8",
                                         512, 0, 511);
    m_neuroHWInTSFoundT_Layer0 = new TH1F("NeuroHWInTSFoundT_Layer0", "Found time of track segments in layer 0",
                                          48, 0, 48);
    m_neuroHWInTSFoundT_Layer1 = new TH1F("NeuroHWInTSFoundT_Layer1", "Found time of track segments in layer 1",
                                          48, 0, 48);
    m_neuroHWInTSFoundT_Layer2 = new TH1F("NeuroHWInTSFoundT_Layer2", "Found time of track segments in layer 2",
                                          48, 0, 48);
    m_neuroHWInTSFoundT_Layer3 = new TH1F("NeuroHWInTSFoundT_Layer3", "Found time of track segments in layer 3",
                                          48, 0, 48);
    m_neuroHWInTSFoundT_Layer4 = new TH1F("NeuroHWInTSFoundT_Layer4", "Found time of track segments in layer 4",
                                          48, 0, 48);
    m_neuroHWInTSFoundT_Layer5 = new TH1F("NeuroHWInTSFoundT_Layer5", "Found time of track segments in layer 5",
                                          48, 0, 48);
    m_neuroHWInTSFoundT_Layer6 = new TH1F("NeuroHWInTSFoundT_Layer6", "Found time of track segments in layer 6",
                                          48, 0, 48);
    m_neuroHWInTSFoundT_Layer7 = new TH1F("NeuroHWInTSFoundT_Layer7", "Found time of track segments in layer 7",
                                          48, 0, 48);
    m_neuroHWInTSFoundT_Layer8 = new TH1F("NeuroHWInTSFoundT_Layer8", "Found time of track segments in layer 8",
                                          48, 0, 48);


    m_neuroHWInTSPrioB_Layer0 = new TH1F("NeuroHWInTSPrioB_Layer0", "Priority bits of track segments in layer 0",
                                         4, 0, 4);
    m_neuroHWInTSPrioB_Layer1 = new TH1F("NeuroHWInTSPrioB_Layer1", "Priority bits of track segments in layer 1",
                                         4, 0, 4);
    m_neuroHWInTSPrioB_Layer2 = new TH1F("NeuroHWInTSPrioB_Layer2", "Priority bits of track segments in layer 2",
                                         4, 0, 4);
    m_neuroHWInTSPrioB_Layer3 = new TH1F("NeuroHWInTSPrioB_Layer3", "Priority bits of track segments in layer 3",
                                         4, 0, 4);
    m_neuroHWInTSPrioB_Layer4 = new TH1F("NeuroHWInTSPrioB_Layer4", "Priority bits of track segments in layer 4",
                                         4, 0, 4);
    m_neuroHWInTSPrioB_Layer5 = new TH1F("NeuroHWInTSPrioB_Layer5", "Priority bits of track segments in layer 5",
                                         4, 0, 4);
    m_neuroHWInTSPrioB_Layer6 = new TH1F("NeuroHWInTSPrioB_Layer6", "Priority bits of track segments in layer 6",
                                         4, 0, 4);
    m_neuroHWInTSPrioB_Layer7 = new TH1F("NeuroHWInTSPrioB_Layer7", "Priority bits of track segments in layer 7",
                                         4, 0, 4);
    m_neuroHWInTSPrioB_Layer8 = new TH1F("NeuroHWInTSPrioB_Layer8", "Priority bits of track segments in layer 8",
                                         4, 0, 4);


    m_neuroHWInTSLR_Layer0 = new TH1F("NeuroHWInTSLR_Layer0", "Left/Right of track segments in layer 0",
                                      4, 0, 4);
    m_neuroHWInTSLR_Layer1 = new TH1F("NeuroHWInTSLR_Layer1", "Left/Right of track segments in layer 1",
                                      4, 0, 4);
    m_neuroHWInTSLR_Layer2 = new TH1F("NeuroHWInTSLR_Layer2", "Left/Right of track segments in layer 2",
                                      4, 0, 4);
    m_neuroHWInTSLR_Layer3 = new TH1F("NeuroHWInTSLR_Layer3", "Left/Right of track segments in layer 3",
                                      4, 0, 4);
    m_neuroHWInTSLR_Layer4 = new TH1F("NeuroHWInTSLR_Layer4", "Left/Right of track segments in layer 4",
                                      4, 0, 4);
    m_neuroHWInTSLR_Layer5 = new TH1F("NeuroHWInTSLR_Layer5", "Left/Right of track segments in layer 5",
                                      4, 0, 4);
    m_neuroHWInTSLR_Layer6 = new TH1F("NeuroHWInTSLR_Layer6", "Left/Right of track segments in layer 6",
                                      4, 0, 4);
    m_neuroHWInTSLR_Layer7 = new TH1F("NeuroHWInTSLR_Layer7", "Left/Right of track segments in layer 7",
                                      4, 0, 4);
    m_neuroHWInTSLR_Layer8 = new TH1F("NeuroHWInTSLR_Layer8", "Left/Right of track segments in layer 8",
                                      4, 0, 4);
    m_neuroHWSelTSPrioT_Layer0 = new TH1F("NeuroHWSelTSPrioT_Layer0", "Priority time of track segments in layer 0",
                                          512, 0, 511);
    m_neuroHWSelTSPrioT_Layer1 = new TH1F("NeuroHWSelTSPrioT_Layer1", "Priority time of track segments in layer 1",
                                          512, 0, 511);
    m_neuroHWSelTSPrioT_Layer2 = new TH1F("NeuroHWSelTSPrioT_Layer2", "Priority time of track segments in layer 2",
                                          512, 0, 511);
    m_neuroHWSelTSPrioT_Layer3 = new TH1F("NeuroHWSelTSPrioT_Layer3", "Priority time of track segments in layer 3",
                                          512, 0, 511);
    m_neuroHWSelTSPrioT_Layer4 = new TH1F("NeuroHWSelTSPrioT_Layer4", "Priority time of track segments in layer 4",
                                          512, 0, 511);
    m_neuroHWSelTSPrioT_Layer5 = new TH1F("NeuroHWSelTSPrioT_Layer5", "Priority time of track segments in layer 5",
                                          512, 0, 511);
    m_neuroHWSelTSPrioT_Layer6 = new TH1F("NeuroHWSelTSPrioT_Layer6", "Priority time of track segments in layer 6",
                                          512, 0, 511);
    m_neuroHWSelTSPrioT_Layer7 = new TH1F("NeuroHWSelTSPrioT_Layer7", "Priority time of track segments in layer 7",
                                          512, 0, 511);
    m_neuroHWSelTSPrioT_Layer8 = new TH1F("NeuroHWSelTSPrioT_Layer8", "Priority time of track segments in layer 8",
                                          512, 0, 511);
    m_neuroHWSelTSFoundT_Layer0 = new TH1F("NeuroHWSelTSFoundT_Layer0",
                                           "First found time of selected TS - found time of Neuro Track in SL 0",
                                           96, -47.99, 48.01);
    m_neuroHWSelTSFoundT_Layer1 = new TH1F("NeuroHWSelTSFoundT_Layer1",
                                           "First found time of selected TS - found time of Neuro Track in SL 1",
                                           96, -47.99, 48.01);
    m_neuroHWSelTSFoundT_Layer2 = new TH1F("NeuroHWSelTSFoundT_Layer2",
                                           "First found time of selected TS - found time of Neuro Track in SL 2",
                                           96, -47.99, 48.01);
    m_neuroHWSelTSFoundT_Layer3 = new TH1F("NeuroHWSelTSFoundT_Layer3",
                                           "First found time of selected TS - found time of Neuro Track in SL 3",
                                           96, -47.99, 48.01);
    m_neuroHWSelTSFoundT_Layer4 = new TH1F("NeuroHWSelTSFoundT_Layer4",
                                           "First found time of selected TS - found time of Neuro Track in SL 4",
                                           96, -47.99, 48.01);
    m_neuroHWSelTSFoundT_Layer5 = new TH1F("NeuroHWSelTSFoundT_Layer5",
                                           "First found time of selected TS - found time of Neuro Track in SL 5",
                                           96, -47.99, 48.01);
    m_neuroHWSelTSFoundT_Layer6 = new TH1F("NeuroHWSelTSFoundT_Layer6",
                                           "First found time of selected TS - found time of Neuro Track in SL 6",
                                           96, -47.99, 48.01);
    m_neuroHWSelTSFoundT_Layer7 = new TH1F("NeuroHWSelTSFoundT_Layer7",
                                           "First found time of selected TS - found time of Neuro Track in SL 7",
                                           96, -47.99, 48.01);
    m_neuroHWSelTSFoundT_Layer8 = new TH1F("NeuroHWSelTSFoundT_Layer8",
                                           "First found time of selected TS - found time of Neuro Track in SL 8",
                                           96, -47.99, 48.01);


    m_neuroHWSelTSPrioB_Layer0 = new TH1F("NeuroHWSelTSPrioB_Layer0", "Priority bits of track segments in layer 0",
                                          4, 0, 4);
    m_neuroHWSelTSPrioB_Layer1 = new TH1F("NeuroHWSelTSPrioB_Layer1", "Priority bits of track segments in layer 1",
                                          4, 0, 4);
    m_neuroHWSelTSPrioB_Layer2 = new TH1F("NeuroHWSelTSPrioB_Layer2", "Priority bits of track segments in layer 2",
                                          4, 0, 4);
    m_neuroHWSelTSPrioB_Layer3 = new TH1F("NeuroHWSelTSPrioB_Layer3", "Priority bits of track segments in layer 3",
                                          4, 0, 4);
    m_neuroHWSelTSPrioB_Layer4 = new TH1F("NeuroHWSelTSPrioB_Layer4", "Priority bits of track segments in layer 4",
                                          4, 0, 4);
    m_neuroHWSelTSPrioB_Layer5 = new TH1F("NeuroHWSelTSPrioB_Layer5", "Priority bits of track segments in layer 5",
                                          4, 0, 4);
    m_neuroHWSelTSPrioB_Layer6 = new TH1F("NeuroHWSelTSPrioB_Layer6", "Priority bits of track segments in layer 6",
                                          4, 0, 4);
    m_neuroHWSelTSPrioB_Layer7 = new TH1F("NeuroHWSelTSPrioB_Layer7", "Priority bits of track segments in layer 7",
                                          4, 0, 4);
    m_neuroHWSelTSPrioB_Layer8 = new TH1F("NeuroHWSelTSPrioB_Layer8", "Priority bits of track segments in layer 8",
                                          4, 0, 4);


    m_neuroHWSelTSLR_Layer0 = new TH1F("NeuroHWSelTSLR_Layer0", "Left/Right of track segments in layer 0",
                                       4, 0, 4);
    m_neuroHWSelTSLR_Layer1 = new TH1F("NeuroHWSelTSLR_Layer1", "Left/Right of track segments in layer 1",
                                       4, 0, 4);
    m_neuroHWSelTSLR_Layer2 = new TH1F("NeuroHWSelTSLR_Layer2", "Left/Right of track segments in layer 2",
                                       4, 0, 4);
    m_neuroHWSelTSLR_Layer3 = new TH1F("NeuroHWSelTSLR_Layer3", "Left/Right of track segments in layer 3",
                                       4, 0, 4);
    m_neuroHWSelTSLR_Layer4 = new TH1F("NeuroHWSelTSLR_Layer4", "Left/Right of track segments in layer 4",
                                       4, 0, 4);
    m_neuroHWSelTSLR_Layer5 = new TH1F("NeuroHWSelTSLR_Layer5", "Left/Right of track segments in layer 5",
                                       4, 0, 4);
    m_neuroHWSelTSLR_Layer6 = new TH1F("NeuroHWSelTSLR_Layer6", "Left/Right of track segments in layer 6",
                                       4, 0, 4);
    m_neuroHWSelTSLR_Layer7 = new TH1F("NeuroHWSelTSLR_Layer7", "Left/Right of track segments in layer 7",
                                       4, 0, 4);
    m_neuroHWSelTSLR_Layer8 = new TH1F("NeuroHWSelTSLR_Layer8", "Left/Right of track segments in layer 8",
                                       4, 0, 4);
    m_neuroHWInputID_Layer0 = new TH1F("NeuroHWInputID_Layer0",
                                       "unpacked id input in layer 0; id",
                                       100, -1, 1);
    m_neuroHWInputT_Layer0 = new TH1F("NeuroHWInputT_Layer0",
                                      "unpacked time input in layer 0; time",
                                      100, -1, 1);
    m_neuroHWInputAlpha_Layer0 = new TH1F("NeuroHWInputAlpha_Layer0",
                                          "unpacked alpha input in layer 0; alpha",
                                          100, -1, 1);
    m_neuroHWInputID_Layer1 = new TH1F("NeuroHWInputID_Layer1",
                                       "unpacked id input in layer 1; id",
                                       100, -1, 1);
    m_neuroHWInputT_Layer1 = new TH1F("NeuroHWInputT_Layer1",
                                      "unpacked time input in layer 1; time",
                                      100, -1, 1);
    m_neuroHWInputAlpha_Layer1 = new TH1F("NeuroHWInputAlpha_Layer1",
                                          "unpacked alpha input in layer 1; alpha",
                                          100, -1, 1);
    m_neuroHWInputID_Layer2 = new TH1F("NeuroHWInputID_Layer2",
                                       "unpacked id input in layer 2; id",
                                       100, -1, 1);
    m_neuroHWInputT_Layer2 = new TH1F("NeuroHWInputT_Layer2",
                                      "unpacked time input in layer 2; time",
                                      100, -1, 1);
    m_neuroHWInputAlpha_Layer2 = new TH1F("NeuroHWInputAlpha_Layer2",
                                          "unpacked alpha input in layer 2; alpha",
                                          100, -1, 1);
    m_neuroHWInputID_Layer3 = new TH1F("NeuroHWInputID_Layer3",
                                       "unpacked id input in layer 3; id",
                                       100, -1, 1);
    m_neuroHWInputT_Layer3 = new TH1F("NeuroHWInputT_Layer3",
                                      "unpacked time input in layer 3; time",
                                      100, -1, 1);
    m_neuroHWInputAlpha_Layer3 = new TH1F("NeuroHWInputAlpha_Layer3",
                                          "unpacked alpha input in layer 3; alpha",
                                          100, -1, 1);
    m_neuroHWInputID_Layer4 = new TH1F("NeuroHWInputID_Layer4",
                                       "unpacked id input in layer 4; id",
                                       100, -1, 1);
    m_neuroHWInputT_Layer4 = new TH1F("NeuroHWInputT_Layer4",
                                      "unpacked time input in layer 4; time",
                                      100, -1, 1);
    m_neuroHWInputAlpha_Layer4 = new TH1F("NeuroHWInputAlpha_Layer4",
                                          "unpacked alpha input in layer 4; alpha",
                                          100, -1, 1);
    m_neuroHWInputID_Layer5 = new TH1F("NeuroHWInputID_Layer5",
                                       "unpacked id input in layer 5; id",
                                       100, -1, 1);
    m_neuroHWInputT_Layer5 = new TH1F("NeuroHWInputT_Layer5",
                                      "unpacked time input in layer 5; time",
                                      100, -1, 1);
    m_neuroHWInputAlpha_Layer5 = new TH1F("NeuroHWInputAlpha_Layer5",
                                          "unpacked alpha input in layer 5; alpha",
                                          100, -1, 1);
    m_neuroHWInputID_Layer6 = new TH1F("NeuroHWInputID_Layer6",
                                       "unpacked id input in layer 6; id",
                                       100, -1, 1);
    m_neuroHWInputT_Layer6 = new TH1F("NeuroHWInputT_Layer6",
                                      "unpacked time input in layer 6; time",
                                      100, -1, 1);
    m_neuroHWInputAlpha_Layer6 = new TH1F("NeuroHWInputAlpha_Layer6",
                                          "unpacked alpha input in layer 6; alpha",
                                          100, -1, 1);
    m_neuroHWInputID_Layer7 = new TH1F("NeuroHWInputID_Layer7",
                                       "unpacked id input in layer 7; id",
                                       100, -1, 1);
    m_neuroHWInputT_Layer7 = new TH1F("NeuroHWInputT_Layer7",
                                      "unpacked time input in layer 7; time",
                                      100, -1, 1);
    m_neuroHWInputAlpha_Layer7 = new TH1F("NeuroHWInputAlpha_Layer7",
                                          "unpacked alpha input in layer 7; alpha",
                                          100, -1, 1);
    m_neuroHWInputID_Layer8 = new TH1F("NeuroHWInputID_Layer8",
                                       "unpacked id input in layer 8; id",
                                       100, -1, 1);
    m_neuroHWInputT_Layer8 = new TH1F("NeuroHWInputT_Layer8",
                                      "unpacked time input in layer 8; time",
                                      100, -1, 1);
    m_neuroHWInputAlpha_Layer8 = new TH1F("NeuroHWInputAlpha_Layer8",
                                          "unpacked alpha input in layer 8; alpha",
                                          100, -1, 1);
  }
  if (m_unpacked2DTracksName != "") {
    m_2DHWOutInvPt = new TH1F("2DHWOutInvPt",
                              "Inverse Pt of 2dtracks; p_{T}^{-1} [GeV^{-1}]",
                              50, 0, 5);
    m_2DHWOutPhi0 = new TH1F("2DHWOutPhi0", "Phi0 of 2dtracks; #phi [#circ]",
                             80, 0, 360);
    m_2DHWOutm_time = new TH1F("2DHWOutM_time", "m_time of 2dtracks; clock cycle",
                               96, -48, 48);
    m_2DHWOutTrackCount = new TH1F("2DHWOutTrackCount", "number of 2dtracks per event", 20, 0, 20);
    m_neuroHWInVs2DOutTrackCount = new TH1F("NeuroHWInVs2DOutTrackCount", "neuro in tracks - 2d out tracks",
                                            20, -10, 10);
  }

  if (!m_limitedoutput && m_simNeuroTracksName != "") {
    m_neuroSWOutZ = new TH1F("NeuroSWOutZ",
                             "z distribution from simulation, hw TS hw 2D; z [cm]",
                             100, -100, 100);
    m_neuroSWOutCosTheta = new TH1F("NeuroSWOutCosTheta",
                                    "cos theta distribution from simulation, hw TS hw 2D; cos(#theta) ",
                                    100, -1, 1);
    m_neuroSWOutInvPt = new TH1F("NeuroSWOutInvPt",
                                 "Inverse Pt distribution from simulation, hw TS hw 2D; p_{T}^{-1} [GeV^{-1}]",
                                 50, 0, 5);
    m_neuroSWOutPhi0 = new TH1F("NeuroSWOutPhi0",
                                "phi distribution from simulation, hw TS hw 2D; #phi [#circ]",
                                80, 0, 360); // shift to reduce the binning error
    m_neuroSWOutHitPattern = new TH1F("NeuroSWOutHitPattern",
                                      "stereo hit pattern of simulated neuro tracks, hw TS hw 2D; pattern",
                                      16, 0, 16); // 4 stereo layers -> 2**4 possible patterns
    m_neuroSWOutTrackCount = new TH1F("NeuroSWOutTrackCount",
                                      "number of neuro tracks per event from simulation, hw TS hw 2D",
                                      20, 0, 20);
    m_neuroSWSector = new TH1F("NeuroSWSector",
                               "sector of simulated neuro tracks, hw TS hw 2D; sector",
                               10, 0, 10);
    // hw TS selected by sw NN
    m_neuroSWSelTSID = new TH1F("NeuroSWSelTSID", "ID of selected track segments",
                                2336, 0, 2335);
    m_neuroSWSelTSCount = new TH1F("NeuroSWSelTSCount", "number of selected TS per SL; sl", 9, 0, 9);
    m_neuroSWSelTSPrioT_Layer0 = new TH1F("NeuroSWSelTSPrioT_Layer0", "Priority time of track segments in layer 0",
                                          512, 0, 511);
    m_neuroSWSelTSPrioT_Layer1 = new TH1F("NeuroSWSelTSPrioT_Layer1", "Priority time of track segments in layer 1",
                                          512, 0, 511);
    m_neuroSWSelTSPrioT_Layer2 = new TH1F("NeuroSWSelTSPrioT_Layer2", "Priority time of track segments in layer 2",
                                          512, 0, 511);
    m_neuroSWSelTSPrioT_Layer3 = new TH1F("NeuroSWSelTSPrioT_Layer3", "Priority time of track segments in layer 3",
                                          512, 0, 511);
    m_neuroSWSelTSPrioT_Layer4 = new TH1F("NeuroSWSelTSPrioT_Layer4", "Priority time of track segments in layer 4",
                                          512, 0, 511);
    m_neuroSWSelTSPrioT_Layer5 = new TH1F("NeuroSWSelTSPrioT_Layer5", "Priority time of track segments in layer 5",
                                          512, 0, 511);
    m_neuroSWSelTSPrioT_Layer6 = new TH1F("NeuroSWSelTSPrioT_Layer6", "Priority time of track segments in layer 6",
                                          512, 0, 511);
    m_neuroSWSelTSPrioT_Layer7 = new TH1F("NeuroSWSelTSPrioT_Layer7", "Priority time of track segments in layer 7",
                                          512, 0, 511);
    m_neuroSWSelTSPrioT_Layer8 = new TH1F("NeuroSWSelTSPrioT_Layer8", "Priority time of track segments in layer 8",
                                          512, 0, 511);
    m_neuroSWSelTSFoundT_Layer0 = new TH1F("NeuroSWSelTSFoundT_Layer0",
                                           "First found time of selected TS - found time of Neuro Track in SL 0",
                                           96, -47.99, 48.01);
    m_neuroSWSelTSFoundT_Layer1 = new TH1F("NeuroSWSelTSFoundT_Layer1",
                                           "First found time of selected TS - found time of Neuro Track in SL 1",
                                           96, -47.99, 48.01);
    m_neuroSWSelTSFoundT_Layer2 = new TH1F("NeuroSWSelTSFoundT_Layer2",
                                           "First found time of selected TS - found time of Neuro Track in SL 2",
                                           96, -47.99, 48.01);
    m_neuroSWSelTSFoundT_Layer3 = new TH1F("NeuroSWSelTSFoundT_Layer3",
                                           "First found time of selected TS - found time of Neuro Track in SL 3",
                                           96, -47.99, 48.01);
    m_neuroSWSelTSFoundT_Layer4 = new TH1F("NeuroSWSelTSFoundT_Layer4",
                                           "First found time of selected TS - found time of Neuro Track in SL 4",
                                           96, -47.99, 48.01);
    m_neuroSWSelTSFoundT_Layer5 = new TH1F("NeuroSWSelTSFoundT_Layer5",
                                           "First found time of selected TS - found time of Neuro Track in SL 5",
                                           96, -47.99, 48.01);
    m_neuroSWSelTSFoundT_Layer6 = new TH1F("NeuroSWSelTSFoundT_Layer6",
                                           "First found time of selected TS - found time of Neuro Track in SL 6",
                                           96, -47.99, 48.01);
    m_neuroSWSelTSFoundT_Layer7 = new TH1F("NeuroSWSelTSFoundT_Layer7",
                                           "First found time of selected TS - found time of Neuro Track in SL 7",
                                           96, -47.99, 48.01);
    m_neuroSWSelTSFoundT_Layer8 = new TH1F("NeuroSWSelTSFoundT_Layer8",
                                           "First found time of selected TS - found time of Neuro Track in SL 8",
                                           96, -47.99, 48.01);


    m_neuroSWSelTSPrioB_Layer0 = new TH1F("NeuroSWSelTSPrioB_Layer0", "Priority bits of track segments in layer 0",
                                          4, 0, 4);
    m_neuroSWSelTSPrioB_Layer1 = new TH1F("NeuroSWSelTSPrioB_Layer1", "Priority bits of track segments in layer 1",
                                          4, 0, 4);
    m_neuroSWSelTSPrioB_Layer2 = new TH1F("NeuroSWSelTSPrioB_Layer2", "Priority bits of track segments in layer 2",
                                          4, 0, 4);
    m_neuroSWSelTSPrioB_Layer3 = new TH1F("NeuroSWSelTSPrioB_Layer3", "Priority bits of track segments in layer 3",
                                          4, 0, 4);
    m_neuroSWSelTSPrioB_Layer4 = new TH1F("NeuroSWSelTSPrioB_Layer4", "Priority bits of track segments in layer 4",
                                          4, 0, 4);
    m_neuroSWSelTSPrioB_Layer5 = new TH1F("NeuroSWSelTSPrioB_Layer5", "Priority bits of track segments in layer 5",
                                          4, 0, 4);
    m_neuroSWSelTSPrioB_Layer6 = new TH1F("NeuroSWSelTSPrioB_Layer6", "Priority bits of track segments in layer 6",
                                          4, 0, 4);
    m_neuroSWSelTSPrioB_Layer7 = new TH1F("NeuroSWSelTSPrioB_Layer7", "Priority bits of track segments in layer 7",
                                          4, 0, 4);
    m_neuroSWSelTSPrioB_Layer8 = new TH1F("NeuroSWSelTSPrioB_Layer8", "Priority bits of track segments in layer 8",
                                          4, 0, 4);


    m_neuroSWSelTSLR_Layer0 = new TH1F("NeuroSWSelTSLR_Layer0", "Left/Right of track segments in layer 0",
                                       4, 0, 4);
    m_neuroSWSelTSLR_Layer1 = new TH1F("NeuroSWSelTSLR_Layer1", "Left/Right of track segments in layer 1",
                                       4, 0, 4);
    m_neuroSWSelTSLR_Layer2 = new TH1F("NeuroSWSelTSLR_Layer2", "Left/Right of track segments in layer 2",
                                       4, 0, 4);
    m_neuroSWSelTSLR_Layer3 = new TH1F("NeuroSWSelTSLR_Layer3", "Left/Right of track segments in layer 3",
                                       4, 0, 4);
    m_neuroSWSelTSLR_Layer4 = new TH1F("NeuroSWSelTSLR_Layer4", "Left/Right of track segments in layer 4",
                                       4, 0, 4);
    m_neuroSWSelTSLR_Layer5 = new TH1F("NeuroSWSelTSLR_Layer5", "Left/Right of track segments in layer 5",
                                       4, 0, 4);
    m_neuroSWSelTSLR_Layer6 = new TH1F("NeuroSWSelTSLR_Layer6", "Left/Right of track segments in layer 6",
                                       4, 0, 4);
    m_neuroSWSelTSLR_Layer7 = new TH1F("NeuroSWSelTSLR_Layer7", "Left/Right of track segments in layer 7",
                                       4, 0, 4);
    m_neuroSWSelTSLR_Layer8 = new TH1F("NeuroSWSelTSLR_Layer8", "Left/Right of track segments in layer 8",
                                       4, 0, 4);

    m_neuroSWInputID_Layer0 = new TH1F("NeuroSWInputID_Layer0",
                                       "simulated id input in layer 0; id",
                                       100, -1, 1);
    m_neuroSWInputT_Layer0 = new TH1F("NeuroSWInputT_Layer0",
                                      "simulated time input in layer 0; time",
                                      100, -1, 1);
    m_neuroSWInputAlpha_Layer0 = new TH1F("NeuroSWInputAlpha_Layer0",
                                          "simulated alpha input in layer 0; alpha",
                                          100, -1, 1);
    m_neuroSWInputID_Layer1 = new TH1F("NeuroSWInputID_Layer1",
                                       "simulated id input in layer 1; id",
                                       100, -1, 1);
    m_neuroSWInputT_Layer1 = new TH1F("NeuroSWInputT_Layer1",
                                      "simulated time input in layer 1; time",
                                      100, -1, 1);
    m_neuroSWInputAlpha_Layer1 = new TH1F("NeuroSWInputAlpha_Layer1",
                                          "simulated alpha input in layer 1; alpha",
                                          100, -1, 1);
    m_neuroSWInputID_Layer2 = new TH1F("NeuroSWInputID_Layer2",
                                       "simulated id input in layer 2; id",
                                       100, -1, 1);
    m_neuroSWInputT_Layer2 = new TH1F("NeuroSWInputT_Layer2",
                                      "simulated time input in layer 2; time",
                                      100, -1, 1);
    m_neuroSWInputAlpha_Layer2 = new TH1F("NeuroSWInputAlpha_Layer2",
                                          "simulated alpha input in layer 2; alpha",
                                          100, -1, 1);
    m_neuroSWInputID_Layer3 = new TH1F("NeuroSWInputID_Layer3",
                                       "simulated id input in layer 3; id",
                                       100, -1, 1);
    m_neuroSWInputT_Layer3 = new TH1F("NeuroSWInputT_Layer3",
                                      "simulated time input in layer 3; time",
                                      100, -1, 1);
    m_neuroSWInputAlpha_Layer3 = new TH1F("NeuroSWInputAlpha_Layer3",
                                          "simulated alpha input in layer 3; alpha",
                                          100, -1, 1);
    m_neuroSWInputID_Layer4 = new TH1F("NeuroSWInputID_Layer4",
                                       "simulated id input in layer 4; id",
                                       100, -1, 1);
    m_neuroSWInputT_Layer4 = new TH1F("NeuroSWInputT_Layer4",
                                      "simulated time input in layer 4; time",
                                      100, -1, 1);
    m_neuroSWInputAlpha_Layer4 = new TH1F("NeuroSWInputAlpha_Layer4",
                                          "simulated alpha input in layer 4; alpha",
                                          100, -1, 1);
    m_neuroSWInputID_Layer5 = new TH1F("NeuroSWInputID_Layer5",
                                       "simulated id input in layer 5; id",
                                       100, -1, 1);
    m_neuroSWInputT_Layer5 = new TH1F("NeuroSWInputT_Layer5",
                                      "simulated time input in layer 5; time",
                                      100, -1, 1);
    m_neuroSWInputAlpha_Layer5 = new TH1F("NeuroSWInputAlpha_Layer5",
                                          "simulated alpha input in layer 5; alpha",
                                          100, -1, 1);
    m_neuroSWInputID_Layer6 = new TH1F("NeuroSWInputID_Layer6",
                                       "simulated id input in layer 6; id",
                                       100, -1, 1);
    m_neuroSWInputT_Layer6 = new TH1F("NeuroSWInputT_Layer6",
                                      "simulated time input in layer 6; time",
                                      100, -1, 1);
    m_neuroSWInputAlpha_Layer6 = new TH1F("NeuroSWInputAlpha_Layer6",
                                          "simulated alpha input in layer 6; alpha",
                                          100, -1, 1);
    m_neuroSWInputID_Layer7 = new TH1F("NeuroSWInputID_Layer7",
                                       "simulated id input in layer 7; id",
                                       100, -1, 1);
    m_neuroSWInputT_Layer7 = new TH1F("NeuroSWInputT_Layer7",
                                      "simulated time input in layer 7; time",
                                      100, -1, 1);
    m_neuroSWInputAlpha_Layer7 = new TH1F("NeuroSWInputAlpha_Layer7",
                                          "simulated alpha input in layer 7; alpha",
                                          100, -1, 1);
    m_neuroSWInputID_Layer8 = new TH1F("NeuroSWInputID_Layer8",
                                       "simulated id input in layer 8; id",
                                       100, -1, 1);
    m_neuroSWInputT_Layer8 = new TH1F("NeuroSWInputT_Layer8",
                                      "simulated time input in layer 8; time",
                                      100, -1, 1);
    m_neuroSWInputAlpha_Layer8 = new TH1F("NeuroSWInputAlpha_Layer8",
                                          "simulated alpha input in layer 8; alpha",
                                          100, -1, 1);
  }

  if (!m_limitedoutput && m_sim2DTracksSWTSName != "") {

    m_2DSWOutInvPt = new TH1F("2DSWOutInvPt",
                              "Inverse Pt of 2dtracks from simulation, sw TS sw 2D; p_{T}^{-1} [GeV^{-1}]",
                              50, 0, 5);
    m_2DSWOutPhi0 = new TH1F("2DSWOutPhi0", "Phi0 of 2dtracks from simulation, sw TS sw 2D; #phi [#circ]",
                             80, 0, 360);
    m_2DSWOutm_time = new TH1F("2DSWOutM_time", "m_time of 2dtracks from simulation, sw TS sw 2D; clock cycle",
                               96, -48, 48);
    m_2DSWOutTrackCount = new TH1F("2DSWOutTrackCount", "number of 2dtracks per event from simulation, sw TS sw 2D", 20, 0, 20);

  }
  if (!m_limitedoutput && m_simNeuroTracksSWTSSW2DName != "") {

    m_neuroSWTSSW2DOutZ = new TH1F("NeuroSWTSSW2DOutZ",
                                   "z distribution from simulation, sw TS sw 2D; z [cm]",
                                   100, -100, 100);
    m_neuroSWTSSW2DOutCosTheta = new TH1F("NeuroSWTSSW2DOutCosTheta",
                                          "cos theta distribution from simulation, sw TS sw 2D; cos(#theta) ",
                                          100, -1, 1);
    m_neuroSWTSSW2DOutInvPt = new TH1F("NeuroSWTSSW2DOutInvPt",
                                       "Inverse Pt distribution from simulation, sw TS sw 2D; p_{T}^{-1} [GeV^{-1}]",
                                       50, 0, 5);
    m_neuroSWTSSW2DOutPhi0 = new TH1F("NeuroSWTSSW2DOutPhi0",
                                      "phi distribution from simulation, sw TS sw 2D; #phi [#circ]",
                                      80, 0, 360);
    m_neuroSWTSSW2DOutHitPattern = new TH1F("NeuroSWTSSW2DOutHitPattern",
                                            "stereo hit pattern of simulated neuro tracks, sw TS sw 2D; pattern",
                                            16, 0, 16); // 4 stereo layers -> 2**4 possible patterns
    m_neuroSWTSSW2DOutTrackCount = new TH1F("NeuroSWTSSW2DOutTrackCount",
                                            "number of simulated neuro tracks per event, sw TS sw 2D",
                                            20, 0, 20);
    m_neuroSWTSSW2DSector = new TH1F("NeuroSWTSSW2DSector",
                                     "sector of simulated neuro tracks, sw TS sw 2D; sector",
                                     10, 0, 10);
    // sw TS incoming
    m_neuroSWTSSW2DInTSID = new TH1F("NeuroSWTSSW2DInTSID", "ID of simulated track segments",
                                     2336, 0, 2335);
    m_neuroSWTSSW2DInTSCount = new TH1F("NeuroSWTSSW2DInTSCount", "number of simulated TS per event", 200, 0, 800);
    m_neuroSWTSSW2DInTSPrioT_Layer0 = new TH1F("NeuroSWTSSW2DInTSPrioT_Layer0", "Priority time of track segments in layer 0",
                                               512, 0, 511);
    m_neuroSWTSSW2DInTSPrioT_Layer1 = new TH1F("NeuroSWTSSW2DInTSPrioT_Layer1", "Priority time of track segments in layer 1",
                                               512, 0, 511);
    m_neuroSWTSSW2DInTSPrioT_Layer2 = new TH1F("NeuroSWTSSW2DInTSPrioT_Layer2", "Priority time of track segments in layer 2",
                                               512, 0, 511);
    m_neuroSWTSSW2DInTSPrioT_Layer3 = new TH1F("NeuroSWTSSW2DInTSPrioT_Layer3", "Priority time of track segments in layer 3",
                                               512, 0, 511);
    m_neuroSWTSSW2DInTSPrioT_Layer4 = new TH1F("NeuroSWTSSW2DInTSPrioT_Layer4", "Priority time of track segments in layer 4",
                                               512, 0, 511);
    m_neuroSWTSSW2DInTSPrioT_Layer5 = new TH1F("NeuroSWTSSW2DInTSPrioT_Layer5", "Priority time of track segments in layer 5",
                                               512, 0, 511);
    m_neuroSWTSSW2DInTSPrioT_Layer6 = new TH1F("NeuroSWTSSW2DInTSPrioT_Layer6", "Priority time of track segments in layer 6",
                                               512, 0, 511);
    m_neuroSWTSSW2DInTSPrioT_Layer7 = new TH1F("NeuroSWTSSW2DInTSPrioT_Layer7", "Priority time of track segments in layer 7",
                                               512, 0, 511);
    m_neuroSWTSSW2DInTSPrioT_Layer8 = new TH1F("NeuroSWTSSW2DInTSPrioT_Layer8", "Priority time of track segments in layer 8",
                                               512, 0, 511);
    m_neuroSWTSSW2DInTSFoundT_Layer0 = new TH1F("NeuroSWTSSW2DInTSFoundT_Layer0",
                                                "First found time of selected TS - found time of Neuro Track in SL 0",
                                                96, -47.99, 48.01);
    m_neuroSWTSSW2DInTSFoundT_Layer1 = new TH1F("NeuroSWTSSW2DInTSFoundT_Layer1",
                                                "First found time of selected TS - found time of Neuro Track in SL 1",
                                                96, -47.99, 48.01);
    m_neuroSWTSSW2DInTSFoundT_Layer2 = new TH1F("NeuroSWTSSW2DInTSFoundT_Layer2",
                                                "First found time of selected TS - found time of Neuro Track in SL 2",
                                                96, -47.99, 48.01);
    m_neuroSWTSSW2DInTSFoundT_Layer3 = new TH1F("NeuroSWTSSW2DInTSFoundT_Layer3",
                                                "First found time of selected TS - found time of Neuro Track in SL 3",
                                                96, -47.99, 48.01);
    m_neuroSWTSSW2DInTSFoundT_Layer4 = new TH1F("NeuroSWTSSW2DInTSFoundT_Layer4",
                                                "First found time of selected TS - found time of Neuro Track in SL 4",
                                                96, -47.99, 48.01);
    m_neuroSWTSSW2DInTSFoundT_Layer5 = new TH1F("NeuroSWTSSW2DInTSFoundT_Layer5",
                                                "First found time of selected TS - found time of Neuro Track in SL 5",
                                                96, -47.99, 48.01);
    m_neuroSWTSSW2DInTSFoundT_Layer6 = new TH1F("NeuroSWTSSW2DInTSFoundT_Layer6",
                                                "First found time of selected TS - found time of Neuro Track in SL 6",
                                                96, -47.99, 48.01);
    m_neuroSWTSSW2DInTSFoundT_Layer7 = new TH1F("NeuroSWTSSW2DInTSFoundT_Layer7",
                                                "First found time of selected TS - found time of Neuro Track in SL 7",
                                                96, -47.99, 48.01);
    m_neuroSWTSSW2DInTSFoundT_Layer8 = new TH1F("NeuroSWTSSW2DInTSFoundT_Layer8",
                                                "First found time of selected TS - found time of Neuro Track in SL 8",
                                                96, -47.99, 48.01);


    m_neuroSWTSSW2DInTSPrioB_Layer0 = new TH1F("NeuroSWTSSW2DInTSPrioB_Layer0", "Priority bits of track segments in layer 0",
                                               4, 0, 4);
    m_neuroSWTSSW2DInTSPrioB_Layer1 = new TH1F("NeuroSWTSSW2DInTSPrioB_Layer1", "Priority bits of track segments in layer 1",
                                               4, 0, 4);
    m_neuroSWTSSW2DInTSPrioB_Layer2 = new TH1F("NeuroSWTSSW2DInTSPrioB_Layer2", "Priority bits of track segments in layer 2",
                                               4, 0, 4);
    m_neuroSWTSSW2DInTSPrioB_Layer3 = new TH1F("NeuroSWTSSW2DInTSPrioB_Layer3", "Priority bits of track segments in layer 3",
                                               4, 0, 4);
    m_neuroSWTSSW2DInTSPrioB_Layer4 = new TH1F("NeuroSWTSSW2DInTSPrioB_Layer4", "Priority bits of track segments in layer 4",
                                               4, 0, 4);
    m_neuroSWTSSW2DInTSPrioB_Layer5 = new TH1F("NeuroSWTSSW2DInTSPrioB_Layer5", "Priority bits of track segments in layer 5",
                                               4, 0, 4);
    m_neuroSWTSSW2DInTSPrioB_Layer6 = new TH1F("NeuroSWTSSW2DInTSPrioB_Layer6", "Priority bits of track segments in layer 6",
                                               4, 0, 4);
    m_neuroSWTSSW2DInTSPrioB_Layer7 = new TH1F("NeuroSWTSSW2DInTSPrioB_Layer7", "Priority bits of track segments in layer 7",
                                               4, 0, 4);
    m_neuroSWTSSW2DInTSPrioB_Layer8 = new TH1F("NeuroSWTSSW2DInTSPrioB_Layer8", "Priority bits of track segments in layer 8",
                                               4, 0, 4);


    m_neuroSWTSSW2DInTSLR_Layer0 = new TH1F("NeuroSWTSSW2DInTSLR_Layer0", "Left/Right of track segments in layer 0",
                                            4, 0, 4);
    m_neuroSWTSSW2DInTSLR_Layer1 = new TH1F("NeuroSWTSSW2DInTSLR_Layer1", "Left/Right of track segments in layer 1",
                                            4, 0, 4);
    m_neuroSWTSSW2DInTSLR_Layer2 = new TH1F("NeuroSWTSSW2DInTSLR_Layer2", "Left/Right of track segments in layer 2",
                                            4, 0, 4);
    m_neuroSWTSSW2DInTSLR_Layer3 = new TH1F("NeuroSWTSSW2DInTSLR_Layer3", "Left/Right of track segments in layer 3",
                                            4, 0, 4);
    m_neuroSWTSSW2DInTSLR_Layer4 = new TH1F("NeuroSWTSSW2DInTSLR_Layer4", "Left/Right of track segments in layer 4",
                                            4, 0, 4);
    m_neuroSWTSSW2DInTSLR_Layer5 = new TH1F("NeuroSWTSSW2DInTSLR_Layer5", "Left/Right of track segments in layer 5",
                                            4, 0, 4);
    m_neuroSWTSSW2DInTSLR_Layer6 = new TH1F("NeuroSWTSSW2DInTSLR_Layer6", "Left/Right of track segments in layer 6",
                                            4, 0, 4);
    m_neuroSWTSSW2DInTSLR_Layer7 = new TH1F("NeuroSWTSSW2DInTSLR_Layer7", "Left/Right of track segments in layer 7",
                                            4, 0, 4);
    m_neuroSWTSSW2DInTSLR_Layer8 = new TH1F("NeuroSWTSSW2DInTSLR_Layer8", "Left/Right of track segments in layer 8",
                                            4, 0, 4);


    // sw TS selected
    m_neuroSWTSSW2DSelTSID = new TH1F("NeuroSWTSSW2DSelTSID", "ID of selected track segments",
                                      2336, 0, 2335);
    m_neuroSWTSSW2DSelTSCount = new TH1F("NeuroSWTSSW2DSelTSCount", "number of selected TS per SL; sl", 9, 0, 9);
    m_neuroSWTSSW2DSelTSPrioT_Layer0 = new TH1F("NeuroSWTSSW2DSelTSPrioT_Layer0", "Priority time of track segments in layer 0",
                                                512, 0, 511);
    m_neuroSWTSSW2DSelTSPrioT_Layer1 = new TH1F("NeuroSWTSSW2DSelTSPrioT_Layer1", "Priority time of track segments in layer 1",
                                                512, 0, 511);
    m_neuroSWTSSW2DSelTSPrioT_Layer2 = new TH1F("NeuroSWTSSW2DSelTSPrioT_Layer2", "Priority time of track segments in layer 2",
                                                512, 0, 511);
    m_neuroSWTSSW2DSelTSPrioT_Layer3 = new TH1F("NeuroSWTSSW2DSelTSPrioT_Layer3", "Priority time of track segments in layer 3",
                                                512, 0, 511);
    m_neuroSWTSSW2DSelTSPrioT_Layer4 = new TH1F("NeuroSWTSSW2DSelTSPrioT_Layer4", "Priority time of track segments in layer 4",
                                                512, 0, 511);
    m_neuroSWTSSW2DSelTSPrioT_Layer5 = new TH1F("NeuroSWTSSW2DSelTSPrioT_Layer5", "Priority time of track segments in layer 5",
                                                512, 0, 511);
    m_neuroSWTSSW2DSelTSPrioT_Layer6 = new TH1F("NeuroSWTSSW2DSelTSPrioT_Layer6", "Priority time of track segments in layer 6",
                                                512, 0, 511);
    m_neuroSWTSSW2DSelTSPrioT_Layer7 = new TH1F("NeuroSWTSSW2DSelTSPrioT_Layer7", "Priority time of track segments in layer 7",
                                                512, 0, 511);
    m_neuroSWTSSW2DSelTSPrioT_Layer8 = new TH1F("NeuroSWTSSW2DSelTSPrioT_Layer8", "Priority time of track segments in layer 8",
                                                512, 0, 511);
    m_neuroSWTSSW2DSelTSFoundT_Layer0 = new TH1F("NeuroSWTSSW2DSelTSFoundT_Layer0",
                                                 "First found time of selected TS - found time of Neuro Track in SL 0",
                                                 96, -47.99, 48.01);
    m_neuroSWTSSW2DSelTSFoundT_Layer1 = new TH1F("NeuroSWTSSW2DSelTSFoundT_Layer1",
                                                 "First found time of selected TS - found time of Neuro Track in SL 1",
                                                 96, -47.99, 48.01);
    m_neuroSWTSSW2DSelTSFoundT_Layer2 = new TH1F("NeuroSWTSSW2DSelTSFoundT_Layer2",
                                                 "First found time of selected TS - found time of Neuro Track in SL 2",
                                                 96, -47.99, 48.01);
    m_neuroSWTSSW2DSelTSFoundT_Layer3 = new TH1F("NeuroSWTSSW2DSelTSFoundT_Layer3",
                                                 "First found time of selected TS - found time of Neuro Track in SL 3",
                                                 96, -47.99, 48.01);
    m_neuroSWTSSW2DSelTSFoundT_Layer4 = new TH1F("NeuroSWTSSW2DSelTSFoundT_Layer4",
                                                 "First found time of selected TS - found time of Neuro Track in SL 4",
                                                 96, -47.99, 48.01);
    m_neuroSWTSSW2DSelTSFoundT_Layer5 = new TH1F("NeuroSWTSSW2DSelTSFoundT_Layer5",
                                                 "First found time of selected TS - found time of Neuro Track in SL 5",
                                                 96, -47.99, 48.01);
    m_neuroSWTSSW2DSelTSFoundT_Layer6 = new TH1F("NeuroSWTSSW2DSelTSFoundT_Layer6",
                                                 "First found time of selected TS - found time of Neuro Track in SL 6",
                                                 96, -47.99, 48.01);
    m_neuroSWTSSW2DSelTSFoundT_Layer7 = new TH1F("NeuroSWTSSW2DSelTSFoundT_Layer7",
                                                 "First found time of selected TS - found time of Neuro Track in SL 7",
                                                 96, -47.99, 48.01);
    m_neuroSWTSSW2DSelTSFoundT_Layer8 = new TH1F("NeuroSWTSSW2DSelTSFoundT_Layer8",
                                                 "First found time of selected TS - found time of Neuro Track in SL 8",
                                                 96, -47.99, 48.01);


    m_neuroSWTSSW2DSelTSPrioB_Layer0 = new TH1F("NeuroSWTSSW2DSelTSPrioB_Layer0", "Priority bits of track segments in layer 0",
                                                4, 0, 4);
    m_neuroSWTSSW2DSelTSPrioB_Layer1 = new TH1F("NeuroSWTSSW2DSelTSPrioB_Layer1", "Priority bits of track segments in layer 1",
                                                4, 0, 4);
    m_neuroSWTSSW2DSelTSPrioB_Layer2 = new TH1F("NeuroSWTSSW2DSelTSPrioB_Layer2", "Priority bits of track segments in layer 2",
                                                4, 0, 4);
    m_neuroSWTSSW2DSelTSPrioB_Layer3 = new TH1F("NeuroSWTSSW2DSelTSPrioB_Layer3", "Priority bits of track segments in layer 3",
                                                4, 0, 4);
    m_neuroSWTSSW2DSelTSPrioB_Layer4 = new TH1F("NeuroSWTSSW2DSelTSPrioB_Layer4", "Priority bits of track segments in layer 4",
                                                4, 0, 4);
    m_neuroSWTSSW2DSelTSPrioB_Layer5 = new TH1F("NeuroSWTSSW2DSelTSPrioB_Layer5", "Priority bits of track segments in layer 5",
                                                4, 0, 4);
    m_neuroSWTSSW2DSelTSPrioB_Layer6 = new TH1F("NeuroSWTSSW2DSelTSPrioB_Layer6", "Priority bits of track segments in layer 6",
                                                4, 0, 4);
    m_neuroSWTSSW2DSelTSPrioB_Layer7 = new TH1F("NeuroSWTSSW2DSelTSPrioB_Layer7", "Priority bits of track segments in layer 7",
                                                4, 0, 4);
    m_neuroSWTSSW2DSelTSPrioB_Layer8 = new TH1F("NeuroSWTSSW2DSelTSPrioB_Layer8", "Priority bits of track segments in layer 8",
                                                4, 0, 4);


    m_neuroSWTSSW2DSelTSLR_Layer0 = new TH1F("NeuroSWTSSW2DSelTSLR_Layer0", "Left/Right of track segments in layer 0",
                                             4, 0, 4);
    m_neuroSWTSSW2DSelTSLR_Layer1 = new TH1F("NeuroSWTSSW2DSelTSLR_Layer1", "Left/Right of track segments in layer 1",
                                             4, 0, 4);
    m_neuroSWTSSW2DSelTSLR_Layer2 = new TH1F("NeuroSWTSSW2DSelTSLR_Layer2", "Left/Right of track segments in layer 2",
                                             4, 0, 4);
    m_neuroSWTSSW2DSelTSLR_Layer3 = new TH1F("NeuroSWTSSW2DSelTSLR_Layer3", "Left/Right of track segments in layer 3",
                                             4, 0, 4);
    m_neuroSWTSSW2DSelTSLR_Layer4 = new TH1F("NeuroSWTSSW2DSelTSLR_Layer4", "Left/Right of track segments in layer 4",
                                             4, 0, 4);
    m_neuroSWTSSW2DSelTSLR_Layer5 = new TH1F("NeuroSWTSSW2DSelTSLR_Layer5", "Left/Right of track segments in layer 5",
                                             4, 0, 4);
    m_neuroSWTSSW2DSelTSLR_Layer6 = new TH1F("NeuroSWTSSW2DSelTSLR_Layer6", "Left/Right of track segments in layer 6",
                                             4, 0, 4);
    m_neuroSWTSSW2DSelTSLR_Layer7 = new TH1F("NeuroSWTSSW2DSelTSLR_Layer7", "Left/Right of track segments in layer 7",
                                             4, 0, 4);
    m_neuroSWTSSW2DSelTSLR_Layer8 = new TH1F("NeuroSWTSSW2DSelTSLR_Layer8", "Left/Right of track segments in layer 8",
                                             4, 0, 4);
    m_neuroSWTSSW2DInputID_Layer0 = new TH1F("NeuroSWTSSW2DInputID_Layer0",
                                             "simulated id input in layer 0; id",
                                             100, -1, 1);
    m_neuroSWTSSW2DInputT_Layer0 = new TH1F("NeuroSWTSSW2DInputT_Layer0",
                                            "simulated time input in layer 0; time",
                                            100, -1, 1);
    m_neuroSWTSSW2DInputAlpha_Layer0 = new TH1F("NeuroSWTSSW2DInputAlpha_Layer0",
                                                "simulated alpha input in layer 0; alpha",
                                                100, -1, 1);
    m_neuroSWTSSW2DInputID_Layer1 = new TH1F("NeuroSWTSSW2DInputID_Layer1",
                                             "simulated id input in layer 1; id",
                                             100, -1, 1);
    m_neuroSWTSSW2DInputT_Layer1 = new TH1F("NeuroSWTSSW2DInputT_Layer1",
                                            "simulated time input in layer 1; time",
                                            100, -1, 1);
    m_neuroSWTSSW2DInputAlpha_Layer1 = new TH1F("NeuroSWTSSW2DInputAlpha_Layer1",
                                                "simulated alpha input in layer 1; alpha",
                                                100, -1, 1);
    m_neuroSWTSSW2DInputID_Layer2 = new TH1F("NeuroSWTSSW2DInputID_Layer2",
                                             "simulated id input in layer 2; id",
                                             100, -1, 1);
    m_neuroSWTSSW2DInputT_Layer2 = new TH1F("NeuroSWTSSW2DInputT_Layer2",
                                            "simulated time input in layer 2; time",
                                            100, -1, 1);
    m_neuroSWTSSW2DInputAlpha_Layer2 = new TH1F("NeuroSWTSSW2DInputAlpha_Layer2",
                                                "simulated alpha input in layer 2; alpha",
                                                100, -1, 1);
    m_neuroSWTSSW2DInputID_Layer3 = new TH1F("NeuroSWTSSW2DInputID_Layer3",
                                             "simulated id input in layer 3; id",
                                             100, -1, 1);
    m_neuroSWTSSW2DInputT_Layer3 = new TH1F("NeuroSWTSSW2DInputT_Layer3",
                                            "simulated time input in layer 3; time",
                                            100, -1, 1);
    m_neuroSWTSSW2DInputAlpha_Layer3 = new TH1F("NeuroSWTSSW2DInputAlpha_Layer3",
                                                "simulated alpha input in layer 3; alpha",
                                                100, -1, 1);
    m_neuroSWTSSW2DInputID_Layer4 = new TH1F("NeuroSWTSSW2DInputID_Layer4",
                                             "simulated id input in layer 4; id",
                                             100, -1, 1);
    m_neuroSWTSSW2DInputT_Layer4 = new TH1F("NeuroSWTSSW2DInputT_Layer4",
                                            "simulated time input in layer 4; time",
                                            100, -1, 1);
    m_neuroSWTSSW2DInputAlpha_Layer4 = new TH1F("NeuroSWTSSW2DInputAlpha_Layer4",
                                                "simulated alpha input in layer 4; alpha",
                                                100, -1, 1);
    m_neuroSWTSSW2DInputID_Layer5 = new TH1F("NeuroSWTSSW2DInputID_Layer5",
                                             "simulated id input in layer 5; id",
                                             100, -1, 1);
    m_neuroSWTSSW2DInputT_Layer5 = new TH1F("NeuroSWTSSW2DInputT_Layer5",
                                            "simulated time input in layer 5; time",
                                            100, -1, 1);
    m_neuroSWTSSW2DInputAlpha_Layer5 = new TH1F("NeuroSWTSSW2DInputAlpha_Layer5",
                                                "simulated alpha input in layer 5; alpha",
                                                100, -1, 1);
    m_neuroSWTSSW2DInputID_Layer6 = new TH1F("NeuroSWTSSW2DInputID_Layer6",
                                             "simulated id input in layer 6; id",
                                             100, -1, 1);
    m_neuroSWTSSW2DInputT_Layer6 = new TH1F("NeuroSWTSSW2DInputT_Layer6",
                                            "simulated time input in layer 6; time",
                                            100, -1, 1);
    m_neuroSWTSSW2DInputAlpha_Layer6 = new TH1F("NeuroSWTSSW2DInputAlpha_Layer6",
                                                "simulated alpha input in layer 6; alpha",
                                                100, -1, 1);
    m_neuroSWTSSW2DInputID_Layer7 = new TH1F("NeuroSWTSSW2DInputID_Layer7",
                                             "simulated id input in layer 7; id",
                                             100, -1, 1);
    m_neuroSWTSSW2DInputT_Layer7 = new TH1F("NeuroSWTSSW2DInputT_Layer7",
                                            "simulated time input in layer 7; time",
                                            100, -1, 1);
    m_neuroSWTSSW2DInputAlpha_Layer7 = new TH1F("NeuroSWTSSW2DInputAlpha_Layer7",
                                                "simulated alpha input in layer 7; alpha",
                                                100, -1, 1);
    m_neuroSWTSSW2DInputID_Layer8 = new TH1F("NeuroSWTSSW2DInputID_Layer8",
                                             "simulated id input in layer 8; id",
                                             100, -1, 1);
    m_neuroSWTSSW2DInputT_Layer8 = new TH1F("NeuroSWTSSW2DInputT_Layer8",
                                            "simulated time input in layer 8; time",
                                            100, -1, 1);
    m_neuroSWTSSW2DInputAlpha_Layer8 = new TH1F("NeuroSWTSSW2DInputAlpha_Layer8",
                                                "simulated alpha input in layer 8; alpha",
                                                100, -1, 1);
  }



  if (!m_limitedoutput && m_unpackedNeuroTracksName != "" && m_simNeuroTracksName != "") {
    m_neuroDeltaZ = new TH1F("NeuroDeltaZ",
                             "difference between unpacked and simulated neuro z; #Delta z [cm]",
                             100, -100, 100); // should be bit-precise, so look at very small range
    m_neuroDeltaTheta = new TH1F("NeuroDeltaTheta",
                                 "difference between unpacked and simulated neuro theta; #Delta #theta [#circ]",
                                 100, -180, 180); // should be bit-precise, so look at very small range
    m_neuroScatterZ = new TH2F("NeuroScatterZ",
                               "unpacked z vs TSIM; hw z [cm]; sw z [cm]",
                               100, -150, 150, 100, -150, 150);
    m_neuroScatterTheta = new TH2F("NeuroScatterTheta",
                                   "unpacked theta vs TSIM; hw #theta [#circ]; sw #theta [#circ]",
                                   100, 0, 270, 100, 0, 270);

    m_neuroDeltaInputID = new TH1F("NeuroDeltaInputID",
                                   "difference between unpacked and simulated ID input; #Delta ID",
                                   100, -0.5, 0.5); // should be bit-precise, so look at very small range
    m_neuroDeltaInputT = new TH1F("NeuroDeltaInputT",
                                  "difference between unpacked and simulated time input; #Delta t",
                                  100, -0.5, 0.5); // should be bit-precise, so look at very small range
    m_neuroDeltaInputAlpha = new TH1F("NeuroDeltaInputAlpha",
                                      "difference between unpacked and simulated alpha input; #Delta alpha",
                                      100, -0.1, 0.1); // should be bit-precise, so look at very small range
    m_neuroDeltaTSID = new TH1F("NeuroDeltaTSID",
                                "difference between unpacked and simulated tsid; #Delta TSID",
                                100, -50, 50);
    m_neuroDeltaSector = new TH1F("NeuroDeltaSector",
                                  "difference between unpacked and simulated sector; #Delta sector",
                                  20, -10, 10);
    m_simSameTS = new TH1F("NeuroSimSameTS",
                           "number of TS selected in both, unpacked and TSIM tracks",
                           20, 0, 20);
    m_simDiffTS = new TH1F("NeuroSimDiffTS",
                           "number of TS selcted in TSIM but not in unpacker",
                           20, 0, 20);
  }

  if (m_recoTracksName != "") {
    //RecoTracks
    m_RecoZ = new TH1F("RecoZ",
                       "z distribution of reconstructed tracks;z [cm]",
                       100, -150, 150);
    m_RecoCosTheta = new TH1F("RecoCosTheta",
                              "cos theta distribution of reconstructed tracks; cos(#theta) ",
                              100, -1, 1);
    m_RecoInvPt = new TH1F("RecoInvPt",
                           "Pt distribution of reconstructed tracks; p_{T}^{-1} [GeV^{-1}]",
                           50, 0, 5);
    m_RecoPhi = new TH1F("RecoPhi",
                         "phi distribution of reconstructed tracks ; #phi [#circ]",
                         80, 0, 360);
    m_RecoD0 = new TH1F("RecoD0",
                        "d0 distribution of reconstructed tracks ; d_{0} [cm]",
                        100, 0, 10);
    m_RecoTrackCount = new TH1F("RecoTrackCount",
                                "number of reconstructed tracks per event",
                                20, 0, 20);
  }
  if (m_recoTracksName != "" && m_unpackedNeuroTracksName != "") {
    //RecoTracks matched to unpacked neuro tracks
    m_RecoHWZ = new TH1F("RecoHWZ",
                         "hw matched z distribution of reconstructed tracks; z [cm]",
                         100, -150, 150);
    m_RecoHWCosTheta = new TH1F("RecoHWCosTheta",
                                "hw matched cos theta distribution of reconstructed tracks; cos(#theta) ",
                                100, -1, 1);
    m_RecoHWInvPt = new TH1F("RecoHWInvPt",
                             "hw matched Pt distribution of reconstructed tracks; p_{T}^{-1} [GeV^{-1}]",
                             50, 0, 5);
    m_RecoHWPhi = new TH1F("RecoHWPhi",
                           "hw matched phi distribution of reconstructed tracks; #phi [#circ]",
                           80, 0, 360);
    m_RecoHWD0 = new TH1F("RecoHWD0",
                          "hw matched d0 distribution of reconstructed tracks ; d_{0} [cm]",
                          100, 0, 10);
    // hw neuro values for tracks matched to reco tracks
    m_neuroRecoHWOutZ = new TH1F("NeuroRecoHWOutZ",
                                 "reco matched z distribution of unpacked neuro tracks; z [cm]",
                                 100, -100, 100);
    m_neuroRecoHWOutCosTheta = new TH1F("NeuroRecoHWOutCosTheta",
                                        "reco matched cos theta distribution of unpacked neuro tracks; cos(#theta) ",
                                        100, -1, 1);
    m_neuroRecoHWOutInvPt = new TH1F("NeuroRecoHWOutInvPt",
                                     "reco matched Pt distribution of unpacked neuro tracks; p_{T}^{-1} [GeV^{-1}]",
                                     50, 0, 5);
    m_neuroRecoHWOutPhi0 = new TH1F("NeuroRecoHWOutPhi0",
                                    "reco matched phi distribution of unpacked neuro tracks; #phi [#circ]",
                                    80, 0, 360); // shift to reduce the binning error
    m_neuroRecoHWOutHitPattern = new TH1F("NeuroRecoUnpackedHitPattern",
                                          "reco matched stereo hit pattern of unpacked neuro tracks; pattern",
                                          16, 0, 16); // 4 stereo layers -> 2**4 possible patterns
    m_neuroRecoHWOutTrackCount = new TH1F("NeuroRecoHWOutTrackCount",
                                          "reco matched number of unpacked neuro tracks per event",
                                          20, 0, 20);
    m_neuroRecoHWSector = new TH1F("NeuroRecoHWSector",
                                   "reco matched sector of unpacked neuro tracks; sector",
                                   10, 0, 10);


    m_DeltaRecoHWZ = new TH1F("DeltaRecoHWZ",
                              "difference between reconstructed and unpacked neuro z; #Delta z [cm]",
                              100, -100, 100);
    m_DeltaRecoHWCosTheta = new TH1F("DeltaRecoHWCosTheta",
                                     "difference between reconstructed and unpacked neuro cos(theta); #Delta cos(#theta)",
                                     100, -1, 1);
    m_DeltaRecoHWInvPt = new TH1F("DeltaRecoHWInvPt",
                                  "difference between reconstructed and unpacked neuro Pt; #Delta p_{T}^{-1} [GeV^{-1}]",
                                  100, -10, 10);
    m_DeltaRecoHWPhi = new TH1F("DeltaRecoHWPhi",
                                "difference between reconstructed and unpacked neuro phi; #Delta #phi [#circ]",
                                80, -180, 180);
  }
  if (!m_limitedoutput && m_recoTracksName != "" && m_unpackedNeuroTracksName != "") {
    m_RecoHWZScatter = new TH2F("RecoHWZScatter",
                                "hw matched reconstruction; reco z [cm]; hw z [cm]",
                                100, -150, 150, 100, -150, 150);
  }



  if (!m_limitedoutput && m_simNeuroTracksName != "") {
    //RecoTracks matched to simulated neuro tracks (hw TS hw 2D sw NN)
    m_RecoSWZ = new TH1F("RecoSWZ",
                         "sw matched z distribution of reconstructed tracks; z [cm]",
                         100, -150, 150); // 1cm bins from -50cm to 50cm
    m_RecoSWCosTheta = new TH1F("RecoSWCosTheta",
                                "sw matched cos theta distribution of reconstructed tracks; cos(#theta) ",
                                100, -1, 1);
    m_RecoSWInvPt = new TH1F("RecoSWInvPt",
                             "sw matched Pt distribution of reconstructed tracks; p_{T}^{-1} [GeV^{-1}]",
                             50, 0, 5);
    m_RecoSWPhi = new TH1F("RecoSWPhi",
                           "sw matched phi distribution of reconstructed tracks ; #phi [#circ]",
                           80, 0, 360);
    m_RecoSWD0 = new TH1F("RecoSWD0",
                          "sw matched d0 distribution of reconstructed tracks ; d_{0} [cm]",
                          100, 0, 10);
    m_RecoSWZScatter = new TH2F("RecoSWZScatter",
                                "sw matched reconstruction; reco z [cm]; sw z [cm]",
                                100, -150, 150, 100, -150, 150);


    // sw neuro values for tracks matched to reco tracks
    m_neuroRecoSWOutZ = new TH1F("NeuroRecoSWOutZ",
                                 "reco matched z distribution from simulation; z [cm]",
                                 100, -100, 100);
    m_neuroRecoSWOutCosTheta = new TH1F("NeuroRecoSWOutCosTheta",
                                        "reco matched cos theta distribution from simulation; cos(#theta) ",
                                        100, -1, 1);
    m_neuroRecoSWOutInvPt = new TH1F("NeuroRecoSWOutInvPt",
                                     "reco matched Pt distribution from simulation; p_{T}^{-1} [GeV^{-1}]",
                                     50, 0, 5);
    m_neuroRecoSWOutPhi0 = new TH1F("NeuroRecoSWOutPhi0",
                                    "reco matched phi distribution from simulation; #phi [#circ]",
                                    80, 0, 360); // shift to reduce the binning error
    m_neuroRecoSWOutHitPattern = new TH1F("NeuroRecoSWHitPattern",
                                          "reco matched stereo hit pattern from simulation; pattern",
                                          16, 0, 16); // 4 stereo layers -> 2**4 possible patterns
    m_neuroRecoSWOutTrackCount = new TH1F("NeuroRecoSWOutTrackCount",
                                          "reco matched number of SW neuro tracks per event",
                                          20, 0, 20);
    m_neuroRecoSWSector = new TH1F("NeuroRecoSWSector",
                                   "reco matched sector from simulation; sector",
                                   10, 0, 10);


    m_DeltaRecoSWZ = new TH1F("DeltaRecoSWZ",
                              "difference between reconstructed and simulated neuro z; #Delta z [cm]",
                              100, -100, 100);
    m_DeltaRecoSWCosTheta = new TH1F("DeltaRecoSWCosTheta",
                                     "difference between reconstructed and simulated neuro cos(theta); #Delta cos(#theta)",
                                     100, -1, 1);
    m_DeltaRecoSWInvPt = new TH1F("DeltaRecoSWInvPt",
                                  "difference between reconstructed and simulated neuro Pt; #Delta p_{T}^{-1} [GeV^{-1}]",
                                  100, -10, 10);
    m_DeltaRecoSWPhi = new TH1F("DeltaRecoSWPhi",
                                "difference between reconstructed and simulated neuro phi; #Delta #phi [#circ]",
                                80, -180, 180);
  }


  if (!m_limitedoutput && m_simNeuroTracksSWTSSW2DName != "") {
    //RecoTracks matched to simulated neuro tracks (sw TS sw 2D sw NN)
    m_RecoSWTSSW2DZ = new TH1F("RecoSWTSSW2DZ",
                               "sw matched z distribution of reconstructed tracks; z [cm]",
                               100, -150, 150); // 1cm bins from -50cm to 50cm
    m_RecoSWTSSW2DCosTheta = new TH1F("RecoSWTSSW2DCosTheta",
                                      "sw matched cos theta distribution of reconstructed tracks; cos(#theta) ",
                                      100, -1, 1);
    m_RecoSWTSSW2DInvPt = new TH1F("RecoSWTSSW2DInvPt",
                                   "sw matched Pt distribution of reconstructed tracks; p_{T}^{-1} [GeV^{-1}]",
                                   50, 0, 5);
    m_RecoSWTSSW2DPhi = new TH1F("RecoSWTSSW2DPhi",
                                 "sw matched phi distribution of reconstructed tracks ; #phi [#circ]",
                                 80, 0, 360);
    m_RecoSWTSSW2DD0 = new TH1F("RecoSWTSSW2DD0",
                                "sw matched d0 distribution of reconstructed tracks ; d_{0} [cm]",
                                100, 0, 10);
    m_RecoSWTSSW2DZScatter = new TH2F("RecoSWTSSW2DZScatter",
                                      "sw matched reconstruction; reco z [cm]; sw z [cm]",
                                      100, -150, 150, 100, -150, 150);


    // sw neuro values for tracks matched to reco tracks (sw TS, sw 2D)
    m_neuroRecoSWTSSW2DOutZ = new TH1F("NeuroRecoSWTSSW2DOutZ",
                                       "reco matched z distribution from simulation; z [cm]",
                                       100, -100, 100);
    m_neuroRecoSWTSSW2DOutCosTheta = new TH1F("NeuroRecoSWTSSW2DOutCosTheta",
                                              "reco matched cos theta distribution from simulation; cos(#theta) ",
                                              100, -1, 1);
    m_neuroRecoSWTSSW2DOutInvPt = new TH1F("NeuroRecoSWTSSW2DOutInvPt",
                                           "reco matched Pt distribution from simulation; p_{T}^{-1} [GeV^{-1}]",
                                           50, 0, 5);
    m_neuroRecoSWTSSW2DOutPhi0 = new TH1F("NeuroRecoSWTSSW2DOutPhi0",
                                          "reco matched phi distribution from simulation; #phi [#circ]",
                                          80, 0, 360); // shift to reduce the binning error
    m_neuroRecoSWTSSW2DOutHitPattern = new TH1F("NeuroRecoSWTSSW2DHitPattern",
                                                "reco matched stereo hit pattern from simulation; pattern",
                                                16, 0, 16); // 4 stereo layers -> 2**4 possible patterns
    m_neuroRecoSWTSSW2DOutTrackCount = new TH1F("NeuroRecoSWTSSW2DOutTrackCount",
                                                "reco matched number of SW neuro tracks per event",
                                                20, 0, 20);
    m_neuroRecoSWTSSW2DSector = new TH1F("NeuroRecoSWTSSW2DSector",
                                         "reco matched sector from simulation; sector",
                                         10, 0, 10);


    m_DeltaRecoSWTSSW2DZ = new TH1F("DeltaRecoSWTSSW2DZ",
                                    "difference between reconstructed and simulated neuro z; #Delta z [cm]",
                                    100, -100, 100);
    m_DeltaRecoSWTSSW2DCosTheta = new TH1F("DeltaRecoSWTSSW2DCosTheta",
                                           "difference between reconstructed and simulated neuro cos(theta); #Delta cos(#theta)",
                                           100, -1, 1);
    m_DeltaRecoSWTSSW2DInvPt = new TH1F("DeltaRecoSWTSSW2DInvPt",
                                        "difference between reconstructed and simulated neuro Pt; #Delta p_{T}^{-1} [GeV^{-1}]",
                                        100, -10, 10);
    m_DeltaRecoSWTSSW2DPhi = new TH1F("DeltaRecoSWTSSW2DPhi",
                                      "difference between reconstructed and simulated neuro phi;#Delta #phi [#circ]",
                                      80, -180, 180);
  }

  // cd back to root directory
  oldDir->cd();
}


void CDCTriggerNeuroDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
  if (m_showRecoTracks && m_recoTracksName == "") {
    m_recoTracksName = "RecoTracks";
  }
  if (m_unpackedSegmentHitsName != "") {
    m_unpackedSegmentHits.isRequired(m_unpackedSegmentHitsName);
  }
  if (m_unpacked2DTracksName != "") {
    m_unpacked2DTracks.isRequired(m_unpacked2DTracksName);
  }
  if (m_unpackedNeuroTracksName != "") {
    m_unpackedNeuroTracks.isRequired(m_unpackedNeuroTracksName);
    m_unpackedNeuroInput2DTracks.isRequired(m_unpackedNeuroInput2DTracksName);
    m_unpackedNeuroInputSegments.isRequired(m_unpackedNeuroInputSegmentsName);
    m_unpackedNeuroTracks.requireRelationTo(m_unpackedNeuroInputSegments);
    m_unpackedNeuroInput2DTracks.requireRelationTo(m_unpackedNeuroTracks);
    m_unpackedNeuroInputVectorName = m_unpackedNeuroTracksName + "Input";
    m_unpackedNeuroInputVector.isRequired(m_unpackedNeuroInputVectorName);
    m_unpackedNeuroTracks.requireRelationTo(m_unpackedNeuroInputVector);
  }

  if (!m_limitedoutput && m_simNeuroTracksName != "") {
    m_simNeuroInputVectorName = m_simNeuroTracksName + "Input";
    m_simNeuroTracks.isRequired(m_simNeuroTracksName);
    m_simNeuroInputVector.isRequired(m_simNeuroInputVectorName);
    m_unpackedNeuroInput2DTracks.requireRelationTo(m_simNeuroTracks);
    m_simNeuroTracks.requireRelationTo(m_simNeuroInputVector);
    m_simNeuroTracks.requireRelationTo(m_unpackedNeuroInputSegments);
  }
  if (!m_limitedoutput && m_simNeuroTracksSWTSSW2DName != "") {
    m_simSegmentHits.isRequired(m_simSegmentHitsName);
    m_sim2DTracksSWTS.isRequired(m_sim2DTracksSWTSName);
    m_simNeuroInputVectorSWTSSW2DName = m_simNeuroTracksSWTSSW2DName + "Input";
    m_simNeuroTracksSWTSSW2D.isRequired(m_simNeuroTracksSWTSSW2DName);
    m_simNeuroInputVectorSWTSSW2D.isRequired(m_simNeuroInputVectorSWTSSW2DName);
    m_simNeuroTracksSWTSSW2D.requireRelationTo(m_simNeuroInputVectorSWTSSW2D);
    m_simNeuroTracksSWTSSW2D.requireRelationTo(m_simSegmentHits);
    m_sim2DTracksSWTS.requireRelationTo(m_simNeuroTracksSWTSSW2D);
  }
  if (m_recoTracksName != "") {
    m_RecoTracks.isRequired("RecoTracks");
    m_RecoTracks.requireRelationTo(m_unpackedNeuroTracks);
  }
  if (!m_limitedoutput && m_recoTracksName != "" && m_simNeuroTracksName != "") {
    m_RecoTracks.requireRelationTo(m_simNeuroTracks);
  }
  if (!m_limitedoutput && m_recoTracksName != "" && m_simNeuroTracksSWTSSW2DName != "") {
    m_RecoTracks.requireRelationTo(m_simNeuroTracksSWTSSW2D);
  }
}

void CDCTriggerNeuroDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  if (m_unpackedNeuroTracksName != "") {

    m_neuroHWOutZ->Reset();
    m_neuroHWOutCosTheta->Reset();
    m_neuroHWOutInvPt->Reset();
    m_neuroHWOutPt->Reset();
    m_neuroHWOutPhi0->Reset();
    m_neuroHWOutHitPattern->Reset();
    m_neuroHWOutm_time->Reset();
    m_neuroHWOutTrackCount->Reset();
    m_neuroHWOutVsInTrackCount->Reset();
    m_neuroHWSector->Reset();

    m_neuroHWInInvPt->Reset();
    m_neuroHWInPhi0->Reset();
    m_neuroHWInm_time->Reset();
    m_neuroHWInTrackCount->Reset();

    m_neuroHWInTSID->Reset();
    m_neuroHWInTSCount->Reset();

    m_neuroHWSelTSID->Reset();
    m_neuroHWSelTSCount->Reset();
  }
  if (m_unpacked2DTracksName != "") {
    m_2DHWOutInvPt->Reset();
    m_2DHWOutPhi0->Reset();
    m_2DHWOutm_time->Reset();
    m_2DHWOutTrackCount->Reset();

  }
  if (m_unpacked2DTracksName != "" && m_unpackedNeuroTracksName != "") {
    m_neuroHWInVs2DOutTrackCount->Reset();
  }
  if (!m_limitedoutput && m_unpackedNeuroTracksName != "") {
    m_neuroHWOutQuad5Z->Reset();
    m_neuroHWOutQuad5CosTheta->Reset();
    m_neuroHWOutQuad5InvPt->Reset();
    m_neuroHWOutQuad5Phi0->Reset();
    m_neuroHWOutQuad0Z->Reset();
    m_neuroHWOutQuad0CosTheta->Reset();
    m_neuroHWOutQuad0InvPt->Reset();
    m_neuroHWOutQuad0Phi0->Reset();
    m_neuroHWOutQuad1Z->Reset();
    m_neuroHWOutQuad1CosTheta->Reset();
    m_neuroHWOutQuad1InvPt->Reset();
    m_neuroHWOutQuad1Phi0->Reset();
    m_neuroHWOutQuad2Z->Reset();
    m_neuroHWOutQuad2CosTheta->Reset();
    m_neuroHWOutQuad2InvPt->Reset();
    m_neuroHWOutQuad2Phi0->Reset();
    m_neuroHWOutQuad3Z->Reset();
    m_neuroHWOutQuad3CosTheta->Reset();
    m_neuroHWOutQuad3InvPt->Reset();
    m_neuroHWOutQuad3Phi0->Reset();

    m_neuroHWInTSPrioT_Layer0->Reset();
    m_neuroHWInTSPrioT_Layer1->Reset();
    m_neuroHWInTSPrioT_Layer2->Reset();
    m_neuroHWInTSPrioT_Layer3->Reset();
    m_neuroHWInTSPrioT_Layer4->Reset();
    m_neuroHWInTSPrioT_Layer5->Reset();
    m_neuroHWInTSPrioT_Layer6->Reset();
    m_neuroHWInTSPrioT_Layer7->Reset();
    m_neuroHWInTSPrioT_Layer8->Reset();
    m_neuroHWInTSFoundT_Layer0->Reset();
    m_neuroHWInTSFoundT_Layer1->Reset();
    m_neuroHWInTSFoundT_Layer2->Reset();
    m_neuroHWInTSFoundT_Layer3->Reset();
    m_neuroHWInTSFoundT_Layer4->Reset();
    m_neuroHWInTSFoundT_Layer5->Reset();
    m_neuroHWInTSFoundT_Layer6->Reset();
    m_neuroHWInTSFoundT_Layer7->Reset();
    m_neuroHWInTSFoundT_Layer8->Reset();

    m_neuroHWInTSPrioB_Layer0->Reset();
    m_neuroHWInTSPrioB_Layer1->Reset();
    m_neuroHWInTSPrioB_Layer2->Reset();
    m_neuroHWInTSPrioB_Layer3->Reset();
    m_neuroHWInTSPrioB_Layer4->Reset();
    m_neuroHWInTSPrioB_Layer5->Reset();
    m_neuroHWInTSPrioB_Layer6->Reset();
    m_neuroHWInTSPrioB_Layer7->Reset();
    m_neuroHWInTSPrioB_Layer8->Reset();
    m_neuroHWInTSLR_Layer0->Reset();
    m_neuroHWInTSLR_Layer1->Reset();
    m_neuroHWInTSLR_Layer2->Reset();
    m_neuroHWInTSLR_Layer3->Reset();
    m_neuroHWInTSLR_Layer4->Reset();
    m_neuroHWInTSLR_Layer5->Reset();
    m_neuroHWInTSLR_Layer6->Reset();
    m_neuroHWInTSLR_Layer7->Reset();
    m_neuroHWInTSLR_Layer8->Reset();

    m_neuroHWSelTSPrioT_Layer0->Reset();
    m_neuroHWSelTSPrioT_Layer1->Reset();
    m_neuroHWSelTSPrioT_Layer2->Reset();
    m_neuroHWSelTSPrioT_Layer3->Reset();
    m_neuroHWSelTSPrioT_Layer4->Reset();
    m_neuroHWSelTSPrioT_Layer5->Reset();
    m_neuroHWSelTSPrioT_Layer6->Reset();
    m_neuroHWSelTSPrioT_Layer7->Reset();
    m_neuroHWSelTSPrioT_Layer8->Reset();
    m_neuroHWSelTSFoundT_Layer0->Reset();
    m_neuroHWSelTSFoundT_Layer1->Reset();
    m_neuroHWSelTSFoundT_Layer2->Reset();
    m_neuroHWSelTSFoundT_Layer3->Reset();
    m_neuroHWSelTSFoundT_Layer4->Reset();
    m_neuroHWSelTSFoundT_Layer5->Reset();
    m_neuroHWSelTSFoundT_Layer6->Reset();
    m_neuroHWSelTSFoundT_Layer7->Reset();
    m_neuroHWSelTSFoundT_Layer8->Reset();

    m_neuroHWSelTSPrioB_Layer0->Reset();
    m_neuroHWSelTSPrioB_Layer1->Reset();
    m_neuroHWSelTSPrioB_Layer2->Reset();
    m_neuroHWSelTSPrioB_Layer3->Reset();
    m_neuroHWSelTSPrioB_Layer4->Reset();
    m_neuroHWSelTSPrioB_Layer5->Reset();
    m_neuroHWSelTSPrioB_Layer6->Reset();
    m_neuroHWSelTSPrioB_Layer7->Reset();
    m_neuroHWSelTSPrioB_Layer8->Reset();
    m_neuroHWSelTSLR_Layer0->Reset();
    m_neuroHWSelTSLR_Layer1->Reset();
    m_neuroHWSelTSLR_Layer2->Reset();
    m_neuroHWSelTSLR_Layer3->Reset();
    m_neuroHWSelTSLR_Layer4->Reset();
    m_neuroHWSelTSLR_Layer5->Reset();
    m_neuroHWSelTSLR_Layer6->Reset();
    m_neuroHWSelTSLR_Layer7->Reset();
    m_neuroHWSelTSLR_Layer8->Reset();

    m_neuroHWInputID_Layer0->Reset();
    m_neuroHWInputT_Layer0->Reset();
    m_neuroHWInputAlpha_Layer0->Reset();
    m_neuroHWInputID_Layer1->Reset();
    m_neuroHWInputT_Layer1->Reset();
    m_neuroHWInputAlpha_Layer1->Reset();
    m_neuroHWInputID_Layer2->Reset();
    m_neuroHWInputT_Layer2->Reset();
    m_neuroHWInputAlpha_Layer2->Reset();
    m_neuroHWInputID_Layer3->Reset();
    m_neuroHWInputT_Layer3->Reset();
    m_neuroHWInputAlpha_Layer3->Reset();
    m_neuroHWInputID_Layer4->Reset();
    m_neuroHWInputT_Layer4->Reset();
    m_neuroHWInputAlpha_Layer4->Reset();
    m_neuroHWInputID_Layer5->Reset();
    m_neuroHWInputT_Layer5->Reset();
    m_neuroHWInputAlpha_Layer5->Reset();
    m_neuroHWInputID_Layer6->Reset();
    m_neuroHWInputT_Layer6->Reset();
    m_neuroHWInputAlpha_Layer6->Reset();
    m_neuroHWInputID_Layer7->Reset();
    m_neuroHWInputT_Layer7->Reset();
    m_neuroHWInputAlpha_Layer7->Reset();
    m_neuroHWInputID_Layer8->Reset();
    m_neuroHWInputT_Layer8->Reset();
    m_neuroHWInputAlpha_Layer8->Reset();
  }
  if (!m_limitedoutput && m_simNeuroTracksName != "") {
    m_neuroSWOutZ->Reset();
    m_neuroSWOutCosTheta->Reset();
    m_neuroSWOutPhi0->Reset();
    m_neuroSWOutInvPt->Reset();
    m_neuroSWOutHitPattern->Reset();
    m_neuroSWOutTrackCount->Reset();
    m_neuroSWSector->Reset();
  }
  if (!m_limitedoutput && m_simNeuroTracksSWTSSW2DName != "") {
    m_2DSWOutInvPt->Reset();
    m_2DSWOutPhi0->Reset();
    m_2DSWOutm_time->Reset();
    m_2DSWOutTrackCount->Reset();

    m_neuroSWTSSW2DOutZ->Reset();
    m_neuroSWTSSW2DOutCosTheta->Reset();
    m_neuroSWTSSW2DOutInvPt->Reset();
    m_neuroSWTSSW2DOutPhi0->Reset();
    m_neuroSWTSSW2DOutHitPattern->Reset();
    m_neuroSWTSSW2DOutTrackCount->Reset();
    m_neuroSWTSSW2DSector->Reset();
  }
  if (!m_limitedoutput && m_simNeuroTracksName != "") {
    m_neuroDeltaZ->Reset();
    m_neuroDeltaTheta->Reset();
    m_neuroScatterZ->Reset();
    m_neuroScatterTheta->Reset();

    m_neuroDeltaInputID->Reset();
    m_neuroDeltaInputT->Reset();
    m_neuroDeltaInputAlpha->Reset();
    m_neuroDeltaTSID->Reset();
    m_neuroDeltaSector->Reset();
    m_simSameTS->Reset();
    m_simDiffTS->Reset();
  }
  if (!m_limitedoutput && m_unpacked2DTracksName != "") {
    m_2DHWInTSPrioT_Layer0->Reset();
    m_2DHWInTSPrioT_Layer2->Reset();
    m_2DHWInTSPrioT_Layer4->Reset();
    m_2DHWInTSPrioT_Layer6->Reset();
    m_2DHWInTSPrioT_Layer8->Reset();
    m_2DHWInTSFoundT_Layer0->Reset();
    m_2DHWInTSFoundT_Layer2->Reset();
    m_2DHWInTSFoundT_Layer4->Reset();
    m_2DHWInTSFoundT_Layer6->Reset();
    m_2DHWInTSFoundT_Layer8->Reset();

    m_2DHWInTSPrioB_Layer0->Reset();
    m_2DHWInTSPrioB_Layer2->Reset();
    m_2DHWInTSPrioB_Layer4->Reset();
    m_2DHWInTSPrioB_Layer6->Reset();
    m_2DHWInTSPrioB_Layer8->Reset();
    m_2DHWInTSLR_Layer0->Reset();
    m_2DHWInTSLR_Layer2->Reset();
    m_2DHWInTSLR_Layer4->Reset();
    m_2DHWInTSLR_Layer6->Reset();
    m_2DHWInTSLR_Layer8->Reset();
  }
  if (!m_limitedoutput && m_simNeuroTracksName != "") {
    m_neuroSWSelTSID->Reset();
    m_neuroSWSelTSCount->Reset();
    m_neuroSWSelTSPrioT_Layer0->Reset();
    m_neuroSWSelTSPrioT_Layer1->Reset();
    m_neuroSWSelTSPrioT_Layer2->Reset();
    m_neuroSWSelTSPrioT_Layer3->Reset();
    m_neuroSWSelTSPrioT_Layer4->Reset();
    m_neuroSWSelTSPrioT_Layer5->Reset();
    m_neuroSWSelTSPrioT_Layer6->Reset();
    m_neuroSWSelTSPrioT_Layer7->Reset();
    m_neuroSWSelTSPrioT_Layer8->Reset();
    m_neuroSWSelTSFoundT_Layer0->Reset();
    m_neuroSWSelTSFoundT_Layer1->Reset();
    m_neuroSWSelTSFoundT_Layer2->Reset();
    m_neuroSWSelTSFoundT_Layer3->Reset();
    m_neuroSWSelTSFoundT_Layer4->Reset();
    m_neuroSWSelTSFoundT_Layer5->Reset();
    m_neuroSWSelTSFoundT_Layer6->Reset();
    m_neuroSWSelTSFoundT_Layer7->Reset();
    m_neuroSWSelTSFoundT_Layer8->Reset();

    m_neuroSWSelTSPrioB_Layer0->Reset();
    m_neuroSWSelTSPrioB_Layer1->Reset();
    m_neuroSWSelTSPrioB_Layer2->Reset();
    m_neuroSWSelTSPrioB_Layer3->Reset();
    m_neuroSWSelTSPrioB_Layer4->Reset();
    m_neuroSWSelTSPrioB_Layer5->Reset();
    m_neuroSWSelTSPrioB_Layer6->Reset();
    m_neuroSWSelTSPrioB_Layer7->Reset();
    m_neuroSWSelTSPrioB_Layer8->Reset();

    m_neuroSWSelTSLR_Layer0->Reset();
    m_neuroSWSelTSLR_Layer1->Reset();
    m_neuroSWSelTSLR_Layer2->Reset();
    m_neuroSWSelTSLR_Layer3->Reset();
    m_neuroSWSelTSLR_Layer4->Reset();
    m_neuroSWSelTSLR_Layer5->Reset();
    m_neuroSWSelTSLR_Layer6->Reset();
    m_neuroSWSelTSLR_Layer7->Reset();
    m_neuroSWSelTSLR_Layer8->Reset();

    m_neuroSWInputID_Layer0->Reset();
    m_neuroSWInputT_Layer0->Reset();
    m_neuroSWInputAlpha_Layer0->Reset();
    m_neuroSWInputID_Layer1->Reset();
    m_neuroSWInputT_Layer1->Reset();
    m_neuroSWInputAlpha_Layer1->Reset();
    m_neuroSWInputID_Layer2->Reset();
    m_neuroSWInputT_Layer2->Reset();
    m_neuroSWInputAlpha_Layer2->Reset();
    m_neuroSWInputID_Layer3->Reset();
    m_neuroSWInputT_Layer3->Reset();
    m_neuroSWInputAlpha_Layer3->Reset();
    m_neuroSWInputID_Layer4->Reset();
    m_neuroSWInputT_Layer4->Reset();
    m_neuroSWInputAlpha_Layer4->Reset();
    m_neuroSWInputID_Layer5->Reset();
    m_neuroSWInputT_Layer5->Reset();
    m_neuroSWInputAlpha_Layer5->Reset();
    m_neuroSWInputID_Layer6->Reset();
    m_neuroSWInputT_Layer6->Reset();
    m_neuroSWInputAlpha_Layer6->Reset();
    m_neuroSWInputID_Layer7->Reset();
    m_neuroSWInputT_Layer7->Reset();
    m_neuroSWInputAlpha_Layer7->Reset();
    m_neuroSWInputID_Layer8->Reset();
    m_neuroSWInputT_Layer8->Reset();
    m_neuroSWInputAlpha_Layer8->Reset();
  }

  if (!m_limitedoutput && m_simNeuroTracksSWTSSW2DName != "") {
    m_neuroSWTSSW2DInTSID->Reset();
    m_neuroSWTSSW2DInTSCount->Reset();
    m_neuroSWTSSW2DInTSPrioT_Layer0->Reset();
    m_neuroSWTSSW2DInTSPrioT_Layer1->Reset();
    m_neuroSWTSSW2DInTSPrioT_Layer2->Reset();
    m_neuroSWTSSW2DInTSPrioT_Layer3->Reset();
    m_neuroSWTSSW2DInTSPrioT_Layer4->Reset();
    m_neuroSWTSSW2DInTSPrioT_Layer5->Reset();
    m_neuroSWTSSW2DInTSPrioT_Layer6->Reset();
    m_neuroSWTSSW2DInTSPrioT_Layer7->Reset();
    m_neuroSWTSSW2DInTSPrioT_Layer8->Reset();
    m_neuroSWTSSW2DInTSFoundT_Layer0->Reset();
    m_neuroSWTSSW2DInTSFoundT_Layer1->Reset();
    m_neuroSWTSSW2DInTSFoundT_Layer2->Reset();
    m_neuroSWTSSW2DInTSFoundT_Layer3->Reset();
    m_neuroSWTSSW2DInTSFoundT_Layer4->Reset();
    m_neuroSWTSSW2DInTSFoundT_Layer5->Reset();
    m_neuroSWTSSW2DInTSFoundT_Layer6->Reset();
    m_neuroSWTSSW2DInTSFoundT_Layer7->Reset();
    m_neuroSWTSSW2DInTSFoundT_Layer8->Reset();
    m_neuroSWTSSW2DInTSPrioB_Layer0->Reset();
    m_neuroSWTSSW2DInTSPrioB_Layer1->Reset();
    m_neuroSWTSSW2DInTSPrioB_Layer2->Reset();
    m_neuroSWTSSW2DInTSPrioB_Layer3->Reset();
    m_neuroSWTSSW2DInTSPrioB_Layer4->Reset();
    m_neuroSWTSSW2DInTSPrioB_Layer5->Reset();
    m_neuroSWTSSW2DInTSPrioB_Layer6->Reset();
    m_neuroSWTSSW2DInTSPrioB_Layer7->Reset();
    m_neuroSWTSSW2DInTSPrioB_Layer8->Reset();
    m_neuroSWTSSW2DInTSLR_Layer0->Reset();
    m_neuroSWTSSW2DInTSLR_Layer1->Reset();
    m_neuroSWTSSW2DInTSLR_Layer2->Reset();
    m_neuroSWTSSW2DInTSLR_Layer3->Reset();
    m_neuroSWTSSW2DInTSLR_Layer4->Reset();
    m_neuroSWTSSW2DInTSLR_Layer5->Reset();
    m_neuroSWTSSW2DInTSLR_Layer6->Reset();
    m_neuroSWTSSW2DInTSLR_Layer7->Reset();
    m_neuroSWTSSW2DInTSLR_Layer8->Reset();

    m_neuroSWTSSW2DSelTSID->Reset();
    m_neuroSWTSSW2DSelTSCount->Reset();
    m_neuroSWTSSW2DSelTSPrioT_Layer0->Reset();
    m_neuroSWTSSW2DSelTSPrioT_Layer1->Reset();
    m_neuroSWTSSW2DSelTSPrioT_Layer2->Reset();
    m_neuroSWTSSW2DSelTSPrioT_Layer3->Reset();
    m_neuroSWTSSW2DSelTSPrioT_Layer4->Reset();
    m_neuroSWTSSW2DSelTSPrioT_Layer5->Reset();
    m_neuroSWTSSW2DSelTSPrioT_Layer6->Reset();
    m_neuroSWTSSW2DSelTSPrioT_Layer7->Reset();
    m_neuroSWTSSW2DSelTSPrioT_Layer8->Reset();
    m_neuroSWTSSW2DSelTSFoundT_Layer0->Reset();
    m_neuroSWTSSW2DSelTSFoundT_Layer1->Reset();
    m_neuroSWTSSW2DSelTSFoundT_Layer2->Reset();
    m_neuroSWTSSW2DSelTSFoundT_Layer3->Reset();
    m_neuroSWTSSW2DSelTSFoundT_Layer4->Reset();
    m_neuroSWTSSW2DSelTSFoundT_Layer5->Reset();
    m_neuroSWTSSW2DSelTSFoundT_Layer6->Reset();
    m_neuroSWTSSW2DSelTSFoundT_Layer7->Reset();
    m_neuroSWTSSW2DSelTSFoundT_Layer8->Reset();
    m_neuroSWTSSW2DSelTSPrioB_Layer0->Reset();
    m_neuroSWTSSW2DSelTSPrioB_Layer1->Reset();
    m_neuroSWTSSW2DSelTSPrioB_Layer2->Reset();
    m_neuroSWTSSW2DSelTSPrioB_Layer3->Reset();
    m_neuroSWTSSW2DSelTSPrioB_Layer4->Reset();
    m_neuroSWTSSW2DSelTSPrioB_Layer5->Reset();
    m_neuroSWTSSW2DSelTSPrioB_Layer6->Reset();
    m_neuroSWTSSW2DSelTSPrioB_Layer7->Reset();
    m_neuroSWTSSW2DSelTSPrioB_Layer8->Reset();
    m_neuroSWTSSW2DSelTSLR_Layer0->Reset();
    m_neuroSWTSSW2DSelTSLR_Layer1->Reset();
    m_neuroSWTSSW2DSelTSLR_Layer2->Reset();
    m_neuroSWTSSW2DSelTSLR_Layer3->Reset();
    m_neuroSWTSSW2DSelTSLR_Layer4->Reset();
    m_neuroSWTSSW2DSelTSLR_Layer5->Reset();
    m_neuroSWTSSW2DSelTSLR_Layer6->Reset();
    m_neuroSWTSSW2DSelTSLR_Layer7->Reset();
    m_neuroSWTSSW2DSelTSLR_Layer8->Reset();

    m_neuroSWTSSW2DInputID_Layer0->Reset();
    m_neuroSWTSSW2DInputT_Layer0->Reset();
    m_neuroSWTSSW2DInputAlpha_Layer0->Reset();
    m_neuroSWTSSW2DInputID_Layer1->Reset();
    m_neuroSWTSSW2DInputT_Layer1->Reset();
    m_neuroSWTSSW2DInputAlpha_Layer1->Reset();
    m_neuroSWTSSW2DInputID_Layer2->Reset();
    m_neuroSWTSSW2DInputT_Layer2->Reset();
    m_neuroSWTSSW2DInputAlpha_Layer2->Reset();
    m_neuroSWTSSW2DInputID_Layer3->Reset();
    m_neuroSWTSSW2DInputT_Layer3->Reset();
    m_neuroSWTSSW2DInputAlpha_Layer3->Reset();
    m_neuroSWTSSW2DInputID_Layer4->Reset();
    m_neuroSWTSSW2DInputT_Layer4->Reset();
    m_neuroSWTSSW2DInputAlpha_Layer4->Reset();
    m_neuroSWTSSW2DInputID_Layer5->Reset();
    m_neuroSWTSSW2DInputT_Layer5->Reset();
    m_neuroSWTSSW2DInputAlpha_Layer5->Reset();
    m_neuroSWTSSW2DInputID_Layer6->Reset();
    m_neuroSWTSSW2DInputT_Layer6->Reset();
    m_neuroSWTSSW2DInputAlpha_Layer6->Reset();
    m_neuroSWTSSW2DInputID_Layer7->Reset();
    m_neuroSWTSSW2DInputT_Layer7->Reset();
    m_neuroSWTSSW2DInputAlpha_Layer7->Reset();
    m_neuroSWTSSW2DInputID_Layer8->Reset();
    m_neuroSWTSSW2DInputT_Layer8->Reset();
    m_neuroSWTSSW2DInputAlpha_Layer8->Reset();
  }
  if (m_recoTracksName != "") {
    m_RecoZ->Reset();
    m_RecoCosTheta->Reset();
    m_RecoInvPt->Reset();
    m_RecoPhi->Reset();
    m_RecoD0->Reset();
    m_RecoTrackCount->Reset();

    m_RecoHWZ->Reset();
    m_RecoHWCosTheta->Reset();
    m_RecoHWInvPt->Reset();
    m_RecoHWPhi->Reset();
    m_RecoHWD0->Reset();
  }
  if (!m_limitedoutput && m_recoTracksName != "") {
    m_RecoHWZScatter->Reset();
  }
  if (m_unpackedNeuroTracksName != "" && m_recoTracksName != "") {
    m_neuroRecoHWOutZ->Reset();
    m_neuroRecoHWOutCosTheta->Reset();
    m_neuroRecoHWOutInvPt->Reset();
    m_neuroRecoHWOutPhi0->Reset();
    m_neuroRecoHWOutHitPattern->Reset();
    m_neuroRecoHWOutTrackCount->Reset();
    m_neuroRecoHWSector->Reset();

    m_DeltaRecoHWZ->Reset();
    m_DeltaRecoHWCosTheta->Reset();
    m_DeltaRecoHWInvPt->Reset();
    m_DeltaRecoHWPhi->Reset();
  }
  if (!m_limitedoutput && m_simNeuroTracksName != "" && m_recoTracksName != "") {
    m_RecoSWZ->Reset();
    m_RecoSWCosTheta->Reset();
    m_RecoSWInvPt->Reset();
    m_RecoSWPhi->Reset();
    m_RecoSWD0->Reset();
    m_RecoSWZScatter->Reset();

    m_neuroRecoSWOutZ->Reset();
    m_neuroRecoSWOutCosTheta->Reset();
    m_neuroRecoSWOutInvPt->Reset();
    m_neuroRecoSWOutPhi0->Reset();
    m_neuroRecoSWOutHitPattern->Reset();
    m_neuroRecoSWOutTrackCount->Reset();
    m_neuroRecoSWSector->Reset();

    m_DeltaRecoSWZ->Reset();
    m_DeltaRecoSWCosTheta->Reset();
    m_DeltaRecoSWInvPt->Reset();
    m_DeltaRecoSWPhi->Reset();
  }
  if (!m_limitedoutput && m_simNeuroTracksSWTSSW2DName != "" && m_recoTracksName != "") {
    m_RecoSWTSSW2DZ->Reset();
    m_RecoSWTSSW2DCosTheta->Reset();
    m_RecoSWTSSW2DInvPt->Reset();
    m_RecoSWTSSW2DPhi->Reset();
    m_RecoSWTSSW2DD0->Reset();
    m_RecoSWTSSW2DZScatter->Reset();

    m_neuroRecoSWTSSW2DOutZ->Reset();
    m_neuroRecoSWTSSW2DOutCosTheta->Reset();
    m_neuroRecoSWTSSW2DOutInvPt->Reset();
    m_neuroRecoSWTSSW2DOutPhi0->Reset();
    m_neuroRecoSWTSSW2DOutHitPattern->Reset();
    m_neuroRecoSWTSSW2DOutTrackCount->Reset();
    m_neuroRecoSWTSSW2DSector->Reset();

    m_DeltaRecoSWTSSW2DZ->Reset();
    m_DeltaRecoSWTSSW2DCosTheta->Reset();
    m_DeltaRecoSWTSSW2DInvPt->Reset();
    m_DeltaRecoSWTSSW2DPhi->Reset();
  }

}

void CDCTriggerNeuroDQMModule::event()
{
  double phinorm ; /* intermediate name for phi to transfrom zo 0->360 */
  if (m_skipWithoutHWTS and m_unpackedNeuroInputSegments.getEntries() == 0) {
    //if (m_unpackedNeuroInputSegments.getEntries() == 0) {
    B2DEBUG(150, "No unpacked TS found, skipping event.");
    return;
  }
  if (m_recoTrackMultiplicity != -1 and m_RecoTracks.getEntries() != m_recoTrackMultiplicity) {
    B2DEBUG(150, "Wrong track multiplicity " << m_RecoTracks.getEntries() << ", skipping event.");
    return;
  }

  StoreObjPtr<EventMetaData> eventMetaData;

  if (m_recoTracksName != "") {
    // a RecoTrack has multiple representations for different particle hypothesis
    // -> just take the first one that does not give errors.
    m_RecoTrackCount->Fill(m_RecoTracks.getEntries());
    bool foundValidRep = false;

    for (RecoTrack& recoTrack : m_RecoTracks) {
      int nhwmatched = 0;
      int nswmatched = 0;
      int nswtssw2dmatched = 0;
      double phi0Target = 0;
      double invptTarget = 0;
      double cosThetaTarget = 0;
      double zTarget = 0;
      double d0Target = 0;
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
          phi0Target = state.getMom().Phi() * 180. / M_PI ;
          if (phi0Target < 0.) {phi0Target = phi0Target + 360. ;}
          invptTarget = state.getCharge() * state.getMom().Pt();
          cosThetaTarget = state.getMom().CosTheta();
          zTarget = state.getPos().Z();
          d0Target = state.getPos().Perp();
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
        if (m_maxRecoZDist != -1.0 and abs(zTarget) > m_maxRecoZDist) {
          B2DEBUG(150, "RecoTrack not close to IP (maxRecoZDist), zReco = " << zTarget);
          continue;
        }
        if (m_maxRecoD0Dist != -1.0 and abs(d0Target) > m_maxRecoD0Dist) {
          B2DEBUG(150, "RecoTrack not close to the z axis (maxRecoD0Dist), d0Reco = " << d0Target);
          continue;
        }
        m_RecoZ->Fill(zTarget);
        m_RecoCosTheta->Fill(cosThetaTarget);
        m_RecoPhi->Fill(phi0Target);
        m_RecoD0->Fill(d0Target);
        m_RecoInvPt->Fill(invptTarget);
        if (m_unpackedNeuroTracksName != "") {
          CDCTriggerTrack* neuroHWTrack = recoTrack.getRelatedTo<CDCTriggerTrack>(m_unpackedNeuroTracksName);

          if (neuroHWTrack) {
            bool valtrack = false;
            try {
              valtrack = neuroHWTrack->getValidStereoBit();
            } catch (...) {
              B2WARNING("HWTrack doesn't have 'valid bit', get it from relations now... ");
              unsigned checkpattern = getPattern(neuroHWTrack, m_unpackedNeuroInputSegmentsName);
              valtrack = isValidPattern(checkpattern);
            }
            if (!valtrack) {
              continue;
            }

            m_RecoHWZ->Fill(zTarget);
            m_RecoHWCosTheta->Fill(cosThetaTarget);
            m_RecoHWPhi->Fill(phi0Target);
            m_RecoHWD0->Fill(d0Target);
            m_RecoHWInvPt->Fill(invptTarget);

            double neuroHWZ = neuroHWTrack->getZ0() ;
            m_neuroRecoHWOutZ->Fill(neuroHWZ);
            double cotTh = neuroHWTrack->getCotTheta() ;
            double cosTh = copysign(1.0, cotTh) / sqrt(1. / (cotTh * cotTh) + 1);
            double neuroHWcosTh = cosTh ;
            m_neuroRecoHWOutCosTheta->Fill(neuroHWcosTh);
            double neuroHWPt = neuroHWTrack->getPt() ;
            m_neuroRecoHWOutInvPt->Fill(neuroHWPt);
            phinorm = neuroHWTrack->getPhi0() * 180 / M_PI ;
            if (phinorm < 0.) {phinorm = phinorm + 360. ;}
            double neuroHWPhi0 = phinorm;
            m_neuroRecoHWOutPhi0->Fill(neuroHWPhi0);
            m_neuroRecoHWOutHitPattern->Fill(getPattern(neuroHWTrack, m_unpackedNeuroInputSegmentsName));
            nhwmatched++;
            unsigned hwMatchedSector =
              neuroHWTrack->getRelatedTo<CDCTriggerMLPInput>(m_unpackedNeuroInputVectorName)->getSector();
            m_neuroRecoHWSector->Fill(hwMatchedSector);

            m_DeltaRecoHWZ->Fill(zTarget - neuroHWZ);
            m_DeltaRecoHWCosTheta->Fill(cosThetaTarget - neuroHWcosTh);
            m_DeltaRecoHWPhi->Fill(phi0Target - neuroHWPhi0);
            m_DeltaRecoHWInvPt->Fill(invptTarget - neuroHWPt);
            if (!m_limitedoutput) {
              m_RecoHWZScatter->Fill(zTarget, neuroHWZ);
            }
          }
          if (!m_limitedoutput && m_simNeuroTracksName != "") {
            CDCTriggerTrack* neuroSWTrack = recoTrack.getRelatedTo<CDCTriggerTrack>(m_simNeuroTracksName);
            if (neuroSWTrack) {
              bool valtrack = false;
              try {
                valtrack = neuroSWTrack->getValidStereoBit();
              } catch (...) {
                B2INFO("SWTrack doesn't have 'valid bit', get it from relations now... ");
                unsigned checkpattern = getPattern(neuroSWTrack, m_unpackedNeuroInputSegmentsName);
                valtrack = isValidPattern(checkpattern);
              }
              if (!valtrack) {
                continue;
              }

              m_RecoSWZ->Fill(zTarget);
              m_RecoSWCosTheta->Fill(cosThetaTarget);
              m_RecoSWPhi->Fill(phi0Target);
              m_RecoSWD0->Fill(d0Target);
              m_RecoSWInvPt->Fill(invptTarget);

              double neuroSWZ = neuroSWTrack->getZ0() ;
              m_neuroRecoSWOutZ->Fill(neuroSWZ);
              double cotTh = neuroSWTrack->getCotTheta() ;
              double cosTh = copysign(1.0, cotTh) / sqrt(1. / (cotTh * cotTh) + 1);
              double neuroSWcosTh = cosTh ;
              m_neuroRecoSWOutCosTheta->Fill(neuroSWcosTh);
              double neuroSWPt = neuroSWTrack->getPt() ;
              m_neuroRecoSWOutInvPt->Fill(neuroSWPt);
              phinorm = neuroSWTrack->getPhi0() * 180 / M_PI ;
              if (phinorm < 0.) {phinorm = phinorm + 360. ;}
              double neuroSWPhi0 = phinorm ;
              m_neuroRecoSWOutPhi0->Fill(neuroSWPhi0);
              m_neuroRecoSWOutHitPattern->Fill(getPattern(neuroSWTrack, m_unpackedNeuroInputSegmentsName));
              nswmatched++;
              unsigned swMatchedSector =
                neuroSWTrack->getRelatedTo<CDCTriggerMLPInput>(m_simNeuroInputVectorName)->getSector();
              m_neuroRecoSWSector->Fill(swMatchedSector);

              m_DeltaRecoSWZ->Fill(zTarget - neuroSWZ);
              m_DeltaRecoSWCosTheta->Fill(cosThetaTarget - neuroSWcosTh);
              m_DeltaRecoSWPhi->Fill(phi0Target - neuroSWPhi0);
              m_DeltaRecoSWInvPt->Fill(invptTarget - neuroSWPt);
              m_RecoSWZScatter->Fill(zTarget, neuroSWZ);
            }
          }
          if (!m_limitedoutput && m_simNeuroTracksSWTSSW2DName != "") {
            CDCTriggerTrack* neuroSWTSSW2DTrack = recoTrack.getRelatedTo<CDCTriggerTrack>(m_simNeuroTracksSWTSSW2DName);
            if (neuroSWTSSW2DTrack) {
              m_RecoSWTSSW2DZ->Fill(zTarget);
              m_RecoSWTSSW2DCosTheta->Fill(cosThetaTarget);
              m_RecoSWTSSW2DPhi->Fill(phi0Target);
              m_RecoSWTSSW2DD0->Fill(d0Target);
              m_RecoSWTSSW2DInvPt->Fill(invptTarget);

              m_neuroRecoSWTSSW2DOutZ->Fill(neuroSWTSSW2DTrack->getZ0());

              double cotTh = neuroSWTSSW2DTrack->getCotTheta();
              double cosTh = copysign(1.0, cotTh) / sqrt(1. / (cotTh * cotTh) + 1);
              m_neuroRecoSWTSSW2DOutCosTheta->Fill(cosTh);
              phinorm = neuroSWTSSW2DTrack->getPhi0() * 180 / M_PI;
              if (phinorm < 0.) {phinorm = phinorm + 360. ;}
              double phiNeuro = phinorm ;
              double invptNeuro = neuroSWTSSW2DTrack->getPt() ;
              m_neuroRecoSWTSSW2DOutInvPt->Fill(invptNeuro);
              m_neuroRecoSWTSSW2DOutPhi0->Fill(phiNeuro);
              m_neuroRecoSWTSSW2DOutHitPattern->Fill(getPattern(neuroSWTSSW2DTrack, m_simSegmentHitsName));
              nswtssw2dmatched++;
              unsigned swtssw2dMatchedSector =
                neuroSWTSSW2DTrack->getRelatedTo<CDCTriggerMLPInput>(m_simNeuroInputVectorSWTSSW2DName)->getSector();
              m_neuroRecoSWTSSW2DSector->Fill(swtssw2dMatchedSector);

              m_DeltaRecoSWTSSW2DZ->Fill(zTarget - neuroSWTSSW2DTrack->getZ0());


              m_DeltaRecoSWTSSW2DCosTheta->Fill(cosThetaTarget - cosTh);
              phinorm = neuroSWTSSW2DTrack->getPhi0() * 180. / M_PI ;
              if (phinorm < 0.) {phinorm = phinorm  + 360. ;}
              m_DeltaRecoSWTSSW2DPhi->Fill(phi0Target - phinorm);
              m_DeltaRecoSWTSSW2DInvPt->Fill(invptTarget - neuroSWTSSW2DTrack->getPt());
              m_RecoSWTSSW2DZScatter->Fill(zTarget, neuroSWTSSW2DTrack->getZ0());
            }
          }
        }
      }
      if (m_unpackedNeuroTracksName != "") {
        m_neuroRecoHWOutTrackCount->Fill(nhwmatched);
        if (!m_limitedoutput && m_simNeuroTracksName != "") {
          m_neuroRecoSWOutTrackCount->Fill(nswmatched);
        }
        if (!m_limitedoutput && m_simNeuroTracksSWTSSW2DName != "") {
          m_neuroRecoSWTSSW2DOutTrackCount->Fill(nswtssw2dmatched);
        }
      }
    }
  }

  if (!m_limitedoutput && m_simNeuroTracksSWTSSW2DName != "") {
    m_neuroSWTSSW2DOutTrackCount->Fill(m_simNeuroTracksSWTSSW2D.getEntries());
    for (CDCTriggerTrack& neuroswTrack : m_simNeuroTracksSWTSSW2D) {
      m_neuroSWTSSW2DOutZ->Fill(neuroswTrack.getZ0());
      double cotThSW = neuroswTrack.getCotTheta();
      double cosThSW = copysign(1.0, cotThSW) / sqrt(1. / (cotThSW * cotThSW) + 1);
      m_neuroSWTSSW2DOutCosTheta->Fill(cosThSW);
      phinorm = neuroswTrack.getPhi0() * 180. / M_PI  ;
      if (phinorm < 0.) {
        phinorm = phinorm + 360. ;
      }
      m_neuroSWTSSW2DOutPhi0->Fill(phinorm);
      m_neuroSWTSSW2DOutInvPt->Fill(neuroswTrack.getPt());
      unsigned simSWTSSW2DSector =
        neuroswTrack.getRelatedTo<CDCTriggerMLPInput>(m_simNeuroInputVectorSWTSSW2DName)->getSector();
      m_neuroSWTSSW2DSector->Fill(simSWTSSW2DSector);

      // sw NN selected sw ts
      unsigned pattern = 0;
      for (const CDCTriggerSegmentHit& hit :
           neuroswTrack.getRelationsTo<CDCTriggerSegmentHit>(m_simSegmentHitsName)) {
        m_neuroSWTSSW2DSelTSID->Fill(hit.getSegmentID());
        unsigned int sl = hit.getISuperLayer();
        m_neuroSWTSSW2DSelTSCount->Fill(sl);
        float neuroTime = neuroswTrack.getTime();

        // find first occurence of hit (used to debug the selcted TS field)
        CDCTriggerSegmentHit firstHit = hit;
        for (CDCTriggerSegmentHit compare : m_simSegmentHits) {
          if (compare.getISuperLayer()       == hit.getISuperLayer()      &&
              compare.getIWireCenter()       == hit.getIWireCenter()      &&
              compare.getPriorityPosition()  == hit.getPriorityPosition() &&
              compare.getLeftRight()         == hit.getLeftRight()        &&
              compare.priorityTime()         == hit.priorityTime()        &&
              compare.foundTime()            < firstHit.foundTime()) {
            firstHit = compare;
          }
        }

        switch (sl) {
          case 0: m_neuroSWTSSW2DSelTSPrioT_Layer0->Fill(hit.priorityTime());
            m_neuroSWTSSW2DSelTSFoundT_Layer0->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWTSSW2DSelTSPrioB_Layer0->Fill(hit.getPriorityPosition());
            m_neuroSWTSSW2DSelTSLR_Layer0->Fill(hit.getLeftRight());
            break;
          case 1: m_neuroSWTSSW2DSelTSPrioT_Layer1->Fill(hit.priorityTime());
            m_neuroSWTSSW2DSelTSFoundT_Layer1->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWTSSW2DSelTSPrioB_Layer1->Fill(hit.getPriorityPosition());
            m_neuroSWTSSW2DSelTSLR_Layer1->Fill(hit.getLeftRight());
            break;
          case 2: m_neuroSWTSSW2DSelTSPrioT_Layer2->Fill(hit.priorityTime());
            m_neuroSWTSSW2DSelTSFoundT_Layer2->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWTSSW2DSelTSPrioB_Layer2->Fill(hit.getPriorityPosition());
            m_neuroSWTSSW2DSelTSLR_Layer2->Fill(hit.getLeftRight());
            break;
          case 3: m_neuroSWTSSW2DSelTSPrioT_Layer3->Fill(hit.priorityTime());
            m_neuroSWTSSW2DSelTSFoundT_Layer3->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWTSSW2DSelTSPrioB_Layer3->Fill(hit.getPriorityPosition());
            m_neuroSWTSSW2DSelTSLR_Layer3->Fill(hit.getLeftRight());
            break;
          case 4: m_neuroSWTSSW2DSelTSPrioT_Layer4->Fill(hit.priorityTime());
            m_neuroSWTSSW2DSelTSFoundT_Layer4->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWTSSW2DSelTSPrioB_Layer4->Fill(hit.getPriorityPosition());
            m_neuroSWTSSW2DSelTSLR_Layer4->Fill(hit.getLeftRight());
            break;
          case 5: m_neuroSWTSSW2DSelTSPrioT_Layer5->Fill(hit.priorityTime());
            m_neuroSWTSSW2DSelTSFoundT_Layer5->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWTSSW2DSelTSPrioB_Layer5->Fill(hit.getPriorityPosition());
            m_neuroSWTSSW2DSelTSLR_Layer5->Fill(hit.getLeftRight());
            break;
          case 6: m_neuroSWTSSW2DSelTSPrioT_Layer6->Fill(hit.priorityTime());
            m_neuroSWTSSW2DSelTSFoundT_Layer6->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWTSSW2DSelTSPrioB_Layer6->Fill(hit.getPriorityPosition());
            m_neuroSWTSSW2DSelTSLR_Layer6->Fill(hit.getLeftRight());
            break;
          case 7: m_neuroSWTSSW2DSelTSPrioT_Layer7->Fill(hit.priorityTime());
            m_neuroSWTSSW2DSelTSFoundT_Layer7->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWTSSW2DSelTSPrioB_Layer7->Fill(hit.getPriorityPosition());
            m_neuroSWTSSW2DSelTSLR_Layer7->Fill(hit.getLeftRight());
            break;
          case 8: m_neuroSWTSSW2DSelTSPrioT_Layer8->Fill(hit.priorityTime());
            m_neuroSWTSSW2DSelTSFoundT_Layer8->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWTSSW2DSelTSPrioB_Layer8->Fill(hit.getPriorityPosition());
            m_neuroSWTSSW2DSelTSLR_Layer8->Fill(hit.getLeftRight());
            break;
        }
        if (sl % 2 == 1) pattern |= (1 << ((sl - 1) / 2));
      }
      m_neuroSWTSSW2DOutHitPattern->Fill(pattern);

      // plot input vector
      vector<float> nnInput =
        neuroswTrack.getRelatedTo<CDCTriggerMLPInput>(m_simNeuroInputVectorSWTSSW2DName)->getInput();
      condFill(m_neuroSWTSSW2DInputID_Layer0    , nnInput[0]);
      condFill(m_neuroSWTSSW2DInputT_Layer0     , nnInput[1]);
      condFill(m_neuroSWTSSW2DInputAlpha_Layer0 , nnInput[2]);
      condFill(m_neuroSWTSSW2DInputID_Layer1    , nnInput[3]);
      condFill(m_neuroSWTSSW2DInputT_Layer1     , nnInput[4]);
      condFill(m_neuroSWTSSW2DInputAlpha_Layer1 , nnInput[5]);
      condFill(m_neuroSWTSSW2DInputID_Layer2    , nnInput[6]);
      condFill(m_neuroSWTSSW2DInputT_Layer2     , nnInput[7]);
      condFill(m_neuroSWTSSW2DInputAlpha_Layer2 , nnInput[8]);
      condFill(m_neuroSWTSSW2DInputID_Layer3    , nnInput[9]);
      condFill(m_neuroSWTSSW2DInputT_Layer3     , nnInput[10]);
      condFill(m_neuroSWTSSW2DInputAlpha_Layer3 , nnInput[11]);
      condFill(m_neuroSWTSSW2DInputID_Layer4    , nnInput[12]);
      condFill(m_neuroSWTSSW2DInputT_Layer4     , nnInput[13]);
      condFill(m_neuroSWTSSW2DInputAlpha_Layer4 , nnInput[14]);
      condFill(m_neuroSWTSSW2DInputID_Layer5    , nnInput[15]);
      condFill(m_neuroSWTSSW2DInputT_Layer5     , nnInput[16]);
      condFill(m_neuroSWTSSW2DInputAlpha_Layer5 , nnInput[17]);
      condFill(m_neuroSWTSSW2DInputID_Layer6    , nnInput[18]);
      condFill(m_neuroSWTSSW2DInputT_Layer6     , nnInput[19]);
      condFill(m_neuroSWTSSW2DInputAlpha_Layer6 , nnInput[20]);
      condFill(m_neuroSWTSSW2DInputID_Layer7    , nnInput[21]);
      condFill(m_neuroSWTSSW2DInputT_Layer7     , nnInput[22]);
      condFill(m_neuroSWTSSW2DInputAlpha_Layer7 , nnInput[23]);
      condFill(m_neuroSWTSSW2DInputID_Layer8    , nnInput[24]);
      condFill(m_neuroSWTSSW2DInputT_Layer8     , nnInput[25]);
      condFill(m_neuroSWTSSW2DInputAlpha_Layer8 , nnInput[26]);
    }
  }
  if (!m_limitedoutput && m_simNeuroTracksName != "") {
    m_neuroSWOutTrackCount->Fill(m_simNeuroTracks.getEntries());
    for (CDCTriggerTrack& neuroswTrack : m_simNeuroTracks) {
      bool valtrack = false;
      try {
        valtrack = neuroswTrack.getValidStereoBit();
      } catch (...) {
        B2INFO("HWTrack doesn't have 'valid bit', get it from relations now... ");
        unsigned checkpattern = getPattern(&neuroswTrack, m_unpackedNeuroInputSegmentsName);
        valtrack = isValidPattern(checkpattern);
      }
      if (!valtrack) {
        continue;
      }

      m_neuroSWOutZ->Fill(neuroswTrack.getZ0());
      double cotThSW = neuroswTrack.getCotTheta();
      double cosThSW = copysign(1.0, cotThSW) / sqrt(1. / (cotThSW * cotThSW) + 1);
      m_neuroSWOutCosTheta->Fill(cosThSW);
      phinorm = neuroswTrack.getPhi0() * 180. / M_PI ;
      if (phinorm < 0.) {phinorm = phinorm + 360. ;}
      m_neuroSWOutPhi0->Fill(phinorm);
      m_neuroSWOutInvPt->Fill(neuroswTrack.getPt());

      unsigned simSector =
        neuroswTrack.getRelatedTo<CDCTriggerMLPInput>(m_simNeuroInputVectorName)->getSector();
      m_neuroSWSector->Fill(simSector);

      // sw NN selected sw ts
      unsigned pattern = 0;
      for (const CDCTriggerSegmentHit& hit :
           neuroswTrack.getRelationsTo<CDCTriggerSegmentHit>(m_unpackedNeuroInputSegmentsName)) {
        m_neuroSWSelTSID->Fill(hit.getSegmentID());
        unsigned int sl = hit.getISuperLayer();
        m_neuroSWSelTSCount->Fill(sl);
        float neuroTime = neuroswTrack.getTime();

        // find first occurence of hit (used to debug the selcted TS field)
        CDCTriggerSegmentHit firstHit = hit;
        for (CDCTriggerSegmentHit compare : m_simSegmentHits) {
          if (compare.getISuperLayer()       == hit.getISuperLayer()      &&
              compare.getIWireCenter()       == hit.getIWireCenter()      &&
              compare.getPriorityPosition()  == hit.getPriorityPosition() &&
              compare.getLeftRight()         == hit.getLeftRight()        &&
              compare.priorityTime()         == hit.priorityTime()        &&
              compare.foundTime()            < firstHit.foundTime()) {
            firstHit = compare;
          }
        }

        switch (sl) {
          case 0: m_neuroSWSelTSPrioT_Layer0->Fill(hit.priorityTime());
            m_neuroSWSelTSFoundT_Layer0->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWSelTSPrioB_Layer0->Fill(hit.getPriorityPosition());
            m_neuroSWSelTSLR_Layer0->Fill(hit.getLeftRight());
            break;
          case 1: m_neuroSWSelTSPrioT_Layer1->Fill(hit.priorityTime());
            m_neuroSWSelTSFoundT_Layer1->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWSelTSPrioB_Layer1->Fill(hit.getPriorityPosition());
            m_neuroSWSelTSLR_Layer1->Fill(hit.getLeftRight());
            break;
          case 2: m_neuroSWSelTSPrioT_Layer2->Fill(hit.priorityTime());
            m_neuroSWSelTSFoundT_Layer2->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWSelTSPrioB_Layer2->Fill(hit.getPriorityPosition());
            m_neuroSWSelTSLR_Layer2->Fill(hit.getLeftRight());
            break;
          case 3: m_neuroSWSelTSPrioT_Layer3->Fill(hit.priorityTime());
            m_neuroSWSelTSFoundT_Layer3->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWSelTSPrioB_Layer3->Fill(hit.getPriorityPosition());
            m_neuroSWSelTSLR_Layer3->Fill(hit.getLeftRight());
            break;
          case 4: m_neuroSWSelTSPrioT_Layer4->Fill(hit.priorityTime());
            m_neuroSWSelTSFoundT_Layer4->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWSelTSPrioB_Layer4->Fill(hit.getPriorityPosition());
            m_neuroSWSelTSLR_Layer4->Fill(hit.getLeftRight());
            break;
          case 5: m_neuroSWSelTSPrioT_Layer5->Fill(hit.priorityTime());
            m_neuroSWSelTSFoundT_Layer5->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWSelTSPrioB_Layer5->Fill(hit.getPriorityPosition());
            m_neuroSWSelTSLR_Layer5->Fill(hit.getLeftRight());
            break;
          case 6: m_neuroSWSelTSPrioT_Layer6->Fill(hit.priorityTime());
            m_neuroSWSelTSFoundT_Layer6->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWSelTSPrioB_Layer6->Fill(hit.getPriorityPosition());
            m_neuroSWSelTSLR_Layer6->Fill(hit.getLeftRight());
            break;
          case 7: m_neuroSWSelTSPrioT_Layer7->Fill(hit.priorityTime());
            m_neuroSWSelTSFoundT_Layer7->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWSelTSPrioB_Layer7->Fill(hit.getPriorityPosition());
            m_neuroSWSelTSLR_Layer7->Fill(hit.getLeftRight());
            break;
          case 8: m_neuroSWSelTSPrioT_Layer8->Fill(hit.priorityTime());
            m_neuroSWSelTSFoundT_Layer8->Fill(firstHit.foundTime() - neuroTime);
            m_neuroSWSelTSPrioB_Layer8->Fill(hit.getPriorityPosition());
            m_neuroSWSelTSLR_Layer8->Fill(hit.getLeftRight());
            break;
        }
        if (sl % 2 == 1) pattern |= (1 << ((sl - 1) / 2));
      }
      m_neuroSWOutHitPattern->Fill(pattern);


      // plot input vector
      vector<float> nnInput =
        neuroswTrack.getRelatedTo<CDCTriggerMLPInput>(m_simNeuroInputVectorName)->getInput();
      condFill(m_neuroSWInputID_Layer0    , nnInput[0]);
      condFill(m_neuroSWInputT_Layer0     , nnInput[1]);
      condFill(m_neuroSWInputAlpha_Layer0 , nnInput[2]);
      condFill(m_neuroSWInputID_Layer1    , nnInput[3]);
      condFill(m_neuroSWInputT_Layer1     , nnInput[4]);
      condFill(m_neuroSWInputAlpha_Layer1 , nnInput[5]);
      condFill(m_neuroSWInputID_Layer2    , nnInput[6]);
      condFill(m_neuroSWInputT_Layer2     , nnInput[7]);
      condFill(m_neuroSWInputAlpha_Layer2 , nnInput[8]);
      condFill(m_neuroSWInputID_Layer3    , nnInput[9]);
      condFill(m_neuroSWInputT_Layer3     , nnInput[10]);
      condFill(m_neuroSWInputAlpha_Layer3 , nnInput[11]);
      condFill(m_neuroSWInputID_Layer4    , nnInput[12]);
      condFill(m_neuroSWInputT_Layer4     , nnInput[13]);
      condFill(m_neuroSWInputAlpha_Layer4 , nnInput[14]);
      condFill(m_neuroSWInputID_Layer5    , nnInput[15]);
      condFill(m_neuroSWInputT_Layer5     , nnInput[16]);
      condFill(m_neuroSWInputAlpha_Layer5 , nnInput[17]);
      condFill(m_neuroSWInputID_Layer6    , nnInput[18]);
      condFill(m_neuroSWInputT_Layer6     , nnInput[19]);
      condFill(m_neuroSWInputAlpha_Layer6 , nnInput[20]);
      condFill(m_neuroSWInputID_Layer7    , nnInput[21]);
      condFill(m_neuroSWInputT_Layer7     , nnInput[22]);
      condFill(m_neuroSWInputAlpha_Layer7 , nnInput[23]);
      condFill(m_neuroSWInputID_Layer8    , nnInput[24]);
      condFill(m_neuroSWInputT_Layer8     , nnInput[25]);
      condFill(m_neuroSWInputAlpha_Layer8 , nnInput[26]);
    }
  }

  int nofintracks = 0;
  if (m_unpackedNeuroTracksName != "") {
    int nofouttracks = 0;
    int nofinsegments = 0;
    // fill neurotrigger histograms
    for (CDCTriggerTrack& neuroTrack : m_unpackedNeuroTracks) {
      bool valtrack = false;
      try {
        valtrack = neuroTrack.getValidStereoBit();
      } catch (...) {
        B2WARNING("NeuroTrack doesn't have 'valid bit', get it from relations now... ");
        unsigned checkpattern = getPattern(&neuroTrack, m_unpackedNeuroInputSegmentsName);
        valtrack = isValidPattern(checkpattern);
      }
      if (!valtrack) {
        continue;
      }
      // count number of tracks
      ++nofouttracks;
      // fill raw distributions
      m_neuroHWOutZ->Fill(neuroTrack.getZ0());
      double cotTh = neuroTrack.getCotTheta();
      double cosTh = copysign(1.0, cotTh) / sqrt(1. / (cotTh * cotTh) + 1);
      m_neuroHWOutCosTheta->Fill(cosTh);
      phinorm = neuroTrack.getPhi0() * 180. / M_PI ;
      if (phinorm < 0.) {phinorm = phinorm + 360. ;}
      m_neuroHWOutPhi0->Fill(phinorm);
      m_neuroHWOutInvPt->Fill(neuroTrack.getPt());
      m_neuroHWOutPt->Fill(neuroTrack.getPt());
      m_neuroHWOutm_time->Fill(neuroTrack.getTime());

      unsigned hwSector =
        neuroTrack.getRelatedTo<CDCTriggerMLPInput>(m_unpackedNeuroInputVectorName)->getSector();
      m_neuroHWSector->Fill(hwSector);

      // fill hists per quadrant
      if (!m_limitedoutput) {
        switch (neuroTrack.getQuadrant()) {
          case -1:
            m_neuroHWOutQuad5Z->Fill(neuroTrack.getZ0());
            m_neuroHWOutQuad5CosTheta->Fill(cosTh);
            phinorm = neuroTrack.getPhi0() * 180. / M_PI ;
            if (phinorm < 0.) { phinorm = phinorm + 360. ;}
            m_neuroHWOutQuad5Phi0->Fill(phinorm);
            m_neuroHWOutQuad5InvPt->Fill(neuroTrack.getPt());
            break;
          case 0:
            m_neuroHWOutQuad0Z->Fill(neuroTrack.getZ0());
            m_neuroHWOutQuad0CosTheta->Fill(cosTh);
            phinorm = neuroTrack.getPhi0() * 180. / M_PI ;
            if (phinorm < 0.) { phinorm = phinorm + 360. ;}
            m_neuroHWOutQuad0Phi0->Fill(phinorm);
            m_neuroHWOutQuad0InvPt->Fill(neuroTrack.getPt());
            break;
          case 1:
            m_neuroHWOutQuad1Z->Fill(neuroTrack.getZ0());
            m_neuroHWOutQuad1CosTheta->Fill(cosTh);
            phinorm = neuroTrack.getPhi0() * 180. / M_PI ;
            if (phinorm < 0.) { phinorm = phinorm + 360. ;}
            m_neuroHWOutQuad1Phi0->Fill(phinorm);
            m_neuroHWOutQuad1InvPt->Fill(neuroTrack.getPt());
            break;
          case 2:
            m_neuroHWOutQuad2Z->Fill(neuroTrack.getZ0());
            m_neuroHWOutQuad2CosTheta->Fill(cosTh);
            phinorm = neuroTrack.getPhi0() * 180. / M_PI ;
            if (phinorm < 0.) { phinorm = phinorm + 360. ;}
            m_neuroHWOutQuad2Phi0->Fill(phinorm);
            m_neuroHWOutQuad2InvPt->Fill(neuroTrack.getPt());
            break;
          case 3:
            m_neuroHWOutQuad3Z->Fill(neuroTrack.getZ0());
            m_neuroHWOutQuad3CosTheta->Fill(cosTh);
            phinorm = neuroTrack.getPhi0() * 180. / M_PI ;
            if (phinorm < 0.) { phinorm = phinorm + 360. ;}
            m_neuroHWOutQuad3Phi0->Fill(phinorm);
            m_neuroHWOutQuad3InvPt->Fill(neuroTrack.getPt());
            break;
        }
      }

      // get related stereo hits
      unsigned pattern = 0;
      for (const CDCTriggerSegmentHit& hit :
           neuroTrack.getRelationsTo<CDCTriggerSegmentHit>(m_unpackedNeuroInputSegmentsName)) {
        m_neuroHWSelTSID->Fill(hit.getSegmentID());
        unsigned int sl = hit.getISuperLayer();
        m_neuroHWSelTSCount->Fill(sl);
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
          }
        }

        if (!m_limitedoutput) {
          switch (sl) {
            case 0: m_neuroHWSelTSPrioT_Layer0->Fill(hit.priorityTime());
              m_neuroHWSelTSFoundT_Layer0->Fill(firstHit.foundTime() - neuroTime);
              m_neuroHWSelTSPrioB_Layer0->Fill(hit.getPriorityPosition());
              m_neuroHWSelTSLR_Layer0->Fill(hit.getLeftRight());
              break;
            case 1: m_neuroHWSelTSPrioT_Layer1->Fill(hit.priorityTime());
              m_neuroHWSelTSFoundT_Layer1->Fill(firstHit.foundTime() - neuroTime);
              m_neuroHWSelTSPrioB_Layer1->Fill(hit.getPriorityPosition());
              m_neuroHWSelTSLR_Layer1->Fill(hit.getLeftRight());
              break;
            case 2: m_neuroHWSelTSPrioT_Layer2->Fill(hit.priorityTime());
              m_neuroHWSelTSFoundT_Layer2->Fill(firstHit.foundTime() - neuroTime);
              m_neuroHWSelTSPrioB_Layer2->Fill(hit.getPriorityPosition());
              m_neuroHWSelTSLR_Layer2->Fill(hit.getLeftRight());
              break;
            case 3: m_neuroHWSelTSPrioT_Layer3->Fill(hit.priorityTime());
              m_neuroHWSelTSFoundT_Layer3->Fill(firstHit.foundTime() - neuroTime);
              m_neuroHWSelTSPrioB_Layer3->Fill(hit.getPriorityPosition());
              m_neuroHWSelTSLR_Layer3->Fill(hit.getLeftRight());
              break;
            case 4: m_neuroHWSelTSPrioT_Layer4->Fill(hit.priorityTime());
              m_neuroHWSelTSFoundT_Layer4->Fill(firstHit.foundTime() - neuroTime);
              m_neuroHWSelTSPrioB_Layer4->Fill(hit.getPriorityPosition());
              m_neuroHWSelTSLR_Layer4->Fill(hit.getLeftRight());
              break;
            case 5: m_neuroHWSelTSPrioT_Layer5->Fill(hit.priorityTime());
              m_neuroHWSelTSFoundT_Layer5->Fill(firstHit.foundTime() - neuroTime);
              m_neuroHWSelTSPrioB_Layer5->Fill(hit.getPriorityPosition());
              m_neuroHWSelTSLR_Layer5->Fill(hit.getLeftRight());
              break;
            case 6: m_neuroHWSelTSPrioT_Layer6->Fill(hit.priorityTime());
              m_neuroHWSelTSFoundT_Layer6->Fill(firstHit.foundTime() - neuroTime);
              m_neuroHWSelTSPrioB_Layer6->Fill(hit.getPriorityPosition());
              m_neuroHWSelTSLR_Layer6->Fill(hit.getLeftRight());
              break;
            case 7: m_neuroHWSelTSPrioT_Layer7->Fill(hit.priorityTime());
              m_neuroHWSelTSFoundT_Layer7->Fill(firstHit.foundTime() - neuroTime);
              m_neuroHWSelTSPrioB_Layer7->Fill(hit.getPriorityPosition());
              m_neuroHWSelTSLR_Layer7->Fill(hit.getLeftRight());
              break;
            case 8: m_neuroHWSelTSPrioT_Layer8->Fill(hit.priorityTime());
              m_neuroHWSelTSFoundT_Layer8->Fill(firstHit.foundTime() - neuroTime);
              m_neuroHWSelTSPrioB_Layer8->Fill(hit.getPriorityPosition());
              m_neuroHWSelTSLR_Layer8->Fill(hit.getLeftRight());
              break;
          }
        }
        if (sl % 2 == 1) pattern |= (1 << ((sl - 1) / 2));
      }
      m_neuroHWOutHitPattern->Fill(pattern);

      // plot input vector
      if (!m_limitedoutput) {
        vector<float> nnInput =
          neuroTrack.getRelatedTo<CDCTriggerMLPInput>(m_unpackedNeuroInputVectorName)->getInput();
        condFill(m_neuroHWInputID_Layer0    , nnInput[0]);
        condFill(m_neuroHWInputT_Layer0     , nnInput[1]);
        condFill(m_neuroHWInputAlpha_Layer0 , nnInput[2]);
        condFill(m_neuroHWInputID_Layer1    , nnInput[3]);
        condFill(m_neuroHWInputT_Layer1     , nnInput[4]);
        condFill(m_neuroHWInputAlpha_Layer1 , nnInput[5]);
        condFill(m_neuroHWInputID_Layer2    , nnInput[6]);
        condFill(m_neuroHWInputT_Layer2     , nnInput[7]);
        condFill(m_neuroHWInputAlpha_Layer2 , nnInput[8]);
        condFill(m_neuroHWInputID_Layer3    , nnInput[9]);
        condFill(m_neuroHWInputT_Layer3     , nnInput[10]);
        condFill(m_neuroHWInputAlpha_Layer3 , nnInput[11]);
        condFill(m_neuroHWInputID_Layer4    , nnInput[12]);
        condFill(m_neuroHWInputT_Layer4     , nnInput[13]);
        condFill(m_neuroHWInputAlpha_Layer4 , nnInput[14]);
        condFill(m_neuroHWInputID_Layer5    , nnInput[15]);
        condFill(m_neuroHWInputT_Layer5     , nnInput[16]);
        condFill(m_neuroHWInputAlpha_Layer5 , nnInput[17]);
        condFill(m_neuroHWInputID_Layer6    , nnInput[18]);
        condFill(m_neuroHWInputT_Layer6     , nnInput[19]);
        condFill(m_neuroHWInputAlpha_Layer6 , nnInput[20]);
        condFill(m_neuroHWInputID_Layer7    , nnInput[21]);
        condFill(m_neuroHWInputT_Layer7     , nnInput[22]);
        condFill(m_neuroHWInputAlpha_Layer7 , nnInput[23]);
        condFill(m_neuroHWInputID_Layer8    , nnInput[24]);
        condFill(m_neuroHWInputT_Layer8     , nnInput[25]);
        condFill(m_neuroHWInputAlpha_Layer8 , nnInput[26]);


        if (m_simNeuroTracksName != "") {
          // get related track from TSIM (via 2D finder track)
          CDCTriggerTrack* finderTrack =
            neuroTrack.getRelatedFrom<CDCTriggerTrack>(m_unpackedNeuroInput2DTracksName);
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


              if (nsameTS >= m_nsamets) {
                if (abs(neuroTrack.getZ0() - neuroSimTrack->getZ0()) > 1) {
                  neuroSimTrack->setQualityVector(2);
                }
                m_neuroDeltaZ->Fill(neuroTrack.getZ0() - neuroSimTrack->getZ0());
                double nnHWtheta = neuroTrack.getDirection().Theta() * 180. / M_PI;
                double nnSWtheta = neuroSimTrack->getDirection().Theta() * 180. / M_PI;
                m_neuroDeltaTheta->Fill(nnHWtheta - nnSWtheta);
                m_neuroScatterZ->Fill(neuroTrack.getZ0(), neuroSimTrack->getZ0());
                m_neuroScatterTheta->Fill(nnHWtheta, nnSWtheta);
                vector<float> unpackedInput =
                  neuroTrack.getRelatedTo<CDCTriggerMLPInput>(m_unpackedNeuroInputVectorName)->getInput();
                vector<float> simInput =
                  neuroSimTrack->getRelatedTo<CDCTriggerMLPInput>(m_simNeuroInputVectorName)->getInput();
                unsigned unpackedSector =
                  neuroTrack.getRelatedTo<CDCTriggerMLPInput>(m_unpackedNeuroInputVectorName)->getSector();
                unsigned simSector =
                  neuroSimTrack->getRelatedTo<CDCTriggerMLPInput>(m_simNeuroInputVectorName)->getSector();
                m_neuroDeltaSector->Fill(unpackedSector - simSector);

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
    }
    for (CDCTriggerTrack& neuroinput2dtrack : m_unpackedNeuroInput2DTracks) {
      nofintracks ++;
      phinorm = neuroinput2dtrack.getPhi0() * 180. / M_PI ;
      if (phinorm < 0.) { phinorm = phinorm + 360. ;}
      m_neuroHWInPhi0->Fill(phinorm);
      m_neuroHWInm_time->Fill(neuroinput2dtrack.getTime());
      m_neuroHWInInvPt->Fill(neuroinput2dtrack.getPt());
    }
    if (nofintracks > 0 || nofouttracks > 0) {
      m_neuroHWInTrackCount->Fill(nofintracks);
      m_neuroHWOutTrackCount->Fill(nofouttracks);
      m_neuroHWOutVsInTrackCount->Fill((nofouttracks - nofintracks));
    }
    for (CDCTriggerSegmentHit& neuroinputsegment : m_unpackedNeuroInputSegments) {
      ++nofinsegments;
      m_neuroHWInTSID->Fill(neuroinputsegment.getSegmentID());
      if (!m_limitedoutput) {
        unsigned int sl = neuroinputsegment.getISuperLayer();
        switch (sl) {
          case 0: m_neuroHWInTSPrioT_Layer0->Fill(neuroinputsegment.priorityTime());
            m_neuroHWInTSFoundT_Layer0->Fill(neuroinputsegment.foundTime());
            m_neuroHWInTSPrioB_Layer0->Fill(neuroinputsegment.getPriorityPosition());
            m_neuroHWInTSLR_Layer0->Fill(neuroinputsegment.getLeftRight());
            break;
          case 1: m_neuroHWInTSPrioT_Layer1->Fill(neuroinputsegment.priorityTime());
            m_neuroHWInTSFoundT_Layer1->Fill(neuroinputsegment.foundTime());
            m_neuroHWInTSPrioB_Layer1->Fill(neuroinputsegment.getPriorityPosition());
            m_neuroHWInTSLR_Layer1->Fill(neuroinputsegment.getLeftRight());
            break;
          case 2: m_neuroHWInTSPrioT_Layer2->Fill(neuroinputsegment.priorityTime());
            m_neuroHWInTSFoundT_Layer2->Fill(neuroinputsegment.foundTime());
            m_neuroHWInTSPrioB_Layer2->Fill(neuroinputsegment.getPriorityPosition());
            m_neuroHWInTSLR_Layer2->Fill(neuroinputsegment.getLeftRight());
            break;
          case 3: m_neuroHWInTSPrioT_Layer3->Fill(neuroinputsegment.priorityTime());
            m_neuroHWInTSFoundT_Layer3->Fill(neuroinputsegment.foundTime());
            m_neuroHWInTSPrioB_Layer3->Fill(neuroinputsegment.getPriorityPosition());
            m_neuroHWInTSLR_Layer3->Fill(neuroinputsegment.getLeftRight());
            break;
          case 4: m_neuroHWInTSPrioT_Layer4->Fill(neuroinputsegment.priorityTime());
            m_neuroHWInTSFoundT_Layer4->Fill(neuroinputsegment.foundTime());
            m_neuroHWInTSPrioB_Layer4->Fill(neuroinputsegment.getPriorityPosition());
            m_neuroHWInTSLR_Layer4->Fill(neuroinputsegment.getLeftRight());
            break;
          case 5: m_neuroHWInTSPrioT_Layer5->Fill(neuroinputsegment.priorityTime());
            m_neuroHWInTSFoundT_Layer5->Fill(neuroinputsegment.foundTime());
            m_neuroHWInTSPrioB_Layer5->Fill(neuroinputsegment.getPriorityPosition());
            m_neuroHWInTSLR_Layer5->Fill(neuroinputsegment.getLeftRight());
            break;
          case 6: m_neuroHWInTSPrioT_Layer6->Fill(neuroinputsegment.priorityTime());
            m_neuroHWInTSFoundT_Layer6->Fill(neuroinputsegment.foundTime());
            m_neuroHWInTSPrioB_Layer6->Fill(neuroinputsegment.getPriorityPosition());
            m_neuroHWInTSLR_Layer6->Fill(neuroinputsegment.getLeftRight());
            break;
          case 7: m_neuroHWInTSPrioT_Layer7->Fill(neuroinputsegment.priorityTime());
            m_neuroHWInTSFoundT_Layer7->Fill(neuroinputsegment.foundTime());
            m_neuroHWInTSPrioB_Layer7->Fill(neuroinputsegment.getPriorityPosition());
            m_neuroHWInTSLR_Layer7->Fill(neuroinputsegment.getLeftRight());
            break;
          case 8: m_neuroHWInTSPrioT_Layer8->Fill(neuroinputsegment.priorityTime());
            m_neuroHWInTSFoundT_Layer8->Fill(neuroinputsegment.foundTime());
            m_neuroHWInTSPrioB_Layer8->Fill(neuroinputsegment.getPriorityPosition());
            m_neuroHWInTSLR_Layer8->Fill(neuroinputsegment.getLeftRight());
            break;
        }
      }
    }
    if (nofinsegments > 0) {
      m_neuroHWInTSCount->Fill(nofinsegments);
    }
  }


  if (!m_limitedoutput && m_simSegmentHitsName != "" && m_sim2DTracksSWTSName != "") {
    m_2DSWOutTrackCount->Fill(m_sim2DTracksSWTS.getEntries());
    for (CDCTriggerTrack& sim2dtrack : m_sim2DTracksSWTS) {
      phinorm = sim2dtrack.getPhi0() * 180. / M_PI ;
      if (phinorm < 0.) { phinorm = phinorm + 360. ;}
      m_2DSWOutPhi0->Fill(phinorm);
      m_2DSWOutm_time->Fill(sim2dtrack.getTime());
      m_2DSWOutInvPt->Fill(sim2dtrack.getPt());
    }
  }
  if (m_unpacked2DTracksName != "") {
    int nof2douttracks = 0;
    for (CDCTriggerTrack& finder2dtrack : m_unpacked2DTracks) {
      ++nof2douttracks;
      phinorm = finder2dtrack.getPhi0() * 180. / M_PI ;
      if (phinorm < 0.) {phinorm = phinorm + 360. ;}
      m_2DHWOutPhi0->Fill(phinorm);
      m_2DHWOutm_time->Fill(finder2dtrack.getTime());
      m_2DHWOutInvPt->Fill(finder2dtrack.getPt());
    }
    if (nof2douttracks > 0) {
      m_2DHWOutTrackCount->Fill(nof2douttracks);
      if (m_unpackedNeuroTracksName != "") {
        m_neuroHWInVs2DOutTrackCount->Fill((nofintracks - nof2douttracks));
      }
    }

    int nof2dinsegments = 0;

    for (CDCTriggerSegmentHit& hit : m_unpackedSegmentHits) {
      ++nof2dinsegments;
      if (!m_limitedoutput) {
        unsigned int sl = hit.getISuperLayer();
        switch (sl) {
          case 0: m_2DHWInTSPrioT_Layer0->Fill(hit.priorityTime());
            m_2DHWInTSFoundT_Layer0->Fill(hit.foundTime());
            m_2DHWInTSPrioB_Layer0->Fill(hit.getPriorityPosition());
            m_2DHWInTSLR_Layer0->Fill(hit.getLeftRight());
            break;
          case 2: m_2DHWInTSPrioT_Layer2->Fill(hit.priorityTime());
            m_2DHWInTSFoundT_Layer2->Fill(hit.foundTime());
            m_2DHWInTSPrioB_Layer2->Fill(hit.getPriorityPosition());
            m_2DHWInTSLR_Layer2->Fill(hit.getLeftRight());
            break;
          case 4: m_2DHWInTSPrioT_Layer4->Fill(hit.priorityTime());
            m_2DHWInTSFoundT_Layer4->Fill(hit.foundTime());
            m_2DHWInTSPrioB_Layer4->Fill(hit.getPriorityPosition());
            m_2DHWInTSLR_Layer4->Fill(hit.getLeftRight());
            break;
          case 6: m_2DHWInTSPrioT_Layer6->Fill(hit.priorityTime());
            m_2DHWInTSFoundT_Layer6->Fill(hit.foundTime());
            m_2DHWInTSPrioB_Layer6->Fill(hit.getPriorityPosition());
            m_2DHWInTSLR_Layer6->Fill(hit.getLeftRight());
            break;
          case 8: m_2DHWInTSPrioT_Layer8->Fill(hit.priorityTime());
            m_2DHWInTSFoundT_Layer8->Fill(hit.foundTime());
            m_2DHWInTSPrioB_Layer8->Fill(hit.getPriorityPosition());
            m_2DHWInTSLR_Layer8->Fill(hit.getLeftRight());
            break;
        }
      }
    }
    if (nof2dinsegments > 0) {
    }
  }


  // sw TS
  if (!m_limitedoutput && m_simNeuroTracksSWTSSW2DName != "") {
    m_neuroSWTSSW2DInTSCount->Fill(m_simSegmentHits.getEntries());
    for (CDCTriggerSegmentHit& hit : m_simSegmentHits) {
      m_neuroSWTSSW2DInTSID->Fill(hit.getSegmentID());
      unsigned int sl = hit.getISuperLayer();
      switch (sl) {
        case 0: m_neuroSWTSSW2DInTSPrioT_Layer0->Fill(hit.priorityTime());
          m_neuroSWTSSW2DInTSFoundT_Layer0->Fill(hit.foundTime());
          m_neuroSWTSSW2DInTSPrioB_Layer0->Fill(hit.getPriorityPosition());
          m_neuroSWTSSW2DInTSLR_Layer0->Fill(hit.getLeftRight());
          break;
        case 1: m_neuroSWTSSW2DInTSPrioT_Layer1->Fill(hit.priorityTime());
          m_neuroSWTSSW2DInTSFoundT_Layer1->Fill(hit.foundTime());
          m_neuroSWTSSW2DInTSPrioB_Layer1->Fill(hit.getPriorityPosition());
          m_neuroSWTSSW2DInTSLR_Layer1->Fill(hit.getLeftRight());
          break;
        case 2: m_neuroSWTSSW2DInTSPrioT_Layer2->Fill(hit.priorityTime());
          m_neuroSWTSSW2DInTSFoundT_Layer2->Fill(hit.foundTime());
          m_neuroSWTSSW2DInTSPrioB_Layer2->Fill(hit.getPriorityPosition());
          m_neuroSWTSSW2DInTSLR_Layer2->Fill(hit.getLeftRight());
          break;
        case 3: m_neuroSWTSSW2DInTSPrioT_Layer3->Fill(hit.priorityTime());
          m_neuroSWTSSW2DInTSFoundT_Layer3->Fill(hit.foundTime());
          m_neuroSWTSSW2DInTSPrioB_Layer3->Fill(hit.getPriorityPosition());
          m_neuroSWTSSW2DInTSLR_Layer3->Fill(hit.getLeftRight());
          break;
        case 4: m_neuroSWTSSW2DInTSPrioT_Layer4->Fill(hit.priorityTime());
          m_neuroSWTSSW2DInTSFoundT_Layer4->Fill(hit.foundTime());
          m_neuroSWTSSW2DInTSPrioB_Layer4->Fill(hit.getPriorityPosition());
          m_neuroSWTSSW2DInTSLR_Layer4->Fill(hit.getLeftRight());
          break;
        case 5: m_neuroSWTSSW2DInTSPrioT_Layer5->Fill(hit.priorityTime());
          m_neuroSWTSSW2DInTSFoundT_Layer5->Fill(hit.foundTime());
          m_neuroSWTSSW2DInTSPrioB_Layer5->Fill(hit.getPriorityPosition());
          m_neuroSWTSSW2DInTSLR_Layer5->Fill(hit.getLeftRight());
          break;
        case 6: m_neuroSWTSSW2DInTSPrioT_Layer6->Fill(hit.priorityTime());
          m_neuroSWTSSW2DInTSFoundT_Layer6->Fill(hit.foundTime());
          m_neuroSWTSSW2DInTSPrioB_Layer6->Fill(hit.getPriorityPosition());
          m_neuroSWTSSW2DInTSLR_Layer6->Fill(hit.getLeftRight());
          break;
        case 7: m_neuroSWTSSW2DInTSPrioT_Layer7->Fill(hit.priorityTime());
          m_neuroSWTSSW2DInTSFoundT_Layer7->Fill(hit.foundTime());
          m_neuroSWTSSW2DInTSPrioB_Layer7->Fill(hit.getPriorityPosition());
          m_neuroSWTSSW2DInTSLR_Layer7->Fill(hit.getLeftRight());
          break;
        case 8: m_neuroSWTSSW2DInTSPrioT_Layer8->Fill(hit.priorityTime());
          m_neuroSWTSSW2DInTSFoundT_Layer8->Fill(hit.foundTime());
          m_neuroSWTSSW2DInTSPrioB_Layer8->Fill(hit.getPriorityPosition());
          m_neuroSWTSSW2DInTSLR_Layer8->Fill(hit.getLeftRight());
          break;
      }
    }
  }

  static constexpr std::array<int, 9> nWiresInSuperLayer = {
    160, 160, 192, 224, 256, 288, 320, 352, 384
  };
  unsigned axhwts = 0;
  unsigned sthwts = 0;
  unsigned axswts = 0;
  unsigned stswts = 0;

  std::vector<TSLine> hwtsoutput;
  std::vector<TSLine> swtsoutput;
  if (m_unpackedSegmentHitsName != "") {
    for (const CDCTriggerSegmentHit& xhit : m_unpackedNeuroInputSegments) {
      if (xhit.getISuperLayer() % 2 == 0) {
        axhwts ++;
      } else {
        sthwts ++;
      }
      int iSL = xhit.getISuperLayer();
      int iTS = xhit.getIWire();
      int nwires = nWiresInSuperLayer[ iSL ];
      if (iSL == 8) {
        iTS += 16;
        if (iTS > nwires) {
          iTS -= nwires;
        }
      }
      int tsIDInTracker = iTS - nwires * xhit.getQuadrant() / 4;
      if (tsIDInTracker < 0) {
        tsIDInTracker += nwires;
      }
      TSLine l(xhit);
      l.strline = "("
                  + padto(std::to_string(xhit.getISuperLayer()), 2) + ", "
                  + padto(std::to_string(xhit.getQuadrant()), 1) + ", "
                  + padto(std::to_string(xhit.getSegmentID()), 4) + ", "
                  + padto(std::to_string(xhit.getIWire()), 5) + ", "
                  + padto(std::to_string(xhit.getPriorityPosition()), 2) + ", "
                  + padto(std::to_string(xhit.getLeftRight()), 2) + ", "
                  + padto(std::to_string(xhit.priorityTime()), 4) + ", "
                  + padto(std::to_string(xhit.foundTime()), 3) + ", "
                  + padto(std::to_string(tsIDInTracker), 4) + ") | ";

      if (m_unpackedNeuroTracksName != "") {
        unsigned count2d = 0;
        for (CDCTriggerTrack& track : m_unpackedNeuroInput2DTracks) {
          count2d++;
          if (have_relation(track, xhit, m_unpackedNeuroInputSegmentsName)) {
            l.strline += std::to_string(count2d);
          } else {
            l.strline += ".";
          }
        }
        l.strline += " | ";
        unsigned counthwn = 0;
        for (CDCTriggerTrack& track : m_unpackedNeuroTracks) {
          counthwn++;
          if (have_relation(track, xhit, m_unpackedNeuroInputSegmentsName)) {
            if (track.getValidStereoBit()) {
              l.strline += std::to_string(counthwn);
            } else {
              l.strline += "x";
            }
          } else {
            l.strline += ".";
          }
        }
        l.strline += " | ";
      }
      if (m_simNeuroTracksName != "") {
        unsigned countswn = 0;
        for (CDCTriggerTrack& track : m_simNeuroTracks) {
          countswn++;
          if (have_relation(track, xhit, m_unpackedNeuroInputSegmentsName)) {
            l.strline += std::to_string(countswn);
          } else {
            l.strline += ".";
          }
        }
        l.strline += " | ";
      }
      if (m_recoTracksName != "") {
        unsigned countreco = 0;
        for (const auto& track : m_RecoTracks) {
          countreco++;
          bool related = false;
          for (const CDCTriggerSegmentHit& ts : track.getRelationsTo<CDCTriggerSegmentHit>(m_unpackedNeuroInputSegmentsName)) {
            if (&ts == &xhit) {related = true;}
          }
          if (related) {
            l.strline += std::to_string(countreco);
          } else {
            l.strline += ".";
          }
        }
        l.strline += " | ";
      }
      if (m_unpackedNeuroTracksName != "") {
        sorted_insert(hwtsoutput, l, m_unpackedNeuroInputSegmentsName, m_unpackedNeuroInput2DTracksName, m_unpackedNeuroTracksName);
      } else {
        hwtsoutput.push_back(l);
      }
    }
  }
  std::string axhw = std::to_string(axhwts) + " / " + std::to_string(sthwts);
  if (!m_limitedoutput && m_simSegmentHitsName != "") {
    for (const CDCTriggerSegmentHit& xhit : m_simSegmentHits) {
      if (xhit.getISuperLayer() % 2 == 0) {
        axswts ++;
      } else {
        stswts ++;
      }
      int iSL = xhit.getISuperLayer();
      int iTS = xhit.getIWire();
      int nwires = nWiresInSuperLayer[ iSL ];
      if (iSL == 8) {
        iTS += 16;
        if (iTS > nwires) {
          iTS -= nwires;
        }
      }
      int tsIDInTracker = iTS - nwires * xhit.getQuadrant() / 4;
      if (tsIDInTracker < 0) {
        tsIDInTracker += nwires;
      }
      TSLine l(xhit);
      l.strline = "("
                  + padto(std::to_string(xhit.getISuperLayer()), 2) + ", "
                  + padto(std::to_string(xhit.getSegmentID()), 4) + ", "
                  + padto(std::to_string(xhit.getIWire()), 5) + ", "
                  + padto(std::to_string(xhit.getPriorityPosition()), 2) + ", "
                  + padto(std::to_string(xhit.getLeftRight()), 2) + ", "
                  + padto(std::to_string(xhit.priorityTime()), 4) + ", "
                  + padto(std::to_string(xhit.foundTime()), 3) + ", "
                  + padto(std::to_string(tsIDInTracker), 4) + ") ";

      if (m_sim2DTracksSWTSName != "") {
        unsigned count2d = 0;
        for (CDCTriggerTrack& track : m_sim2DTracksSWTS) {
          count2d++;
          if (have_relation(track, xhit, m_simSegmentHitsName)) {
            l.strline += std::to_string(count2d);
          } else {
            l.strline += ".";
          }
        }
        l.strline += " | ";
      }
      if (m_simNeuroTracksSWTSSW2DName != "") {
        unsigned countswn = 0;
        for (CDCTriggerTrack& track : m_simNeuroTracksSWTSSW2D) {
          countswn++;
          if (have_relation(track, xhit, m_simSegmentHitsName)) {
            l.strline += std::to_string(countswn);
          } else {
            l.strline += ".";
          }
        }
        l.strline += " | ";
      }
      if (m_recoTracksName != "") {
        unsigned countreco = 0;
        for (const auto& track : m_RecoTracks) {
          countreco++;
          bool related = false;
          for (const CDCTriggerSegmentHit& ts : track.getRelationsTo<CDCTriggerSegmentHit>(m_simSegmentHitsName)) {
            if (&ts == &xhit) {related = true;}
          }
          if (related) {
            l.strline += std::to_string(countreco);
          } else {
            l.strline += ".";
          }
        }
        l.strline += " | ";
      }
      if (m_simNeuroTracksSWTSSW2DName != "") {
        sorted_insert(swtsoutput, l, m_simSegmentHitsName, m_sim2DTracksSWTSName, m_simNeuroTracksSWTSSW2DName);
      } else {
        swtsoutput.push_back(l);
      }
    }
  }
  std::string hwtsstring = "(SL, Q, SID , WID  , PP, LR, pT , fT , TSID) | HWNNIn2D | HWNeuro | SWNeuro | ";
  std::string swtsstring = "(SL, Q, SID , WID  , PP, LR, pT , fT , TSID) | SW2D | SWNeuro | ";
  if (m_recoTracksName != "") {
    hwtsstring += "Reco |  ";
    swtsstring += "Reco |  ";
  }
  B2DEBUG(10, padright(" ", 100));
  B2DEBUG(10, "----------------------------------------------------------------------------------------------------");
  B2DEBUG(10, padright(" ", 100));
  std::string experimentstring = "Experiment " + std::to_string(eventMetaData->getExperiment()) + "  Run " +
                                 std::to_string(eventMetaData->getRun()) + "  Event " + std::to_string(eventMetaData->getEvent());
  B2DEBUG(10, padright(experimentstring, 100));
  B2DEBUG(10, padright(" ", 100));
  B2DEBUG(10, padright(" ", 100));
  if (m_unpackedNeuroTracksName != "" && m_unpacked2DTracksName != "") {
    B2DEBUG(10,  padright("Number of NN HW TS (Axial/Stereo): ", 40) << padright(axhw, 60));
    B2DEBUG(10,  padright("Number of HW 2DFinderTracks: ", 40)            << padright(std::to_string(m_unpacked2DTracks.getEntries()),
            60));
    unsigned f2dtrn = 0;
    for (CDCTriggerTrack& ltrack : m_unpacked2DTracks) {
      f2dtrn++;
      std::stringstream strpt;
      std::stringstream stromega;
      std::stringstream strphi;
      strpt       << std::fixed << std::setprecision(2)   << ltrack.getPt();
      stromega    << std::fixed << std::setprecision(2)   << ltrack.getOmega();
      strphi      << std::fixed << std::setprecision(2)   << (ltrack.getPhi0() * 180. / M_PI);
      std::string trs  = "    HW2DFinderTrack Nr. " + std::to_string(f2dtrn) + " (pt, omega, phi) = ";
      trs += padto(strpt.str(), 6) + ", " + padto(stromega.str(), 6) + ", " + padto(strphi.str(), 6) + ")";
      B2DEBUG(15, padright(trs, 100));
    }
    B2DEBUG(10,  padright("Number of HW NNInput2DFinderTracks: ",
                          40)     << padright(std::to_string(m_unpackedNeuroInput2DTracks.getEntries()), 60));
    unsigned n2dtrn = 0;
    for (CDCTriggerTrack& ltrack : m_unpackedNeuroInput2DTracks) {
      n2dtrn++;
      std::stringstream strpt;
      std::stringstream stromega;
      std::stringstream strphi;
      strpt       << std::fixed << std::setprecision(2)   << ltrack.getPt();
      stromega    << std::fixed << std::setprecision(2)   << ltrack.getOmega();
      strphi      << std::fixed << std::setprecision(2)   << (ltrack.getPhi0() * 180. / M_PI);
      std::string trs  = "    HWNeuroInput2DTrack Nr. " + std::to_string(n2dtrn) + " (pt, omega, phi) = ";
      trs += padto(strpt.str(), 6) + ", " + padto(stromega.str(), 6) + ", " + padto(strphi.str(), 6) + ", " + ")";
      B2DEBUG(15, padright(trs, 100));
    }
    B2DEBUG(10,  padright("Number of HW NeuroTracks: ",
                          40)               << padright(std::to_string(m_unpackedNeuroTracks.getEntries()), 60));
    unsigned hwntrn = 0;
    for (CDCTriggerTrack& ltrack : m_unpackedNeuroTracks) {
      hwntrn++;
      std::stringstream strpt;
      std::stringstream stromega;
      std::stringstream strphi;
      std::stringstream strtheta;
      std::stringstream strz;
      strpt       << std::fixed << std::setprecision(2)   << ltrack.getPt();
      stromega    << std::fixed << std::setprecision(2)   << ltrack.getOmega();
      strphi      << std::fixed << std::setprecision(2)   << (ltrack.getPhi0() * 180. / M_PI);
      strtheta    << std::fixed << std::setprecision(2)   << (ltrack.getDirection().Theta() * 180. / M_PI);
      strz        << std::fixed << std::setprecision(2)   << ltrack.getZ0();
      std::string trs  = "    HWNeuroTrack Nr. " + std::to_string(hwntrn) + " (pt, omega, phi, theta, z) = ";
      trs += padto(strpt.str(), 6) + ", " + padto(stromega.str(), 6) + ", " + padto(strphi.str(), 6) + ", " + padto(strtheta.str(),
             6) + ", " + padto(strz.str(), 6) + ")";
      B2DEBUG(15, padright(trs, 100));
      std::string infostr = "      Found old track: ( ";
      for (bool x : ltrack.getFoundOldTrack()) {
        infostr += std::to_string(x) + " ";
      }
      infostr += "), ";
      infostr = padright(infostr, 50);
      infostr += "Drift threshold: ( ";
      for (bool x : ltrack.getDriftThreshold()) {
        infostr += std::to_string(x) + " ";
      }
      infostr += ")";
      infostr += (ltrack.getValidStereoBit()) ? " valid" : " NOT valid";
      B2DEBUG(15, padright(infostr, 100));
      std::string info2str = "      Expert Network Number: " + std::to_string(ltrack.getExpert());
      info2str += ", TSVector: (";
      for (bool x : ltrack.getTSVector()) {
        info2str += std::to_string(x) + " ";
      }
      info2str += ")";
      info2str += ", Q=";
      info2str += std::to_string(ltrack.getQualityVector());
      B2DEBUG(15, padright(info2str, 100));
      CDCTriggerTrack* ftrack = ltrack.getRelatedFrom<CDCTriggerTrack>(m_unpackedNeuroInput2DTracksName);
      CDCTriggerTrack* strack = ftrack->getRelatedTo<CDCTriggerTrack>(m_simNeuroTracksName);
      if (strack) {
        vector<float> unpackedInput =
          ltrack.getRelatedTo<CDCTriggerMLPInput>(m_unpackedNeuroInputVectorName)->getInput();
        // vector<float> simInput = ltrack.getRelatedTo<CDCTriggerMLPInput>(m_unpackedNeuroInputVectorName)->getInput();
        vector<float> simInput =
          strack->getRelatedTo<CDCTriggerMLPInput>(m_simNeuroInputVectorName)->getInput();
        B2DEBUG(20, padright("      Input Vector unpacked (id, t, alpha), sim (id, t, alpha), delta (id, t, alpha):", 100));
        for (unsigned ii = 0; ii < unpackedInput.size(); ii += 3) {
          std::string lla = "      " + std::to_string(ii / 3) + ")";
          std::string llb = "      " + std::to_string(ii / 3) + ")";
          lla += "(" + padright(std::to_string(unpackedInput[ii]), 8) + " " + padright(std::to_string(unpackedInput[ii + 1]),
                 8) + " " + padright(std::to_string(unpackedInput[ii + 2]), 8) + "),(" + padright(std::to_string(simInput[ii]),
                     8) + " " + padright(std::to_string(simInput[ii + 1]), 8) + " " + padright(std::to_string(simInput[ii + 2]),
                         8) + "),(" + padright(std::to_string(unpackedInput[ii] - simInput[ii]),
                                               8) + " " + padright(std::to_string(unpackedInput[ii + 1] - simInput[ii + 1]),
                                                                   8) + " " + padright(std::to_string(unpackedInput[ii + 2] - simInput[ii + 2]), 8) + ")";
          llb += "  (" + padright(std::to_string(int(unpackedInput[ii] * 4096)),
                                  8) + " " + padright(std::to_string(int(unpackedInput[ii + 1] * 4096)),
                                                      8) + " " + padright(std::to_string(int(unpackedInput[ii + 2] * 4096)),
                                                          8) + "),(" + padright(std::to_string(int(simInput[ii] * 4096)), 8) + " " + padright(std::to_string(int(simInput[ii + 1] * 4096)),
                                                              8) + " " + padright(std::to_string(int(simInput[ii + 2] * 4096)),
                                                                  8) + "),(" + padright(std::to_string(int(unpackedInput[ii] * 4096 - simInput[ii] * 4096)),
                                                                      8) + " " + padright(std::to_string(int(unpackedInput[ii + 1] * 4096 - simInput[ii + 1] * 4096)),
                                                                          8) + " " + padright(std::to_string(int(unpackedInput[ii + 2] * 4096 - simInput[ii + 2] * 4096)), 8) + ")";

          //  cout << hex;
          //  cout.setf(ios::showbase);
          //  cout << "   (" << setw(11) << (int)(unpackedInput[ii]  * 4096) << ", "                       << setw(11) << (int)(
          //         unpackedInput[ii + 1] * 4096) << ", "                            << setw(11) << (int)(unpackedInput[ii + 2] * 4096) << "), ";
          //  cout << "(" << setw(11) << (int)(simInput[ii]       * 4096) << ", "                       << setw(11) << (int)(
          //         simInput[ii + 1]      * 4096) << ", "                            << setw(11) << (int)(simInput[ii + 2]      * 4096) << "), ";
          //  cout << "(" << setw(11) << (int)(unpackedInput[ii] * 4096 - simInput[ii] * 4096)  << ", " << setw(11) << (int)(
          //         unpackedInput[ii + 1] * 4096 - simInput[ii + 1] * 4096) << ", "  << setw(11) << (int)(unpackedInput[ii + 2] * 4096 - simInput[ii +
          //             2] * 4096) << "), " << endl;
          //  cout.unsetf(ios::showbase);
          //  cout << dec;
          //std::cout << " (" << simInput[ii] / unpackedInput[ii] << std::endl << ", " << simInput[ii + 1] /  unpackedInput[ii + 1] << ", " <<
          //          simInput[ii + 2] / unpackedInput[ii + 2] << ")" << std::endl;
          B2DEBUG(30, padright(lla, 100));
          B2DEBUG(20, padright(llb, 100));
        }
      }
    }


  }
  if (!m_limitedoutput && m_simNeuroTracksName != "") {
    B2DEBUG(10,  padright("Number of SW NeuroTracks: ", 40)             << padright(std::to_string(m_simNeuroTracks.getEntries()), 60));
    unsigned swntrn = 0;
    for (CDCTriggerTrack& ltrack : m_simNeuroTracks) {
      swntrn++;
      std::stringstream strpt;
      std::stringstream stromega;
      std::stringstream strphi;
      std::stringstream strtheta;
      std::stringstream strz;
      std::stringstream strquality;
      strpt       << std::fixed << std::setprecision(2)   << ltrack.getPt();
      stromega    << std::fixed << std::setprecision(2)   << ltrack.getOmega();
      strphi      << std::fixed << std::setprecision(2)   << (ltrack.getPhi0() * 180. / M_PI);
      strtheta    << std::fixed << std::setprecision(2)   << (ltrack.getDirection().Theta() * 180. / M_PI);
      strz        << std::fixed << std::setprecision(2)   << ltrack.getZ0();
      strquality << std::fixed << ltrack.getQualityVector();
      std::string trs  = "    SWNeuroTrack Nr. " + std::to_string(swntrn) + " (pt, omega, phi, theta, z) = ";
      trs += padto(strpt.str(), 6) + ", " + padto(stromega.str(), 6) + ", " + padto(strphi.str(), 6) + ", " + padto(strtheta.str(),
             6) + ", " + padto(strz.str(), 6) + ")" + ", Q=" + strquality.str();
      B2DEBUG(15, padright(trs, 100));
    }
  }
  if (!m_limitedoutput && m_simSegmentHitsName != "") {
    std::string axsw = std::to_string(axswts) + " / " + std::to_string(stswts);
    B2DEBUG(10,  padright("Number of AllSW TS (Axial/Stereo): ", 40)    << padright(axsw, 60));
    if (m_sim2DTracksSWTSName != "" && m_simNeuroTracksSWTSSW2DName != "") {
      B2DEBUG(10,  padright("Number of SW 2DFinderSWTSTracks: ", 40)          << padright(std::to_string(m_sim2DTracksSWTS.getEntries()),
              60));
      unsigned sf2dtrn = 0;
      for (CDCTriggerTrack& ltrack : m_sim2DTracksSWTS) {
        sf2dtrn++;
        std::stringstream strpt;
        std::stringstream stromega;
        std::stringstream strphi;
        strpt       << std::fixed << std::setprecision(2)   << ltrack.getPt();
        stromega    << std::fixed << std::setprecision(2)   << ltrack.getOmega();
        strphi      << std::fixed << std::setprecision(2)   << (ltrack.getPhi0() * 180. / M_PI);
        std::string trs  = "    SW2DFinderSWTSTrack Nr. " + std::to_string(sf2dtrn) + " (pt, omega, phi) = ";
        trs += padto(strpt.str(), 6) + ", " + padto(stromega.str(), 6) + ", " + padto(strphi.str(), 6) + ")";
        B2DEBUG(15, padright(trs, 100));
      }
      B2DEBUG(10,  padright("Number of SW NeuroSWTSSW2DTracks: ",
                            40)          << padright(std::to_string(m_simNeuroTracksSWTSSW2D.getEntries()), 60));
      unsigned aswntrn = 0;
      for (CDCTriggerTrack& ltrack : m_simNeuroTracksSWTSSW2D) {
        aswntrn++;
        std::stringstream strpt;
        std::stringstream stromega;
        std::stringstream strphi;
        std::stringstream strtheta;
        std::stringstream strz;
        strpt       << std::fixed << std::setprecision(2)   << ltrack.getPt();
        stromega    << std::fixed << std::setprecision(2)   << ltrack.getOmega();
        strphi      << std::fixed << std::setprecision(2)   << (ltrack.getPhi0() * 180. / M_PI);
        strtheta    << std::fixed << std::setprecision(2)   << (ltrack.getDirection().Theta() * 180. / M_PI);
        strz        << std::fixed << std::setprecision(2)   << ltrack.getZ0();
        std::string trs  = "    SWNeuroSWTSSW2DTrack Nr. " + std::to_string(aswntrn) + " (pt, omega, phi, theta, z) = ";
        trs += padto(strpt.str(), 6) + ", " + padto(stromega.str(), 6) + ", " + padto(strphi.str(), 6) + ", " + padto(strtheta.str(),
               6) + ", " + padto(strz.str(), 6) + ")";
        B2DEBUG(15, padright(trs, 100));
        vector<float> simInputSWTSSW2D =
          ltrack.getRelatedTo<CDCTriggerMLPInput>(m_simNeuroInputVectorSWTSSW2DName)->getInput();
        B2DEBUG(20, padright("      Input Vector simulated (id, t, alpha):", 100));
        for (unsigned ii = 0; ii < simInputSWTSSW2D.size(); ii += 3) {
          std::string lla = "      " + std::to_string(ii / 3) + ")";
          std::string llb = "      " + std::to_string(ii / 3) + ")";
          lla += "(" + padright(std::to_string(simInputSWTSSW2D[ii]),
                                8) + " " + padright(std::to_string(simInputSWTSSW2D[ii + 1]), 8) + " " + padright(std::to_string(simInputSWTSSW2D[ii + 2]),
                                    8) + ")";
          llb += "  (" + padright(std::to_string(int(simInputSWTSSW2D[ii] * 4096)),
                                  8) + " " + padright(std::to_string(int(simInputSWTSSW2D[ii + 1] * 4096)),
                                                      8) + " " + padright(std::to_string(int(simInputSWTSSW2D[ii + 2] * 4096)),
                                                          8) + ")";
          B2DEBUG(30, padright(lla, 100));
          B2DEBUG(20, padright(llb, 100));
        }

      }
    }
  }
  if (m_recoTracksName != "") {
    B2DEBUG(10,  padright("Number of  RecoTracks: ", 40)                  << padright(std::to_string(m_RecoTracks.getEntries()), 60));
    unsigned recotrn = 0;
    for (RecoTrack& ltrack : m_RecoTracks) {
      double phi0Target = 0;
      double invptTarget = 0;
      double thetaTarget = 0;
      double zTarget = 0;
      double d0Target = 0;
      bool foundValidRep = false;
      for (genfit::AbsTrackRep* rep : ltrack.getRepresentations()) {
        if (!ltrack.wasFitSuccessful(rep))
          continue;
        // get state (position, momentum etc.) from hit closest to IP and
        // extrapolate to z-axis (may throw an exception -> continue to next representation)
        try {
          genfit::MeasuredStateOnPlane state =
            ltrack.getMeasuredStateOnPlaneClosestTo(TVector3(0, 0, 0), rep);
          rep->extrapolateToLine(state, TVector3(0, 0, -1000), TVector3(0, 0, 2000));
          // TODO check after matching
          //  // flip tracks if necessary, such that trigger tracks and reco tracks
          //  // point in the same direction
          //  if (state.getMom().Dot(m_tracks[itrack]->getDirection()) < 0) {
          //    state.setPosMom(state.getPos(), -state.getMom());
          //    state.setChargeSign(-state.getCharge());
          //  }
          // get track parameters
          phi0Target = state.getMom().Phi() * 180. / M_PI ;
          if (phi0Target < 0.) {phi0Target = phi0Target + 360. ;}
          invptTarget = state.getCharge() * state.getMom().Pt();
          thetaTarget = acos(state.getMom().CosTheta()) * 180 / M_PI;
          zTarget = state.getPos().Z();
          d0Target = state.getPos().Perp();
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
        if (m_maxRecoZDist != -1.0 and abs(zTarget) > m_maxRecoZDist) {
          B2DEBUG(150, "RecoTrack not close to IP (maxRecoZDist), zReco = " << zTarget);
          continue;
        }
        if (m_maxRecoD0Dist != -1.0 and abs(d0Target) > m_maxRecoD0Dist) {
          B2DEBUG(150, "RecoTrack not close to the z axis (maxRecoD0Dist), d0Reco = " << d0Target);
          continue;
        }
      }
      recotrn++;
      std::stringstream strpt;
      std::stringstream stromega;
      std::stringstream strphi;
      std::stringstream strtheta;
      std::stringstream strz;
      strpt       << std::fixed << std::setprecision(2)   << invptTarget;
      stromega    << std::fixed << std::setprecision(2)   << d0Target;
      strphi      << std::fixed << std::setprecision(2)   << phi0Target;
      strtheta    << std::fixed << std::setprecision(2)   << thetaTarget;
      strz        << std::fixed << std::setprecision(2)   << zTarget;
      std::string trs  = "    RecoTrack Nr. " + std::to_string(recotrn) + " (invpt, d0, phi, theta, z) = ";
      trs += padto(strpt.str(), 6) + ", " + padto(stromega.str(), 6) + ", " + padto(strphi.str(), 6) + ", " + padto(strtheta.str(),
             6) + ", " + padto(strz.str(), 6) + ")";
      B2DEBUG(15, padright(trs, 100));
    }
  }
  B2DEBUG(10, padright(" ", 100));
  B2DEBUG(10, padright(" ", 100));
  if (m_unpackedSegmentHitsName != "") {
    B2DEBUG(15, padright("Detailed information about HW TS ", 100));
    B2DEBUG(15, padright(" ", 100));
    B2DEBUG(15, padright(hwtsstring, 100));
    for (auto x : hwtsoutput) {
      B2DEBUG(15, padright(x.strline, 100));
    }
    B2DEBUG(15, padright(" ", 100));
  }
  if (!m_limitedoutput && m_simSegmentHitsName != "") {
    B2DEBUG(15, padright(" ", 100));
    B2DEBUG(15, padright("Detailed information about SW TS ", 100));
    B2DEBUG(15, padright(" ", 100));
    B2DEBUG(15, padright(swtsstring, 100));
    for (auto x : swtsoutput) {
      B2DEBUG(15, padright(x.strline, 100));
    }
  }
}


void CDCTriggerNeuroDQMModule::endRun()
{
}


void CDCTriggerNeuroDQMModule::terminate()
{
}
