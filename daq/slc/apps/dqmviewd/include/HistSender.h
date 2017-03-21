#ifndef _Belle2_HistSender
#define _Belle2_HistSender

#include <daq/slc/system/TCPSocket.h>

#include <daq/slc/base/Writer.h>

#include <TH1.h>

#include <vector>

namespace Belle2 {

  class DQMViewCallback;
  class DQMFileReader;

  class HistSender {

  private:
    static const int FLAG_CONFIG;
    static const int FLAG_UPDATE;
    static const int FLAG_LIST;

  public:
    HistSender(TCPSocket& socket, DQMViewCallback* callback)
      : m_callback(callback), m_socket(socket) {}
    ~HistSender();

  public:
    bool update(std::vector<TH1*>& hist);

  private:
    DQMViewCallback* m_callback;
    TCPSocket m_socket;

  };

}

#endif
