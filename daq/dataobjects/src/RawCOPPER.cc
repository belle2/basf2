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

int RawCOPPER::GetNumB2lBlock()
{
  int cnt = 0;
  if (m_buffer[ POS_CH_A_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ] > 0) cnt++;
  if (m_buffer[ POS_CH_B_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ] > 0) cnt++;
  if (m_buffer[ POS_CH_C_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ] > 0) cnt++;
  if (m_buffer[ POS_CH_D_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ] > 0) cnt++;
  return cnt;
}



int RawCOPPER::GetOffset1stB2l()
{
  return
    tmp_header.RAWHEADER_NWORDS
    + SIZE_COPPER_HEADER;
}

int RawCOPPER::GetOffset2ndB2l()
{
  return
    GetOffset1stB2l()
    + m_buffer[ POS_CH_B_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ];
}

int RawCOPPER::GetOffset3rdB2l()
{
  return
    GetOffset2ndB2l()
    + m_buffer[ POS_CH_C_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ];
}

int RawCOPPER::GetOffset4thB2l()
{
  return
    GetOffset3rdB2l()
    + m_buffer[ POS_CH_D_DATA_LENGTH + tmp_header.RAWHEADER_NWORDS ];
}



int* RawCOPPER::Get1stFEEBuffer()
{
  RawHeader rawhdr;
  rawhdr.SetBuffer(GetRawHdrBufPtr());
  if (rawhdr.GetOffset1stB2l() != GetOffset1stB2l()) {
    printf("Data position info is inconsistent. Exting...\n");
    exit(1);
  }
  if (rawhdr.GetOffset1stB2l() > m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ rawhdr.GetOffset1stB2l() ]);
}

int* RawCOPPER::Get2ndFEEBuffer()
{
  RawHeader rawhdr;
  rawhdr.SetBuffer(GetRawHdrBufPtr());
  if (rawhdr.GetOffset2ndB2l() != GetOffset2ndB2l()) {
    printf("Data position info is inconsistent. Exting...\n");
    exit(1);
  }
  if (rawhdr.GetOffset2ndB2l() > m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ rawhdr.GetOffset2ndB2l() ]);

}

int* RawCOPPER::Get3rdFEEBuffer()
{
  RawHeader rawhdr;
  rawhdr.SetBuffer(GetRawHdrBufPtr());
  if (rawhdr.GetOffset3rdB2l() != GetOffset3rdB2l()) {
    printf("Data position info is inconsistent. Exting...\n");
    exit(1);
  }
  if (rawhdr.GetOffset3rdB2l() > m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ rawhdr.GetOffset3rdB2l() ]);

}

int* RawCOPPER::Get4thFEEBuffer()
{
  RawHeader rawhdr;
  rawhdr.SetBuffer(GetRawHdrBufPtr());
  if (rawhdr.GetOffset4thB2l() != GetOffset4thB2l()) {
    printf("Data position info is inconsistent. Exting...\n");
    exit(1);
  }
  if (rawhdr.GetOffset4thB2l() > m_nwords) {
    printf("Data size is smaller than data position info. Exting...\n");
    exit(1);
  }
  return &(m_buffer[ rawhdr.GetOffset4thB2l() ]);

}
