/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Francesco Tenchini                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
      TVector3 beta = getBoostVector();
      double gamma = 1 / TMath::Sqrt(1 - beta.Mag2());
      TLorentzVector beamFourVec(beta, 1.);
      return beamFourVec * gamma * getCMSEnergy();
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




