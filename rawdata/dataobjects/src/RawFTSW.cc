//+
// File : RawFTSW.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawFTSW.h>
//#include <daq/rawdata/modules/DAQConsts.h>
//#define WO_FIRST_EVENUM_CHECK
#define NO_DATA_CHECK

using namespace std;
using namespace Belle2;

ClassImp(RawFTSW);

RawFTSW::RawFTSW()
{
}

RawFTSW::~RawFTSW()
{
}


int RawFTSW::Get15bitTLUTag(int n)
{
  return (int)((unsigned int)(m_buffer[ GetBufferPos(n) +  POS_FTSW_4 ]) & 0x00007FFF);
}

unsigned int RawFTSW::GetTTCtimeTRGType(int n)
{
  return (unsigned int)(m_buffer[ GetBufferPos(n) +  POS_TT_CTIME_TRGTYPE ]);
}


int RawFTSW::GetTTCtime(int n)
{
  return (int)((GetTTCtimeTRGType(n) & TTCTIME_MASK) >> TTCTIME_SHIFT);
}

unsigned int RawFTSW::GetTTUtime(int n)
{
  return (unsigned int)(m_buffer[ GetBufferPos(n) +  POS_TT_UTIME ]);
}

void RawFTSW::GetTTTimeVal(int n, struct timeval* tv)
{
  tv->tv_sec = GetTTUtime(n);
  tv->tv_usec = (int)(((double)GetTTCtime(n)) / 127.216);

  return ;
}


int RawFTSW::GetNwords(int n)
{
  return  m_buffer[ GetBufferPos(n) +  POS_NWORDS ];
}

int RawFTSW::GetNwordsHeader(int n)
{
  return  m_buffer[ GetBufferPos(n) +  POS_HDR_NWORDS ];
}


int RawFTSW::GetFTSWNodeID(int n)
{
  return  m_buffer[ GetBufferPos(n) +  POS_NODE_ID ];
}

unsigned int RawFTSW::GetEveNo(int n)
{
  return m_buffer[  GetBufferPos(n) + POS_EVE_NO ];
}

unsigned int RawFTSW::GetMagicTrailer(int n)
{
  return m_buffer[  GetBufferPos(n) + POS_MAGIC_1 ];
}

void RawFTSW::CheckData(int n,
                        unsigned int prev_evenum, unsigned int* cur_evenum,
                        int prev_runsubrun_no, int* cur_runsubrun_no)
{
  int err_flag = 0;
  char err_buf[500];
  *cur_evenum = GetEveNo(n);
  *cur_runsubrun_no = GetRunNoSubRunNo(n);

//     printf("[DEBUG] ========== dump a data block : block # %d==========\n", n);
//     for (int k = 0 ; k < GetBlockNwords(n); k++) {
//       printf("0x%.8x ", (GetBuffer(n))[k]);
//       if (k % 10 == 9) printf("\n[DEBUG] ");
//     }
//     fflush(stderr);

#ifndef NO_DATA_CHECK
#ifdef WO_FIRST_EVENUM_CHECK
  if (prev_evenum != 0xFFFFFFFF && *cur_evenum != 0) {
#else
  if (prev_runsubrun_no == *cur_runsubrun_no && prev_runsubrun_no >= 0) {
#endif
    if ((unsigned int)(prev_evenum + 1) != *cur_evenum) {
      sprintf(err_buf, "CORRUPTED DATA: Event # jump : i %d prev 0x%x cur 0x%x : Exiting...\n %s %s %d\n",
              n, prev_evenum, *cur_evenum, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      err_flag = 1;
    }
  }
#endif

  if (GetNwords(n) != SIZE_FTSW_PACKET) {
    sprintf(err_buf, "CORRUPTED DATA: invalid FTSW packet length : block %d nwords %d must be %d : Exiting...\n %s %s %d\n",
            n, GetNwords(n), SIZE_FTSW_PACKET, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }

  if (GetMagicTrailer(n) != FTSW_MAGIC_TRAILER) {
    sprintf(err_buf, "CORRUPTED DATA: invalid magic word : block %d magic word 0x%x must be 0x%x : Exiting...\n %s %s %d\n",
            n, GetMagicTrailer(n), FTSW_MAGIC_TRAILER, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }



  if (err_flag == 1) {
    printf("[DEBUG] ========== dump a data block : block # %d==========\n", n);
    printf("[DEBUG] ");
    for (int k = 0 ; k < GetBlockNwords(n); k++) {
      printf("0x%.8x ", (GetBuffer(n))[k]);
      if (k % 10 == 9) printf("\n[DEBUG] ");
    }
    fflush(stderr);
    string err_str = err_buf; throw (err_str);
    sleep(1234567);
    exit(-1);
  }

  return;
}
