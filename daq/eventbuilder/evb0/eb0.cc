#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <string>
#include "send_header_and_trailer.h"
#include "raw_copper_header_and_trailer.h"

//#define DEBUG
//#define TIME

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

FILE* fp_from_basf2 = NULL;

#ifdef TIME

#define EVE_MAX 600
#define EVE_MIN 100
#define EVE_DISP 700

double GetTimeSec()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec + t.tv_usec * 1.e-6 - 1376878536.);
}
#endif

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

  void init() {
    //  void init(int from) {
    clear();
    //    port_from = from;
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
      exit(1);
      return 0;
    }

    //     char* buffer_body = buffer + sizeof( struct SendHeader );
    //     int len_body = len - sizeof( struct SendHeader ) - sizeof( struct SendTrailer );
    char* buffer_body = buffer;
    int len_body = len;
    //    modify_rawheader( (int*)buffer_body );

    fwrite(buffer_body, len_body);
    fflush(fp_to);
    //    printf("Sent %d bytes\n", len_body);

    return len;
  };

  int listen() {
    struct addrinfo* res = 0, hint;

    memset(&hint, 0, sizeof(hint));

    char portstr[64];

    snprintf(portstr, sizeof(portstr), "%d", port_to);

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
  while (true) {
    ret = connect(fd, res->ai_addr, res->ai_addrlen);
    if (ret == 0) break;
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

  std::string copper_hostname[10];

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
        for (int i = 0 ; i < nstream; i++) {
          copper_hostname[ i ] = argv[optind + i];
          printf("COPPER %d %s\n", optind, copper_hostname[ i ].c_str());
        }

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
  connection conn_hdr;

  connection conn[nstream];


  if (basf2 || !discard) {
    std::string host;
    int port;

    if (basf2) {
      host = "localhost";
      port = port_to_basf2;
    } else {
      host = "localhost";
      port = port_to;
    }

    printf("Accepting from %s port %d...\n", host.c_str(), port);
    FILE* fp = fp_accept(host.c_str(), port, "w");
    printf("Done\n");
    for (int i = 0; i < nstream; i++) {
      conn[i].set_fp_to(fp);
    }


  }

  connection conn_basf2;
  if (basf2 && !discard) {
    FILE* fp;
    char host_to[20] = "localhost";
    printf("Done\n");
    printf("Accepting from %s port %d...\n", host_to, port_to);
    fp = fp_accept("localhost", port_to, "w");
    conn_basf2.set_fp_to(fp);

    fp = fp_connect("localhost", port_from_basf2, "r");
    conn_basf2.set_fp_from(fp);

  }



  for (int i = 0; i < nstream; i++) {
    //    int port_from = baseport_from + i;
    int port_from = baseport_from;
    //    conn[i].set_fp_from( fp_connect("localhost", port_from, "r") );
    conn[i].set_fp_from(fp_connect(copper_hostname[ i ].c_str(), port_from, "r"));
  }

  //   if( add_hdr ){
  //     conn_hdr.set_fp_to( fp_to );
  //   }


  char* buffer = new char[max_event_size];
  struct timeval tv0, tv1;

  int prev_nevent = -1;
  total_size = 0;

  int* hdr[hdr_nwords];

#ifdef TIME
  int diff_cnt = 0;
  double time_diff1[500];
  double time_diff2[500];
  double time_diff3[500];
  double time_diff4[500];
#endif

  for (int nevent = 0; ; nevent++) {

#ifdef TIME
    if (nevent >= EVE_MIN && nevent < EVE_MAX) {
      time_diff1[ diff_cnt ] = GetTimeSec();
    }
#endif

    for (int nentry = 0; nentry < NUM_ENTRY; nentry++) {
      //       if( add_hdr ){
      //  fill_hdr( hdr );
      //  conn_hdr.copy( hdr, hdr_nwords*sizeof(int) );
      //       }

      for (int i = 0; i < nstream; i++) {
        uint32_t nword;
        if (! basf2 && discard) {
          if (0 == conn[i].fread((char*)&nword, sizeof(nword))) {
            exit(0);
          }
#ifdef DEBUG
          printf("Size %d\n", nword * sizeof(int));
#endif

          if (0 == conn[i].fread(buffer, sizeof(nword) * (nword - 1))) {
            exit(0);
          }
        } else {
          if (0 == conn[i].copy((char*)&nword, sizeof(nword))) {
            exit(0);
          }
#ifdef DEBUG
          printf("Size %d\n", nword * sizeof(int));
#endif
          if (0 == conn[i].copy(buffer, sizeof(nword) * (nword - 1))) {
            exit(0);
          }
#ifdef DEBUG
          //    for( int i = 0; i < nword-1;i++){
          //      printf( "0x%.8x ", *( (int*)buffer + i ) );
          //      if( ( i + 1 ) % 10 == 0 ) printf("\n %6d : ", i );
          //    }
          //    printf("\n");
          //    printf("\n");
#endif
        }
        total_size += sizeof(nword) * nword;


#ifdef DEBUG
        printf("neve %d ent %d size %d\n" , nevent, nentry, sizeof(nword)*nword);

#endif
      }


    }

#ifdef TIME
    if (nevent >= EVE_MIN && nevent < EVE_MAX) {
      time_diff2[ diff_cnt ] = GetTimeSec();
    }
#endif


    //
    // Receive data from basf2 and send to evb
    //
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

#ifdef DEBUG
        for (int i = 0; i < nword - 1; i++) {
          printf("0x%.8x ", *((int*)buffer + i));
          if ((i + 1) % 10 == 0) printf("\n %6d :: ", i);
        }
        printf("\n");
        printf("\n");

#endif

      }

#ifdef TIME
      if (nevent >= EVE_MIN && nevent < EVE_MAX) {
        time_diff3[ diff_cnt ] = GetTimeSec();
        diff_cnt++;
      }
#endif



    }

    //
    // Rate Monitor
    //
    if (nevent % (interval / NUM_ENTRY) == 0) {
      if (nevent > 0) {
        gettimeofday(&tv1, 0);
        double tdiff = (double)(tv1.tv_sec - tv0.tv_sec);
        tdiff += (double)(tv1.tv_usec - tv0.tv_usec) / 1000000.0;

        printf("event %d\n", nevent * NUM_ENTRY);
//         printf("%d event %.2f sec %.2f MB/s %.2lf kHz %d %d %lf %lf\n",
//                nevent * NUM_ENTRY, tdiff, total_size / 1000000.0 / tdiff,
//                (nevent - prev_nevent)*NUM_ENTRY / tdiff / 1000.
//               );
        total_size = 0;
        prev_nevent = nevent;
        tv0 = tv1;
      }

    }

#ifdef TIME
    if (nevent == EVE_DISP) {
      for (int i = 0; i < EVE_MAX - EVE_MIN; i++) {
        printf("%d %lf %lf %lf\n",
               i, time_diff1[i], time_diff2[i], time_diff3[i]);
      }


      fflush(stdout);
    }
#endif
  }
}
