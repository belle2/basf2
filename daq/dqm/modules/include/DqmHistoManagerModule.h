#ifndef DQM_HISTO_MANAGER_H
#define DQM_HISTO_MANAGER_H
//+
// File : DqmHistoManager.h
// Description : A module to manage histograms/ntuples/ttrees for DQM
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 27 - Jul - 2010
//-


#include <framework/core/Module.h>
#include <daq/dataflow/EvtSocket.h>

#include <framework/core/Environment.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/RbTuple.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/EvtMessage.h>

#include "TChain.h"
#include "TFile.h"
#include "TH1.h"
#include "TTree.h"
#include "TKey.h"
#include "TIterator.h"
#include "TDirectory.h"

#include <string>
#include <iostream>


#define DQM_SOCKET 9899

namespace Belle2 {

  /*! Class definition of DqmHistoManager module */
  class DqmHistoManagerModule : public Module {
  public:

    //! Constructor and Destructor
    DqmHistoManagerModule();
    virtual ~DqmHistoManagerModule();

    //! module functions
    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void event();
    virtual void terminate();

  private:
    int StreamHistograms(TDirectory*, MsgHandler*);

  private:
    std::string m_workdir;  /**< Name of working directory */
    std::string m_histfile; /**< Name of histogram output file. */
    std::string m_hostname; /**< Host name to send histograms */
    int m_port;
    int m_interval;
    int m_dumpinterval;

    bool        m_initmain; /**< True if initialize() was called. */
    bool        m_initialized;
    int m_nevent;

    // Socket interface
    EvtSocketSend*  m_sock;
    MsgHandler*     m_msg;
    int             m_nobjs;



  };
} // Namaspace Belle2

#endif /* HISTO_MANAGER_H */
