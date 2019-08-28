/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * returns dE/dx truncated mean
     */
    double dedx(const Particle* part);

    /**
     * returns dE/dx truncated mean without the hadron saturation correction
     */
    double dedxnosat(const Particle* part);

    /**
     * returns the momentum valid in the CDC
     */
    double pCDC(const Particle* part);

    /**
     * returns the chi value for electrons from CDC dEdx
     */
    double chiE_CDCdEdx(const Particle* part);

    /**
     * returns the chi value for muons from CDC dEdx
     */
    double chiMu_CDCdEdx(const Particle* part);

    /**
     * returns the chi value for pions from CDC dEdx
     */
    double chiPi_CDCdEdx(const Particle* part);

    /**
     * returns the chi value for kaons from CDC dEdx
     */
    double chiK_CDCdEdx(const Particle* part);

    /**
     * returns the chi value for protons from CDC dEdx
     */
    double chiP_CDCdEdx(const Particle* part);

    /**
     * returns the chi value for duetrons from CDC dEdx
     */
    double chiD_CDCdEdx(const Particle* part);

  }
} // Belle2 namespace

