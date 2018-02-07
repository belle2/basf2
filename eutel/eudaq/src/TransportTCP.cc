#include <eutel/eudaq/TransportTCP.h>
#include <eutel/eudaq/Exception.h>
#include <eutel/eudaq/Time.h>
#include <eutel/eudaq/Utils.h>

#if EUDAQ_PLATFORM_IS(WIN32) || EUDAQ_PLATFORM_IS(MINGW)
# include <eutel/eudaq/TransportTCP_WIN32.h>
#else
# include <eutel/eudaq/TransportTCP_POSIX.h>
#endif

#include <sys/types.h>
#include <errno.h>
//#include <unistd.h>

#include <iostream>
#include <ostream>
#include <iostream>

namespace eudaq {

  const std::string TCPServer::name = "tcp";

  namespace {

    static const int MAXPENDING = 16;
    static const int MAX_BUFFER_SIZE = 10000;

    static int to_int(char c)
    {
      return static_cast<unsigned char>(c);
    }

#ifdef MSG_NOSIGNAL
    // On Linux (and cygwin?) send(...) can be told to
    // ignore signals by setting the flag below
    static const int FLAGS = MSG_NOSIGNAL;
    static void setup_signal()
    {
    }
#else
#   include <signal.h>
    // On Mac OS X (BSD?) this flag is not available,
    // so we have to set the signal handler to ignore
    static const int FLAGS = 0;
    static void setup_signal()
    {
      // static so that it is only done once
      static sig_t sig = signal(SIGPIPE, SIG_IGN);
      (void)sig;
    }
#endif

    static void do_send_data(SOCKET sock, const unsigned char* data, size_t len)
    {
      //if (len > 500000) std::cout << "Starting send" << std::endl;
      size_t sent = 0;
      do {
        int result = send(sock,
                          reinterpret_cast<const char*>(data + sent),
                          static_cast<int>(len - sent),
                          FLAGS);
        if (result > 0) {
          sent += result;
        } else if (result < 0 && (LastSockError() == EAGAIN || LastSockError() == EINTR)) {
          // continue
        } else if (result == 0) {
          EUDAQ_THROW("Connection reset by peer");
        } else if (result < 0) {
          EUDAQ_THROW(LastSockErrorString("Error sending data"));
        }
      } while (sent < len);
      //if (len > 500000) std::cout << "Done send" << std::endl;
    }

    static void do_send_packet(SOCKET sock, const unsigned char* data, size_t length)
    {
      //if (length > 500000) std::cout << "Starting send packet" << std::endl;
      if (length < 1020) {
        size_t len = length;
        std::string buffer(len + 4, '\0');
        for (int i = 0; i < 4; ++i) {
          buffer[i] = static_cast<char>(len & 0xff);
          len >>= 8;
        }
        std::copy(data, data + length, &buffer[4]);
        do_send_data(sock, reinterpret_cast<const unsigned char*>(&buffer[0]), buffer.length());
      } else {
        size_t len = length;
        unsigned char buffer[4] = {0};
        for (int i = 0; i < 4; ++i) {
          buffer[i] = static_cast<unsigned char>(len & 0xff);
          len >>= 8;
        }
        do_send_data(sock, buffer, 4);
        do_send_data(sock, data, length);
      }
      //if (length > 500000) std::cout << "Done send packet" << std::endl;
    }

//     static void send_data(SOCKET sock, unsigned long data) {
//       std::string str;
//       for (int i = 0; i < 4; ++i) {
//         str += static_cast<char>(data & 0xff);
//         data >>= 8;
//       }
//       send_data(sock, str);
//     }

  } // anonymous namespace

  bool ConnectionInfoTCP::Matches(const ConnectionInfo& other) const
  {
    const ConnectionInfoTCP* ptr = dynamic_cast<const ConnectionInfoTCP*>(&other);
    //std::cout << " [Match: " << m_fd << " == " << (ptr ? to_string(ptr->m_fd) : "null") << "] " << std::flush;
    if (ptr && (ptr->m_fd == m_fd)) return true;
    return false;
  }

  void ConnectionInfoTCP::Print(std::ostream& os) const
  {
    ConnectionInfo::Print(os);
    os << " (" /*<< m_fd << ","*/ << m_host << ")";
  }

