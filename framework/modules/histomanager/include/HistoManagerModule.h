//+
// File : HistoManager.h
// Description : A module to manage histograms/ntuples/ttrees
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 27 - Jul - 2010
//-

#pragma once

#include <framework/core/Module.h>

#include <string>

namespace Belle2 {
  /** Class definition of HistoManager module */
  class HistoManagerModule : public Module {
  public:

    //! Constructor and Destructor
    HistoManagerModule();
    virtual ~HistoManagerModule();

    //! module functions
    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void event();
    virtual void terminate();

  private:
    std::string m_workdir; /**< Name of working directory. */
    std::string m_histoFileName; /**< Name of histogram output file. */
    bool        m_initmain; /**< True if initialize() was called. */
    bool        m_tupleManagerInitialized; /**< True if RbTupleManager was initialized. */
  };
} // Namespace Belle2
