#include <daq/slc/system/LogFile.h>

#include <TMapFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TRandom.h>

int main(int argc, char** argv)
{
  while (true) {
    TMapFile* file = TMapFile::Create(argv[1]);
    file->Print();
    TMapRec* mr = file->GetFirst();
    while (file->OrgAddress(mr)) {
      TObject* obj = file->Get(mr->GetName());
      if (obj != NULL) {
        TString class_name = obj->ClassName();
        if (class_name.Contains("TH1") || class_name.Contains("TH2")) {
          TH1* h = (TH1*)obj;
        }
      }
      mr = mr->GetNext();
    }
    file->Close();
    sleep(5);
  }
}

