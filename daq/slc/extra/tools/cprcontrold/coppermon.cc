#include "HSLBController.h"
#include "COPPERController.h"
#include "TTRXController.h"

#include <unistd.h>
#include <cstdio>

using namespace Belle2;

int main()
{
  COPPERController copper;
  copper.open();
  TTRXController ttrx;
  ttrx.open();
  HSLBController hslb;
  hslb.open(0);

  while (true) {
    copper.monitor();
    printf("FIFO full               : %s\n", (copper.isFifoFull() ? "true" : "false"));
    printf("FIFO empty              : %s\n", (copper.isFifoEmpty() ? "true" : "false"));
    printf("Length FIFO full        : %s\n",
           (copper.isLengthFifoFull() ? "true" : "false"));
    printf("\n");

    hslb.monitor();
    printf("Belle2 link down        : %s\n", (hslb.isBelle2LinkDown() ? "true" : "false"));
    printf("COPPER fifo full        : %s\n", (hslb.isCOPPERFifoFull() ? "true" : "false"));
    printf("COPPER length fifo full : %s\n", (hslb.isCOPPERLengthFifoFull() ? "true" : "false"));
    printf("HSLB Fifo full          : %s\n", (hslb.isFifoFull() ? "true" : "false"));
    printf("HSLB CRC error          : %s\n", (hslb.isCRCError() ? "true" : "false"));
    printf("\n");

    ttrx.monitor();
    printf("Belle2 link error       : %s\n", (ttrx.isBelle2LinkError() ? "true" : "false"));
    printf("Link up error           : %s\n", (ttrx.isLinkUpError() ? "true" : "false"));
    printf("\n");
    sleep(1);
  }
  copper.close();
  return 0;
}

