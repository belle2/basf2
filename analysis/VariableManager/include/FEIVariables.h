#pragma once
// include VariableManager
#include <analysis/VariableManager/Manager.h>

// include the Belle II Particle class
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/StoreArray.h>
#include <framework/utilities/Conversion.h>
#include <framework/logging/Logger.h>
#include <boost/algorithm/string.hpp>
#include <analysis/VariableManager/Manager.h>
#include <map>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <limits>

// put variable in the Belle2::Variable namespace
namespace Belle2 {
  namespace Variable {
    // Your code goes here
    double mostcommonBTagIndex(const Particle* part);
    int finddescendant(const MCParticle* mcpart);
    std::vector<double> truthFSPTag(double BTag_index);
    Manager::FunctionPtr percentageMissingParticlesBTag(const std::vector<std::string>&  arguments);
    Manager::FunctionPtr percentageWrongParticlesBTag(const std::vector<std::string>&  arguments);
  } // Variable namespace
} // Belle2 namespace
