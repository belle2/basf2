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

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

int main(int argc, char** argv)
{
  DIR* dir;
  struct dirent* ent;
  if ((dir = opendir("/tmp")) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (strncmp(ent->d_name, "SHM", 3) == 0) {
        //  printf ("%s\n", ent->d_name);
        int shmid, semid;
        sscanf(ent->d_name, "SHM%d-SEM%d", &shmid, &semid);
        //  printf ( "shmid = %d, semid = %d\n", shmid, semid );
        if (shmid > 0) {
          shmctl(shmid, IPC_RMID, (struct shmid_ds*) 0);
          printf("SHM %d deleted. ", shmid);
        }
        if (semid > 0) {
          semctl(semid, 1, IPC_RMID);
          printf("SEM %d deleted. ", semid);
        }
        printf("\n");
        char strbuf[1024];
        sprintf(strbuf, "/tmp/%s", ent->d_name);
        unlink(strbuf);
      }
    }
    closedir(dir);
  } else {
    perror("");
    return -1;
  }
}
