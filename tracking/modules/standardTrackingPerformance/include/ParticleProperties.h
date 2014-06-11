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
    double px; /**< measured momentum in x direction */
    double py; /**< measured momentum in y direction */
    double pz; /**< measured momentum in z direction */
    double pt; /**< measured transverse momentum */
    double ptot; /**< measured total momentum */
    double cosTheta; /**< polar angle of measured momentum vector */
    double x; /** < measured x value of position */
    double y; /** < measured y value of position */
    double z; /** < measured z value of position */


    double mass_gen; /**< generated mass */
    double px_gen; /**< generated momentum in x direction */
    double py_gen; /**< generated momentum in y direction */
    double pz_gen; /**< generated momentum in z direction */
    double pt_gen; /**< generated transverse momentum */
    double ptot_gen; /**< generated total momentum */
    double cosTheta_gen; /**< polar angle of generated momentum vector */
    double x_gen; /**< x value of generated position */
    double y_gen; /**< y value of generated position */
    double z_gen; /**< z value of generated position */

    void setAllTo(double value) {
      mass = value;
      px = value;
      py = value;
      pz = value;
      pt = value;
      ptot = value;
      cosTheta = value;
      x = value;
      y = value;
      z = value;

      mass_gen = value;
      px_gen = value;
      py_gen = value;
      pz_gen = value;
      pt = value;
      pt_gen = value;
      ptot_gen = value;
      cosTheta_gen = value;
      x_gen = value;
      y_gen = value;
      z_gen = value;
    }

    void operator=(double value) {
      setAllTo(value);
    }
  };

}


#endif /* PARTICLEPROPERTIES_H_ */
