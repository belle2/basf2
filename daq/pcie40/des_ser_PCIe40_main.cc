#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
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
#include <netdb.h>
#include <vector>
#include <set>
#include <map>
#include <cstring>
#include <omp.h>
#include <mqueue.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <fstream>
#include <errno.h>
#include "pcie40_b2dma.h"
#include "dma_driverlib.h"
#include "pcie40_s_pages.h"
#include <rawdata/dataobjects/RawHeader_latest.h>
#include <rawdata/dataobjects/RawTrailer_latest.h>
#include <rawdata/dataobjects/PreRawCOPPERFormat_latest.h>
#include <rawdata/dataobjects/PostRawCOPPERFormat_latest.h>


/////////////////////////////////////////
// Parameter for operation
////////////////////////////////////////
//#define NOT_SEND
#define NUM_CLIENTS_PER_THREAD 1
#define NUM_SENDER_THREADS 5
//#define NUM_SENDER_THREADS 1
//#define NO_ERROR_STOP
//#define PCIE40_NODE_ID 0x03000001
/////////////////////////////////////////
// Parameter for dummy-data
////////////////////////////////////////
//#define DUMMY_REDUCED_DATA

/////////////////////////////////////////
// Parameter for data-contents
////////////////////////////////////////

//
// Format (PCIe40)
//
#define NW_RAW_HEADER 56
#define NW_RAW_TRAILER 4
#define NW_REDUCED_B2L_HEADER 2
#define NW_REDUCED_B2L_TRAILER 2
#define NW_NONREDUCED_B2L_HEADER 6
#define NW_NONREDUCED_B2L_TRAILER 3

//
// Sender format
//
#define NW_SEND_HEADER 6
#define NW_SEND_TRAILER 2


//
// return value of checkEventData()
//
#define DATACHECK_OK 0
#define DATACHECK_NG 1
#define DATACHECK_OK_BUT_ERRFLAG_IN_HDR 2

//
// Dummy data contents
//
#define CTIME_VAL 0x12345601
#ifdef DUMMY_REDUCED_DATA
#define NW_B2L_HEADER 3
#define NW_B2L_TRAILER 2
#else
#define NW_B2L_HEADER 7
#define NW_B2L_TRAILER 3
#endif

using namespace std;

unsigned int* data_1[NUM_SENDER_THREADS];
unsigned int* data_2[NUM_SENDER_THREADS];

//pthread_t sender_thr[NUM_CLIENTS];
pthread_mutex_t mtx1_ch[NUM_SENDER_THREADS];
pthread_mutex_t mtx2_ch[NUM_SENDER_THREADS];
pthread_mutex_t mtx_sender_log;

int buffer_filled[NUM_SENDER_THREADS][2];
int copy_nwords[NUM_SENDER_THREADS][2];

struct sender_argv {
  int sender_id;
  int run_no;  // For dummy data
  int nwords_per_fee; // For dummy data
  int ncpr; // For dummy data
  int nhslb; // For dummy data
  unsigned int node_id;
  std::vector< int > valid_ch;
};

/////////////////////////////////////////////////////////
// From main_pcie40_dmahirate.cpp
/////////////////////////////////////////////////////////
bool exit_on_error = false ;
int nTot = 100000 ;
//int max_number_of_messages = 0x10000000;
unsigned int max_number_of_messages = 10;


/////////////////////////////////////////////////////////
// Error counter in checkDMAHeader()
/////////////////////////////////////////////////////////
unsigned int dmaerr_no_data = 0;
unsigned int dmaerr_bad_size = 0;
unsigned int dmaerr_bad_size_dmatrl = 0;
unsigned int dmaerr_bad_word_size = 0;
unsigned int dmaerr_bad_header = 0;
unsigned int dmaerr_bad_dmatrl = 0;
/////////////////////////////////////////////////////////
// Error counter in checkEventData()
/////////////////////////////////////////////////////////

unsigned int total_crc_good[NUM_SENDER_THREADS] = {0};
int total_crc_errors[NUM_SENDER_THREADS] = {0};
unsigned int err_flag_cnt[NUM_SENDER_THREADS] = {0};
unsigned int cur_evtnum[NUM_SENDER_THREADS] = {0};

unsigned int err_not_reduced[NUM_SENDER_THREADS] = {0};
unsigned int err_bad_7f7f[NUM_SENDER_THREADS] = {0};
unsigned int err_bad_runnum[NUM_SENDER_THREADS] = {0};
unsigned int err_bad_linknum[NUM_SENDER_THREADS] = {0};
unsigned int err_bad_evenum[NUM_SENDER_THREADS] = {0};
unsigned int err_bad_ffaa[NUM_SENDER_THREADS] = {0};
unsigned int err_bad_ff55[NUM_SENDER_THREADS] = {0};
unsigned int err_bad_linksize[NUM_SENDER_THREADS] = {0};
unsigned int err_link_eve_jump[NUM_SENDER_THREADS] = {0};
unsigned int crc_err_ch[NUM_SENDER_THREADS][ MAX_PCIE40_CH];
unsigned int cur_exprun[NUM_SENDER_THREADS] = {0};

// format of ROB header
#define OFFSET_HDR 0
#define EVENT_LEN_POS 0+OFFSET_HDR
#define MAGIC_7F7F_POS 1+OFFSET_HDR
#define RUNNO_POS 2+OFFSET_HDR
#define EVENUM_POS 3+OFFSET_HDR
#define CTIME_POS 4+OFFSET_HDR
#define UTIME_POS 5+OFFSET_HDR
#define NODEID_POS 6+OFFSET_HDR
#define ERR_POS 7+OFFSET_HDR
#define POS_TABLE_POS 8+OFFSET_HDR
#define LEN_ROB_HEADER 56
#define LEN_ROB_TRAILER 4

#define CRC_START_POS 1
#define NON_CRC_COUNTS_REDUCED 3
#define NON_CRC_COUNTS_NOTREDUCED 4

// format b2link header
#define FFAA_POS 0
#define LINK_EVE_POS 2
#define NO_PROC
#define CRC_CHECK

#define MAX_EVENT_WORDS 500000 // 2MB
#define DMA_HDR_WORDS 8

// DMA header format
#define DMA_WORDS_OF_256BITS 0
#define DMA_HDR_MAGIC 1
#define DMA_SIZE_IN_BYTES 2
#define MAX_DMA_WORDS_OF_256BITS 0xFF


/////////////////////////////////////////////////////////
// hostname
/////////////////////////////////////////////////////////
std::map< string , unsigned int > host_nodeid;
char hostnamebuf[50];


unsigned int n_messages[17] = {0};
// std::map< int , int > n_messages = {
//   { 0 , 0 } , // no data
//   { 1 , 0 } , // bad header
//   { 2 , 0 } , // bad size
//   { 3 , 0 } , // Bad word size
//   { 4 , 0 } , // Bad belle2 header
//   { 5 , 0 } , // bad trailer size
//   { 6 , 0 } , // bad trailer
//   { 7 , 0 } , // bad 7ff code
//   { 8 , 0 } , // bad version
//   { 9 , 0 } , // bad runnber
//   { 10 , 0 } , // bad event numnber
//   { 11 , 0 } , // bad link number
//   { 12 , 0 } , // bad FFAA
//   { 13 , 0 } , // bad link size
//   { 14 , 0 } , // bad data size
//   { 15 , 0 } , // Bad CRC
//   { 16 , 0 }   // missing links
// };

extern "C" int  ecs_open(int dev, int bar);
extern "C" void ecs_close(int dev, int bar);
extern "C" unsigned ecs_read(int dev, int bar, unsigned add) ;
extern "C" int ecs_write(int dev, int bar, unsigned add, int val);

// struct shm_crc {
//   int cnt ;
//   int complete ;
//   int first_crc ;
//   int crc_data ;
//   //  unsigned int data[ 10000 ] ;
//   unsigned int data[ 9998 ] ;
//   unsigned int eve;
//   int ch;
// };

const int CRC16_XMODEM_TABLE[] = {
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
  0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
  0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
  0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
  0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
  0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
  0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
  0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
  0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
  0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
  0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
  0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
  0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
  0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
  0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
  0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
  0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
  0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
  0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
  0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
  0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
  0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
  0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
  0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
  0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
  0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
  0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
  0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
} ;

void crc_calc(unsigned int& crc, const unsigned int& data)
{
  int byte1, byte2, byte3, byte4 ;
  byte1 = data & 0xFF;
  byte2 = (data & 0xFF00) >> 8;
  byte3 = (data & 0xFF0000) >> 16;
  byte4 = (data & 0xFF000000) >> 24;
  crc = (((crc) << 8) & 0xff00) ^ CRC16_XMODEM_TABLE[(((crc) >> 8) & 0xff)^byte4] ;
  crc = (((crc) << 8) & 0xff00) ^ CRC16_XMODEM_TABLE[(((crc) >> 8) & 0xff)^byte3] ;
  crc = (((crc) << 8) & 0xff00) ^ CRC16_XMODEM_TABLE[(((crc) >> 8) & 0xff)^byte2] ;
  crc = (((crc) << 8) & 0xff00) ^ CRC16_XMODEM_TABLE[(((crc) >> 8) & 0xff)^byte1] ;
}

unsigned int get_crc(unsigned int* data , int length , unsigned int initial_value)
{
  unsigned int result = initial_value ;
  //  printf("get_crc()\n");
  for (int i = 0 ; i < length ; ++i) {
    crc_calc(result , data[ i ]) ;
    //    printf("%.8d %.8x %.8x\n", i, result, data[i]);
  }
  return result ;
}


int getEventNumber(unsigned int* data)
{
  if (0 != data) return data[4] ;
  else return -1 ;
}

void printHeader(unsigned int* data)
{
  if (0 != data) {
    printf("Header  : %8X%8X%8X%8X%8X%8X%8X%8X\n" , data[7], data[6] , data[5] , data[4],
           data[3], data[2], data[1], data[0]) ;
  } else printf("No data\n")  ;
}

void printTrailer(unsigned int* data)
{
  if (0 != data) {
    printf("Trailer  : %8X%8X%8X%8X%8X%8X%8X%8X\n" , data[7], data[6] , data[5] , data[4],
           data[3], data[2], data[1], data[0]) ;
  } else printf("No data\n")  ;
}

void printData(unsigned int* data)
{
  if (0 != data) {
    printf("Data  : %8X%8X%8X%8X%8X%8X%8X%8X\n" , data[7], data[6] , data[5] , data[4],
           data[3], data[2], data[1], data[0]) ;
  } else printf("No data\n")  ;
}

void writeToFile(std::ofstream& the_file , unsigned int* data , int size)
{
  the_file << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl ; // to separate events
  for (int i = 0 ; i < 8 * (size - 2) ; ++i) {     // Write the data in 32bit values
    the_file << std::hex <<  data[ i ] << std::endl ;
  }
}

void printLine(unsigned int* data, int pos)
{
  pos = pos - (pos % 8);
  printf("pos %.8x : ", pos);
  for (int i = pos; i < pos + 8; i++) {
    printf(" %.8x", data[ pos + i]);
  }
  printf("\n");
  fflush(stdout);
}

