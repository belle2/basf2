#ifndef EUDAQ_INCLUDED_TransportNULL
#define EUDAQ_INCLUDED_TransportNULL

#include <eutel/eudaq/TransportFactory.h>

#include <vector>
#include <string>
#include <map>

namespace eudaq {

  class NULLServer : public TransportServer {
  public:
    NULLServer(const std::string& param);
    virtual ~NULLServer();

    virtual void Close(const ConnectionInfo& id);
    virtual void SendPacket(const unsigned char* data, size_t len,
                            const ConnectionInfo& id = ConnectionInfo::ALL,
                            bool duringconnect = false);
    virtual void ProcessEvents(int timeout);

    virtual std::string ConnectionString() const;
    virtual bool IsNull() const { return true; }
    static const std::string name;
  private:
  };

  class NULLClient: public TransportClient {
  public:
    NULLClient(const std::string& param);
    virtual ~NULLClient();

    virtual void SendPacket(const unsigned char* data, size_t len,
                            const ConnectionInfo& id = ConnectionInfo::ALL,
                            bool = false);
    virtual void ProcessEvents(int timeout = -1);
    virtual bool IsNull() const { return true; }
  private:
    ConnectionInfo m_buf;
  };

}

#endif // EUDAQ_INCLUDED_TransportNULL
