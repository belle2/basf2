/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_DynamicLoader_h
#define _Belle2_DynamicLoader_h

#include <string>

namespace Belle2 {

  class DynamicLoader {

  public:
    DynamicLoader() {}
    DynamicLoader(const std::string& path)
    {
      open(path);
    }
    ~DynamicLoader();

  public:
    void* open(const std::string& lib_path);
    void* load(const std::string& func_name);
    void close();

  private:
    void* m_handle;

  };

}

#endif
