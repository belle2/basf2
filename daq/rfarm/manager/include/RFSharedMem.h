#ifndef RFSHAREDMEM_H
#define RFSHAREDMEM_H
//+
// File : RFSharedMem.h
// Description : Shared Memory manager for RFARM node control
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 20 - June - 2013
//-

#define RFSHM_MAX_PROCESS 100
#define RFSHM_TYPE_SOCK2RB 1
#define RFSHM_TYPE_RB2SOCK 2
#define RFSHM_TYPE_STORE 3
#define RFSHM_TYPE_BASF2 4
#define RFSHM_TYPE_UNUSABLE -1

#include "SharedMem.h"

namespace Belle2 {

  // Cell to exchange information of client and server
  struct RfShm_Cell {
    pid_t pid;
    int type;
    int command;
    int par[2];
    int retval;
    int status;
    int nevent;
    int nqueue;
    float flowrate;
    float avesize;
    float evtrate;
    int elapsed;

  };

  // A collection of cells to be placed on a shared memory
  struct RfShm_Data {
    int ncell;
    char nodename[16];
    RfShm_Cell cell[RFSHM_MAX_PROCESS];
  };

  // A class to manage cells
  class RFSharedMem : public SharedMem {
  public:
    RFSharedMem(char* name);
    ~RFSharedMem();

    RfShm_Cell& GetCell(int id);
  };
}
#endif
