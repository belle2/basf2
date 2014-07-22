//+
// File : rawfile2rb.cc
// Description : Read raw data dump file and put record in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <unistd.h>
#include <cstdlib>

#include <daq/storage/BinData.h>

#include <daq/slc/readout/RunInfoBuffer.h>
#include <daq/slc/readout/ronode_info.h>

#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/system/LogFile.h>

#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>

using namespace Belle2;

int open_server_socket(const std::string& ip, unsigned short port);
int accept_socket(int fd, sockaddr_in& addr);
int connect_socket(const std::string& ip, unsigned short port);
int write_socket(int fd, const void* buf, size_t count);
int read_socket(int fd, void* buf, size_t count);
int select_socket(int fd, int sec, int usec = 0);

void close_socketin(int& sock_in, RunInfoBuffer& info)
{
  close(sock_in);
  info.setInputPort(0);
  info.setInputAddress(0);
  sock_in = -1;
}

void close_socketout(int& sock_out, RunInfoBuffer& info)
{
  close(sock_out);
  info.setOutputPort(0);
  info.setOutputAddress(0);
  sock_out = -1;
}

int main(int argc, char** argv)
{
  if (argc < 6) {
    LogFile::debug("%s <nodename> <nodeid> <remote_host> "
                   "<remote_port> <local_port>", argv[0]);
    return 1;
  }
  RunInfoBuffer info;
  info.open(argv[1], sizeof(ronode_info), false);
  info.init();
  info.clear();
  LogFile::info("starting ro dummy.");
  usleep(500000);
  info.reportRunning();

  int ssock = open_server_socket("0.0.0.0", atoi(argv[5]));

  const size_t buf_size = 1024 * 1024;
  int* buf = new int[buf_size];
  BinData data;
  data.setBuffer(buf);
  int expno = 1;
  int runno = 1;
  int subno = 0;
  int sock_in = -1;
  int sock_out = -1;

  while (true) {
    // accepting connection from reader processes
    while (sock_out <= 0) {
      sockaddr_in sa;
      sock_out = accept_socket(ssock, sa);
      if (sock_out > 0) {
        if (setsockopt(sock_out, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size)) != 0) {
          perror("setsockopt");
        } else {
          memset(&sa, 0, sizeof(sockaddr_in));
          socklen_t sa_len = sizeof(sa);
          if (getsockname(sock_out, (struct sockaddr*)&sa, (socklen_t*)&sa_len) == 0) {
            int port = ntohs(sa.sin_port);
            info.setOutputPort(port);
            info.setOutputAddress(sa.sin_addr.s_addr);
            LogFile::debug("Output host:port=%s:%d",
                           inet_ntoa(sa.sin_addr), port);
            break;
          }
        }
      }
      close_socketout(sock_out, info);
      sleep(5);
    }
    // connecting to sender process
    while (sock_in <= 0) {
      sock_in = connect_socket(argv[3], atoi(argv[4]));
      if (sock_in > 0) {
        if (setsockopt(sock_in, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size)) != 0) {
          perror("setsockopt");
        } else {
          sockaddr_in sa;
          memset(&sa, 0, sizeof(sockaddr_in));
          socklen_t sa_len = sizeof(sa);
          if (getsockname(sock_in, (struct sockaddr*)&sa, (socklen_t*)&sa_len) == 0) {
            int port = ntohs(sa.sin_port);
            info.setInputPort(port);
            info.setInputAddress(sa.sin_addr.s_addr);
            LogFile::debug("Inputput host:port=%s:%d",
                           inet_ntoa(sa.sin_addr), port);
            break;
          }
        }
      }
      close_socketin(sock_in, info);
      sleep(5);
    }
    // ready for data taking
    info.reportReady();
    while (true) {
      int sta = 0;
      int nbyte = 0;
      if ((sta = read_socket(sock_in, buf, sizeof(int))) < 0) {
        close_socketin(sock_in, info);
        break;
      }
      nbyte += sta;
      if (info.getInputNBytes() == 0) {
        info.reportRunning();
      }
      if ((sta = read_socket(sock_in, (buf + 1), (buf[0] - 1) * sizeof(int))) < 0) {
        close_socketin(sock_in, info);
        break;
      }
      nbyte += sta;
      if (expno < data.getExpNumber() || runno < data.getRunNumber() ||
          subno < data.getSubNumber()) {
        // initializing run info for new run
        expno = data.getExpNumber();
        runno = data.getRunNumber();
        subno = data.getSubNumber();
        info.setExpNumber(expno);
        info.setRunNumber(runno);
        info.setSubNumber(subno);
        info.setInputCount(0);
        info.setInputNBytes(0);
        info.setOutputCount(0);
        info.setOutputNBytes(0);
      }
      // incrementing event counter and total number of bytes of read data
      info.addInputCount(1);
      info.addInputNBytes(nbyte);
      if ((sta = write_socket(sock_out, buf, data.getByteSize())) < 0) {
        close_socketout(sock_out, info);
        break;
      }
      info.addOutputCount(1);
      info.addOutputNBytes(sta);
      if (info.getOutputCount() % 10000 == 0) {
        LogFile::info("in:%ld:%d out:%ld:%d", info.getInputNBytes(), info.getInputCount(),
                      info.getOutputNBytes(), info.getOutputCount());
      }
    }
    LogFile::info("disconnected.");
    sleep(5);
  }
  return 0;
}

