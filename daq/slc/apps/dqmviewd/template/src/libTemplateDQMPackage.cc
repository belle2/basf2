#include "daq/slc/apps/dqmviewd/template/TemplateDQMPackage.h"

extern "C" void* createTemplateDQMPackage(const char* name,
                                          const char* filename)
{
  return new Belle2::TemplateDQMPackage(name, filename);
}