void printEventData(unsigned int* data)
{
  int eventSize = ((data[ 0 ] & 0xFFFF)) - 1 ;     // minus header
  printf(" eve   0 : ");
  for (int i = 0 ; i < eventSize; ++i) {
    printf("%.8x ", data[ i ]);
    if (i % 8 == 7)printf("\n eve %.3x : ", i);
  }
  fflush(stdout);
}

void printEventData(unsigned int* data, int size)
{
  printf("%.8x : ", 0);
  if (0 != data) {
    for (int i = 0 ; i < size; ++i) {
      printf("%.8x ", data[ i ]);
      if (i % 8 == 7)printf("\n%.8x : ", i + 1);
    }
  } else printf("No data\n")  ;
  printf("\n");
  fflush(stdout);
}

void printEventData(unsigned int* data, int size, int sender_id)
{
  printf("thread %d : %.8x : ", sender_id, 0);
  if (0 != data) {
    for (int i = 0 ; i < size; ++i) {
      printf("%.8x ", data[ i ]);
      if (i % 8 == 7)printf("\nthread %d : %.8x : ", sender_id, i + 1);
    }
  } else printf("No data\n")  ;
  printf("\n");
  fflush(stdout);
}

void printFullData(unsigned int* data)
{
  printf("Header  : %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x\n",
         data[0], data[1] , data[2] , data[3],
         data[4], data[5], data[6], data[7]) ;
  // printf(   "Header  : %8X%8X%8X%8X%8X%8X%8X%8X\n" , data[7], data[6] ,data[5] ,data[4],
  //           data[3], data[2], data[1], data[0] ) ;

  int eventSize = ((data[ 0 ] & 0xFF)) - 1 ;     // minus header

  printf(" data   0 : ");
  for (int i = 0 ; i < eventSize * 8 ; ++i) {
    printf("%.8x ", data[ i ]);
    if (i % 8 == 7)printf("\n data %.3x : ", i);
  }
  // for ( int i = 1 ; i < eventSize + 10 ; ++i ) {
  //   printf( "data %3d: %8X%8X%8X%8X%8X%8X%8X%8X\n" , i-1 , data[ 8*i+7 ] , data[ 8*i+6 ] , data[ 8*i+5 ] ,
  //           data[ 8*i+4 ], data[ 8*i+3 ], data[ 8*i+2 ], data[ 8*i+1 ], data[ 8*i ] ) ;
  // }
  printf("\nTrailer : %8X %8X %8X %8X %8X %8X %8X %8X\n" , data[8 * eventSize + 7], data[8 * eventSize + 6] ,
         data[8 * eventSize + 5] , data[8 * eventSize + 4],
         data[8 * eventSize + 3], data[8 * eventSize + 2], data[8 * eventSize + 1], data[8 * eventSize]) ;
  fflush(stdout);
}


int checkDMAHeader(unsigned int*& data , unsigned int& size , double& dsize , int& total_pages , int& index_pages)
{
  if (data == 0) {
    pthread_mutex_lock(&(mtx_sender_log));
    n_messages[ 0 ] = n_messages[ 0 ] + 1 ;
    if (n_messages[ 0 ] < max_number_of_messages) {
      printf("No data\n") ;
    }
    dmaerr_no_data++;
    pthread_mutex_unlock(&(mtx_sender_log));
    return 1 ;
  }

  unsigned int fragment_size = data[ DMA_WORDS_OF_256BITS ] & 0xFFFF ;
  dsize += fragment_size * 32 ;  // in bytes

  if (((data[ DMA_WORDS_OF_256BITS ] & 0xFFFF0000) != 0xEEEE0000) ||
      (data[ DMA_HDR_MAGIC ] != 0xAAAAEEEE) ||
      ((data[ DMA_SIZE_IN_BYTES ] & 0xFFFF) != 0xAAAA)) {
    pthread_mutex_lock(&(mtx_sender_log));
    n_messages[ 4 ] = n_messages[ 4 ] + 1 ;
    if (n_messages[ 4 ] < max_number_of_messages) {
      printf("[FATAL] Invalid DMA header format. ( %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x )\n",
             data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]) ;
      printFullData(data);
    }
    dmaerr_bad_header++;
    pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
    exit(1);
#endif
    return 5 ;
  } else if ((data[ DMA_WORDS_OF_256BITS ] & 0xFFFF) > MAX_DMA_WORDS_OF_256BITS) {
    pthread_mutex_lock(&(mtx_sender_log));
    n_messages[ 2 ] = n_messages[ 2 ] + 1 ;
    if (n_messages[ 2 ] < max_number_of_messages) {
      printf("[FATAL] Too large DMA packet(= %lf bytes). ( %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x )\n",
             dsize,
             data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]) ;
    }
    dmaerr_bad_size++;
    pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
    exit(1);
#endif
    return 3 ;
  } else if (((data[ DMA_SIZE_IN_BYTES ] & 0xFFFF0000) >> 16) != (fragment_size * 32)) {
    pthread_mutex_lock(&(mtx_sender_log));
    n_messages[ 3 ] = n_messages[ 3 ] + 1 ;
    if (n_messages[ 3 ] < max_number_of_messages) {
      printf("[FATAL] Inconsistent between byte-size( = %u ) and 8words-size( = %u ) in DMA header. ( %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x )\n"
             ,
             (data[ DMA_SIZE_IN_BYTES ] & 0xFFFF0000) >> 16 , fragment_size * 32,
             data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]) ;
    }
    dmaerr_bad_word_size++;
    pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
    exit(1);
#endif
    return 4 ;
  } else

    // Checktrailer
    if (data[ 8 * (fragment_size - 1) ] != fragment_size) {
      pthread_mutex_lock(&(mtx_sender_log));
      n_messages[ 5 ] = n_messages[ 5 ] + 1 ;
      if (n_messages[ 5 ] < max_number_of_messages) {
        printf("Bad size in trailer : size %.8x size in hdr %.8x bef %.8x\n" , data[8 * (fragment_size - 1)], fragment_size,
               data[ 8 * (fragment_size - 2) ]) ;
        printLine(data, 8 * (fragment_size - 1));
        printFullData(data);
        //      printEventData(data, fragment_size);
      }
      if (data[ 8 * (fragment_size - 2) ] == fragment_size) {
        dmaerr_bad_size_dmatrl++;
        fragment_size--;
      }
      pthread_mutex_unlock(&(mtx_sender_log));
    }
  // if ( ( data[ 8*(fragment_size-1)+1 ] != 0 ) || ( data[ 8*(fragment_size-1)+2 ] != 0 ) ||
  //          ( data[ 8*(fragment_size-1)+3 ] != 0 ) || ( data[ 8*(fragment_size-1)+4 ] != 0 ) ||
  //          ( data[ 8*(fragment_size-1)+5 ] != 0 ) || ( data[ 8*(fragment_size-1)+6 ] != 0 ) ||
  //          ( data[ 8*(fragment_size-1)+7 ] != 0 ) ) {
  //   n_messages[ 6 ] = n_messages[ 6 ] + 1 ;
  //   if ( n_messages[ 6 ] < max_number_of_messages ) {
  //     printf( "Bad trailer\n" ) ;
  //     printTrailer( &data[ 8*(fragment_size-1) ] ) ;
  //   }
  //   err_bad_dmatrl[sender_id]++;
  //   //    return 7 ;
  // }

  total_pages = (data[ 4 ] & 0xFFFF0000) >> 16 ;
  index_pages = (data[ 4 ] & 0xFFFF) ;

  size = fragment_size ;

  // Remve header and trailer from data
  // unsigned int * tmp = new unsigned int[ S_PAGE_SLOT_SIZE/4 ] ;
  // memcpy( tmp , &data[ 8 ], 8*(fragment_size-2)*4 ) ;
  // delete [] data ;
  // data = tmp ;
  // if ( total_pages != 1 ) return -1 ;
  return 0 ;
}


double getTimeSec()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (t.tv_sec + t.tv_usec * 1.e-6);
}

