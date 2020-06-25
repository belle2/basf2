#include "daq/dqm/DqmMemFile.h"

#include <TH1F.h>

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{

  //  DqmMemFile* mfile = new DqmMemFile ( "dqmmemfile" );
  DqmMemFile* mfile = new DqmMemFile(argv[1]);

  for (;;) {
    TMemFile* file = mfile->LoadMemFile();
    file->ls();
    file->Print();
    //    file->Dump();
    TH1F* h1 = (TH1F*) file->Get("Mbc");
    if (h1 != NULL)
      h1->Print();
    sleep(1);
    //    delete file;
  }
  //    file->Write();
  //    file->Close();
}
