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
   * Class to hold Lorentz transformations from/to CMS and boost vector.
   *
   * The beam parameters are read from the conditions database through accessors shared by all
   * instances, and the derived quantities (Lorentz rotations, CMS energy, ...) are cached. The cache
   * is invalidated whenever one of the payloads changes or the DBStore is reset, so constructing
   * this class is free and its getters are cheap.
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
    static ROOT::Math::XYZVector getBoostVector();

    /**
     * Returns CMS energy of e+e- (aka. invariant mass in any system)
     */
    static double getCMSEnergy();

    /**
     * Returns LAB four-momentum of e+e-, i.e. pHER + pLER
     */
    static ROOT::Math::PxPyPzEVector getBeamFourMomentum();

    /**
     * Returns Lorentz transformation from Lab to CMS
     * @return const reference to Lorentz rotation matrix
     */
    static const ROOT::Math::LorentzRotation rotateLabToCms();

    /**
     * Returns Lorentz transformation from CMS to Lab
     * @return const reference to Lorentz rotation matrix
     */
    static const ROOT::Math::LorentzRotation rotateCmsToLab();

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

  };

} // Belle2 namespace




