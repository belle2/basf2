/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <string>
#include <TDirectory.h>

#include <daq/dataflow/EvtSocket.h>
#include <framework/pcore/MsgHandler.h>

namespace Belle2 {
  class HistoRelay2 {
  public:
    HistoRelay2(std::string& filename, std::string& dest, int port);
    ~HistoRelay2();
    /** Copy constructor (disabled). */
    HistoRelay2(const HistoRelay2&) = delete;

    /** Operator = (disabled). */
    HistoRelay2& operator=(const HistoRelay2&) = delete;

    int collect();
  private:
    std::string m_filename;
    EvtSocketSend* m_sock;
    std::string m_dest;
    int m_port;

    static EvtMessage* StreamFile(std::string& filename);
    static int StreamHistograms(TDirectory* curdir, MsgHandler* msg, int& numobjs);

  };
}

