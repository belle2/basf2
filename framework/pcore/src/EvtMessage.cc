// +
// File : EvtMessage.cc
// Description : Low level interface to transfer serialized object
//
// Author : Soohyung Lee and Ryosuke Itoh
// Date : 24 - Jul - 2008
// Modified : 9 - Jun - 2010
//-

#include <framework/pcore/EvtMessage.h>

#include <iostream>
#include <cstdlib>
#include <string>

//#include "event/Event.h"

using namespace std;
using namespace Belle2;


/// @brief Null constructor
EvtMessage::EvtMessage(void)
{
  m_data = NULL;
  m_buftype = 0;
}

/// @brief Constructor of EvtMessage using existing buffer
/// @param msg Actual data
/// @param size Length of the actual data
EvtMessage::EvtMessage(char* data)
{
  m_data = data;
  m_buftype = 0;
}


/// @brief Constructor of EvtMessage allocating new buffer
/// @param msg  data
/// @param size Length of the data (TMessage)
EvtMessage::EvtMessage(char* sobjs, int size, RECORD_TYPE type = MSG_EVENT)
{
  m_data = new char[size+sizeof(EvtHeader)]; // Allocate new buffer
  msg(sobjs, size, type);
  m_buftype = 1;
}

/// @brief Copy constructor of EvtMessage class
/// @param evtmsg Original EvtMessage object
EvtMessage::EvtMessage(EvtMessage& evtmsg)
{
  *this = evtmsg;
}

/// @brief Destructor of EvtMessage class
EvtMessage::~EvtMessage(void)
{
  if (m_buftype == 1)
    delete [] m_data;
  m_buftype = 0;
}

/// @brief Overridden assign operator
/// @param obj Source object
/// @param Pointer to assigned object
EvtMessage& EvtMessage::operator=(EvtMessage & obj)
{
  if (this != &obj) {
    this->buffer(obj.buffer());
  }

  return *this;
}

/// @brief Get buffer
/// @return BUffer address

char* EvtMessage::buffer(void)
{
  return m_data;
}

// @brief Set buffer
// @param Existing buffer address

void EvtMessage::buffer(char* bufadr)
{
  int size = *(int*)bufadr;
  m_data = new char[size];
  memcpy(m_data, bufadr, size);
  m_buftype = 1;
}

// @brief size
// @return record size
int EvtMessage::size(void)
{
  return (((EvtHeader*)m_data)->size);
}

// @brief msgsize
// @return message size
int EvtMessage::msg_size(void)
{
  return (((EvtHeader*)m_data)->size - sizeof(EvtHeader));
}

// Record type
RECORD_TYPE EvtMessage::type(void)
{
  return (((EvtHeader*)m_data)->rectype);
}

void EvtMessage::type(RECORD_TYPE type)
{
  ((EvtHeader*)m_data)->rectype = type;
}

// Source of this record
int EvtMessage::src(void)
{
  return (((EvtHeader*)m_data)->src);
}

void EvtMessage::src(int src)
{
  ((EvtHeader*)m_data)->src = src;
}

// Destination of this record
int EvtMessage::dest(void)
{
  return (((EvtHeader*)m_data)->dest);
}

void EvtMessage::dest(int dest)
{
  ((EvtHeader*)m_data)->dest = dest;
}

// Time stamp
struct timeval EvtMessage::time(void) {
  return (((EvtHeader*)m_data)->timestamp);
}

void EvtMessage::time(struct timeval& tbuf)
{
  ((EvtHeader*)m_data)->timestamp = tbuf;
}

// Event Header
EvtHeader* EvtMessage::header(void)
{
  return ((EvtHeader*)m_data);
}

// Message body
char* EvtMessage::msg(void)
{
  return (m_data + sizeof(EvtHeader));
}

// set_message

void EvtMessage::msg(char const* msgin, int size, RECORD_TYPE type)
{
  EvtHeader* hdr = (EvtHeader*)m_data;
  hdr->size = size + sizeof(EvtHeader);
  hdr->rectype = type;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  hdr->timestamp = tv;
  hdr->src = -1;
  hdr->dest = -1;
  if (size > 0)
    memcpy(m_data + sizeof(EvtHeader), msgin, size);
}


