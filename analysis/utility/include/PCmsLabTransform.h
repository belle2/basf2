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
     * Returns boost vector
     */
    TLorentzVector getBoostVector() const
    {
      return getBeamParams().getHER() + getBeamParams().getLER();
    }

    /**
     * Returns CMS energy of e+e- (aka. invariant mass in any system)
     */
    double getCMSEnergy() const
    {
      return getBeamParams().getMass();
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

    /** Get currently valid beam parameters from database, or from gearbox if not available. */
    const BeamParameters& getBeamParams() const;

  private:
    const DBObjPtr<BeamParameters> m_beamParams; /**< actually performs calculations. */
  };

} // Belle2 namespace




