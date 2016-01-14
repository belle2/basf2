#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define LISTENQ 1
#define SERVER


#define NW_SEND_HEADER 6
#define NW_SEND_TRAILER 2
#define NW_RAW_HEADER 12
#define NW_RAW_TRAILER 2

#define REDUCED_DATA

#ifdef REDUCED_DATA
#define NW_COPPER_HEADER 0
#define NW_B2L_HEADER 2
#define NW_B2L_TRAILER 1
#define NW_COPPER_TRAILER 0
#else
#define NW_COPPER_HEADER 13
#define NW_B2L_HEADER 6
#define NW_B2L_TRAILER 3
#define NW_COPPER_TRAILER 3
#endif

using namespace std;

double getTimeSec()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec + t.tv_usec * 1.e-6 - 1417570000.);
}

int fillDataContents(int* buf, int nwords_per_fee, unsigned int node_id, int ncpr, int nhslb, int run)
{
  int nwords =  NW_SEND_HEADER + NW_SEND_TRAILER +
                ncpr * (NW_RAW_HEADER + NW_COPPER_HEADER +
                        (NW_B2L_HEADER + NW_B2L_TRAILER + nwords_per_fee) * nhslb
                        + NW_COPPER_TRAILER + NW_RAW_TRAILER);


  // Send Header
  int offset = 0;
  buf[ offset + 0 ] = nwords;
  buf[ offset + 1 ] = 6;
  buf[ offset + 2 ] = (1 << 16) | ncpr;
  unsigned int exp_run = run;
  buf[ offset + 3 ] = exp_run;
  buf[ offset + 5 ] = node_id;
  offset += 6;


  for (int k = 0; k < ncpr; k++) {
    //
    // RawHeader
    //
    int cpr_nwords = NW_RAW_HEADER + NW_COPPER_HEADER +
                     (NW_B2L_HEADER + NW_B2L_TRAILER + nwords_per_fee) * nhslb
                     + NW_COPPER_TRAILER + NW_RAW_TRAILER;
    int finesse_nwords = nwords_per_fee + NW_B2L_HEADER + NW_B2L_TRAILER;
    unsigned int ctime = 0x12345601;
    unsigned int utime = 0x98765432;


    buf[ offset +  0 ] = cpr_nwords;
#ifdef REDUCED_DATA
    buf[ offset +  1 ] = 0x7f7f020c;
#else
    buf[ offset +  1 ] = 0x7f7f820c;
#endif
    buf[ offset +  2 ] = exp_run;
    buf[ offset +  4 ] = ctime;
    buf[ offset +  5 ] = utime;
    buf[ offset +  6 ] = node_id + k;
    buf[ offset +  7 ] = 0x34567890;
    buf[ offset +  8 ] = NW_RAW_HEADER + NW_COPPER_HEADER;
    buf[ offset +  9 ] = buf[ offset +  8 ] + finesse_nwords;
    if (nhslb > 1) {
      buf[ offset +  10 ] = buf[ offset +  9 ] + finesse_nwords;
    } else {
      buf[ offset +  10 ] = buf[ offset +  9 ];
    }
    if (nhslb > 2) {
      buf[ offset +  11 ] = buf[ offset +  10 ] + finesse_nwords;
    } else {
      buf[ offset +  11 ] = buf[ offset +  10 ];
    }
    offset += 12;

#ifdef REDUCED_DATA
#else
    // COPPER header
    buf[ offset +  0 ] = 0x7fff0008;
    buf[ offset +  2 ] = 0;
    buf[ offset +  3 ] = 0;
    buf[ offset +  4 ] = 0;
    buf[ offset +  5 ] = 0;
    buf[ offset +  6 ] = 0;
    buf[ offset +  7 ] = 0xfffffafa;
    buf[ offset +  8 ] = cpr_nwords - (NW_RAW_HEADER + 7 + 2 + NW_RAW_TRAILER);
    buf[ offset +  9 ] = finesse_nwords;
    buf[ offset +  10 ] = finesse_nwords;
    buf[ offset +  11 ] = finesse_nwords;
    buf[ offset +  12 ] = finesse_nwords;
    offset += 13;
#endif

    for (int i = 0; i < nhslb ; i++) {
#ifdef REDUCED_DATA
      buf[ offset +  0 ] = 0xffaa0000;
      buf[ offset +  1 ] = ctime;
      offset += 2;
#else
      buf[ offset +  0 ] = 0xffaa0000;
      buf[ offset +  1 ] = ctime;
      buf[ offset +  3 ] = utime;
      buf[ offset +  4 ] = exp_run;
      buf[ offset +  5 ] = ctime;
      offset += 6;
#endif

      for (int j = offset; j < offset + nwords_per_fee; j++) {
        buf[ j ] = rand();
      }
      offset += nwords_per_fee;
#ifdef REDUCED_DATA
      buf[ offset  ] = 0;
      offset += 1;
#else
      buf[ offset  ] = ctime;
      buf[ offset + 1 ] = 0;
      buf[ offset + 2 ] = 0xff550000;
      offset += 3;
#endif
    }

#ifdef REDUCED_DATA
#else
    // COPPER trailer/Raw Trailer
    buf[ offset ] = 0xfffff5f5;
    buf[ offset + 1 ] = 0;
    buf[ offset + 2 ] = 0x7fff0009;
    offset += 3;
#endif

    buf[ offset  ] = 0;
    buf[ offset + 1 ] = 0x7fff0006;
    offset += 2;
  }

  // Send trailer
  buf[ offset ] = 0;
  buf[ offset + 1 ] = 0x7fff0000;
  offset += 2;
  return offset;
}



