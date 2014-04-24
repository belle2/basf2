#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char** argv)
{
  if (argc < 2) {
    printf("usage: shm_clean <path to shm>");
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    if (shm_unlink(argv[i]) < 0) {
      perror("shm_unlink");
      return 1;
    }
  }
  return 0;
}
