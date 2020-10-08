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
#include <limits.h>
#include <signal.h>
#include <poll.h>


/////////////////////////////////////////
// Parameter for data-contents
////////////////////////////////////////
//#define REDUCED_DATA
//#define CRC_ON
#define LISTENQ 1
#define NUM_CLIENTS 5
#define MAX_EVENT 1000

// Format (PCIe40)
#define NW_SEND_HEADER 6
#define NW_SEND_TRAILER 2

#define NW_RAW_HEADER 8
#define NW_RAW_TRAILER 4

#ifdef REDUCED_DATA
#define NW_B2L_HEADER 3
#define NW_B2L_TRAILER 2
#else
#define NW_B2L_HEADER 7
#define NW_B2L_TRAILER 3
#endif

#define CTIME_VAL 0x12345601

using namespace std;

unsigned short CalcCRC16LittleEndian(unsigned short crc16, const int buf[], int nwords)
{

  if (nwords < 0) {
    char err_buf[500];
    sprintf(err_buf, "nwords value(%d) is invalid. Cannot calculate CRC16. Exiting...\n %s %s %d\n",
            nwords, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    string err_str = err_buf;
    throw (err_str);
  }

  const unsigned short CRC16Table0x1021[ 256 ] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,

    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,

    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,

    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
  };

  int cnt = 0, nints = 0;
  //  printf("### %.8x %.4x\n", buf[ 0 ], crc16);
  while (nwords != 0) {

    unsigned char temp_buf = *((unsigned char*)(buf + nints) + (-(cnt % 4) + 3));
    crc16 = CRC16Table0x1021[(crc16 >> (16 - CHAR_BIT)) ^ temp_buf ] ^ (crc16 << CHAR_BIT);
    //    printf("%.2x %.4x\n", temp_buf, crc16);
    if ((cnt % 4) == 3) {
      nwords--;
      nints++;
      //      printf("### %.8x\n", buf[ nints ] );
    }

    cnt++;
  }


  return crc16;

}


double getTimeSec()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec + t.tv_usec * 1.e-6 - 1417570000.);
}

int fillDataContents(int* buf, int nwords_per_fee, unsigned int node_id, int ncpr, int nhslb, int run)
{
  int nwords =  NW_SEND_HEADER + NW_SEND_TRAILER +
                ncpr * (NW_RAW_HEADER +
                        (NW_B2L_HEADER + NW_B2L_TRAILER + nwords_per_fee) * nhslb
                        + NW_RAW_TRAILER);

  // Send Header
  int offset = 0;
  buf[ offset + 0 ] = nwords;
  buf[ offset + 1 ] = 6;
  buf[ offset + 2 ] = (1 << 16) | ncpr;
  unsigned int exp_run = run << 8;
  buf[ offset + 3 ] = exp_run;
  buf[ offset + 5 ] = node_id;
  offset += NW_SEND_HEADER;

  for (int k = 0; k < ncpr; k++) {
    int top_pos = offset;
    //
    // RawHeader
    //
    int cpr_nwords = NW_RAW_HEADER +
                     (NW_B2L_HEADER + NW_B2L_TRAILER + nwords_per_fee) * nhslb
                     + NW_RAW_TRAILER;
    int finesse_nwords = nwords_per_fee + NW_B2L_HEADER + NW_B2L_TRAILER;
    unsigned int ctime = CTIME_VAL;
    unsigned int utime = 0x98765432;

    buf[ offset +  0 ] = cpr_nwords;
#ifdef REDUCED_DATA
    buf[ offset +  1 ] = 0x7f7f020c;
#else
    buf[ offset +  1 ] = 0x7f7f820c;
#endif
    buf[ offset +  2 ] = exp_run;
    printf("run_no %d\n", exp_run); fflush(stdout);
    buf[ offset +  4 ] = ctime;
    buf[ offset +  5 ] = utime;
    buf[ offset +  6 ] = node_id + k;
    buf[ offset +  7 ] = 0x34567890;
    offset += NW_RAW_HEADER;

    for (int i = 0; i < nhslb ; i++) {
#ifdef REDUCED_DATA
      buf[ offset +  0 ] = nwords_per_fee + 3;
      buf[ offset +  1 ] = 0xffaa0000;
      buf[ offset +  2 ] = ctime;
#else
      buf[ offset +  0 ] = nwords_per_fee + 7;
      buf[ offset +  1 ] = 0xffaa0000;
      buf[ offset +  3 ] = ctime;
      buf[ offset +  4 ] = utime;
      buf[ offset +  5 ] = exp_run;
      buf[ offset +  6 ] = ctime;
#endif
      offset += NW_B2L_HEADER;

      for (int j = offset; j < offset + nwords_per_fee; j++) {
        buf[ j ] = rand();
      }
      offset += nwords_per_fee;

#ifdef REDUCED_DATA
      buf[ offset  ] = 0;
      buf[ offset + 1 ] = 0xff550000;
#else
      buf[ offset  ] = ctime;
      buf[ offset + 1 ] = 0;
      buf[ offset + 2 ] = 0xff550000;
#endif

      offset += NW_B2L_TRAILER;
    }
    buf[ offset  ] = 0x0; // error bits
    buf[ offset + 1 ] = 0x0; // error slots
    buf[ offset + 2 ] = 0x0; // XOR checksum
    buf[ offset + 3 ] = 0x7fff0006;
    offset += NW_RAW_TRAILER;
  }

  // Send trailer
  buf[ offset ] = 0;
  buf[ offset + 1 ] = 0x7fff0000;
  offset += NW_SEND_TRAILER;
  return offset;
}



