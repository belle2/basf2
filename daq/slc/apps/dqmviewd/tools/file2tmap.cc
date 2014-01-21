#include <TFile.h>
#include <TMapFile.h>
#include <TH1.h>
#include <TH2F.h>
#include <TRandom.h>
#include <TKey.h>

int main(int argc, char** argv)
{
  TMapFile* map = TMapFile::Create(argv[1], "recreate", 100000, "test");
  TFile* file = new TFile(argv[2], "recreate");
  TIter next(file->GetListOfKeys());
  TKey* key;
  while ((key = (TKey*)next())) {
    TH1* h = (TH1*)file->Get(key->GetName());
    map->Add(h);
  }
  map->Update();
}