void reduceHdrTrl(unsigned int* data , unsigned int& event_nwords)
{

  //  TO CHECK LATER unsigned int event_size = data[ 8 ] ;
  unsigned int event_length = data[ Belle2::RawHeader_latest::POS_NWORDS ];
  if (event_length > 0x100000) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] Too large event size. : 0x%.8x : %u words. Exiting...\n",
           data[ Belle2::RawHeader_latest::POS_NWORDS ],
           event_length);
    printEventData(data, (event_length & 0xfffff));
    pthread_mutex_unlock(&(mtx_sender_log));
    exit(1);
  }

  unsigned int* temp_data = new unsigned int[event_length];
  memset(temp_data, 0, event_length * sizeof(unsigned int));

  if (data[ Belle2::RawHeader_latest::POS_VERSION_HDRNWORDS ] & 0x00008000) {
    // Remove non-reduced flag
    data[ Belle2::RawHeader_latest::POS_VERSION_HDRNWORDS ] =
      data[ Belle2::RawHeader_latest::POS_VERSION_HDRNWORDS ] & 0xffff7fff;
    // Remove error-flag
    data[ Belle2::RawHeader_latest::POS_TRUNC_MASK_DATATYPE ] = 0;
  } else {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] reduceHdrTrl() must not be used for already reduced-event. 7f7f word = %.8x . Exiting...\n",
           data[ Belle2::RawHeader_latest::POS_VERSION_HDRNWORDS ]);
    fflush(stdout);
    pthread_mutex_unlock(&(mtx_sender_log));
    exit(1);
  }

  // Copy header before the position table
  unsigned int cur_pos = 0;
  unsigned int dst_cur_pos = 0;
  memcpy(temp_data + dst_cur_pos, data + cur_pos,
         Belle2::RawHeader_latest::POS_CH_POS_TABLE * sizeof(unsigned int));

  cur_pos = Belle2::RawHeader_latest::POS_CH_POS_TABLE;
  dst_cur_pos = Belle2::RawHeader_latest::POS_CH_POS_TABLE;


  // Check eror flag in ROB header
  int red_linksize = 0;

  temp_data[ Belle2::RawHeader_latest::POS_CH_POS_TABLE ] = data[ Belle2::RawHeader_latest::POS_CH_POS_TABLE ];
  for (int i = 0; i < MAX_PCIE40_CH ; i++) {

    int linksize = 0;
    if (i < MAX_PCIE40_CH - 1) {
      linksize = data[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + (i + 1) ]
                 - data[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + i ];
    } else {
      linksize = event_length - (data[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + (MAX_PCIE40_CH - 1) ] +
                                 Belle2::RawTrailer_latest::RAWTRAILER_NWORDS);
    }

    if (linksize < 0) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[FATAL] event size(= %d ) for ch %d is negative. Exiting...\n",
             linksize, i);
      printEventData(data, (event_length & 0xfffff));
      fflush(stdout);
      pthread_mutex_unlock(&(mtx_sender_log));
      exit(1);
    } else if (linksize == 0) {
      // this channel is not used.
      if (i < MAX_PCIE40_CH - 1) {
        temp_data[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + i + 1 ] =
          temp_data[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + i ];
      }
      continue;
    } else {
      red_linksize = linksize
                     - (Belle2::PreRawCOPPERFormat_latest::SIZE_B2LHSLB_HEADER
                        + Belle2::PreRawCOPPERFormat_latest::SIZE_B2LFEE_HEADER
                        - Belle2::PostRawCOPPERFormat_latest::SIZE_B2LHSLB_HEADER
                        - Belle2::PostRawCOPPERFormat_latest::SIZE_B2LFEE_HEADER)
                     - (Belle2::PreRawCOPPERFormat_latest::SIZE_B2LHSLB_TRAILER
                        + Belle2::PreRawCOPPERFormat_latest::SIZE_B2LFEE_TRAILER
                        - Belle2::PostRawCOPPERFormat_latest::SIZE_B2LHSLB_TRAILER
                        - Belle2::PostRawCOPPERFormat_latest::SIZE_B2LFEE_TRAILER);

      if (i < MAX_PCIE40_CH - 1) {
        temp_data[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + i + 1 ] =
          temp_data[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + i ] + red_linksize;
      }
    }

    // Set position of data
    dst_cur_pos = temp_data[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + i ];
    cur_pos = data[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + i ];

    // Copy data(B2LHSLB header)
    temp_data[ dst_cur_pos + Belle2::PostRawCOPPERFormat_latest::POS_B2LHSLB_MAGIC]
      = data[ cur_pos + Belle2::PreRawCOPPERFormat_latest::POS_MAGIC_B2LHSLB ];
    dst_cur_pos = dst_cur_pos + Belle2::PostRawCOPPERFormat_latest::SIZE_B2LHSLB_HEADER;
    cur_pos = cur_pos + Belle2::PreRawCOPPERFormat_latest::SIZE_B2LHSLB_HEADER;


    // Copy data(B2LFEE header)
    temp_data[ dst_cur_pos + Belle2::PostRawCOPPERFormat_latest::POS_B2L_CTIME ]
      =  data[ cur_pos + Belle2::PreRawCOPPERFormat_latest::POS_B2L_CTIME ];
    dst_cur_pos = dst_cur_pos + Belle2::PostRawCOPPERFormat_latest::SIZE_B2LFEE_HEADER;
    cur_pos = cur_pos + Belle2::PreRawCOPPERFormat_latest::SIZE_B2LFEE_HEADER;

    // Copy data( Detector data )
    int numwords_det_buffer = red_linksize
                              - Belle2::PostRawCOPPERFormat_latest::SIZE_B2LHSLB_HEADER
                              - Belle2::PostRawCOPPERFormat_latest::SIZE_B2LFEE_HEADER
                              - Belle2::PostRawCOPPERFormat_latest::SIZE_B2LHSLB_TRAILER
                              - Belle2::PostRawCOPPERFormat_latest::SIZE_B2LFEE_TRAILER;

    memcpy(temp_data +  dst_cur_pos, data +  cur_pos,
           numwords_det_buffer * sizeof(unsigned int));
    dst_cur_pos = dst_cur_pos + numwords_det_buffer;
    cur_pos = cur_pos + numwords_det_buffer;

    // Copy data( B2L FEE trailer )
    temp_data[ dst_cur_pos + Belle2::PostRawCOPPERFormat_latest::POS_B2LFEE_ERRCNT_CRC16 ]
      =  data[ cur_pos + Belle2::PreRawCOPPERFormat_latest::POS_CHKSUM_B2LFEE ];
    dst_cur_pos += Belle2::PostRawCOPPERFormat_latest::SIZE_B2LFEE_TRAILER;
    cur_pos += Belle2::PreRawCOPPERFormat_latest::SIZE_B2LFEE_TRAILER;

    // Copy data( B2L HSLB trailer )
    temp_data[ dst_cur_pos + Belle2::PostRawCOPPERFormat_latest::POS_B2LHSLB_TRL_MAGIC ]
      =  data[ cur_pos + Belle2::PreRawCOPPERFormat_latest::POS_CHKSUM_B2LHSLB ];
    dst_cur_pos += Belle2::PostRawCOPPERFormat_latest::SIZE_B2LHSLB_TRAILER;
    cur_pos += Belle2::PreRawCOPPERFormat_latest::SIZE_B2LHSLB_TRAILER;
  }


  // Copy RawCOPPER trailer
  temp_data[ dst_cur_pos + Belle2::RawTrailer_latest::POS_ERROR_BIT_CH1 ] =
    data[ cur_pos + Belle2::RawTrailer_latest::POS_ERROR_BIT_CH1 ];
  temp_data[ dst_cur_pos + Belle2::RawTrailer_latest::POS_ERROR_CH2 ] =
    data[ cur_pos + Belle2::RawTrailer_latest::POS_ERROR_CH2 ];
  temp_data[ dst_cur_pos + Belle2::RawTrailer_latest::POS_CHKSUM ] =
    data[ cur_pos + Belle2::RawTrailer_latest::POS_CHKSUM ];
  temp_data[ dst_cur_pos + Belle2::RawTrailer_latest::POS_TERM_WORD ] =
    data[ cur_pos + Belle2::RawTrailer_latest::POS_TERM_WORD ];

  dst_cur_pos += Belle2::RawTrailer_latest::RAWTRAILER_NWORDS;
  cur_pos += Belle2::RawTrailer_latest::RAWTRAILER_NWORDS;

  if (dst_cur_pos > cur_pos) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] reduced data-size ( %d words ) in reduceHdrTrl() is larger than the original size ( %d words). Exiting...\n",
           dst_cur_pos, cur_pos);
    fflush(stdout);
    pthread_mutex_unlock(&(mtx_sender_log));
    exit(1);
  }

  event_nwords = dst_cur_pos;
  temp_data[ Belle2::RawHeader_latest::POS_NWORDS ] = event_nwords;

  memset(data, 0, event_nwords * sizeof(unsigned int));
  memcpy(data, temp_data, dst_cur_pos * sizeof(unsigned int));


  delete temp_data;

  return ;
}


