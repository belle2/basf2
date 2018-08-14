/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yu Hu                                                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/KFitVariables.h>
#include <analysis/VariableManager/Variable.h>

#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {
  namespace Variable {

// Chi2 and Prob -------------------------------------------

    double FourCKFitChi2(const Particle* part)
    {
      if (part->hasExtraInfo("FourCFitChi2")) return part->getExtraInfo("FourCFitChi2");
      else {
        B2WARNING("The ExtraInfo 'FourCFitChi2' not found!");
        return NAN;
      }
    }

    double FourCKFitProb(const Particle* part)
    {
      if (part->hasExtraInfo("FourCFitProb")) return part->getExtraInfo("FourCFitProb");
      else {
        B2WARNING("The ExtraInfo 'FourCFitProb' not found!");
        return NAN;
      }
    }


    VARIABLE_GROUP("FourCKFit");
    REGISTER_VARIABLE("FourCKFitChi2", FourCKFitChi2, "Chi2 of four momentum-constraint kinematical fit in KFit");
    REGISTER_VARIABLE("FourCKFitProb", FourCKFitProb, "Prob of four momentum-constraint kinematical fit in KFit");


  }
}
