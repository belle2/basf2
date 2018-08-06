
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc, Thomas Keck                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
// Own include
#include <analysis/utility/MCMatching.h>
#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <vector>

namespace Belle2 {
  class Particle;

  namespace Variable {
    Manager::FunctionPtr flightTimeOfDaughterErr(const std::vector<std::string>& arguments);
    Manager::FunctionPtr flightDistanceOfDaughterErr(const std::vector<std::string>& arguments);
    Manager::FunctionPtr flightTimeOfDaughter(const std::vector<std::string>& arguments);
    Manager::FunctionPtr flightDistanceOfDaughter(const std::vector<std::string>& arguments);
    Manager::FunctionPtr flightDistanceOfGrandDaughter(const std::vector<std::string>& arguments);
  }
} //namespace Belle2
