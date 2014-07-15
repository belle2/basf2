#ifndef _NEIGHBOR_H_
#define _NEIGHBOR_H_

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// #include <assert.h>
#include <netdb.h>
#include <poll.h>

#include "log.h"

#include "strferror.h"

class neighbor {
protected:
  int m_fd; /* original file descriptor */
  int m_fd_duped; /* duped fd, used by m_fp */
  FILE* m_fp;
  char m_remote_host[1024];
  char m_remote_port[64];
  const char* m_mode;
  int m_cleared;
  char m_vbuf[40 * 1024 * 1024]; /* vbuf */

  void clear_remote_info() {
    memset(&m_remote_host, 0, sizeof(m_remote_host));
    memset(&m_remote_port, 0, sizeof(m_remote_port));
  };

  void extend_vbuf() {
    // log("setvbuf for %d to %d bytes\n", fileno(m_fp), sizeof(m_vbuf));
    int ret = setvbuf(m_fp, m_vbuf, _IOFBF, sizeof(m_vbuf));
    if (ret != 0) {
      log("setvbuf failed");
      exit(1);
    }
  };

  const char* strferror(FILE* fp, const char* mode) const {
    return ::strferror(fp, mode);
  };

  const char* strferror() const {
    return ::strferror(m_fp, m_mode);
  }

public:

  neighbor() {
    m_fd = -1;
    m_fp = NULL;
    m_cleared = 0;
    clear_remote_info();
  };

  ~neighbor() {
    if (m_fp) {
      fclose(m_fp);
      m_fp = NULL;
    }
  };

  void open(const char* host_and_port,
            const char* mode = "r",
            int window_size = -1,
            int timeout = 30) {

    m_mode = mode;

    char* p = strdup(host_and_port);
    char* q = rindex(p, ':');
    if (q == NULL) {
      log("no port number in %s\n", host_and_port);
      exit(1);
    }
    char* port_str = q + 1;
    *q = '\0';
    char* host_str = p;

    strncpy(m_remote_host, host_str, sizeof(m_remote_host));
    strncpy(m_remote_port, port_str, sizeof(m_remote_port));

    free(p);

    addrinfo* res = NULL, hint;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;

    int ret;

    ret = getaddrinfo(m_remote_host, m_remote_port, &hint, &res);
    if (ret != 0) {
      log("getaddrinfo of %s:%s failed: (%d) = %s\n",
          m_remote_host, m_remote_port, ret, gai_strerror(ret));
      exit(1);
    }


    m_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (m_fd < 0) {
      log("failed to create socket: %s\n", strerror(errno));
      exit(1);
    }

    if (window_size != -1) {
      ret = setsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, &window_size, sizeof(window_size));
      if (ret) {
        log("failed to SO_SNDBUF: %s\n", strerror(errno));
      }
      ret = setsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, &window_size, sizeof(window_size));
      if (ret) {
        log("failed to SO_RCVBUF: %s\n", strerror(errno));
      }
    }

    for (int i = 0; i < timeout; i++) {
      ret = connect(m_fd, res->ai_addr, res->ai_addrlen);
      if (ret == 0)
        break;

      log("failed to connect %s (%d/%d): %s\n", host_and_port, i, timeout, strerror(errno));
      poll(NULL, 0, 1000);
    }

    freeaddrinfo(res);

    if (ret != 0) {
      log("give up to connect %s\n", host_and_port);
      exit(0);
    }
    log("connected %s:%s\n", m_remote_host, m_remote_port);

    m_fd_duped = dup(m_fd);
    if (m_fd_duped == -1) {
      log("failed to dup %d: %s\n", m_fd, strerror(errno));
      exit(1);
    }

    m_fp = fdopen(m_fd_duped, mode);
#if 0
    log("new m_fp = %x\n", m_fp);
#endif

    extend_vbuf();

  };

  void clear_input() {
    int fd = fileno(m_fp);

    if (fd != m_fd_duped) {
      log("m_fd_duped (%d) != fileno(m_fp) (%d)\n", m_fd_duped, fd);
      exit(1);
    }

    pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN;

    while (1) {
      int ret = poll(fds, 1, 0);
      if (ret < 0) {
        /* poll failed. should not be occur... */
        log("poll failed: %s\n", strerror(errno));
        exit(1);
        break;
      }

      if (!(fds[0].revents & POLLIN)) {
        /* no data in socket */
        break;
      }

      char ch;
      ret = read(fd, &ch, sizeof(ch));
      if (ret == 0) {
        /* EOF. socket is closed */
        break;
      }
    }
    close(fd);
    m_fd_duped = -1;
    m_cleared = 1;
  };

  void reopen() {
    log("old m_fp = %x\n", m_fp);
    fclose(m_fp);
    m_fp = NULL;
    m_fd_duped = dup(m_fd);
    if (m_fd_duped == -1) {
      log("failed to dup %d: %s\n", m_fd, strerror(errno));
      exit(0);
    }
    m_fp = fdopen(m_fd_duped, m_mode);
    log("new m_fp = %x\n", m_fp);
    m_cleared = 0;

    extend_vbuf();
  };

  void open(int fd, const char* mode = "w") {
    m_fd = fd;
    m_fd_duped = dup(fd);
    m_fp = fdopen(m_fd_duped, mode);
    if (m_fp == NULL) {
      log("failed to fdopen: %s\n", strerror(errno));
      exit(1);
    }
    log("new m_fp = %x\n", m_fp);
  };

  const char* remote() {
    int ret;
    if (m_remote_host[0] != '\0')
      return m_remote_host;

    sockaddr sa;
    socklen_t sa_len = sizeof(sa);
    ret = getpeername(m_fd, &sa, &sa_len);

    if (ret) {
      log("failed to getpeername %d: %s\n", m_fd, strerror(errno));
      clear_remote_info();
      return m_remote_host;
    }

    if (sa_len != sizeof(sockaddr_in) &&
        sa_len != sizeof(sockaddr_in6)) {
      log("socklen of fd(%d) is %d\n", m_fd, sa_len);
      clear_remote_info();
      return m_remote_host;
    }

    ret = getnameinfo(&sa, sa_len,
                      m_remote_host, sizeof(m_remote_host),
                      m_remote_port, sizeof(m_remote_port),
                      NI_NUMERICHOST | NI_NUMERICSERV);

    if (ret != 0) {
      log("getnameinfo failed: %s\n", gai_strerror(ret));
      clear_remote_info();
    }

    return m_remote_host;
  };

  const char* const remote_host() const {
    return m_remote_host;
  };

  const char* const remote_port() const {
    return m_remote_port;
  };

  FILE* file() {
    return m_fp;
  };

  bool is_opened() const {
    return (m_fd != -1);
  };

};

#endif
