#define MSG_NOSIGNAL 0
#ifndef EINPROGRESS
#define EINPROGRESS WSAEINPROGRESS
#endif
#ifndef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif

#include <map>
#if EUDAQ_PLATFORM_IS(MINGW)
#  include <windows.h>
#else
//#  include <afxwin.h>
#endif
#include <winsock.h>

namespace eudaq {

  namespace {

    // List of Winsock error constants mapped to an interpretation string.
    // Note that this list must remain sorted by the error constants'
    // values, because we do a binary search on the list when looking up
    // items.
    std::pair<int, std::string> g_ErrorList[] = {
      std::make_pair(0,                  "No error"),
      std::make_pair(WSAEINTR,           "Interrupted system call"),
      std::make_pair(WSAEBADF,           "Bad file number"),
      std::make_pair(WSAEACCES,          "Permission denied"),
      std::make_pair(WSAEFAULT,          "Bad address"),
      std::make_pair(WSAEINVAL,          "Invalid argument"),
      std::make_pair(WSAEMFILE,          "Too many open sockets"),
      std::make_pair(WSAEWOULDBLOCK,     "Operation would block"),
      std::make_pair(WSAEINPROGRESS,     "Operation now in progress"),
      std::make_pair(WSAEALREADY,        "Operation already in progress"),
      std::make_pair(WSAENOTSOCK,        "Socket operation on non-socket"),
      std::make_pair(WSAEDESTADDRREQ,    "Destination address required"),
      std::make_pair(WSAEMSGSIZE,        "Message too long"),
      std::make_pair(WSAEPROTOTYPE,      "Protocol wrong type for socket"),
      std::make_pair(WSAENOPROTOOPT,     "Bad protocol option"),
      std::make_pair(WSAEPROTONOSUPPORT, "Protocol not supported"),
      std::make_pair(WSAESOCKTNOSUPPORT, "Socket type not supported"),
      std::make_pair(WSAEOPNOTSUPP,      "Operation not supported on socket"),
      std::make_pair(WSAEPFNOSUPPORT,    "Protocol family not supported"),
      std::make_pair(WSAEAFNOSUPPORT,    "Address family not supported"),
      std::make_pair(WSAEADDRINUSE,      "Address already in use"),
      std::make_pair(WSAEADDRNOTAVAIL,   "Can't assign requested address"),
      std::make_pair(WSAENETDOWN,        "Network is down"),
      std::make_pair(WSAENETUNREACH,     "Network is unreachable"),
      std::make_pair(WSAENETRESET,       "Net connection reset"),
      std::make_pair(WSAECONNABORTED,    "Software caused connection abort"),
      std::make_pair(WSAECONNRESET,      "Connection reset by peer"),
      std::make_pair(WSAENOBUFS,         "No buffer space available"),
      std::make_pair(WSAEISCONN,         "Socket is already connected"),
      std::make_pair(WSAENOTCONN,        "Socket is not connected"),
      std::make_pair(WSAESHUTDOWN,       "Can't send after socket shutdown"),
      std::make_pair(WSAETOOMANYREFS,    "Too many references, can't splice"),
      std::make_pair(WSAETIMEDOUT,       "Connection timed out"),
      std::make_pair(WSAECONNREFUSED,    "Connection refused"),
      std::make_pair(WSAELOOP,           "Too many levels of symbolic links"),
      std::make_pair(WSAENAMETOOLONG,    "File name too long"),
      std::make_pair(WSAEHOSTDOWN,       "Host is down"),
      std::make_pair(WSAEHOSTUNREACH,    "No route to host"),
      std::make_pair(WSAENOTEMPTY,       "Directory not empty"),
      std::make_pair(WSAEPROCLIM,        "Too many processes"),
      std::make_pair(WSAEUSERS,          "Too many users"),
      std::make_pair(WSAEDQUOT,          "Disc quota exceeded"),
      std::make_pair(WSAESTALE,          "Stale NFS file handle"),
      std::make_pair(WSAEREMOTE,         "Too many levels of remote in path"),
      std::make_pair(WSASYSNOTREADY,     "Network system is unavailable"),
      std::make_pair(WSAVERNOTSUPPORTED, "Winsock version out of range"),
      std::make_pair(WSANOTINITIALISED,  "WSAStartup not yet called"),
      std::make_pair(WSAEDISCON,         "Graceful shutdown in progress"),
      std::make_pair(WSAHOST_NOT_FOUND,  "Host not found"),
      std::make_pair(WSANO_DATA,         "No host data of that type was found")
    };
    static const int k_NumMessages = sizeof g_ErrorList / sizeof * g_ErrorList;
    std::map<int, std::string> g_ErrorCodes(g_ErrorList,
                                            g_ErrorList + k_NumMessages);

    typedef int socklen_t;

    static int LastSockError()
    {
      return WSAGetLastError();
    }

    static inline std::string LastSockErrorString(const std::string& msg)
    {
      std::map<int, std::string>::const_iterator it = g_ErrorCodes.find(WSAGetLastError());
      std::string err = (it == g_ErrorCodes.end() ? "Unknown Error" : it->second);
      return msg + ": " + err;
    }

    static void setup_socket(SOCKET sock)
    {
      unsigned long one = 1;
      ioctlsocket(sock, FIONBIO, &one);
    }

    class WSAHelper {
    public:
      WSAHelper()
      {
        WSAData wsadata;
        WSAStartup(MAKEWORD(2, 2), &wsadata);
      }
      ~WSAHelper()
      {
        WSACleanup();
      }
    };
    static WSAHelper theHelper;


  }

}
