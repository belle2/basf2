/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/dqm/HistoRelay.h"

#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

using namespace Belle2;
using namespace std;

HistoRelay::HistoRelay(string& file, string& dest, int port)
{
  //  m_map = TMapFile::Create(file.c_str());
  m_memfile = new DqmMemFile(file);
  m_sock = new EvtSocketSend(dest.c_str(), port);
  //  m_msg = new MsgHandler(0);
}

HistoRelay::~HistoRelay()
{
  delete m_sock;
  delete m_msg;
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
