/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vxd/geometry/SensorInfoBase.h>

#include <list>

namespace Belle2 {
  class ROIDetPlane;

  /** This class appends the VXDIntercept infos of a track to the list of intercepts.
   */
  class ROIGeometry {

  public:
    /** Constructor.
     */
    ROIGeometry();

    /** Destructor.
     */
    ~ROIGeometry();

    std::list<ROIDetPlane> m_planeList; /**< list of planes*/

    void fillPlaneList(double toleranceZ, double tolerancePhi,
                       const VXD::SensorInfoBase::SensorType det); /**< fill the list of planes*/

    /** Append to selectedPlanes those in the internal list for which a sensor is near the RecoTrack position */
    void appendSelectedPlanes(std::list<ROIDetPlane>* selectedPlanes, ROOT::Math::XYZVector recoTrackPosition, int layer);

  };
}

