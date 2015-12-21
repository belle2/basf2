//+
// File : storageoutput.h
// Description : Sequential ROOT I/O output module for DAQ
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//        9  - Dec - 2013 ; update for DAQ
//-

#ifndef _Belle2_DQMHistAnalysisOutput_h
#define _Belle2_DQMHistAnalysisOutput_h

#include <framework/core/Module.h>
#include <daq/slc/apps/dqmviewd/modules/DQMHistAnalysis.h>

#include <string>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisOutputModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisOutputModule();
    virtual ~DQMHistAnalysisOutputModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:

  };
} // end namespace Belle2

#endif
