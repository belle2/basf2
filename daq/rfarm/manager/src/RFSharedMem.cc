//+
// File : RFSharedMem.cc
// Description : Shared Memory Manager for RFARM
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 26 - June - 2013
//-

#include "daq/rfarm/manager/RFSharedMem.h"

#include <strings.h>

using namespace std;
using namespace Belle2;

RFSharedMem::RFSharedMem(const char* name) : SharedMem(name, sizeof(RfShm_Data))
{
  RfShm_Data* rfshm = (RfShm_Data*) ptr();
  if (SharedMem::IsCreated()) {
    bzero((char*)rfshm, sizeof(RfShm_Data));
    for (int i = 0; i < RFSHM_MAX_PROCESS; i++) {
      (rfshm->cell[i]).pid = 0;
      (rfshm->cell[i]).type = 0;
      (rfshm->cell[i]).command = -1;
    }
  }
}

RFSharedMem::~RFSharedMem()
{
}

RfShm_Cell& RFSharedMem::GetCell(int id)
{
  RfShm_Data* rfshm = (RfShm_Data*) ptr();
  return rfshm->cell[id];
}

