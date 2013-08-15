#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int total_size;
int max_event_size = 1 * 1024 * 1024;
int nstream = 1;
int baseport_from = 32000;
int port_to = 65000;
int interval = 1000;
int vbuf_size = 0;
int discard = 0;

int fd_to = -1;
FILE* fp_to = NULL;
char* vbuf_to = NULL;

class connection {
private:
  int fd_listen; /* socket */
  int fd_from; /* socket */
  FILE* fp_from;
  FILE* fp_to;
  int port_from;
  char* vbuf_from;

  void clear() {
    fd_listen = -1;
    fd_from = -1;
    fp_from = NULL;
    fp_to = NULL;
    port_from = -1;
  };

public:
  connection() {
    clear();
    vbuf_from = NULL;
  };

  ~connection() {
    fclose(fp_from);
    close(fd_from);
    if (vbuf_from)
      delete [] vbuf_from;
    clear();
  };

  void init(int from) {
    clear();
    port_from = from;
  };

  int fread(char* buffer, int len) {
    return ::fread(buffer, len, 1, fp_from);
  };

  int fwrite(char* buffer, int len) {
    assert(fp_to);
    return ::fwrite(buffer, len, 1, fp_to);
  };

  int copy(char* buffer, int len) {
    int ret;
    ret = fread(buffer, len);
    if (ret != 1 && feof(fp_from)) {
      return 0;
    }
    fwrite(buffer, len);
    return len;
  };

  int listen() {
    struct addrinfo* res = 0, hint;

    memset(&hint, 0, sizeof(hint));

    char portstr[64];

    snprintf(portstr, sizeof(portstr), "%d", port_from);

    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    int ret;

    ret = getaddrinfo(NULL, portstr, &hint, &res);
    assert(ret == 0);

    fd_listen = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    assert(fd_listen != -1);

    const int on = 1;

    ret = setsockopt(fd_listen, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    assert(ret == 0);

    ret = setsockopt(fd_listen, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    assert(ret == 0);

    ret = bind(fd_listen, res->ai_addr, res->ai_addrlen);
    assert(ret == 0);

    ret = ::listen(fd_listen, 1);
    assert(ret == 0);

    return fd_listen;
  };

  int accept() {
    int fd_from = ::accept(fd_listen, 0, 0);
    assert(fd_from != -1);
    close(fd_listen);
    fp_from = fdopen(fd_from, "r");
    assert(fp_from);

    if (vbuf_size) {
      int ret;
      if (vbuf_size == -1) {
        ret = setvbuf(fp_from, 0, _IONBF, 0);
      } else {
        vbuf_from = new char[vbuf_size];
        ret = setvbuf(fp_from, vbuf_from, _IOFBF, vbuf_size);
      }
      assert(ret == 0);
    }

    return fd_from;
  };

  void set_fp_to(FILE* x) {
    fp_to = x;
  };
};

FILE* fp_connect(const char* hostname, unsigned short port, const char* opt /* for fdopen */)
{
  struct addrinfo* res = 0, hint;
  char portstr[64];

  memset(&hint, 0, sizeof(hint));

  snprintf(portstr, sizeof(portstr), "%d", port_to);

  hint.ai_family = AF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;

  int ret;
  int fd;

  ret = getaddrinfo(hostname, portstr, &hint, &res);
  assert(ret == 0);

  fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  ret = connect(fd, res->ai_addr, res->ai_addrlen);
  assert(ret == 0);

  FILE* fp;

  fp = fdopen(fd, "w");
  assert(fp);

  if (vbuf_size) {
    int ret;
    if (vbuf_size == -1) {
      ret = setvbuf(fp, 0, _IONBF, 0);
    } else {
      vbuf_to = new char[vbuf_size];
      ret = setvbuf(fp, vbuf_to, _IOFBF, vbuf_size);
    }
    assert(ret == 0);
  }

  return fp;
}


main(int argc, char** argv)
{
  int ch;
  while (-1 != (ch = getopt(argc, argv, "i:s:S:n:p:P:D"))) {
    switch (ch) {
      case 'i':
        interval = strtol(optarg, 0, 0);
        break;
      case 'S':
        vbuf_size = strtol(optarg, 0 , 0);
        break;
      case 'n':
        nstream = strtol(optarg, 0, 0);
        break;
      case 'p':
        baseport_from = strtol(optarg, 0, 0);
        break;
      case 'P':
        port_to = strtol(optarg, 0, 0);
        break;
      case 'D':
        discard = 1;
        break;
    }
  }

  connection conn[nstream];

  for (int i = 0; i < nstream; i++) {
    int from = baseport_from + i;
    printf("stream%d %d => %d\n", i, from, port_to);

    conn[i].init(from);
    conn[i].listen();
  }


  for (int i = 0; i < nstream; i++) {
    printf("accepting %d...", i);
    fflush(stdout);
    conn[i].accept();
    printf("done\n");
  }

  if (!discard) {
    fp_to = fp_connect("127.0.0.1", port_to, "w");
    for (int i = 0; i < nstream; i++) {
      conn[i].set_fp_to(fp_to);
    }
  }

  char* buffer = new char[max_event_size];

  struct timeval tv0, tv1;

  total_size = 0;

  for (int nevent = 0; ; nevent++) {
    for (int i = 0; i < nstream; i++) {
      uint32_t nword;

      if (discard) {
        if (0 == conn[i].fread((char*)&nword, sizeof(nword))) {
          exit(0);
        }
        if (0 == conn[i].fread(buffer, sizeof(nword) * (nword - 1))) {
          exit(0);
        }
      } else {
        if (0 == conn[i].copy((char*)&nword, sizeof(nword))) {
          exit(0);
        }
        if (0 == conn[i].copy(buffer, sizeof(nword) * (nword - 1))) {
          exit(0);
        }
      }

      total_size += sizeof(nword) * nword;
    }
    if (nevent % interval == 0) {
      gettimeofday(&tv1, 0);
      if (nevent > 0) {
        double tdiff = (double)(tv1.tv_sec - tv0.tv_sec);
        tdiff += (double)(tv1.tv_usec - tv0.tv_usec) / 1000000.0;

        printf("%d event %f sec %f MB/s\n",
               nevent, tdiff, total_size / 1000000.0 / tdiff);
        total_size = 0;
      }
      tv0 = tv1;
    }
  }

}
