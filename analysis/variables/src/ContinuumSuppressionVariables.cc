/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

#include <cmath>


namespace Belle2 {
  namespace Variable {

    Manager::FunctionPtr R2WithMask(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("An empty argument is not allowed for the variable R2."
                "Either provide no argument or a valid mask name.");
      std::string maskName = arguments[0];
      auto func = [maskName](const Particle * particle) -> double {
        const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>(maskName);
        if (!qq)
          return std::numeric_limits<float>::quiet_NaN();

        return qq->getR2();
      };
      return func;
    }

    double R2(const Particle* particle)
    {
      RelationVector<ContinuumSuppression> continuumSuppressionRelations = particle->getRelationsTo<ContinuumSuppression>("ALL");
      if (continuumSuppressionRelations.size() == 1) {
        const ContinuumSuppression* qq = continuumSuppressionRelations[0];
        return qq->getR2();
      } else {
        if (continuumSuppressionRelations.size() > 1) {
          B2ERROR("The return value of R2 is ambiguous. Please provide the mask name as argument.");
        }
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    Manager::FunctionPtr thrustBmWithMask(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("An empty argument is not allowed for the variable thrustBm."
                "Either provide no argument or a valid mask name.");
      std::string maskName = arguments[0];
      auto func = [maskName](const Particle * particle) -> double {
        const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>(maskName);
        if (!qq)
          return std::numeric_limits<float>::quiet_NaN();

        return qq->getThrustBm();
      };
      return func;
    }

    double thrustBm(const Particle* particle)
    {
      RelationVector<ContinuumSuppression> continuumSuppressionRelations = particle->getRelationsTo<ContinuumSuppression>("ALL");
      if (continuumSuppressionRelations.size() == 1) {
        const ContinuumSuppression* qq = continuumSuppressionRelations[0];
        return qq->getThrustBm();
      } else {
        if (continuumSuppressionRelations.size() > 1) {
          B2ERROR("The return value of thrustBm is ambiguous. Please provide the mask name as argument.");
        }
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    Manager::FunctionPtr thrustOmWithMask(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("An empty argument is not allowed for the variable thrustOm."
                "Either provide no argument or a valid mask name.");
      std::string maskName = arguments[0];
      auto func = [maskName](const Particle * particle) -> double {
        const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>(maskName);
        if (!qq)
          return std::numeric_limits<float>::quiet_NaN();

        return qq->getThrustOm();
      };
      return func;
    }

    double thrustOm(const Particle* particle)
    {
      RelationVector<ContinuumSuppression> continuumSuppressionRelations = particle->getRelationsTo<ContinuumSuppression>("ALL");
      if (continuumSuppressionRelations.size() == 1) {
        const ContinuumSuppression* qq = continuumSuppressionRelations[0];
        return qq->getThrustOm();
      } else {
        if (continuumSuppressionRelations.size() > 1) {
          B2ERROR("The return value of thrustOm is ambiguous. Please provide the mask name as argument.");
        }
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    Manager::FunctionPtr cosTBTOWithMask(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("An empty argument is not allowed for the variable cosTBTO."
                "Either provide no argument or a valid mask name.");
      std::string maskName = arguments[0];
      auto func = [maskName](const Particle * particle) -> double {
        const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>(maskName);
        if (!qq)
          return std::numeric_limits<float>::quiet_NaN();

        return qq->getCosTBTO();
      };
      return func;
    }

    double cosTBTO(const Particle* particle)
    {
      RelationVector<ContinuumSuppression> continuumSuppressionRelations = particle->getRelationsTo<ContinuumSuppression>("ALL");
      if (continuumSuppressionRelations.size() == 1) {
        const ContinuumSuppression* qq = continuumSuppressionRelations[0];
        return qq->getCosTBTO();
      } else {
        if (continuumSuppressionRelations.size() > 1) {
          B2ERROR("The return value of cosTBTO is ambiguous. Please provide the mask name as argument.");
        }
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    Manager::FunctionPtr cosTBzWithMask(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("An empty argument is not allowed for the variable cosTBz."
                "Either provide no argument or a valid mask name.");
      std::string maskName = arguments[0];
      auto func = [maskName](const Particle * particle) -> double {
        const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>(maskName);
        if (!qq)
          return std::numeric_limits<float>::quiet_NaN();

        return qq->getCosTBz();
      };
      return func;
    }

    double cosTBz(const Particle* particle)
    {
      RelationVector<ContinuumSuppression> continuumSuppressionRelations = particle->getRelationsTo<ContinuumSuppression>("ALL");
      if (continuumSuppressionRelations.size() == 1) {
        const ContinuumSuppression* qq = continuumSuppressionRelations[0];
        return qq->getCosTBz();
      } else {
        if (continuumSuppressionRelations.size() > 1) {
          B2ERROR("The return value of cosTBz is ambiguous. Please provide the mask name as argument.");
        }
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    Manager::FunctionPtr KSFWVariables(const std::vector<std::string>& arguments)
    {
      if (arguments.size() > 0 && arguments.size() < 4) {
        bool useFS1 = false;
        auto variableName = arguments[0];
        std::string maskName = "";
        if (arguments.size() >= 2) {
          if (arguments[1] == "FS1") {
            useFS1 = true;
          } else {
            maskName = arguments[1];
          }
          if (arguments.size() == 3) {
            maskName = arguments[2];
            if (maskName == "FS1") {
              B2ERROR("It looks like you provided the arguments for KSFWVariables in the wrong order."
                      "If you want to use the KSFW moments calculated from the B final state particles, the second argument has to be 'FS1'."
                      "The third argument would then have to be the ROE mask name.");
            }
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

        // throw helpful error if name provided was not in allowed list
        if (index == -1) {
          std::string allowed = "";
          for (auto n : names)
            allowed += n + ", ";
          B2FATAL("Variable name provided: " << variableName << " is not one of the allowed options. Please choose from one of:" << allowed);
        }

        auto func = [index, useFS1, maskName](const Particle * particle) -> double {
          RelationVector<ContinuumSuppression> continuumSuppressionRelations = particle->getRelationsTo<ContinuumSuppression>("ALL");
          ContinuumSuppression* qq = nullptr;
          if (maskName.empty())
          {
            if (continuumSuppressionRelations.size() == 1) {
              qq = continuumSuppressionRelations[0];
            } else if (continuumSuppressionRelations.size() > 1) {
              B2ERROR("The return value of KSFWVariables is ambiguous. Please provide the mask name as argument.");
            }
          } else {
            qq = particle->getRelatedTo<ContinuumSuppression>(maskName);
          }
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
        B2FATAL("Wrong number of arguments for meta function KSFWVariables. It only takes between one and three arguments."
                " The first argument must be the variable. If you want to use the KSFW moments calculated from the B final state particles, set 'FS1' as second argument."
                " You can also provide the ROE mask name as second or third argument.");
      }
    }

    Manager::FunctionPtr CleoConesCS(const std::vector<std::string>& arguments)
    {
      if (arguments.size() > 0 && arguments.size() < 4) {

        int coneNumber = 0;
        try {
          coneNumber = Belle2::convertString<int>(arguments[0]);
        } catch (std::invalid_argument&) {
          B2FATAL("The first argument of the CleoConeCS meta function must be an integer!");
        }

        bool useROE = false;
        std::string maskName = "";
        if (arguments.size() >= 2) {
          if (arguments[1] == "ROE") {
            useROE = true;
          } else {
            maskName = arguments[1];
          }
          if (arguments.size() == 3) {
            maskName = arguments[2];
            if (maskName == "ROE") {
              B2ERROR("It looks like you provided the arguments for CleoConeCS in the wrong order."
                      "If you want to use the CleoCones calculated from all final state particles, the second argument has to be 'ROE'."
                      "The third argument would then have to be the ROE mask name.");
            }
          }
        }

        auto func = [coneNumber, useROE, maskName](const Particle * particle) -> double {
          RelationVector<ContinuumSuppression> continuumSuppressionRelations = particle->getRelationsTo<ContinuumSuppression>("ALL");
          ContinuumSuppression* qq = nullptr;
          if (maskName.empty())
          {
            if (continuumSuppressionRelations.size() == 1) {
              qq = continuumSuppressionRelations[0];
            } else if (continuumSuppressionRelations.size() > 1) {
              B2ERROR("The return value of CleoConeCS is ambiguous. Please provide the mask name as argument.");
            }
          } else {
            qq = particle->getRelatedTo<ContinuumSuppression>(maskName);
          }
          if (!qq)
            return std::numeric_limits<double>::quiet_NaN();

          std::vector<float> cleoCones = qq->getCleoConesALL();
          if (useROE)
            cleoCones = qq->getCleoConesROE();
          return cleoCones.at(coneNumber - 1);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for CleoConeCS function. It only takes between one and three arguments."
                "The first argument must be the cone number. If you want to use the CleoCones calculated from all final state particles, set 'ROE' as second argument."
                "You can also provide the ROE mask name as second or third argument.");
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
      if (arguments.size() == 2 || arguments.size() == 3) {
        auto variableName = arguments[0];
        std::string mode = arguments[1];

        const bool modeisSignal = mode == "Signal";
        const bool modeisAuto = mode == "Auto";

        if (not modeisSignal and (mode != "ROE") and not modeisAuto)
          B2FATAL("Second argument in useBThrustFrame can only be 'Signal', 'ROE' or 'Auto'. Your argument was " + mode);

        const Variable::Manager::Var* var = Manager::Instance().getVariable(variableName);

        std::string maskName = arguments.size() == 3 ? arguments[2] : "";

        auto func = [var, modeisSignal, modeisAuto, maskName](const Particle * particle) -> double {
          StoreObjPtr<RestOfEvent> roe("RestOfEvent");
          const Particle* Bparticle = roe->getRelated<Particle>();
          RelationVector<ContinuumSuppression> continuumSuppressionRelations = Bparticle->getRelationsTo<ContinuumSuppression>("ALL");
          ContinuumSuppression* qq = nullptr;
          if (maskName.empty())
          {
            if (continuumSuppressionRelations.size() == 1) {
              qq = continuumSuppressionRelations[0];
            } else if (continuumSuppressionRelations.size() > 1) {
              B2ERROR("The return value of useBThrustFrame is ambiguous. Please provide the mask name as argument.");
            }
          } else {
            qq = Bparticle->getRelatedTo<ContinuumSuppression>(maskName);
          }
          if (!qq)
            return std::numeric_limits<double>::quiet_NaN();

          double isinROE = isInRestOfEvent(particle);
          ROOT::Math::XYZVector newZ;
          if (modeisSignal or (modeisAuto and isinROE < 0.5))
            newZ = qq->getThrustB();
          else
            newZ = qq->getThrustO();

          ROOT::Math::XYZVector newY(0, 0, 0);
          if (newZ.z() == 0 and newZ.y() == 0)
            newY.SetX(1);
          else{
            newY.SetY(newZ.z());
            newY.SetZ(-newZ.y());
          }
          ROOT::Math::XYZVector newX = newY.Cross(newZ);

          UseReferenceFrame<CMSRotationFrame> signalframe(newX, newY, newZ);

          return var->function(particle);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function useBThrustFrame. It only takes two or three arguments. The first argument must be the variable."
                "The second can either be 'Signal', 'ROE', or 'Auto'."
                "The third argument is optional (as long as the ContinuumSuppression was built only once) and can define a specific ROE mask name.");
      }
    }


    VARIABLE_GROUP("Continuum Suppression");
    REGISTER_VARIABLE("R2(maskname)", R2WithMask, R"DOC(
Returns reduced Fox-Wolfram R2, defined as ratio of the i-th to the 0-th order Fox Wolfram moments.

.. warning:: You have to run the Continuum Suppression builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
)DOC");
    REGISTER_VARIABLE("R2", R2 , R"DOC(
Returns reduced Fox-Wolfram R2, defined as ratio of the i-th to the 0-th order Fox Wolfram moments.

.. warning:: You have to run the Continuum Suppression builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
:noindex:
)DOC");
    REGISTER_VARIABLE("thrustBm(maskname)", thrustBmWithMask, R"DOC(
Returns magnitude of the signal B thrust axis.

.. warning:: You have to run the Continuum Suppression builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
)DOC");
    REGISTER_VARIABLE("thrustBm", thrustBm, R"DOC(
Returns magnitude of the signal B thrust axis.

.. warning:: You have to run the Continuum Suppression builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
:noindex:
)DOC");
    REGISTER_VARIABLE("thrustOm(maskname)", thrustOmWithMask, R"DOC(
Returns magnitude of the ROE thrust axis.

.. warning:: You have to run the Continuum Suppression builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
)DOC");
    REGISTER_VARIABLE("thrustOm", thrustOm, R"DOC(
Returns magnitude of the ROE thrust axis.

.. warning:: You have to run the Continuum Suppression builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
:noindex:
)DOC");
    REGISTER_VARIABLE("cosTBTO(maskname)", cosTBTOWithMask, R"DOC(
Returns cosine of angle between thrust axis of the signal B and thrust axis of ROE.

.. warning:: You have to run the Continuum Suppression builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
)DOC");
    REGISTER_VARIABLE("cosTBTO", cosTBTO, R"DOC(
Returns cosine of angle between thrust axis of the signal B and thrust axis of ROE.

.. warning:: You have to run the Continuum Suppression builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
:noindex:
)DOC");
    REGISTER_VARIABLE("cosTBz(maskname)", cosTBzWithMask, R"DOC(
Returns cosine of angle between thrust axis of the signal B and z-axis.

.. warning:: You have to run the Continuum Suppression builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
)DOC");
    REGISTER_VARIABLE("cosTBz", cosTBz, R"DOC(
Returns cosine of angle between thrust axis of the signal B and z-axis.

.. warning:: You have to run the Continuum Suppression builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
:noindex:
)DOC");
    REGISTER_VARIABLE("KSFWVariables(variable[, string, string])", KSFWVariables,  R"DOC(
Returns variable et, mm2, or one of the 16 KSFW moments.
The second and third arguments are optional unless you have created multiple instances of the ContinuumSuppression with different ROE masks.
In that case the desired ROE mask name must be provided as well.
If the second argument is set to 'FS1', the KSFW moment is calculated from the B final state daughters.
Otherwise, the KSFW moment is calculated from the B primary daughters.
The ROE mask name is then either the second or the third argument and must not be called 'FS1'.
Allowed input values for ``variable`` argument are the following:

* mm2,   et
* hso00, hso01, hso02, hso03, hso04
* hso10, hso12, hso14
* hso20, hso22, hso24
* hoo0,  hoo1,  hoo2,  hoo3,  hoo4.

.. warning:: You have to run the Continuum Suppression builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
)DOC");

    REGISTER_VARIABLE("CleoConeCS(integer[, string, string])", CleoConesCS, R"DOC(
Returns i-th cleo cones from the continuum suppression. The allowed inputs for the ``integer`` argument are integers from *1* to *9*.
The second and third arguments are optional unless you have created multiple instances of the ContinuumSuppression with different ROE masks.
In that case the desired ROE mask name must be provided as well.
If the second argument is set to 'ROE', the CleoCones are calculated only from ROE particles.
Otherwise, the CleoCones are calculated from all final state particles.
The ROE mask name is then either the second or the third argument and must not be called 'ROE'.

.. warning:: You have to run the Continuum Suppression builder module for this variable to be meaningful.
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

.. warning:: You have to run the Continuum Suppression builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_continuumsuppression` and `buildContinuumSuppression`.
)DOC");

  }
}
