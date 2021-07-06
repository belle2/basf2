/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/rfarm/manager/SharedMem.h"

#include "TH1F.h"
#include "TMemFile.h"
#include "TObject.h"

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  SharedMem* shm = new SharedMem("testhisto", 2000000);
  //  TMemFile* file = new TMemFile ( "TestHisto", (char*)shm->ptr(), 2000000, "RECREATE", "", 0 );
  TMemFile* file = new TMemFile("TestHisto", "RECREATE");
  int cpysize = file->CopyTo(shm->ptr(), 500000);

  //  TMapFile* file = TMapFile::Create("TestHisto", "RECREATE", 2000000);
  //  TFile* file = new TFile ( "testhist.root", "RECREATE" );
  TH1F* h1 = new TH1F("testhisto", "testhisto", 100, 0.0, 100.0);
  file->Write(0, TObject::kOverwrite);

  //  file->Add(h1);
  h1->Fill(5.0, 5.0);
  //  cpysize = file->CopyTo( shm->ptr(), 500000 );
  //  for (int i=0; i<10; i++ ) {
  for (int i = 0; i < 100; i++) {
    h1->Fill((float)i, (float)i);
    sleep(1);
    //      file->Update();
    //      cpysize = file->CopyTo( shm.ptr(), 2000000 );
    file->Write(0, TObject::kOverwrite);
    shm->lock();
    cpysize = file->CopyTo(shm->ptr(), 500000);
    shm->unlock();
    printf("Objects copied : cpysize = %d\n", cpysize);
  }
  //    h1->Reset();
  //  }
  cpysize = file->CopyTo(shm->ptr(), 500000);
  file->ls();
  file->Print();
  file->Write();
  file->Close();
}
