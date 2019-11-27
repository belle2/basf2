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
    double CDCdEdx_chiE(const Particle* part);

    /**
     * returns the chi value for muons from CDC dEdx
     */
    double CDCdEdx_chiMu(const Particle* part);

    /**
     * returns the chi value for pions from CDC dEdx
     */
    double CDCdEdx_chiPi(const Particle* part);

    /**
     * returns the chi value for kaons from CDC dEdx
     */
    double CDCdEdx_chiK(const Particle* part);

    /**
     * returns the chi value for protons from CDC dEdx
     */
    double CDCdEdx_chiP(const Particle* part);

    /**
     * returns the chi value for duetrons from CDC dEdx
     */
    double CDCdEdx_chiD(const Particle* part);
    /**
     * returns the momentum valid in the SVD
     */
    double SVD_p(const Particle* part);
    /**
     * returns the MC true momentum valid in the SVD
     */
    double SVD_pTrue(const Particle* part);
    /**
     * returns the dEdx truncated mean valid in the SVD
     */
    double SVD_dedx(const Particle* part);
    /**
     * returns the number of hits valid in the SVD
     */
    double SVD_nHits(const Particle* part);
    /**
     *returns cos(theta) of the track valid in the SVD
     */
    double SVD_CosTheta(const Particle* part);


  }
} // Belle2 namespace

