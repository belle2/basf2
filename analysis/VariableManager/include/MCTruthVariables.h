/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunliffe                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * check that neutrals were seen in ECL, and charged were seen in SVD
     */
    double isReconstructible(const Particle*);

    /**
     * was theicle seen in the PXD
     */
    double seenInPXD(const Particle*);

    /**
     * was theicle seen in the SVD
     */
    double seenInSVD(const Particle*);

    /**
     * was theicle seen in the CDC
     */
    double seenInCDC(const Particle*);

    /**
     * was theicle seen in the TOP
     */
    double seenInTOP(const Particle*);

    /**
     * was theicle seen in the ECL
     */
    double seenInECL(const Particle*);

    /**
     * was theicle seen in the ARICH
     */
    double seenInARICH(const Particle*);

    /**
     * was theicle seen in the KLM
     */
    double seenInKLM(const Particle*);
  }
}
