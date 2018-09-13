//+
// File : DQMHistSnapshots.h
// Description :
//
// Author : Boqun Wang, U. of Cincinnati
// Date : yesterday
//-

#pragma once

// EPICS
#ifdef _BELLE2_EPICS
#include "cadef.h"
// #include "dbDefs.h"
// #include "epicsString.h"
// #include "cantProceed.h"
#endif

#include <framework/core/Module.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TFile.h>
#include <time.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistSnapshotsModule : public DQMHistAnalysisModule {

    typedef struct {
      TH1* histo;
      TCanvas* canvas;
    } SSNODE;

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistSnapshotsModule();
    virtual ~DQMHistSnapshotsModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();
    SSNODE* find_snapshot(TString a);
    TCanvas* find_canvas(TString s);

    // Data members
  private:
    /** Struct for extracted parameters */
    std::vector<SSNODE*> m_ssnode;
    int m_check_interval;
    time_t m_last_check;

    TH1* GetHisto(TString histoname);

  };
} // end namespace Belle2

