/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Pablo Goldenzweig                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VariableManager/ContinuumSuppressionVariables.h>
#include <analysis/VariableManager/ParameterVariables.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ContinuumSuppression.h>
#include <analysis/utility/ReferenceFrame.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/utilities/Conversion.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <TLorentzVector.h>
#include <TVectorF.h>
#include <TVector3.h>

#include <cmath>

#include <boost/lexical_cast.hpp>


namespace Belle2 {
  namespace Variable {

    double R2(const Particle* particle)
    {
      double result = -1.0;

      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return result;

      result = qq->getR2();

      return result;
    }

    double thrustBm(const Particle* particle)
    {
      double result = -1.0;

      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return result;

      result = qq->getThrustBm();

      return result;
    }

    double thrustOm(const Particle* particle)
    {
      double result = -1.0;

      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return result;

      result = qq->getThrustOm();

      return result;
    }

    double cosTBTO(const Particle* particle)
    {
      double result = -1.0;

      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return result;

      result = qq->getCosTBTO();

      return result;
    }

    double cosTBz(const Particle* particle)
    {
      double result = -1.0;

      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return result;

      result = qq->getCosTBz();

      return result;
    }

    Manager::FunctionPtr KSFWVariables(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1 || arguments.size() == 2) {
        bool useFS1 = false;
        auto variableName = arguments[0];
        if (arguments.size() == 2) {
          if (arguments[1] == "FS1") {
            useFS1 = true;
          } else {
            B2FATAL("Second argument in KSFWVariables can only be 'FS1' to use the KSFW moments calculated from the B final state particles! Do not include a second argument to use the default KSFW moments calculated from the B primary daughters.");
          }
        }
        int index = -1;

        std::vector<std::string> names = {"mm2",   "et",
                                          "hso00", "hso01", "hso02", "hso03", "hso04",
                                          "hso10", "hso12", "hso14",
                                          "hso20", "hso22", "hso24",
                                          "hoo0",  "hoo1",  "hoo2",  "hoo3",  "hoo4"
                                         };

        for (unsigned i = 0; i < names.size(); ++i) {
          if (variableName == names[i])
            index = i;
        }

        auto func = [index, useFS1](const Particle * particle) -> double {
          const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
          std::vector<float> ksfw = qq->getKsfwFS0();
          if (useFS1)
            ksfw = qq->getKsfwFS1();
          return ksfw.at(index);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function KSFWVariables. It only takes one or two arguments. The first argument must be the variable and the second can either be left blank or must be FS1 to use the KSFW moments calculated from the B final state particles.");
      }
    }

    double CleoCones(const Particle* particle, const std::vector<double>& cone)
    {
      if (!particle)
        return -999;

      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return -999;

      const auto& cleoCones = qq->getCleoCones();
      return cleoCones.at(cone[0] - 1);
    }

    Manager::FunctionPtr transformedNetworkOutput(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 3) {
        double low = 0;
        double high = 0;
        try {
          low  = Belle2::convertString<double>(arguments[1]);
          high = Belle2::convertString<double>(arguments[2]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("Second and third argument of transformedNetworkOutput meta function must be doubles!");
          return nullptr;
        }
        auto extraInfoName = arguments[0];
        auto func = [extraInfoName, low, high](const Particle * particle) -> double {
          if (particle == nullptr)
          {
            StoreObjPtr<EventExtraInfo> eventExtraInfo;
            return eventExtraInfo->getExtraInfo(extraInfoName);
          }
          return std::log(((particle->getExtraInfo(extraInfoName)) - low) / (high - (particle->getExtraInfo(extraInfoName))));
        };
        return func;
      } else {
        B2WARNING("Wrong number of arguments for meta function transformedNetworkOutput");
        return nullptr;
      }
    }


    VARIABLE_GROUP("Continuum Suppression");
    REGISTER_VARIABLE("R2"       , R2      , "Reduced Fox-Wolfram moment R2");
    REGISTER_VARIABLE("thrustBm" , thrustBm, "Magnitude of the B thrust axis");
    REGISTER_VARIABLE("thrustOm" , thrustOm, "Magnitude of the ROE thrust axis");
    REGISTER_VARIABLE("cosTBTO"  , cosTBTO , "Cosine of angle between thrust axis of B and thrust axis of ROE");
    REGISTER_VARIABLE("cosTBz"   , cosTBz  , "Cosine of angle between thrust axis of B and z-axis");
    REGISTER_VARIABLE("KSFWVariables(variable,finalState)", KSFWVariables,
                      "Returns et, mm2, or one of the 16 KSFW moments. If only the variable is specified, the KSFW moment calculated from the B primary daughters is returned. If finalState is set to FS1, the KSFW moment calculated from the B final state daughters is returned.");
    REGISTER_VARIABLE("CleoCone(integer i)", CleoCones,
                      "Returns i-th cleo cones.\n"
                      "Useful for ContinuumSuppression.\n"
                      "Given particle needs a related ContinuumSuppression object (built using the ContinuumSuppressionBuilder).\n"
                      "Returns -999 if particle is nullptr or if particle has no related ContinuumSuppression object.");
    REGISTER_VARIABLE("transformedNetworkOutput(name, low, high)", transformedNetworkOutput,
                      "Transforms the network output C->C' via: C'=log((C-low)/(high-C))");

  }
}
