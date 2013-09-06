/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROIDETPLANE_H_
#define ROIDETPLANE_H_

#include <GFDetPlane.h>
#include <pxd/geometry/SensorInfo.h>

namespace Belle2 {

  /** ROIDetPlane describes the plane containing a sensor.
   *
   *
   */

  class ROIDetPlane : public GFDetPlane {

  public:

    ROIDetPlane() {};

    ROIDetPlane(const VxdID& sensorInfo);

    ~ROIDetPlane();

    inline VxdID getSensorInfo() {return m_sensorInfo;};

  private:

    VxdID m_sensorInfo;


  };

}

#endif //ROIDETPLANE_H_
