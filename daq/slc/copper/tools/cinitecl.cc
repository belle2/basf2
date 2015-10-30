#include "daq/slc/copper/ecl/ECLFEE.h"
#include "daq/slc/database/DBObject.h"

using namespace Belle2;

int main()
{
  for (int i = 0; i < 4; i++) {
    HSLB hslb;
    hslb.open(i);
    ECLFEE fee;
    DBObject obj;
    fee.boot(hslb, obj);
    fee.load(hslb, obj);
  }
  return 0;
}
