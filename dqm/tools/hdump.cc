#include "dqm/DqmMemFile.h"
#include <iostream>


using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{

  if (argc < 2) {
    cout << "Usage: hdump dqmmemfile" << endl;
    return 1;
  }
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
