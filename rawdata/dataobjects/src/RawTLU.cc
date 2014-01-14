//+
// File : RawTLU.cc
// Description : Module to handle raw data from TLU(for the telescope beam test)
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Jan - 2014
//-

#include <rawdata/dataobjects/RawTLU.h>
using namespace std;
using namespace Belle2;

ClassImp(RawTLU);

RawTLU::RawTLU()
{
}

RawTLU::~RawTLU()
{
}


int RawTLU::GetNwords(int n)
{
  return  m_buffer[ GetBufferPos(n) +  POS_NWORDS ];
}

int RawTLU::GetNwordsHeader(int n)
{
  return  m_buffer[ GetBufferPos(n) +  POS_HDR_NWORDS ];
}


int RawTLU::GetNodeID(int n)
{
  return  m_buffer[ GetBufferPos(n) +  POS_NODE_ID ];
}

unsigned int RawTLU::GetEveNo(int n)
{
  return m_buffer[  GetBufferPos(n) + POS_EVE_NO ];
}


int RawTLU::GetRunNo(int n)
{
  return m_buffer[  GetBufferPos(n) + POS_RUN_NO ];
}


unsigned int RawTLU::GetMagicTrailer(int n)
{
  return m_buffer[  GetBufferPos(n) + POS_MAGIC_1 ];
}

unsigned int RawTLU::GetTLUEventTag(int n)
{
  return m_buffer[  GetBufferPos(n) + POS_TLU_EVENTTAG ];
}


void RawTLU::CheckData(int n,
                       unsigned int prev_evenum, unsigned int* cur_evenum)
{
  int err_flag = 0;
  char err_buf[500];
  *cur_evenum = GetEveNo(n);

#ifndef NO_DATA_CHECK
  if (prev_evenum != 0xFFFFFFFF && *cur_evenum != 0) {
    if ((unsigned int)(prev_evenum + 1) != *cur_evenum) {
      sprintf(err_buf, "Event # jump : i %d prev 0x%x cur 0x%x : Exiting...\n %s %s %d\n",
              n, prev_evenum, *cur_evenum, __FILE__, __PRETTY_FUNCTION__, __LINE__);
      err_flag = 1;
    }
  }
#endif

  if (GetEveNo(n) != GetTLUEventTag(n)) {
    sprintf(err_buf, "invalid TLU event tag : blodk %d header eve # %d TLU tag %d. Exiting...\n %s %s %d\n",
            n, GetEveNo(n), GetTLUEventTag(n), __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }


  if (GetNwords(n) != SIZE_TLU_PACKET) {
    sprintf(err_buf, "invalid TLU packet length : block %d nwords %d must be %d : Exiting...\n %s %s %d\n",
            n, GetNwords(n), SIZE_TLU_PACKET, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }

  if (GetMagicTrailer(n) != TLU_MAGIC_TRAILER) {
    sprintf(err_buf, "invalid magic word : block %d magic word 0x%x must be 0x%x : Exiting...\n %s %s %d\n",
            n, GetMagicTrailer(n), TLU_MAGIC_TRAILER, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    err_flag = 1;
  }

  if (err_flag == 1) {
    printf("========== dump a data block : block # %d==========\n", n);
    for (int k = 0 ; k < GetBlockNwords(n); k++) {
      printf("0x%.8x ", (GetBuffer(n))[k]);
      if (k % 10 == 9)printf("\n");
    }
    fflush(stdout);
    string err_str = err_buf; throw (err_str);
    sleep(1234567);
    exit(-1);
  }

  return;
}
