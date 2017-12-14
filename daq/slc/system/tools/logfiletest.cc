#include <daq/slc/system/LogFile.h>

#include <unistd.h>

using namespace Belle2;

int main()
{
  LogFile::open("test");
  while (true) {
    LogFile::info("TESTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT");
    usleep(10000);
  }
}
