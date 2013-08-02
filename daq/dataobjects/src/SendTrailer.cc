//+
// File : SendTrailer.h
// Description : Module to handle SendTrailer attached by COPPER DAQ
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-


#include "daq/dataobjects/SendTrailer.h"

using namespace std;
using namespace Belle2;

ClassImp(SendTrailer);

SendTrailer::SendTrailer()
{
  initialize();
  //  cout << "SendTrailer NULL constructor" << endl;
}

SendTrailer::SendTrailer(unsigned int* buffer)
{
  memcpy(m_trailer, buffer, NUM_TRAILER_WORDS * sizeof(int));
}

SendTrailer::~SendTrailer()
{
}

unsigned int* SendTrailer::trailer()
{
  return m_trailer;
}

void SendTrailer::trailer(unsigned int* bufin)
{
  memcpy(m_trailer, bufin, NUM_TRAILER_WORDS * sizeof(int));
}

void SendTrailer::initialize()
{
  memset(m_trailer, 0, NUM_TRAILER_WORDS * sizeof(int));
  set_magic_word();
}


void SendTrailer::set_chksum(int chksum)
{
  m_trailer[ POS_CHKSUM ] = chksum;
}

void SendTrailer::set_magic_word()
{
  m_trailer[ POS_TERM_WORD ] = MAGIC_WORD_SEND_TRAILER;
}

unsigned int SendTrailer::get_magic_word()
{
  return m_trailer[ POS_TERM_WORD ];
}

int SendTrailer::get_trl_nwords()
{
  return NUM_TRAILER_WORDS;
}
