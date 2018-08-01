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
#include <analysis/VariableManager/OrcaKinFitVariables.h>
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

    double Chi2(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitChi2")) return part->getExtraInfo("OrcaKinFitChi2");
      else return NAN;
    }

    double Prob(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitProb")) return part->getExtraInfo("OrcaKinFitProb");
      else return NAN;
    }

    double ErrorCode(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitErrorCode")) return part->getExtraInfo("OrcaKinFitErrorCode");
      else return NAN;
    }


// For 1C fit ------------------------------------------------------------

    double UnmeasuredTheta(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredTheta")) return part->getExtraInfo("OrcaKinFitUnmeasuredTheta");
      else return NAN;
    }

    double UnmeasuredPhi(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredPhi")) return part->getExtraInfo("OrcaKinFitUnmeasuredPhi");
      else return NAN;
    }

    double UnmeasuredE(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredE")) return part->getExtraInfo("OrcaKinFitUnmeasuredE");
      else return NAN;
    }


    double UnmeasuredErrorTheta(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredErrorTheta")) return part->getExtraInfo("OrcaKinFitUnmeasuredErrorTheta");
      else return NAN;
    }

    double UnmeasuredErrorPhi(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredErrorPhi")) return part->getExtraInfo("OrcaKinFitUnmeasuredErrorPhi");
      else return NAN;
    }

    double UnmeasuredErrorE(const Particle* part)
    {
      if (part->hasExtraInfo("OrcaKinFitUnmeasuredErrorE")) return part->getExtraInfo("OrcaKinFitUnmeasuredErrorE");
      else return NAN;
    }


    VARIABLE_GROUP("OrcaKinFit");
    REGISTER_VARIABLE("OrcaKinFitChi2", Chi2, "OrcaKinFit Chi2");
    REGISTER_VARIABLE("OrcaKinFitProb", Prob, "OrcaKinFit Prob");
    REGISTER_VARIABLE("OrcaKinFitErrorCode", ErrorCode, "OrcaKinFit ErrorCode");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredTheta", UnmeasuredTheta, "Theta of Unmeasured photon in OrcaKinFit 1C");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredPhi", UnmeasuredPhi, "Phi of Unmeasured photon in OrcaKinFit 1C");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredE", UnmeasuredE, "energy of Unmeasured photon in OrcaKinFit 1C");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredErrorTheta", UnmeasuredErrorTheta, "Theta error of Unmeasured photon in OrcaKinFit 1C");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredErrorPhi", UnmeasuredErrorPhi, "Phi error of Unmeasured photon in OrcaKinFit 1C");
    REGISTER_VARIABLE("OrcaKinFitUnmeasuredErrorE", UnmeasuredErrorE, "energy error of Unmeasured photon in OrcaKinFit 1C");


  }
}
