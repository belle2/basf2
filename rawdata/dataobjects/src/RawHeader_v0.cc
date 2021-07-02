/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : RawHeader_v0.cc
// Description : Module to handle RawHeader_v0 attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include <rawdata/dataobjects/RawHeader_v0.h>



using namespace Belle2;

//ClassImp(RawHeader_v0);

RawHeader_v0::RawHeader_v0()
{
  m_buffer = NULL;
  //  initialize();
  //  cout << "RawHeader_v0 NULL constructor" << endl;
}

RawHeader_v0::RawHeader_v0(int* buffer)
{
  m_buffer = buffer;
}

RawHeader_v0::~RawHeader_v0()
{
}


int RawHeader_v0::AddNodeInfo(int node_id)
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

int RawHeader_v0::GetNodeInfo(int node_no, int* node_id)
{
  CheckGetBuffer();
  if (node_no >= GetNumNodes()) {
    return -1;
  }
  *node_id = m_buffer[ POS_NODES_1 + node_no ];

  return 0;
}
