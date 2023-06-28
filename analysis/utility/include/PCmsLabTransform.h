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
#include <framework/geometry/B2Vector3.h>

#include <Math/Boost.h>
#include <Math/AxisAngle.h>
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
    B2Vector3D getBoostVector() const
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
     * Returns CMS energy of e+e- (aka. invariant mass in any system)
     */
    double getCMSEnergyError() const
    {
      return m_invariantMassDB->getMassError();
    }

    /**
     * Returns LAB four-momentum of e+e-, i.e. pHER + pLER
     */
    ROOT::Math::PxPyPzEVector getBeamFourMomentum() const
    {
      return rotateCmsToLab() * ROOT::Math::PxPyPzEVector(0, 0, 0, getCMSEnergy());
    }

    /**
     * Returns LAB four-momentum of e+e-, i.e. pHER + pLER
     */
    ROOT::Math::PxPyPzEVector getBeamFourMomentumError() const
    {
      return rotateCmsToLab() * ROOT::Math::PxPyPzEVector(0, 0, 0, getCMSEnergyError());
    }

    /**
     * Returns Lorentz transformation from Lab to CMS
     * @return const reference to Lorentz rotation matrix
     * Similar transformation done in MCInitialParticles::calculateBoost()
     */
    const ROOT::Math::LorentzRotation rotateLabToCms() const
    {
      //boost to CM frame
      ROOT::Math::LorentzRotation boost(ROOT::Math::Boost(-1.*getBoostVector()));


      //rotation such that the collision axis is aligned with z-axis
      ROOT::Math::XYZVector zaxis(0., 0., 1.); //target collision axis

      double tanAngleXZ = tan(m_axisCmsDB->getAngleXZ());
      double tanAngleYZ = tan(m_axisCmsDB->getAngleYZ());
      double Norm   = 1 / sqrt(1 + pow(tanAngleXZ, 2) + pow(tanAngleYZ, 2));
      ROOT::Math::XYZVector electronCMS(Norm * tanAngleXZ, Norm * tanAngleYZ, Norm); //current collision axis

      ROOT::Math::XYZVector rotAxis = zaxis.Cross(electronCMS);
      double rotangle = asin(rotAxis.R());

      ROOT::Math::LorentzRotation rotation(ROOT::Math::AxisAngle(rotAxis, -rotangle));


      ROOT::Math::LorentzRotation trans = rotation * boost;
      return trans;
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




