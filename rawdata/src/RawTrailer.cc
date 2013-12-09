//+
// File : RawTrailer.cc
// Description : Module to handle RawTraeiler attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/RawTrailer.h>

using namespace std;
using namespace Belle2;

ClassImp(RawTrailer);

RawTrailer::RawTrailer()
{
  m_buffer = NULL;
}

RawTrailer::~RawTrailer()
{
}

void RawTrailer::CheckBuffer()
{
  if (m_buffer == NULL) {
    perror("m_buffer is NULL. Exiting...");
    exit(1);
  }
}

int* RawTrailer::GetBuffer()
{
  return m_buffer;
}

void RawTrailer::SetBuffer(int* bufin)
{
  m_buffer = bufin;
}

void RawTrailer::Initialize()
{
  SetMagicWord();
}

void RawTrailer::SetChksum(int chksum)
{
  CheckBuffer();
  m_buffer[ POS_CHKSUM ] = chksum;
}

void RawTrailer::SetMagicWord()
{
  CheckBuffer();
  m_buffer[ POS_TERM_WORD ] = MAGIC_WORD_TERM_TRAILER;
}

int RawTrailer::GetMagicWord()
{
  CheckBuffer();
  return m_buffer[ POS_TERM_WORD ];
}

unsigned int RawTrailer::GetChksum()
{
  CheckBuffer();
  return (unsigned int)(m_buffer[ POS_CHKSUM ]);
}

int RawTrailer::GetTrlNwords()
{
  return RAWTRAILER_NWORDS;
}


