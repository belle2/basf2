//+
// File : RawTrailer_v0.cc
// Description : Module to handle RawTraeiler attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawTrailer_v0.h>

using namespace std;
using namespace Belle2;

ClassImp(RawTrailer_v0);

RawTrailer_v0::RawTrailer_v0()
{
  m_buffer = NULL;
}

RawTrailer_v0::~RawTrailer_v0()
{
}
