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

