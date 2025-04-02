/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/dqm/HistoRelay2.h"
#include <framework/pcore/EvtMessage.h>
#include <TFile.h>
#include <TH1.h>
#include <TText.h>
#include <TKey.h>

using namespace Belle2;
using namespace std;

HistoRelay2::HistoRelay2(string& filename, string& dest, int port)
{
  m_dest = dest;
  m_port = port;
  m_filename = filename;
  m_sock = new EvtSocketSend(m_dest, m_port);
}

HistoRelay2::~HistoRelay2()
{
  delete m_sock;
}

int HistoRelay2::collect()
{
  //  printf ( "HistoRelay2 : collect!!\n" );
  EvtMessage* msg = StreamFile(m_filename);

  if (!msg) return 1; // error indicator

  auto ret = m_sock->send(msg);

  delete (msg);

  if (ret < 0) {
    // Socket error, e.g server died
    delete m_sock;
    // try reconnect
    printf("HistoRelay2: socket seems dead -> reconnect\n");
    m_sock = new EvtSocketSend(m_dest, m_port);
  }

  return 0;
}

// Copy Shared Memory to local and stream
EvtMessage* HistoRelay2::StreamFile(string& filename)
{
  TFile* file = new TFile((std::string("/dev/shm/") + filename).c_str(), "read");
  if (file == NULL || file->IsZombie()) return NULL;
  file->cd();
  MsgHandler hdl(0);
  int numobjs = 0;
  StreamHistograms(gDirectory, &hdl, numobjs);
  file->Close();
  delete file;
  //  printf ( "DqmMemFile::StreamMemFile : streamed %d histograms in EvtMessage\n", numobjs );
  EvtMessage* msg = hdl.encode_msg(MSG_EVENT);
  (msg->header())->reserved[0] = 0;
  (msg->header())->reserved[1] = numobjs;
  return msg;
}

int HistoRelay2::StreamHistograms(TDirectory* curdir, MsgHandler* msg, int& numobjs)
{
  TList* keylist = curdir->GetListOfKeys();

  TIter nextkey(keylist);
  TKey* key = 0;
  while ((key = (TKey*)nextkey())) {
    TObject* obj = curdir->FindObjectAny(key->GetName());
    if (obj->IsA()->InheritsFrom("TH1")) {
      TH1* h1 = (TH1*) obj;
      //      printf ( "Key = %s, entry = %f\n", key->GetName(), h1->GetEntries() );
      msg->add(h1, h1->GetName());
      numobjs++;
    } else if (obj->IsA()->InheritsFrom(TDirectory::Class())) {
      //      printf ( "New directory found  %s, Go into subdir\n", obj->GetName() );
      TDirectory* tdir = (TDirectory*) obj;
      //      m_msg->add(tdir, tdir->GetName());
      TText subdir(0, 0, tdir->GetName());
      msg->add(&subdir, "SUBDIR:" + string(obj->GetName())) ;
      numobjs++;
      tdir->cd();
      StreamHistograms(tdir, msg, numobjs);
      TText command(0, 0, "COMMAND:EXIT");
      msg->add(&command, "SUBDIR:EXIT");
      numobjs++;
      curdir->cd();
    }
  }
  return 0;
}
