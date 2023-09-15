/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "des_ser_PCIe40_main.h"
using namespace std;

#ifdef SPLIT_ECL_ECLTRG
const std::vector<int> splitted_ch {16}; // recl3: cpr6001-6008,cpr13001 (0-15,16ch)
#endif

#ifndef USE_ZMQ
unsigned int* data_1[NUM_SENDER_THREADS];
unsigned int* data_2[NUM_SENDER_THREADS];

//pthread_t sender_thr[NUM_CLIENTS];
pthread_mutex_t mtx1_ch[NUM_SENDER_THREADS];
pthread_mutex_t mtx2_ch[NUM_SENDER_THREADS];
#endif
pthread_mutex_t mtx_sender_log;

#ifndef USE_ZMQ
int buffer_filled[NUM_SENDER_THREADS][2];
int copy_nwords[NUM_SENDER_THREADS][2];
#endif

#ifdef USE_ZMQ
/////////////////////////////////////////////////////////
// Handshake by ZMQ
/////////////////////////////////////////////////////////
zmq::socket_t* zmq_writer[NUM_SENDER_THREADS];
zmq::socket_t* zmq_reader[NUM_SENDER_THREADS];
#endif

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

/////////////////////////////////////////////////////////
// hostname
/////////////////////////////////////////////////////////
std::map< string, unsigned int > host_nodeid;
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

unsigned int get_crc(unsigned int* data, int length, unsigned int initial_value)
{
  unsigned int result = initial_value ;
  //  printf("get_crc()\n");
  for (int i = 0 ; i < length ; ++i) {
    crc_calc(result, data[ i ]) ;
    //    printf("%.8d %.8x %.8x\n", i, result, data[i]);
  }
  return result ;
}


int getEventNumber(const unsigned int* data)
{
  if (0 != data) return data[4] ;
  else return -1 ;
}

void printHeader(unsigned int* data)
{
  if (0 != data) {
    printf("Header  : %8X%8X%8X%8X%8X%8X%8X%8X\n", data[7], data[6], data[5], data[4],
           data[3], data[2], data[1], data[0]) ;
  } else printf("No data\n")  ;
}

void printTrailer(unsigned int* data)
{
  if (0 != data) {
    printf("Trailer  : %8X%8X%8X%8X%8X%8X%8X%8X\n", data[7], data[6], data[5], data[4],
           data[3], data[2], data[1], data[0]) ;
  } else printf("No data\n")  ;
}

void printData(unsigned int* data)
{
  if (0 != data) {
    printf("Data  : %8X%8X%8X%8X%8X%8X%8X%8X\n", data[7], data[6], data[5], data[4],
           data[3], data[2], data[1], data[0]) ;
  } else printf("No data\n")  ;
}

void writeToFile(std::ofstream& the_file, const unsigned int* data, int size)
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
         data[0], data[1], data[2], data[3],
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
  printf("\nTrailer : %8X %8X %8X %8X %8X %8X %8X %8X\n", data[8 * eventSize + 7], data[8 * eventSize + 6],
         data[8 * eventSize + 5], data[8 * eventSize + 4],
         data[8 * eventSize + 3], data[8 * eventSize + 2], data[8 * eventSize + 1], data[8 * eventSize]) ;
  fflush(stdout);
}

int get1stChannel(const unsigned int*& data)
{
  int ret_1st_ch = -1;
  unsigned int event_length = data[ Belle2::RawHeader_latest::POS_NWORDS ];

  for (int i = 0; i <  MAX_PCIE40_CH; i++) {
    int linksize = 0;
    if (i < MAX_PCIE40_CH - 1) {
      linksize = data[ POS_TABLE_POS + (i + 1) ] - data[ POS_TABLE_POS + i ];
    } else {
      linksize = event_length - (data[ POS_TABLE_POS + (MAX_PCIE40_CH - 1) ] + LEN_ROB_TRAILER);
    }
    if (linksize > 0) {
      ret_1st_ch = i;
      break;
    }
  }
  return ret_1st_ch;
}


