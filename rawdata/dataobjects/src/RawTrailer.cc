//+
// File : RawTrailer.cc
// Description : Module to handle RawTraeiler attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawTrailer.h>

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