int checkEventData(int sdr_id, unsigned int* data , unsigned int size , unsigned int& exprun ,
                   unsigned int& evtnum, unsigned int node_id, std::vector< int > valid_ch, int sender_id)
{
  int expected_number_of_links = valid_ch.size() ;
  int reduced_flag = 1; // 0 : not-reduced(error event) 1: reduced
  //  TO CHECK LATER unsigned int event_size = data[ 8 ] ;

  unsigned int event_length = data[ EVENT_LEN_POS ];

  if (event_length > 0x100000) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] Too large event size. : 0x%.8x : %d words. Exiting...\n", data[ EVENT_LEN_POS ],
           data[ EVENT_LEN_POS ]);
    printEventData(data, (event_length & 0xfffff), sender_id);
    pthread_mutex_unlock(&(mtx_sender_log));
    exit(1);
  } else if (event_length == 0) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] Specified event size is zero. : 0x%.8x : %u words. Exiting...\n",
           data[ EVENT_LEN_POS ], event_length);
    printEventData(data, 24, sender_id);
    pthread_mutex_unlock(&(mtx_sender_log));
    exit(1);
  }

  if ((data[ MAGIC_7F7F_POS ] & 0xFFFF0000) != 0x7F7F0000) {
    char err_buf[500] = {0};
    pthread_mutex_lock(&(mtx_sender_log));
    n_messages[ 7 ] = n_messages[ 7 ] + 1 ;
    if (n_messages[ 7 ] < max_number_of_messages) {
      sprintf(err_buf, "[FATAL] thread %d : ERROR_EVENT :  Invalid Magic word in ReadOut Board header( 0x%.8x ) : It must be 0x7f7f????\n"
              ,
              sender_id, data[ MAGIC_7F7F_POS ]) ;
      printf("%s\n", err_buf); fflush(stdout);
      printEventData(data, event_length, sender_id);
    }
    err_bad_7f7f[sender_id]++;
    pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
    exit(1);
#endif
    //    return 1 ;
  }

  //
  // Store nodeID
  //
  data[ NODEID_POS ] = node_id;
  // if ( ( data[ 1 ] & 0xFF00 ) >> 8 != 0 ) {
  //    pthread_mutex_lock(&(mtx_sender_log));
  //   n_messages[ 8 ] = n_messages[ 8 ] + 1 ;
  //   if ( n_messages[ 8 ] < max_number_of_messages )
  //     printf( "Bad version\n" ) ;
  //    pthread_mutex_unlock(&(mtx_sender_log));
  //   return 1 ;
  // }

  if (evtnum == 0 && sdr_id == 0) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[DEBUG] thread %d :  Printing the 1st event.\n", sender_id);
    printEventData(data, event_length, sender_id);
    pthread_mutex_unlock(&(mtx_sender_log));
  }

  if (data[ MAGIC_7F7F_POS ] & 0x00008000) {
    // not-reduced
    reduced_flag = 0;

    if (data[ ERR_POS ] == 0) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[FATAL] thread %d :  Inconsistent header %.8x and errorbit %.8x\n", sender_id, data[ MAGIC_7F7F_POS ], data[ ERR_POS ]);
      printEventData(data, event_length, sender_id);
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }
  } else {
    // reduced
    reduced_flag = 1;
    if (data[ ERR_POS ] != 0) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[FATAL] thread %d :  Inconsistent header %.8x and errorbit %.8x\n", sender_id, data[ MAGIC_7F7F_POS ], data[ ERR_POS ]);
      printEventData(data, event_length, sender_id);
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }
  }

  // event # check
  if (evtnum + NUM_SENDER_THREADS != data[EVENUM_POS]) {
    if (exprun == data[RUNNO_POS] && exprun != 0) {
      pthread_mutex_lock(&(mtx_sender_log));
      n_messages[ 10 ] = n_messages[ 10 ] + 1 ;
      if (n_messages[ 10 ] < max_number_of_messages) {
        char err_buf[500] = {0};
        sprintf(err_buf,
                "[FATAL] thread %d : %s ch=%d : ERROR_EVENT : Invalid event_number. Exiting...: cur 32bit eve %u preveve %u for all channels : prun %u crun %u\n %s %s %d\n",
                sender_id, hostnamebuf, -1,
                data[EVENUM_POS], evtnum + (NUM_SENDER_THREADS - 1),
                exprun, data[RUNNO_POS],
                __FILE__, __PRETTY_FUNCTION__, __LINE__);
        //        printf("[FATAL] Bad event number prev %.8x cur %.8x\n" , evtnum , data[EVENUM_POS]) ;
        printf("%s\n", err_buf); fflush(stdout);
        printEventData(data, event_length, sender_id);
      }
      err_bad_evenum[sender_id]++;
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }
  }
  evtnum = data[EVENUM_POS];

  // printf("checkEventData()\n"); fflush(stdout);
  // printEventData(data);

  if (exprun == 0) {
    exprun = data[RUNNO_POS];
    cur_exprun[sender_id] = exprun;
  } else {
    if (exprun != data[RUNNO_POS]) {
      if (evtnum < 0 || evtnum >= NUM_SENDER_THREADS) {
        pthread_mutex_lock(&(mtx_sender_log));
        n_messages[ 9 ] = n_messages[ 9 ] + 1 ;
        if (n_messages[ 9 ] < max_number_of_messages) {
          printf("[FATAL] thread %d :  Bad exprun(now %.8x prev. %.8x) : exp %d run %d sub %d : Exiting...\n", sender_id,
                 exprun, data[RUNNO_POS],
                 data[RUNNO_POS]  >> 22,
                 (data[RUNNO_POS] & 0x003fff00) >> 8,
                 data[RUNNO_POS] & 0x000000ff
                ) ;
          printEventData(data, event_length, sender_id);
          // printLine(data, RUNNO_POS);
          // printFullData(data);
        }
        err_bad_runnum[sender_id]++;
        pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
        exit(1);
#endif
      } else {
        exprun = data[RUNNO_POS];
        cur_exprun[sender_id] = exprun;
      }
    }
  }

  // if (0 == runnumber)
  //   runnumber = (data[RUNNO_POS] & 0xFFFFFF00) >> 8 ;
  // else {
  //   if (runnumber != ((data[RUNNO_POS] & 0xFFFFFF00) >> 8))  {
  //        pthread_mutex_lock(&(mtx_sender_log));
  //     n_messages[ 9 ] = n_messages[ 9 ] + 1 ;
  //     if (n_messages[ 9 ] < max_number_of_messages) {
  //       printf("[FATAL] Bad runnumber: %.8x\n", (data[RUNNO_POS] & 0xFFFFFF00) >> 8) ;
  //    printEventData(data, event_length);
  //     }
  //     err_bad_runnum[sender_id]++;
  //    pthread_mutex_unlock(&(mtx_sender_log));
  //     exit(1);
  //   }
  // }


  unsigned int myevtnum = data[ 3 ] ;
  unsigned int ctime = data[ 4 ] ;
  unsigned int utime = data[ 5] ;
  unsigned int exp_run = data[ 2 ] ;

  unsigned int crc_init = 0xFFFF ;
  unsigned int f_crc[ 4 ]  = { ctime , myevtnum , utime , exp_run } ;
  unsigned int first_crc = 0;

  // find number of links
  unsigned int cur_pos = 8 ;
  int non_crc_counts = 0;
  // Check eror flag in ROB header

  unsigned int first_b2lctime = 0;
  int first_b2lctime_flag = 0;

  if (reduced_flag == 1) {
    first_crc = get_crc(f_crc , 4 , crc_init) ;
    non_crc_counts = NON_CRC_COUNTS_REDUCED;
  } else {
    err_flag_cnt[sender_id]++;
    //    printf("ERROR flag : Printing a whole event... %u\n", myevtnum);
    // printEventData(data, event_length);
    first_crc = crc_init;
    non_crc_counts = NON_CRC_COUNTS_NOTREDUCED;
  }

  int first_eve_flag = 0;
  int link_cnt = 0;


  for (int i = 0; i <  MAX_PCIE40_CH; i++) {
    if (i == 0) first_b2lctime_flag = 0;
    //  while ( true ) {
    //    unsigned int linknumber = ( data[ cur_pos + 1 ] & 0xFF00 ) >> 8 ;
    int linksize = 0;
    if (i < 47) {
      linksize = data[ POS_TABLE_POS + (i + 1) ] - data[ POS_TABLE_POS + i ];
    } else {
      linksize = event_length - (data[ POS_TABLE_POS + 47 ] + LEN_ROB_TRAILER);
    }
    if (linksize <= 0) continue;
    cur_pos = data[ POS_TABLE_POS + i ] + OFFSET_HDR;

    // compare valid ch with register value
    //    printf("sdr %d i %d lcnt %d validch %d\n", sdr_id, i, link_cnt, valid_ch[link_cnt]);
    if (valid_ch[link_cnt] != i) {
      pthread_mutex_lock(&(mtx_sender_log));
      n_messages[ 11 ] = n_messages[ 11 ] + 1 ;
      if (n_messages[ 11 ] < max_number_of_messages) {
        printf("[FATAL] thread %d : A valid ch in data(=%d) is not equal to regeister value(%d) for masking\n" , sender_id, i,
               valid_ch[link_cnt]) ;
        printEventData(data, event_length, sender_id);
      }
      err_bad_linknum[sender_id]++;
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }

    if ((data[ cur_pos + FFAA_POS ] & 0xFFFF0000) != 0xFFAA0000) {
      pthread_mutex_lock(&(mtx_sender_log));
      n_messages[ 12 ] = n_messages[ 12 ] + 1 ;
      if (n_messages[ 12 ] < max_number_of_messages) {
        printf("[FATAL] thread %d : Bad FFAA for linknumber %d\n", sender_id, i) ;
        printLine(data, cur_pos + FFAA_POS);
        printEventData(data, event_length, sender_id);
      }
      err_bad_ffaa[sender_id]++;
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
      //      return 1 ;
    }

    // b2link time check
    if (evtnum % 1000000 == 1000) {
      if (reduced_flag == 1) {
        time_t timer;
        struct tm* t_st;
        time(&timer);
        t_st = localtime(&timer);
        if (first_b2lctime_flag == 0) {
          first_b2lctime = data[ cur_pos + FFAA_POS + 1 ];
          first_b2lctime_flag = 1;
        }
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[DEBUG] thread %d : eve %u ch %3d B2Lctime %.8x %12u diff %12d %s", sender_id, evtnum, i, data[ cur_pos + FFAA_POS + 1 ],
               data[ cur_pos + FFAA_POS + 1 ], data[ cur_pos + FFAA_POS + 1 ] - first_b2lctime, asctime(t_st));
        pthread_mutex_unlock(&(mtx_sender_log));
      }
    }

    if (((data[ cur_pos + linksize - 1 ]) & 0xFFFF0000) != 0xFF550000) {
      pthread_mutex_lock(&(mtx_sender_log));
      n_messages[ 14 ] = n_messages[ 14 ] + 1 ;
      if (n_messages[ 14 ] < max_number_of_messages) {
        printf("[FATAL] thread %d : Bad ff55 %X pos %.8x ch %d\n" , sender_id, data[ cur_pos + linksize - 1  ], cur_pos + linksize - 1  ,
               i) ;
        printEventData(data, event_length + 16, sender_id);
      }
      err_bad_ff55[sender_id]++;
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }

    // event # jump
    if (first_eve_flag == 0) {
      first_eve_flag = 1;
    }

    unsigned int eve_link_8bits =  data[ cur_pos + FFAA_POS ]  & 0x000000ff;


    if ((evtnum & 0x000000FF) != eve_link_8bits) {
      pthread_mutex_lock(&(mtx_sender_log));
      err_link_eve_jump[sender_id]++;
      if (err_link_eve_jump[sender_id] < max_number_of_messages) {
        printf("[FATAL] thread %d : event diff. in ch %d cur_eve %.8x ffaa %.8x\n", sender_id, i, evtnum, data[ cur_pos + FFAA_POS ]);
        printEventData(data, event_length, sender_id);
      }
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }

    unsigned int ch_ffaa = (data[ cur_pos + FFAA_POS ] >> 8)  & 0x000000ff;
    if ((unsigned int)i != ch_ffaa) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[FATAL] thread %d :  Ch number is differnt ch %d ffaa %.8x\n", sender_id, i, data[ cur_pos + FFAA_POS ]);
      printEventData(data, event_length, sender_id);
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }

    if ((cur_pos + linksize) > (8 * size)) {
      pthread_mutex_lock(&(mtx_sender_log));
      n_messages[ 13 ] = n_messages[ 13 ] + 1 ;
      if (n_messages[ 13 ] < max_number_of_messages) {
        printf("[FATAL] thread %d : Bad link size %d %d\n", sender_id, (cur_pos + linksize) , (8 * size)) ;
      }
      printEventData(data, event_length, sender_id);
      err_bad_linksize[sender_id]++;
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }


    unsigned int crc_data = data[ cur_pos + linksize - 2 ] & 0xFFFF ;
    int size = linksize - non_crc_counts;
    unsigned int value = crc_data;
    unsigned int* data_for_crc = data + cur_pos + CRC_START_POS;
#ifdef CRC_CHECK
    if (get_crc(data_for_crc , size , first_crc) != value) {
      pthread_mutex_lock(&(mtx_sender_log));
      if (crc_err_ch[sender_id][i] == 0) {
        printf("[FATAL] thread %d : CRC Error calc %.4X data %.8X eve %u ch %d\n" , sender_id,
               get_crc(data_for_crc , size , first_crc) ,
               data[ cur_pos + linksize - 2 ], myevtnum, i) ;
        printf("[DEBUG] thread %d : crc_error : Printing a whole event...\n", sender_id);
        printEventData(data, event_length, sender_id);

      }
      crc_err_ch[sender_id][i]++;
      total_crc_errors[sdr_id]++;
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    } else {
      total_crc_good[sdr_id]++ ;
      //      printf("crc check ch %d pos %d val %.4x\n", i, cur_pos, value);
      //      if( true ){
    }
#endif // CRC_CHECK

    if (evtnum % 1000000 == 0) {
      //    if (total_crc_good[sdr_id] % (1000000 + sdr_id) == 0) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[DEBUG] thread %d :  CRC Good  calc %.4X data %.4X eve %u ch %d crcOK %u crcNG %d errflag %u\n" , sender_id,
             get_crc(data_for_crc , size , first_crc) ,
             value, myevtnum, i, total_crc_good[sdr_id], total_crc_errors[sender_id], err_flag_cnt[sender_id]) ;
      printf("[DEBUG] thread %d : crc_err_cnt : ", sender_id);
      for (int j = 0; j <  MAX_PCIE40_CH; j++) {
        if (crc_err_ch[sender_id][j] > 0) {
          printf("ch %d %u : ", j, crc_err_ch[sender_id][j]);
        }
      }
      printf("\n");
      fflush(stdout);
      pthread_mutex_unlock(&(mtx_sender_log));
    }

    link_cnt++;

    if (((data[ cur_pos + linksize ] & 0xFFFF0000) == 0x7FFF0000)) break ;
    cur_pos = cur_pos + linksize ;
    linksize = data[ cur_pos ] & 0xFFFF ;
  }

  if (link_cnt != expected_number_of_links) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] thread %d :  # of links(%d) in data is not the same as exptected(=%d). : Exiting...\n", sender_id,
           link_cnt, expected_number_of_links);
    printEventData(data, event_length, sender_id);
    pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
    exit(1);
#endif
  }

  //  err_bad_ff55[sender_id]++;
  if (reduced_flag == 0) {
    pthread_mutex_lock(&(mtx_sender_log));
    if (err_not_reduced[sender_id] < max_number_of_messages) {
      printf("[WARNING]  thread %d : Error-flag was set by the data-check module in PCIe40 FPGA.\n", sender_id);
      printEventData(data, event_length, sender_id);
    }
    err_not_reduced[sender_id]++;
    pthread_mutex_unlock(&(mtx_sender_log));
    //    exit(1);
    return DATACHECK_OK_BUT_ERRFLAG_IN_HDR;
  }

  return DATACHECK_OK;
}

