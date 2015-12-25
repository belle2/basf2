#include <daq/dqm/analysis/HistMemory.h>

#include <daq/slc/system/LogFile.h>

#include <cstdio>
#include <cstring>
#include <iostream>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    printf("Usage : %s <path_to_tmapfile>\n", argv[0]);
    return 1;
  }
  HistMemory memory;
  memory.open(argv[1], 10000000);

  int count = 0;
  while (true) {
    std::vector<TH1*>& hist(memory.deserialize());
    if (hist.size() > 0) {
      for (size_t i = 0; i < hist.size(); i++) {
        LogFile::info("%s : %f", hist[i]->GetName(), hist[i]->GetEntries());
      }
    }
    sleep(1);
    count++;
  }
}
