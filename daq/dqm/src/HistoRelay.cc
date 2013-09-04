//+
// File : HistoRelay.cc
// Description : Read Histograms in a TMapFile periodically and
//               relay them to HistoServer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 3 - Sep - 2013
//-

#include "daq/dqm/HistoRelay.h"

using namespace Belle2;
using namespace std;

HistoRelay::HistoRelay(string& file, string& dest, int port)
{
  m_map = TMapFile::Create(file.c_str());
  //  m_map->CreateSemaphore();
  //  m_map->ReleaseSemaphore();

  m_sock = new EvtSocketSend(dest.c_str(), port);
  m_msg = new MsgHandler(0);
}

HistoRelay::~HistoRelay()
{
  delete m_sock;
  delete m_msg;
  //  delete m_map;
}

int HistoRelay::collect()
{
  //  m_map->AcquireSemaphore();
  int nobjs = 0;
  TMapRec* mr = m_map->GetFirst();
  while (m_map->OrgAddress(mr)) {
    TObject* obj = m_map->Get(mr->GetName());
    if (obj != NULL) {
      TH1* h1 = (TH1*) obj;
      m_msg->add(h1, h1->GetName());
      nobjs++;
    }
  }
  //  m_map->ReleaseSemaphore();

  EvtMessage* msg = m_msg->encode_msg(MSG_EVENT);

  (msg->header())->reserved[0] = 0;
  (msg->header())->reserved[1] = nobjs;
  (msg->header())->reserved[2] = 0;

  m_sock->send(msg);

  delete(msg);

  return 0;
}

