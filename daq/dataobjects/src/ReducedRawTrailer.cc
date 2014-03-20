//+
// File : ReducedRawTrailer.cc
// Description : Module to handle RawTraeiler attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <daq/dataobjects/ReducedRawTrailer.h>

using namespace std;
using namespace Belle2;

ClassImp(ReducedRawTrailer);

ReducedRawTrailer::ReducedRawTrailer()
{
  m_buffer = NULL;
}

ReducedRawTrailer::~ReducedRawTrailer()
{
}
