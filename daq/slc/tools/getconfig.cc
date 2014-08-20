#include <daq/slc/base/ConfigFile.h>

#include <iostream>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) return 1;
  ConfigFile file("slowcontrol", argv[1]);
  for (int i = 2; i < argc; i++) {
    std::cout << file.get(argv[i]) << " " << std::endl;
  }
  return 0;
}
