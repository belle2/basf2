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
#include <mdst/dbobjects/CollisionAxisCMS.h>
#include <framework/database/DBObjPtr.h>
#include <framework/utilities/LabToCms.h>

#include <Math/LorentzRotation.h>
#include <Math/Vector4D.h>

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
    ROOT::Math::XYZVector getBoostVector() const
    {
      return ROOT::Math::XYZVector(m_boostVectorDB->getBoost());
    }

    /**
     * Returns CMS energy of e+e- (aka. invariant mass in any system)
     */
    double getCMSEnergy() const
    {
      return m_invariantMassDB->getMass();
    }

    /**
     * Returns LAB four-momentum of e+e-, i.e. pHER + pLER
     */
    ROOT::Math::PxPyPzEVector getBeamFourMomentum() const
    {
      return rotateCmsToLab() * ROOT::Math::PxPyPzEVector(0, 0, 0, getCMSEnergy());
    }

    /**
     * Returns Lorentz transformation from Lab to CMS
     * @return const reference to Lorentz rotation matrix
     */
    const ROOT::Math::LorentzRotation rotateLabToCms() const
    {
      return LabToCms::rotateLabToCms(getBoostVector(), m_axisCmsDB->getAngleXZ(), m_axisCmsDB->getAngleYZ());
    }

    /**
     * Returns Lorentz transformation from CMS to Lab
     * @return const reference to Lorentz rotation matrix
     */
    const ROOT::Math::LorentzRotation rotateCmsToLab() const
    {
      return rotateLabToCms().Inverse();
    }

    /**
     * Transforms Lorentz vector into CM System
     * @param vec Lorentz vector in Laboratory System
     * @return Lorentz vector in CM System
     */
    static ROOT::Math::PxPyPzMVector labToCms(const ROOT::Math::PxPyPzMVector& vec);

    /**
     * Transforms Lorentz vector into Laboratory System
     * @param vec Lorentz vector in CM System
     * @return Lorentz vector in Laboratory System
     */
    static ROOT::Math::PxPyPzMVector cmsToLab(const ROOT::Math::PxPyPzMVector& vec);

    /**
     * Transforms Lorentz vector into CM System
     * @param vec Lorentz vector in Laboratory System
     * @return Lorentz vector in CM System
     */
    static ROOT::Math::PxPyPzEVector labToCms(const ROOT::Math::PxPyPzEVector& vec);

    /**
     * Transforms Lorentz vector into Laboratory System
     * @param vec Lorentz vector in CM System
     * @return Lorentz vector in Laboratory System
     */
    static ROOT::Math::PxPyPzEVector cmsToLab(const ROOT::Math::PxPyPzEVector& vec);

  private:
    const DBObjPtr<CollisionInvariantMass> m_invariantMassDB; /**< db object for invariant mass. */
    const DBObjPtr<CollisionBoostVector> m_boostVectorDB; /**< db object for boost vector. */
    const DBObjPtr<CollisionAxisCMS> m_axisCmsDB; /**< db object for collision axis in CM system from boost. */
  };

} // Belle2 namespace




