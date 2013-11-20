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


    double mass; /**< measured mass */
    double pt; /**< measured transverse momentum */
    double ptot; /**< measured total momentum */
    double cosTheta; /**< polar angle of measured momentum vector */

    double mass_gen; /**< generated mass */
    double pt_gen; /**< generated transverse momentum */
    double ptot_gen; /**< generated total momentum */
    double cosTheta_gen; /**< polar angle of generated momentum vector */
  };

}


#endif /* PARTICLEPROPERTIES_H_ */
