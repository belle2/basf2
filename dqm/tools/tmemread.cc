#include "dqm/HistoServer.h"

#include "dqm/SharedMem.h"
#include "TMemFile.h"
#include "TObject.h"

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{

  SharedMem* shm = new SharedMem("testhisto", 2000000);
  char* buf = new char[8000000];

  for (;;) {
    shm->lock();
    memcpy(buf, shm->ptr(), 8000000);
    shm->unlock();
    TMemFile* file = new TMemFile("TestHisto", (char*)shm->ptr(), 2000000, "READ", "", 0);
    file->ls();
    file->Print();
    TH1F* h1 = (TH1F*) file->Get("testhisto");
    h1->Print();
    sleep(1);
    delete file;
  }
  //    file->Write();
  //    file->Close();
}
