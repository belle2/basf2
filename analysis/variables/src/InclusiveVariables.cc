/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sviat Bilokin                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/variables/InclusiveVariables.h>
#include <analysis/VariableManager/Manager.h>

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Conversion.h>

#include <boost/lexical_cast.hpp>
using namespace std;

namespace Belle2 {
  namespace Variable {

    double nDaughterPhotons(const Particle* particle)
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

    double nDaughterNeutralHadrons(const Particle* particle)
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

    Manager::FunctionPtr nDaughterCharged(const std::vector<std::string>& arguments)
    {

      int pdgCode = 0;
      if (arguments.size() == 1) {
        try {
          pdgCode = Belle2::convertString<int>(arguments[0]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("The first argument of  meta function must be a positive integer!");
          return nullptr;
        }
      }
      auto func = [pdgCode](const Particle * particle) -> double {
        int result = 0;
        auto fspDaughters = particle->getFinalStateDaughters();
        for (auto* daughter : fspDaughters)
        {
          if (pdgCode != 0) {
            if (abs(daughter->getPDGCode()) == pdgCode) {
              result++;
            }
          } else if (daughter->getParticleType() == Particle::EParticleType::c_Track) {
            result++;
          }
        }
        return result;
      };
      return func;
    }

    double nCompositeDaughters(const Particle* particle)
    {
      int result = 0;
      auto fspDaughters = particle->getFinalStateDaughters();
      for (auto* daughter : fspDaughters) {
        if (daughter->getParticleType() == Particle::EParticleType::c_Composite) {
          result++;
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
          for (unsigned j = 0; j < particle->getNDaughters(); ++j)
          {
            sum += var->function(particle->getDaughter(j));
          }
          return sum / particle->getNDaughters();
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function daughterAverageOf");
      }
    }



    // ---

    VARIABLE_GROUP("For fully-inclusive particles");

    REGISTER_VARIABLE("nDaughterPhotons",   nDaughterPhotons,
                      "Returns number of final state daugher photons.");
    REGISTER_VARIABLE("nDaughterNeutralHadrons",   nDaughterNeutralHadrons,
                      "Returns number of KL_0 or neutrons among the final state daughers.");
    REGISTER_VARIABLE("nDaughterCharged(pdg)",   nDaughterCharged,
                      "Returns number of charged daughers with PDG code or number of all charged daughters if no argument has been provided.");
    REGISTER_VARIABLE("nCompositeDaughters",   nCompositeDaughters,
                      "Returns number of final state composite daughers.");
    REGISTER_VARIABLE("daughterAverageOf(variable)", daughterAverageOf,
                      "Returns average of a variable over all daughters.")
  }
}
