/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <mdst/dbobjects/CollisionBoostVector.h>
#include <mdst/dbobjects/CollisionInvariantMass.h>
#include <framework/database/DBObjPtr.h>

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
     * Returns boost vector (beta=p/E)
     */
    TVector3 getBoostVector() const
    {
      return m_boostVectorDB->getBoost();
    }

    /**
     * Returns CMS energy of e+e- (aka. invariant mass in any system)
     */
    double getCMSEnergy() const
    {
      return m_invariantMassDB->getMass();
    }

    /**
     * Returns LAB four-momentum of e+e-
     */
    TLorentzVector getBeamFourMomentum() const
    {
      return rotateCmsToLab() * TLorentzVector(0, 0, 0, getCMSEnergy());
    }

    /**
     * Returns Lorentz transformation from Lab to CMS
     * @return const reference to Lorentz rotation matrix
     */
    const TLorentzRotation rotateLabToCms() const
    {
      TLorentzRotation rotation(-1.*getBoostVector());
      return rotation;
    }

    /**
     * Returns Lorentz transformation from CMS to Lab
     * @return const reference to Lorentz rotation matrix
     */
    const TLorentzRotation rotateCmsToLab() const
    {
      return rotateLabToCms().Inverse();
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
    const DBObjPtr<CollisionInvariantMass> m_invariantMassDB; /**< db object for invariant mass. */
    const DBObjPtr<CollisionBoostVector> m_boostVectorDB; /**< db object for boost vector. */
  };

} // Belle2 namespace




