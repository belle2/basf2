/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yu Hu                                                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/variables/OrcaKinFitVariables.h>
#include <analysis/VariableManager/Variable.h>

#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {
  namespace Variable {

// Chi2 and Prob -------------------------------------------

    double OrcaKinFitChi2(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitChi2")) return part->getExtraInfo("OrcaKinFitChi2");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitChi2' not found!");
        return NAN;
      }
    }

    double OrcaKinFitProb(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitProb")) return part->getExtraInfo("OrcaKinFitProb");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitProb' not found!");
        return NAN;
      }
    }

    double ErrorCode(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitErrorCode")) return part->getExtraInfo("OrcaKinFitErrorCode");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitErrorCode' not found!");
        return NAN;
      }
    }


// For 1C fit ------------------------------------------------------------

    double UnmeasuredTheta(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredTheta")) return part->getExtraInfo("OrcaKinFitUnmeasuredTheta");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredTheta' not found!");
        return NAN;
      }
    }

    double UnmeasuredPhi(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredPhi")) return part->getExtraInfo("OrcaKinFitUnmeasuredPhi");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredPhi' not found!");
        return NAN;
      }
    }

    double UnmeasuredE(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredE")) return part->getExtraInfo("OrcaKinFitUnmeasuredE");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredE' not found!");
        return NAN;
      }
    }

    double UnmeasuredErrorTheta(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredErrorTheta")) return part->getExtraInfo("OrcaKinFitUnmeasuredErrorTheta");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredErrorTheta' not found!");
        return NAN;
      }
    }

    double UnmeasuredErrorPhi(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredErrorPhi")) return part->getExtraInfo("OrcaKinFitUnmeasuredErrorPhi");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredErrorPhi' not found!");
        return NAN;
      }
    }

    double UnmeasuredErrorE(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredErrorE")) return part->getExtraInfo("OrcaKinFitUnmeasuredErrorE");
      else {
        B2WARNING("The ExtraInfo 'OrcaKinFitUnmeasuredErrorE' not found!");
        return NAN;
      }
    }


    VARIABLE_GROUP("OrcaKinFit");
    REGISTER_VARIABLE("OrcaKinFitChi2", OrcaKinFitChi2, "OrcaKinFit Chi2");
    REGISTER_VARIABLE("OrcaKinFitProb", OrcaKinFitProb, "OrcaKinFit Prob");
    REGISTER_VARIABLE("OrcaKinFitErrorCode", ErrorCode, "OrcaKinFit ErrorCode");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredTheta", UnmeasuredTheta, "Theta of Unmeasured particle in OrcaKinFit 1C");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredPhi", UnmeasuredPhi, "Phi of Unmeasured particle in OrcaKinFit 1C");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredE", UnmeasuredE, "energy of Unmeasured particle in OrcaKinFit 1C");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredErrorTheta", UnmeasuredErrorTheta, "Theta error of Unmeasured particle in OrcaKinFit 1C");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredErrorPhi", UnmeasuredErrorPhi, "Phi error of Unmeasured particle in OrcaKinFit 1C");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredErrorE", UnmeasuredErrorE, "energy error of Unmeasured particle in OrcaKinFit 1C");


  }
}