inline void addEvent(int* buf, int nwords_per_fee, unsigned int event, int ncpr, int nhslb)
//inline void addEvent(int* buf, int nwords, unsigned int event)
{
  int offset = 0;
  int prev_offset;
  buf[ offset + 4 ] = event;
  offset += NW_SEND_HEADER;

  for (int k = 0; k < ncpr; k++) {
    int nwords = buf[ offset ];
    int posback_xorchksum = 2;
    int pos_xorchksum = offset + nwords - posback_xorchksum;
    prev_offset = offset;
    if (buf[ offset + 4 ] != CTIME_VAL) {
      printf("[FATAL] data-production error 2 0x%.x", buf[ offset + 4 ]);
      fflush(stdout);
      exit(1);
    }
    // RawHeader
    buf[ pos_xorchksum ] ^= buf[ offset + 3];
    buf[ offset + 3] = event;
    buf[ pos_xorchksum ] ^= buf[ offset + 3];

    // COPPER header
    offset += NW_RAW_HEADER;
    for (int i = 0; i < nhslb ; i++) {
      if ((buf[ offset + 1 ] & 0xffff0000) != 0xffaa0000) {
        printf("[FATAL] data-production error 3 : 0x%.x hslb %d cpr %d\n", buf[ offset ], i, k);
        fflush(stdout);
        exit(1);
      }
      buf[ offset +  1 ] = 0xffaa0000 + (event & 0xffff);
      buf[ offset +  3 ] = event;

#ifdef CRC_ON
      int* crc_buf = buf + offset + 2; // 1 => size of HSLB B2L header
      int crc_nwords = nwords_per_fee + 5; // 5 => size of FEE B2L header
      unsigned short temp_crc16 = CalcCRC16LittleEndian(0xffff, crc_buf, crc_nwords);
      buf[ offset + NW_B2L_HEADER + nwords_per_fee + 1 ] = ((event & 0x0000ffff) <<  16) | temp_crc16;
#endif

#ifdef REDUCED_DATA
      offset += NW_B2L_HEADER + nwords_per_fee + NW_B2L_TRAILER;
#else
      offset += NW_B2L_HEADER + nwords_per_fee + NW_B2L_TRAILER;
#endif
    }
    offset += NW_RAW_TRAILER;
    unsigned int xor_chksum = 0;
    unsigned int xor_chksum2 = 0;
  }

}


