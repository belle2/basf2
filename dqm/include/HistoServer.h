#ifndef HISTOSERVER_H
#define HISTOSERVER_H
//+
// File : HistoServer.h
// Description : Histogram Server
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 18 - Dec - 2012
//-

#include <string>

#include <dqm/SocketLib.h>
#include <dqm/EvtSocket.h>
#include <dqm/EvtSocketManager.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

#include <dqm/DqmMemFile.h>

#include <dqm/HistoManager.h>

#define MAXBUFSIZE 160000000

#define MAPFILESIZE 512000000

// interval to invoke histogram merge (in msec*2)
#define MERGE_INTERVAL 5000

namespace Belle2 {

  class HistoServer {
  public:
    HistoServer(int port, std::string& mapfile);
    ~HistoServer();

    int init();

    int server();  // Loop

  private:
    EvtSocketRecv* m_sock;
    EvtSocketManager* m_man;
    int m_port;
    int m_force_exit;

  private:
    std::string& m_filename;
    DqmMemFile* m_memfile;
    HistoManager* m_hman;
  };
}
#endif