inline void addEvent(int* buf, int nwords_per_fee, unsigned int event, int ncpr, int nhslb)
//inline void addEvent(int* buf, int nwords, unsigned int event)
{
  int offset = 0;
  buf[ offset + 4 ] = event;
  offset += NW_SEND_HEADER;

  for (int k = 0; k < ncpr; k++) {
    if (buf[ offset + 4 ] != 0x12345601) {
      printf("ERROR 2 0x%.x", buf[ offset + 4 ]);
      fflush(stdout);
      exit(1);
    }
    // RawHeader
    buf[ offset + 3] = event;

#ifdef REDUCED_DATA
    offset += NW_RAW_HEADER + NW_COPPER_HEADER +
              nhslb * (NW_B2L_HEADER + nwords_per_fee + NW_B2L_TRAILER)
              + NW_COPPER_TRAILER + NW_RAW_TRAILER;
#else
    // COPPER header
    offset += NW_RAW_HEADER;
    buf[ offset +  1 ] = event;
    offset += NW_COPPER_HEADER;

    for (int i = 0; i < nhslb ; i++) {
      if ((buf[ offset ] & 0xffff0000) != 0xffaa0000) {
        printf("ERROR 3 0x%.x hslb %d cpr %d\n", buf[ offset ], i, k);
        fflush(stdout);
        exit(1);
      }
      buf[ offset +  0 ] = 0xffaa0000 + (event & 0xffff);
      buf[ offset +  2 ] = event;
      offset += NW_B2L_HEADER + nwords_per_fee + NW_B2L_TRAILER;
    }
    offset += NW_COPPER_TRAILER + NW_RAW_TRAILER;
#endif

  }

}


