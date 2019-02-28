/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yu Hu                                                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>

namespace Belle2 {
  class Particle;

  namespace Variable {


    /**
     * return Chi2 of OrcaKinFit
     */
    double OrcaKinFitChi2(const Particle* part);

    /**
     * return Prob of OrcaKinFit
     */
    double OrcaKinFitProb(const Particle* part);

    /**
     * return ErrorCode of OrcaKinFit
     */
    double ErrorCode(const Particle* part);

    /**
     * return Unmeasured Theta of OrcaKinFit 1C
     */
    double UnmeasuredTheta(const Particle* part);

    /**
     * return Unmeasured Phi of OrcaKinFit 1C
     */
    double UnmeasuredPhi(const Particle* part);

    /**
     * return Unmeasured E of OrcaKinFit 1C
     */
    double UnmeasuredE(const Particle* part);

    /**
     * return Unmeasured Theta error of OrcaKinFit 1C
     */
    double UnmeasuredErrorTheta(const Particle* part);

    /**
     * return Unmeasured Phi error of OrcaKinFit 1C
     */
    double UnmeasuredErrorPhi(const Particle* part);

    /**
     * return Unmeasured E error of OrcaKinFit 1C
     */
    double UnmeasuredErrorE(const Particle* part);


  }
} // Belle2 namespace
