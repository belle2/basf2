#include <daq/slc/system/FileReader.h>
#include <daq/slc/system/File.h>

#include <cstdlib>
#include <cstdio>

using namespace Belle2;

int main(int argc, char** argv)
{
  File file;
  file.open(argv[1]);
  int val;
  unsigned long long count = 0;
  try {
    while (true) {
      file.read(&val, sizeof(int));
      printf("%08x ", val);
      count++;
      if (count > 0 && count % 10 == 0) {
        printf("\n");
      }
    }
  } catch (const IOException& e) {

  }
  return 0;
}
