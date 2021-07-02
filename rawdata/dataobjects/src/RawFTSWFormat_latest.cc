/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : RawFTSWFormat_latest.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 18 - Feb - 2019
//-
#include <rawdata/dataobjects/RawFTSWFormat_latest.h>


using namespace std;
using namespace Belle2;

RawFTSWFormat_latest::RawFTSWFormat_latest()
{
}

RawFTSWFormat_latest::~RawFTSWFormat_latest()
{
}


unsigned int RawFTSWFormat_latest::GetTTCtimeTRGType(int n)
{
  return (unsigned int)(m_buffer[ GetBufferPos(n) +  POS_TT_CTIME_TRGTYPE ]);
}



int RawFTSWFormat_latest::GetTTCtime(int n)
{
  return (int)((GetTTCtimeTRGType(n) & TTCTIME_MASK) >> TTCTIME_SHIFT);
}

int RawFTSWFormat_latest::GetTRGType(int n)
{
  return (int)(GetTTCtimeTRGType(n) & TRGTYPE_MASK);
}

unsigned int RawFTSWFormat_latest::GetTTUtime(int n)
{
  return (unsigned int)(m_buffer[ GetBufferPos(n) +  POS_TT_UTIME ]);
}

void RawFTSWFormat_latest::GetTTTimeVal(int n, struct timeval* tv)
{
  tv->tv_sec = GetTTUtime(n);
  tv->tv_usec = (int)(((double)GetTTCtime(n)) / 127.216);

  return ;
}

void RawFTSWFormat_latest::GetTTTimeSpec(int n, struct timespec* ts)
{
  ts->tv_sec = GetTTUtime(n);
  ts->tv_nsec = (long)(((double)GetTTCtime(n)) / 0.127216);

  return ;
}

void RawFTSWFormat_latest::GetPCTimeVal(int n, struct timeval* tv)
{
  tv->tv_sec = (unsigned int)(m_buffer[ GetBufferPos(n) +  POS_TVSEC_FROM_PC ]);
  tv->tv_usec = (unsigned int)(m_buffer[ GetBufferPos(n) +  POS_TVUSEC_FROM_PC ]);
  return ;
}


unsigned long long int RawFTSWFormat_latest::GetTTTimeNs(int n)
{
  return (unsigned long long int)GetTTUtime(n) * 1e9 + (long)((double)GetTTCtime(n) / 0.127216);
}

int RawFTSWFormat_latest::GetNwordsHeader(int n)
{
  return  m_buffer[ GetBufferPos(n) +  POS_HDR_NWORDS ];
}


unsigned int RawFTSWFormat_latest::GetFTSWNodeID(int n)
{
  return (unsigned int)(m_buffer[ GetBufferPos(n) +  POS_NODE_ID ]);
}

unsigned int RawFTSWFormat_latest::GetEveNo(int n)
{
  return m_buffer[  GetBufferPos(n) + POS_EVE_NO_1 ];
}

unsigned int RawFTSWFormat_latest::GetMagicTrailer(int n)
{
  return m_buffer[  GetBufferPos(n) + POS_MAGIC_1 ];
}

int RawFTSWFormat_latest::GetIsHER(int n)
{
  int* buffer = GetBuffer(n);
  /* cppcheck-suppress shiftTooManyBitsSigned */
  int ret = (buffer[ POS_INJECTION_INFO ] & INJ_HER_LER_MASK) >> INJ_HER_LER_SHIFT;
  return ret; // 1 -> HER, 0 -> LER
}


unsigned int RawFTSWFormat_latest::GetTimeSinceLastInjection(int n)
{
  unsigned int* buffer = (unsigned int*) GetBuffer(n);
  unsigned int ret = (buffer[ POS_INJECTION_INFO ] & INJ_TIME_MASK) >> INJ_TIME_SHIFT;
  return ret;
}

unsigned int RawFTSWFormat_latest::GetTimeSincePrevTrigger(int n)
{
  unsigned int* buffer = (unsigned int*) GetBuffer(n);
  unsigned int ret = buffer[ POS_TIME_SINCE_PREV_TRG ];
  return ret;
}

unsigned int RawFTSWFormat_latest::GetBunchNumber(int n)
{
  unsigned int* buffer = (unsigned int*) GetBuffer(n);
  unsigned int ret = (buffer[ POS_BUNCH_NUM ] & INJ_BUNCH_MASK) >> INJ_BUNCH_SHIFT;
  return ret;
}

unsigned int RawFTSWFormat_latest::GetFrameCount(int n)
{
  unsigned int* buffer = (unsigned int*) GetBuffer(n);
  unsigned int ret = buffer[ POS_FRAME_COUNT ];
  return ret;
}


void RawFTSWFormat_latest::CheckData(int n,
                                     unsigned int prev_evenum, unsigned int* cur_evenum,
                                     unsigned int prev_exprunsubrun_no, unsigned int* cur_exprunsubrun_no)
{
  int err_flag = 0;
  char err_buf[500];
  *cur_evenum = GetEveNo(n);
  *cur_exprunsubrun_no = GetExpRunSubrun(n);


#ifndef NO_DATA_CHECK
  if (prev_exprunsubrun_no == *cur_exprunsubrun_no) {
    if ((unsigned int)(prev_evenum + 1) != *cur_evenum) {
      sprintf(err_buf, "[FATAL] ERROR_EVENT : Event # jump : i %d prev 0x%x cur 0x%x : Exiting...\n %s %s %d\n",
              n, prev_evenum, *cur_evenum, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      printf("%s", err_buf);
      err_flag = 1;
    }
  }
#endif

  if (GetBlockNwords(n) != SIZE_FTSW_PACKET) {
    sprintf(err_buf, "[FATAL] ERROR_EVENT : invalid FTSW packet length : block %d nwords %d must be %d : Exiting...\n %s %s %d\n",
            n, GetBlockNwords(n), SIZE_FTSW_PACKET, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf);
    err_flag = 1;
  }

  if (GetMagicTrailer(n) != FTSW_MAGIC_TRAILER) {
    sprintf(err_buf, "[FATAL] ERROR_EVENT : invalid magic word : block %d magic word 0x%x must be 0x%x : Exiting...\n %s %s %d\n",
            n, GetMagicTrailer(n), FTSW_MAGIC_TRAILER, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    printf("%s", err_buf); fflush(stdout);
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
    B2FATAL(err_buf);
  }

  return;
}
