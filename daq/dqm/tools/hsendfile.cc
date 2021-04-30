#include <daq/dataflow/EvtSocket.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

#include "TObject.h"
#include "TText.h"
#include "TH1.h"
#include "TKey.h"
#include "TFile.h"
#include "TIterator.h"
#include "TDirectory.h"
#include "TList.h"

#include <stdio.h>
#include <string>

using namespace Belle2;
using namespace std;

void StreamHistogramsInDir(TDirectory* curdir, MsgHandler* hdl, int& numobjs)
{
  TList* keylist = curdir->GetListOfKeys();

  TIter nextkey(keylist);
  TKey* key = 0;
  int nkeys = 0;
  int nobjs = 0;
  while ((key = (TKey*)nextkey())) {
    nkeys++;
    TObject* obj = key->ReadObj();
    if (obj->IsA()->InheritsFrom("TH1")) {
      TH1* h1 = (TH1*) obj;
      hdl->add(h1, h1->GetName());
      nobjs++;
      numobjs++;
    } else if (obj->IsA()->InheritsFrom(TDirectory::Class())) {
      TDirectory* tdir = (TDirectory*) obj;
      printf("subdir: %s\n", tdir->GetName());
      TText subdir(0, 0, tdir->GetName());
      hdl->add(&subdir, "SUBDIR:" + string(obj->GetName())) ;
      nobjs++;
      numobjs++;
      tdir->cd();
      StreamHistogramsInDir(tdir, hdl, numobjs);
      TText command(0, 0, "COMMAND:EXIT");
      hdl->add(&command, "SUBDIR:EXIT");
      nobjs++;
      numobjs++;
      curdir->cd();
    }
  }
}

int main(int argc, char** argv)
{
  if (argc < 4) {
    printf("Usage : hsendfile file host port\n");
    exit(-1);
  }
  string file = string(argv[1]);
  string host = string(argv[2]);
  int port = atoi(argv[3]);

  EvtSocketSend* sock = new EvtSocketSend(host.c_str(), port);

  MsgHandler hdl(0);
  int numobjs = 0;

  TFile* f = new TFile(file.c_str());

  StreamHistogramsInDir(f, &hdl, numobjs);

  EvtMessage* msg = hdl.encode_msg(MSG_EVENT);
  (msg->header())->reserved[0] = 0;
  (msg->header())->reserved[1] = numobjs;

  sock->send(msg);
  delete(msg);
  delete(sock);

  f->Close();
  delete f;

  return 0;
}
