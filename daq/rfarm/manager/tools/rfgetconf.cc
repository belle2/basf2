//+
// File : rfgetconf.cc
// Description : Retrieve configuration for given item
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 27 - Mar - 2015
//-

#include <cstdlib>
#include <stdio.h>
#include <string.h>

#include "daq/rfarm/manager/RFConf.h"

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  // Usage : getconf conffile arg1 arg2 arg3

  if (argc < 3) {
    printf("Usage : rfgetconf configfile arg1 arg2 arg3\n");
    exit(-1);
  }

  RFConf conf(argv[1]);

  if (argc == 3 || strcmp(argv[3], "NULL") == 0)
    printf("%s", conf.getconf(argv[2]));
  else if (argc == 4 || strcmp(argv[4], "NULL") == 0)
    printf("%s", conf.getconf(argv[2], argv[3]));
  else
    printf("%s", conf.getconf(argv[2], argv[3], argv[4]));

  exit(0);
}