int open_server_socket(const std::string& ip, unsigned short port)
{
  int fd = 0;
  sockaddr_in addr;
  memset(&addr, 0, sizeof(sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  fd = socket(PF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    fd = 0;
    return -1;
  }
  int enable = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1) {
    fd = 0;
    return -1;
  }
  struct hostent* host = NULL;
  host = gethostbyname(ip.c_str());
  if (host == NULL) {
    unsigned long ip_address = inet_addr(ip.c_str());
    if ((signed long) ip_address < 0) {
      return -1;
    } else {
      host = gethostbyaddr((char*)&ip_address, sizeof(ip_address), AF_INET);
    }
  }
  if (host == NULL) {
    return -1;
  }
  addr.sin_addr.s_addr = (*(unsigned long*) host->h_addr_list[0]);

  if (bind(fd, (const sockaddr*) & (addr), sizeof(sockaddr_in)) != 0) {
    return -1;
  }
  if (listen(fd, 5) != 0) {
    return -1;
  }
  return fd;
}

int accept_socket(int fd, sockaddr_in& addr)
{
  socklen_t len = sizeof(sockaddr_in);
  memset(&addr, 0, sizeof(sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  int cfd;
  errno = 0;
  while (true) {
    if ((cfd = ::accept(fd, (sockaddr*) & (addr), &len)) == -1) {
      switch (errno) {
        case EINTR: continue;
        case EAGAIN: continue;
        default:
          perror("accept");
          return -1;
      }
    }
    break;
  }
  return cfd;
}

int connect_socket(const std::string& ip, unsigned short port)
{
  int socket_fd;
  sockaddr_in addr;
  memset(&addr, 0, sizeof(sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    return -1;
  }
  struct hostent* host = NULL;
  host = gethostbyname(ip.c_str());
  if (host == NULL) {
    unsigned long ip_address = inet_addr(ip.c_str());
    if ((signed long) ip_address < 0) {
      close(socket_fd);
      return -1;
    } else {
      host = gethostbyaddr((char*)&ip_address, sizeof(ip_address), AF_INET);
    }
  }
  addr.sin_addr.s_addr = (*(unsigned long*) host->h_addr_list[0]);

  if (::connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    close(socket_fd);
    return -1;
  }

  return socket_fd;
}

int write_socket(int fd, const void* buf, size_t count)
{
  int ret;
  size_t c = 0;
  while (c < count) {
    errno = 0;
    ret = send(fd, ((unsigned char*)buf + c), (count - c), MSG_NOSIGNAL);
    if (ret <= 0) {
      switch (errno) {
        case EINTR: continue;
        case ENETUNREACH:
        case EHOSTUNREACH:
        case ETIMEDOUT:
          usleep(500);
          continue;
        default:
          perror("send");
          return -1;
      }
    }
    c += ret;
  }
  return c;
}

int read_socket(int fd, void* buf, size_t count)
{
  size_t c = 0;
  int ret;
  while (c < count) {
    errno = 0;
    ret = recv(fd, ((unsigned char*)buf + c), (count - c), 0);
    if (ret <= 0) {
      switch (errno) {
        case EINTR: continue;
        case EAGAIN: continue;
        default:
          perror("recv");
          return -1;
      }
    }
    c += ret;
  }
  return c;
}

int select_socket(int fd, int sec, int usec)
{
  if (fd <= 0) {
    return false;
  }
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);
  int ret;
  if (sec >= 0 && usec >= 0) {
    timeval t = {sec, usec};
    ret = ::select(FD_SETSIZE, &fds, NULL, NULL, &t);
  } else {
    ret = ::select(FD_SETSIZE, &fds, NULL, NULL, NULL);
  }
  if (ret < 0) {
    perror("select");
    return -1;
  }
  if (FD_ISSET(fd, &fds)) {
    return fd;
  } else {
    return 0;
  }
}
