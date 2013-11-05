#include "FTSWFIFOListener.h"

#include <cstdlib>
#include <cstdio>

using namespace Belle2;

void FTSWFIFOListener::run()
{
  while (true) {
    while (true) {
      int stafifo = read_ftsw(_ftsw_fd, FTSWREG_STAFIFO);
      if ((stafifo & 0x10000000) == 0) break;
    }
    int val1 = read_ftsw(_ftsw_fd, FTSWREG_FIFO);
    int val2 = read_ftsw(_ftsw_fd, FTSWREG_FIFO);
    printf("%08x %08x\n", val1, val2);
  }
}
