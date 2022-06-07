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
// Filename : TRGTOPWaveformPlotterModule.h
// Section  : TRG GRL
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : DQM module for TRGTOP
//---------------------------------------------------------------
#ifndef TRGTOPWAVEFORMPLOTTERMODULE_h
#define TRGTOPWAVEFORMPLOTTERMODULE_h

#include <framework/core/Module.h>

#include <vector>

//#include <TFile.h>
//#include <TDirectory.h>

#include <TCanvas.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TAxis.h>
//#include <TLegend.h>
#include <TPaveText.h>

//#include <TTree.h>
//#include <TH1I.h>
//#include <TH2I.h>
//#include <TH1D.h>

//#include <framework/core/HistoModule.h>

#include <string>
#include <algorithm>

#define NUMBER_OF_TOP_SLOTS 16
#define MAX_NUMBER_OF_CLOCK_CYCLES 384

#define MAX_TIMESTAMP_RANGE 47000

namespace Belle2 {

  class TRGTOPWaveformPlotterModule  : public Module {

  public:
    //! Costructor
    TRGTOPWaveformPlotterModule();

    //! Destructor
    virtual ~TRGTOPWaveformPlotterModule() {}
    //    virtual ~TRGTOPWaveformPlotterModule();

  public:

    //! initialize
    virtual void initialize() override;
    //! begin Run
    virtual void beginRun() override;
    //! Event
    virtual void event() override;
    //! End Run
    virtual void endRun() override;
    //! terminate
    //    virtual void terminate() override {}
    virtual void terminate() override;

  private:

    //! TDirectory
    // TDirectory* oldROOTDir = nullptr;
    //! TDirectory
    // TDirectory* myROOTDir = nullptr;

    /** pointer to ROOT file with TTree for efficiecy studies */
    //    TFile* m_file;

    TCanvas* m_myCanvas[NUMBER_OF_TOP_SLOTS];

    TGraph* m_timeStampsGraphMainReadout[NUMBER_OF_TOP_SLOTS];
    TGraph* m_timeStampsGraphTriggerReadout[NUMBER_OF_TOP_SLOTS];

    TMultiGraph* m_myMultiGraph[NUMBER_OF_TOP_SLOTS];

    TPaveText* m_myPaveText[NUMBER_OF_TOP_SLOTS];

    Int_t m_timeStampsMainReadout[NUMBER_OF_TOP_SLOTS][MAX_NUMBER_OF_CLOCK_CYCLES];
    Int_t m_timeStampsTriggerReadout[NUMBER_OF_TOP_SLOTS][MAX_NUMBER_OF_CLOCK_CYCLES];

    Int_t m_clockCyclesMainReadout[NUMBER_OF_TOP_SLOTS][MAX_NUMBER_OF_CLOCK_CYCLES];
    Int_t m_clockCyclesTriggerReadout[NUMBER_OF_TOP_SLOTS][MAX_NUMBER_OF_CLOCK_CYCLES];

    // various parameters and their interpretation

    int m_plottingMode;

    double m_markerSizeHits;
    double m_markerSizeTimestamps;

    int m_markerTypeHits;
    int m_markerTypeTimestamps;

    int m_firstAssumedClockCycle;

    int m_cutPlotMinNumberTriggerReadoutHits;

    int m_cutPlotMinNumberMainReadoutHits;

    int m_xMin;
    int m_xMax;
    int m_yMin;
    int m_yMax;

    int m_showGridx;
    int m_showGridy;

    int m_canvasXMin;
    int m_canvasYMin;
    int m_canvasXSize;
    int m_canvasYSize;

    int m_shiftCanvas;
    int m_xShiftCanvas;
    int m_yShiftCanvas;

  };

}

#endif
