/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/dataobjects/SndHdrTemp.h"

using namespace std;
using namespace Belle2;

ClassImp(SndHdrTemp);

SndHdrTemp::SndHdrTemp()
{
  Initialize();
  //  cout << "SndHdrTemp NULL constructor" << endl;
}

SndHdrTemp::SndHdrTemp(int* buffer)
{
  //  cout << "SndHdrTemp constructor with buffer" << endl;
  memcpy(m_buffer, buffer, SENDHDR_NWORDS * sizeof(int));
}

SndHdrTemp::~SndHdrTemp()
{
}

int* SndHdrTemp::GetBuffer()
{
  return m_buffer;
}

void SndHdrTemp::SetBuffer(int* bufin)
{
  memcpy(m_buffer, bufin, SENDHDR_NWORDS * sizeof(int));
}

void SndHdrTemp::Initialize()
{
  memset(m_buffer, 0, sizeof(int)* SENDHDR_NWORDS);
  m_buffer[ POS_HDR_NWORDS ] = SENDHDR_NWORDS;
}


// Set Values

void SndHdrTemp::SetNwords(int total_data_nwords)
{
  m_buffer[ POS_NWORDS ] = total_data_nwords;
}

void SndHdrTemp::SetExpRunWord(unsigned int exp_run)
{
  m_buffer[ POS_EXP_RUN_NUM ] = exp_run;
}

void SndHdrTemp::SetNumEventsinPacket(int num_events)
{
  m_buffer[ POS_NUM_EVE_NUM_NODES ] =
    (m_buffer[ POS_NUM_EVE_NUM_NODES ] & 0x0000FFFF) |
    ((num_events << 16) & 0xFFFF0000);
}

void SndHdrTemp::SetNumNodesinPacket(int num_nodes)
{
  m_buffer[ POS_NUM_EVE_NUM_NODES ] =
    (m_buffer[ POS_NUM_EVE_NUM_NODES ] & 0xFFFF0000) |
    (num_nodes & 0x0000FFFF);
}


void SndHdrTemp::SetEventNumber(int eve_num)
{
  m_buffer[ POS_EVE_NUM ] = eve_num;
}


void SndHdrTemp::SetNodeID(int node_id)
{
  m_buffer[ POS_NODE_ID ] = node_id;
}


void SndHdrTemp::SetRunNum(int run_num)
{

  unsigned int inv_mask = ~((unsigned int)(tmp_header.RUNNO_MASK));
  m_buffer[ POS_EXP_RUN_NUM ] =
    ((unsigned int)m_buffer[ POS_EXP_RUN_NUM ] & inv_mask) |
    (((unsigned int)run_num << tmp_header.RUNNO_SHIFT) & tmp_header.RUNNO_MASK);

  return;
}

void SndHdrTemp::SetSubRunNum(int subrun_num)
{

  unsigned int inv_mask = ~((unsigned int)(tmp_header.SUBRUNNO_MASK));
  m_buffer[ POS_EXP_RUN_NUM ] =
    ((unsigned int)m_buffer[ POS_EXP_RUN_NUM ] & inv_mask) | ((unsigned int)subrun_num & tmp_header.SUBRUNNO_MASK);
  return;
}

void SndHdrTemp::SetExpNum(int exp_num)
{

  unsigned int inv_mask = ~((unsigned int)(tmp_header.EXP_MASK));
  m_buffer[ POS_EXP_RUN_NUM ] =
    ((unsigned int)m_buffer[ POS_EXP_RUN_NUM ] & inv_mask) |
    (((unsigned int)exp_num << tmp_header.EXP_SHIFT) & tmp_header.EXP_MASK);

  return;
}



// Get Values

int SndHdrTemp::GetTotalNwords() {  return m_buffer[ POS_NWORDS ];}
int SndHdrTemp::GetHdrNwords() {    return SENDHDR_NWORDS; }
int SndHdrTemp::GetNumEventsinPacket()
{
  return
    ((m_buffer[ POS_NUM_EVE_NUM_NODES ] & 0xFFFF0000) >> 16) & 0x0000FFFF;

}
int SndHdrTemp::GetNumNodesinPacket() {  return m_buffer[ POS_NUM_EVE_NUM_NODES ] & 0x0000FFFF; }

int SndHdrTemp::GetEventNumber() { return m_buffer[ POS_EVE_NUM ]; }

int SndHdrTemp::GetNodeID() { return m_buffer[ POS_NODE_ID ]; }

int SndHdrTemp::GetRunNum()
{
  return (((unsigned int)(m_buffer[ POS_EXP_RUN_NUM ]) & tmp_header.RUNNO_MASK) >> tmp_header.RUNNO_SHIFT);
}

int SndHdrTemp::GetSubRunNum()
{
  return ((unsigned int)(m_buffer[ POS_EXP_RUN_NUM ]) & tmp_header.SUBRUNNO_MASK);
}

int SndHdrTemp::GetRunNumSubRunNum()
{

  return ((unsigned int)(m_buffer[ POS_EXP_RUN_NUM ]) &
          (tmp_header.RUNNO_MASK | tmp_header.SUBRUNNO_MASK));
}


int SndHdrTemp::GetExpNum()
{

  return (((unsigned int)(m_buffer[ POS_EXP_RUN_NUM ]) & tmp_header.EXP_MASK) >> tmp_header.EXP_SHIFT);
}


