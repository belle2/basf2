//+
// File : RawCOPPER.cc
// Description : Module to handle raw data from COPPER
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

int* RawCOPPER::GetRawHdrBufPtr()
{
  if (m_buffer == NULL || m_nwords <= 0) {
    printf("RawCOPPER buffer is not available.\n");
    exit(1);
  }
  return &(m_buffer[ 0 ]);
}

int* RawCOPPER::GetRawTrlBufPtr()
{
  RawTrailer trl;
  if (m_buffer == NULL || (m_nwords - trl.GetTrlNwords() < 0)) {
    printf("RawCOPPER buffer is not available.\n");
    exit(1);
  }
  return &(m_buffer[ m_nwords - trl.GetTrlNwords() ]);
}



int RawCOPPER::Size()
{
  return m_nwords;
}

int RawCOPPER::GetBodyNwords()
{
  return m_nwords;
}

// int* RawCOPPER::AllocateBuffer(int nwords)
// {
//   m_nwords = nwords;
//   if (!m_use_prealloc_buf && m_buffer != NULL) delete[] m_buffer;
//   m_buffer = new int[nwords];
//   m_use_prealloc_buf = false;
//   return m_buffer;
// }

int* RawCOPPER::GetBuffer()
{
  return m_buffer;
}


void RawCOPPER::SetBuffer(int* bufin, int nwords, int malloc_flag)
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

  //
  // Assign header and trailer
  //
//   m_header.SetBuffer(&(bufin[ 0 ]));
//   m_trailer.SetBuffer(&(bufin[ m_nwords - m_trailer.GetTrlNwords() ]));

}



int RawCOPPER::GetCOPPERNodeId()
{
  return 0;
}

unsigned int RawCOPPER::GetCOPPEREveNo()
{
  return m_buffer[ POS_EVE_NUM_COPPER + tmp_header.RAWHEADER_NWORDS ];
}

unsigned int RawCOPPER::GetB2LFEEHdr1()
{
  return m_buffer[ 0 + SIZE_B2LHSLB_HEADER + POS_EVE_NUM_COPPER + tmp_header.RAWHEADER_NWORDS ];
}

unsigned int RawCOPPER::GetB2LFEEHdr2()
{
  return m_buffer[ 1 + SIZE_B2LHSLB_HEADER + POS_EVE_NUM_COPPER + tmp_header.RAWHEADER_NWORDS ];
}

int RawCOPPER::GetSubsysId()
{
  unsigned int subsys = m_buffer[ POS_SUBSYSTEM_ID + tmp_header.RAWHEADER_NWORDS ];
  unsigned int crate = m_buffer[ POS_CRATE_ID + tmp_header.RAWHEADER_NWORDS ];
  unsigned int slot  = m_buffer[ POS_SLOT_ID + tmp_header.RAWHEADER_NWORDS ];

  return
    ((subsys << 16) & 0xFFFF0000) |
    ((crate << 8) & 0x0000FF00) |
    (slot & 0x000000FF);
}

int RawCOPPER::GetNumFINNESSEBlock()
{
  int cnt = 0;
  if (m_buffer[ POS_CH_A_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ] > 0) cnt++;
  if (m_buffer[ POS_CH_B_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ] > 0) cnt++;
  if (m_buffer[ POS_CH_C_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ] > 0) cnt++;
  if (m_buffer[ POS_CH_D_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ] > 0) cnt++;
  return cnt;
}



int RawCOPPER::GetOffset1stFINNESSE()
{
  return
    tmp_header.RAWHEADER_NWORDS
    + SIZE_COPPER_HEADER;
  //  tmp_header.RAWHEADER_NWORDS : RawCOPPER header length
  //  SIZE_COPPER_HEADER : COPPER header length

}

int RawCOPPER::GetOffset2ndFINNESSE()
{
  return
    GetOffset1stFINNESSE()
    + m_buffer[ POS_CH_B_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ];
}

int RawCOPPER::GetOffset3rdFINNESSE()
{
  return
    GetOffset2ndFINNESSE()
    + m_buffer[ POS_CH_C_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ];
}

int RawCOPPER::GetOffset4thFINNESSE()
{
  return
    GetOffset3rdFINNESSE()
    + m_buffer[ POS_CH_D_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ];
}



int* RawCOPPER::Get1stFINNESSEBuffer()
{
  RawHeader rawhdr;
  rawhdr.SetBuffer(GetRawHdrBufPtr());
  if (rawhdr.GetOffset1stFINNESSE() != GetOffset1stFINNESSE()) {
    printf("Data position info is inconsistent. Exting...\n");
    exit(1);
  }
  if (rawhdr.GetOffset1stFINNESSE() > m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ rawhdr.GetOffset1stFINNESSE() ]);
}

int* RawCOPPER::Get2ndFINNESSEBuffer()
{
  RawHeader rawhdr;
  rawhdr.SetBuffer(GetRawHdrBufPtr());
  if (rawhdr.GetOffset2ndFINNESSE() != GetOffset2ndFINNESSE()) {
    printf("Data position info is inconsistent. Exting...\n");
    exit(1);
  }
  if (rawhdr.GetOffset2ndFINNESSE() > m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ rawhdr.GetOffset2ndFINNESSE() ]);

}

int* RawCOPPER::Get3rdFINNESSEBuffer()
{
  RawHeader rawhdr;
  rawhdr.SetBuffer(GetRawHdrBufPtr());
  if (rawhdr.GetOffset3rdFINNESSE() != GetOffset3rdFINNESSE()) {
    printf("Data position info is inconsistent. Exting...\n");
    exit(1);
  }
  if (rawhdr.GetOffset3rdFINNESSE() > m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ rawhdr.GetOffset3rdFINNESSE() ]);

}

int* RawCOPPER::Get4thFINNESSEBuffer()
{
  RawHeader rawhdr;
  rawhdr.SetBuffer(GetRawHdrBufPtr());
  if (rawhdr.GetOffset4thFINNESSE() != GetOffset4thFINNESSE()) {
    printf("Data position info is inconsistent. Exting...\n");
    exit(1);
  }
  if (rawhdr.GetOffset4thFINNESSE() > m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ rawhdr.GetOffset4thFINNESSE() ]);

}


int* RawCOPPER::Get1stDetectorBuffer()
{
  RawHeader rawhdr;
  rawhdr.SetBuffer(GetRawHdrBufPtr());
  return &(m_buffer[ rawhdr.GetOffset1stFINNESSE() + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER ]);
}

int* RawCOPPER::Get2ndDetectorBuffer()
{
  RawHeader rawhdr;
  rawhdr.SetBuffer(GetRawHdrBufPtr());
  return &(m_buffer[ rawhdr.GetOffset2ndFINNESSE() + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER ]);
}

int* RawCOPPER::Get3rdDetectorBuffer()
{
  RawHeader rawhdr;
  rawhdr.SetBuffer(GetRawHdrBufPtr());
  return &(m_buffer[ rawhdr.GetOffset3rdFINNESSE() + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER ]);
}

int* RawCOPPER::Get4thDetectorBuffer()
{
  RawHeader rawhdr;
  rawhdr.SetBuffer(GetRawHdrBufPtr());
  return &(m_buffer[ rawhdr.GetOffset4thFINNESSE() + SIZE_B2LHSLB_HEADER + SIZE_B2LFEE_HEADER ]);
}
