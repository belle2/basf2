//+
// File : RawHeader.cc
// Description : Module to handle RawHeader attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawHeader.h>


using namespace std;
using namespace Belle2;

ClassImp(RawHeader);

RawHeader::RawHeader()
{
  m_buffer = NULL;
  //  initialize();
  //  cout << "RawHeader NULL constructor" << endl;
}

RawHeader::RawHeader(int* buffer)
{
  m_buffer = buffer;
}

RawHeader::~RawHeader()
{
}


int RawHeader::AddNodeInfo(int node_id)
{
  CheckSetBuffer();
  // When the number of total nodes exceeds NUM_MAX_NODES
  if (GetNumNodes() >= NUM_MAX_NODES) {
    m_buffer[ POS_NUM_NODES ]++;
    return -1;
  }

  m_buffer[ POS_NODES_1 + m_buffer[ POS_NUM_NODES ] ] = node_id;
  m_buffer[ POS_NUM_NODES ]++;
  return 0;
}

int RawHeader::GetNodeInfo(int node_no, int* node_id)
{
  CheckGetBuffer();
  if (node_no >= GetNumNodes()) {
    return -1;
  }
  *node_id = m_buffer[ POS_NODES_1 + node_no ];

  return 0;
}
