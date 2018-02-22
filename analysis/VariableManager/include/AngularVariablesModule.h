/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hikari Hirata                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

// include VariableManager
#include <analysis/VariableManager/Manager.h>

// add include statement for the Particle class
#include <analysis/dataobjects/Particle.h>

namespace Belle2 {
  namespace Variable {

    double helicityAngle(const Particle* particle, const std::vector<double>& daughter_indices);


  } // Variable namespace
}