void checkEventGenerator(unsigned int* data , int i , unsigned int size)
{
  if (data == 0) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("No data\n") ;
    pthread_mutex_unlock(&(mtx_sender_log));
    return ;
  }

  if (i != getEventNumber(data)) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("Event number mismatch %d %d\n" ,
           getEventNumber(data) , i) ;
    pthread_mutex_unlock(&(mtx_sender_log));
  }
  // Check header
  //  if ( ( data[7] != 0 ) || ( data[6] != 0 ) || ( data[5] != 0 ) || ( data[3] != 0 ) ) {
  if ((data[7] != 0) || (data[6] != 0)) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("Bad header 3 %.8x %.8x\n", data[7], data[6]) ;
    printHeader(data) ;
    pthread_mutex_unlock(&(mtx_sender_log));
  } else if ((data[ 0 ] & 0xFFFF) != size) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("Bad size %d %d\n" , data[0] & 0xFFFF , size) ;
    printLine(data, EVENT_LEN_POS);
    pthread_mutex_unlock(&(mtx_sender_log));
  } else if (((data[ 2 ] & 0xFFFF0000) >> 16) != (size * 32)) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("Bad word size %d %d\n" , (data[ 2 ] & 0xFFFF0000) >> 16 , size * 32) ;
    printHeader(data) ;
    pthread_mutex_unlock(&(mtx_sender_log));
  } else if (((data[ 0 ] & 0xFFFF0000) != 0xEEEE0000) ||
             (data[ 1 ] != 0xAAAAEEEE) ||
             ((data[ 2 ] & 0xFFFF) != 0xAAAA)) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("Bad header 4\n") ;
    printHeader(data) ;
    printEventData(data, size);
    pthread_mutex_unlock(&(mtx_sender_log));
  }
  // Check trailer
  if (data[ 8 * (size - 1) ] != size) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("Bad size in trailer %.8x %.8x\n" , data[8 * (size - 1)], size) ;
    printLine(data, 8 * (size - 1));
    pthread_mutex_unlock(&(mtx_sender_log));
  } else if ((data[ 8 * (size - 1) + 1 ] != 0) || (data[ 8 * (size - 1) + 2 ] != 0) ||
             (data[ 8 * (size - 1) + 3 ] != 0) || (data[ 8 * (size - 1) + 4 ] != 0) ||
             (data[ 8 * (size - 1) + 5 ] != 0) || (data[ 8 * (size - 1) + 6 ] != 0) ||
             (data[ 8 * (size - 1) + 7 ] != 0)) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("Bad trailer\n") ;
    printTrailer(&data[ 8 * (size - 1) ]) ;
    pthread_mutex_unlock(&(mtx_sender_log));
  }
  // Check data
  for (unsigned int j = 1 ; j < (size - 1) ; ++j) {
    if (data[ 8 * j ] != j) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("Bad data number %d %d\n" , data[8 * j] , j) ;
      pthread_mutex_unlock(&(mtx_sender_log));
    }  else if (data[8 * j + 1] != 0) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("Bad data\n") ;
      printData(&data[8 * j]) ;
      pthread_mutex_unlock(&(mtx_sender_log));
    } else if ((data[8 * j + 2] != 0xFFFFFFFF) || (data[8 * j + 3] != 0xEEEEEEEE) ||
               (data[8 * j + 4] != 0xDDDDDDDD) || (data[8 * j + 5] != 0xCCCCCCCC) ||
               (data[8 * j + 6] != 0xBBBBBBBB) || (data[8 * j + 7] != 0xAAAAAAAA)) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("Bad data\n") ;
      printData(&data[8 * j]) ;
      pthread_mutex_unlock(&(mtx_sender_log));
    }
  }
}


unsigned short CalcCRC16LittleEndian(unsigned short crc16, const int buf[], int nwords)
{

  if (nwords < 0) {

    char err_buf[500];
    pthread_mutex_lock(&(mtx_sender_log));
    sprintf(err_buf, "nwords value(%d) is invalid. Cannot calculate CRC16. Exiting...\n %s %s %d\n",
            nwords, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
    pthread_mutex_unlock(&(mtx_sender_log));
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
    //
    // RawHeader
    //
    int cpr_nwords = NW_RAW_HEADER +
                     (NW_B2L_HEADER + NW_B2L_TRAILER + nwords_per_fee) * nhslb
                     + NW_RAW_TRAILER;
    unsigned int ctime = CTIME_VAL;
    unsigned int utime = 0x98765432;

    buf[ offset +  0 ] = cpr_nwords;
#ifdef DUMMY_REDUCED_DATA
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
#ifdef DUMMY_REDUCED_DATA
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

#ifdef DUMMY_REDUCED_DATA
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
  buf[ offset + 4 ] = event;
  offset += NW_SEND_HEADER;

  for (int k = 0; k < ncpr; k++) {
    int nwords = buf[ offset ];
    int posback_xorchksum = 2;
    int pos_xorchksum = offset + nwords - posback_xorchksum;
    if (buf[ offset + 4 ] != CTIME_VAL) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[FATAL] data-production error 2 0x%.x", buf[ offset + 4 ]);
      fflush(stdout);
      pthread_mutex_unlock(&(mtx_sender_log));
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
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[FATAL] data-production error 3 : 0x%.x hslb %d cpr %d\n", buf[ offset ], i, k);
        fflush(stdout);
        pthread_mutex_unlock(&(mtx_sender_log));
        exit(1);
      }
      buf[ offset +  1 ] = 0xffaa0000 + (event & 0xffff);
      buf[ offset +  3 ] = event;

      //Adding CRC part
      int* crc_buf = buf + offset + 2; // 1 => size of HSLB B2L header
      int crc_nwords = nwords_per_fee + 5; // 5 => size of FEE B2L header
      unsigned short temp_crc16 = CalcCRC16LittleEndian(0xffff, crc_buf, crc_nwords);
      buf[ offset + NW_B2L_HEADER + nwords_per_fee + 1 ] = ((event & 0x0000ffff) <<  16) | temp_crc16;

#ifdef DUMMY_REDUCED_DATA
      offset += NW_B2L_HEADER + nwords_per_fee + NW_B2L_TRAILER;
#else
      offset += NW_B2L_HEADER + nwords_per_fee + NW_B2L_TRAILER;
#endif
    }
    offset += NW_RAW_TRAILER;
    // unsigned int xor_chksum = 0;
    // unsigned int xor_chksum2 = 0;
  }

}

