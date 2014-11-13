/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PCMSLABTRANSFORM_H
#define PCMSLABTRANSFORM_H

#include <TLorentzRotation.h>
#include <TLorentzVector.h>

namespace Belle2 {

  /**
   * Class to hold Lorentz transformations from/to CMS and boost vector
   */
  class PCmsLabTransform {

  public:

    /**
     * Constructor
     */
    PCmsLabTransform();

    /**
     * Update method
     * @param Eher energy of High Energy Ring
     * @param Eler energy of Low Energy Ring
     * @param cross_angle beam crossing angle
     * @param angle angle of low energy beam in respect to z-axis
     */
    void update(double Eher, double Eler, double cross_angle, double angle);

    /**
     * Returns Lorentz transformation from CMS to Lab
     * @return const reference to Lorentz rotation matrix
     */
    const TLorentzRotation& rotateCmsToLab() const {
      return m_cms2lab;
    }

    /**
     * Returns Lorentz transformation from Lab to CMS
     * @return const reference to Lorentz rotation matrix
     */
    const TLorentzRotation& rotateLabToCms() const {
      return m_lab2cms;
    }

    /**
     * Returns boost vector
     * @return const reference to Lorentz vector
     */
    const TLorentzVector& getBoostVector() const {
      return m_boost;
    }

    /**
     * Returns CMS energy of e+e-
     * @return CMS energy of e+e-
     */
    double getCMSEnergy() const {
      return m_cmsEnergy;
    }

    /**
     * Transforms Lorentz vector into CM System
     * @param vec Lorentz vector in Laboratory System
     * @return Lorentz vector in CM System
     */
    static TLorentzVector labToCms(const TLorentzVector& vec);

    /**
     * Transforms Lorentz vector into Laboratory System
     * @param vec Lorentz vector in CM System
     * @return Lorentz vector in Laboratory System
     */
    static TLorentzVector cmsToLab(const TLorentzVector& vec);

  private:
    static bool m_initialized;          /**< true when class initialized */
    static TLorentzRotation m_cms2lab;  /**< from CMS to Lab */
    static TLorentzRotation m_lab2cms;  /**< from Lab to CMS */
    static TLorentzVector m_boost;      /**< boost vector */
    static double m_cmsEnergy;          /**< CMS energy of e+e- */
  };

} // Belle2 namespace

#endif



