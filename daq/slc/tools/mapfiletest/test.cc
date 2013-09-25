#include <TMapFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TRandom.h>

int main()
{
  TMapFile* file = TMapFile::Create("/tmp/test.map", "recreate", 100000, "test");
  TH1F* h1 = new TH1F("h1", "Histogram 1;X value;Entires", 100, -20, 20);
  h1->SetLineColor(kBlue);
  h1->SetFillColor(-1);
  TH1F* h1_2 = new TH1F("h1_2", "Histogram 2;X value;Entires", 100, -20, 20);
  h1_2->SetLineColor(kRed);
  h1_2->SetFillColor(-1);
  TH2F* h2 = new TH2F("h2", "2D-Histogram;X value;Y value", 40, -40, 40, 30, -30, 30);
  //TH1F* h3 = new TH1F("h3", "Histogram 3;X value;Entires", 100, -20, 20);
  //h3->SetLineColor(kBlue);
  //h3->SetFillColor(-1);
  //TH1F* h3_2 = new TH1F("h3_2", "Histogram 3-2;X value;Entires", 100, -20, 20);
  //h3_2->SetLineColor(kRed);
  //h3_2->SetFillColor(-1);
  //TH2F* h4 = new TH2F("h4", "2D-Histogram 2;X value;Y value", 40, -40, 40, 30, -30, 30);
  file->Add(h1);
  file->Add(h1_2);
  file->Add(h2);
  //file->Add(h3);
  //file->Add(h3_2);
  //file->Add(h4);
  int count = 0;
  while (true) {
    int n = gRandom->Gaus(100, 2);
    if (count % 5 == 0) n *= 1.5;
    for (Int_t i = 0; i < n; i++)
      h1->Fill(gRandom->Gaus(0, 10));
    for (Int_t i = 0; i < 100; i++)
      h1_2->Fill(gRandom->Gaus(0, 10));
    for (Int_t i = 0; i < 1000; i++)
      h2->Fill(gRandom->Gaus(-5, 20), gRandom->Gaus(5, 12));
    file->Update();
    sleep(5);
    count++;
  }
}
