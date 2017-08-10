/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <pxd/geometry/SensorInfo.h>

#include <genfit/DetPlane.h>
#include <genfit/SharedPlanePtr.h>

namespace Belle2 {
  /** ROIDetPlane describes the plane containing a sensor.
   */

  class ROIDetPlane : public genfit::DetPlane {

  public:
    ROIDetPlane() {};

    ROIDetPlane(const VxdID& sensorInfo, double toleranceZ, double tolerancePhi); /**< the constructor*/

    ROIDetPlane(const VxdID& sensorInfo); /**< the constructo - needed for testbeam package*/

    virtual ~ROIDetPlane();

    /**return the sensor ID*/
    VxdID getSensorInfo() {return m_sensorInfo;};

    bool isSensorInRange(TVector3 trackPosition, int layer);

    void setSharedPlanePtr(genfit::SharedPlanePtr spp) {m_sharedPlanePtr = spp;};
    genfit::SharedPlanePtr getSharedPlanePtr() {return m_sharedPlanePtr;};

  private:

    double m_sensorPhiMin;
    double m_sensorPhiMax;
    double m_sensorZMin;
    double m_sensorZMax;
    int m_layer;

    VxdID m_sensorInfo; /**< the sensor ID*/


    genfit::SharedPlanePtr m_sharedPlanePtr; /**< the shared plane pointer*/

  };
}
