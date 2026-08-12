/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef DQM_HISTO_MANAGER_H
#define DQM_HISTO_MANAGER_H

#include <framework/core/Module.h>
#include <daq/dataflow/EvtSocket.h>

#include <framework/pcore/MsgHandler.h>

#include <TH1.h>
#include <TDirectory.h>

#include <string>
#include <time.h>

#define DQM_SOCKET 9899

namespace Belle2 {

  /*! Class definition of DqmHistoManager module */
  class DqmHistoManagerModule : public Module {
  public:

    //! Constructor and Destructor
    DqmHistoManagerModule();
    virtual ~DqmHistoManagerModule() override;

    //! module functions
    void initialize() override;
    void beginRun() override;
    void endRun() override;
    void event() override;
    void terminate() override;

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
    int         m_nevent = 0;
    time_t      m_ptime = 0;
    time_t      m_dtime = 0;
    time_t      m_pstep = 0;
    time_t      m_dstep = 0;


    // Socket interface
    EvtSocketSend*  m_sock = nullptr;
    MsgHandler*     m_msg = nullptr;
    int             m_nobjs = 0;



  };
} // Namaspace Belle2

#endif /* HISTO_MANAGER_H */