  void ConnectionInfoTCP::append(size_t length, const char* data)
  {
    m_buf += std::string(data, length);
    update_length();
    //std::cout << *this << " - append: " << m_len << ", " << m_buf.size() << std::endl;
  }

  bool ConnectionInfoTCP::havepacket() const
  {
    return m_buf.length() >= m_len + 4;
  }

  std::string ConnectionInfoTCP::getpacket()
  {
    if (!havepacket()) EUDAQ_THROW("No packet available");
    std::string packet(m_buf, 4, m_len);
    m_buf.erase(0, m_len + 4);
    update_length(true);
    //std::cout << *this << " - getpacket: " << m_len << ", " << m_buf.size() << std::endl;
    return packet;
  }

  void ConnectionInfoTCP::update_length(bool force)
  {
    //std::cout << "DBG: len=" << m_len << ", buf=" << m_buf.length();
    if (force || m_len == 0) {
      m_len = 0;
      if (m_buf.length() >= 4) {
        for (int i = 0; i < 4; ++i) {
          m_len |= to_int(m_buf[i]) << (8 * i);
        }
//         std::cout << " (" << to_hex(m_buf[3], 2)
//                   << " " << to_hex(m_buf[2], 2)
//                   << " " << to_hex(m_buf[1], 2)
//                   << " " << to_hex(m_buf[0], 2)
//                   << ")";
      }
    }
    //std::cout << ", len=" << m_len << std::endl;
  }

