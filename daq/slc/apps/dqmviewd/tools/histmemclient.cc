#include <daq/dqm/DqmMemFile.h>

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
  DqmMemFile* memory = new DqmMemFile(argv[1]);

  int count = 0;
  while (true) {
    TMemFile* file = memory->LoadMemFile();
    TIter next(file->GetListOfKeys());
    TKey* key = NULL;
    while ((key = (TKey*)next())) {
      TH1* hist = (TH1*)key->ReadObj();
      LogFile::info("%s : %f", hist->GetName(), hist->GetEntries());
    }
    sleep(1);
    count++;
  }
}
