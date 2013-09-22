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
  m_nwords = 0;
  m_buffer = NULL;
  m_use_prealloc_buf = false;
}

RawCOPPER::~RawCOPPER()
{
  if (!m_use_prealloc_buf && m_buffer != NULL) {
    delete[] m_buffer;
  }
}


int RawCOPPER::GetBufferPos(int n)
{
  if (m_buffer == NULL || m_nwords <= 0) {
    printf("RawCOPPER buffer is not available.\n");
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


int RawCOPPER::TotalBufNwords()
{
  return m_nwords;
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


int RawCOPPER::GetCprBlockNwords(int n)
{
  int size;
  //  printf("aaaaaaaaaaaaa %d %d\n", n, m_num_events * m_num_nodes );
  if (n == (m_num_events * m_num_nodes) - 1) {
    size =  m_nwords - GetBufferPos(n);
  } else {
    size = GetBufferPos(n + 1) - GetBufferPos(n);
  }
  return size;
}

// int* RawCOPPER::AllocateBuffer(int nwords)
// {
//   m_nwords = nwords;
//   if (!m_use_prealloc_buf && m_buffer != NULL) delete[] m_buffer;
//   m_buffer = new int[nwords];
//   m_use_prealloc_buf = false;
//   return m_buffer;
// }

int* RawCOPPER::GetWholeBuffer()
{
  return m_buffer;
}

int* RawCOPPER::GetBuffer(int n)
{
  int pos_nwords = GetBufferPos(n);
  return &(m_buffer[ pos_nwords ]);
}


void RawCOPPER::SetBuffer(int* bufin, int nwords, int malloc_flag, int num_events, int num_nodes)
{
  if (!m_use_prealloc_buf && m_buffer != NULL) delete[] m_buffer;

  if (malloc_flag == 0) {
    m_use_prealloc_buf = true;
  } else {
    m_use_prealloc_buf = false;
  }
  //  m_nwords = bufin[0];
  m_nwords = nwords;
  m_buffer = bufin;

  m_num_nodes = num_nodes;
  m_num_events = num_events;

  //
  // Assign header and trailer
  //
//   m_header.SetBuffer(&(bufin[ 0 ]));
//   m_trailer.SetBuffer(&(bufin[ m_nwords - m_trailer.GetTrlNwords() ]));

}



int RawCOPPER::GetCOPPERNodeId(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.POS_SUBSYS_ID;
  return m_buffer[ pos_nwords ];
}

unsigned int RawCOPPER::GetCOPPEREveNo(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + POS_EVE_NUM_COPPER + hdr.RAWHEADER_NWORDS;
  return (unsigned int)(m_buffer[ pos_nwords ]);
}

int RawCOPPER::GetSubsysId(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.POS_SUBSYS_ID;
  return m_buffer[ pos_nwords ];

//   unsigned int subsys = m_buffer[ POS_SUBSYSTEM_ID + tmp_header.RAWHEADER_NWORDS ];
//   unsigned int crate = m_buffer[ POS_CRATE_ID + tmp_header.RAWHEADER_NWORDS ];
//   unsigned int slot  = m_buffer[ POS_SLOT_ID + tmp_header.RAWHEADER_NWORDS ];
//   return
//     ((subsys << 16) & 0xFFFF0000) |
//     ((crate << 8) & 0x0000FF00) |
//     (slot & 0x000000FF);
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



int RawCOPPER::GetOffset1stFINNESSE(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + SIZE_COPPER_HEADER;
  return pos_nwords;
}

int RawCOPPER::GetOffset2ndFINNESSE(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_CH_A_DATA_LENGTH;
  return GetOffset1stFINNESSE(n) + m_buffer[ pos_nwords ];
}

int RawCOPPER::GetOffset3rdFINNESSE(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_CH_B_DATA_LENGTH;
  return GetOffset2ndFINNESSE(n) + m_buffer[ pos_nwords ];
}

int RawCOPPER::GetOffset4thFINNESSE(int n)
{
  RawHeader hdr;
  int pos_nwords = GetBufferPos(n) + hdr.RAWHEADER_NWORDS + POS_CH_C_DATA_LENGTH;
  return GetOffset3rdFINNESSE(n) + m_buffer[ pos_nwords ];
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
