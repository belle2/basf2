/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc, Thomas Keck                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/KFitVariables.h>
#include <analysis/VariableManager/Variable.h>

#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/EventExtraInfo.h>



// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <TLorentzVector.h>
#include <TRandom.h>
#include <TVectorF.h>
#include <TVector3.h>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

// Chi2 and Prob -------------------------------------------

    double FourCKFitChi2(const Particle* part)
    {
      if (part->hasExtraInfo("FourCFitChi2")) return part->getExtraInfo("FourCFitChi2");
      else return -999;
    }

    double FourCKFitProb(const Particle* part)
    {
      if (part->hasExtraInfo("FourCFitProb")) return part->getExtraInfo("FourCFitProb");
      else return -999;
    }


// Recoil Kinematics related ---------------------------------------------


    VARIABLE_GROUP("FourCKFit");
    REGISTER_VARIABLE("FourCKFitChi2", FourCKFitChi2, "Chi2 of four momentum-constraint kinematical fit in KFit");
    REGISTER_VARIABLE("FourCKFitProb", FourCKFitProb, "Prob of four momentum-constraint kinematical fit in KFit");


  }
}
