#include "daq/dqm/HistoServer.h"

int main(int argc, char** argv)
{
  TMapFile* file = TMapFile::Create("TestHisto", "RECREATE", 2000000);
  TH1F* h1 = new TH1F("testhisto", "testhisto", 100, 0.0, 100.0);
  file->Add(h1);
  for (;;) {
    for (int i = 0; i < 100; i++) {
      h1->Fill((float)i, (float)i);
      sleep(1);
      file->Update();
    }
    h1->Reset();
  }
}
