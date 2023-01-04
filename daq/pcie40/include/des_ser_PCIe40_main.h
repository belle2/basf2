/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef DESSERPCIE40MAIN_H
#define DESSERPCIE40MAIN_H

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

#define USE_ZMQ
#ifdef USE_ZMQ
#include "zmq.hpp"
#endif

/////////////////////////////////////////
// Parameter for operation
////////////////////////////////////////
//#define NOT_SEND
#define NUM_CLIENTS_PER_THREAD 1
#define NUM_SENDER_THREADS 5
//#define NUM_SENDER_THREADS 1
//#define NO_ERROR_STOP

#define NODE_ID_TEST_BENCH 0x99000001
/////////////////////////////////////////
// Parameter for dummy-data
////////////////////////////////////////
//#define DUMMY_REDUCED_DATA

/////////////////////////////////////////
// Parameter for split ECL and ECLTRG data on recl3
////////////////////////////////////////
//#define SPLIT_ECL_ECLTRG
#ifdef SPLIT_ECL_ECLTRG
#define NUM_SUB_EVE 2
#else
#define NUM_SUB_EVE 1
#endif // #ifdef SPLIT_ECL_ECLTRG

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

#ifdef SPLIT_ECL_ECLTRG
#define ECLTRG_NODE_ID 0x13000001
#endif

struct sender_argv {
  int sender_id;
  int run_no;  // For dummy data
  int nwords_per_fee; // For dummy data
  int ncpr; // For dummy data
  int nhslb; // For dummy data
  unsigned int node_id;
  std::vector< int > valid_ch;
};

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
//#define FFAA_POS 0
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


//
// CRC calculation
//
void crc_calc(unsigned int& crc, const unsigned int& data);

unsigned int get_crc(unsigned int* data, int length, unsigned int initial_value);

unsigned short CalcCRC16LittleEndian(unsigned short crc16, const int buf[], int nwords);

//
// Get data information
//
int getEventNumber(unsigned int* data);

int get1stChannel(unsigned int*& data);

//
// Debug print
//
void printHeader(unsigned int* data);

void printTrailer(unsigned int* data);

void printData(unsigned int* data);

void writeToFile(std::ofstream& the_file, unsigned int* data, int size);

void printLine(unsigned int* data, int pos);

void printEventData(unsigned int* data);

void printEventData(unsigned int* data, int size);

void printEventData(unsigned int* data, int size, int sender_id);

void printFullData(unsigned int* data);

void printEventNumberError(unsigned int*& data, const unsigned int evtnum, const unsigned int exprun, const int eve_diff,
                           const int sender_id);

//
// Data check
//
void checkUtimeCtimeTRGType(unsigned int*& data, const int sender_id);

int checkDMAHeader(unsigned int*& data, unsigned int& size, double& dsize, int& total_pages, int& index_pages);

double getTimeSec();

int checkEventData(int sender_id, unsigned int* data, unsigned int event_nwords, unsigned int& exprun,
                   unsigned int& evtnum, unsigned int node_id, std::vector< int > valid_ch);

void checkEventGenerator(unsigned int* data, int i, unsigned int size);

//
// Data manupilation
//
void reduceHdrTrl(unsigned int* data, unsigned int& event_nwords);

int fillDataContents(int* buf, int nwords_per_fee, unsigned int node_id, int ncpr, int nhslb, int run);

void addEvent(int* buf, int nwords_per_fee, unsigned int event, int ncpr, int nhslb);

void split_Ecltrg(int sender_id, unsigned int* data, std::vector< int > valid_ch,
                  unsigned int* data_main, unsigned int* data_splitted,
                  int& event_nwords_main, int& event_nwords_splitted,
                  unsigned int splitted_node_id, std::vector< int > splitted_ch);

void* sender(void* arg);
//int sender_id, int   run_no, int nwords_per_fee, int ncpr, int nhslb, std::vector< int > valid_ch)

#endif //DESSERPCIE40MAIN_H
