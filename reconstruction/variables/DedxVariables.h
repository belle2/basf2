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
     * returns dE/dx truncated mean
     */
    double CDCdedx(const Particle* part);

    /**
     * returns dE/dx truncated mean without the hadron saturation correction
     */
    double CDCdedxnosat(const Particle* part);

    /**
     * returns the momentum valid in the CDC
     */
    double pCDC(const Particle* part);

    /**
     * returns the costh valid in the CDC
     */
    double costhCDC(const Particle* part);

    /**
     * returns the ndedx hits to dedx tracks
     */
    double CDCdEdx_nhits(const Particle* part);

    /**
     * returns the ndedx hits layer to dedx tracks
     */
    double CDCdEdx_lnhits(const Particle* part);

    /**
     * returns the ndedx hit layer used to dedx tracks
     */
    double CDCdEdx_lnhitsused(const Particle* part);

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
    double SVDdedx(const Particle* part);
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

