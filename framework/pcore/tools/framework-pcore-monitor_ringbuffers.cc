/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/pcore/RingBuffer.h>
#include <framework/logging/Logger.h>

#include <iostream>
#include <iomanip>

#include <dirent.h>
#include <cstdio>
#include <unistd.h>
#include <cstring>

#include <sys/ipc.h>
#include <sys/shm.h>


void rbinfo(int shmid)
{
  const int* shmadr = (int*) shmat(shmid, nullptr, SHM_RDONLY);
  if (shmadr == (int*) - 1) {
    B2FATAL("RingBuffer: Attaching to shared memory segment via shmat() failed");
  }
  const auto* m_bufinfo = reinterpret_cast<const Belle2::RingBufInfo*>(shmadr);

  long filled_bytes = m_bufinfo->wptr - m_bufinfo->rptr;
  if (filled_bytes < 0)
    filled_bytes += m_bufinfo->size;
  filled_bytes *= sizeof(int); //weird size unit
  double filled_MB = (filled_bytes) / 1024.0 / 1024.0;
  std::cout << "RB " << shmid
            << " #Tx: " << m_bufinfo->numAttachedTx << "\t"
            << " #busy: " << m_bufinfo->nbusy << "\t"
            << m_bufinfo->nbuf << " entries\t"
            << filled_MB << " MiB filled\t(" << 100.0 * filled_bytes / double(m_bufinfo->size * sizeof(int)) << " %)"
            << "\n";

  shmdt(shmadr);
}

std::vector<int> findRingBuffers()
{
  std::vector<int> buffer_SHMs;
  DIR* dir;
  if ((dir = opendir("/tmp")) != nullptr) {
    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
      if (strncmp(ent->d_name, "SHM", 3) == 0) {
        int shmid, semid;
        char name[256];
        sscanf(ent->d_name, "SHM%d-SEM%d-%255s", &shmid, &semid, name);
        if (shmid > 0) {
          shmid_ds shmInfo;
          if (shmctl(shmid, IPC_STAT, &shmInfo) == 0) {
            //only use SHM segments which still have a process attached
            //Note that nattch counter is decreased by both shmdt() and exit()
            if (shmInfo.shm_nattch != 0) {
              buffer_SHMs.push_back(shmid);
            }
          }
        }
      }
    }

    closedir(dir);
  } else {
    perror("");
  }
  return buffer_SHMs;
}


int main(int argc, char**)
{
  if (argc > 1) {
    printf("Usage : monitor_ringbuffers\n");

    printf("\n Shows fill-state information on all active RingBuffers on this system.\n\n");
    return -1;
  }

  std::cout << std::setprecision(2) << std::fixed;
  while (true) {
    std::vector<int> buffer_SHMs = findRingBuffers();
    if (buffer_SHMs.empty())
      break;
    for (int shmid : buffer_SHMs)
      rbinfo(shmid);
    std::cout << "\n";
    usleep(100000);
  }
  std::cout << "No Ringbuffers found, exiting.\n";

  return 0;
}
