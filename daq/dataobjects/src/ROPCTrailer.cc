//+
// File : ROPCTrailer.cc
// Description : Module to handle ROPCTraeiler attached on ROPC
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include "daq/dataobjects/ROPCTrailer.h"

using namespace std;
using namespace Belle2;

ClassImp(ROPCTrailer);

ROPCTrailer::ROPCTrailer()
{
  m_buffer = NULL;
}

ROPCTrailer::~ROPCTrailer()
{
}

void ROPCTrailer::CheckBuffer()
{
  if (m_buffer == NULL) {
    perror("m_buffer is NULL. Exiting...");
    exit(1);
  }
}

int* ROPCTrailer::GetBuffer()
{
  return m_buffer;
}

void ROPCTrailer::SetBuffer(int* bufin)
{
  m_buffer = bufin;
}

void ROPCTrailer::Initialize()
{
  SetMagicWord();
}

void ROPCTrailer::SetChksum(int chksum)
{
  CheckBuffer();
  m_buffer[ POS_CHKSUM ] = chksum;
}

void ROPCTrailer::SetMagicWord()
{
  CheckBuffer();
  m_buffer[ POS_TERM_WORD ] = MAGIC_WORD_TERM_TRAILER;
}

int ROPCTrailer::GetMagicWord()
{
  CheckBuffer();
  return m_buffer[ POS_TERM_WORD ];
}

int ROPCTrailer::GetTrlNwords()
{
  return ROPCTRAILER_NWORDS;
}
