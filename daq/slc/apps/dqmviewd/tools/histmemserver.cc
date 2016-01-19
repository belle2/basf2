//#include <daq/dqm/analysis/HistMemory.h>
#include <daq/dqm/DqmMemFile.h>

#include <TMapFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TRandom.h>
#include <cstdio>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    printf("Usage : %s <path_to_tmapfile>\n", argv[0]);
    return 1;
  }
  DqmMemFile* memory = new DqmMemFile(argv[1], "write");
  TMemFile* file = memory->GetMemFile();
  file->cd();
  TH1F* hx1 = new TH1F("FirstDet/h_HitXPositionCh01", "Hit X position (Ch-01);X [mm];Entires", 1000, -20, 20);
  TH1F* hy1 = new TH1F("FirstDet/h_HitYPositionCh01", "Hit Y position (Ch-01);Y [mm];Entires", 1000, -30, 30);
  TH2F* hxy1 = new TH2F("FirstDet/h_HitXYPositionCh01", "Hit X - Y (Ch-01);X [mm];Y [mm]", 90, -20, 20, 90, -30, 30);
  TH1F* h2[20];
  for (int i = 0; i < 20; i++) {
    h2[i] = new TH1F(Form("SecondDet/h_HitCh%02d", i + 1),
                     Form("ADC count (Channel-%02d); ADC Count [digit];Entires", i + 1), 200, 0, 200);
  }
  TH1F* h3 = new TH1F("ThirdDet/h_Trigger", "Trigger information;;", 5, 0, 5);
  h3->GetXaxis()->SetBinLabel(1, "CDC");
  h3->GetXaxis()->SetBinLabel(2, "ECL");
  h3->GetXaxis()->SetBinLabel(3, "BPID");
  h3->GetXaxis()->SetBinLabel(4, "EPID");
  h3->GetXaxis()->SetBinLabel(5, "KLM");
  std::vector<TH1*> hist;
  /*
  memory.add(hx1);
  memory.add(hy1);
  memory.add(hxy1);
  for (int i = 0; i < 20; i++) {
    memory.add(h2[i]);
  }
  memory.add(h3);
  */

  int count = 0;
  while (true) {
    for (Int_t i = 0; i < 10000; i++) {
      Double_t x = gRandom->Gaus(0, 10);
      Double_t y = gRandom->Gaus(0, 10);
      hx1->Fill(x);
      hy1->Fill(y);
      hxy1->Fill(x, y);
    }
    for (Int_t i = 0; i < 20; i++) {
      for (Int_t j = 0; j < 500; j++) {
        Double_t x = gRandom->Gaus(30 + i * 20, 10);
        h2[i]->Fill(x);
      }
    }
    for (Int_t i = 0; i < 200; i++) {
      Double_t x = gRandom->Gaus(2, 3);
      h3->Fill(x);
    }
    memory->UpdateSharedMem();
    sleep(30);
    count++;
  }
}
