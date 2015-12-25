#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include "b2eb_format.h"
#include "onsen_tcp.h"

#include "combined_event_from_eb2.h"

int
main(int argc, char** argv)
{
  combined_event_t eb2(stdin);

  while (1) {
    eb2.fetch();
    eb2.fwrite_onsen_body(stdout);
  }
}
