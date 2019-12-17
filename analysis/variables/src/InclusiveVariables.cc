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

#include <mdst/dataobjects/MCParticle.h>

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
          B2WARNING("The meta variable nDaughterCharged accepts zero or one arguments!");
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
          } else if (abs(daughter->getCharge()) > 0) {
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
      auto fspDaughters = particle->getDaughters();
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
                      "of all charged daughters if no argument has been provided.");
    REGISTER_VARIABLE("nCompositeDaughters",   nCompositeDaughters,
                      "Returns the number of final state composite daughters.");
    REGISTER_VARIABLE("daughterAverageOf(variable)", daughterAverageOf,
                      "Returns the mean value of a variable over all daughters.")
  }
}
