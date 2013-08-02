//+
// File : SendHeader.cc
// Description : Module to handle SendHeader attached by COPPER DAQ
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#include "daq/dataobjects/SendHeader.h"

using namespace std;
using namespace Belle2;

ClassImp(SendHeader);

SendHeader::SendHeader()
{
  initialize();
  //  cout << "SendHeader NULL constructor" << endl;
}

SendHeader::SendHeader(unsigned int* buffer)
{
  //  cout << "SendHeader constructor with buffer" << endl;
  memcpy(m_header, buffer, NUM_HDR_WORDS * sizeof(int));
}

SendHeader::~SendHeader()
{
}

unsigned int* SendHeader::header()
{
  return m_header;
}

void SendHeader::header(unsigned int* bufin)
{
  memcpy(m_header, bufin, NUM_HDR_WORDS * sizeof(int));
}

void SendHeader::initialize()
{
  memset(m_header, 0, sizeof(int)* NUM_HDR_WORDS);
  m_header[ POS_HDR_NWORDS ] = NUM_HDR_WORDS;
}

void SendHeader::set_nwords(int total_data_nwords)
{
  m_header[ POS_NWORDS ] = total_data_nwords;
}

void SendHeader::set_num_events_in_packet(int num_events)
{
  m_header[ POS_NUM_EVENTS_IN_PACKET ] = num_events;
}


int SendHeader::get_nwords() {  return m_header[ POS_NWORDS ];}
int SendHeader::get_hdr_nwords() {    return NUM_HDR_WORDS; }
int SendHeader::get_num_events_in_packet() {  return m_header[ POS_NUM_EVENTS_IN_PACKET ]; }

