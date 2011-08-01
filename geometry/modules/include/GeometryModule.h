/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOMETRYMODULE_H
#define GEOMETRYMODULE_H

#include <framework/core/Module.h>

namespace Belle2 {
  class GeometryModule: public Module {
  public:
    GeometryModule();
    ~GeometryModule() {};

    void beginRun();
  protected:
    std::string m_geometryPath;
    std::string m_geometryTypeString;
    int m_geometryType;
  };
} //Belle2 namespace
#endif
