/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "TMapFile.h"
#include "TH1.h"

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
