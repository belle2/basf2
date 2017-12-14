#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/copper/HSLB.h>

#include <daq/slc/copper/arich/HAPDFEE.h>

#include <cstdio>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    printf("usage: %s <config>\n", argv[0]);
    return 1;
  }

  DBObject obj = DBObjectLoader::load(argv[1]);
  HSLB hslb;
  //hslb.open(0);
  HAPDFEE().load(hslb, obj);
  return 0;
}
