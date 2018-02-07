#ifndef EUDAQ_INCLUDED_TransportTCP
#define EUDAQ_INCLUDED_TransportTCP

#include <eutel/eudaq/TransportFactory.h>
#include <eutel/eudaq/Platform.h>

#if EUDAQ_PLATFORM_IS(WIN32) || EUDAQ_PLATFORM_IS(MINGW)
# include <winsock2.h>
#else
# include <sys/select.h>
typedef int SOCKET;
#endif

#include <vector>
#include <string>
#include <map>

namespace eudaq {

  class ConnectionInfoTCP : public ConnectionInfo {
  public:
    ConnectionInfoTCP(SOCKET fd, const std::string& host = "") : m_fd(fd), m_host(host), m_len(0), m_buf("") {}
    void append(size_t length, const char* data);
    bool havepacket() const;
    std::string getpacket();
    SOCKET GetFd() const { return m_fd; }
    void Disable() { m_state = -1; m_len = 0; m_buf = ""; }
    virtual bool Matches(const ConnectionInfo& other) const;
    virtual void Print(std::ostream&) const;
    virtual std::string GetRemote() const { return m_host; }
    virtual ConnectionInfo* Clone() const { return new ConnectionInfoTCP(*this); }
  private:
    void update_length(bool = false);
    SOCKET m_fd;
    std::string m_host;
    size_t m_len;
    std::string m_buf;
  };

  class TCPServer : public TransportServer {
  public:
    TCPServer(const std::string& param);
    virtual ~TCPServer();

    virtual void Close(const ConnectionInfo& id);
    virtual void SendPacket(const unsigned char* data, size_t len,
                            const ConnectionInfo& id = ConnectionInfo::ALL,
                            bool duringconnect = false);
    virtual void ProcessEvents(int timeout);

    virtual std::string ConnectionString() const;
    static const std::string name;
  private:
    int m_port;
    SOCKET m_srvsock;
    SOCKET m_maxfd;
    fd_set m_fdset;

    ConnectionInfoTCP& GetInfo(SOCKET fd) const;
    //typedef std::map<int, TCPConnection> map_t;
    //map_t m_map;
  };

  class TCPClient: public TransportClient {
  public:
    TCPClient(const std::string& param);
    virtual ~TCPClient();

    virtual void SendPacket(const unsigned char* data, size_t len,
                            const ConnectionInfo& id = ConnectionInfo::ALL,
                            bool = false);
    virtual void ProcessEvents(int timeout = -1);
  private:
    void OpenConnection();
    std::string m_server;
    int m_port;
    SOCKET m_sock;
    ConnectionInfoTCP m_buf;
  };

}

#endif // EUDAQ_INCLUDED_TransportTCP
