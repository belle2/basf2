#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fstream>

#include "ecl_collector_lib.h"

const unsigned int MEM_BIN_ADDR = 0xA7000000;

int main(int argc, char** argv)
{
  std::ifstream fin("/home/run/shaper/config.dat");
  int col;
  char ip [256];
  while (fin >> col) {
    sprintf(ip, "192.168.1.%d", col);
    sh_boot(ip, 16, MEM_BIN_ADDR);
  }

  return 0;
}
