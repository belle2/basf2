#include <daq/slc/base/ConfigFile.h>

#include <iostream>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) return 1;
  std::cout << ConfigFile("slowcontrol", argv[1]).get(argv[2]) << std::endl;
  return 0;
}
