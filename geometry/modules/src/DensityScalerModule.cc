/**************************************************************************
    * BASF2 (Belle Analysis Framework 2)                                     *
    * Copyright(C) 2019 - Belle II Collaboration                             *
    *                                                                        *
    * Author: The Belle II Collaboration                                     *
    * Contributors: Martin Ritter                                            *
    *                                                                        *
    * This software is provided "as is" without any warranty.                *
    **************************************************************************/

#include <geometry/modules/DensityScalerModule.h>
#include <geometry/GeometryManager.h>
#include <geometry/Materials.h>
// we want std::set parameters
#include <framework/core/ModuleParam.templateDetails.h>

using namespace Belle2;

REG_MODULE(DensityScaler)

DensityScalerModule::DensityScalerModule() : Module()
{
  // Set module properties
  setDescription(R"DOC("Scale density of simulation/reconstruction geometry

This is obviously only meant for systematic studies to scale the density in different
parts of the detector to estimate the effect on for example trackinrmance.

This module is to be placed **before** the Geometry module itself and will then
change the density of all materials which are created using the Materials service

Warning:
  This doesn't affect any materials created manually somewhere. A way to check is
  to set a global scaling factor and then do a material scan with the :b2:mod:`MaterialScan`
  module split by materials. The scaled materials will have a different name
  starting with 'scaled:' followed by the scaling factor and the original name
)DOC");

  // Parameter definitions
  addParam("scaling", m_scaling,
           "The scale factor for each geometry component that should be scaled. '*' "
           "can be used to scale all components by the same factor. If a component is "
           "specified manually and '*' is present the result will be the product of both)");
  addParam("ignoredMaterials", m_ignoredMaterials, "A set of material names which should "
           "**not** be scaled, usually things like vaccum or air. Set to empty set to scale everything",
           geometry::Materials::getInstance().getDensityScaleIgnoredMaterials());
}

void DensityScalerModule::initialize()
{
  geometry::Materials::getInstance().setDensityScaleIgnoredMaterials(m_ignoredMaterials);
  if(geometry::GeometryManager::getInstance().getTopVolume()) {
    B2FATAL("Geometry already created, DensityScaler needs to come before the Geometry module");
  }
  geometry::GeometryManager::getInstance().setDensityScaling(m_scaling);
}
