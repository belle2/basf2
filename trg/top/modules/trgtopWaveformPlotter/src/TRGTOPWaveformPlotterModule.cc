/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGTOPWaveformPlotterModule.cc
// Section  : TRG TOP
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : TOP-CDC matching studies module for TRGTOP
//---------------------------------------------------------------
// 1.00 : 2020/09/20 : First version
//
// Modeled after / heavily borrowing from GDL, GRL and ECL DQM modules
//---------------------------------------------------------------
#include <trg/top/modules/trgtopWaveformPlotter/TRGTOPWaveformPlotterModule.h>
//#include <mdst/dataobjects/SoftwareTriggerResult.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include "trg/top/dataobjects/TRGTOPTimeStampsSlot.h"
#include "trg/top/dataobjects/TRGTOPTimeStamp.h"

#include "trg/top/dataobjects/TRGTOPWaveFormTimeStampsSlot.h"
#include "trg/top/dataobjects/TRGTOPWaveFormTimeStamp.h"

#include "trg/top/dataobjects/TRGTOPSlotTiming.h"

#include "trg/top/dataobjects/TRGTOPTimingISimSlot.h"

#include <framework/datastore/RelationVector.h>

#include <framework/logging/Logger.h>
#include <boost/algorithm/string.hpp>

#include <iostream>

//using namespace std;
using namespace Belle2;

REG_MODULE(TRGTOPWaveformPlotter);

TRGTOPWaveformPlotterModule::TRGTOPWaveformPlotterModule() : Module::Module()
{

  setDescription("TRGTOP Waveform Plotter");

  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions

  addParam("plottingMode", m_plottingMode,
           "Plotting mode: 0 (forced ranges for x and y), 1 (slot-level zoom-in), 2 (global zoom-in; default), 3 (global zoom-in for y and xmax, forced left-side edge for x), 4 (global zoom-in for y, forced range for x), anything else (full ranges)",
           2);

  addParam("firstAssumedClockCycle", m_firstAssumedClockCycle,
           "First assumed clock cycle for main readout hits",
           108);

  addParam("cutPlotMinNumberTriggerReadoutHits", m_cutPlotMinNumberTriggerReadoutHits,
           "Minimum number of trigger timestamps cut for drawing the waveform",
           5);

  addParam("cutPlotMinNumberMainReadoutHits", m_cutPlotMinNumberMainReadoutHits,
           "Minimum number of main readout hits cut for drawing the hits",
           5);

  addParam("markerSizeHits", m_markerSizeHits,
           "Marker size (default: 0.85) used to plot main readout hits",
           0.85);

  addParam("markerSizeTimestamps", m_markerSizeTimestamps,
           "Marker size (default: 0.65) used to plot trigger readout timestamps",
           0.65);

  addParam("markerTypeHits", m_markerTypeHits,
           "Marker type (default: 21) used to plot main readout hits",
           21);

  addParam("markerTypeTimestamps", m_markerTypeTimestamps,
           "Marker type (default: 20) used to plot trigger readout timestamps",
           20);

  addParam("xMin", m_xMin,
           "xMin for plotting (default: 100)",
           100);

  addParam("xMax", m_xMax,
           "xMax for plotting (default: 200)",
           200);

  addParam("yMin", m_yMin,
           "yMin for plotting (default: 0)",
           0);

  addParam("yMax", m_yMax,
           "yMax for plotting (default: 47000)",
           47000);

  addParam("showGridx", m_showGridx,
           "Show grid x (default: 0 (no))",
           0);

  addParam("showGridy", m_showGridy,
           "Show grid y (default: 1 (yes))",
           1);

  addParam("canvasXMin", m_canvasXMin,
           "TCanvas x min (default: 0))",
           0);

  addParam("canvasYMin", m_canvasYMin,
           "TCanvas y min (default: 0))",
           0);

  addParam("canvasXSize", m_canvasXSize,
           "TCanvas x size (default: 900))",
           900);

  addParam("canvasYSize", m_canvasYSize,
           "TCanvas y size (default: 600))",
           600);

  addParam("shiftCanvas", m_shiftCanvas,
           "Shift canvases (default: 1 (yes)))",
           1);

  addParam("xShiftCanvas", m_xShiftCanvas,
           "x shift canvases (default: 900))",
           910);

  addParam("yShiftCanvas", m_yShiftCanvas,
           "y shift canvases (default: 600))",
           610);

}

void TRGTOPWaveformPlotterModule::beginRun()
{

  //  myROOTDir->cd();

  //  oldROOTDir->cd();

}

