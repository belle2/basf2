/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida (original), CDC group                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <map>
#include <iostream>
#include <TObject.h>
#include <TVector3.h>

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

    const TVector3& getTriggerPosition() const
    {
      return m_position;
    }

    const TVector3& getTriggerDirection() const
    {
      return m_direction;
    }

    const TVector3& getPmtPosition() const
    {
      return m_pmtPosition;
    }

    double getLightPropSpeed() const
    {
      return m_lightPropSpeed;
    }

  private:
    TVector3 m_position;    /**< center position of the trigger plane. */
    TVector3 m_direction;   /**< normal direction to the trigger plane. */
    TVector3 m_pmtPosition; /**< edge position of the pmt. */
    double m_lightPropSpeed = 12.9925; /**< light of speed in the trigger counter. */
    ClassDef(CDCTriggerPlane, 1); /**< ClassDef */
  };

} // end namespace Belle2

