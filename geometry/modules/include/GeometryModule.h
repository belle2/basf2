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
  /** Module to faciliate Geometry creation. */
  class GeometryModule: public Module {
  public:
    /** Constructor */
    GeometryModule();
    /** Destructor */
    ~GeometryModule() {};
    /** Create geometry */
    void initialize();
  protected:
    /** Path for the geometry in the parameter space */
    std::string m_geometryPath;
    /** Type of the geometry to be built */
    int m_geometryType;
    /** Components to be created, if empty all components will be created */
    std::vector<std::string> m_components;
    /** Components to excluded from creation */
    std::vector<std::string> m_excluded;
    /** Components to be added in addition to the default ones */
    std::vector<std::string> m_additional;

    /** Backwards compatibility with the old parameter Components */
    std::vector<std::string> m_componentsOld;
    /** Backwards compatibility with the old parameter ExcludedComponents */
    std::vector<std::string> m_excludedOld;
  };
} //Belle2 namespace
#endif
