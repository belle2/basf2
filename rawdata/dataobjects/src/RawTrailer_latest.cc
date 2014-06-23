//+
// File : RawTrailer_latest.cc
// Description : Module to handle RawTraeiler attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawTrailer_latest.h>

using namespace std;
using namespace Belle2;

ClassImp(RawTrailer_latest);

RawTrailer_latest::RawTrailer_latest()
{
  m_buffer = NULL;
}

RawTrailer_latest::~RawTrailer_latest()
{
}
