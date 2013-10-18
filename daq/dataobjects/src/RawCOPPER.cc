//+
// File : RawCOPPER.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include "daq/dataobjects/RawCOPPER.h"

using namespace std;
using namespace Belle2;

ClassImp(RawCOPPER);

RawCOPPER::RawCOPPER()
{
}

RawCOPPER::~RawCOPPER()
{
}


int* RawCOPPER::GetRawHdrBufPtr(int n)
{
  int pos_nwords = GetBufferPos(n);
  return &(m_buffer[ pos_nwords ]);
}

int* RawCOPPER::GetRawTrlBufPtr(int n)
{
  int pos_nwords;
  RawTrailer trl;

  if (n == (m_num_events * m_num_nodes) - 1) {
    pos_nwords = m_nwords - trl.GetTrlNwords();
  } else {
    pos_nwords = GetBufferPos(n + 1) - trl.GetTrlNwords();
  }
  return &(m_buffer[ pos_nwords ]);
}


int RawCOPPER::GetBufferPos(int n)
{
  if (m_buffer == NULL || m_nwords <= 0) {
    printf("[ERROR] RawPacket buffer(%p) is not available or length(%d) is not set.\n", m_buffer, m_nwords);
    exit(1);
  }

  if (n >= (m_num_events * m_num_nodes)) {
    printf("Invalid COPPER block No. (%d : max %d ) is specified. Exiting... ", n, (m_num_events * m_num_nodes));
    exit(1);
  }
  int pos_nwords = 0;
  for (int i = 1; i <= n ; i++) {

//     for( int j = 0; j < 127; j++){
//       printf("%.8x ", m_buffer[j]);
//       if(j % 10 == 9) printf("\n");
//     }
//     printf("\n\n");

    int size = tmp_header.RAWHEADER_NWORDS
               + m_buffer[ pos_nwords + tmp_header.RAWHEADER_NWORDS + POS_DATA_LENGTH ]
               + SIZE_COPPER_FRONT_HEADER
               + SIZE_COPPER_TRAILER
               + tmp_trailer.RAWTRAILER_NWORDS;


    pos_nwords +=  size;
    if (pos_nwords >= m_nwords) {
      printf("value of pos_nwords(%d) is larger than m_nwords(%d). Exiting...", pos_nwords, m_nwords);
      exit(1);
    }
  }
  return pos_nwords;

}


int RawCOPPER::GetCOPPERNodeId(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.POS_SUBSYS_ID;
  return m_buffer[ pos_nwords ];
}


int RawCOPPER::GetExpNo(int n)
{
  RawHeader hdr;
  hdr.SetBuffer(GetBuffer(n));
  return hdr.GetExpNo();
}

int RawCOPPER::GetRunNo(int n)
{
  RawHeader hdr;
  hdr.SetBuffer(GetBuffer(n));
  return hdr.GetRunNo();
}

unsigned int RawCOPPER::GetEveNo(int n)
{
  RawHeader hdr;
  hdr.SetBuffer(GetBuffer(n));
  return hdr.GetEveNo();
}


int RawCOPPER::GetSubsysId(int n)
{
  RawHeader hdr;
  hdr.SetBuffer(GetBuffer(n));
  return hdr.GetSubsysId();
}


int RawCOPPER::GetDataType(int n)
{
  RawHeader hdr;
  hdr.SetBuffer(GetBuffer(n));
  return hdr.GetDataType();
}

int RawCOPPER::GetTruncMask(int n)
{
  RawHeader hdr;
  hdr.SetBuffer(GetBuffer(n));
  return hdr.GetTruncMask();
}


unsigned int RawCOPPER::GetCOPPERCounter(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + POS_EVE_NUM_COPPER + hdr.RAWHEADER_NWORDS;
  return (unsigned int)(m_buffer[ pos_nwords ]);
}