//int sender_id, int   run_no, int nwords_per_fee, int ncpr, int nhslb, std::vector< int > valid_ch)
void* sender(void* arg)
{
  //
  // Get arguments
  //
  sender_argv* snd_arg = (sender_argv*)arg;
  int sender_id = snd_arg->sender_id;
  unsigned int node_id = snd_arg->node_id;
  vector<int> valid_ch = snd_arg->valid_ch;

  //
  // data
  //
  int total_words = 0;
  unsigned int* buff = new unsigned int[MAX_EVENT_WORDS];

  //
  // network connection
  //
  int port_to = 31000 + sender_id + 1;

#ifndef NOT_SEND
  //
  // Bind and listen
  //
  int fd_listen;
  struct sockaddr_in sock_listen;
  sock_listen.sin_family = AF_INET;
  //  sock_listen.sin_addr.s_addr = *(unsigned int*)host->h_addr_list[0];
  sock_listen.sin_addr.s_addr = htonl(INADDR_ANY);

  socklen_t addrlen = sizeof(sock_listen);
  sock_listen.sin_port = htons(port_to);
  fd_listen = socket(PF_INET, SOCK_STREAM, 0);

  int flags = 1;
  int ret = setsockopt(fd_listen, SOL_SOCKET, SO_REUSEADDR, &flags, (socklen_t)sizeof(flags));
  if (ret < 0) {
    perror("Failed to set REUSEADDR");
  }

  if (bind(fd_listen, (struct sockaddr*)&sock_listen, sizeof(struct sockaddr)) < 0) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] thread %d : Failed to bind. Maybe other programs have already occupied this port(%d). Exiting...\n",
           sender_id,
           port_to); fflush(stdout);
    // Check the process occupying the port 30000.
    FILE* fp;
    char buf[256];
    char cmdline[500];
    sprintf(cmdline, "/usr/sbin/ss -ap | grep %d", port_to);
    if ((fp = popen(cmdline, "r")) == NULL) {
      printf("[WARNING] thread %d : Failed to run %s\n", sender_id,
             cmdline);
    }
    pthread_mutex_unlock(&(mtx_sender_log));

    while (fgets(buf, 256, fp) != NULL) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[INFO] thread %d : Failed to bind. output of ss(port %d) : %s\n", sender_id,
             port_to, buf); fflush(stdout);
      pthread_mutex_unlock(&(mtx_sender_log));
    }
    // Error message
    fclose(fp);
    char err_buf[500];
    pthread_mutex_lock(&(mtx_sender_log));
    sprintf(err_buf, "[FATAL] thread %d : Failed to bind.(%s) Maybe other programs have already occupied this port(%d). Exiting...",
            sender_id,
            strerror(errno), port_to);
    printf("%s\n", err_buf); fflush(stdout);
    pthread_mutex_unlock(&(mtx_sender_log));
    //    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(1);
  }

  int val1 = 0;
  setsockopt(fd_listen, IPPROTO_TCP, TCP_NODELAY, &val1, (socklen_t)sizeof(val1));
  int backlog = 1;
  if (listen(fd_listen, backlog) < 0) {
    char err_buf[500];
    pthread_mutex_lock(&(mtx_sender_log));
    sprintf(err_buf, "[FATAL] thread %d : Failed in listen(%s). Exting...", sender_id,
            strerror(errno));
    printf("%s\n", err_buf); fflush(stdout);
    pthread_mutex_unlock(&(mtx_sender_log));
    //    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  }

  //
  // Accept
  //
  int fd_accept;
  struct sockaddr_in sock_accept;
  pthread_mutex_lock(&(mtx_sender_log));
  printf("[DEBUG] thread %d : Accepting... : port %d\n", sender_id,
         port_to);
  fflush(stdout);
  pthread_mutex_unlock(&(mtx_sender_log));

  if ((fd_accept = accept(fd_listen, (struct sockaddr*) & (sock_accept), &addrlen)) == 0) {
    char err_buf[500];
    pthread_mutex_lock(&(mtx_sender_log));
    sprintf(err_buf, "[FATAL] thread %d : Failed to accept(%s). Exiting...", sender_id,
            strerror(errno));
    printf("%s\n", err_buf); fflush(stdout);
    pthread_mutex_unlock(&(mtx_sender_log));
    //    print_err.PrintError(err_buf, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    exit(-1);
  } else {
    //    B2INFO("Done.");
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[INFO] thread %d : Connection(port %d) from eb0 was accepted\n", sender_id, port_to); fflush(stdout);
    pthread_mutex_unlock(&(mtx_sender_log));

    //    set timepout option
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    ret = setsockopt(fd_accept, SOL_SOCKET, SO_SNDTIMEO, &timeout, (socklen_t)sizeof(timeout));
    if (ret < 0) {
      pthread_mutex_lock(&(mtx_sender_log));
      char err_buf[500];
      sprintf(err_buf, "[FATAL] thread %d : Failed to set TIMEOUT. Exiting...", sender_id);
      printf("%s\n", err_buf); fflush(stdout);
      pthread_mutex_unlock(&(mtx_sender_log));
      exit(-1);
    }
  }

  if (fd_listen) {
    close(fd_listen);
  }
#endif

  double init_time = getTimeSec();
  double prev_time = init_time;

  unsigned long long int cnt = 0;
  unsigned long long int prev_cnt = 0;
  unsigned long long int start_cnt = 3000;

  unsigned int exprun = 0;
  unsigned int evtnum = 0;

  int buffer_id = 0;
  unsigned int send_nwords = 0;
  for (
#ifdef MAX_EVENT
    int j = 0; j < MAX_EVENT; j++
#else
    ;;
#endif
  ) {
    if (buffer_id == 0) {
      while (1) {
        if (buffer_filled[sender_id][0] == 1)break;
        usleep(1);
      }
      {
        pthread_mutex_lock(&(mtx1_ch[sender_id]));
        memcpy((buff + NW_SEND_HEADER) , data_1[sender_id], copy_nwords[sender_id][0] * sizeof(unsigned int));
        send_nwords = copy_nwords[sender_id][0];
        buffer_filled[sender_id][0] = 0;
        pthread_mutex_unlock(&(mtx1_ch[sender_id]));
      }
    } else {

      while (1) {
        if (buffer_filled[sender_id][1] == 1)break;
        usleep(1);
      }

      {
        pthread_mutex_lock(&(mtx2_ch[sender_id]));
        memcpy((buff + NW_SEND_HEADER), data_2[sender_id], copy_nwords[sender_id][1] * sizeof(unsigned int));
        send_nwords = copy_nwords[sender_id][1];
        buffer_filled[sender_id][1] = 0;
        pthread_mutex_unlock(&(mtx2_ch[sender_id]));
      }
    }

    //
    // Check data
    //
    if (buff == NULL) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[FATAL] thread %d : buffer in sender is NULL(= %p )\n", sender_id, buff); fflush(stdout);
      pthread_mutex_unlock(&(mtx_sender_log));
      exit(1);
    }
    int ret = checkEventData(sender_id, buff + NW_SEND_HEADER, send_nwords, exprun, evtnum, node_id, valid_ch, sender_id);

    if (ret != DATACHECK_OK) {
      if (ret == DATACHECK_OK_BUT_ERRFLAG_IN_HDR) {
        //        err_bad_ffaa[sender_id]++;
        unsigned int event_nwords = send_nwords - NW_SEND_HEADER - NW_SEND_TRAILER;
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[WARNING] thread %d : fake-error events are detected. Header and trailer reduction will be made and data are checked again.\n",
               sender_id);
        fflush(stdout);
        pthread_mutex_unlock(&(mtx_sender_log));
        reduceHdrTrl(buff + NW_SEND_HEADER, event_nwords);
        send_nwords = event_nwords + NW_SEND_HEADER + NW_SEND_TRAILER;

        if (evtnum != 0) {
          evtnum -= NUM_SENDER_THREADS; // To go through checkEventData().
        }
        int ret = checkEventData(sender_id, buff + NW_SEND_HEADER, send_nwords, exprun, evtnum, node_id, valid_ch, sender_id);
        if (ret != DATACHECK_OK) {
          pthread_mutex_lock(&(mtx_sender_log));
          printf("[FATAL] thread %d : checkEventData() detected an error after reduceHdrTrl(). Exiting...\n", sender_id);
          fflush(stdout);
          pthread_mutex_unlock(&(mtx_sender_log));
          exit(1);
        }
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[FATAL] thread %d : Currently, we will not tolerate a fake-error event. Exiting...\n", sender_id);
        fflush(stdout);
        pthread_mutex_unlock(&(mtx_sender_log));
        exit(1);
      } else {
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[FATAL] thread %d : checkEventData() detected an error. Exiting...\n", sender_id);
        fflush(stdout);
        pthread_mutex_unlock(&(mtx_sender_log));
        exit(1);
      }
    }

    //
    // Filling SendHeader
    //
    buff[ 0 ] = send_nwords + NW_SEND_HEADER + NW_SEND_TRAILER;
    buff[ 1 ] = 6;
    buff[ 2 ] = 0x00010001;
    buff[ 3 ] = buff[ NW_SEND_HEADER + 2 ];
    buff[ 4 ] = buff[ NW_SEND_HEADER + 3 ];
    buff[ 5 ] = buff[ NW_SEND_HEADER + 6 ];
    //
    // Filling SendTrailer
    //
    buff[ send_nwords + NW_SEND_HEADER ] = 0x0;
    buff[ send_nwords + NW_SEND_HEADER + 1 ] = 0x7fff0007;

#ifndef NOT_SEND
    ret = 0;
    int sent_bytes = 0;
    while (true) {
      if ((ret = write(fd_accept, (char*)buff + sent_bytes, (send_nwords + NW_SEND_HEADER + NW_SEND_TRAILER)
                       * sizeof(unsigned int) - sent_bytes)) <= 0) {
        if (errno == EINTR) {
          continue;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
          continue;
        } else {
          perror("[DEBuG] write() failed");
          pthread_mutex_lock(&(mtx_sender_log));
          printf("[FATAL] thread %d : write() failed. Return value of write() = %d\n", sender_id, ret);
          fflush(stdout);
          pthread_mutex_unlock(&(mtx_sender_log));
          exit(1);
        }
      }
      sent_bytes += ret;
      if (sent_bytes == (int)((send_nwords + NW_SEND_HEADER + NW_SEND_TRAILER)
                              * sizeof(unsigned int))) {
        break;
      } else if (sent_bytes > (int)((send_nwords + NW_SEND_HEADER + NW_SEND_TRAILER)
                                    * sizeof(unsigned int))) {
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[FATAL] thread %d : Too many bytes are sent\n", sender_id);
        fflush(stdout);
        pthread_mutex_unlock(&(mtx_sender_log));
        exit(1);
      }
    }
#endif
    if (buffer_id == 0) {
      buffer_id = 1;
    } else {
      buffer_id = 0;
    }
    cnt++;

    if (cnt == start_cnt) init_time = getTimeSec();
    if (cnt % 1000000 == 1) {
      if (cnt > start_cnt) {
        double cur_time = getTimeSec();
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[INFO] thread %d : evt %lld time %.1lf dataflow %.1lf MB/s rate %.2lf kHz : so far dataflow %.1lf MB/s rate %.2lf kHz size %d\n",
               sender_id,
               cnt, cur_time - init_time,
               NUM_CLIENTS_PER_THREAD * (cnt - prev_cnt)*total_words * sizeof(int) / 1000000. / (cur_time - prev_time),
               (cnt - prev_cnt) / (cur_time - prev_time) / 1000. ,
               NUM_CLIENTS_PER_THREAD * (cnt - start_cnt)*total_words * sizeof(int) / 1000000. / (cur_time - init_time),
               (cnt - start_cnt) / (cur_time - init_time) / 1000. , total_words);

        fflush(stdout);
        pthread_mutex_unlock(&(mtx_sender_log));
        prev_time = cur_time;
        prev_cnt = cnt;
      } else {
        //  printf("Eve %lld\n", cnt);fflush(stdout);
      }
    }
  }

  delete buff;
#ifndef NOT_SEND
  close(fd_accept);
#endif
  return (void*)0;
}

