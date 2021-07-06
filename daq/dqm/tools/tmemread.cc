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

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{

  SharedMem* shm = new SharedMem("testhisto", 2000000);
  char* buf = new char[8000000];

  for (;;) {
    shm->lock();
    memcpy(buf, shm->ptr(), 8000000);
    shm->unlock();
    TMemFile* file = new TMemFile("TestHisto", (char*)shm->ptr(), 2000000, "READ", "", 0);
    file->ls();
    file->Print();
    TH1F* h1 = (TH1F*) file->Get("testhisto");
    h1->Print();
    sleep(1);
    delete file;
  }
  //    file->Write();
  //    file->Close();
}
