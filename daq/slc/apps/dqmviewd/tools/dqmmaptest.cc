#include <TMapFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TRandom.h>

int main(int argc, char** argv)
{
  TMapFile* file = TMapFile::Create(argv[1], "recreate", 512000000, "test");
  TH1F* h1 = new TH1F("h1", "Histogram 1;X value;Entires", 10000, -20, 20);
  h1->SetLineColor(kBlue);
  h1->SetFillColor(-1);
  TH1F* h1_2 = new TH1F("h1_2", "Histogram 2;X value;Entires", 10000, -20, 20);
  h1_2->SetLineColor(kRed);
  h1_2->SetFillColor(-1);
  TH2F* h2 = new TH2F("h2", "2D-Histogram;X value;Y value", 900, -40, 40, 900, -30, 30);
  TH2F* h2_1 = new TH2F("h2_1", "2D-Histogram;X value;Y value", 900, -40, 40, 900, -30, 30);
  TH2F* h2_2 = new TH2F("h2_2", "2D-Histogram;X value;Y value", 900, -40, 40, 900, -30, 30);
  TH2F* h2_3 = new TH2F("h2_3", "2D-Histogram;X value;Y value", 900, -40, 40, 900, -30, 30);
  TH2F* h2_4 = new TH2F("h2_4", "2D-Histogram;X value;Y value", 900, -40, 40, 900, -30, 30);
  TH2F* h2_5 = new TH2F("h2_5", "2D-Histogram;X value;Y value", 900, -40, 40, 900, -30, 30);
  TH2F* h2_6 = new TH2F("h2_6", "2D-Histogram;X value;Y value", 900, -40, 40, 900, -30, 30);
  file->Add(h1);
  file->Add(h1_2);
  file->Add(h2);
  file->Add(h2_1);
  file->Add(h2_2);
  file->Add(h2_3);
  file->Add(h2_4);
  file->Add(h2_5);
  file->Add(h2_6);
  int count = 0;
  while (true) {
    int n = gRandom->Gaus(100, 2);
    if (count % 5 == 0) n *= 1.5;
    for (Int_t i = 0; i < n; i++)
      h1->Fill(gRandom->Gaus(0, 10));
    for (Int_t i = 0; i < 10000; i++)
      h1_2->Fill(gRandom->Gaus(0, 10));
    for (Int_t i = 0; i < 100000; i++) {
      h2->Fill(gRandom->Gaus(-5, 20), gRandom->Gaus(5, 12));
      h2_1->Fill(gRandom->Gaus(-5, 20), gRandom->Gaus(5, 12));
      h2_2->Fill(gRandom->Gaus(-5, 20), gRandom->Gaus(5, 12));
      h2_3->Fill(gRandom->Gaus(-5, 20), gRandom->Gaus(5, 12));
      h2_4->Fill(gRandom->Gaus(-5, 20), gRandom->Gaus(5, 12));
      h2_5->Fill(gRandom->Gaus(-5, 20), gRandom->Gaus(5, 12));
      h2_6->Fill(gRandom->Gaus(-5, 20), gRandom->Gaus(5, 12));
    }
    file->Update();
    sleep(5);
    count++;
  }
}
