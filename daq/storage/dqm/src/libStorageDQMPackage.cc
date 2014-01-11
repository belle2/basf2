#include "daq/storage/dqm/StorageDQMPackage.h"

extern "C" void* createStorageDQMPackage(const char* name,
                                         const char* filename)
{
  return new Belle2::StorageDQMPackage(name, filename);
}
