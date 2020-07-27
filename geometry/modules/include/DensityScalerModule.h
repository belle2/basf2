/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <set>

namespace Belle2 {
  /**
  * Scale density of simulation/reconstruction geometry
  *
  * This is obviously only meant for systematic studies to scale the density in
  * different parts of the detector to estimate the effect on for example
  * tracking performance.
  */
  class DensityScalerModule : public Module {
  public:
    /**
    * Constructor: Sets the description, the properties and the parameters of the module.
    */
    DensityScalerModule();
    /** Set the scaling */
    virtual void initialize() override;
  private:
    /** The scale factor for each geometry component that should be scaled. "*"
     * can be used to scale all components by the same factor. If a component is
     * specified manually and "*" is present the result will be the product of
     * both */
    std::map<std::string, double> m_scaling;
    /** Set of material names to **not** scale */
    std::set<std::string> m_ignoredMaterials;
  };
}
