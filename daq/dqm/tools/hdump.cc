#include "daq/dqm/DqmMemFile.h"

#include "TObject.h"
#include "TKey.h"
#include "TIterator.h"
#include "TDirectory.h"
#include "TList.h"
#include "TH1.h"

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{

  DqmMemFile* mfile = new DqmMemFile(argv[1]);

  TMemFile* file = mfile->LoadMemFile();
  file->ls();
  file->Print();

  TList* keylist = file->GetListOfKeys();
  TIter nextkey(keylist);
  TKey* key = 0;
  while ((key = (TKey*)nextkey())) {
    TObject* obj = key->ReadObj();
    if (obj->IsA()->InheritsFrom(TH1::Class())) {
      TH1* hist = (TH1*) obj;
      printf("histo: %s, title: %s \n", hist->GetName(), hist->GetTitle());
    }
  }

  delete(mfile);
}
