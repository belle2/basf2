//+
// File : RawTLU.cc
// Description : Module to handle raw data from TLU(for the telescope beam test)
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Jan - 2014
//-

#include <rawdata/dataobjects/RawTLU.h>
using namespace std;
using namespace Belle2;

RawTLU::RawTLU()
{
  m_access = new RawTLUFormat;
}

RawTLU::~RawTLU()
{
  if (m_access != NULL) delete m_access;
  m_access = NULL;
}
