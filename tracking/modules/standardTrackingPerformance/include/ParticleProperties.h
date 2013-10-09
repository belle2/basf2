/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Ziegler                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLEPROPERTIES_H_
#define PARTICLEPROPERTIES_H_

namespace Belle2 {
  /** This struct is used by the StandardTrackingPerformanceModule to save information
   * of reconstructed tracks
   */
  struct ParticleProperties {

    /**< measured mass */
    double mass;

    /**< measured transverse momentum */
    double pt;

    /**< measured total momentum */
    double ptot;

    /**< polar angle of measured momentum vector */
    double cosTheta;

    /**< generated mass */
    double mass_gen;

    /**< generated transverse momentum */
    double pt_gen;

    /**< generated total momentum */
    double ptot_gen;

    /**< polar angle of generated momentum vector */
    double cosTheta_gen;
  };

}


#endif /* PARTICLEPROPERTIES_H_ */
