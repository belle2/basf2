#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "send_header_and_trailer.h"
#include "raw_copper_header_and_trailer.h"


static int NUM_ENTRY = 50;
int total_size;
int max_event_size = 1 * 1024 * 1024;
const int hdr_nwords = 10;

int nstream = 1;
int baseport_from = 33000;

int port_to_basf2 = 35000;
int port_from_basf2 = 37000;

int port_to = 65000;

int interval = 10000;
int vbuf_size = 0;
int discard = 0;
int basf2 = 0;
int add_hdr = 0;

int fd_to = -1;
FILE* fp_to = NULL;
char* vbuf_to = NULL;

//FILE * fp_from;
char* vbuf_from = NULL;

FILE* fp_basf2_to = NULL;


void fill_hdr(int* buf)
{



}

void modify_rawheader(int* buf)
{

  return;
}

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
#ifdef DEBUG
    printf("Reading data... size %d\n", len);
#endif
    ret = fread(buffer, len);
    if (ret != 1 && feof(fp_from)) {
      return 0;
    }
#ifdef DEBUG
    printf("done.\n");
    printf("Sending data... size %d\n", len);
#endif

    char* buffer_body = buffer + sizeof(struct SendHeader);
    int len_body = len - sizeof(struct SendHeader) - sizeof(struct SendTrailer);
    modify_rawheader((int*)buffer_body);

    fwrite(buffer_body, len_body);
    fflush(fp_to);

#ifdef DEBUG
    printf("done.\n");

    for (int i = 0; i < len / sizeof(int); i++) {
      printf("0x%.8x ", *((int*)buffer + i));
      if ((i + 1) % 10 == 0) printf("\n %6d : ", i);
    }
    printf("\n");
    printf("\n");
#endif

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

  void set_fp_from(FILE* x) {
    fp_from = x;
  };
};


FILE* fp_accept(const char* local_hostname, unsigned short port_from, const char* opt /* for fdopen */)
{


  struct addrinfo* res = 0, hint;

  memset(&hint, 0, sizeof(hint));

  char portstr[64];

  snprintf(portstr, sizeof(portstr), "%d", port_from);

  hint.ai_family = AF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;
  hint.ai_flags = AI_PASSIVE;

  int ret;

  //    ret = getaddrinfo(NULL, portstr, &hint, &res);
  ret = getaddrinfo(local_hostname, portstr, &hint, &res);
  assert(ret == 0);

  int fd_listen = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
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

  // accept

  int fd_from = ::accept(fd_listen, 0, 0);
  assert(fd_from != -1);

  close(fd_listen);

  FILE* fp_from = fdopen(fd_from, opt);
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
  return fp_from;

}

FILE* fp_connect(const char* hostname, unsigned short port, const char* opt /* for fdopen */)
{
  struct addrinfo* res = 0, hint;
  char portstr[64];

  memset(&hint, 0, sizeof(hint));

  snprintf(portstr, sizeof(portstr), "%d", port);

  hint.ai_family = AF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;

  int ret;
  int fd;

  ret = getaddrinfo(hostname, portstr, &hint, &res);
  assert(ret == 0);

  fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  ret  = -1;
  printf("connecting to %s (%d) socket %d\n", hostname, port, fd);
  while (ret != 0) {
    ret = connect(fd, res->ai_addr, res->ai_addrlen);
    sleep(1);
  }

  assert(ret == 0);
  printf("Done\n");

  FILE* fp;

  fp = fdopen(fd, opt);
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
  while (-1 != (ch = getopt(argc, argv, "i:s:S:n:p:P:Dbh"))) {
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
      case 'b':
        basf2 = 1;
        break;
      case 'h':
        add_hdr = 1;
        break;
    }
  }

  connection conn[nstream];
  connection conn_hdr;
  connection conn_basf2;

  if (!discard) {
    //  fp_to = fp_connect("127.0.0.1", port_to, "w");
    //     if( basf2 ){
    //       fp_to = fp_connect("localhost", port_to_basf2, "w");
    //       // fp_to = fp_connect("192.168.100.11", port_to, "w");
    //     }else{
    //       fp_to = fp_accept("localhost", port_to, "w");
    //     }
    printf("Accepting...\n");
    fp_to = fp_accept("localhost", port_to, "w");
    printf("Done\n");
    for (int i = 0; i < nstream; i++) {
      conn[i].set_fp_to(fp_to);
    }
  }

//   if( basf2 ){
//     conn_basf2.init( port_from_basf2 );
//     conn_basf2.listen();
//     fp_basf2_to = fp_connect("192.168.100.11", port_to, "w");
//     // fp_basf2_to = fp_connect("localhost", port_to_basf2, "w");
//     conn_basf2.set_fp_to(fp_basf2_to);
//     printf("accepting from basf2...");
//     conn_basf2.accept();
//     printf("done\n");
//   }

//   if( add_hdr ){
//     conn_hdr.set_fp_to( fp_to );
//   }

  for (int i = 0; i < nstream; i++) {
    int port_from = baseport_from + i;
    conn[i].set_fp_from(fp_connect("localhost", port_from, "r"));
  }

  char* buffer = new char[max_event_size];
  struct timeval tv0, tv1;

  int prev_nevent = -1;
  total_size = 0;

  int* hdr[hdr_nwords];

  for (int nevent = 0; ; nevent++) {
    for (int nentry = 0; nentry < NUM_ENTRY; nentry++) {

#ifdef DEBUG
      printf("neve %d ent %d\n" , nevent, nentry);
#endif

      //       if( add_hdr ){
      //  fill_hdr( hdr );
      //  conn_hdr.copy( hdr, hdr_nwords*sizeof(int) );
      //       }

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
    }


    if (!discard && basf2) {
      for (int nentry = 0; nentry < NUM_ENTRY; nentry++) {
        uint32_t nword;
#ifdef DEBUG
        printf("Try to read size\n");
#endif
        if (0 == conn_basf2.copy((char*)&nword, sizeof(nword))) {
          exit(0);
        }
#ifdef DEBUG
        printf("size2 %d ent %d eve %d\n", nword, nentry, nevent);
#endif
        if (0 == conn_basf2.copy(buffer, sizeof(nword) * (nword - 1))) {
          exit(0);
        }
      }
    }


    if (nevent % (interval / NUM_ENTRY) == 0) {
      gettimeofday(&tv1, 0);
      if (nevent > 0) {
        double tdiff = (double)(tv1.tv_sec - tv0.tv_sec);
        tdiff += (double)(tv1.tv_usec - tv0.tv_usec) / 1000000.0;
        printf("%d event %f sec %f MB/s %lf kHz %d %d %lf %lf\n",
               nevent, tdiff, total_size / 1000000.0 / tdiff,
               (nevent - prev_nevent) / tdiff
              );
        total_size = 0;
        prev_nevent = nevent;
      }
      tv0 = tv1;
    }
  }
}
