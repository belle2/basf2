#ifndef _Belle2_HistSender
#define _Belle2_HistSender

#include <daq/slc/system/TCPSocket.h>

#include <daq/slc/base/Writer.h>

namespace Belle2 {

  class DQMViewMaster;
  class DQMFileReader;

  class HistSender {

  private:
    static const int FLAG_CONFIG;
    static const int FLAG_UPDATE;
    static const int FLAG_LIST;

  public:
    HistSender(TCPSocket& socket, DQMViewMaster& master)
      : _master(master), _socket(socket) {}
    ~HistSender();

  public:
    void run();

  private:
    void sendContents(DQMFileReader& reader, Writer& writer)
    throw(IOException);


  private:
    DQMViewMaster& _master;
    TCPSocket _socket;

  };

}

#endif
