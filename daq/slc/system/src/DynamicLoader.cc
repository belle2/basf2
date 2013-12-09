#include "daq/slc/system/DynamicLoader.h"

#include <daq/slc/base/StringUtil.h>

#include <dlfcn.h>

using namespace Belle2;

DynamicLoader::~DynamicLoader() throw()
{
}

void DynamicLoader::close() throw()
{
  if (_handle != NULL) {
    dlclose(_handle);
    _handle = NULL;
  }
}

void* DynamicLoader::open(const std::string& lib_path)
throw(DynamicLoadException)
{
  void* handle = dlopen(lib_path.c_str(), RTLD_LAZY);
  if (!handle) {
    throw (DynamicLoadException(__FILE__, __LINE__,
                                Belle2::form("dlopen: %s", dlerror())));
  }
  _handle = handle;
  return handle;
}

void* DynamicLoader::load(const std::string& funcname)
throw(DynamicLoadException)
{
  char* error = NULL;
  void* func = dlsym(_handle, funcname.c_str());
  if ((error = dlerror()) != NULL) {
    throw (DynamicLoadException(__FILE__, __LINE__,
                                Belle2::form("dlopen: %s", error)));
  }
  return func;
}
