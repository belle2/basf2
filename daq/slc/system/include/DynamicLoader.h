#ifndef _Belle2_DynamicLoader_h
#define _Belle2_DynamicLoader_h

#include "daq/slc/system/DynamicLoadException.h"

namespace Belle2 {

  class DynamicLoader {

  public:
    DynamicLoader() {}
    ~DynamicLoader() throw();

  public:
    void* open(const std::string& lib_path)
    throw(DynamicLoadException);
    void* load(const std::string& func_name)
    throw(DynamicLoadException);
    void close() throw();

  private:
    void* m_handle;

  };

}

#endif
