//+
// File : rawheader.cc
// Description : pseudo raw data header for test
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 3 - Sep - 2010
//-

#include "daq/dataobjects/RawHeader.h"

using namespace std;
using namespace Belle2;

ClassImp(RawHeader);

RawHeader::RawHeader()
{
  //  cout << "RawHeader NULL constructor" << endl;
}

RawHeader::RawHeader(unsigned int* buffer)
{
  //  cout << "RawHeader constructor with buffer" << endl;
  memcpy(m_header, buffer, HEADER_SIZE * 4);
}

RawHeader::~RawHeader()
{
}

unsigned int* RawHeader::header()
{
  return m_header;
}

void RawHeader::header(unsigned int* bufin)
{
  memcpy(m_header, bufin, HEADER_SIZE * 4);
}
