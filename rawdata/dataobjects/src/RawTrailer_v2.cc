//+
// File : RawTrailer_v2.cc
// Description : Module to handle RawTraeiler attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawTrailer_v2.h>

using namespace Belle2;

//ClassImp(RawTrailer_v2);

RawTrailer_v2::RawTrailer_v2()
{
  m_buffer = NULL;
}

RawTrailer_v2::~RawTrailer_v2()
{
}
