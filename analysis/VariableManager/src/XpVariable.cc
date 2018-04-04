/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hikari Hirata                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VariableManager/XpVariable.h>
#include <analysis/VariableManager/Manager.h>

#include <analysis/utility/PCmsLabTransform.h>

using namespace std;

namespace Belle2 {
  namespace Variable {
    double particleXp(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector p4 = part -> get4Vector();
      TLorentzVector p4CMS = T.rotateLabToCms() * p4;
      //      float s = T.getBeamParams().getMass();
      float s = T.getCMSEnergy();
      float M = part->getMass();
      return p4CMS.P() / TMath::Sqrt(s * s / 4 - M * M);
    }
    REGISTER_VARIABLE("Xp", particleXp, "scaled momentum xp");
  }
}

