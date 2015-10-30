#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "ecl_collector_lib.h"

const unsigned int MEM_BIN_ADDR = 0xA7000000;

int main(int argc, char** argv)
{

  if (argc < 2) {
    printf("usage: %s <col> \n", argv[0]);
    return 1;
  }
  int col = atoi(argv[1]);
  char ip [256];
  sprintf(ip, "192.168.1.%d", col);
  sh_boot(ip, 16, MEM_BIN_ADDR);

  return 0;
}
