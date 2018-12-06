//+
// File : RawFTSWFormat.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-
#include <rawdata/dataobjects/RawFTSWFormat.h>


using namespace std;
using namespace Belle2;

RawFTSWFormat::RawFTSWFormat()
{
}

RawFTSWFormat::~RawFTSWFormat()
{
}


int RawFTSWFormat::Get15bitTLUTag(int n)
{
  return (int)((unsigned int)(m_buffer[ GetBufferPos(n) +  POS_FTSW_4 ]) & 0x00007FFF);
}

unsigned int RawFTSWFormat::GetTTCtimeTRGType(int n)
{
  return (unsigned int)(m_buffer[ GetBufferPos(n) +  POS_TT_CTIME_TRGTYPE ]);
}



int RawFTSWFormat::GetTTCtime(int n)
{
  return (int)((GetTTCtimeTRGType(n) & TTCTIME_MASK) >> TTCTIME_SHIFT);
}

int RawFTSWFormat::GetTRGType(int n)
{
  return (int)(GetTTCtimeTRGType(n) & TRGTYPE_MASK);
}

unsigned int RawFTSWFormat::GetTTUtime(int n)
{
  return (unsigned int)(m_buffer[ GetBufferPos(n) +  POS_TT_UTIME ]);
}

void RawFTSWFormat::GetTTTimeVal(int n, struct timeval* tv)
{
  tv->tv_sec = GetTTUtime(n);
  tv->tv_usec = (int)(((double)GetTTCtime(n)) / 127.216);

  return ;
}

void RawFTSWFormat::GetTTTimeSpec(int n, struct timespec* ts)
{
  ts->tv_sec = GetTTUtime(n);
  ts->tv_nsec = (long)(((double)GetTTCtime(n)) / 0.127216);

  return ;
}

unsigned long long int RawFTSWFormat::GetTTTimeNs(int n)
{
  return (unsigned long long int)GetTTUtime(n) * 1e9 + (long)((double)GetTTCtime(n) / 0.127216);
}

int RawFTSWFormat::GetNwordsHeader(int n)
{
  return  m_buffer[ GetBufferPos(n) +  POS_HDR_NWORDS ];
}


unsigned int RawFTSWFormat::GetFTSWNodeID(int n)
{
  return (unsigned int)(m_buffer[ GetBufferPos(n) +  POS_NODE_ID ]);
}

unsigned int RawFTSWFormat::GetEveNo(int n)
{
  return m_buffer[  GetBufferPos(n) + POS_EVE_NO ];
}

unsigned int RawFTSWFormat::GetMagicTrailer(int n)
{
  return m_buffer[  GetBufferPos(n) + POS_MAGIC_1 ];
}

void RawFTSWFormat::CheckData(int n,
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
    printf("%s", err_buf);
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

  }

  return;
}