int RawCOPPER::GetNumFINNESSEBlock(int n)
{

  RawHeader hdr;
  int cnt = 0;
  int pos_nwords = GetBufferPos(n) + POS_CH_A_DATA_LENGTH + hdr.RAWHEADER_NWORDS;
  if (m_buffer[ pos_nwords ] > 0) cnt++;
  pos_nwords = GetBufferPos(n) + POS_CH_B_DATA_LENGTH + hdr.RAWHEADER_NWORDS;
  if (m_buffer[ POS_CH_B_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ] > 0) cnt++;
  pos_nwords = GetBufferPos(n) + POS_CH_C_DATA_LENGTH + hdr.RAWHEADER_NWORDS;
  if (m_buffer[ POS_CH_C_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ] > 0) cnt++;
  pos_nwords = GetBufferPos(n) + POS_CH_D_DATA_LENGTH + hdr.RAWHEADER_NWORDS;
  if (m_buffer[ POS_CH_D_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ] > 0) cnt++;
  return cnt;
}

int RawCOPPER::Get1stDetectorNwords(int n)
{
  return Get1stFINNESSENwords(n) -  SIZE_B2LHSLB_HEADER -
         SIZE_B2LFEE_HEADER - SIZE_B2LFEE_TRAILER - SIZE_B2LHSLB_TRAILER;
}

int RawCOPPER::Get2ndDetectorNwords(int n)
{
  return Get2ndFINNESSENwords(n) -  SIZE_B2LHSLB_HEADER -
         SIZE_B2LFEE_HEADER - SIZE_B2LFEE_TRAILER - SIZE_B2LHSLB_TRAILER;
}

int RawCOPPER::Get3rdDetectorNwords(int n)
{
  return Get3rdFINNESSENwords(n) -  SIZE_B2LHSLB_HEADER -
         SIZE_B2LFEE_HEADER - SIZE_B2LFEE_TRAILER - SIZE_B2LHSLB_TRAILER;
}

int RawCOPPER::Get4thDetectorNwords(int n)
{
  return Get4thFINNESSENwords(n) -  SIZE_B2LHSLB_HEADER -
         SIZE_B2LFEE_HEADER - SIZE_B2LFEE_TRAILER - SIZE_B2LHSLB_TRAILER;
}


int RawCOPPER::Get1stFINNESSENwords(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_CH_A_DATA_LENGTH;
  return m_buffer[ pos_nwords ];
}

int RawCOPPER::Get2ndFINNESSENwords(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_CH_B_DATA_LENGTH;
  return m_buffer[ pos_nwords ];
}

int RawCOPPER::Get3rdFINNESSENwords(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_CH_C_DATA_LENGTH;
  return m_buffer[ pos_nwords ];
}

int RawCOPPER::Get4thFINNESSENwords(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_CH_D_DATA_LENGTH;
  return m_buffer[ pos_nwords ];
}


int RawCOPPER::GetOffset1stFINNESSE(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + SIZE_COPPER_HEADER;
  return pos_nwords;
}

int RawCOPPER::GetOffset2ndFINNESSE(int n)
{
  return GetOffset1stFINNESSE(n) + Get1stFINNESSENwords(n);
}

int RawCOPPER::GetOffset3rdFINNESSE(int n)
{
  return GetOffset2ndFINNESSE(n) + Get2ndFINNESSENwords(n);
}

int RawCOPPER::GetOffset4thFINNESSE(int n)
{
  return GetOffset3rdFINNESSE(n) + Get3rdFINNESSENwords(n);
}


int* RawCOPPER::Get1stFINNESSEBuffer(int n)
{
  int pos_nwords = GetOffset1stFINNESSE(n);
  if (pos_nwords >= m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ pos_nwords]);
}

int* RawCOPPER::Get2ndFINNESSEBuffer(int n)
{
  int pos_nwords = GetOffset2ndFINNESSE(n);
  if (pos_nwords >= m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ pos_nwords]);
}

