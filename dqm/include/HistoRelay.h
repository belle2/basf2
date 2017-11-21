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
#include "TMemFile.h"

#include <dqm/EvtSocket.h>
#include <dqm/DqmMemFile.h>

namespace Belle2 {
  class HistoRelay {
  public:
    HistoRelay(std::string& filename, std::string& dest, int port);
    HistoRelay(const HistoRelay& hr);
    ~HistoRelay();
    HistoRelay& operator=(const HistoRelay& hr);

    int collect();
  private:
    std::string& m_filename;
    std::string& m_dest;
    int m_port;
    DqmMemFile* m_memfile;
    EvtSocketSend* m_sock;
    MsgHandler* m_msg;

  };
}

#endif

