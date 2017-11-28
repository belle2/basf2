//+
// File : HistoRelay.cc
// Description : Read Histograms in a TMapFile periodically and
//               relay them to HistoServer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 3 - Sep - 2013
//-

#include "dqm/HistoRelay.h"

#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

using namespace Belle2;
using namespace std;

HistoRelay::HistoRelay(string& file, string& dest, int port) : m_filename(file), m_dest(dest), m_port(port)
{
  //  m_map = TMapFile::Create(file.c_str());
  m_memfile = new DqmMemFile(file);
  m_sock = new EvtSocketSend(dest.c_str(), port);
  //  m_msg = new MsgHandler(0);
  m_msg = NULL;
}

HistoRelay::HistoRelay(const HistoRelay& hr) : m_filename(hr.m_filename), m_dest(hr.m_dest), m_port(hr.m_port)
{
  m_memfile = new DqmMemFile(hr.m_filename);
  m_sock = new EvtSocketSend(hr.m_dest, hr.m_port);
  m_msg = NULL;
}

HistoRelay::~HistoRelay()
{
  delete m_memfile;
  delete m_sock;
  delete m_msg;
}

HistoRelay& HistoRelay::operator=(const HistoRelay& other)
{
  if (this != &other) {
    if (m_memfile != NULL) delete m_memfile;
    if (m_sock != NULL) delete m_sock;
    if (m_msg != NULL) delete m_msg;
    m_filename = other.m_filename;
    m_dest = other.m_dest;
    m_port = other.m_port;
    m_memfile = new DqmMemFile(m_filename);
    m_sock = new EvtSocketSend(m_dest, m_port);
    m_msg = NULL;
  }
  return *this;
}

int HistoRelay::collect()
{
  //  printf ( "HistoRelay : collect!!\n" );
  //  m_map = TMapFile::Create(m_filename.c_str());
  EvtMessage* msg = m_memfile->StreamMemFile();

  /* Old Impl.
  EvtMessage* msg = m_msg->encode_msg(MSG_EVENT);

  (msg->header())->reserved[0] = 0;
  (msg->header())->reserved[1] = nobjs;
  (msg->header())->reserved[2] = 0;

  //  printf("HistoRelay : Sending %d histograms\n", nobjs);
  */

  m_sock->send(msg);

  delete(msg);

  return 0;
}
