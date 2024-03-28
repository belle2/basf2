/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>

#include <daq/dataflow/EvtSocket.h>
#include <daq/dataflow/EvtSocketManager.h>

#include <daq/dqm/HistoManager2.h>


namespace Belle2 {

  class HistoServer2 {
  public:
    enum {c_maxBufSize = 160000000 };
// interval to invoke histogram merge (in msec*2)
    enum { c_mergeIntervall = 5000 };

  public:
    HistoServer2(int port, const std::string& mapfile);
    ~HistoServer2();

    int init();

    int server();  // Loop

  private:
    EvtSocketRecv* m_sock;
    EvtSocketManager* m_man;
    int m_port;
    int m_force_exit;

  private:
    std::string m_filename;
    HistoManager2* m_hman;
  };
}

