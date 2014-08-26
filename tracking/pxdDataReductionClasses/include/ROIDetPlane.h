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

namespace Belle2 {
  /** ROIDetPlane describes the plane containing a sensor.
   */

  class ROIDetPlane : public genfit::DetPlane {

  public:
    ROIDetPlane() {};

    ROIDetPlane(const VxdID& sensorInfo); /**< the constructor*/

    virtual ~ROIDetPlane();

    /**return the sensor ID*/
    VxdID getSensorInfo() {return m_sensorInfo;};

  private:
    VxdID m_sensorInfo; /**< the sensor ID*/
  };
}
