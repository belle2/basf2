/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <dirent.h>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

int main(int argc, char** argv)
{
  int mode = 2; // unnamed IPC resources only
  if (argc > 1) {
    if (strcmp(argv[1], "all")  == 0)
      mode = 0;
    else if (strcmp(argv[1], "named") == 0)
      mode = 1;
    else if (strcmp(argv[1], "unnamed") == 0)
      mode = 2;
    else {
      printf("Usage : clear_basf2_ipc {all|named|unnamed} \n");
      printf("all: all IPCs, named: named IPCs only, unnamed: basf2 internal IPCs only(default)\n");

      printf("\n Cleans up inter-process communication (IPC) resources left behind if basf2 crashes.\n\n");
      printf("Normally, this shouldn't be necessary, but resources may remain after a bad crash or basf2 is killed using SIGKILL.\n");
      printf("Resources currently in use are not touched.\n");
      printf("The return code is set to 1 when uncleaned resources are found, 0 if none are found, and -1 on error.\n");
      return -1;
    }
  }

  DIR* dir;
  if ((dir = opendir("/tmp")) != nullptr) {
    int ret = 0;
    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
      if (strncmp(ent->d_name, "SHM", 3) == 0) {
        //  printf ("%s\n", ent->d_name);
        int shmid, semid;
        char name[256];
        sscanf(ent->d_name, "SHM%d-SEM%d-%255s", &shmid, &semid, name);
        //  printf ( "name : %s - shmid = %d, semid = %d\n", name, shmid, semid );
        int unnamed = strcmp(name, "UNNAMED");
        bool deleted = false;
        bool stale = false;
        if (shmid > 0) {
          if (mode == 0 || (unnamed == 0 && mode == 2) ||
              (unnamed != 0 && mode == 1)) {

            shmid_ds shmInfo;
            if (shmctl(shmid, IPC_STAT, &shmInfo) != 0) {
              printf("Removing stale file %s\n", ent->d_name);
              stale = true; //already gone (note: stale files do not produce non-zero return code)
            } else {
              //Don't remove SHM segments which still have a process attached
              //Note that nattch counter is decreased by both shmdt() and exit()
              if (shmInfo.shm_nattch != 0) {
                printf("/tmp/%s still has %ld processes attached, skipped.\n", ent->d_name, shmInfo.shm_nattch);
                continue;
              }

              if (shmctl(shmid, IPC_RMID, (struct shmid_ds*) nullptr) == 0) {
                printf("SHM %d deleted. ", shmid);
                deleted = true;
              }
            }
          }
        }
        if (semid > 0) {
          if (mode == 0 || (unnamed == 0 && mode == 2) ||
              (unnamed != 0 && mode == 1)) {
            if (semctl(semid, 1, IPC_RMID) == 0) {
              printf("SEM %d deleted. ", semid);
              deleted = true;
            }
          }
        }
        if (deleted or stale) {
          ret = 1;
          printf("\n");
          char strbuf[1024];
          snprintf(strbuf, 1024, "/tmp/%s", ent->d_name);
          unlink(strbuf);
          if (unnamed != 0) {
            snprintf(strbuf, 1024, "/tmp/%s_%s", getenv("USER"), name);
            unlink(strbuf);
          }
        }
      }
    }
    closedir(dir);
    return ret;
  } else {
    perror("");
    return -1;
  }
}
