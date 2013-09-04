#ifndef HISTORELAY_H
#define HISTORELAY_H
//+
// File : HistoRelay.h
// Description : Read Histograms in a TMapFile periodically and
//               relay them to HistoServer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 3 - Sep - 2013
//-

#include <string>

#include "TH1.h"
#include "TMapFile.h"

#include <daq/dataflow/EvtSocket.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/EvtMessage.h>

namespace Belle2 {
  class HistoRelay {
  public:
    HistoRelay(std::string& filename, std::string& dest, int port);
    ~HistoRelay();

    int collect();
  private:
    TMapFile* m_map;
    EvtSocketSend* m_sock;
    MsgHandler* m_msg;

  };
}

#endif

