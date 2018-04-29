/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <math.h>
#include <root/TObject.h>
#include <root/TVector3.h>


namespace Belle2 {

  /**
  * The DATCON ROI class.
  *
  */
  class DATCONMPH : public TObject {
  public:
    /** Standard constructor */
    DATCONMPH() : m_sensorID(), m_localCoordinate(), m_localPixel(), m_qualityOfHit(0) {}

    /** Constructor for extrapolated hit (MPH) providing sensor ID and local coordinate */
    DATCONMPH(VxdID sensorID, TVector2 localCoordinate, double qualityOfHit):
      m_sensorID(sensorID), m_localCoordinate(localCoordinate), m_qualityOfHit(qualityOfHit) {}

    /** Constructor for extrapolated hit (MPH) providing sensor ID, local coordinate and pixelID */
    DATCONMPH(VxdID sensorID, TVector2 localCoordinate, TVector2 localPixel, double qualityOfHit):
      m_sensorID(sensorID), m_localCoordinate(localCoordinate), m_localPixel(localPixel), m_qualityOfHit(qualityOfHit) {}

    ~DATCONMPH() {}

    /** Get sensor ID */
    VxdID getSensorID()           { return m_sensorID; }
    /** Get local coordinate */
    TVector2 getLocalCoordinate() { return m_localCoordinate; }
    /** Get local Pixel coordinate */
    TVector2 getLocalPixel()      { return m_localPixel; }
    /** Get quality of hit */
    double getQualityOfHit()      { return m_qualityOfHit; }

  private:
    /** Sensor ID */
    VxdID m_sensorID;
    TVector2 m_localCoordinate;
    TVector2 m_localPixel;
    double m_qualityOfHit;

    ClassDef(DATCONMPH, 2)

  }; // class DATCONMPH

} // end namespace Belle2
