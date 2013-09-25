#include <TMapFile.h>
#include <TH1F.h>
#include <TRandom.h>

#include <iostream>

int main()
{
  TMapFile* file = TMapFile::Create("test.map");
  TMapRec* mr = file->GetFirst();
  while (file->OrgAddress(mr)) {
    TObject* obj = file->Get(mr->GetName());
    if (obj != NULL) {
      TString class_name = obj->ClassName();
      std::cout << class_name.Data() << std::endl;
    }
    mr = mr->GetNext();
  }
  /*
  TH1F* h = new TH1F("h1", ";H1;X;Y", 100, -20, 20);
  for( Int_t i = 0; i < 1000; i++ )
    h->Fill( gRandom->Gaus( 0, 10 ) );
  file->Add(h);
  CanvasPanelInfo* panel = new CanvasPanelInfo("c_test");
  file->Add(panel);
  */
}
