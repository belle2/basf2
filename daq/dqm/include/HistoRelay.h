/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef HISTORELAY_H
#define HISTORELAY_H

#include <string>

#include <daq/dataflow/EvtSocket.h>
#include <daq/dqm/DqmMemFile.h>

namespace Belle2 {
  class HistoRelay {
  public:
    HistoRelay(std::string& filename, std::string& dest, int port);
    ~HistoRelay();

    int collect();
  private:
    std::string m_filename;
    DqmMemFile* m_memfile;
    EvtSocketSend* m_sock;
    MsgHandler* m_msg;

  };
}

#endif

