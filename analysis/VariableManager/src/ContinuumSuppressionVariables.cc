/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Pablo Goldenzweig, Dennis Weyland                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VariableManager/ContinuumSuppressionVariables.h>
#include <analysis/VariableManager/ParameterVariables.h>
#include <analysis/VariableManager/ROEVariables.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ContinuumSuppression.h>
#include <analysis/utility/ReferenceFrame.h>
#include <analysis/ClusterUtility/ClusterUtils.h>
#include <analysis/ContinuumSuppression/FoxWolfram.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/utilities/Conversion.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>

#include <TLorentzVector.h>
#include <TVectorF.h>
#include <TVector3.h>

#include <cmath>

#include <boost/lexical_cast.hpp>


namespace Belle2 {
  namespace Variable {

    double R2EventLevel(const Particle*)
    {
      std::vector<TVector3> p3_all;

      StoreArray<Track> tracks;
      for (int i = 0; i < tracks.getEntries(); ++i) {
        const TrackFitResult* iTrack = tracks[i]->getTrackFitResultWithClosestMass(tracks[i]->getRelated<PIDLikelihood>()->getMostLikely());
        if (iTrack == nullptr) continue;
        if (iTrack->getChargeSign() != 0) {
          Particle particle(tracks[i], Const::pion);
          PCmsLabTransform T;
          TLorentzVector p_cms = T.rotateLabToCms() * particle.get4Vector();
          p3_all.push_back(p_cms.Vect());
        }
      }

      StoreArray<ECLCluster> eclClusters;
      for (int i = 0; i < eclClusters.getEntries(); ++i) {
        // sum only momentum of T1 (1) and N1 (5) ECLClusters
        // other clusters are duplicates
        if (eclClusters[i]->getHypothesisId() != 1 &&
            eclClusters[i]->getHypothesisId() != 5)
          continue;

        ClusterUtils C;
        TLorentzVector momECLCluster = C.Get4MomentumFromCluster(eclClusters[i]);
        if (momECLCluster == momECLCluster) {
          if (eclClusters[i]->isNeutral()) {
            Particle particle(eclClusters[i]);
            PCmsLabTransform T;
            TLorentzVector p_cms = T.rotateLabToCms() * particle.get4Vector();
            p3_all.push_back(p_cms.Vect());
          }
        }
      }

      FoxWolfram FW(p3_all);
      return FW.R(2);
    }

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

    Manager::FunctionPtr CleoCones(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1 || arguments.size() == 2) {
        bool useROE = false;
        auto coneNumber = arguments[0];
        if (arguments.size() == 2) {
          if (arguments[1] == "ROE") {
            useROE = true;
          } else {
            B2FATAL("Second argument in CleoCones can only be 'ROE' to use the CleoCones calculated from the ROE only! Do not include a second argument to use the default CleoCones calculated from all final state particles.");
          }
        }

        auto func = [coneNumber, useROE](const Particle * particle) -> double {
          const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
          std::vector<float> cleoCones = qq->getCleoConesALL();
          if (useROE)
            cleoCones = qq->getCleoConesROE();
          return cleoCones.at(stoi(coneNumber) - 1);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for CleoCones function. It only takes one or two arguments. The first argument must be the cone number and the second can either be left blank or must 'ROE' to use the CleoCones calculated from all final state particles.");
      }
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

    Manager::FunctionPtr useThrustFrame(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        auto variableName = arguments[0];
        if (arguments[1] != "Signal" and arguments[1] != "ROE" and arguments[1] !=  "Auto")
          B2FATAL("Second argument in useThrustFrame can only be 'Signal', 'ROE' or 'Auto'. Your argument was " + arguments[1]);

        std::string mode = arguments[1];
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);

        const bool modeisSignal = mode == "Signal";
        const bool modeisAuto = mode == "Auto";

        auto func = [var, modeisSignal, modeisAuto](const Particle * particle) -> double {
          StoreObjPtr<RestOfEvent> roe("RestOfEvent");
          const Particle* Bparticle = roe->getRelated<Particle>();
          const ContinuumSuppression* qq = Bparticle->getRelatedTo<ContinuumSuppression>();
          double isinROE = isInRestOfEvent(particle);
          TVector3 newZ;
          if (modeisSignal or (modeisAuto and isinROE < 0.5))
            newZ = qq->getThrustB();
          else
            newZ = qq->getThrustO();

          TVector3 newY(0, 0, 0);
          if (newZ(2) == 0 and newZ(1) == 0)
            newY(0) = 1;
          else{
            newY(1) = newZ(2);
            newY(2) = -newZ(1);
          }
          TVector3 newX = newY.Cross(newZ);

          UseReferenceFrame<CMSRotationFrame> signalframe(newX, newY, newZ);

          return var->function(particle);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function useThrustFrame. It only takes two arguments. The first argument must be the variable and the second can either be .");
      }
    }


    VARIABLE_GROUP("Continuum Suppression");
    REGISTER_VARIABLE("R2EventLevel", R2EventLevel, "Event-Level Reduced Fox-Wolfram moment R2");
    REGISTER_VARIABLE("R2"          , R2          , "Reduced Fox-Wolfram moment R2");
    REGISTER_VARIABLE("thrustBm"    , thrustBm    , "Magnitude of the B thrust axis");
    REGISTER_VARIABLE("thrustOm"    , thrustOm    , "Magnitude of the ROE thrust axis");
    REGISTER_VARIABLE("cosTBTO"     , cosTBTO     , "Cosine of angle between thrust axis of B and thrust axis of ROE");
    REGISTER_VARIABLE("cosTBz"      , cosTBz      , "Cosine of angle between thrust axis of B and z-axis");
    REGISTER_VARIABLE("KSFWVariables(variable,string)", KSFWVariables,
                      "Returns variable et, mm2, or one of the 16 KSFW moments. If only the variable is specified, the KSFW moment calculated from the B primary daughters is returned. If string is set to FS1, the KSFW moment calculated from the B final state daughters is returned.");
    REGISTER_VARIABLE("CleoCone(integer,string)", CleoCones,
                      "Returns i-th cleo cones. If only the variable is specified, the CleoCones are calculated from all final state particles. If string is set to 'ROE', the CleoCones are calculated only from ROE particles.\n"
                      "Useful for ContinuumSuppression.\n"
                      "Given particle needs a related ContinuumSuppression object (built using the ContinuumSuppressionBuilder).\n"
                      "Returns -999 if particle is nullptr or if particle has no related ContinuumSuppression object.");
    REGISTER_VARIABLE("transformedNetworkOutput(name, low, high)", transformedNetworkOutput,
                      "Transforms the network output C->C' via: C'=log((C-low)/(high-C))");
    REGISTER_VARIABLE("useThrustFrame(variable, mode)", useThrustFrame,
                      "Returns the variable in respect to rotated coordinates, in which z lies on the specified thrust axis.\n"
                      "If mode is set to Signal it will use the thrust axis of the reconstructed B candidate, if mode is set to ROE it will use the ROE thrust axis.\n"
                      "If mode is set to Auto the function use the thrust axis based on isInRestOfEvent(particle).\n"
                      "Like isinRestofEvent, you have to use path.for_each( . . .) to use this MetaVariable.")

  }
}
