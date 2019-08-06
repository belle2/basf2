/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/dbobjects/BeamParameters.h>
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
     * Returns Lorentz transformation from CMS to Lab
     * @return const reference to Lorentz rotation matrix
     */
    const TLorentzRotation& rotateCmsToLab() const
    {
      return getBeamParams().getCMSToLab();
    }

    /**
     * Returns Lorentz transformation from Lab to CMS
     * @return const reference to Lorentz rotation matrix
     */
    const TLorentzRotation& rotateLabToCms() const
    {
      return getBeamParams().getLabToCMS();
    }

    /**
     * Returns boost vector (beta=p/E)
     */
    TVector3 getBoostVector() const
    {
      return getBeamFourMomentum().BoostVector();
    }

    /**
     * Returns LAB four-momentum of e+e-
     */
    TLorentzVector getBeamFourMomentum() const
    {
      return getBeamParams().getHER() + getBeamParams().getLER();
    }

    /**
     * Returns beam crossing angle
     */
    double getBeamCrossingAngle() const
    {
      return getBeamParams().getHER().Vect().Angle(-1.0 * getBeamParams().getLER().Vect());
    }

    /**
     * Returns CMS energy of e+e- (aka. invariant mass in any system)
     */
    double getCMSEnergy() const
    {
      return getCollisionInvariantMass().getMass();
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

    /** Get currently valid nominal beam parameters from database. */
    const BeamParameters& getBeamParams() const;

    /** Get currently valid beam invariant mass from database. */
    const CollisionInvariantMass& getCollisionInvariantMass() const;

    /** Get currently valid beam boost vector from database. */
    const CollisionBoostVector& getCollisionBoostVector() const;

  private:
    const DBObjPtr<BeamParameters> m_beamParamsDB; /**< db object for beam parameters. */
    const DBObjPtr<CollisionInvariantMass> m_invariantMassDB; /**< db object for invariant mass. */
    const DBObjPtr<CollisionBoostVector> m_boostVectorDB; /**< db object for boost vector. */
  };

} // Belle2 namespace




