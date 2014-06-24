#include <TMapFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TRandom.h>

int main(int argc, char** argv)
{
  TMapFile* file = TMapFile::Create(argv[1], "recreate", 512000000, "test");
  TH1F* hx1 = new TH1F("h_HitXPositionCh01", "Hit X position (Ch-01);X [mm];Entires", 1000, -20, 20);
  TH1F* hy1 = new TH1F("h_HitYPositionCh01", "Hit Y position (Ch-01);Y [mm];Entires", 1000, -30, 30);
  TH2F* hxy1 = new TH2F("h_HitXYPositionCh01", "Hit X - Y (Ch-01);X [mm];Y [mm]", 90, -20, 20, 90, -30, 30);
  TH1F* hx2 = new TH1F("h_HitXPositionCh02", "Hit X position (Ch-02);X [mm];Entires", 1000, -20, 20);
  TH1F* hy2 = new TH1F("h_HitYPositionCh02", "Hit Y position (Ch-02);Y [mm];Entires", 1000, -30, 30);
  TH2F* hxy2 = new TH2F("h_HitXYPositionCh02", "Hit X - Y (Ch-02);X [mm];Y [mm]", 90, -20, 20, 90, -30, 30);
  TH1F* hx3 = new TH1F("h_HitXPositionCh03", "Hit X position (Ch-03);X [mm];Entires", 1000, -20, 20);
  TH1F* hy3 = new TH1F("h_HitYPositionCh03", "Hit Y position (Ch-03);Y [mm];Entires", 1000, -30, 30);
  TH2F* hxy3 = new TH2F("h_HitXYPositionCh03", "Hit X - Y (Ch-03);X [mm];Y [mm]", 90, -20, 20, 90, -30, 30);
  file->Add(hx1);
  file->Add(hy1);
  file->Add(hxy1);
  file->Add(hx2);
  file->Add(hy2);
  file->Add(hxy2);
  file->Add(hx3);
  file->Add(hy3);
  file->Add(hxy3);
  int count = 0;
  while (true) {
    for (Int_t i = 0; i < 10000; i++) {
      Double_t x = gRandom->Gaus(0, 10);
      Double_t y = gRandom->Gaus(0, 10);
      hx1->Fill(x);
      hy1->Fill(y);
      hxy1->Fill(x, y);
    }
    for (Int_t i = 0; i < 10000; i++) {
      Double_t x = gRandom->Gaus(0, 10);
      Double_t y = gRandom->Gaus(0, 10);
      hx2->Fill(x);
      hy2->Fill(y);
      hxy2->Fill(x, y);
    }
    for (Int_t i = 0; i < 10000; i++) {
      Double_t x = gRandom->Gaus(0, 10);
      Double_t y = gRandom->Gaus(0, 10);
      hx3->Fill(x);
      hy3->Fill(y);
      hxy3->Fill(x, y);
    }
    file->Update();
    sleep(5);
    count++;
  }
}
