#ifndef _Belle2_GUICommunicator_hh
#define _Belle2_GUICommunicator_hh

#include "system/Mutex.h"
#include "system/TCPSocketWriter.h"
#include "system/TCPSocketReader.h"

#include "MessageBox.h"

#include <list>

namespace Belle2 {

  class GUICommunicator {

  public:
    typedef std::list<GUICommunicator*> GUICommunicatorList;
    static void push(const LogMessage& msg);

  private:
    static GUICommunicatorList __comm_l;
    static Mutex __mutex;

  public:
    GUICommunicator(const TCPSocket& socket);
    ~GUICommunicator() throw();

  public:
    void run();
    bool isReady() { return _is_ready; }

  private:
    TCPSocket _socket;
    TCPSocketWriter _writer;
    TCPSocketReader _reader;
    Mutex _mutex;
    MessageBox _box;
    bool _is_ready;
  };

}

#endif

