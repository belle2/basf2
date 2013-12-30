//+
// File : clipc.cc
// Description : Remove uncleaned IPC resources
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 30 - Dec - 2013
//

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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
  }
  //  printf ( "mode = %d\n", mode );

  DIR* dir;
  struct dirent* ent;
  if ((dir = opendir("/tmp")) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (strncmp(ent->d_name, "SHM", 3) == 0) {
        //  printf ("%s\n", ent->d_name);
        int shmid, semid;
        char name[256];
        sscanf(ent->d_name, "SHM%d-SEM%d-%s", &shmid, &semid, name);
        //  printf ( "name : %s - shmid = %d, semid = %d\n", name, shmid, semid );
        int unnamed = strcmp(name, "UNNAMED");
        bool deleted = false;
        if (shmid > 0) {
          if (mode == 0 || (unnamed == 0 && mode == 2) ||
              (unnamed != 0 && mode == 1)) {
            shmctl(shmid, IPC_RMID, (struct shmid_ds*) 0);
            printf("SHM %d deleted. ", shmid);
            deleted = true;
          }
        }
        if (semid > 0) {
          if (mode == 0 || (unnamed == 0 && mode == 2) ||
              (unnamed != 0 && mode == 1)) {
            semctl(semid, 1, IPC_RMID);
            printf("SEM %d deleted. ", semid);
            deleted = true;
          }
        }
        if (deleted) {
          printf("\n");
          char strbuf[1024];
          sprintf(strbuf, "/tmp/%s", ent->d_name);
          unlink(strbuf);
          if (unnamed != 0) {
            sprintf(strbuf, "/tmp/%s_%s", getenv("USER"), name);
            unlink(strbuf);
          }
        }
      }
    }
    closedir(dir);
  } else {
    perror("");
    return -1;
  }
}
