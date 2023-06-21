/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/dqm/HistoRelay.h"
#include <framework/pcore/EvtMessage.h>

using namespace Belle2;
using namespace std;

HistoRelay::HistoRelay(string& file, string& dest, int port)
{
  m_dest = dest;
  m_port = port;
  m_memfile = new DqmMemFile(file);
  m_sock = new EvtSocketSend(m_dest, m_port);
}

HistoRelay::~HistoRelay()
{
  delete m_sock;
  delete m_memfile;
}

int HistoRelay::collect()
{
  //  printf ( "HistoRelay : collect!!\n" );
  EvtMessage* msg = m_memfile->StreamMemFile();

  auto ret = m_sock->send(msg);

  delete (msg);

  if (ret < 0) {
    // Socket error, e.g server died
    delete m_sock;
    // try reconnect
    printf("HistoRelay: socket seems dead -> reconnect\n");
    m_sock = new EvtSocketSend(m_dest, m_port);
  }

  return 0;
}
