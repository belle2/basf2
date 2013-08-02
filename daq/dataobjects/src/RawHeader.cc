//+
// File : RawHeader.cc
// Description : Module to handle RawHeader attached to raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include "daq/dataobjects/RawHeader.h"

using namespace std;
using namespace Belle2;

ClassImp(RawHeader);

RawHeader::RawHeader()
{
  initialize();
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

void RawHeader::initialize()
{
  memset(m_header, 0, sizeof(int)*HEADER_SIZE);
  m_header[ POS_HDR_NWORDS ] = MIN_POS_TERM_HEADER + 1;
  m_header[ POS_NUM_NODES ] = 0;
  m_header[ POS_TERM_FIXED_PART ] = MAGIC_WORD_TERM_FIXED_PART;
  m_header[ MIN_POS_TERM_HEADER ] = MAGIC_WORD_TERM_HEADER;
}


void RawHeader::set_nwords(int nwords)
{
  m_header[ POS_NWORDS ] = nwords;
}

void RawHeader::set_hdr_nwords()
{
  m_header[ POS_HDR_NWORDS ] = (MIN_POS_TERM_HEADER + 1) + SIZE_NODE_INFO * get_num_nodes();
}

void RawHeader::set_exp_no(int exp_no)
{
  m_header[ POS_EXP_RUN_NO ] = (m_header[ POS_EXP_RUN_NO ] & 0x003FFFFF) | ((exp_no << 22) & 0xFFC00000);
}
void RawHeader::set_run_no(int run_no)
{
  m_header[ POS_EXP_RUN_NO ] = (m_header[ POS_EXP_RUN_NO ] & 0xFFC00000) | (run_no & 0x003FFFFF);
}
void RawHeader::set_eve_no(unsigned int eve_no)
{
  m_header[ POS_EVE_NO ] = eve_no;
}
void RawHeader::set_subsys_id(int subsys_id)
{
  m_header[ POS_SUBSYS_ID ] = subsys_id;
}
void RawHeader::set_data_type(int data_type)
{
  m_header[ POS_DATA_TYPE ] = data_type;
}
void RawHeader::set_trunc_mask(int trunc_mask)
{
  m_header[ POS_TRUNC_MASK ] = trunc_mask;
}

void RawHeader::set_num_b2l_block(int num_b2l_block)
{
  m_header[ POS_NUM_B2L_BLOCK ] = num_b2l_block;
}

void RawHeader::set_offset_1st_b2l(int offset_1st_b2l)
{
  m_header[ POS_OFFSET_1ST_B2L ] = offset_1st_b2l;
}

void RawHeader::set_offset_2nd_b2l(int offset_2nd_b2l)
{
  m_header[ POS_OFFSET_2ND_B2L ] = offset_2nd_b2l;
}
void RawHeader::set_offset_3rd_b2l(int offset_3rd_b2l)
{
  m_header[ POS_OFFSET_3RD_B2L ] = offset_3rd_b2l;
}
void RawHeader::set_offset_4th_b2l(int offset_4th_b2l)
{
  m_header[ POS_OFFSET_4TH_B2L ] = offset_4th_b2l;
}

int RawHeader::add_nodeinfo_and_update_hdrsize(int node_id, int eve_no)
{

  // When the number of total nodes exceeds NUM_MAX_NODES
  if (get_num_nodes() >= NUM_MAX_NODES) {
    m_header[ MIN_POS_TERM_HEADER + NUM_MAX_NODES * SIZE_NODE_INFO + POS_NODE_ID ] = -1;
    m_header[ MIN_POS_TERM_HEADER + NUM_MAX_NODES * SIZE_NODE_INFO + POS_NODE_EVE_NO ] = -1;
    m_header[ POS_NUM_NODES ] = NUM_MAX_NODES + 1;
    set_hdr_nwords();
    m_header[ get_hdr_nwords() - 1 ] = MAGIC_WORD_TERM_HEADER;
    return -1;
  }

  m_header[ MIN_POS_TERM_HEADER + m_header[ POS_NUM_NODES ]*SIZE_NODE_INFO + POS_NODE_ID ] = node_id;
  m_header[ MIN_POS_TERM_HEADER + m_header[ POS_NUM_NODES ]*SIZE_NODE_INFO + POS_NODE_EVE_NO ] = eve_no;
  m_header[ POS_NUM_NODES ]++;
  set_hdr_nwords();
  m_header[ get_hdr_nwords() - 1 ] = MAGIC_WORD_TERM_HEADER;

  return 0;
}

int RawHeader::get_nwords() {  return m_header[ POS_NWORDS ];}
int RawHeader::get_hdr_nwords() {    return m_header[ POS_HDR_NWORDS ]; }
int RawHeader::get_exp_no()
{
  return ((m_header[ POS_EXP_RUN_NO ] >> 22) & 0x000003FF);
}
int RawHeader::get_run_no()
{
  return (m_header[ POS_EXP_RUN_NO ]  & 0x003FFFFF);
}

unsigned int RawHeader::get_eve_no() {  return m_header[ POS_EVE_NO ]; }
int RawHeader::get_subsys_id() {  return m_header[ POS_SUBSYS_ID ]; }
int RawHeader::get_data_type() {  return m_header[ POS_DATA_TYPE ]; }
int RawHeader::get_trunc_mask() {  return m_header[ POS_TRUNC_MASK ]; }
int RawHeader::get_num_b2l_block() {  return m_header[ POS_NUM_B2L_BLOCK ]; }
int RawHeader::get_offset_1st_b2l() {  return m_header[ POS_OFFSET_1ST_B2L ]; }
int RawHeader::get_offset_2nd_b2l() {  return m_header[ POS_OFFSET_2ND_B2L ]; }
int RawHeader::get_offset_3rd_b2l() {  return m_header[ POS_OFFSET_3RD_B2L ]; }
int RawHeader::get_offset_4th_b2l() {  return m_header[ POS_OFFSET_4TH_B2L ]; }
int RawHeader::get_num_nodes() {  return m_header[ POS_NUM_NODES ]; }

int RawHeader::get_node_info(int node_no, int* node_id, int* eve_no)
{
  if (node_no >= get_num_nodes()) {
    return -1;
  }
  *node_id = m_header[ MIN_POS_TERM_HEADER + node_no * SIZE_NODE_INFO + POS_NODE_ID ];
  *eve_no = m_header[ MIN_POS_TERM_HEADER + node_no * SIZE_NODE_INFO + POS_NODE_EVE_NO ];
  return 0;
}

unsigned int RawHeader::get_magic_word_fixed_part() {  return m_header[ POS_TERM_FIXED_PART ]; }

unsigned int RawHeader::get_magic_word_entire_header()
{
  return m_header[ MIN_POS_TERM_HEADER + SIZE_NODE_INFO * get_num_nodes() ];
}

