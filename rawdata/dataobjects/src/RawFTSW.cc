//+
// File : RawFTSW.cc
// Description : Module to handle raw data from COPPER.
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawFTSW.h>
using namespace std;
using namespace Belle2;

RawFTSW::RawFTSW()
{
  m_access = new RawFTSWFormat;
}

RawFTSW::~RawFTSW()
{
  if (m_access != NULL) delete m_access;
  m_access = NULL;
}