int main(int argc, char** argv)
{

  printf("###################################################\n");
#ifdef REDUCED_DATA
  printf("#  PCIe40 data after reduction (#define REDUCED_DATA)    #\n");
#else
  printf("#  PCIe40 data before reduction (//#define REDUCED_DATA) #\n");
#endif
  printf("###################################################\n");

  if (argc != 6) {
    printf("Usage : %s <node ID> <run#> <nwords of det. buf per FEE> <# of CPR per COPPER> <# of HSLBs>\n", argv[ 0 ]);
    exit(1);
  }

  //
  // network connection
  //
  int listenfd;
  struct sockaddr_in servaddr;
  struct pollfd client[NUM_CLIENTS + 1];

  //
  // dummy data
  //
  unsigned int node_id = 0;
  sscanf(argv[1], "0x%x", &node_id);

  int run_no = atoi(argv[2]);
  int nwords_per_fee = atoi(argv[3]);
  int ncpr = atoi(argv[4]);
  int nhslb = atoi(argv[5]);

  int total_words =  NW_SEND_HEADER + NW_SEND_TRAILER +
                     ncpr * (NW_RAW_HEADER + (NW_B2L_HEADER + NW_B2L_TRAILER + nwords_per_fee) * nhslb +
                             NW_RAW_TRAILER);
  printf("TET %d %d %d %d %d\n ", NW_SEND_HEADER + NW_SEND_TRAILER, ncpr,
         NW_RAW_HEADER, (NW_B2L_HEADER + NW_B2L_TRAILER + nwords_per_fee) * nhslb, NW_RAW_TRAILER);
  int buff[total_words];

  //
  // Prepare header
  //
  int temp_ret = fillDataContents(buff, nwords_per_fee, node_id, ncpr, nhslb, run_no);
  if (temp_ret != total_words) {
    printf("[FATAL] data-production error 1 %d %d\n", total_words, temp_ret);
    fflush(stdout);
    exit(1);
  }

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(30000);

  int flags = 1;
  int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flags, (socklen_t)sizeof(flags));
  if (ret < 0) {
    perror("Failed to set REUSEADDR");
  }

  bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

  listen(listenfd, LISTENQ);

  client[0].fd = listenfd;
  client[0].events = POLLRDNORM;

  for (int i = 1; i <= NUM_CLIENTS; i++) {
    client[i].fd = -1;
  }

  int maxi = 0;
  int id = 0;
  int nconn = 0;

  while (1) {
    int nready = poll(client, maxi + 1, -1);
    if (client[0].revents & POLLRDNORM) {
      printf("Accepting..."); fflush(stdout);
      int connfd = accept(listenfd, (struct sockaddr*) NULL, NULL);
      for (id = 1; id <= NUM_CLIENTS; id++) {
        if (client[id].fd < 0) {
          client[id].fd = connfd;
          client[id].events = POLLRDNORM;
          nconn++;
          printf("Done. connections (%d/%d)", nconn, NUM_CLIENTS); fflush(stdout);
          break;
        }
      }
      if (id > NUM_CLIENTS) {
        perror("[FATAL] too many clients.");
        exit(1);
      }
      if (id > maxi) maxi = id;
      if (nconn >= NUM_CLIENTS) break;
      //      if( -nready <= 0 ) continue;

    } else {
      printf("No connection request. Still waiting..."); fflush(stdout);
    }
  }

  printf("All connections accepted\n"); fflush(stdout);

  double init_time = getTimeSec();
  double prev_time = init_time;

  unsigned long long int cnt = 0;
  unsigned long long int prev_cnt = 0;
  unsigned long long int start_cnt = 300000;

#ifdef MAX_EVENT
  for (int j = 0; j < MAX_EVENT; j++) {
#else
  for (;;) {
#endif
    //    addEvent(buff, total_words, cnt);
    addEvent(buff, nwords_per_fee, cnt, ncpr, nhslb);
    //    printf("cnt %d bytes\n", cnt*total_words); fflush(stdout);
    //    sprintf( buff, "event %d dessa", cnt );

    // for(int i = 0 ; i < total_words ; i++){
    //    printf("%.8x ", buff[ i ]);
    //    if( i % 10 == 9 ) printf("\n");
    //  }

    for (int i = 1 ; i <= NUM_CLIENTS ; i++) {
      int ret = 0;
      if ((ret = write(client[i].fd, buff, total_words * sizeof(int))) <= 0) {
        printf("[FATAL] Return value %d\n", ret);
        fflush(stdout);
        exit(1);
      }
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
               NUM_CLIENTS * (cnt - prev_cnt)*total_words * sizeof(int) / 1000000. / (cur_time - prev_time),
               (cnt - prev_cnt) / (cur_time - prev_time) / 1000. ,
               NUM_CLIENTS * (cnt - start_cnt)*total_words * sizeof(int) / 1000000. / (cur_time - init_time),
               (cnt - start_cnt) / (cur_time - init_time) / 1000. , total_words);

        fflush(stdout);
        prev_time = cur_time;
        prev_cnt = cnt;
      } else {
        //  printf("Eve %lld\n", cnt);fflush(stdout);
      }
    }
  }


  for (int i = 1; i <= NUM_CLIENTS; i++) {
    close(client[i].fd);
  }


}