int main(int argc, char** argv)
{

#ifdef SERVER
  if (argc != 6) {
    printf("Usage : %s <node ID> <run#> <nwords of det. buf per FEE> <# of CPR per COPPER> <# of HSLBs>\n", argv[ 0 ]);
    exit(1);
  }
#else
  if (argc != 8) {
    printf("Usage : %s <node ID> <run#> <nwords of det. buf per FEE> <# of CPR per COPPER>  <# of HSLBs> <ropc hostname> <ropc port> argc %d\n",
           argv[ 0 ], argc);
    exit(1);
  }
#endif

  unsigned int node_id = 0;
  sscanf(argv[1], "0x%x", &node_id);

  int run_no = atoi(argv[2]);
  printf("run_no %d\n", run_no); fflush(stdout);

  int nwords_per_fee = atoi(argv[3]);
  int ncpr = atoi(argv[4]);
  int nhslb = atoi(argv[5]);

  int listenfd, connfd;
  struct sockaddr_in servaddr;
  int total_words =  NW_SEND_HEADER + NW_SEND_TRAILER +
                     ncpr * (NW_RAW_HEADER + NW_COPPER_HEADER + (NW_B2L_HEADER + NW_B2L_TRAILER + nwords_per_fee) * nhslb + NW_COPPER_TRAILER +
                             NW_RAW_TRAILER);
  printf("TET %d %d %d %d %d\n ", NW_SEND_HEADER + NW_SEND_TRAILER, ncpr,
         NW_RAW_HEADER + NW_COPPER_HEADER, (NW_B2L_HEADER + NW_B2L_TRAILER + nwords_per_fee) * nhslb, NW_COPPER_TRAILER +   NW_RAW_TRAILER);

  int buff[total_words];

  //
  // Prepare header
  //
  int temp_ret = fillDataContents(buff, nwords_per_fee, node_id, ncpr, nhslb, run_no);
  if (temp_ret != total_words) {
    printf("ERROR1 %d %d\n", total_words, temp_ret);
    fflush(stdout);
    exit(1);
  }

  //  for(int i = 0 ; i < total_words ; i++){
  //     printf("%.8x ", buff[ i ]);
  //     if( i % 10 == 9 ) printf("\n");
  //   }

#ifdef SERVER
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(33000);

  int flags = 1;
  int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flags, (socklen_t)sizeof(flags));
  if (ret < 0) {
    perror("Failed to set REUSEADDR");
  }

  bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

  listen(listenfd, LISTENQ);
  printf("Accepting..."); fflush(stdout);
  connfd = accept(listenfd, (struct sockaddr*) NULL, NULL);
  printf("Done."); fflush(stdout);
#else

  //   if( argc != 3 ){
  //     printf("Usage : %s <IPaddress> <port>\n", argv[0]);
  //     exit(1);
  //   }

  if ((connfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket error");
    exit(1);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(argv[7]));
  if (inet_pton(AF_INET, argv[6], &servaddr.sin_addr) <= 0) {
    perror("inetpton error");
    exit(1);
  }

  while (true) {
    if (connect(connfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
      perror("connect error");
      continue;
      // exit(1);
    }
    sleep(1);
    break;
  }


#endif

  printf("Connection Accepted\n"); fflush(stdout);

  double init_time = getTimeSec();
  double prev_time = init_time;

  unsigned long long int cnt = 0;
  unsigned long long int prev_cnt = 0;
  unsigned long long int start_cnt = 300000;
  for (;;) {
    //    addEvent(buff, total_words, cnt);
    addEvent(buff, nwords_per_fee, cnt, ncpr, nhslb);
    //    printf("cnt %d bytes\n", cnt*total_words); fflush(stdout);
    //    sprintf( buff, "event %d dessa", cnt );
    int ret = 0;
    if ((ret = write(connfd, buff, total_words * sizeof(int))) <= 0) {
      printf("[FATAL] Return value %d\n", ret);
      fflush(stdout);
      exit(1);
    }

    cnt++;

    if (cnt == start_cnt) init_time = getTimeSec();
    if (cnt % 10000 == 1) {
      if (cnt > start_cnt) {
        double cur_time = getTimeSec();
        printf("run %d evt %lld time %.1lf dataflow %.1lf MB/s rate %.2lf kHz : so far dataflow %.1lf MB/s rate %.2lf kHz size %d\n",
               run_no,
               cnt,
               cur_time - init_time,
               (cnt - prev_cnt)*total_words * sizeof(int) / 1000000. / (cur_time - prev_time),
               (cnt - prev_cnt) / (cur_time - prev_time) / 1000. ,
               (cnt - start_cnt)*total_words * sizeof(int) / 1000000. / (cur_time - init_time),
               (cnt - start_cnt) / (cur_time - init_time) / 1000. , total_words);

        fflush(stdout);
        prev_time = cur_time;
        prev_cnt = cnt;
      } else {
        //  printf("Eve %lld\n", cnt);fflush(stdout);
      }
    }
  }
  close(connfd);
}