void TRGTOPWaveformPlotterModule::initialize()
{

  //  oldROOTDir = gDirectory;

  //  m_file = new TFile(m_outputTTreeFileName.c_str(), "RECREATE");

  //  myROOTDir = gDirectory;

  for (int slot = 1; slot <= NUMBER_OF_TOP_SLOTS; slot++) {
    m_timeStampsGraphMainReadout[slot - 1] = 0;
    m_timeStampsGraphTriggerReadout[slot - 1] = 0;
    m_myMultiGraph[slot - 1] = 0;
    m_myPaveText[slot - 1] = 0;
    m_myCanvas[slot - 1] = 0;
  }

  //  if (!oldROOTDir) oldROOTDir->cd();

}

void TRGTOPWaveformPlotterModule::endRun()
{

}

void TRGTOPWaveformPlotterModule::event()
{

  //  oldROOTDir = gDirectory;
  //  myROOTDir->cd();

  for (int slot = 1; slot <= NUMBER_OF_TOP_SLOTS; slot++) {

    //
    // The code below assumes that none of the higher-level object (such as TCanvas) own any of the embedded objects (such as TGraph's)
    //

    if (m_timeStampsGraphMainReadout[slot - 1]) {
      delete m_timeStampsGraphMainReadout[slot - 1];
      m_timeStampsGraphMainReadout[slot - 1] = NULL;
    }

    if (m_timeStampsGraphTriggerReadout[slot - 1]) {
      delete m_timeStampsGraphTriggerReadout[slot - 1];
      m_timeStampsGraphTriggerReadout[slot - 1] = NULL;
    }

    if (m_myMultiGraph[slot - 1]) {
      delete m_myMultiGraph[slot - 1];
      m_myMultiGraph[slot - 1] = NULL;
    }

    if (m_myPaveText[slot - 1]) {
      delete m_myPaveText[slot - 1];
      m_myPaveText[slot - 1] = NULL;
    }

    if (m_myCanvas[slot - 1]) {
      delete m_myCanvas[slot - 1];
      m_myCanvas[slot - 1] = NULL;
    }

  }

  // TOP timestamps made from TOPRawDigits
  StoreArray<TRGTOPTimeStamp> trgtopTimeStamps("TRGTOPTimeStamps");
  StoreArray<TRGTOPTimeStampsSlot> trgtopTimeStampsSlots("TRGTOPTimeStampsSlots");

  // TRG TOP waveform readout
  StoreArray<TRGTOPWaveFormTimeStamp> trgtopWaveFormTimeStamps("TRGTOPWaveFormTimeStamps");
  StoreArray<TRGTOPWaveFormTimeStampsSlot> trgtopWaveFormTimeStampsSlots("TRGTOPWaveFormTimeStampsSlots");

  // TOP TRG slot-level t0 decisions
  StoreArray<TRGTOPSlotTiming> trgtopSlotTimingAll("TRGTOPSlotTimings");

  // ISim results for main readout (for all slots and all decisions for each slot)
  StoreArray<TRGTOPTimingISim> trgtopTimingISimMainReadoutAll("TRGTOPTimingISimMainReadouts");

  // ISim results for trigger readout (for all slots and all decisions for each slot)
  StoreArray<TRGTOPTimingISim> trgtopTimingISimTriggerReadoutAll("TRGTOPTimingISimTriggerReadouts");


  //  if (!trgtopWaveFormTimeStamps) return;
  //  if (!trgtopWaveFormTimeStampsSlots) return;

  // ISim results for main readout hits

  bool topTRGISimMRODecisionPresent[NUMBER_OF_TOP_SLOTS] = {false};
  int topTRGISimMRODecisionTiming[NUMBER_OF_TOP_SLOTS] = {0};
  int topTRGISimMRODecisionNTS[NUMBER_OF_TOP_SLOTS] = {0};
  int topTRGISimMRODecisionNumber[NUMBER_OF_TOP_SLOTS] = {0};

  std::string topTRGIsimMRODecisionText[NUMBER_OF_TOP_SLOTS];
  for (int slot = 1; slot <= NUMBER_OF_TOP_SLOTS; slot++) topTRGIsimMRODecisionText[slot - 1] = "All iSim TOP decisions: ";

  for (const auto& slotDecision : trgtopTimingISimMainReadoutAll) {

    int slot = slotDecision.getSlotId();

    topTRGISimMRODecisionPresent[slot - 1] = true;
    topTRGISimMRODecisionTiming[slot - 1] = slotDecision.getSlotTiming();
    topTRGISimMRODecisionNTS[slot - 1] = topTRGISimMRODecisionNTS[slot - 1] + slotDecision.getSlotNHits();
    topTRGISimMRODecisionNumber[slot - 1] = topTRGISimMRODecisionNumber[slot - 1] + 1;

    std::stringstream ss1;
    ss1 << slotDecision.getSlotNHits();
    std::stringstream ss2;
    ss2 << slotDecision.getSlotTiming();
    std::stringstream ss3;
    ss3 << slotDecision.getSlotDecisionClockCycle();
    topTRGIsimMRODecisionText[slot - 1] = topTRGIsimMRODecisionText[slot - 1] + " " + ss2.str() + " ( " + ss1.str() + " [" + ss3.str() +
                                          "]" + " ); ";

  }

  // ISim results for trigger readout hits

  bool topTRGISimWFRODecisionPresent[NUMBER_OF_TOP_SLOTS] = {false};
  int topTRGISimWFRODecisionTiming[NUMBER_OF_TOP_SLOTS] = {0};
  int topTRGISimWFRODecisionNTS[NUMBER_OF_TOP_SLOTS] = {0};
  int topTRGISimWFRODecisionNumber[NUMBER_OF_TOP_SLOTS] = {0};

  std::string topTRGIsimWFRODecisionText[NUMBER_OF_TOP_SLOTS];
  for (int slot = 1; slot <= NUMBER_OF_TOP_SLOTS; slot++) topTRGIsimWFRODecisionText[slot - 1] = "All iSim TRG decisions: ";

  for (const auto& slotDecision : trgtopTimingISimTriggerReadoutAll) {

    int slot = slotDecision.getSlotId();

    topTRGISimWFRODecisionPresent[slot - 1] = true;
    topTRGISimWFRODecisionTiming[slot - 1] = slotDecision.getSlotTiming();
    topTRGISimWFRODecisionNTS[slot - 1] = topTRGISimWFRODecisionNTS[slot - 1] + slotDecision.getSlotNHits();
    topTRGISimWFRODecisionNumber[slot - 1] = topTRGISimWFRODecisionNumber[slot - 1] + 1;

    std::stringstream ss1;
    ss1 << slotDecision.getSlotNHits();
    std::stringstream ss2;
    ss2 << slotDecision.getSlotTiming();
    std::stringstream ss3;
    ss3 << slotDecision.getSlotDecisionClockCycle();
    topTRGIsimWFRODecisionText[slot - 1] = topTRGIsimWFRODecisionText[slot - 1] + " " + ss2.str() + " ( " + ss1.str() + " [" + ss3.str()
                                           + "]" + " ); ";

  }

  // slot-level online trigger decisions

  bool topTRGDecisionPresent[NUMBER_OF_TOP_SLOTS] = {false};
  int topTRGDecisionTiming[NUMBER_OF_TOP_SLOTS] = {0};
  int topTRGDecisionNTS[NUMBER_OF_TOP_SLOTS] = {0};
  int topTRGDecisionNumber[NUMBER_OF_TOP_SLOTS] = {0};
  //  int topTRGDecisionClockCycle[NUMBER_OF_TOP_SLOTS] = {0};

  std::string topTRGDecisionText[NUMBER_OF_TOP_SLOTS];
  for (int slot = 1; slot <= NUMBER_OF_TOP_SLOTS; slot++) topTRGDecisionText[slot - 1] = "All online TRG decisions: ";

  for (const auto& slotDecision : trgtopSlotTimingAll) {

    // for purposes of visualization do not display information received from the OTHER board
    if (slotDecision.isThisBoard()) {

      int slot = slotDecision.getSlotId();

      topTRGDecisionPresent[slot - 1] = true;
      topTRGDecisionTiming[slot - 1] = slotDecision.getSlotTiming();
      topTRGDecisionNTS[slot - 1] = topTRGDecisionNTS[slot - 1] + slotDecision.getSlotNHits();
      //      topTRGDecisionClockCycle[slot-1] = slotDecision.getSlotDecisionClockCycle();
      topTRGDecisionNumber[slot - 1] = topTRGDecisionNumber[slot - 1] + 1;

      std::stringstream ss1;
      ss1 << slotDecision.getSlotNHits();
      std::stringstream ss2;
      ss2 << slotDecision.getSlotTiming() / 2;
      std::stringstream ss3;
      ss3 << slotDecision.getSlotDecisionClockCycle();
      topTRGDecisionText[slot - 1] = topTRGDecisionText[slot - 1] + " " + ss2.str() + " ( " + ss1.str() + " [" + ss3.str() + "]" + " ); ";

    }
  }

  // main readout

  bool topTRGMROPresent[NUMBER_OF_TOP_SLOTS] = {false};
  int topTRGMRONTS[NUMBER_OF_TOP_SLOTS] = {0};

  int clockCycleMainReadout[NUMBER_OF_TOP_SLOTS] = {0};
  int indexClockCycleMainReadout[NUMBER_OF_TOP_SLOTS] = {0};

  int xMinMRO[NUMBER_OF_TOP_SLOTS] = {0};
  int xMaxMRO[NUMBER_OF_TOP_SLOTS] = {0};

  int yMinMRO[NUMBER_OF_TOP_SLOTS] = {0};
  int yMaxMRO[NUMBER_OF_TOP_SLOTS] = {0};

  for (const auto& slotTSS : trgtopTimeStampsSlots) {

    int slot = slotTSS.getSlotId();

    clockCycleMainReadout[slot - 1] = m_firstAssumedClockCycle;

    xMinMRO[slot - 1] = -1;
    xMaxMRO[slot - 1] = -1;

    yMinMRO[slot - 1] = -1;
    yMaxMRO[slot - 1] = -1;

    int nHits = slotTSS.getNumberOfTimeStamps();

    if (nHits != 0) {

      topTRGMROPresent[slot - 1] = true;
      topTRGMRONTS[slot - 1] = nHits;

      if (topTRGMRONTS[slot - 1] > MAX_NUMBER_OF_CLOCK_CYCLES) topTRGMRONTS[slot - 1] = MAX_NUMBER_OF_CLOCK_CYCLES;

      for (const auto& timeStamp : slotTSS.getRelationsTo<TRGTOPTimeStamp>()) {
        int value = timeStamp.getTimeStamp();

        m_clockCyclesMainReadout[slot - 1][indexClockCycleMainReadout[slot - 1]] = clockCycleMainReadout[slot - 1];
        m_timeStampsMainReadout[slot - 1][indexClockCycleMainReadout[slot - 1]] = value;

        if (xMinMRO[slot - 1] == -1
            || clockCycleMainReadout[slot - 1] < xMinMRO[slot - 1]) xMinMRO[slot - 1] = clockCycleMainReadout[slot - 1];
        if (xMaxMRO[slot - 1] == -1
            || clockCycleMainReadout[slot - 1] > xMaxMRO[slot - 1]) xMaxMRO[slot - 1] = clockCycleMainReadout[slot - 1];

        if (yMinMRO[slot - 1] == -1 || value < yMinMRO[slot - 1]) yMinMRO[slot - 1] = value;
        if (yMaxMRO[slot - 1] == -1 || value > yMaxMRO[slot - 1]) yMaxMRO[slot - 1] = value;

        if (indexClockCycleMainReadout[slot - 1] < MAX_NUMBER_OF_CLOCK_CYCLES) {
          indexClockCycleMainReadout[slot - 1]++;
          clockCycleMainReadout[slot - 1]++;
        }
      }
    }
  }

  // waveform readout

  bool topTRGWFROPresent[NUMBER_OF_TOP_SLOTS] = {false};
  int topTRGWFRONTS[NUMBER_OF_TOP_SLOTS] = {0};

  int clockCycleTriggerReadout[NUMBER_OF_TOP_SLOTS] = {0};
  int indexClockCycleTriggerReadout[NUMBER_OF_TOP_SLOTS] = {0};

  int xMinWFRO[NUMBER_OF_TOP_SLOTS] = {0};
  int xMaxWFRO[NUMBER_OF_TOP_SLOTS] = {0};

  int yMinWFRO[NUMBER_OF_TOP_SLOTS] = {0};
  int yMaxWFRO[NUMBER_OF_TOP_SLOTS] = {0};

  for (auto& slotWaveFormTSS : trgtopWaveFormTimeStampsSlots) {

    int slot = slotWaveFormTSS.getSlotId();

    xMinWFRO[slot - 1] = -1;
    xMaxWFRO[slot - 1] = -1;

    yMinWFRO[slot - 1] = -1;
    yMaxWFRO[slot - 1] = -1;

    int nHits = slotWaveFormTSS.getNumberOfActualTimeStamps();

    if (nHits != 0) {

      topTRGWFROPresent[slot - 1] = true;
      topTRGWFRONTS[slot - 1] = nHits;

      if (topTRGWFRONTS[slot - 1] > MAX_NUMBER_OF_CLOCK_CYCLES) topTRGWFRONTS[slot - 1] = MAX_NUMBER_OF_CLOCK_CYCLES;

      for (auto& timeStamp : slotWaveFormTSS.getRelationsTo<TRGTOPWaveFormTimeStamp>()) {

        if (!timeStamp.isEmptyClockCycle()) {

          int value = timeStamp.getTimeStamp();

          m_clockCyclesTriggerReadout[slot - 1][indexClockCycleTriggerReadout[slot - 1]] = clockCycleTriggerReadout[slot - 1];
          m_timeStampsTriggerReadout[slot - 1][indexClockCycleTriggerReadout[slot - 1]] = value;

          if (xMinWFRO[slot - 1] == -1
              || clockCycleTriggerReadout[slot - 1] < xMinWFRO[slot - 1]) xMinWFRO[slot - 1] = clockCycleTriggerReadout[slot - 1];
          if (xMaxWFRO[slot - 1] == -1
              || clockCycleTriggerReadout[slot - 1] > xMaxWFRO[slot - 1]) xMaxWFRO[slot - 1] = clockCycleTriggerReadout[slot - 1];

          if (yMinWFRO[slot - 1] == -1 || value < yMinWFRO[slot - 1]) yMinWFRO[slot - 1] = value;
          if (yMaxWFRO[slot - 1] == -1 || value > yMaxWFRO[slot - 1]) yMaxWFRO[slot - 1] = value;

          if (indexClockCycleTriggerReadout[slot - 1] < MAX_NUMBER_OF_CLOCK_CYCLES) {
            indexClockCycleTriggerReadout[slot - 1]++;
          }
        }
        clockCycleTriggerReadout[slot - 1]++;
      }
    }
  }

  // plotting

  // for each slot
  // decide if there is anything available (and requested) to plot for this slot
  // then create a TMultiGraph
  // then create at least one TGraph and add it to the list in TMultiGraph
  // finally, decide how to plot this TMultiGraph and do that

  bool plotMyCanvas[NUMBER_OF_TOP_SLOTS] = {false};

  int xMin = -1;
  int xMax = -1;
  int yMin = -1;
  int yMax = -1;

  for (int slot = 1; slot <= NUMBER_OF_TOP_SLOTS; slot++) {
    if (topTRGMRONTS[slot - 1] > 0 || topTRGWFRONTS[slot - 1] > 0) {
      if (topTRGMRONTS[slot - 1] >= m_cutPlotMinNumberMainReadoutHits) {
        if (topTRGWFRONTS[slot - 1] >= m_cutPlotMinNumberTriggerReadoutHits) {
          plotMyCanvas[slot - 1] = true;
          if (xMinMRO[slot - 1] != -1 && (xMin == -1 || xMinMRO[slot - 1] < xMin)) xMin = xMinMRO[slot - 1];
          if (xMaxMRO[slot - 1] != -1 && (xMax == -1 || xMaxMRO[slot - 1] > xMax)) xMax = xMaxMRO[slot - 1];
          if (yMinMRO[slot - 1] != -1 && (yMin == -1 || yMinMRO[slot - 1] < yMin)) yMin = yMinMRO[slot - 1];
          if (yMaxMRO[slot - 1] != -1 && (yMax == -1 || yMaxMRO[slot - 1] > yMax)) yMax = yMaxMRO[slot - 1];

          if (xMinWFRO[slot - 1] != -1 && (xMin == -1 || xMinWFRO[slot - 1] < xMin)) xMin = xMinWFRO[slot - 1];
          if (xMaxWFRO[slot - 1] != -1 && (xMax == -1 || xMaxWFRO[slot - 1] > xMax)) xMax = xMaxWFRO[slot - 1];
          if (yMinWFRO[slot - 1] != -1 && (yMin == -1 || yMinWFRO[slot - 1] < yMin)) yMin = yMinWFRO[slot - 1];
          if (yMaxWFRO[slot - 1] != -1 && (yMax == -1 || yMaxWFRO[slot - 1] > yMax)) yMax = yMaxWFRO[slot - 1];
        }
      }
    }
  }

  xMin = std::max(xMin - 10, 0);
  xMax = xMax + 10;
  yMin = std::max(yMin - 10, 0);
  yMax = yMax + 10;

  std::string myMultiGraphTitle[NUMBER_OF_TOP_SLOTS];

  std::string myPaveTextInfo[NUMBER_OF_TOP_SLOTS][8];

  for (int slot = 1; slot <= NUMBER_OF_TOP_SLOTS; slot++) {

    if (plotMyCanvas[slot - 1]) {

      m_myMultiGraph[slot - 1] = new TMultiGraph();
      // In principle, we can allow TCanvas (where this TMultiGraph will be plotted) to assume the ownership over this graph.
      // Then, when memory is freed, it should be sufficient to delete TCanvas only, as it would delete the graph also.
      //    m_myMultiGraph[slot-1]->SetBit(kCanDelete);
      std::stringstream slotInfo;
      slotInfo << slot;
      myMultiGraphTitle[slot - 1] = "Slot " + slotInfo.str() + " : ";

      m_myPaveText[slot - 1] = new TPaveText(0.40, 0.10, 0.90, 0.30, "bl NDC");
      //      m_myPaveText[slot-1]->SetHeader(myMultiGraphTitle[slot-1].c_str(),"C");;

      if (topTRGMROPresent[slot - 1]) {

        m_timeStampsGraphMainReadout[slot - 1] = new TGraph(topTRGMRONTS[slot - 1], &m_clockCyclesMainReadout[slot - 1][0],
                                                            &m_timeStampsMainReadout[slot - 1][0]);
        // In principle, we can allow TCanvas (where this TGraph will be plotted) to assume the ownership over this graph.
        // Then, when memory is freed, it should be sufficient to delete TCanvas only, as it would delete the graph also.
        //  m_timeStampsGraphMainReadout[slot-1]->SetBit(kCanDelete);

        m_timeStampsGraphMainReadout[slot - 1]->SetMarkerStyle(m_markerTypeHits);
        m_timeStampsGraphMainReadout[slot - 1]->SetMarkerSize(m_markerSizeHits);
        m_timeStampsGraphMainReadout[slot - 1]->SetMarkerColor(kRed);

        m_myMultiGraph[slot - 1]->Add(m_timeStampsGraphMainReadout[slot - 1], "AP");
      }


      if (topTRGWFROPresent[slot - 1]) {

        m_timeStampsGraphTriggerReadout[slot - 1] = new TGraph(topTRGWFRONTS[slot - 1], &m_clockCyclesTriggerReadout[slot - 1][0],
                                                               &m_timeStampsTriggerReadout[slot - 1][0]);
        // In principle, we can allow TCanvas (where this TGraph will be plotted) to assume the ownership over this graph.
        // Then, when memory is freed, it should be sufficient to delete TCanvas only, as it would delete the graph also.
        //  m_timeStampsGraphTriggerReadout[slot-1]->SetBit(kCanDelete);

        m_timeStampsGraphTriggerReadout[slot - 1]->SetMarkerStyle(m_markerTypeTimestamps);
        m_timeStampsGraphTriggerReadout[slot - 1]->SetMarkerSize(m_markerSizeTimestamps);
        m_timeStampsGraphTriggerReadout[slot - 1]->SetLineWidth(3);

        int color = kBlue;
        if (!topTRGDecisionPresent[slot - 1]) color = kBlack;

        m_timeStampsGraphTriggerReadout[slot - 1]->SetMarkerColor(color);

        m_myMultiGraph[slot - 1]->Add(m_timeStampsGraphTriggerReadout[slot - 1], "AP");
      }
    }
  }

  int canvasXMin = m_canvasXMin;
  int canvasYMin = m_canvasYMin;
  int canvasXSize = m_canvasXSize;
  int canvasYSize = m_canvasYSize;

  for (int slot = 1; slot <= NUMBER_OF_TOP_SLOTS; slot++) {

    if (plotMyCanvas[slot - 1]) {

      myMultiGraphTitle[slot - 1] = myMultiGraphTitle[slot - 1] + " TOP / TRG / L1 / iSim TOP / iSim TRG hits:";

      std::stringstream nHitInfoMainReadout;
      nHitInfoMainReadout << topTRGMRONTS[slot - 1];
      myMultiGraphTitle[slot - 1] = myMultiGraphTitle[slot - 1] + " " + nHitInfoMainReadout.str();

      myPaveTextInfo[slot - 1][0] = "N hits (TOP readout): " + nHitInfoMainReadout.str();

      std::stringstream nHitInfoTriggerReadout;
      nHitInfoTriggerReadout << topTRGWFRONTS[slot - 1];
      myMultiGraphTitle[slot - 1] = myMultiGraphTitle[slot - 1] + " / " + nHitInfoTriggerReadout.str();

      myPaveTextInfo[slot - 1][1] = "N timestamps (TRG readout): " + nHitInfoTriggerReadout.str();

      std::stringstream nHitInfoTriggerDecision;
      nHitInfoTriggerDecision << topTRGDecisionNTS[slot - 1];
      myMultiGraphTitle[slot - 1] = myMultiGraphTitle[slot - 1] + " / " + nHitInfoTriggerDecision.str();

      myPaveTextInfo[slot - 1][2] = "N timestamps TRG online: " + nHitInfoTriggerDecision.str();

      std::stringstream nHitInfoISimTriggerDecisionMainReadout;
      nHitInfoISimTriggerDecisionMainReadout << topTRGISimMRODecisionNTS[slot - 1];
      myMultiGraphTitle[slot - 1] = myMultiGraphTitle[slot - 1] + " / " + nHitInfoISimTriggerDecisionMainReadout.str();

      myPaveTextInfo[slot - 1][3] = "N timestamps TRG ISim (all, TOP readout): " + nHitInfoISimTriggerDecisionMainReadout.str();

      std::stringstream nHitInfoISimTriggerDecisionTriggerReadout;
      nHitInfoISimTriggerDecisionTriggerReadout << topTRGISimWFRODecisionNTS[slot - 1];
      myMultiGraphTitle[slot - 1] = myMultiGraphTitle[slot - 1] + " / " + nHitInfoISimTriggerDecisionTriggerReadout.str();

      myPaveTextInfo[slot - 1][4] = "N timestamps TRG ISim (all, TRG readout): " + nHitInfoISimTriggerDecisionTriggerReadout.str();

      //      myMultiGraphTitle[slot-1] = myMultiGraphTitle[slot-1] + ", t0 online/ISim main/TRG: ";

      if (topTRGDecisionPresent[slot - 1]) {
        std::stringstream nTimingInfoTriggerDecision;
        nTimingInfoTriggerDecision << topTRGDecisionTiming[slot - 1] / 2;
        std::stringstream nNumberInfoTriggerDecision;
        nNumberInfoTriggerDecision << topTRGDecisionNumber[slot - 1];
        myPaveTextInfo[slot - 1][5] = "Number of online TRG decisions: " + nNumberInfoTriggerDecision.str() + ", most recent t0: " +
                                      nTimingInfoTriggerDecision.str();
        //  myMultiGraphTitle[slot-1] = myMultiGraphTitle[slot-1] + nNumberInfoTriggerDecision.str() + " ( ";
        //  myMultiGraphTitle[slot-1] = myMultiGraphTitle[slot-1] + nTimingInfoTriggerDecision.str() + " ) / ";
      } else {
        myPaveTextInfo[slot - 1][5] = "No online TRG decisions";
        //  myMultiGraphTitle[slot-1] = myMultiGraphTitle[slot-1] + "none / ";
      }

      if (topTRGISimMRODecisionPresent[slot - 1]) {
        std::stringstream nTimingInfoTriggerDecision;
        nTimingInfoTriggerDecision << topTRGISimMRODecisionTiming[slot - 1];
        std::stringstream nNumberInfoTriggerDecision;
        nNumberInfoTriggerDecision << topTRGISimMRODecisionNumber[slot - 1];
        myPaveTextInfo[slot - 1][6] = "Number of ISim TOP readout TRG decisions: " + nNumberInfoTriggerDecision.str() + ", most recent t0: "
                                      + nTimingInfoTriggerDecision.str();
        //  myMultiGraphTitle[slot-1] = myMultiGraphTitle[slot-1] + nNumberInfoTriggerDecision.str() + " ( ";
        //  myMultiGraphTitle[slot-1] = myMultiGraphTitle[slot-1] + nTimingInfoTriggerDecision.str() + " ) / ";
      } else {
        myPaveTextInfo[slot - 1][6] = "No iSim TOP readout TRG decisions";
        //  myMultiGraphTitle[slot-1] = myMultiGraphTitle[slot-1] + " none / ";
      }

      if (topTRGISimWFRODecisionPresent[slot - 1]) {
        std::stringstream nTimingInfoTriggerDecision;
        nTimingInfoTriggerDecision << topTRGISimWFRODecisionTiming[slot - 1];
        std::stringstream nNumberInfoTriggerDecision;
        nNumberInfoTriggerDecision << topTRGISimWFRODecisionNumber[slot - 1];
        myPaveTextInfo[slot - 1][7] = "Number of ISim TRG readout TRG decisions: " + nNumberInfoTriggerDecision.str() + ", most recent t0: "
                                      + nTimingInfoTriggerDecision.str();
        //  myMultiGraphTitle[slot-1] = myMultiGraphTitle[slot-1] + nNumberInfoTriggerDecision.str() + " ( ";
        //  myMultiGraphTitle[slot-1] = myMultiGraphTitle[slot-1] + nTimingInfoTriggerDecision.str() + " )";
      } else {
        myPaveTextInfo[slot - 1][7] = "No iSim TRG readout TRG decisions";
        //  myMultiGraphTitle[slot-1] = myMultiGraphTitle[slot-1] + "none";
      }

      myMultiGraphTitle[slot - 1] = myMultiGraphTitle[slot - 1] + "; Clock cycle (8ns units); Hit time (2ns units)";
      m_myMultiGraph[slot - 1]->SetTitle(myMultiGraphTitle[slot - 1].c_str());

      std::stringstream slotInfo;
      slotInfo << slot;
      std::string name = "c_" + slotInfo.str();
      std::string title = "Main readout hits (red) and trigger readout timestamps (blue/black) for slot " + slotInfo.str();
      m_myCanvas[slot - 1] = new TCanvas(name.c_str(), title.c_str(), canvasXMin, canvasYMin, canvasXSize, canvasYSize);
      if (m_shiftCanvas) {
        canvasXMin = canvasXMin + m_xShiftCanvas;
        if (canvasXMin >= 1000) {
          canvasXMin = m_canvasXMin;
          canvasYMin = canvasYMin + m_yShiftCanvas;
          if (canvasYMin >= 700) {
            canvasYMin = m_canvasYMin;
          }
        }
      }

      m_myMultiGraph[slot - 1]->Draw("AP");

      TAxis* xAxis = m_myMultiGraph[slot - 1]->GetXaxis();

      if (m_plottingMode == 0) {
        m_myMultiGraph[slot - 1]->SetMinimum(m_yMin);
        m_myMultiGraph[slot - 1]->SetMaximum(m_yMax);
        xAxis->SetLimits(m_xMin, m_xMax);
      } else if (m_plottingMode == 1) {
        m_myMultiGraph[slot - 1]->SetMinimum(std::min(yMinMRO[slot - 1], yMinWFRO[slot - 1]));
        m_myMultiGraph[slot - 1]->SetMaximum(std::max(yMaxMRO[slot - 1], yMaxWFRO[slot - 1]));
        xAxis->SetLimits(std::min(xMinMRO[slot - 1], xMinWFRO[slot - 1]), std::max(xMaxMRO[slot - 1], xMaxWFRO[slot - 1]));
      } else if (m_plottingMode == 2) {
        m_myMultiGraph[slot - 1]->SetMinimum(yMin);
        m_myMultiGraph[slot - 1]->SetMaximum(yMax);
        xAxis->SetLimits(xMin, xMax);
      } else if (m_plottingMode == 3) {
        m_myMultiGraph[slot - 1]->SetMinimum(yMin);
        m_myMultiGraph[slot - 1]->SetMaximum(yMax);
        xAxis->SetLimits(m_xMin, xMax);
      } else if (m_plottingMode == 4) {
        m_myMultiGraph[slot - 1]->SetMinimum(yMin);
        m_myMultiGraph[slot - 1]->SetMaximum(yMax);
        xAxis->SetLimits(m_xMin, m_xMax);
      } else {
        m_myMultiGraph[slot - 1]->SetMinimum(0);
        m_myMultiGraph[slot - 1]->SetMaximum(MAX_TIMESTAMP_RANGE);
        xAxis->SetLimits(0, MAX_NUMBER_OF_CLOCK_CYCLES);
      }

      m_myMultiGraph[slot - 1]->Draw("AP");
      m_myCanvas[slot - 1]->SetGridx(m_showGridx);
      m_myCanvas[slot - 1]->SetGridy(m_showGridy);

      //      m_myPaveText[slot-1]->AddText(myPaveTextInfo[slot-1][0].c_str());
      //      m_myPaveText[slot-1]->AddText(myPaveTextInfo[slot-1][1].c_str());
      //      m_myPaveText[slot-1]->AddText(myPaveTextInfo[slot-1][2].c_str());
      //      m_myPaveText[slot-1]->AddText(myPaveTextInfo[slot-1][3].c_str());
      //      m_myPaveText[slot-1]->AddText(myPaveTextInfo[slot-1][4].c_str());
      m_myPaveText[slot - 1]->AddText(myPaveTextInfo[slot - 1][5].c_str());
      m_myPaveText[slot - 1]->AddText(myPaveTextInfo[slot - 1][6].c_str());
      m_myPaveText[slot - 1]->AddText(myPaveTextInfo[slot - 1][7].c_str());

      m_myPaveText[slot - 1]->AddText(topTRGDecisionText[slot - 1].c_str());
      m_myPaveText[slot - 1]->AddText(topTRGIsimMRODecisionText[slot - 1].c_str());
      m_myPaveText[slot - 1]->AddText(topTRGIsimWFRODecisionText[slot - 1].c_str());

      m_myPaveText[slot - 1]->SetBorderSize(1);
      //      m_myPaveText[slot-1]->SetFillColor(19);
      m_myPaveText[slot - 1]->SetFillStyle(0);
      m_myPaveText[slot - 1]->SetTextFont(40);
      m_myPaveText[slot - 1]->SetTextAlign(12);
      m_myPaveText[slot - 1]->Draw();

      m_myCanvas[slot - 1]->Update();

    }
  }

  //  if (!oldROOTDir) oldROOTDir->cd();
}

void TRGTOPWaveformPlotterModule::terminate()
{
  //  oldROOTDir = gDirectory;
  //  myROOTDir->cd();

  //  if (!oldROOTDir) oldROOTDir->cd();
}

//TRGTOPWaveformPlotterModule::~TRGTOPWaveformPlotterModule()
//{
//}

