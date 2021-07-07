/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef RFSHAREDMEM_H
#define RFSHAREDMEM_H

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
    RFSharedMem(const char* name);
    ~RFSharedMem();

    RfShm_Cell& GetCell(int id);
  };
}
#endif
