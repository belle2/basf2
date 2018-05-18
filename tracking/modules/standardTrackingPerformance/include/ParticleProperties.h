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

    static const int maxNweights = 160; /**< the maximum number of stored weights */

    double mass; /**< measured mass */
    double px; /**< measured momentum in x direction */
    double py; /**< measured momentum in y direction */
    double pz; /**< measured momentum in z direction */
    double pt; /**< measured transverse momentum */
    double ptot; /**< measured total momentum */
    double cosTheta; /**< polar angle of measured momentum vector */
    double phi; /**< azimuthal angle of measured momentum vector */
    double x; /** < measured x value of position */
    double y; /** < measured y value of position */
    double z; /** < measured z value of position */

    int nPXDhits;  /** Number of PXD hits in reconstructed track  */
    int nSVDhits;  /** Number of SVD hits in reconstructed track  */
    int nCDChits;  /** Number of CDC hits in reconstructed track  */

    int nWeights;  /** Number of entries in weights array  */
    float weights[maxNweights];  /** Weights of the hits in sequence  */

    int pdg_gen; /** PDG code of generated particle */
    double mass_gen; /**< generated mass */
    double px_gen; /**< generated momentum in x direction */
    double py_gen; /**< generated momentum in y direction */
    double pz_gen; /**< generated momentum in z direction */
    double pt_gen; /**< generated transverse momentum */
    double ptot_gen; /**< generated total momentum */
    double cosTheta_gen; /**< polar angle of generated momentum vector */
    double phi_gen; /**< azimuthal angle of generated momentum vector */
    double x_gen; /**< x value of generated position */
    double y_gen; /**< y value of generated position */
    double z_gen; /**< z value of generated position */

    /**
     * Set all the values of the data structure to one value
     * for initialization
     */
    void setAllTo(double value)
    {
      mass = value;
      px = value;
      py = value;
      pz = value;
      pt = value;
      ptot = value;
      cosTheta = value;
      phi = value;
      x = value;
      y = value;
      z = value;

      nPXDhits = (int)value;
      nSVDhits = (int)value;
      nCDChits = (int)value;

      nWeights = 0;
      for (int i = 0; i < maxNweights; ++i)
        weights[i] = 0;

      pdg_gen = 0;
      mass_gen = value;
      px_gen = value;
      py_gen = value;
      pz_gen = value;
      pt = value;
      pt_gen = value;
      ptot_gen = value;
      cosTheta_gen = value;
      phi_gen = value;
      x_gen = value;
      y_gen = value;
      z_gen = value;
    }

    /**
     * Same as setAllTo but usable as C++ assignment operator
     */
    void operator=(double value)
    {
      setAllTo(value);
    }
  };

}


#endif /* PARTICLEPROPERTIES_H_ */
