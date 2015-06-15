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
#define NWORDS_PER_COPPER 48 // 16 -> 1kB/16COPER
#define LISTENQ 1
#define SERVER

using namespace std;

double getTimeSec()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec + t.tv_usec * 1.e-6 - 1417570000.);
}

void fillDataContents(int* buf, int nwords, unsigned int node_id)
{

  // rawheader, copperheader, copper trailer, raw trailer, b2l header and trailer, send header, send trailer;

  int offset = 0;
  buf[ offset + 0 ] = nwords;
  buf[ offset + 1 ] = 6;
  buf[ offset + 2 ] = 0x00010001;
  unsigned int exp_run = 0x00400101;
  buf[ offset + 3 ] = exp_run;

  buf[ offset + 5 ] = node_id;

  //
  // RawHeader
  //
  offset = 6;
  buf[ offset +  0 ] = nwords - 6 - 2;
  //  buf[ offset +  1 ] = 0x7f7f010c;
  buf[ offset +  1 ] = 0x7f7f820c;
  buf[ offset +  2 ] = exp_run;
  unsigned int ctime = 0x12345601;
  buf[ offset +  4 ] = ctime;
  unsigned int utime = 0x98765432;
  buf[ offset +  5 ] = utime;
  buf[ offset +  6 ] = node_id;
  buf[ offset +  7 ] = 0x34567890;
  int finesse_nwords = (nwords - (6 + 12 + 13 + 3 + 2 + 2)) / 4;
  int residual = (nwords - (6 + 12 + 13 + 3 + 2 + 2)) - finesse_nwords * 4;
  buf[ offset +  8 ] = 12 + 13;
  buf[ offset +  9 ] = buf[ offset +  8 ] + finesse_nwords;
  buf[ offset +  10 ] = buf[ offset +  9 ] + finesse_nwords;
  buf[ offset +  11 ] = buf[ offset +  10 ] + finesse_nwords;

  offset = 6 + 12;

  // COPPER header
  buf[ offset +  0 ] = 0x7fff0008;
  buf[ offset +  2 ] = 0;
  buf[ offset +  3 ] = 0;
  buf[ offset +  4 ] = 0;
  buf[ offset +  5 ] = 0;
  buf[ offset +  6 ] = 0;
  buf[ offset +  7 ] = 0xfffffafa;
  buf[ offset +  8 ] = nwords - (6 + 12 + 7 + 2 + 2 + 2);
  buf[ offset +  9 ] = finesse_nwords;
  buf[ offset +  10 ] = finesse_nwords;
  buf[ offset +  11 ] = finesse_nwords;
  buf[ offset +  12 ] = finesse_nwords + residual;


  for (int i = 0; i < 4 ; i++) {
    offset = 6 + 12 + 13 + i * finesse_nwords;
    buf[ offset +  0 ] = 0xffaa0000;
    buf[ offset +  1 ] = ctime;
    buf[ offset +  3 ] = utime;
    buf[ offset +  4 ] = exp_run;
    buf[ offset +  5 ] = ctime;

    int add = 0;
    if (i == 3) add = residual;

    offset = 6 + 12 + 13 + 5 + i * finesse_nwords;
    for (int j = offset; j < offset + finesse_nwords + add - 3; j++) {
      buf[ j ] = rand();
      buf[ j ] = 0;
    }

    offset = 6 + 12 + 13 + (i + 1) * finesse_nwords + add;
    buf[ offset - 3  ] = ctime;
    buf[ offset - 2  ] = 0;
    buf[ offset - 1  ] = 0xff550000;
  }

  // COPPER trailer/Raw Trailer
  offset = nwords - 2;
  buf[ offset - 5  ] = 0xfffff5f5;
  buf[ offset - 4  ] = 0;
  buf[ offset - 3  ] = 0x7fff0009;
  buf[ offset - 2  ] = 0;
  buf[ offset - 1  ] = 0x7fff0006;

  // Send trailer
  buf[ nwords - 2 ] = 0;
  buf[ nwords - 1 ] = 0x7fff0000;

  return;
}



inline void addEvent(int* buf, int nwords, unsigned int event)
{
  int offset = 0;
  buf[ offset + 4 ] = event;
  offset = 6;
  buf[ offset + 3] = event;
  offset = 6 + 12;
  buf[ offset +  1 ] = event;

  int finesse_nwords = (nwords - (6 + 12 + 13 + 3 + 2 + 2)) / 4;
  for (int i = 0; i < 4 ; i++) {
    offset = 6 + 12 + 13 + i * finesse_nwords;
    buf[ offset +  0 ] = 0xffaa0000 + (event & 0xffff);
    buf[ offset +  2 ] = event;
  }

}

int main(int argc, char** argv)
{
  int run_no = 0;

#ifdef SERVER
  //  if( argc != 2 && argc != 3  ){
  if (argc < 2) {
    printf("Usage : %s <node ID> (run#)\n", argv[ 0 ]);
    exit(1);
  }
#else
  if (argc != 4 && argc != 5) {
    printf("Usage : %s <node ID> <run#> <ropc hostname> <ropc port> argc %d\n", argv[ 0 ], argc);
    exit(1);
  }

#endif

  if (argc == 3 || argc == 5) run_no = atoi(argv[2]);
  printf("run_no %d\n", run_no); fflush(stdout);

  unsigned int node_id = atoi(argv[1]);


  int listenfd, connfd;
  struct sockaddr_in servaddr;
  int total_words = NWORDS_PER_COPPER + 6 + 12 + 13 + (5 + 3) * 4 + 3 + 2 + 2;

  int buff[total_words];

  //
  // Prepare header
  //
  fillDataContents(buff, total_words, node_id);


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
  servaddr.sin_port = htons(atoi(argv[4]));
  if (inet_pton(AF_INET, argv[3], &servaddr.sin_addr) <= 0) {
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
    addEvent(buff, total_words, cnt);
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
