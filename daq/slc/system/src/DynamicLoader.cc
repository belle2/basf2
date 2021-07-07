/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/DynamicLoader.h"
#include "daq/slc/system/DynamicLoadException.h"

#include <dlfcn.h>

using namespace Belle2;

DynamicLoader::~DynamicLoader()
{
}

void DynamicLoader::close()
{
  if (m_handle != NULL) {
    dlclose(m_handle);
    m_handle = NULL;
  }
}

void* DynamicLoader::open(const std::string& lib_path)
{
  void* handle = dlopen(lib_path.c_str(), RTLD_LAZY);
  if (!handle) {
    throw (DynamicLoadException("dlopen: %s", dlerror()));
  }
  m_handle = handle;
  return handle;
}

void* DynamicLoader::load(const std::string& funcname)
{
  char* error = NULL;
  void* func = dlsym(m_handle, funcname.c_str());
  if ((error = dlerror()) != NULL) {
    throw (DynamicLoadException("dlopen: %s", error));
  }
  return func;
}
