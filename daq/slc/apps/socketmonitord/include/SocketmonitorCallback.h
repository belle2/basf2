#ifndef _Belle2_SocketmonitorCallback_h
#define _Belle2_SocketmonitorCallback_h

#include "daq/slc/nsm/NSMCallback.h"

#include "daq/slc/readout/IOInfo.h"

#include "daq/slc/base/IOException.h"

namespace Belle2 {

  class SocketmonitorCallback : public NSMCallback {

  public:
    SocketmonitorCallback(const std::string& nodename,
                          int timout = 5);
    virtual ~SocketmonitorCallback() throw();

  public:
    void addSocket(const std::string& hostname,
                   int port, bool islocal) throw(IOException);

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();

  private:
    std::vector<bool> m_islocal;
    std::vector<std::string> m_hostname;
    std::vector<IOInfo> m_info;

  };

}

#endif
