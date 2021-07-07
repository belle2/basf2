/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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
