#include <assert.h>
#include <stdio.h>
#include "b2eb_format.h"

int
main()
{
  SendHeader h;
  h.exp(0x555);
  h.run(0xAAA);

  printf("%x\n", h.exp());
  printf("%x\n", h.run());

  h.nevent(1);
  h.nboard(2);

  uint32_t* lp = (unsigned int*)&h;

  assert(lp[2] == 0x10002);


  for (int i = 0; i < 4; i++) {
    printf("%d %x\n", i, lp[i]);
  }
}
