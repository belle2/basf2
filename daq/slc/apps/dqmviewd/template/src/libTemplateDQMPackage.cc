#include "daq/slc/apps/dqmviewd/template/TemplateDQMPackage.h"

extern "C" void* createTemplateDQMPackage(const char* name)
{
  return new Belle2::TemplateDQMPackage(name);
}
