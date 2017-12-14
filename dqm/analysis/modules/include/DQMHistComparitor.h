//+
// File : DQMHistComparitor.h
// Description :
//
// Author : Bjoern Spruck, Uni Mainz
// Date : yesterday
//-

#ifndef _Belle2_DQMHistComparitor_h
#define _Belle2_DQMHistComparitor_h

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

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistComparitorModule : public DQMHistAnalysisModule {

    typedef struct {
#ifdef _BELLE2_EPICS
      chid    mychid;
#endif
      bool epicsflag;
      TString histo1;
      TString histo2;
      TCanvas* canvas;
      float warning;
      float error;
      int min_entries;
    } CMPNODE;

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistComparitorModule();
    virtual ~DQMHistComparitorModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:
    /** Parameter list for histograms */
    std::vector< std::vector<std::string>> m_histlist;
    /** Struct for extracted parameters + EPICS PV */
    std::vector<CMPNODE*> m_pnode;
    /** Reference Histogram Root file name */
    std::string m_refFileName;
    /** The pointer to the reference file */
    TFile* m_refFile;

    TH1* GetHisto(TString histoname);

  };
} // end namespace Belle2

#endif
