//+
// File : DQMHistAnalysisOut.h
// Description : DQM Analysis, dump histograms to file (as reference histograms)
//
// Author : B. Spruck
// Date : 11 - Dec - 2017 ; first commit
//-

#ifndef _Belle2_DQMHistAnalysisOutputFile_h
#define _Belle2_DQMHistAnalysisOutputFile_h

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <string>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisOutputFileModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisOutputFileModule();
    virtual ~DQMHistAnalysisOutputFileModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:

    std::string m_filename;//! file name of root file
    std::string m_histogramDirectoryName;//! Directory name within root file where to place things
    bool m_saveHistos;//! Write all Histos to file
    bool m_saveCanvases;//! Write all Canvases to file
  };
} // end namespace Belle2

#endif
