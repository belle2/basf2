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

#include <vxd/dataobjects/VxdID.h>

#include <genfit/DetPlane.h>
#include <genfit/SharedPlanePtr.h>

#include <TVector3.h>

namespace Belle2 {
  /** ROIDetPlane describes the plane containing a sensor.
   */

  class ROIDetPlane : public genfit::DetPlane {

  public:

    // TODO: Using u and v tolerances seems to be more "natural", or use theta and phi so its consistent for u and v direction
    ROIDetPlane(const VxdID& vxdID, double toleranceZ, double tolerancePhi); /**< the constructor*/

    explicit ROIDetPlane(const VxdID& vxdID); /**< the constructor - needed for testbeam package*/

    // Needed by root. Dont use!
    ROIDetPlane() = default;


    /**return the sensor ID*/
    VxdID getVxdID() {return m_vxdID;};

    /** determine if the sensor is in range */
    bool isSensorInRange(const TVector3& trackPosition, int layer);

    /** assign the shared-plane pointer */
    void setSharedPlanePtr(genfit::SharedPlanePtr spp) {m_sharedPlanePtr = spp;};
    /** return the shared-plane pointer */
    genfit::SharedPlanePtr getSharedPlanePtr() {return m_sharedPlanePtr;};

  private:
    VxdID m_vxdID; /**< the sensor ID*/

    /** these two vectors define the planes going through the upper and lower edge of the sensor */
    TVector3 m_orthoVec_upper; /**< vector normal to the vector from 0,0,0 to a point on the upper edge of the sensor in x-y-plane */
    TVector3 m_orthoVec_lower; /**< vector normal to the vector from 0,0,0 to a point on the lower edge of the sensor in x-y-plane */

    double m_sensorZMin = 0; /**< min of allowed global z range*/
    double m_sensorZMax = 0; /**< max of allowed global z range*/

    int m_layer = 0; /**< layer number */


    genfit::SharedPlanePtr m_sharedPlanePtr; /**< the shared plane pointer*/

  };
}