  TCPServer::TCPServer(const std::string& param)
    : m_port(from_string(param, 44000)),
      m_srvsock(socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)),
      m_maxfd(m_srvsock)
  {
    if (m_srvsock == (SOCKET) - 1) EUDAQ_THROW(LastSockErrorString("Failed to create socket"));
    setup_signal();
    FD_ZERO(&m_fdset);
    FD_SET(m_srvsock, &m_fdset);

    setup_socket(m_srvsock);

    sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(m_port);

    if (bind(m_srvsock, (sockaddr*) &addr, sizeof addr)) {
      closesocket(m_srvsock);
      EUDAQ_THROW(LastSockErrorString("Failed to bind socket: " + param));
    }
    if (listen(m_srvsock, MAXPENDING)) {
      closesocket(m_srvsock);
      EUDAQ_THROW(LastSockErrorString("Failed to listen on socket: " + param));
    }
  }

  TCPServer::~TCPServer()
  {
    for (size_t i = 0; i < m_conn.size(); ++i) {
      ConnectionInfoTCP* inf =
        dynamic_cast<ConnectionInfoTCP*>(m_conn[i].get());
      if (inf && inf->IsEnabled()) {
        closesocket(inf->GetFd());
      }
    }
    closesocket(m_srvsock);
  }

  ConnectionInfoTCP& TCPServer::GetInfo(SOCKET fd) const
  {
    const ConnectionInfoTCP tofind(fd);
    for (size_t i = 0; i < m_conn.size(); ++i) {
      if (tofind.Matches(*m_conn[i]) && m_conn[i]->GetState() >= 0) {
        ConnectionInfoTCP* inf =
          dynamic_cast<ConnectionInfoTCP*>(m_conn[i].get());
        return *inf;
      }
    }
    EUDAQ_THROW("BUG: please report it");
  }

  void TCPServer::Close(const ConnectionInfo& id)
  {
    for (size_t i = 0; i < m_conn.size(); ++i) {
      if (id.Matches(*m_conn[i])) {
        ConnectionInfoTCP* inf =
          dynamic_cast<ConnectionInfoTCP*>(m_conn[i].get());
        if (inf && inf->IsEnabled()) {
          SOCKET fd = inf->GetFd();
          inf->Disable();
          FD_CLR(fd, &m_fdset);
          closesocket(fd);
        }
      }
    }
  }

  void TCPServer::SendPacket(const unsigned char* data, size_t len, const ConnectionInfo& id, bool duringconnect)
  {
    //std::cout << "SendPacket to " << id << std::endl;
    for (size_t i = 0; i < m_conn.size(); ++i) {
      //std::cout << "- " << i << ": " << *m_conn[i] << std::flush;
      if (id.Matches(*m_conn[i])) {
        ConnectionInfoTCP* inf =
          dynamic_cast<ConnectionInfoTCP*>(m_conn[i].get());
        if (inf && inf->IsEnabled() && (inf->GetState() > 0 || duringconnect)) {
          //std::cout << " ok" << std::endl;
          do_send_packet(inf->GetFd(), data, len);
        } //else std::cout << " not quite" << std::endl;
      } //else std::cout << " nope" << std::endl;
    }
  }

  void TCPServer::ProcessEvents(int timeout)
  {
    //std::cout << "DEBUG: Process..." << std::endl;
    Time t_start = Time::Current(), /*t_curr = t_start,*/ t_remain = Time(0, timeout);
    bool done = false;
    bool dbg = true;
    do {
      fd_set tempset;
      memcpy(&tempset, &m_fdset, sizeof(tempset));
      //std::cout << "select timeout=" << t_remain << std::endl;
      timeval timeremain = t_remain;
      int result = select(static_cast<int>(m_maxfd + 1), &tempset, NULL, NULL, &timeremain);
      //std::cout << "select done" << std::endl;

      if (result == 0) {
        //std::cout << "timeout" << std::endl;
      } else if (result < 0 && LastSockError() != EINTR) {
        std::cout << LastSockErrorString("Error in select()") << std::endl;
      } else if (result > 0) {

        if (FD_ISSET(m_srvsock, &tempset)) {
          sockaddr_in addr;
          socklen_t len = sizeof(addr);
          SOCKET peersock = accept(static_cast<int>(m_srvsock), (sockaddr*)&addr, &len);
          if (peersock == INVALID_SOCKET) {
            std::cout << LastSockErrorString("Error in accept()") << std::endl;
          } else {
            //std::cout << "Connect " << peersock << " from " << inet_ntoa(addr.sin_addr) << std::endl;
            FD_SET(peersock, &m_fdset);
            m_maxfd = (m_maxfd < peersock) ? peersock : m_maxfd;
            setup_socket(peersock);
            std::string host = inet_ntoa(addr.sin_addr);
            host += ":" + to_string(ntohs(addr.sin_port));
            counted_ptr<ConnectionInfo> ptr(new ConnectionInfoTCP(peersock, host));
            bool inserted = false;
            for (size_t i = 0; i < m_conn.size(); ++i) {
              if (m_conn[i]->GetState() < 0) {
                m_conn[i] = ptr;
                inserted = true;
              }
            }
            if (!inserted) m_conn.push_back(ptr);
            m_events.push(TransportEvent(TransportEvent::CONNECT, *ptr));
            FD_CLR(m_srvsock, &tempset);
          }
        }
        for (SOCKET j = 0; j < m_maxfd + 1; j++) {
          if (FD_ISSET(j, &tempset)) {
            char buffer[MAX_BUFFER_SIZE + 1];
            do {
              result = recv(j, buffer, MAX_BUFFER_SIZE, 0);
            } while (result == -1 && LastSockError() == EINTR);
            if (result > 0) {
              buffer[result] = 0;
              ConnectionInfoTCP& m = GetInfo(j);
              m.append(result, buffer);
              while (m.havepacket()) {
                done = true;
                m_events.push(TransportEvent(TransportEvent::RECEIVE, m, m.getpacket()));
              }
            } //else /*if (result == 0)*/ {
            if (result == 0) {
              if (dbg) std::cout << "Server #" << j << ", return=" << result << " WSAError:" << LastSockError() << ", --> was closed " <<
                                   std::endl;
              ConnectionInfoTCP& m = GetInfo(j);
              m_events.push(TransportEvent(TransportEvent::DISCONNECT, m));
              m.Disable();
              closesocket(j);
              FD_CLR(j, &m_fdset);
            }
            if (result == -1) {
              if (dbg) std::cout << "Server #" << j << ", return=" << result << " WSAError:" << LastSockError() << std::endl;
            }
          } // end if (FD_ISSET(j, &amp;tempset))
        } // end for (j=0;...)
      } // end else if (result > 0)
      t_remain = Time(0, timeout) + t_start - Time::Current();
    } while (!done && t_remain > Time(0));
  }

  std::string TCPServer::ConnectionString() const
  {
    const char* host = getenv("HOSTNAME");
    if (!host) host = "localhost";
    //gethostname(buf, sizeof buf);
    return name + "://" + host + ":" + to_string(m_port);
  }

  TCPClient::TCPClient(const std::string& param)
    : m_server(param),
      m_port(44000),
      m_sock(socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)),
      m_buf(ConnectionInfoTCP(m_sock, param))
  {
    if (m_sock == (SOCKET) - 1) EUDAQ_THROW(LastSockErrorString("Failed to create socket"));

    size_t i = param.find(':');
    if (i != std::string::npos) {
      m_server = trim(std::string(param, 0, i));
      m_port = from_string(std::string(param, i + 1), 44000);
    }
    if (m_server == "") m_server = "localhost";

    OpenConnection();
  }

  void TCPClient::OpenConnection()
  {
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);

    hostent* host = gethostbyname(m_server.c_str());
    if (!host) {
      closesocket(m_sock);
      EUDAQ_THROW(LastSockErrorString("Error looking up address \'" + m_server + "\'"));
    }
    memcpy((char*) &addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
    if (connect(m_sock, (sockaddr*) &addr, sizeof(addr)) &&
        LastSockError() != EINPROGRESS &&
        LastSockError() != EWOULDBLOCK) {
      EUDAQ_THROW(LastSockErrorString("Are you sure the server is running? - Error "
                                      + to_string(LastSockError()) + " connecting to " +
                                      m_server + ":" + to_string(m_port)));
    }
    setup_socket(m_sock); // set to non-blocking
  }

  void TCPClient::SendPacket(const unsigned char* data, size_t len, const ConnectionInfo& id, bool)
  {
    //std::cout << "Sending packet to " << id << std::endl;
    if (id.Matches(m_buf)) {
      //std::cout << " ok" << std::endl;
      do_send_packet(m_buf.GetFd(), data, len);
    }
    //std::cout << "Sent" << std::endl;
  }

  void TCPClient::ProcessEvents(int timeout)
  {
    //std::cout << "ProcessEvents()" << std::endl;
    Time t_start = Time::Current(), /*t_curr = t_start,*/ t_remain = Time(0, timeout);
    bool done = false;
    bool dbg = false;
    do {
      fd_set tempset;
      FD_ZERO(&tempset);
      FD_SET(m_sock, &tempset);
      timeval timeremain = t_remain;
      SOCKET result = select(static_cast<int>(m_sock + 1), &tempset, NULL, NULL, &timeremain);
      //std::cout << "Select result=" << result << std::endl;

      bool donereading = false;
      do {
        char buffer[MAX_BUFFER_SIZE + 1];
        do {
          result = recv(m_sock, buffer, MAX_BUFFER_SIZE, 0);
        } while (result == (SOCKET) - 1 && LastSockError() == EINTR);
        if (result == (SOCKET) - 1 && LastSockError() == EWOULDBLOCK) {
          if (dbg)  std::cout << "ResultClient = " << result << std::endl;
          donereading = true;
        }
        if (result == -1) {
          if (dbg) std::cout << "Client, return=" << result << " WSAError:" << LastSockError() << ", --> Time out. " << std::endl;
        }
        if (result == 0) {
          if (dbg) std::cout << "Client, return=" << result << " WSAError:" << LastSockError() << ", --> WARN: Connection closed (?) " <<
                               std::endl;
          donereading = true;
          EUDAQ_THROW(LastSockErrorString("SocketClient Error (" + to_string(LastSockError()) + ")"));
        }
        if (result > 0) {
          m_buf.append(result, buffer);
          while (m_buf.havepacket()) {
            m_events.push(TransportEvent(TransportEvent::RECEIVE, m_buf, m_buf.getpacket()));
            done = true;
          }
        }
      } while (!donereading);
      t_remain = Time(0, timeout) + t_start - Time::Current();
      //std::cout << "Remaining: " << t_remain << (t_remain > Time(0) ? " >0" : " <0")<< std::endl;
    } while (!done && t_remain > Time(0));
    //std::cout << "done" << std::endl;
  }

  TCPClient::~TCPClient()
  {
    closesocket(m_sock);
  }

}