int main(int argc, char** argv)
{

  ///////////////////////////////////////////////
  // From main_pcie40_dmahirate.cpp
  ///////////////////////////////////////////////
  bool isData = true ;
  bool writeInFile = false ;
  if (argc != 2) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] Invalid usage of %s : %s <node ID>, node ID = 0x0, if you are not using the Belle II DAQ system.\n",
           argv[0], argv[0]) ;
    pthread_mutex_unlock(&(mtx_sender_log));
    return 0 ;
  }


  char* endptr;
  unsigned int pcie40_node_id = (unsigned int)strtol(argv[1], &endptr, 0);
  //  char tmp_arg[20];
  // if( argv[1][0] == 'x' || argv[1][0] == 'X' || argv[1][1] == 'x' || argv[1][1] == 'X' ){
  //   strncpy(tmp_arg, argv[1] + 2, 8);
  //   pcie40_node_id = (unsigned int)strtol(tmp_arg, &endptr, 0) ;
  // }else{
  //   pcie40_node_id = (unsigned int)strtol(tmp_arg, &endptr, 16) ;
  // }

  host_nodeid[ "rsvd1" ] = 0x01000001;
  host_nodeid[ "rsvd2" ] = 0x01000002;
  host_nodeid[ "rsvd3" ] = 0x01000003;
  host_nodeid[ "rsvd4" ] = 0x01000004;
  host_nodeid[ "rsvd5" ] = 0x01000005;
  host_nodeid[ "rcdc1" ] = 0x02000001;
  host_nodeid[ "rcdc2" ] = 0x02000002;
  host_nodeid[ "rcdc3" ] = 0x02000003;
  host_nodeid[ "rcdc4" ] = 0x02000004;
  host_nodeid[ "rcdc5" ] = 0x02000005;
  host_nodeid[ "rcdc6" ] = 0x02000006;
  host_nodeid[ "rcdc7" ] = 0x02000007;
  host_nodeid[ "rcdc8" ] = 0x02000008;
  host_nodeid[ "rtop1" ] = 0x03000001;
  host_nodeid[ "rtop2" ] = 0x03000002;
  host_nodeid[ "rari1" ] = 0x04000001;
  host_nodeid[ "rari2" ] = 0x04000002;
  host_nodeid[ "recl1" ] = 0x05000001;
  host_nodeid[ "recl2" ] = 0x05000002;
  host_nodeid[ "recl3" ] = 0x05000003;
  host_nodeid[ "rklm1" ] = 0x07000001;
  host_nodeid[ "rtrg1" ] = 0x10000001;

  gethostname(hostnamebuf, sizeof(hostnamebuf));
  if (pcie40_node_id != 0) {
    std::map<string, unsigned int>::iterator itr;
    itr = host_nodeid.find(hostnamebuf);
    if (itr != host_nodeid.end()) {
      if (itr->second != pcie40_node_id) {
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[FATAL] Node_id argument ( 0x%.8x ) is invalid. Node_id for %s is 0x%.8x. Exiting...\n",
               pcie40_node_id, (itr->first).c_str(), itr->second);
        pthread_mutex_unlock(&(mtx_sender_log));
        exit(1);
      } else {
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[DEBUG] (hostname %s, nodeid 0x%.8x ) concides with stored info.(  %s 0x%.8x )\n", hostnamebuf, pcie40_node_id,
               (itr->first).c_str(), itr->second); fflush(stdout);
        pthread_mutex_unlock(&(mtx_sender_log));
      }
    } else {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[FATAL] This sever's hostname is not for a PCIe40 ROPC( %s ). Exiting...\n", hostnamebuf);
      pthread_mutex_unlock(&(mtx_sender_log));
      exit(1);
    }

  }
  fflush(stdout);

  ///////////////////////////////////////////////
  // buffer for inter-threads communication
  ///////////////////////////////////////////////
  for (int i = 0; i < NUM_SENDER_THREADS; i++) {
    data_1[i] = new unsigned int[MAX_EVENT_WORDS];
    data_2[i] = new unsigned int[MAX_EVENT_WORDS];
  }




  ///////////////////////////////////////////////
  // Initialize variables
  ///////////////////////////////////////////////
  double init_time = getTimeSec();
  double prev_time = init_time;
  unsigned long long int cnt = 0;
  unsigned long long int prev_cnt = 0;
  unsigned long long int start_cnt = 300000;
  int buffer_id[NUM_SENDER_THREADS];
  int total_words = 0;
  for (int i = 0; i < NUM_SENDER_THREADS; i++) {
    buffer_id[i] = 0;
    buffer_filled[i][0] = 0;
    buffer_filled[i][1] = 0;
    copy_nwords[i][0] = 0;
    copy_nwords[i][1] = 0;
  }


  ///////////////////////////////////////////////
  // Initialize PCIe40
  ///////////////////////////////////////////////
  printf("[DEBUG] Initializing PCIe40 readout...\n");  fflush(stdout);

  std::ofstream the_file ;
  if (writeInFile) the_file.open("data_file.txt") ;
  double triggerRate = 400 ; // kHz
  double data_size = 0. ;
  int size = 0x1F ;
  int res = ecs_open(0 , 0) ;

  if (-1 == res) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("ERROR: Could not open device (BAR 0)\n") ;
    pthread_mutex_unlock(&(mtx_sender_log));
  } else {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("SUCCESS: Device opened for ECS 0\n");
    pthread_mutex_unlock(&(mtx_sender_log));
  }

  res = ecs_open(0 , 2) ;
  if (-1 == res) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("ERROR: Could not open device (BAR 2)\n") ;
    pthread_mutex_unlock(&(mtx_sender_log));
  } else {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("SUCCESS: Device opened for ECS 2\n");
    pthread_mutex_unlock(&(mtx_sender_log));
  }
  // DMA part
  res = dma_open(0) ;
  if (-1 == res) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("ERROR: Could not open device (DMA)\n") ;
    pthread_mutex_unlock(&(mtx_sender_log));
  } else {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("SUCCESS: Device opened for DMA\n");
    pthread_mutex_unlock(&(mtx_sender_log));
  }


  // Read the active links
  unsigned int masks0 = ecs_read(0 , 2 , 0x50520) ;
  unsigned int masks1 = ecs_read(0 , 2 , 0x50540) ;
  std::vector< int > valid_ch ;
  valid_ch.clear();
  for (int i = 0 ; i < 24 ; i++) {
    if ((masks0 & (1 << i)) != 0) valid_ch.push_back(i) ;
  }
  for (int i = 24 ; i <  MAX_PCIE40_CH ; i++) {
    if ((masks1 & (1 << (i - 24))) != 0) valid_ch.push_back(i) ;
  }


  // printf("[DEBUG] mask stauts\n");
  // printf("[DEBUG] mask register : %.8x %.8x\n", masks0, masks1);
  // int temp_valid_pos = 0;
  // for (int i = 0 ; i <  MAX_PCIE40_CH ; i++) {
  //   if (valid_ch[temp_valid_pos] == i) {
  //     printf("[DEBUG] ch %d 1\n", i);
  //     temp_valid_pos++;
  //   } else {
  //     printf("[DEBUG] ch %d 0\n", i);
  //   }
  // }
  int num_of_chs = valid_ch.size() ;
  pthread_mutex_lock(&(mtx_sender_log));
  printf("[DEBUG] # of used channels = %d\n", num_of_chs); fflush(stdout);
  pthread_mutex_unlock(&(mtx_sender_log));

  // initialize sum of error counters;
  for (int i = 0; i < NUM_SENDER_THREADS; i++) {
    for (int j = 0; j <  MAX_PCIE40_CH; j++) {
      crc_err_ch[i][j] = 0;
    }
  }

  //
  // ulreset to clear FIFOs and rest state machines in userlogic
  //
  ecs_write(0 , 2 ,  0x00050400 , 0x0);
  ecs_write(0 , 2 ,  0x00050400 , 0x4);
  ecs_write(0 , 2 ,  0x00050400 , 0x0);

  unsigned int initial_value = pcie40_getNbWordInEvent(0) ;
  if (initial_value == 0) pcie40_setNbWordInEvent(0 , 0xFF) ;
  pcie40_dmaReset(0) ;
  pcie40_dmaStop(0) ;
  if (! isData) {
    pcie40_enableGenerator(0) ;
    pcie40_useDataFromGenerator(0) ;
  } else {
    pcie40_disableGenerator(0) ;
    pcie40_useDataFromFibers(0) ;
  }
  //pcie40_useFreeSignal( 0 ) ;
  int t_rate = 10416.666 / ((double) triggerRate) - 1 ;
  pcie40_setGeneratorTriggerFrequency(0 , t_rate) ;
  pcie40_setNbEvents(0 , 0) ;
  // start DAQ
  pcie40_resetSPages(0) ;
  pcie40_dmaSetReadout(0) ;
  pcie40_setSizeFromHeader(0) ;
  if (! isData)
    pcie40_setNbWordInEvent(0 , size) ;
  pcie40_setBusyLevel(0 , 0x502) ;
  dma_initialize(0) ;

  pthread_mutex_lock(&(mtx_sender_log));
  printf("[DEBUG] PCIe40 readout was initialized.\n");  fflush(stdout);
  pthread_mutex_unlock(&(mtx_sender_log));

  ///////////////////////////////////////////////
  // Make sender threads
  ///////////////////////////////////////////////
  int   run_no;
  pthread_t sender_thr[NUM_SENDER_THREADS];
  //  std::thread sender_thr[NUM_SENDER_THREADS];
  sender_argv snd_argv[NUM_SENDER_THREADS];

  for (int i = 0; i < NUM_SENDER_THREADS; i++) {
    snd_argv[i].sender_id = i;
    snd_argv[i].valid_ch = valid_ch;
    snd_argv[i].node_id = pcie40_node_id;
    int ret = pthread_create(&(sender_thr[i]), NULL, sender, &(snd_argv[i]));
    if (ret != 0) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[FATAL] Failed to create a thread. ret = %d. Exting...\n", ret);
      fflush(stdout);
      pthread_mutex_unlock(&(mtx_sender_log));
      exit(1);
    }
  }

#ifndef CRC_CHECK
  pthread_mutex_lock(&(mtx_sender_log));
  //            printf("[WARNING] CRC check by software is disabled now !! Relying on check in PCIe40 firmware\n"); fflush(stdout);
  printf("[FATAL] CRC check by software is disabled now !! Relying on check in PCIe40 firmware\n"); fflush(stdout);
  pthread_mutex_unlock(&(mtx_sender_log));
  exit(1);
