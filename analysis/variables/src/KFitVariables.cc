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
        B2WARNING("The ExtraInfo 'FourCFitChi2' could not be found!");
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    double FourCKFitProb(const Particle* part)
    {
      if (part->hasExtraInfo("FourCFitProb")) return part->getExtraInfo("FourCFitProb");
      else {
        B2WARNING("The ExtraInfo 'FourCFitProb' could not be found!");
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    double Chi2TracksLBoost(const Particle* part)
    {
      if (part->hasExtraInfo("chiSquared_trackL")) return part->getExtraInfo("chiSquared_trackL");
      else {
        B2WARNING("The ExtraInfo 'chiSquared_trackL' could not be found!");
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    double KFitnTracks(const Particle* part)
    {
      if (part->hasExtraInfo("kFit_nTracks")) return part->getExtraInfo("kFit_nTracks");
      else {
        B2WARNING("The ExtraInfo 'kFit_nTracks' could not be found!");
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    VARIABLE_GROUP("KFit variables");
    REGISTER_VARIABLE("FourCKFitChi2", FourCKFitChi2, "Chi2 of four momentum-constraint kinematical fit in KFit");
    REGISTER_VARIABLE("FourCKFitProb", FourCKFitProb, "Prob of four momentum-constraint kinematical fit in KFit");

    REGISTER_VARIABLE("TracksLBoostChi2", Chi2TracksLBoost,
                      "indicator of vertex KFit quality based on accumulated change of track positions");
    REGISTER_VARIABLE("KFit_nTracks", KFitnTracks, "number of tracks used in the vertex KFit");
  }
}
