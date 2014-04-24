#include <TFile.h>
#include <TMapFile.h>
#include <TH1.h>
#include <TH2F.h>
#include <TRandom.h>
#include <TKey.h>

#include <iostream>

int main(int argc, char** argv)
{
  TMapFile* map = TMapFile::Create(argv[1], "recreate", 10000000, "test");
  TFile* file = new TFile(argv[2]);
  int count = 0;
  while (true) {
    TIter next(file->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {
      TH1* h = (TH1*)file->Get(key->GetName());
      if (count == 0) map->Add(h);
      else h->Scale(count);
      std::cout << h->GetName() << " " << h->Integral() << std::endl;
    }
    map->Update();
    sleep(5);
    count++;
  }
}
