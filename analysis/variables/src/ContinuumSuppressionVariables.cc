/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Pablo Goldenzweig, Dennis Weyland                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/variables/ContinuumSuppressionVariables.h>
#include <analysis/variables/ROEVariables.h>
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

#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>

#include <TLorentzVector.h>
#include <TVector3.h>

#include <cmath>


namespace Belle2 {
  namespace Variable {

    double R2EventLevel(const Particle*)
    {
      B2WARNING("The variable R2EventLevel is deprecated. Use `foxWolframR2` and ma.buildEventShape(inputListNames=[], default_cleanup=True, allMoments=False, cleoCones=True, collisionAxis=True, foxWolfram=True, harmonicMoments=True, jets=True, sphericity=True, thrust=True, checkForDuplicates=False, path=mypath)");

      std::vector<TVector3> p3_all;

      StoreArray<Track> tracks;
      for (int i = 0; i < tracks.getEntries(); ++i) {
        // deal with multiple possible track hypotheses in the track fit: try to
        // retrieve the most likely from PID, maybe this fit failed so then
        // create a particle with whatever is closest with a TrackFitResult
        Const::ParticleType mostLikely = tracks[i]->getRelated<PIDLikelihood>()->getMostLikely();
        const TrackFitResult* iTrack = tracks[i]->getTrackFitResultWithClosestMass(mostLikely);
        if (iTrack == nullptr) continue;
        if (iTrack->getChargeSign() != 0) {
          Particle particle(tracks[i], iTrack->getParticleType());
          PCmsLabTransform T;
          TLorentzVector p_cms = T.rotateLabToCms() * particle.get4Vector();
          p3_all.push_back(p_cms.Vect());
        }
      }

      StoreArray<ECLCluster> eclClusters;
      for (int i = 0; i < eclClusters.getEntries(); ++i) {
        // sum only ECLClusters which have the N1 (n photons) hypothesis
        if (!eclClusters[i]->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
          continue;

        ClusterUtils C;
        TLorentzVector momECLCluster = C.Get4MomentumFromCluster(eclClusters[i], ECLCluster::EHypothesisBit::c_nPhotons);
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
      FW.calculateBasicMoments();
      return FW.getR(2);
    }

    double R2(const Particle* particle)
    {
      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return std::numeric_limits<float>::quiet_NaN();

      return qq->getR2();
    }

    double thrustBm(const Particle* particle)
    {
      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return std::numeric_limits<float>::quiet_NaN();

      return qq->getThrustBm();
    }

    double thrustOm(const Particle* particle)
    {
      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return std::numeric_limits<float>::quiet_NaN();

      return qq->getThrustOm();
    }

    double cosTBTO(const Particle* particle)
    {
      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return std::numeric_limits<float>::quiet_NaN();

      return qq->getCosTBTO();
    }

    double cosTBz(const Particle* particle)
    {
      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return std::numeric_limits<float>::quiet_NaN();

      return qq->getCosTBz();
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

        // all possible names
        std::vector<std::string> names = {"mm2",   "et",
                                          "hso00", "hso01", "hso02", "hso03", "hso04",
                                          "hso10", "hso12", "hso14",
                                          "hso20", "hso22", "hso24",
                                          "hoo0",  "hoo1",  "hoo2",  "hoo3",  "hoo4"
                                         };

        // find the index of the name
        for (unsigned i = 0; i < names.size(); ++i) {
          if (variableName == names[i])
            index = i;
        }

        // throw helfpul error if name provided was not in allowed list
        if (index == -1) {
          std::string allowed = "";
          for (auto n : names)
            allowed += n + ", ";
          B2FATAL("Variable name provided: " << variableName << " is not one of the allowed options. Please choose from one of:" << allowed);
        }

        auto func = [index, useFS1](const Particle * particle) -> double {
          const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
          if (!qq) return std::numeric_limits<double>::quiet_NaN();

          // get the KSFW moments
          std::vector<float> ksfw = qq->getKsfwFS0();
          if (useFS1)
            ksfw = qq->getKsfwFS1();

          if (ksfw.size() == 0) B2FATAL("Could not find any KSFW moments");
          return ksfw.at(index);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function KSFWVariables. It only takes one or two arguments. The first argument must be the variable and the second can either be left blank or must be FS1 to use the KSFW moments calculated from the B final state particles.");
      }
    }

    Manager::FunctionPtr CleoConesCS(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1 || arguments.size() == 2) {

        int coneNumber = 0;
        try {
          coneNumber = Belle2::convertString<int>(arguments[0]);
        } catch (std::invalid_argument&) {
          B2FATAL("The first argument of the CleoConeCS meta function must be an integer!");
        }

        bool useROE = false;
        if (arguments.size() == 2) {
          if (arguments[1] == "ROE") {
            useROE = true;
          } else {
            B2FATAL("Second argument in CleoCones can only be 'ROE' to use the CleoCones calculated from the ROE only! Do not include a second argument to use the default CleoCones calculated from all final state particles.");
          }
        }

        auto func = [coneNumber, useROE](const Particle * particle) -> double {
          const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
          if (!qq)
            return std::numeric_limits<double>::quiet_NaN();
          std::vector<float> cleoCones = qq->getCleoConesALL();
          if (useROE)
            cleoCones = qq->getCleoConesROE();
          return cleoCones.at(coneNumber - 1);
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
        } catch (std::invalid_argument&) {
          B2FATAL("Second and third argument of transformedNetworkOutput meta function must be doubles!");
        }

        auto extraInfoName = arguments[0];
        auto func = [extraInfoName, low, high](const Particle * particle) -> double {
          if (particle == nullptr)
          {
            StoreObjPtr<EventExtraInfo> eventExtraInfo;
            if (eventExtraInfo->hasExtraInfo(extraInfoName)) {
              return eventExtraInfo->getExtraInfo(extraInfoName);
            } else {
              return std::numeric_limits<double>::quiet_NaN();
            }
          }
          if (particle->hasExtraInfo(extraInfoName))
          {
            return std::log(((particle->getExtraInfo(extraInfoName)) - low) / (high - (particle->getExtraInfo(extraInfoName))));
          } else {
            return std::numeric_limits<double>::quiet_NaN();
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function transformedNetworkOutput");
      }
    }

    Manager::FunctionPtr useBThrustFrame(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        auto variableName = arguments[0];
        std::string mode = arguments[1];

        const bool modeisSignal = mode == "Signal";
        const bool modeisAuto = mode == "Auto";

        if (not modeisSignal and (mode != "ROE") and not modeisAuto)
          B2FATAL("Second argument in useBThrustFrame can only be 'Signal', 'ROE' or 'Auto'. Your argument was " + mode);

        const Variable::Manager::Var* var = Manager::Instance().getVariable(variableName);

        auto func = [var, modeisSignal, modeisAuto](const Particle * particle) -> double {
          StoreObjPtr<RestOfEvent> roe("RestOfEvent");
          const Particle* Bparticle = roe->getRelated<Particle>();
          const ContinuumSuppression* qq = Bparticle->getRelatedTo<ContinuumSuppression>();
          if (!qq)
            return std::numeric_limits<double>::quiet_NaN();
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
        B2FATAL("Wrong number of arguments for meta function useBThrustFrame. It only takes two arguments. The first argument must be the variable and the second can either be .");
      }
    }


    VARIABLE_GROUP("Continuum Suppression");
    REGISTER_VARIABLE("R2EventLevel", R2EventLevel,
                      "[Eventbased] Event-Level Reduced Fox-Wolfram moment R2.");
    MAKE_DEPRECATED("R2EventLevel", false, "release-06-00-00", R"DOC(
                     The same value can be calculated with the Event Shape module, see  :b2:var:`foxWolframR`.)DOC");
    REGISTER_VARIABLE("R2"          , R2          , R"DOC(
Returns reduced Fox-Wolfram R2, defined as ratio of the i-th to the 0-th order Fox Wolfram moments.

.. warning:: You have to run the Contiunuum Suppression builder module for these variables to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
)DOC");
    REGISTER_VARIABLE("thrustBm"    , thrustBm    , R"DOC(
Returns magnitude of the signal B thrust axis.

.. warning:: You have to run the Contiunuum Suppression builder module for these variables to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
)DOC");
    REGISTER_VARIABLE("thrustOm"    , thrustOm    , R"DOC(
Returns magnitude of the ROE thrust axis.

.. warning:: You have to run the Contiunuum Suppression builder module for these variables to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
)DOC");
    REGISTER_VARIABLE("cosTBTO"     , cosTBTO     , R"DOC(
Returns cosine of angle between thrust axis of the signal B and thrust axis of ROE.

.. warning:: You have to run the Contiunuum Suppression builder module for these variables to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
)DOC");
    REGISTER_VARIABLE("cosTBz"      , cosTBz      , R"DOC(
Returns cosine of angle between thrust axis of the signal B and z-axis.

.. warning:: You have to run the Contiunuum Suppression builder module for these variables to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
)DOC");
    REGISTER_VARIABLE("KSFWVariables(variable,string)", KSFWVariables,  R"DOC(
Returns variable et, mm2, or one of the 16 KSFW moments. If only the ``variable`` argument is specified, the KSFW moment calculated from the B primary daughters is returned. 
If string is set to ``FS1``, the KSFW moment calculated from the B final state daughters is returned.
Allowed input values for ``variable`` argument are the following:

* mm2,   et
* hso00, hso01, hso02, hso03, hso04
* hso10, hso12, hso14
* hso20, hso22, hso24
* hoo0,  hoo1,  hoo2,  hoo3,  hoo4.

.. warning:: You have to run the Contiunuum Suppression builder module for these variables to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
)DOC");

    REGISTER_VARIABLE("CleoConeCS(integer,string)", CleoConesCS, R"DOC(
Returns i-th cleo cones from the continuum suppression. The allowed inputs for ``integer`` argument are integers from 1 to 9. 
If only the ``integer`` argument is specified, the CleoCones are calculated from all final state particles. 
The ``string`` argument is optional and the only allowed input value is 'ROE', which sets the CleoCones to be calculated only from ROE particles.

.. warning:: You have to run the Contiunuum Suppression builder module for these variables to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
)DOC");

    REGISTER_VARIABLE("transformedNetworkOutput(name, low, high)", transformedNetworkOutput, R"DOC(
Transforms the network output :math:`C \to C'` via: :math:`C'=\operatorname{log}((C-\mathrm{low})/(\mathrm{high}-C))`.
The arguments of the metavariable are the following:

* ``name`` is the `extraInfo` name, where the network output :math:`C` has been stored. If particle is not specified, event `extraInfo` is used instead;
* ``low``, ``high`` are floating point numbers.

Returns NaN, if the `extraInfo` has not been found.
)DOC");

    REGISTER_VARIABLE("useBThrustFrame(variable, mode)", useBThrustFrame,  R"DOC(
Returns the variable with respect to rotated coordinates, in which z lies on the specified thrust axis.
If mode is set to ``Signal`` it will use the thrust axis of the reconstructed B candidate, if mode is set to ROE it will use the ROE thrust axis.
If mode is set to ``Auto`` the function use the thrust axis based on Rest Of Event (ROE) particles.
Like :b2:var:`isInRestOfEvent`, one has to use this metavariable in ROE loop.

.. warning:: You have to run the Contiunuum Suppression builder module for these variables to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
)DOC");

  }
}