void printEventNumberError(unsigned int*& data, const unsigned int evtnum, const unsigned int exprun, const int eve_diff,
                           const int sender_id)
{
  //
  // event # check ( Since this check is done in a single thread, only differnce in the prev. event came to this thread can be checked.
  // So, if event # from PCIe40 are in order like, 0, 3, 2, 10002, 9746, 5, 8, 7, 10007, 9753, No event jump can be issued.
  // eb0 will check futher check.
  //

  unsigned int event_length = data[ Belle2::RawHeader_latest::POS_NWORDS ];
  char err_buf[2000] = {0};
  int reduced_flag = 1; // 0 : not-reduced(error event) 1: reduced
  if (data[ MAGIC_7F7F_POS ] & 0x00008000) {
    reduced_flag = 0;
  }

  pthread_mutex_lock(&(mtx_sender_log));
  n_messages[ 10 ] = n_messages[ 10 ] + 1 ;
  if (reduced_flag == 1) {
    sprintf(err_buf,
            "[FATAL] thread %d : %s ch=%d : ERROR_EVENT : Invalid event_number. Exiting...: cur 32bit eve %u preveve %u for all channels : prun %u crun %u\n %s %s %d\n",
            sender_id, hostnamebuf, get1stChannel(data),
            data[EVENUM_POS], evtnum + (eve_diff - 1),
            exprun, data[RUNNO_POS],
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
  } else {
    sprintf(err_buf,
            "[FATAL] thread %d : %s ch=%d : ERROR_EVENT : Invalid event_number. Exiting...: cur 32bit eve %u preveve %u ( ",
            sender_id, hostnamebuf, get1stChannel(data),
            data[EVENUM_POS], evtnum + (eve_diff - 1));
    int temp_pos = 0;
    unsigned int temp_eve = 0;
    for (int i = 0; i <  MAX_PCIE40_CH; i++) {
      int linksize = 0;
      if (i < MAX_PCIE40_CH - 1) {
        linksize = data[ POS_TABLE_POS + (i + 1) ] - data[ POS_TABLE_POS + i ];
      } else {
        linksize = event_length - (data[ POS_TABLE_POS + (MAX_PCIE40_CH - 1) ] + LEN_ROB_TRAILER);
      }
      if (linksize <= 0) continue;
      temp_pos = data[ POS_TABLE_POS + i ] + OFFSET_HDR;
      temp_eve = data[ temp_pos +
                                Belle2::PreRawCOPPERFormat_latest::SIZE_B2LHSLB_HEADER +
                                Belle2::PreRawCOPPERFormat_latest::POS_TT_TAG ];

      if (evtnum + eve_diff != temp_eve) {
        sprintf(err_buf + strlen(err_buf),
                "ch %d eve 0x%.8x : ",
                i, temp_eve);
      }
    }
    sprintf(err_buf + strlen(err_buf), "prun %u crun %u\n %s %s %d\n",
            exprun, data[RUNNO_POS],
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
  }
  printf("%s\n", err_buf); fflush(stdout);
  printEventData(data, event_length, sender_id);
  err_bad_evenum[sender_id]++;
  pthread_mutex_unlock(&(mtx_sender_log));
  return;
}


void checkUtimeCtimeTRGType(unsigned int*& data, const int sender_id)
{
  unsigned int event_length = data[ Belle2::RawHeader_latest::POS_NWORDS ];
  unsigned int new_exprun = data[ Belle2::RawHeader_latest::POS_EXP_RUN_NO ] ;
  unsigned int new_evtnum = data[ Belle2::RawHeader_latest::POS_EVE_NO ] ;
  //
  // Check the 7f7f magic word
  //
  if ((data[ MAGIC_7F7F_POS ] & 0xFFFF0000) != 0x7F7F0000) {
    char err_buf[500] = {0};
    pthread_mutex_lock(&(mtx_sender_log));
    sprintf(err_buf,
            "[FATAL] thread %d : %s : ERROR_EVENT : Invalid Magic word in ReadOut Board header( 0x%.8x ) : It must be 0x7f7f???? : eve %u exp %d run %d sub %d : %s %s %d",
            sender_id, hostnamebuf, data[ MAGIC_7F7F_POS ],
            new_evtnum,
            (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
            (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
            (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
            __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s\n", err_buf); fflush(stdout);
    printEventData(data, event_length, sender_id);
    pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
    exit(1);
#endif
  }

  //
  // Check if non data-reduction bit was set or not.
  //
  if (!(data[ MAGIC_7F7F_POS ] & 0x00008000)) {
    // reduced
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] thread %d : %s : This function cannot be used for already reduced data. 7f7f header is 0x%.8x : eve %u exp %d run %d sub %d : %s %s %d\n",
           sender_id, hostnamebuf, data[ MAGIC_7F7F_POS ],
           new_evtnum,
           (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
           (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
           (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
           __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printEventData(data, event_length, sender_id);
    pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
    exit(1);
#endif
  }


  //
  // Check consistency of B2L header over all input channels
  //
  int flag = 0, err_flag = 0, err_ch = -1;
  unsigned int temp_utime = 0, temp_ctime_trgtype = 0, temp_eve = 0, temp_exprun = 0;
  unsigned int utime[MAX_PCIE40_CH], ctime_trgtype[MAX_PCIE40_CH], eve[MAX_PCIE40_CH], exprun[MAX_PCIE40_CH];
  int used_ch[MAX_PCIE40_CH] = {0};
  int first_ch = -1;

  memset(utime, 0, sizeof(utime));
  memset(ctime_trgtype, 0, sizeof(ctime_trgtype));
  memset(eve, 0, sizeof(eve));
  memset(exprun, 0, sizeof(exprun));

  for (int i = 0; i <  MAX_PCIE40_CH; i++) {
    unsigned int temp_ctime_trgtype_footer = 0, temp_eve_footer = 0;
    int linksize = 0;
    if (i < MAX_PCIE40_CH - 1) {
      linksize = data[ POS_TABLE_POS + (i + 1) ] - data[ POS_TABLE_POS + i ];
    } else {
      linksize = event_length - (data[ POS_TABLE_POS + (MAX_PCIE40_CH - 1) ] + LEN_ROB_TRAILER);
    }
    if (linksize <= 0) {
      continue;
    } else {
      used_ch[ i ] = 1;
    }

    int temp_pos = data[ POS_TABLE_POS + i ] + OFFSET_HDR;
    ctime_trgtype[ i ] = data[ temp_pos +
                               Belle2::PreRawCOPPERFormat_latest::SIZE_B2LHSLB_HEADER +
                               Belle2::PreRawCOPPERFormat_latest::POS_TT_CTIME_TYPE ];
    eve[ i ] = data[ temp_pos +
                     Belle2::PreRawCOPPERFormat_latest::SIZE_B2LHSLB_HEADER +
                     Belle2::PreRawCOPPERFormat_latest::POS_TT_TAG ];
    utime[ i ] = data[ temp_pos +
                       Belle2::PreRawCOPPERFormat_latest::SIZE_B2LHSLB_HEADER +
                       Belle2::PreRawCOPPERFormat_latest::POS_TT_UTIME ];
    exprun[ i ] = data[ temp_pos +
                        Belle2::PreRawCOPPERFormat_latest::SIZE_B2LHSLB_HEADER +
                        Belle2::PreRawCOPPERFormat_latest::POS_EXP_RUN ];
    temp_ctime_trgtype_footer = data[ temp_pos + linksize +
                                               - (Belle2::PreRawCOPPERFormat_latest::SIZE_B2LFEE_TRAILER +
                                                  Belle2::PreRawCOPPERFormat_latest::SIZE_B2LHSLB_TRAILER) +
                                               Belle2::PreRawCOPPERFormat_latest::POS_TT_CTIME_B2LFEE ];
    temp_eve_footer = data[ temp_pos + linksize +
                                     - (Belle2::PreRawCOPPERFormat_latest::SIZE_B2LFEE_TRAILER +
                                        Belle2::PreRawCOPPERFormat_latest::SIZE_B2LHSLB_TRAILER) +
                                     Belle2::PreRawCOPPERFormat_latest::POS_CHKSUM_B2LFEE ];

    if (flag == 0) {
      temp_ctime_trgtype = ctime_trgtype[ i ];
      temp_eve = eve[ i ];
      temp_utime = utime[ i ];
      temp_exprun = exprun[ i ];
      flag = 1;
      first_ch = i;

      if (temp_eve != new_evtnum) {
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[FATAL] thread %d : %s ch=%d : ERROR_EVENT : Invalid event_number. Exiting...: eve in ROBheader = 0x%.8x , ch %d 's eve = 0x%.8x : exp %d run %d sub %d : %s %s %d\n",
               sender_id, hostnamebuf, i,
               new_evtnum, i, temp_eve,
               (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
               (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
               (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
               __FILE__, __PRETTY_FUNCTION__, __LINE__);
        printEventData(data, event_length, sender_id);
        pthread_mutex_unlock(&(mtx_sender_log));
        exit(1);
      }

    } else {
      if (temp_ctime_trgtype != ctime_trgtype[ i ] || temp_utime != utime[ i ] ||
          temp_eve != eve[ i ] || temp_exprun != exprun[ i ]) {
        err_ch = i;
        err_flag = 1;
      }

    }

    //
    // Mismatch between header and trailer
    //
    if (temp_ctime_trgtype != temp_ctime_trgtype_footer || (temp_eve & 0xffff) != ((temp_eve_footer >> 16) & 0xffff)) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[FATAL] thread %d : %s ch=%d : ERROR_EVENT : mismatch(finesse %d) between header(ctime 0x%.8x eve 0x%.8x) and footer(ctime 0x%.8x eve_crc16 0x%.8x). Exiting... : exp %d run %d sub %d : %s %s %d\n",
             sender_id, hostnamebuf, i, i,
             temp_ctime_trgtype,  temp_eve, temp_ctime_trgtype_footer, temp_eve_footer,
             (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
             (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
             (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
             __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printEventData(data, event_length, sender_id);
      pthread_mutex_unlock(&(mtx_sender_log));
      exit(1);
    }
  }


  //
  // Mismatch over channels
  //
  if (err_flag == 1) {
    pthread_mutex_lock(&(mtx_sender_log));
    char err_buf[20000];
    sprintf(err_buf,
            "[FATAL] thread %d : %s ch= %d or %d : ERROR_EVENT : mismatch header value over FINESSEs ( between ch %d and ch %d ). Exiting...: ",
            sender_id, hostnamebuf, err_ch, first_ch, err_ch, first_ch);
    for (int i = 0; i <  MAX_PCIE40_CH; i++) {
      if (used_ch[ i ] == 1) {
        sprintf(err_buf + strlen(err_buf),
                "\nch = %d ctimeTRGtype 0x%.8x utime 0x%.8x eve 0x%.8x exprun 0x%.8x",
                i, ctime_trgtype[ i ], utime[ i ], eve[ i ], exprun[ i ]);
      }
    }
    printf("%s\n", err_buf); fflush(stdout);
    pthread_mutex_unlock(&(mtx_sender_log));
    exit(1);
  }

  return;
}


int checkDMAHeader(unsigned int*& data, unsigned int& size, double& dsize, int& total_pages, int& index_pages)
{
  if (data == 0) {
    pthread_mutex_lock(&(mtx_sender_log));
    n_messages[ 0 ] = n_messages[ 0 ] + 1 ;
    if (n_messages[ 0 ] < max_number_of_messages) {
      printf("[WARNING] Null pointer to data buffer\n") ;
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
             (data[ DMA_SIZE_IN_BYTES ] & 0xFFFF0000) >> 16, fragment_size * 32,
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
        printf("Bad size in trailer : size %.8x size in hdr %.8x bef %.8x\n", data[8 * (fragment_size - 1)], fragment_size,
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

void reduceHdrTrl(unsigned int* data, unsigned int& event_nwords)
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
    printf("[FATAL] reduced data-size ( %u words ) in reduceHdrTrl() is larger than the original size ( %u words). Exiting...\n",
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


int checkEventData(int sender_id, unsigned int* data, unsigned int event_nwords, unsigned int& exprun,
                   unsigned int& evtnum, unsigned int node_id, std::vector< int > valid_ch)
{
  int expected_number_of_links = valid_ch.size() ;
  int reduced_flag = 1; // 0 : not-reduced(error event) 1: reduced

  // For error message
  unsigned int new_exprun = data[ Belle2::RawHeader_latest::POS_EXP_RUN_NO ] ;
  unsigned int new_evtnum = data[ Belle2::RawHeader_latest::POS_EVE_NO ] ;

  //  TO CHECK LATER unsigned int event_size = data[ 8 ] ;
  //
  // Check if event length is not too long or zero.
  //
  unsigned int event_length = data[ EVENT_LEN_POS ];
  if (event_length > 0x100000) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] thread %d : %s : ERROR_EVENT : Too large event size. : 0x%.8x : %u words. : exp %d run %d sub %d : Exiting...\n",
           sender_id, hostnamebuf,
           data[ EVENT_LEN_POS ], data[ EVENT_LEN_POS ],
           (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
           (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
           (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK)
          );

    printEventData(data, (event_length & 0xfffff), sender_id);
    pthread_mutex_unlock(&(mtx_sender_log));
    exit(1);
  } else if (event_length == 0) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] thread %d : %s : ERROR_EVENT : Specified event size is zero. : 0x%.8x : %u words. : exp %d run %d sub %d : Exiting...\n",
           sender_id, hostnamebuf,
           data[ EVENT_LEN_POS ], event_length,
           (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
           (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
           (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK)
          );
    printEventData(data, 24, sender_id);
    pthread_mutex_unlock(&(mtx_sender_log));
    exit(1);
  }

  //
  // Check the 7f7f magic word
  //
  if ((data[ MAGIC_7F7F_POS ] & 0xFFFF0000) != 0x7F7F0000) {
    char err_buf[500] = {0};
    pthread_mutex_lock(&(mtx_sender_log));
    n_messages[ 7 ] = n_messages[ 7 ] + 1 ;
    if (n_messages[ 7 ] < max_number_of_messages) {
      sprintf(err_buf,
              "[FATAL] thread %d : %s : ERROR_EVENT :  Invalid Magic word in ReadOut Board header( 0x%.8x ) : It must be 0x7f7f???? : exp %d run %d sub %d",
              sender_id, hostnamebuf, data[ MAGIC_7F7F_POS ],
              (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
              (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
              (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK)
             ) ;
      printf("%s\n", err_buf); fflush(stdout);
      printEventData(data, event_length, sender_id);
    }
    err_bad_7f7f[sender_id]++;
    pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
    exit(1);
#endif
  }

  //
  // Store nodeID
  //
  data[ NODEID_POS ] = node_id;

  //
  // Check if non data-reduction bit was set or not.
  //
  int ffaa_pos = 0, ff55_pos_from_end = 0;
  if (data[ MAGIC_7F7F_POS ] & 0x00008000) {
    // not-reduced
    reduced_flag = 0;
    ffaa_pos = Belle2::PreRawCOPPERFormat_latest::POS_MAGIC_B2LHSLB;
    ff55_pos_from_end = - Belle2::PreRawCOPPERFormat_latest::SIZE_B2LHSLB_TRAILER +
                        Belle2::PreRawCOPPERFormat_latest::POS_CHKSUM_B2LHSLB;

    // printf("[WARNING] thread %d : Error was detected by data-check core in PCIe40 FPGA. : exp %d run %d sub %d\n",
    //          sender_id, data[ MAGIC_7F7F_POS ],
    //          (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
    //          (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
    //          (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK)
    //         );

    //     if (data[ ERR_POS ] == 0) {
    //       pthread_mutex_lock(&(mtx_sender_log));
    //       printf("[FATAL] thread %d : Data error was deteced by PCIe40 FPGA. Header %.8x, Errorbit %.8x\n", sender_id, data[ MAGIC_7F7F_POS ],
    //              data[ ERR_POS ]);
    //       printEventData(data, event_length, sender_id);
    //       pthread_mutex_unlock(&(mtx_sender_log));
    // #ifndef NO_ERROR_STOP
    //       exit(1);
    // #endif
    //    }
  } else {
    // reduced
    reduced_flag = 1;
    ffaa_pos = Belle2::PostRawCOPPERFormat_latest::POS_B2LHSLB_MAGIC;
    ff55_pos_from_end = - Belle2::PostRawCOPPERFormat_latest::SIZE_B2LHSLB_TRAILER +
                        Belle2::PostRawCOPPERFormat_latest::POS_B2LHSLB_TRL_MAGIC;
    if (data[ ERR_POS ] != 0) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[FATAL] thread %d : %s : Inconsistency between header(no error found by FPGA) %.8x and errorbit %.8x (error-bit is non-zero) : exp %d run %d sub %d\n",
             sender_id, hostnamebuf, data[ MAGIC_7F7F_POS ], data[ ERR_POS ],
             (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
             (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
             (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK)
            );
      printEventData(data, event_length, sender_id);
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }
  }

  //
  // Check event # incrementation
  //
  if (evtnum + NUM_SENDER_THREADS  != data[EVENUM_POS]) {
    if (exprun == data[RUNNO_POS]
        && exprun != 0) { // After a run-change or if this is the 1st event, event incrementation is not checked.
      printEventNumberError(data, evtnum, exprun, NUM_SENDER_THREADS, sender_id);
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }
  }


  //
  // Check exprun #
  //
  if (exprun == 0) { // default value of exprun
    exprun = data[RUNNO_POS];
  } else {
    if (exprun != data[RUNNO_POS]) {
      if (new_evtnum >= NUM_SENDER_THREADS) {
        pthread_mutex_lock(&(mtx_sender_log));
        n_messages[ 9 ] = n_messages[ 9 ] + 1 ;
        if (n_messages[ 9 ] < max_number_of_messages) {
          printf("[FATAL] thread %d : %s ch=%d : ERROR_EVENT : Bad exprun(now %.8x prev. %.8x) : exp %d run %d sub %d : Exiting...\n",
                 sender_id, hostnamebuf, get1stChannel(data),
                 exprun, data[RUNNO_POS],
                 (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
                 (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
                 (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK)
                ) ;
          printEventData(data, event_length, sender_id);
        }
        err_bad_runnum[sender_id]++;
        pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
        exit(1);
#endif
      } else {
        if (sender_id == 0) {
          printf("[DEBUG] thread %d : Run number was changed. cur exprun %.8x prev. exprun %.8x cur eve %.8x : exp %d run %d sub %d\n",
                 sender_id, data[RUNNO_POS], exprun, new_evtnum,
                 (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
                 (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
                 (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK)
                );
        }
        //
        // A new run was started.
        //
        exprun = data[RUNNO_POS];
        //
        // Initialize error counter when run # is changed.
        //
        total_crc_good[sender_id] = 0;
        total_crc_errors[sender_id] = 0;
        err_flag_cnt[sender_id] = 0;
        cur_evtnum[sender_id] = 0;
        err_not_reduced[sender_id] = 0;
        err_bad_7f7f[sender_id] = 0;
        err_bad_runnum[sender_id] = 0;
        err_bad_linknum[sender_id] = 0;
        err_bad_evenum[sender_id] = 0;
        err_bad_ffaa[sender_id] = 0;
        err_bad_ff55[sender_id] = 0;
        err_bad_linksize[sender_id] = 0;
        err_link_eve_jump[sender_id] = 0;
      }
    }
  }

  //
  // Checking each channel's header
  //
  unsigned int ctime = data[ Belle2::RawHeader_latest::POS_TTCTIME_TRGTYPE ] ;
  unsigned int utime = data[ Belle2::RawHeader_latest::POS_TTUTIME ] ;

  unsigned int crc_init = 0xFFFF ;
  unsigned int f_crc[ 4 ]  = { ctime, new_evtnum, utime, new_exprun } ;
  unsigned int first_crc = 0;

  // find number of links
  unsigned int cur_pos = 0 ;
  int non_crc_counts = 0;
  // Check eror flag in ROB header

  unsigned int first_b2lctime = 0;
  int first_b2lctime_flag = 0;

  if (reduced_flag == 1) {
    first_crc = get_crc(f_crc, 4, crc_init) ;
    non_crc_counts = NON_CRC_COUNTS_REDUCED;
  } else {
    err_flag_cnt[sender_id]++;
    //    printf("ERROR flag : Printing a whole event... %u\n", new_evtnum);
    // printEventData(data, event_length);
    first_crc = crc_init;
    non_crc_counts = NON_CRC_COUNTS_NOTREDUCED;
  }

  int first_eve_flag = 0;
  int link_cnt = 0;

  //
  // Loop over input channels
  //
  for (int i = 0; i <  MAX_PCIE40_CH; i++) {
    if (i == 0) first_b2lctime_flag = 0;

    int linksize = 0;
    if (i < MAX_PCIE40_CH - 1) {
      linksize = data[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + (i + 1) ]
                 - data[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + i ];
    } else {
      linksize = event_length - (data[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + (MAX_PCIE40_CH - 1) ] +
                                 Belle2::RawTrailer_latest::RAWTRAILER_NWORDS);
    }
    if (linksize <= 0) continue;
    cur_pos = data[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + i ] + OFFSET_HDR;

    //
    // compare valid ch with register value
    //
    if (valid_ch[link_cnt] != i) {
      pthread_mutex_lock(&(mtx_sender_log));
      n_messages[ 11 ] = n_messages[ 11 ] + 1 ;
      if (n_messages[ 11 ] < max_number_of_messages) {
        printf("[FATAL] thread %d : %s ch=%d or %d : ERROR_EVENT : HSLB or PCIe40 channel found in data is ch %d but the next channel must be ch %d according to masking register info. of PCIe40. Please check the status of channel masking.  : exp %d run %d sub %d\n",
               sender_id,
               hostnamebuf, i, valid_ch[link_cnt],
               i, valid_ch[link_cnt],
               (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
               (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
               (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK)
              );
        printEventData(data, event_length, sender_id);
      }
      err_bad_linknum[sender_id]++;
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }

    //
    // Check FFAA value
    //
    if ((data[ cur_pos + ffaa_pos ] & 0xFFFF0000) != 0xFFAA0000) {
      pthread_mutex_lock(&(mtx_sender_log));
      n_messages[ 12 ] = n_messages[ 12 ] + 1 ;
      if (n_messages[ 12 ] < max_number_of_messages) {
        char err_buf[500];
        sprintf(err_buf,
                "[FATAL] thread %d : %s ch=%d : ERROR_EVENT : HSLB or PCIe40 header magic word(0xffaa) is invalid. header %.8x : exp %d run %d sub %d : %s %s %d\n",
                sender_id,
                hostnamebuf, i,
                data[ cur_pos + ffaa_pos ],
                (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
                (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
                (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
                __FILE__, __PRETTY_FUNCTION__, __LINE__);
        printf("%s\n", err_buf); fflush(stdout);
        printLine(data, cur_pos + ffaa_pos);
        printEventData(data, event_length, sender_id);
      }
      err_bad_ffaa[sender_id]++;
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }

    //
    // b2link time check ( Only thread == 0 )
    //
    if (new_evtnum % 1000000 == 1000) {
      if (reduced_flag == 1) {
        time_t timer;
        struct tm* t_st;
        time(&timer);
        t_st = localtime(&timer);
        if (first_b2lctime_flag == 0) {
          first_b2lctime = data[ cur_pos +
                                         Belle2::PostRawCOPPERFormat_latest::SIZE_B2LHSLB_HEADER +
                                         Belle2::PostRawCOPPERFormat_latest::POS_B2L_CTIME ];
          first_b2lctime_flag = 1;
        }
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[DEBUG] thread %d : eve %u ch %3d B2Lctime 0x%.8x diff %.2lf [us] : exp %d run %d sub %d : %s",
               sender_id, new_evtnum, i,
               data[ cur_pos +
                             Belle2::PostRawCOPPERFormat_latest::SIZE_B2LHSLB_HEADER +
                             Belle2::PostRawCOPPERFormat_latest::POS_B2L_CTIME ],
               ((int)(data[ cur_pos +
                                    Belle2::PostRawCOPPERFormat_latest::SIZE_B2LHSLB_HEADER +
                                    Belle2::PostRawCOPPERFormat_latest::POS_B2L_CTIME ]
                      - first_b2lctime)) / 127.22,
               (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
               (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
               (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
               asctime(t_st));
        pthread_mutex_unlock(&(mtx_sender_log));
      }
    }

    // event # jump
    if (first_eve_flag == 0) {
      first_eve_flag = 1;
    }

    //
    // Check event number in ffaa header
    //
    unsigned int eve_link_8bits =  data[ cur_pos + ffaa_pos ]  & 0x000000ff;
    if ((new_evtnum & 0x000000FF) != eve_link_8bits) {
      pthread_mutex_lock(&(mtx_sender_log));
      err_link_eve_jump[sender_id]++;
      if (err_link_eve_jump[sender_id] < max_number_of_messages) {
        char err_buf[500] = {0};
        sprintf(err_buf,
                "[FATAL] thread %d : %s ch=%d : ERROR_EVENT : Invalid event_number (= lower 8bits in ffaa header -> 0x%.2x). Exiting...: eve 0x%.8x ffaa header 0x%.8x : exp %d run %d sub %d : %s %s %d",
                sender_id,
                hostnamebuf, i,
                data[ cur_pos + ffaa_pos ] & 0xff, new_evtnum, data[ cur_pos + ffaa_pos ],
                (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
                (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
                (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
                __FILE__, __PRETTY_FUNCTION__, __LINE__);
        printf("%s\n", err_buf); fflush(stdout);
        printEventData(data, event_length, sender_id);
      }
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }

    //
    // Check channel number in ffaa header
    //
    unsigned int ch_ffaa = (data[ cur_pos + ffaa_pos ] >> 8)  & 0x000000ff;
    if ((unsigned int)i != ch_ffaa) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[FATAL] thread %d : %s ch=%d : ERROR_EVENT : HSLB or PCIe40 channel-number is differnt. It should be ch %d in the channel table in the ROB header buf ffaa header info says ch is %u (%.8x). : exp %d run %d sub %d : %s %s %d\n",
             sender_id, hostnamebuf,  i,
             i, (data[ cur_pos + ffaa_pos ] >> 8) & 0xff,
             data[ cur_pos + ffaa_pos ],
             (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
             (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
             (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
             __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printEventData(data, event_length, sender_id);
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }


#ifdef SPLIT_ECL_ECLTRG
    //
    // Check ECLTRG FEE is connected to a proper channel
    //
    unsigned int ecl_ecltrg_1stword = 0;
    if (reduced_flag == 0) {
      ecl_ecltrg_1stword = data[ cur_pos + ffaa_pos +
                                         Belle2::PreRawCOPPERFormat_latest::SIZE_B2LHSLB_HEADER +
                                         Belle2::PreRawCOPPERFormat_latest::SIZE_B2LFEE_HEADER ];
    } else {
      ecl_ecltrg_1stword = data[ cur_pos + ffaa_pos +
                                         Belle2::PostRawCOPPERFormat_latest::SIZE_B2LHSLB_HEADER +
                                         Belle2::PostRawCOPPERFormat_latest::SIZE_B2LFEE_HEADER ];
    }

    if (((ecl_ecltrg_1stword & 0xffff0000) >> 16) == 0) {
      // ECL data
      for (int j = 0; j < splitted_ch.size(); j++) {
        if (splitted_ch[j] == i) {
          pthread_mutex_lock(&(mtx_sender_log));
          printf("[FATAL] thread %d : %s ch=%d : ECL data(1st word = %.8x , eve = %.8x ) are detected in ECLTRG channel. Maybe, fiber connection mismatch. Exiting... : exp %d run %d sub %d : %s %s %d\n",
                 sender_id,
                 hostnamebuf,  i,
                 ecl_ecltrg_1stword,
                 new_evtnum,
                 (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
                 (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
                 (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
                 __FILE__, __PRETTY_FUNCTION__, __LINE__);
          printEventData(data, event_length, sender_id);
          pthread_mutex_unlock(&(mtx_sender_log));
          exit(1);
        }
      }
    } else {
      // ECLTRG data
      int ecltrg_flag = 0;
      for (int j = 0; j < splitted_ch.size(); j++) {
        if (splitted_ch[j] == i) {
          ecltrg_flag = 1;
          break;
        }
      }

      if (ecltrg_flag == 0) {
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[FATAL] thread %d : %s ch=%d : ECLTRG data(1st word = %.8x , eve = %.8x ) are detected in ECL channel. Maybe, fiber connection mismatch. Exiting... : exp %d run %d sub %d : %s %s %d\n",
               sender_id,
               hostnamebuf,  i,
               ecl_ecltrg_1stword,
               new_evtnum,
               (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
               (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
               (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
               __FILE__, __PRETTY_FUNCTION__, __LINE__);
        printEventData(data, event_length, sender_id);
        pthread_mutex_unlock(&(mtx_sender_log));
        exit(1);
      }
    }
#endif // SPLIT_ECL_ECLTRG


    //
    // Check if the current position exceeds the event end
    //
    if (cur_pos + linksize > event_nwords - Belle2::RawTrailer_latest::RAWTRAILER_NWORDS) {
      pthread_mutex_lock(&(mtx_sender_log));
      n_messages[ 13 ] = n_messages[ 13 ] + 1 ;
      if (n_messages[ 13 ] < max_number_of_messages) {
        printf("[FATAL] thread %d : %s ch=%d : ERROR_EVENT : The end position ( %u words ) of this channel data exceeds event size( %u words ). Exiting... : exp %d run %d sub %d : %s %s %d\n",
               sender_id,
               hostnamebuf,  i,
               (cur_pos + linksize), event_nwords,
               (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
               (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
               (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
               __FILE__, __PRETTY_FUNCTION__, __LINE__);

      }
      printEventData(data, event_length, sender_id);
      err_bad_linksize[sender_id]++;
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }

    //
    // Check FF55 value
    //
    if (((data[ cur_pos + linksize + ff55_pos_from_end ]) & 0xFFFF0000) != 0xFF550000) {
      pthread_mutex_lock(&(mtx_sender_log));
      n_messages[ 14 ] = n_messages[ 14 ] + 1 ;
      if (n_messages[ 14 ] < max_number_of_messages) {
        char err_buf[500];
        sprintf(err_buf,
                "[FATAL] thread %d : %s ch=%d : ERROR_EVENT : HSLB or PCIe40 trailer magic word(0xff55) is invalid. foooter %.8x (pos.=0x%.x) : exp %d run %d sub %d : %s %s %d",
                sender_id,
                hostnamebuf, i,
                data[ cur_pos + linksize + ff55_pos_from_end ], cur_pos + linksize + ff55_pos_from_end,
                (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
                (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
                (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
                __FILE__, __PRETTY_FUNCTION__, __LINE__);
        printf("%s\n", err_buf); fflush(stdout);
        printEventData(data, event_length + 16, sender_id);
      }
      err_bad_ff55[sender_id]++;
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }



    //
    // CRC check
    //
    unsigned int crc_data = data[ cur_pos + linksize - 2 ] & 0xFFFF ;
    int size = linksize - non_crc_counts;
    unsigned int value = crc_data;
    unsigned int* data_for_crc = data + cur_pos + CRC_START_POS;
#ifdef CRC_CHECK
    if (get_crc(data_for_crc, size, first_crc) != value) {
      pthread_mutex_lock(&(mtx_sender_log));
      // Currently, zero-torellance for a CRC error.
      //      if (crc_err_ch[sender_id][i] == 0) {
      printf("[FATAL] thread %d : %s ch=%d : ERROR_EVENT : PRE CRC16 error or POST B2link event CRC16 error. data(%x) calc(%x) : eve %u exp %d run %d sub %d : %s %s %d\n",
             sender_id,
             hostnamebuf, i,
             value, get_crc(data_for_crc, size, first_crc),
             new_evtnum,
             (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
             (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
             (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
             __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printEventData(data, event_length, sender_id);
      //      }

      crc_err_ch[sender_id][i]++;
      total_crc_errors[sender_id]++;
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    } else {
      total_crc_good[sender_id]++ ;
    }
#endif // CRC_CHECK


    //
    // Monitoring CRC check status
    //
    if (new_evtnum % 1000000 == 0) {
      //    if (total_crc_good[sdr_id] % (1000000 + sdr_id) == 0) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[DEBUG] thread %d :  CRC Good  calc %.4X data %.4X eve %u ch %d crcOK %u crcNG %d errflag %u : exp %d run %d sub %d\n",
             sender_id,
             get_crc(data_for_crc, size, first_crc),
             value, new_evtnum, i, total_crc_good[sender_id], total_crc_errors[sender_id], err_flag_cnt[sender_id],
             (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
             (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
             (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK)
            ) ;
      int temp_err_cnt = 0;

      for (int j = 0; j <  MAX_PCIE40_CH; j++) {
        if (crc_err_ch[sender_id][j] > 0) {
          if (temp_err_cnt == 0) {
            printf("[DEBUG] thread %d : crc_err_cnt : ", sender_id);
            temp_err_cnt = 1;
          }
          printf("ch %d %u : ", j, crc_err_ch[sender_id][j]);
        }
      }
      if (temp_err_cnt != 0) {
        printf("\n");
      }
      fflush(stdout);
      pthread_mutex_unlock(&(mtx_sender_log));
    }

    //
    // Check the end of the event
    //
    link_cnt++;
    cur_pos = cur_pos + linksize ;
    if (((data[ cur_pos ] & 0xFFFF0000) == 0x7FFF0000)) break ;

  }

  //
  // Check if the current position exceeds the event end
  //
  if (cur_pos != event_nwords - Belle2::RawTrailer_latest::RAWTRAILER_NWORDS) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] thread %d : %s : ERROR_EVENT : The end position of channel data( %u-th word ) does not coincide with the start of RawTrailer( %d-th word ). Exiting... : exp %d run %d sub %d : %s %s %d\n",
           sender_id,
           hostnamebuf,
           cur_pos, event_nwords - Belle2::RawTrailer_latest::RAWTRAILER_NWORDS,
           (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
           (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
           (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
           __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printEventData(data, event_length, sender_id);
    pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
    exit(1);
#endif
  }


  //
  // Check the consistency of number of input links
  //
  if (link_cnt != expected_number_of_links) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] thread %d : %s : ERROR_EVENT : # of links(%d) in data is not the same as exptected(=%d). : Exiting... : exp %d run %d sub %d : %s %s %d\n",
           sender_id,
           hostnamebuf,
           link_cnt, expected_number_of_links,
           (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
           (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
           (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
           __FILE__, __PRETTY_FUNCTION__, __LINE__);

    printEventData(data, event_length, sender_id);
    pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
    exit(1);
#endif
  }


  //
  // Printing the 1st event
  //
  if (new_evtnum == 0) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[DEBUG] thread %d :  Printing the 1st event.\n", sender_id);
    printEventData(data, event_length, sender_id);
    pthread_mutex_unlock(&(mtx_sender_log));
  }



  //
  // Check unreduced header consistency
  //
  int ret = DATACHECK_OK;
  if (reduced_flag == 0) {
    checkUtimeCtimeTRGType(data, sender_id);
    pthread_mutex_lock(&(mtx_sender_log));
    if (err_not_reduced[sender_id] < max_number_of_messages) {
      printf("[WARNING] thread %d : %s ch=%d : ERROR_EVENT : Error-flag was set by the data-check module in PCIe40 FPGA. : eve %u prev thr eve %u : exp %d run %d sub %d : %s %s %d\n",
             sender_id,
             hostnamebuf, -1, new_evtnum, evtnum,
             (new_exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
             (new_exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
             (new_exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
             __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printEventData(data, event_length, sender_id);
    }
    err_not_reduced[sender_id]++;
    pthread_mutex_unlock(&(mtx_sender_log));
    //    exit(1); // zero-torellance  policy
    ret = DATACHECK_OK_BUT_ERRFLAG_IN_HDR;
  }

  evtnum = data[EVENUM_POS];
  return ret;
}

void checkEventGenerator(unsigned int* data, int i, unsigned int size)
{
  if (data == 0) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("No data\n") ;
    pthread_mutex_unlock(&(mtx_sender_log));
    return ;
  }

  if (i != getEventNumber(data)) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("Event number mismatch %d %d\n",
           getEventNumber(data), i) ;
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
    printf("Bad size %u %u\n", data[0] & 0xFFFF, size) ;
    printLine(data, EVENT_LEN_POS);
    pthread_mutex_unlock(&(mtx_sender_log));
  } else if (((data[ 2 ] & 0xFFFF0000) >> 16) != (size * 32)) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("Bad word size %u %u\n", (data[ 2 ] & 0xFFFF0000) >> 16, size * 32) ;
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
    printf("Bad size in trailer %.8x %.8x\n", data[8 * (size - 1)], size) ;
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
      printf("Bad data number %u %u\n", data[8 * j], j) ;
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
    printf("run_no %u\n", exp_run); fflush(stdout);
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

void split_Ecltrg(int sender_id, unsigned int* data, std::vector< int > valid_ch,
                  unsigned int* data_main, unsigned int* data_splitted,
                  int& event_nwords_main, int& event_nwords_splitted,
                  unsigned int splitted_node_id, std::vector< int > splitted_ch)
{
  unsigned int event_length = data[ Belle2::RawHeader_latest::POS_NWORDS ];
  // pthread_mutex_lock(&(mtx_sender_log));
  // printf("[DEBUG] Before splitting : sdrid %d.  Exiting...\n",
  //        sender_id);
  // printEventData(data, (event_length & 0xfffff));
  // pthread_mutex_unlock(&(mtx_sender_log));
  // Check event size

  if (event_length > 0x100000) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] Too large event size. : sdrid %d : 0x%.8x : %u words. Exiting...\n", sender_id, data[ EVENT_LEN_POS ],
           data[ EVENT_LEN_POS ]);
    printEventData(data, (event_length & 0xfffff));
    pthread_mutex_unlock(&(mtx_sender_log));
    exit(1);
  } else if (event_length == 0) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] Specified event size is zero. : 0x%.8x : %u words. Exiting...\n",
           data[ EVENT_LEN_POS ], event_length);
    printEventData(data, 24);
    pthread_mutex_unlock(&(mtx_sender_log));
    exit(1);
  }

  // Check magic word
  if ((data[ MAGIC_7F7F_POS ] & 0xFFFF0000) != 0x7F7F0000) {
    pthread_mutex_lock(&(mtx_sender_log));
    n_messages[ 7 ] = n_messages[ 7 ] + 1 ;
    if (n_messages[ 7 ] < max_number_of_messages) {
      printf("Bad code 7F7F ( 0x%.8x )\n", data[ MAGIC_7F7F_POS ]) ;
      //      printLine(data, MAGIC_7F7F_POS);
      printEventData(data, event_length);
    }
    err_bad_7f7f[sender_id]++;
    pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
    exit(1);
#endif
    //    return 1 ;
  }


  // Copy RawHeader
  memcpy(data_main, data, Belle2::RawHeader_latest::POS_CH_POS_TABLE * sizeof(unsigned int));
  memcpy(data_splitted, data, Belle2::RawHeader_latest::POS_CH_POS_TABLE * sizeof(unsigned int));
  data_splitted[ Belle2::RawHeader_latest::POS_NODE_ID ] = splitted_node_id;

  int cur_pos = 0;
  int cur_ch_main = 0;
  int prev_ch_main = -1;
  int cur_ch_splitted = 0;
  int prev_ch_splitted = -1;
  int cur_pos_main = Belle2::RawHeader_latest::RAWHEADER_NWORDS;
  int cur_pos_splitted = Belle2::RawHeader_latest::RAWHEADER_NWORDS;
  int link_cnt = 0;

  int cnt_main = 0;
  int cnt_splitted = 0;

  for (int i = 0; i <  MAX_PCIE40_CH; i++) {
    // Calculate linksize
    int linksize = 0;
    if (i < 47) {
      linksize = data[ POS_TABLE_POS + (i + 1) ] - data[ POS_TABLE_POS + i ];
    } else {
      linksize = event_length - (data[ POS_TABLE_POS + 47 ] + LEN_ROB_TRAILER);
    }
    if (linksize <= 0) continue;
    cur_pos = data[ POS_TABLE_POS + i ] + OFFSET_HDR;

    // compare valid ch with register value
    if (valid_ch[link_cnt] != i) {
      pthread_mutex_lock(&(mtx_sender_log));
      n_messages[ 11 ] = n_messages[ 11 ] + 1 ;
      if (n_messages[ 11 ] < max_number_of_messages) {
        printf("[FATAL] A valid ch in data(=%d) is not equal to regeister value(%d) for masking\n", i, valid_ch[link_cnt]) ;
        printEventData(data, event_length);
      }
      err_bad_linknum[sender_id]++;
      pthread_mutex_unlock(&(mtx_sender_log));
#ifndef NO_ERROR_STOP
      exit(1);
#endif
    }

    // Check main ch or splitted ch
    int splitted_ch_flag = 0;
    for (int j = 0; j < splitted_ch.size(); j++) {
      if (splitted_ch[j] == i) {
        splitted_ch_flag = 1;
        break;
      }
    }

    // Filling pos-table
    if (splitted_ch_flag == 0) {
      data_main[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + i ] = cur_pos_main;
      for (int j = prev_ch_main + 1; j < i; j++) {
        data_main[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + j ] = cur_pos_main;
      }
      memcpy(data_main + cur_pos_main, data + cur_pos, linksize * sizeof(unsigned int));
      cur_pos_main += linksize;
      prev_ch_main = i;
      cnt_main++;
    } else {
      data_splitted[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + i ] = cur_pos_splitted;
      for (int j = prev_ch_splitted + 1; j < i; j++) {
        data_splitted[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + j ] = cur_pos_splitted;
      }
      memcpy(data_splitted + cur_pos_splitted, data + cur_pos, linksize * sizeof(unsigned int));
      cur_pos_splitted += linksize;
      prev_ch_splitted = i;
      cnt_splitted++;
    }
    link_cnt++;
  }

  if (cnt_main == 0 || cnt_splitted == 0) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] No channels for ECL(# of used ch = %d) or ECLTRG(# of used ch = %d) data. Exiting...\n",
           cnt_main, cnt_splitted);
    pthread_mutex_unlock(&(mtx_sender_log));
    //    exit(1);
  }

  // Fill remaining position table
  for (int i = prev_ch_main + 1; i < MAX_PCIE40_CH; i++) {
    data_main[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + i ] = cur_pos_main;
  }
  for (int i = prev_ch_splitted + 1; i < MAX_PCIE40_CH; i++) {
    data_splitted[ Belle2::RawHeader_latest::POS_CH_POS_TABLE + i ] = cur_pos_splitted;
  }

  // Calcurate each event-length
  unsigned int eve_size_main = cur_pos_main + Belle2::RawTrailer_latest::RAWTRAILER_NWORDS;
  unsigned int eve_size_splitted = cur_pos_splitted + Belle2::RawTrailer_latest::RAWTRAILER_NWORDS;
  data_main[ Belle2::RawHeader_latest::POS_NWORDS ] = eve_size_main;
  data_splitted[ Belle2::RawHeader_latest::POS_NWORDS ] = eve_size_splitted;
  event_nwords_main = eve_size_main;
  event_nwords_splitted = eve_size_splitted;

  // Copy RawTrailer (Currently 00000000 00000000 00000000 7fff0006. So, just copy the 4 words.)
  memcpy(data_main + cur_pos_main, data + event_length - Belle2::RawTrailer_latest::RAWTRAILER_NWORDS,
         Belle2::RawTrailer_latest::RAWTRAILER_NWORDS * sizeof(unsigned int));
  memcpy(data_splitted + cur_pos_splitted, data + event_length - Belle2::RawTrailer_latest::RAWTRAILER_NWORDS,
         Belle2::RawTrailer_latest::RAWTRAILER_NWORDS * sizeof(unsigned int));

  // Copy back to data buffer
  memcpy(data, data_main, eve_size_main * sizeof(unsigned int));
  memcpy(data + eve_size_main, data_splitted, eve_size_splitted * sizeof(unsigned int));

  // pthread_mutex_lock(&(mtx_sender_log));
  // printf("[DEBUG]Splitted data sender %d\n",
  //        sender_id);
  // printEventData(data, eve_size_main + eve_size_splitted);
  // printf("[DEBUG]main data sender %d\n",
  //        sender_id);
  // printEventData(data_main, eve_size_main);
  // printf("[DEBUG]split data sender %d\n",
  //        sender_id);
  // printEventData(data_splitted, eve_size_splitted);
  // pthread_mutex_unlock(&(mtx_sender_log));

  return;
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

#ifdef SPLIT_ECL_ECLTRG
  vector<int> valid_main_ch;
  vector<int> valid_splitted_ch;
  unsigned int* buff_main = new unsigned int[MAX_EVENT_WORDS];
  unsigned int* buff_splitted = new unsigned int[MAX_EVENT_WORDS];

  int split_main_use = 0; // some unmasked channels for ECL
  int split_sub_use = 0; // some unmasked channels for ECLTRG

  // Prepare valid_main table
  for (int k = 0; k < valid_ch.size(); k++) {
    int splitted_ch_flag = 0;
    for (int l = 0; l < splitted_ch.size(); l++) {
      if (splitted_ch[l] == valid_ch[k]) {
        splitted_ch_flag = 1;
        break;
      }
    }
    if (splitted_ch_flag == 0) {
      valid_main_ch.push_back(valid_ch[k]);
      split_main_use = 1;
    } else {
      valid_splitted_ch.push_back(valid_ch[k]);
      split_sub_use = 1;
    }
  }

  if (split_main_use == 0 && split_sub_use == 0) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] thread %d : No channels are used for this PCIe40 board (ECL/ECLTRG) in %s. Please mask this readout PC with runcontrol GUI (or exclude sub-system if this is the only readout PC of the sub-system). Exiting..\n",
           sender_id, hostnamebuf);
    fflush(stdout);
    pthread_mutex_unlock(&(mtx_sender_log));
    exit(1);
  }
#endif // SPLIT_ECL_ECLTRG


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
    printf("[FATAL] thread %d : %s : Failed to bind(%s). Maybe other programs have already occupied this port(%d). Exiting...\n",
           sender_id, hostnamebuf, strerror(errno),
           port_to); fflush(stdout);
    pthread_mutex_unlock(&(mtx_sender_log));

    // Check the process occupying the port 3100?.
    FILE* fp;
    char buf[256];
    char cmdline[500];
    sprintf(cmdline, "/usr/sbin/ss -ap | grep %d", port_to);
    if ((fp = popen(cmdline, "r")) == NULL) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[WARNING] thread %d : Failed to run %s\n", sender_id,
             cmdline);
      pthread_mutex_unlock(&(mtx_sender_log));
    }

    while (fgets(buf, 256, fp) != NULL) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[DEBUG] thread %d : Port %d is used by : %s\n", sender_id,
             port_to, buf); fflush(stdout);
      pthread_mutex_unlock(&(mtx_sender_log));
    }
    fclose(fp);
    exit(1);
  }

  int val1 = 0;
  setsockopt(fd_listen, IPPROTO_TCP, TCP_NODELAY, &val1, (socklen_t)sizeof(val1));
  int backlog = 1;
  if (listen(fd_listen, backlog) < 0) {
    char err_buf[500];
    pthread_mutex_lock(&(mtx_sender_log));
    sprintf(err_buf, "[FATAL] thread %d : %s : Failed in listen(%s). Exting...",
            sender_id, hostnamebuf,
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
    sprintf(err_buf, "[FATAL] thread %d : %s : Failed to accept(%s). Exiting...",
            sender_id, hostnamebuf,
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
      sprintf(err_buf, "[FATAL] thread %d : %s : Failed to set TIMEOUT. Exiting...", sender_id, hostnamebuf);
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

#ifndef USE_ZMQ
  int buffer_id = 0;
#endif
  unsigned int tot_event_nwords = 0;
  for (
#ifdef MAX_EVENT
    int j = 0; j < MAX_EVENT; j++
#else
    ;;
#endif
  ) {

#ifdef USE_ZMQ
    // Copy data from ZMQ (experimental)
    //
    {
      zmq::message_t zevent;
      zmq_reader[sender_id]->recv(&zevent);
      memcpy(buff + NW_SEND_HEADER, zevent.data(), zevent.size());
      tot_event_nwords = zevent.size() / sizeof(unsigned int);
    }
#else
    // Copy data from buffer (orignal)
    //
    if (buffer_id == 0) {
      while (1) {
        if (buffer_filled[sender_id][0] == 1)break;
        usleep(1);
      }
      {
        pthread_mutex_lock(&(mtx1_ch[sender_id]));
        memcpy((buff + NW_SEND_HEADER), data_1[sender_id], copy_nwords[sender_id][0] * sizeof(unsigned int));
        tot_event_nwords = copy_nwords[sender_id][0];
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
        tot_event_nwords = copy_nwords[sender_id][1];
        buffer_filled[sender_id][1] = 0;
        pthread_mutex_unlock(&(mtx2_ch[sender_id]));
      }
    }
#endif

    //
    // Check data
    //
    if (buff == NULL) {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[FATAL] thread %d : %s : buffer in sender is NULL(= %p )\n", sender_id, hostnamebuf, buff); fflush(stdout);
      pthread_mutex_unlock(&(mtx_sender_log));
      exit(1);
    }

#ifdef SPLIT_ECL_ECLTRG
    int event_nwords_main = 0, event_nwords_splitted = 0;
    if (split_main_use == 1 && split_sub_use == 1) {
      split_Ecltrg(sender_id, buff + NW_SEND_HEADER, valid_ch,
                   buff_main, buff_splitted, event_nwords_main, event_nwords_splitted, ECLTRG_NODE_ID, splitted_ch);
      tot_event_nwords = event_nwords_main + event_nwords_splitted;
    } else if (split_main_use == 1 && split_sub_use == 0) {
      event_nwords_main = tot_event_nwords;
    } else if (split_main_use == 0 && split_sub_use == 1) {
      event_nwords_splitted = tot_event_nwords;
    } else {
      pthread_mutex_lock(&(mtx_sender_log));
      printf("[FATAL] thread %d : %s : No channels are used for this PCIe40 board (ECL/ECLTRG). Please mask this readout PC with runcontrol GUI (or exclude sub-system if this is the only readout PC of the sub-system). Exiting..\n",
             sender_id, hostnamebuf);
      fflush(stdout);
      pthread_mutex_unlock(&(mtx_sender_log));
      exit(1);
    }
#endif // SPLIT_ECL_ECLTRG

    unsigned int prev_exprun = exprun;
    unsigned int prev_evtnum = evtnum;

#ifdef SPLIT_ECL_ECLTRG
    for (int k = 0; k < NUM_SUB_EVE ; k++) {
#endif // SPLIT_ECL_ECLTRG
      unsigned int* eve_buff = NULL;
      unsigned int event_nwords = 0;
      int ret = 0;

#ifdef SPLIT_ECL_ECLTRG
      if (k == 0) {
        if (split_main_use == 0) continue;
        event_nwords = event_nwords_main;
        eve_buff = buff + NW_SEND_HEADER;
        ret = checkEventData(sender_id, eve_buff, event_nwords_main,
                             exprun, evtnum, node_id, valid_main_ch);
      } else if (k == 1) {
        if (split_sub_use == 0) continue;
        exprun = prev_exprun;
        evtnum = prev_evtnum;
        event_nwords = event_nwords_splitted;
        eve_buff = buff + NW_SEND_HEADER + event_nwords_main;
        ret = checkEventData(sender_id, eve_buff, event_nwords_splitted,
                             exprun, evtnum, ECLTRG_NODE_ID, valid_splitted_ch);
      } else {
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[FATAL] thread %d : # of sub-events must be 1 or 2(for ECL,ECLTRG). k = %d  Exiting... : exp %d run %d sub %d : %s %s %d\n",
               sender_id, k,
               (exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
               (exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
               (exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
               __FILE__, __PRETTY_FUNCTION__, __LINE__);
        fflush(stdout);
        exit(1);
        // pthread_mutex_unlock(&(mtx_sender_log)); //TODO can be removed?
      }
#else
      event_nwords = tot_event_nwords;
      eve_buff = buff + NW_SEND_HEADER;
      ret = checkEventData(sender_id, eve_buff, event_nwords, exprun, evtnum, node_id, valid_ch);
#endif

      if (ret != DATACHECK_OK) {
        if (ret == DATACHECK_OK_BUT_ERRFLAG_IN_HDR) {
          //        err_bad_ffaa[sender_id]++;
          unsigned int reduced_event_nwords = 0;
          pthread_mutex_lock(&(mtx_sender_log));
          printf("[WARNING] thread %d : fake-error events are detected. Header and trailer reduction will be made and data are checked again.\n",
                 sender_id);
          fflush(stdout);
          pthread_mutex_unlock(&(mtx_sender_log));
          reduceHdrTrl(eve_buff, reduced_event_nwords);
          tot_event_nwords -= (event_nwords - reduced_event_nwords);
          event_nwords = reduced_event_nwords;

          exprun = prev_exprun;
          evtnum = prev_evtnum;

          int ret = 0;
#ifdef SPLIT_ECL_ECLTRG
          if (k == 0) {
            if (event_nwords_splitted != 0) {
              memcpy(buff_splitted, eve_buff + event_nwords_main, event_nwords_splitted * sizeof(unsigned int));
              memcpy(eve_buff + reduced_event_nwords, buff_splitted, event_nwords_splitted * sizeof(unsigned int));
            }
            event_nwords_main = reduced_event_nwords;
            ret = checkEventData(sender_id, eve_buff, reduced_event_nwords, exprun, evtnum, node_id, valid_main_ch);
          } else {
            event_nwords_splitted = reduced_event_nwords;
            ret = checkEventData(sender_id, eve_buff, reduced_event_nwords, exprun, evtnum, node_id, valid_splitted_ch);
          }
#else
          ret = checkEventData(sender_id, eve_buff, reduced_event_nwords, exprun, evtnum, node_id, valid_ch);
#endif //SPLIT_ECL_ECLTRG

          if (ret != DATACHECK_OK) {
            pthread_mutex_lock(&(mtx_sender_log));
            printf("[FATAL] thread %d : %s : checkEventData() detected an error after reduceHdrTrl(). Exiting...\n", sender_id, hostnamebuf);
            fflush(stdout);
            pthread_mutex_unlock(&(mtx_sender_log));
            exit(1);
          }
          pthread_mutex_lock(&(mtx_sender_log));
          printf("[WARNING] thread %d : %s : Data-check was passed. This event is treated as a normal event.\n", sender_id, hostnamebuf);
          //        printf("[FATAL] thread %d : Currently, we will not tolerate a fake-error event. Exiting...\n", sender_id);
          printEventData(eve_buff, reduced_event_nwords);
          fflush(stdout);
          pthread_mutex_unlock(&(mtx_sender_log));
          //        exit(1);
        } else {
          pthread_mutex_lock(&(mtx_sender_log));
          printf("[FATAL] thread %d : %s : checkEventData() detected an error. Exiting...\n", sender_id, hostnamebuf);
          fflush(stdout);
          pthread_mutex_unlock(&(mtx_sender_log));
          exit(1);
        }
      }

      if (eve_buff[ 1 ]  & 0xfffff000 != 0x7f7f0000 ||
          eve_buff[ event_nwords - 1  ] != 0x7fff0006) {
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[FATAL] thread %d : %s : ERROR_EVENT : Invalid Magic word in header( pos=0x%x, %.8x ) and/or trailer( pos=0x%x, 0x%.8x ) : eve %u exp %d run %d sub %d : %s %s %d\n",
               sender_id, hostnamebuf,
               1, eve_buff[ 1 ],
               event_nwords - 1, eve_buff[ event_nwords - 1 ],
               evtnum,
               (exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
               (exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
               (exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
               __FILE__, __PRETTY_FUNCTION__, __LINE__);
        printEventData(eve_buff, event_nwords, sender_id);
        fflush(stdout);
        pthread_mutex_unlock(&(mtx_sender_log));
        exit(1);
      }
#ifdef SPLIT_ECL_ECLTRG
    }
#endif

    //
    // For TOP feature extraction function
    //



    //
    // Filling SendHeader
    //
    buff[ 0 ] = tot_event_nwords + NW_SEND_HEADER + NW_SEND_TRAILER;
    buff[ 1 ] = 6;
#ifdef SPLIT_ECL_ECLTRG
    if (split_main_use == 1 && split_sub_use == 1) {
      buff[ 2 ] = 0x00010002; // nevent = 1, nboards = 2
    } else {
      buff[ 2 ] = 0x00010001; // nevent = 1, nboards = 1
    }
#else
    buff[ 2 ] = 0x00010001; // nevent = 1, nboards = 1
#endif //SPLIT_ECL_ECLTRG
    buff[ 3 ] = buff[ NW_SEND_HEADER + 2 ];
    buff[ 4 ] = buff[ NW_SEND_HEADER + 3 ];
    buff[ 5 ] = buff[ NW_SEND_HEADER + 6 ];
    //
    // Filling SendTrailer
    //
    buff[ tot_event_nwords + NW_SEND_HEADER ] = 0x0;
    buff[ tot_event_nwords + NW_SEND_HEADER + 1 ] = 0x7fff0007;

#ifndef NOT_SEND
    ret = 0;
    int sent_bytes = 0;
    // pthread_mutex_lock(&(mtx_sender_log));
    // printf("[DEBUG] thread %d : sent words %d + sndhdr %d + sndtrl %d\n", sender_id, tot_event_nwords, NW_SEND_HEADER,  NW_SEND_TRAILER );
    // printEventData( buff, tot_event_nwords + NW_SEND_HEADER + NW_SEND_TRAILER, sender_id);
    // pthread_mutex_unlock(&(mtx_sender_log));

    if ((buff[ NW_SEND_HEADER + 1 ] & 0xfffff000) != 0x7f7f0000 ||
        buff[ NW_SEND_HEADER + tot_event_nwords - 1  ] != 0x7fff0006) {
      pthread_mutex_lock(&(mtx_sender_log));

      printf("[FATAL] thread %d : %s : ERROR_EVENT : Invalid Magic word in the 1st header( pos=0x%x, 0x%.8x ) and/or the last trailer( pos=0x%x, 0x%.8x ) : eve %u exp %d run %d sub %d : %s %s %d\n",
             sender_id, hostnamebuf, NW_SEND_HEADER + 1, buff[ NW_SEND_HEADER + 1 ],
             NW_SEND_HEADER + tot_event_nwords - 1, buff[ NW_SEND_HEADER + tot_event_nwords - 1 ],
             evtnum,
             (exprun & Belle2::RawHeader_latest::EXP_MASK) >> Belle2::RawHeader_latest::EXP_SHIFT,
             (exprun & Belle2::RawHeader_latest::RUNNO_MASK) >> Belle2::RawHeader_latest::RUNNO_SHIFT,
             (exprun & Belle2::RawHeader_latest::SUBRUNNO_MASK),
             __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printEventData(buff, tot_event_nwords + NW_SEND_HEADER + NW_SEND_TRAILER, sender_id);
      fflush(stdout);
      pthread_mutex_unlock(&(mtx_sender_log));
      exit(1);
    }

    while (true) {
      if ((ret = write(fd_accept, (char*)buff + sent_bytes, (tot_event_nwords + NW_SEND_HEADER + NW_SEND_TRAILER)
                       * sizeof(unsigned int) - sent_bytes)) <= 0) {
        if (errno == EINTR) {
          continue;
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
          continue;
        } else {
          perror("[DEBuG] write() failed");
          pthread_mutex_lock(&(mtx_sender_log));
          printf("[FATAL] thread %d : %s : write() failed. Return value of write() = %d\n", sender_id, hostnamebuf, ret);
          fflush(stdout);
          pthread_mutex_unlock(&(mtx_sender_log));
          exit(1);
        }
      }
      sent_bytes += ret;
      if (sent_bytes == (int)((tot_event_nwords + NW_SEND_HEADER + NW_SEND_TRAILER)
                              * sizeof(unsigned int))) {
        break;
      } else if (sent_bytes > (int)((tot_event_nwords + NW_SEND_HEADER + NW_SEND_TRAILER)
                                    * sizeof(unsigned int))) {
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[FATAL] thread %d : %s : Too many bytes are sent\n", sender_id, hostnamebuf);
        fflush(stdout);
        pthread_mutex_unlock(&(mtx_sender_log));
        exit(1);
      }
    }
#endif
#ifndef USE_ZMQ
    if (buffer_id == 0) {
      buffer_id = 1;
    } else {
      buffer_id = 0;
    }
#endif
    cnt++;

    if (cnt == start_cnt) init_time = getTimeSec();
    if (cnt % 1000000 == 1) {
      if (cnt > start_cnt) {
        double cur_time = getTimeSec();
        pthread_mutex_lock(&(mtx_sender_log));
        printf("[INFO] thread %d : evt %llu time %.1lf dataflow %.1lf MB/s rate %.2lf kHz : so far dataflow %.1lf MB/s rate %.2lf kHz size %d\n",
               sender_id,
               cnt, cur_time - init_time,
               NUM_CLIENTS_PER_THREAD * (cnt - prev_cnt)*total_words * sizeof(int) / 1000000. / (cur_time - prev_time),
               (cnt - prev_cnt) / (cur_time - prev_time) / 1000.,
               NUM_CLIENTS_PER_THREAD * (cnt - start_cnt)*total_words * sizeof(int) / 1000000. / (cur_time - init_time),
               (cnt - start_cnt) / (cur_time - init_time) / 1000., total_words);

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
  if (pcie40_node_id != NODE_ID_TEST_BENCH) {
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
      printf("[FATAL] This sever's hostname is not for a PCIe40 ROPC( %s ). Please use 0x%.8x for a test. Exiting...\n", hostnamebuf,
             NODE_ID_TEST_BENCH);
      pthread_mutex_unlock(&(mtx_sender_log));
      exit(1);
    }

  }
  fflush(stdout);

#ifdef USE_ZMQ
  ///////////////////////////////////////////////
  // ZMQ initialize
  ///////////////////////////////////////////////
  zmq::context_t ctx(0);
  const pid_t pid = getpid();
  for (int i = 0; i < NUM_SENDER_THREADS; i++) {
    zmq_writer[i] = new zmq::socket_t(ctx, ZMQ_PAIR);
    zmq_reader[i] = new zmq::socket_t(ctx, ZMQ_PAIR);
    char zpath[256];
    snprintf(zpath, sizeof(zpath), "inproc:///dev/shm/des_ser_PCIe40_main.%d.%d.ipc", pid, i);
    zmq_writer[i]->bind(zpath);
    zmq_reader[i]->connect(zpath);
  }
#else
  ///////////////////////////////////////////////
  // buffer for inter-threads communication
  ///////////////////////////////////////////////
  for (int i = 0; i < NUM_SENDER_THREADS; i++) {
    data_1[i] = new unsigned int[MAX_EVENT_WORDS];
    data_2[i] = new unsigned int[MAX_EVENT_WORDS];
  }
#endif

  ///////////////////////////////////////////////
  // Initialize variables
  ///////////////////////////////////////////////
  double init_time = getTimeSec();
  double prev_time = init_time;
  unsigned long long int cnt = 0;
  unsigned long long int prev_cnt = 0;
  unsigned long long int start_cnt = 300000;
#ifndef USE_ZMQ
  int buffer_id[NUM_SENDER_THREADS];
#endif
  int total_words = 0;
#ifndef USE_ZMQ
  for (int i = 0; i < NUM_SENDER_THREADS; i++) {
    buffer_id[i] = 0;
    buffer_filled[i][0] = 0;
    buffer_filled[i][1] = 0;
    copy_nwords[i][0] = 0;
    copy_nwords[i][1] = 0;
  }
#endif


  ///////////////////////////////////////////////
  // Initialize PCIe40
  ///////////////////////////////////////////////
  printf("[DEBUG] Initializing PCIe40 readout...\n");  fflush(stdout);

  std::ofstream the_file ;
  if (writeInFile) the_file.open("data_file.txt") ;
  double triggerRate = 400 ; // kHz
  double data_size = 0. ;
  int size = 0x1F ;
  int res = ecs_open(0, 0) ;

  if (-1 == res) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("ERROR: Could not open device (BAR 0)\n") ;
    pthread_mutex_unlock(&(mtx_sender_log));
  } else {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("SUCCESS: Device opened for ECS 0\n");
    pthread_mutex_unlock(&(mtx_sender_log));
  }

  res = ecs_open(0, 2) ;
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
  unsigned int masks0 = ecs_read(0, 2, 0x50520) ;
  unsigned int masks1 = ecs_read(0, 2, 0x50540) ;
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
  if (num_of_chs <= 0) {
    pthread_mutex_lock(&(mtx_sender_log));
    printf("[FATAL] %s : No channels are used for this PCIe40 board. Please mask this readout PC with runcontrol GUI (or exclude sub-system if this is the only readout PC of the sub-system). Exiting..\n",
           hostnamebuf);
    fflush(stdout);
    pthread_mutex_unlock(&(mtx_sender_log));
    exit(1);
  }

  // initialize sum of error counters;
  for (int i = 0; i < NUM_SENDER_THREADS; i++) {
    for (int j = 0; j <  MAX_PCIE40_CH; j++) {
      crc_err_ch[i][j] = 0;
    }
  }

  //
  // ulreset to clear FIFOs and rest state machines in userlogic
  //
  ecs_write(0, 2,  0x00050400, 0x0);
  ecs_write(0, 2,  0x00050400, 0x4);
  ecs_write(0, 2,  0x00050400, 0x0);

  unsigned int initial_value = pcie40_getNbWordInEvent(0) ;
  if (initial_value == 0) pcie40_setNbWordInEvent(0, 0xFF) ;
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
  pcie40_setGeneratorTriggerFrequency(0, t_rate) ;
  pcie40_setNbEvents(0, 0) ;
  // start DAQ
  pcie40_resetSPages(0) ;
  pcie40_dmaSetReadout(0) ;
  pcie40_setSizeFromHeader(0) ;
  if (! isData)
    pcie40_setNbWordInEvent(0, size) ;
  pcie40_setBusyLevel(0, 0x502) ;
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
  unsigned int exprun = 0;
  unsigned int prev_exprun = 0;
  int errors = 0 ;
  unsigned int esize = 0 ;
  int total_pages = 0 ;
  int index_pages = 0 ;
  int previous_index = 0 ;
  unsigned int frag_size = 0 ;
  //  auto t1 = std::chrono::high_resolution_clock::now();
  double m_start_time = getTimeSec();
  double m_prev_time = 0.;
  double total_size_bytes = 0.;
  double prev_total_size_bytes = 0.;
  double total_eve_cnt = 0.;
  double prev_total_eve_cnt = 0.;
  int first_flag = 0;
  int first_event_flag = 0;
  unsigned int evecnt = 0;
  unsigned int prev_evecnt = 0;
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
        unsigned int event_words = 0;
        data = pcie40_getSuperPageCopy(0, (get_sp_cnt / S_PAGE_SLOT_NMB) % S_PAGES, get_sp_cnt % S_PAGE_SLOT_NMB) ;
        if (! isData) {
          checkEventGenerator(data, get_sp_cnt, size);
        } else {
          // Check DMA header and trailer
          int ret = checkDMAHeader(data, frag_size, data_size, total_pages, index_pages) ;

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
              memcpy(&combined_data[ previous_index ], data + DMA_HDR_WORDS, 8 * (frag_size - 2) * 4) ;
              delete [] data ;
              data = NULL;
              previous_index = previous_index + 8 * (frag_size - 2) ;

              // Get more DMA packets to complete an event
              if (index_pages != (total_pages - 1)) {
                get_sp_cnt++;
                if ((get_sp_cnt > 0) && ((get_sp_cnt % S_PAGE_SLOT_NMB) == 0)) pcie40_freeSuperPage(0, 1) ;
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
            // if (exit_on_error) exit(0) ; //TODO can be removed?
            // errors++ ;                   //TODO can be removed?
          }

          //
          // End of an event
          //
          {
            if (combined_data != NULL) {
              //              if (k < 10)printFullData(combined_data + dma_hdr_offset);
              event_words = combined_data[ dma_hdr_offset + EVENT_LEN_POS ];
              if (event_words < 32000) {
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
        if (event_words > 0 && event_words < MAX_EVENT_WORDS) {

          //
          // Check event # incrementation
          //
          unsigned int* temp_data = combined_data + dma_hdr_offset;
          if (evtnum + 1  != temp_data[EVENUM_POS]) {
            if (exprun == temp_data[RUNNO_POS]
                && exprun != 0) { // After a run-change or if this is the 1st event, event incrementation is not checked.
              printEventNumberError(temp_data, evtnum, exprun, 1, -1);
#ifndef NO_ERROR_STOP
              exit(1);
#endif
            }
          }

          if (exprun != prev_exprun || exprun == 0) {
            m_start_time = getTimeSec();
            m_prev_time = m_start_time;
            evecnt = 1;
            prev_evecnt = 1;
            total_eve_cnt = 1;
            prev_total_eve_cnt = 0;
            total_size_bytes = 0.;
            prev_total_size_bytes = 0.;
          }
          evtnum = temp_data[EVENUM_POS];
          prev_exprun = exprun;
          exprun = temp_data[RUNNO_POS];

          //
          // Copy data to buffer
          //
          client_id = client_id % NUM_SENDER_THREADS;
#ifdef USE_ZMQ
          // by ZMQ (experimental)
          zmq_writer[client_id]->send(combined_data + dma_hdr_offset, event_words * sizeof(unsigned int));
#else
          // by double buffer (original)
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
#endif
          client_id++;
        } else {
          pthread_mutex_lock(&(mtx_sender_log));
          printf("[FATAL] Invalid event-size %u\n", event_words);
          fflush(stdout);
          pthread_mutex_unlock(&(mtx_sender_log));
          exit(1);
        }

        //
        // Error-count monitor
        //
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
            ((evecnt % 10000) == 0 && 0 < evecnt && evecnt < 100000) ||
            evecnt == 1
           ) {
          unsigned int sum_total_crc_good = 0;
          unsigned int sum_total_crc_errors = 0;
          unsigned int sum_err_not_reduced = 0;
          unsigned int sum_err_bad_7f7f = 0;
          unsigned int sum_err_bad_runnum = 0;
          unsigned int sum_err_bad_linknum = 0;
          unsigned int sum_err_bad_evenum = 0;
          unsigned int sum_err_bad_ffaa = 0;
          unsigned int sum_err_bad_ff55 = 0;
          unsigned int sum_err_bad_linksize = 0;
          unsigned int sum_err_link_eve_jump = 0;

          if (evecnt != 1) {
            unsigned int sum_err_flag_cnt = 0;
            unsigned int sum_cur_evtnum = 0;
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

              // if (cur_exprun[0] != cur_exprun[l]) {
              //   pthread_mutex_lock(&(mtx_sender_log));
              //   printf("[FATAL] exprun mismatch thr 0 = 0x%.8x , thr %d = 0x%.8x", cur_exprun[0], l, cur_exprun[l]);
              //   pthread_mutex_unlock(&(mtx_sender_log));
              //   exit(1);
              // }

              for (int m = 0; m <  MAX_PCIE40_CH; m++) {
                sum_crc_err_ch[m] += crc_err_ch[l][m];
              }
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

          printf("[DEBUG] Event %12u Rate %6.2lf[kHz] Recvd %6.2lf[MB/s] RunTime %8.2lf[s] interval %8.4lf[s] evenum %12u exprun 0x%.8x eve_size %6.2lf[kB] numch %d nonred %u crcok %u crcng %u evejump %u bad_7f7f %u bad_runnum %u bad_linknum %u bad_evenum %u bad_ffaa %u bad_ff55 %u bad_linksize %u no_data %u bad_header %u bad_size %u bad_size_dmatrl %u bad_dmatrl %u bad_word_size %u %s",
                 evecnt, (evecnt  - prev_evecnt) / interval / 1.e3,
                 (total_size_bytes - prev_total_size_bytes) / interval / 1.e6,
                 total_time,                 interval,
                 evtnum, exprun,
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
          prev_evecnt = evecnt;
          prev_total_eve_cnt = total_eve_cnt;
        }

        if ((k % 100) == 0)
          if (writeInFile) writeToFile(the_file, data, esize) ;
        if ((get_sp_cnt > 0) && ((get_sp_cnt % S_PAGE_SLOT_NMB) == 0)) pcie40_freeSuperPage(0, 1) ;
      }
    }

    //
    // Rate Monitor
    //
    cnt++;
    if (cnt == start_cnt) init_time = getTimeSec();
    if (cnt % 10000 == 1) {
      if (cnt > start_cnt) {
        double cur_time = getTimeSec();
        pthread_mutex_lock(&(mtx_sender_log));
        printf("run %d evt %llu time %.1lf dataflow %.1lf MB/s rate %.2lf kHz : so far dataflow %.1lf MB/s rate %.2lf kHz size %d\n",
               run_no,
               cnt,
               cur_time - init_time,
               NUM_SENDER_THREADS * (cnt - prev_cnt)*total_words * sizeof(int) / 1000000. / (cur_time - prev_time),
               (cnt - prev_cnt) / (cur_time - prev_time) / 1000.,
               NUM_SENDER_THREADS * (cnt - start_cnt)*total_words * sizeof(int) / 1000000. / (cur_time - init_time),
               (cnt - start_cnt) / (cur_time - init_time) / 1000., total_words);
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
  ecs_close(0, 0) ;
  ecs_close(0, 2) ;
  dma_close(0) ;
  if (writeInFile) the_file.close() ;

  //
  // Close threads and delete buffers
  //
  for (int i = 0; i < NUM_SENDER_THREADS; i++) {
    pthread_join(sender_thr[i], NULL);
#ifndef USE_ZMQ
    pthread_mutex_destroy(&(mtx1_ch[i]));
    pthread_mutex_destroy(&(mtx2_ch[i]));
    delete data_1[i];
    delete data_2[i];
#endif
  }
  pthread_mutex_destroy(&mtx_sender_log);
  return 0;
}
