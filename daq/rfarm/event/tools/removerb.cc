/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "framework/pcore/RingBuffer.h"

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 2) {
    printf("remove : rbufname \n");
    exit(-1);
  }
  string pathname = string("/tmp/") + string(getenv("USER"))
                    + string("_RB_") + string(argv[1]);
  FILE* fd = fopen(pathname.c_str(), "r");
  if (fd == NULL) {
    printf("[removerb] No such RingBuffer : %s\n", argv[1]);
    exit(-1);
  }
  int shmid, semid;
  fscanf(fd, "%d", &shmid);
  fscanf(fd, "%d", &semid);
  fclose(fd);

  printf("Removing shmid = %d, semid = %d\n", shmid, semid);
  shmctl(shmid, IPC_RMID, (struct shmid_ds*) 0);
  semctl(semid, 1, IPC_RMID);
  printf("Removing pathfile = %s\n", pathname.c_str());
  unlink(pathname.c_str());

  // remove id file
  char fname[1024];
  sscanf(fname, "/tmp/SHM%d-SEM%d-RB_%s", argv[1]);
  unlink(fname);
}



