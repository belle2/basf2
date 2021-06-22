/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/EventShapeContainer.h>

#include <analysis/utility/ReferenceFrame.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/utilities/Conversion.h>

#include <TLorentzVector.h>
#include <TVectorF.h>
#include <TVector3.h>

#include <boost/algorithm/string.hpp>

namespace Belle2 {
  namespace Variable {

    Manager::FunctionPtr foxWolframR(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1) {
        B2ERROR("foxWolframR cannot be called without providing the moment order");
        return nullptr;
      }

      int order = -1;
      try {
        order = Belle2::convertString<int>(arguments[0]);
      } catch (std::invalid_argument&) {
        B2ERROR("Argument of foxWolframR must be an integer");
        return nullptr;
      }

      if (order < 0 || order > 8) {
        B2ERROR("The Fox-Wolfram moment order must be within 0 and 8.");
        return nullptr;
      }

      auto func = [order](const Particle*) -> double{

        StoreObjPtr<EventShapeContainer> evtShapeCont;
        if (!evtShapeCont)
        {
          B2ERROR("No EventShapeContainer object has been found in the datastore");
          return std::numeric_limits<float>::quiet_NaN();
        }
        if (evtShapeCont->getFWMoment(0) == 0)
        {
          B2ERROR("The 0th-order FoxWolfram moment is zero");
          return std::numeric_limits<float>::quiet_NaN();
        }
        return evtShapeCont->getFWMoment(order) / evtShapeCont->getFWMoment(0);
      };
      return func;
    }



    Manager::FunctionPtr foxWolframH(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1) {
        B2ERROR("foxWolframH cannot be called without providing the moment order");
        return nullptr;
      }

      int order = -1;
      try {
        order = Belle2::convertString<int>(arguments[0]);
      } catch (std::invalid_argument&) {
        B2ERROR("Argument of foxWolframH must be an integer");
        return nullptr;
      }

      if (order < 0 || order > 8) {
        B2ERROR("The Fox-Wolfram moment order must be within 0 and 8.");
        return nullptr;
      }

