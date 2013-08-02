//+
// File : RawTrailer.cc
// Description : Module to handle RawTraeiler attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include "daq/dataobjects/RawTrailer.h"

using namespace std;
using namespace Belle2;

ClassImp(RawTrailer);

RawTrailer::RawTrailer()
{
  initialize();
  //  cout << "RawTrailer NULL constructor" << endl;
}

RawTrailer::RawTrailer(unsigned int* buffer)
{
  memcpy(m_trailer, buffer, RAWCOPPER_TRAILER_SIZE * sizeof(int));
}

RawTrailer::~RawTrailer()
{
}

unsigned int* RawTrailer::trailer()
{
  return m_trailer;
}

void RawTrailer::trailer(unsigned int* bufin)
{
  memcpy(m_trailer, bufin, RAWCOPPER_TRAILER_SIZE * sizeof(int));
}

void RawTrailer::initialize()
{
  memset(m_trailer, 0, RAWCOPPER_TRAILER_SIZE * sizeof(int));
  set_magic_word();
}


void RawTrailer::set_chksum(int chksum)
{
  m_trailer[ POS_CHKSUM ] = chksum;
}

void RawTrailer::set_magic_word()
{
  m_trailer[ POS_TERM_WORD ] = MAGIC_WORD_TERM_TRAILER;
}

unsigned int RawTrailer::get_magic_word()
{
  return m_trailer[ POS_TERM_WORD ];
}

int RawTrailer::get_trl_nwords()
{
  return RAWCOPPER_TRAILER_SIZE;
}
