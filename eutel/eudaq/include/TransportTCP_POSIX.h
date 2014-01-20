#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

namespace eudaq {

  namespace {

    static inline void closesocket(SOCKET s)
    {
      close(s);
    }

    static inline int LastSockError()
    {
      return errno;
    }

    static inline std::string LastSockErrorString(const std::string& msg)
    {
      return msg + ": " + strerror(errno);
    }

    static void setup_socket(SOCKET sock)
    {
      /// Set non-blocking mode
      int iof = fcntl(sock, F_GETFL, 0);
      if (iof != -1)
        fcntl(sock, F_SETFL, iof | O_NONBLOCK);

      /// Allow the socket to rebind to an address that is still shutting down
      /// Useful if the server is quickly shut down then restarted
      int one = 1;
      setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);

      /// Periodically send packets to keep the connection open
      setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof one);

      /// Try to send any remaining data when a socket is closed
      linger ling;
      ling.l_onoff = 1; ///< Enable linger mode
      ling.l_linger = 1; ///< Linger timeout in seconds
      setsockopt(sock, SOL_SOCKET, SO_LINGER, &ling, sizeof ling);
    }

  }

}