      auto func = [order](const Particle*) -> double{

        StoreObjPtr<EventShapeContainer> evtShapeCont;
        if (!evtShapeCont)
        {
          B2ERROR("No EventShapeContainer object has been found in the datastore");
          return std::numeric_limits<float>::quiet_NaN();
        }
        return evtShapeCont->getFWMoment(order);
      };
      return func;
    }



    Manager::FunctionPtr harmonicMoment(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 2) {
        B2ERROR("harmonicMoment requires two arguments: the harmonic order (0-8) and the reference axis name (thrust or collision)");
        return nullptr;
      }

      int order = -1;
      try {
        order = Belle2::convertString<int>(arguments[0]);
      } catch (std::invalid_argument&) {
        B2ERROR("First argument of harmonicMoment must be an integer");
        return nullptr;
      }
      std::string axisName =  arguments[1];
      boost::to_lower(axisName);

      if (order < 0 || order > 8) {
        B2ERROR("The Fox-Wolfram moment order must be within 0 and 8.");
        return nullptr;
      }
      if (axisName != "thrust" && axisName != "collision") {
        B2ERROR("Invalid axis name "  << arguments[1] << ". The valid options are thrust and collision");
        return nullptr;
      }

      auto func = [order, axisName](const Particle*) -> double{
        StoreObjPtr<EventShapeContainer> evtShapeCont;
        if (!evtShapeCont)
        {
          B2ERROR("No EventShapeContainer object has been found in the datastore");
          return std::numeric_limits<float>::quiet_NaN();
        }
        if (axisName == "thrust")
          return evtShapeCont->getHarmonicMomentThrust(order);
        else
          return evtShapeCont->getHarmonicMomentCollision(order);
      };
      return func;
    }


    Manager::FunctionPtr cleoCone(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 2) {
        B2ERROR("cleoCone requires two arguments: the cone order (0-9) and the reference axis name (thrust or collision)");
        return nullptr;
      }

      int order = -1;
      try {
        order = Belle2::convertString<int>(arguments[0]);
      } catch (std::invalid_argument&) {
        B2ERROR("Argument of cleoCone must be an integer");
        return nullptr;
      }
      std::string axisName =  arguments[1];
      boost::to_lower(axisName);

      if (order < 0 || order > 8) {
        B2ERROR("The CLEO cone order must be within 0 and 8.");
        return nullptr;
      }
      if (axisName != "thrust" && axisName != "collision") {
        B2ERROR("Invalid axis name "  << arguments[1] << ". The valid options are thrust and collision");
        return nullptr;
      }

      auto func = [order, axisName](const Particle*) -> double{
        StoreObjPtr<EventShapeContainer> evtShapeCont;
        if (!evtShapeCont)
        {
          B2ERROR("No EventShapeContainer object has been found in the datastore");
          return std::numeric_limits<float>::quiet_NaN();
        }
        if (axisName == "thrust")
          return evtShapeCont->getCleoConeThrust(order);
        else
          return evtShapeCont->getCleoConeCollision(order);
      };
      return func;
    }



    double foxWolframR1(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      if (evtShapeCont->getFWMoment(0) == 0) {
        B2ERROR("The 0th-order FoxWolfram moment is zero");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getFWMoment(1) / evtShapeCont->getFWMoment(0);
    }

    double foxWolframR2(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      if (evtShapeCont->getFWMoment(0) == 0) {
        B2ERROR("The 0th-order FoxWolfram moment is zero");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getFWMoment(2) / evtShapeCont->getFWMoment(0);
    }

    double foxWolframR3(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      if (evtShapeCont->getFWMoment(0) == 0) {
        B2ERROR("The 0th-order FoxWolfram moment is zero");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getFWMoment(3) / evtShapeCont->getFWMoment(0);
    }

    double foxWolframR4(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      if (evtShapeCont->getFWMoment(0) == 0) {
        B2ERROR("The 0th-order FoxWolfram moment is zero");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getFWMoment(4) / evtShapeCont->getFWMoment(0);
    }


    double harmonicMomentThrust0(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getHarmonicMomentThrust(0);
    }

    double harmonicMomentThrust1(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getHarmonicMomentThrust(1);
    }

    double harmonicMomentThrust2(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getHarmonicMomentThrust(2);
    }

    double harmonicMomentThrust3(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getHarmonicMomentThrust(3);
    }

    double harmonicMomentThrust4(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getHarmonicMomentThrust(4);
    }


    double cleoConeThrust0(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getCleoConeThrust(0);
    }

    double cleoConeThrust1(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getCleoConeThrust(1);
    }

    double cleoConeThrust2(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getCleoConeThrust(2);
    }

    double cleoConeThrust3(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getCleoConeThrust(3);
    }

    double cleoConeThrust4(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getCleoConeThrust(4);
    }

    double cleoConeThrust5(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getCleoConeThrust(5);
    }

    double cleoConeThrust6(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getCleoConeThrust(6);
    }

    double cleoConeThrust7(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getCleoConeThrust(7);
    }

    double cleoConeThrust8(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getCleoConeThrust(8);
    }


    double sphericity(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      // (3/2)(lamda_2 + lambda_3)
      return 1.5 * (evtShapeCont->getSphericityEigenvalue(1) + evtShapeCont->getSphericityEigenvalue(2)) ;
    }

    double aplanarity(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      // (3/2)(lambda_3)
      return 1.5 * evtShapeCont->getSphericityEigenvalue(2);
    }

    double forwardHemisphereMass(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getForwardHemisphere4Momentum().Mag();
    }

    double forwardHemisphereX(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getForwardHemisphere4Momentum().Vect().X();
    }

    double forwardHemisphereY(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getForwardHemisphere4Momentum().Vect().Y();
    }

    double forwardHemisphereZ(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getForwardHemisphere4Momentum().Vect().Z();
    }

    double forwardHemisphereMomentum(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getForwardHemisphere4Momentum().Vect().Mag();
    }

    double forwardHemisphereEnergy(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getForwardHemisphere4Momentum().E();
    }




    double backwardHemisphereMass(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getBackwardHemisphere4Momentum().Mag();
    }


    double backwardHemisphereX(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getBackwardHemisphere4Momentum().Vect().X();
    }

    double backwardHemisphereY(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getBackwardHemisphere4Momentum().Vect().Y();
    }

    double backwardHemisphereZ(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getBackwardHemisphere4Momentum().Vect().Z();
    }

    double backwardHemisphereMomentum(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getBackwardHemisphere4Momentum().Vect().Mag();
    }

    double backwardHemisphereEnergy(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getBackwardHemisphere4Momentum().E();
    }





    double thrust(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getThrust();
    }


    double thrustAxisX(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getThrustAxis().X();
    }

    double thrustAxisY(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getThrustAxis().Y();
    }

    double thrustAxisZ(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getThrustAxis().Z();
    }


    double thrustAxisCosTheta(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getThrustAxis().CosTheta();
    }


    Manager::FunctionPtr useThrustFrame(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto variableName = arguments[0];

        const Variable::Manager::Var* var = Manager::Instance().getVariable(variableName);

        auto func = [var](const Particle * particle) -> double {
          StoreObjPtr<EventShapeContainer> evtShapeCont;
          if (!evtShapeCont)
          {
            B2ERROR("No EventShapeContainer object has been found in the datastore");
            return std::numeric_limits<float>::quiet_NaN();
          }

          TVector3 newZ = evtShapeCont->getThrustAxis();
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
        B2FATAL("Wrong number of arguments for meta function useThrustFrame. It only takes one argument, the variable name.");
      }
    }


    VARIABLE_GROUP("EventShape");

    REGISTER_VARIABLE("foxWolframR(i)", foxWolframR, R"DOC(
[Eventbased] Ratio of the i-th to the 0-th order Fox Wolfram moments. The order ``i`` can go from 0 up to 8th.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("foxWolframH(i)", foxWolframH, R"DOC(
[Eventbased] Returns i-th order Fox Wolfram moment. The order ``i`` can go from 0 up to 8th."

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("harmonicMoment(i, axisName)", harmonicMoment, R"DOC(
[Eventbased] Returns i-th order harmonic moment, calculated with respect to the axis ``axisName``. The order ``i`` can go from 0 up to 8th, the ``axisName`` can be either 'thrust' or 'collision'.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("cleoCone(i, axisName)", cleoCone, R"DOC(
[Eventbased] Returns i-th order Cleo cone, calculated with respect to the axis ``axisName``. The order ``i`` can go from 0 up to 8th, the ``axisName`` can be either 'thrust' or 'collision'.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("useThrustFrame(variable)", useThrustFrame, R"DOC(
Evaluates a variable value in the thrust reference frame.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");


    REGISTER_VARIABLE("foxWolframR1", foxWolframR1, R"DOC(
[Eventbased]  Ratio of the 1-st to the 0-th order Fox Wolfram moments. This is just an alias of foxWolframR(1) defined for the user's convenience.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("foxWolframR2", foxWolframR2, R"DOC(
[Eventbased]  Ratio of the 2-nd to the 0-th order Fox Wolfram moments. This is just an alias of foxWolframR(2) defined for the user's convenience.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("foxWolframR3", foxWolframR3, R"DOC(
[Eventbased]  Ratio of the 3-rd to the 0-th order Fox Wolfram moments. This is just an alias of foxWolframR(3) defined for the user's convenience.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("foxWolframR4", foxWolframR4, R"DOC(
[Eventbased]  Ratio of the 4-th to the 0-th order Fox Wolfram moments. This is just an alias of foxWolframR(4) defined for the user's convenience.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");

    REGISTER_VARIABLE("harmonicMomentThrust0", harmonicMomentThrust0, R"DOC(
[Eventbased] Harmonic moment of the 0th order calculated with respect to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("harmonicMomentThrust1", harmonicMomentThrust1, R"DOC(
[Eventbased] Harmonic moment of the 1st order calculated with respect to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("harmonicMomentThrust2", harmonicMomentThrust2, R"DOC(
[Eventbased] Harmonic moment of the 2nd order calculated with respect to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("harmonicMomentThrust3", harmonicMomentThrust3, R"DOC(
[Eventbased] Harmonic moment of the 3rd order calculated with respect to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("harmonicMomentThrust4", harmonicMomentThrust4, R"DOC(
[Eventbased] Harmonic moment of the 4th order calculated with respect to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");

    REGISTER_VARIABLE("cleoConeThrust0", cleoConeThrust0, R"DOC(
[Eventbased] 0th Cleo cone calculated with respect to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("cleoConeThrust1", cleoConeThrust1, R"DOC(
[Eventbased] 1st Cleo cone calculated with respect to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("cleoConeThrust2", cleoConeThrust2, R"DOC(
[Eventbased] 2nd Cleo cone calculated with respect to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("cleoConeThrust3", cleoConeThrust3, R"DOC(
[Eventbased] 3rd Cleo cone calculated with respect to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("cleoConeThrust4", cleoConeThrust4, R"DOC(
[Eventbased] 4th Cleo cone calculated with respect to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("cleoConeThrust5", cleoConeThrust5, R"DOC(
[Eventbased] 5th Cleo cone calculated with respect to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("cleoConeThrust6", cleoConeThrust6, R"DOC(
[Eventbased] 6th Cleo cone calculated with respect to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("cleoConeThrust7", cleoConeThrust7, R"DOC(
[Eventbased] 7th Cleo cone calculated with respect to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("cleoConeThrust8", cleoConeThrust8, R"DOC(
[Eventbased] 8th Cleo cone calculated with respect to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");


    REGISTER_VARIABLE("sphericity", sphericity, R"DOC(
[Eventbased] Event sphericity, defined as the linear combination of the sphericity eigenvalues :math:`\\lambda_i`: :math:`S = (3/2)(\\lambda_2+\\lambda_3)`.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("aplanarity", aplanarity, R"DOC(
[Eventbased] Event aplanarity, defined as the 3/2 of the third sphericity eigenvalue.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");

    REGISTER_VARIABLE("thrust", thrust, R"DOC(
[Eventbased] Event thrust.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("thrustAxisX", thrustAxisX, R"DOC(
[Eventbased] X component of the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("thrustAxisY", thrustAxisY, R"DOC(
[Eventbased] Y component of the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("thrustAxisZ", thrustAxisZ, R"DOC(
[Eventbased] Z component of the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("thrustAxisCosTheta", thrustAxisCosTheta, R"DOC(      
[Eventbased] Cosine of the polar angle component of the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");

    REGISTER_VARIABLE("forwardHemisphereMass", forwardHemisphereMass, R"DOC(
[Eventbased] Invariant mass of the particles flying in the same direction as the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("forwardHemisphereX", forwardHemisphereX, R"DOC(
[Eventbased] X component of the total momentum of the particles flying in the same direction as the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("forwardHemisphereY", forwardHemisphereY, R"DOC(
[Eventbased] Y component of the total momentum of the particles flying in the same direction as the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("forwardHemisphereZ", forwardHemisphereZ, R"DOC(
[Eventbased] Z component of the total momentum of the particles flying in the same  direction of the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("forwardHemisphereMomentum", forwardHemisphereMomentum, R"DOC(
[Eventbased] Total momentum of the particles flying in the same direction as the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("forwardHemisphereEnergy", forwardHemisphereEnergy, R"DOC(
[Eventbased] Total energy of the particles flying in the same direction as the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");

    REGISTER_VARIABLE("backwardHemisphereMass", backwardHemisphereMass, R"DOC(
[Eventbased] Invariant mass of the particles flying in the direction opposite to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("backwardHemisphereX", backwardHemisphereX, R"DOC(
[Eventbased] X component of the total momentum of the particles flying in the direction opposite to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("backwardHemisphereY", backwardHemisphereY, R"DOC(
[Eventbased] Y component of the total momentum of the particles flying in the direction opposite to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("backwardHemisphereZ", backwardHemisphereZ, R"DOC(
[Eventbased] Z component of the total momentum of the particles flying in the direction opposite to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("backwardHemisphereMomentum", backwardHemisphereMomentum, R"DOC(
[Eventbased] Total momentum of the particles flying in the direction opposite to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
    REGISTER_VARIABLE("backwardHemisphereEnergy", backwardHemisphereEnergy, R"DOC(
[Eventbased] Total energy of the particles flying in the direction opposite to the thrust axis.

.. warning:: You have to run the Event Shape builder module for this variable to be meaningful.
.. seealso:: :ref:`analysis_eventshape` and `modularAnalysis.buildEventShape`.
)DOC");
  }
}
