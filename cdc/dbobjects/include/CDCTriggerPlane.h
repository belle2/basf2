/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <Math/Vector3D.h>

namespace Belle2 {

  /**
   * Database object for timing offset (t0).
   */
  class CDCTriggerPlane: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCTriggerPlane() {}

    /**
     * Getter for the center posotion of the trigger plane.
     */
    const ROOT::Math::XYZVector& getTriggerPosition() const
    {
      return m_position;
    }
    /**
     * Getter for the normal vector to the trigger plane.
     */
    const ROOT::Math::XYZVector& getTriggerDirection() const
    {
      return m_direction;
    }

    /**
     * Getter for the pmt position.
     */
    const ROOT::Math::XYZVector& getPmtPosition() const
    {
      return m_pmtPosition;
    }

    /**
     * Getter for the light speed in the scintillator.
     */
    double getLightPropSpeed() const
    {
      return m_lightPropSpeed;
    }

  private:
    ROOT::Math::XYZVector m_position;    /**< center position of the trigger plane. */
    ROOT::Math::XYZVector m_direction;   /**< normal direction to the trigger plane. */
    ROOT::Math::XYZVector m_pmtPosition; /**< edge position of the pmt. */
    double m_lightPropSpeed = 12.9925; /**< light of speed in the trigger counter. */
    ClassDef(CDCTriggerPlane, 2); /**< ClassDef */
  };

} // end namespace Belle2

