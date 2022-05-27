/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/variables/InclusiveVariables.h>

#include <analysis/dataobjects/Particle.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Conversion.h>

using namespace std;

namespace Belle2 {
  namespace Variable {

    int nDaughterPhotons(const Particle* particle)
    {
      int result = 0;
      auto fspDaughters = particle->getFinalStateDaughters();
      for (auto* daughter : fspDaughters) {
        if (abs(daughter->getPDGCode()) == Const::photon.getPDGCode()) {
          result++;
        }
      }
      return result;
    }

    int nDaughterNeutralHadrons(const Particle* particle)
    {
      int result = 0;
      auto fspDaughters = particle->getFinalStateDaughters();
      for (auto* daughter : fspDaughters) {
        if (abs(daughter->getPDGCode()) == Const::neutron.getPDGCode()
            or abs(daughter->getPDGCode()) == Const::Klong.getPDGCode()) {
          result++;
        }
      }
      return result;
    }

    int nDaughterCharged(const Particle* particle, const std::vector<double>& argument)
    {
      int absPDGCode = 0;
      if (argument.size() == 1) {
        absPDGCode = abs(std::lround(argument[0]));
      }

      int result = 0;
      auto fspDaughters = particle->getFinalStateDaughters();
      for (auto* daughter : fspDaughters) {
        if (absPDGCode != 0) {
          if (abs(daughter->getPDGCode()) == absPDGCode) {
            result++;
          }
        } else if (abs(daughter->getCharge()) > 0) {
          result++;
        }
      }
      return result;
    }

    int nCompositeDaughters(const Particle* particle, const std::vector<double>& argument)
    {
      int absPDGCode = 0;
      if (argument.size() == 1) {
        absPDGCode = abs(std::lround(argument[0]));
      }

      int result = 0;
      auto primaryDaughters = particle->getDaughters();
      for (auto* daughter : primaryDaughters) {
        if (daughter->getParticleSource() == Particle::EParticleSourceObject::c_Composite or
            daughter->getParticleSource() == Particle::EParticleSourceObject::c_V0) {
          if (absPDGCode != 0) {
            if (abs(daughter->getPDGCode()) == absPDGCode) {
              result++;
            }
          } else {
            result++;
          }
        }
      }
      return result;
    }

    int nCompositeAllGenerationDaughters(const Particle* particle, const std::vector<double>& argument)
    {
      int absPDGCode = 0;
      if (argument.size() == 1) {
        absPDGCode = abs(std::lround(argument[0]));
      }

      int result = 0;
      auto allDaughters = particle->getAllDaughters();
      for (auto* daughter : allDaughters) {
        if (daughter->getParticleSource() == Particle::EParticleSourceObject::c_Composite or
            daughter->getParticleSource() == Particle::EParticleSourceObject::c_V0) {
          if (absPDGCode != 0) {
            if (abs(daughter->getPDGCode()) == absPDGCode) {
              result++;
            }
          } else {
            result++;
          }
        }
      }
      return result;
    }

    Manager::FunctionPtr daughterAverageOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {
          double sum = 0.0;
          if (particle->getNDaughters() == 0)
          {
            return std::numeric_limits<double>::quiet_NaN();
          }
          if (std::holds_alternative<double>(var->function(particle->getDaughter(0))))
          {
            for (unsigned j = 0; j < particle->getNDaughters(); ++j) {
              sum += std::get<double>(var->function(particle->getDaughter(j)));
            }
          } else if (std::holds_alternative<int>(var->function(particle->getDaughter(0))))
          {
            for (unsigned j = 0; j < particle->getNDaughters(); ++j) {
              sum += std::get<int>(var->function(particle->getDaughter(j)));
            }
          }
          return sum / particle->getNDaughters();
        };
        return func;
      } else {
        B2FATAL("The meta variable daughterAverageOf requires only one argument!");
      }
    }

    // ---

    VARIABLE_GROUP("For fully-inclusive particles");

    REGISTER_VARIABLE("nDaughterPhotons",   nDaughterPhotons,
                      "Returns the number of final state daughter photons.");
    REGISTER_VARIABLE("nDaughterNeutralHadrons",   nDaughterNeutralHadrons,
                      "Returns the number of K_L0 or neutrons among the final state daughters.");
    REGISTER_VARIABLE("nDaughterCharged(pdg)",   nDaughterCharged,
                      "Returns the number of charged daughters with the provided PDG code or the number "
                      "of all charged daughters if no argument has been provided. "
                      "The variable is flavor agnostic and it returns the sum of the number of particle and anti-particle.");
    REGISTER_VARIABLE("nCompositeDaughters(pdg)",   nCompositeDaughters,
                      "Returns the number of primary composite daughters with the provided PDG code or the number"
                      "of all primary composite daughters if no argument has been provided. "
                      "The variable is flavor agnostic and it returns the sum of the number of particle and anti-particle.");
    REGISTER_VARIABLE("nCompositeAllGenerationDaughters(pdg)",   nCompositeAllGenerationDaughters,
                      "Returns the number of all generations' composite daughters with the provided PDG code or the number"
                      "of all generations' composite daughters if no argument has been provided. "
                      "The variable is flavor agnostic and it returns the sum of the number of particle and anti-particle.");
    REGISTER_METAVARIABLE("daughterAverageOf(variable)", daughterAverageOf,
                          "Returns the mean value of a variable over all daughters.", Manager::VariableDataType::c_double)
  }
}
