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
    * The DATCON Most Probable Hit (MPH) class.
    * This class contains information about the extrapolated hits on the
    * PXD calculated by DATCON.
    */
  class DATCONMostProbableHit : public TObject {
  public:
    /** Standard constructor */
    DATCONMostProbableHit() : m_sensorID(), m_localCoordinate(), m_qualityOfHit(0) {}

    /** Constructor for extrapolated hit (MPH) providing sensor ID and local coordinate */
    DATCONMostProbableHit(VxdID sensorID, TVector2 localCoordinate, double qualityOfHit):
      m_sensorID(sensorID), m_localCoordinate(localCoordinate), m_qualityOfHit(qualityOfHit) {}

    /** Basic destructor */
    ~DATCONMostProbableHit() {}

    /** Get sensor ID */
    VxdID getSensorID()           { return m_sensorID; }
    /** Get local coordinate */
    TVector2 getLocalCoordinate() { return m_localCoordinate; }
    /** Get quality of hit */
    double getQualityOfHit()      { return m_qualityOfHit; }

  private:
    /** Sensor ID of the MPH */
    VxdID m_sensorID;
    /** Local coordinate on the sensor of the MPH */
    TVector2 m_localCoordinate;
    /** Quality of the extrapolated hit */
    double m_qualityOfHit;

    ClassDef(DATCONMostProbableHit, 2)

  }; // class DATCONMostProbableHit

} // end namespace Belle2
