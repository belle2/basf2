/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/variables/KFitVariables.h>

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
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    double FourCKFitProb(const Particle* part)
    {
      if (part->hasExtraInfo("FourCFitProb")) return part->getExtraInfo("FourCFitProb");
      else {
        B2WARNING("The ExtraInfo 'FourCFitProb' not found!");
        return std::numeric_limits<float>::quiet_NaN();
      }
    }


    VARIABLE_GROUP("FourCKFit");
    REGISTER_VARIABLE("FourCKFitChi2", FourCKFitChi2, "Chi2 of four momentum-constraint kinematical fit in KFit");
    REGISTER_VARIABLE("FourCKFitProb", FourCKFitProb, "Prob of four momentum-constraint kinematical fit in KFit");


  }
}