#endif

  ///////////////////////////////////////////////
  // Initialize readout variables
  ///////////////////////////////////////////////
  int rv ;
  unsigned int* data ;
  unsigned int* combined_data = NULL;
  unsigned int* buf_combined = new unsigned int[ MAX_EVENT_WORDS ];
  int new_buf_combined = 0;
  long long int get_sp_cnt = 0 ;
  //  int get_sp_cnt = 0x7fff0000 ;
  int k = 0 ;

  unsigned int evtnum = 0;
  int errors = 0 ;
  unsigned int esize = 0 ;
  int total_pages = 0 ;
  int index_pages = 0 ;
  int previous_index = 0 ;
  unsigned int frag_size = 0 ;
  //  auto t1 = std::chrono::high_resolution_clock::now();
  double m_start_time = getTimeSec();
  double m_prev_time = 0.;
  unsigned int m_prev_nevt = 0;
  unsigned int event_words = 0;
  double total_size_bytes = 0.;
  double prev_total_size_bytes = 0.;
  double total_eve_cnt = 0.;
  double prev_total_eve_cnt = 0.;
  int first_flag = 0;
  int first_event_flag = 0;
  unsigned int evecnt = 0;
  int client_id = 0;
  int dma_hdr_offset = 0;

  ///////////////////////////////////////////////
  // Main loop
  ///////////////////////////////////////////////
  pthread_mutex_lock(&(mtx_sender_log));
  printf("[INFO] des_ser_PCIe40_main: Reading the 1st event from a PCIe40 board...\n"); fflush(stdout);
  pthread_mutex_unlock(&(mtx_sender_log));
  for (;;) {
    ///////////////////////////////////////////////
    // Main loop
    ///////////////////////////////////////////////
    while (true) {
      //      usleep(100000);
      // start DMA and wait for one or more super pages of data
      rv = pcie40_dmaStart(0) ;
      //printf( "Number of super page received: %d\n" , rv ) ;
      //    #pragma omp parallel for
      for (int j = 0 ; j < rv * S_PAGE_SLOT_NMB ; ++j) {
        event_words = 0;
        data = pcie40_getSuperPageCopy(0 , (get_sp_cnt / S_PAGE_SLOT_NMB) % S_PAGES  , get_sp_cnt % S_PAGE_SLOT_NMB) ;
        if (! isData) {
          checkEventGenerator(data , get_sp_cnt , size);
        } else {
          // Check DMA header and trailer
          int ret = checkDMAHeader(data , frag_size , data_size , total_pages , index_pages) ;

          if (first_event_flag == 0) {
            pthread_mutex_lock(&(mtx_sender_log));
            printf("[INFO] des_ser_PCIe40_main: Done. the size of the 1st packet is %d bytes.\n", (int)data_size); fflush(stdout);
            pthread_mutex_unlock(&(mtx_sender_log));
            first_event_flag = 1;
          }

          if (first_flag == 0 && index_pages != 0 && ret < 1) {
            pthread_mutex_lock(&(mtx_sender_log));
            printf("Invalid index error : tot %d index %d ret %d\n", total_pages, index_pages, ret);
            pthread_mutex_unlock(&(mtx_sender_log));
            ret = 1;
          }
          first_flag = 1;

          if (ret == 0) {   // No error in checkDMAHeader()
            if (total_pages > 1 && total_pages <= 0xffff) {   // Multiple DMA packets for an event
              //
              // Prepare buffer for combined data
              //
              if (index_pages == 0) {
                esize = frag_size ;
                if (total_pages * S_PAGE_SLOT_SIZE / 4 > MAX_EVENT_WORDS) {
                  new_buf_combined = 1;
                  combined_data = new unsigned int[ total_pages * S_PAGE_SLOT_SIZE / 4 ] ;
                } else {
                  new_buf_combined = 0;
                  combined_data = buf_combined;
                }
              } else {
                esize += frag_size ;
              }

              if (combined_data == NULL) {
                pthread_mutex_lock(&(mtx_sender_log));
                printf("[FATAL] Data buffer is not yet allocated. %p\n", combined_data);
                pthread_mutex_unlock(&(mtx_sender_log));
                fflush(stdout);
                exit(1);
              }
              // Store a DMA packet in buffer for combined data
              memcpy(&combined_data[ previous_index ] , data + DMA_HDR_WORDS , 8 * (frag_size - 2) * 4) ;
              delete [] data ;
              data = NULL;
              previous_index = previous_index + 8 * (frag_size - 2) ;

              // Get more DMA packets to complete an event
              if (index_pages != (total_pages - 1)) {
                get_sp_cnt++;
                if ((get_sp_cnt > 0) && ((get_sp_cnt % S_PAGE_SLOT_NMB) == 0)) pcie40_freeSuperPage(0 , 1) ;
                continue ; //
              }
              // End of an event
              dma_hdr_offset = 0;

            } else if (total_pages == 1) { // One DMA packet for an event
              // End of an event
              esize = frag_size ;
              combined_data = data;
              new_buf_combined = 2;   // Delete data[] later
              dma_hdr_offset = DMA_HDR_WORDS;
            } else {
              pthread_mutex_lock(&(mtx_sender_log));
              printf("Invalid total pages %d\n", total_pages);
              pthread_mutex_unlock(&(mtx_sender_log));
              exit(1);
            }
          } else {
            exit(1);
            if (exit_on_error) exit(0) ;
            errors++ ;
          }

          //
          // End of an event
          //
          if (ret < 1) {
            if (combined_data != NULL) {
              //              if (k < 10)printFullData(combined_data + dma_hdr_offset);
              event_words = combined_data[ dma_hdr_offset + EVENT_LEN_POS ];
              if (event_words >= 0 &&  event_words < 32000) {
                total_size_bytes += ((double)event_words) * 4.;
                total_eve_cnt++;
              } else {
                pthread_mutex_lock(&(mtx_sender_log));
                printf("Strange event size %.8x ret %d\n", event_words, ret);
                printFullData(combined_data + dma_hdr_offset);
                pthread_mutex_unlock(&(mtx_sender_log));
              }
            }
            evecnt++;
          }
        }
        previous_index = 0 ;

        //
        // Send data to senders
        //
        //  if( false ){
        if (event_words > 0 && event_words < MAX_EVENT_WORDS) {
          //  printf("sta 2\n");fflush(stdout);
          client_id = client_id % NUM_SENDER_THREADS;

          if (buffer_id[client_id] == 0) {
            while (1) {
              if (buffer_filled[client_id][0] == 0)break;
              usleep(1);
            }

            {
              pthread_mutex_lock(&(mtx1_ch[client_id]));
              memcpy(data_1[client_id], combined_data + dma_hdr_offset, event_words * sizeof(unsigned int));
              copy_nwords[client_id][0] = event_words;
              buffer_filled[client_id][0] = 1;
              pthread_mutex_unlock(&(mtx1_ch[client_id]));
            }
          } else {
            while (1) {
              if (buffer_filled[client_id][1] == 0)break;
              usleep(1);
            }
            {
              pthread_mutex_lock(&(mtx2_ch[client_id]));
              fflush(stdout);
              memcpy(data_2[client_id], combined_data + dma_hdr_offset, event_words * sizeof(unsigned int));
              copy_nwords[client_id][1] = event_words;
              buffer_filled[client_id][1] = 1;
              pthread_mutex_unlock(&(mtx2_ch[client_id]));
            }
          }

          if (buffer_id[client_id] == 0) {
            buffer_id[client_id] = 1;
          } else {
            buffer_id[client_id] = 0;
          }
          client_id++;
        } else {
          pthread_mutex_lock(&(mtx_sender_log));
          printf("[FATAL] Invalid event-size %d\n", event_words);
          fflush(stdout);
          pthread_mutex_unlock(&(mtx_sender_log));
          exit(1);
        }

        previous_index = 0 ;
        if (new_buf_combined == 1) {
          delete [] combined_data ;
          combined_data = NULL;
        } else if (new_buf_combined == 2) {
          delete [] data ;
        }
        first_flag = 0;
        // if ( i != getEventNumber( data ) ) printf( "Mismatch event number %d %d\n" , i , getEventNumber( data ) ) ;
        get_sp_cnt++;
        ++k ;
        if ((evecnt % 100000) == 0 ||
            (0 <= evecnt && evecnt < 2) ||
            ((evecnt % 10000) == 0 && 0 < evecnt && evecnt < 100000)) {
          unsigned int sum_total_crc_good = 0;
          unsigned int sum_total_crc_errors = 0;
          unsigned int sum_err_flag_cnt = 0;
          unsigned int sum_cur_evtnum = 0;
          unsigned int sum_err_not_reduced = 0;
          unsigned int sum_err_bad_7f7f = 0;
          unsigned int sum_err_bad_runnum = 0;
          unsigned int sum_err_bad_linknum = 0;
          unsigned int sum_err_bad_evenum = 0;
          unsigned int sum_err_bad_ffaa = 0;
          unsigned int sum_err_bad_ff55 = 0;
          unsigned int sum_err_bad_linksize = 0;
          unsigned int sum_err_link_eve_jump = 0;
          unsigned int sum_crc_err_ch[ MAX_PCIE40_CH] = {0};

          for (int l = 0; l < NUM_SENDER_THREADS; l++) {
            sum_total_crc_good += total_crc_good[l];
            sum_total_crc_errors +=  total_crc_errors[l];
            sum_err_flag_cnt +=  err_flag_cnt[l];
            sum_cur_evtnum +=  cur_evtnum[l];

            sum_err_not_reduced += err_not_reduced[l];
            sum_err_bad_7f7f +=  err_bad_7f7f[l];
            sum_err_bad_runnum +=  err_bad_runnum[l];
            sum_err_bad_linknum +=  err_bad_linknum[l];
            sum_err_bad_evenum +=  err_bad_evenum[l];
            sum_err_bad_ffaa +=  err_bad_ffaa[l];
            sum_err_bad_ff55 +=  err_bad_ff55[l];
            sum_err_bad_linksize +=  err_bad_linksize[l];
            sum_err_link_eve_jump +=  err_link_eve_jump[l];

            if (cur_exprun[0] != cur_exprun[l]) {
              pthread_mutex_lock(&(mtx_sender_log));
              printf("[FATAL] exprun mismatch thr 0 = 0x%.8x , thr %d = 0x%.8x", cur_exprun[0], l, cur_exprun[l]);
              pthread_mutex_unlock(&(mtx_sender_log));
              exit(1);
            }

            for (int m = 0; m <  MAX_PCIE40_CH; m++) {
              sum_crc_err_ch[m] += crc_err_ch[l][m];
            }
          }
          double cur_time = getTimeSec();
          double total_time = cur_time - m_start_time;
          double interval = cur_time - m_prev_time;
          m_prev_time = cur_time;
          time_t timer;
          struct tm* t_st;
          time(&timer);
          t_st = localtime(&timer);
          pthread_mutex_lock(&(mtx_sender_log));
          printf("[INFO] Event %12d %12d exprun %.8x Rate %6.2lf[kHz] Data %6.2lf[MB/s] RunTime %8.2lf[s] interval %8.4lf[s] eve_size %6.2lf[kB] numch %d nonred %u crcok %u crcng %u evejump %d bad_7f7f %d bad_runnum %d bad_linknum %d bad_evenum %d bad_ffaa %d bad_ff55 %d bad_linksize %d no_data %d bad_header %d bad_size %d bad_size_dmatrl %d bad_dmatrl %d bad_word_size %d %s",
                 evecnt - 1, evtnum, cur_exprun[0], (evecnt  - m_prev_nevt) / interval / 1.e3,
                 (total_size_bytes - prev_total_size_bytes) / interval / 1.e6,
                 total_time,
                 interval,
                 (total_size_bytes - prev_total_size_bytes) / (total_eve_cnt - prev_total_eve_cnt) / 1.e3,
                 num_of_chs,
                 sum_err_not_reduced, sum_total_crc_good,  sum_total_crc_errors,  sum_err_link_eve_jump,
                 sum_err_bad_7f7f,
                 sum_err_bad_runnum, sum_err_bad_linknum, sum_err_bad_evenum, sum_err_bad_ffaa, sum_err_bad_ff55, sum_err_bad_linksize,
                 dmaerr_no_data, dmaerr_bad_header, dmaerr_bad_size, dmaerr_bad_size_dmatrl, dmaerr_bad_dmatrl, dmaerr_bad_word_size,
                 asctime(t_st));
          fflush(stdout);
          pthread_mutex_unlock(&(mtx_sender_log));
          prev_total_size_bytes = total_size_bytes;
          m_prev_nevt = k;
          prev_total_eve_cnt = total_eve_cnt;
        }

        if ((k % 100) == 0)
          if (writeInFile) writeToFile(the_file , data , esize) ;
        if ((get_sp_cnt > 0) && ((get_sp_cnt % S_PAGE_SLOT_NMB) == 0)) pcie40_freeSuperPage(0 , 1) ;
      }
    }

    //
    // Status Monitor
    //
    cnt++;
    if (cnt == start_cnt) init_time = getTimeSec();
    if (cnt % 10000 == 1) {
      if (cnt > start_cnt) {
        double cur_time = getTimeSec();
        pthread_mutex_lock(&(mtx_sender_log));
        printf("run %d evt %lld time %.1lf dataflow %.1lf MB/s rate %.2lf kHz : so far dataflow %.1lf MB/s rate %.2lf kHz size %d\n",
               run_no,
               cnt,
               cur_time - init_time,
               NUM_SENDER_THREADS * (cnt - prev_cnt)*total_words * sizeof(int) / 1000000. / (cur_time - prev_time),
               (cnt - prev_cnt) / (cur_time - prev_time) / 1000. ,
               NUM_SENDER_THREADS * (cnt - start_cnt)*total_words * sizeof(int) / 1000000. / (cur_time - init_time),
               (cnt - start_cnt) / (cur_time - init_time) / 1000. , total_words);
        fflush(stdout);
        pthread_mutex_unlock(&(mtx_sender_log));
        prev_time = cur_time;
        prev_cnt = cnt;
      } else {
        //  printf("Eve %lld\n", cnt);fflush(stdout);
      }
    }
  }

  //
  // Close PCIe40
  //
  ecs_close(0 , 0) ;
  ecs_close(0 , 2) ;
  dma_close(0) ;
  if (writeInFile) the_file.close() ;

  //
  // Close threads and delete buffers
  //
  for (int i = 0; i < NUM_SENDER_THREADS; i++) {
    pthread_join(sender_thr[i], NULL);
    pthread_mutex_destroy(&(mtx1_ch[i]));
    pthread_mutex_destroy(&(mtx2_ch[i]));
    delete data_1[i];
    delete data_2[i];
  }
  pthread_mutex_destroy(&mtx_sender_log);
  return 0;
}