int* RawCOPPER::Get3rdFINNESSEBuffer(int n)
{
  int pos_nwords = GetOffset3rdFINNESSE(n);
  if (pos_nwords >= m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ pos_nwords]);
}

int* RawCOPPER::Get4thFINNESSEBuffer(int n)
{
  int pos_nwords = GetOffset4thFINNESSE(n);
  if (pos_nwords >= m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ pos_nwords]);
}


int* RawCOPPER::Get1stDetectorBuffer(int n)
{
  int pos_nwords = GetOffset1stFINNESSE(n) + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER;
  return &(m_buffer[ pos_nwords ]);
}

int* RawCOPPER::Get2ndDetectorBuffer(int n)
{
  int pos_nwords = GetOffset2ndFINNESSE(n) + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER;
  return &(m_buffer[ pos_nwords ]);
}

int* RawCOPPER::Get3rdDetectorBuffer(int n)
{
  int pos_nwords = GetOffset3rdFINNESSE(n) + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER;
  return &(m_buffer[ pos_nwords ]);
}

int* RawCOPPER::Get4thDetectorBuffer(int n)
{
  int pos_nwords = GetOffset4thFINNESSE(n) + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER;
  return &(m_buffer[ pos_nwords ]);
}

int* RawCOPPER::GetExpRunBuf(int n)
{
  int pos_nwords = GetOffset1stFINNESSE(n) + SIZE_B2LHSLB_HEADER + POS_EXP_RUN;
  return &(m_buffer[ pos_nwords ]);
}

int* RawCOPPER::GetFTSW2Words(int n)
{
  int pos_nwords = GetOffset1stFINNESSE(n) + SIZE_B2LHSLB_HEADER + POS_FTSW1;
  return &(m_buffer[ pos_nwords ]);
}

int RawCOPPER::GetFTSW16bitEventNumber(int n)
{
  int pos_nwords = GetOffset1stFINNESSE(n) + SIZE_B2LHSLB_HEADER + POS_FTSW2;
  return (m_buffer[ pos_nwords ] & 0x0000FFFF);
}


unsigned int RawCOPPER::GetMagic7FFF0008(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_MAGIC_COPPER_1;
  return (unsigned int)(m_buffer[ pos_nwords ]);
}

unsigned int RawCOPPER::GetMagicFFFFFAFA(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_MAGIC_COPPER_2;
  return (unsigned int)(m_buffer[ pos_nwords ]);
}

unsigned int RawCOPPER::GetMagic7FFF0009(int n)
{
  RawTrailer trl;
  int pos_nwords = GetBufferPos(n) + GetBlockNwords(n) - trl.GetTrlNwords() - 1;
  return (unsigned int)(m_buffer[ pos_nwords ]);
}


double RawCOPPER::GetEventUnixTime(int n)
{
  int pos = GetOffset1stFINNESSE(n) + SIZE_B2LHSLB_HEADER + POS_EXP_RUN;
  int run = m_buffer[ pos ] & 0x3FFFFF;
//    printf("Time %d %d %lf\n",
//     sp8test_run_starttime[ run ] & 0xFFFF,
//     ( m_buffer[ GetOffset1stFINNESSE(n) + SIZE_B2LHSLB_HEADER + POS_FTSW2 ] >> 16 ) & 0xFFFF,
//     (double)( ( m_buffer[ GetOffset1stFINNESSE(n) + SIZE_B2LHSLB_HEADER + POS_FTSW1 ] >> 4 ) & 0x7FFFFFF )/1.27e8
//     );
  return (double)(sp8test_run_starttime[ run ] & 0xFFFF0000) +
         (double)((m_buffer[ GetOffset1stFINNESSE(n) + SIZE_B2LHSLB_HEADER + POS_FTSW2 ] >> 16) & 0xFFFF) +
         (double)((m_buffer[ GetOffset1stFINNESSE(n) + SIZE_B2LHSLB_HEADER + POS_FTSW1 ] >> 4) & 0x7FFFFFF) / 1.27e8;
}


