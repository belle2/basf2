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
  //  m_map = TMapFile::Create(file.c_str());
  m_filename = file;
  m_sock = new EvtSocketSend(dest.c_str(), port);
  m_msg = new MsgHandler(0);
}

HistoRelay::~HistoRelay()
{
  delete m_sock;
  delete m_msg;
  m_map->Close();
}

int HistoRelay::collect()
{
  //  printf ( "HistoRelay : collect!!\n" );
  m_map = TMapFile::Create(m_filename.c_str());
  int nobjs = 0;
  TMapRec* mr = m_map->GetFirst();
  while (m_map->OrgAddress(mr)) {
    //    printf ( "HistoRelay : name = %s\n", mr->GetName() );
    TObject* obj = m_map->Get(mr->GetName());
    if (obj != NULL) {
      TH1* h1 = (TH1*) obj;
      m_msg->add(h1, h1->GetName());
      //      printf ( "HistoRelay : obj = %s\n", h1->GetName() );
      nobjs++;
    }
    delete obj;
    mr = mr->GetNext();
  }
  m_map->Close();
  delete m_map;

  EvtMessage* msg = m_msg->encode_msg(MSG_EVENT);

  (msg->header())->reserved[0] = 0;
  (msg->header())->reserved[1] = nobjs;
  (msg->header())->reserved[2] = 0;

  //  printf("HistoRelay : Sending %d histograms\n", nobjs);

  m_sock->send(msg);

  delete(msg);

  return 0;
}

