//+
// File : ReducedRawHeader.cc
// Description : Module to handle ReducedRawHeader attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <daq/dataobjects/ReducedRawHeader.h>


using namespace std;
using namespace Belle2;

ClassImp(ReducedRawHeader);

ReducedRawHeader::ReducedRawHeader()
{
  m_buffer = NULL;
  //  initialize();
  //  cout << "ReducedRawHeader NULL constructor" << endl;
}

ReducedRawHeader::ReducedRawHeader(int* buffer)
{
  m_buffer = buffer;
}

ReducedRawHeader::~ReducedRawHeader()
{
}


// int ReducedRawHeader::AddNodeInfo(int node_id)
// {
//   CheckSetBuffer();
//   // When the number of total nodes exceeds NUM_MAX_NODES
//   if (GetNumNodes() >= NUM_MAX_NODES) {
//     m_buffer[ POS_NUM_NODES ]++;
//     return -1;
//   }

//   m_buffer[ POS_NODES_1 + m_buffer[ POS_NUM_NODES ] ] = node_id;
//   m_buffer[ POS_NUM_NODES ]++;
//   return 0;
// }

// int ReducedRawHeader::GetNodeInfo(int node_no, int* node_id)
// {
//   CheckGetBuffer();
//   if (node_no >= GetNumNodes()) {
//     return -1;
//   }
//   *node_id = m_buffer[ POS_NODES_1 + node_no ];

//   return 0;
// }
