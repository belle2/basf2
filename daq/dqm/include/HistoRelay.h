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

