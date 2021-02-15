/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Qingyuan Liu                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/utilities/PXDUtilities.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace PXD {

    std::shared_ptr<TrackState> getTrackStateOnModule(
      const VXD::SensorInfoBase& pxdSensorInfo,
      RecoTrack& recoTrack, double lambda)
    {
      // get sensor plane, always enable alignment.
      auto centerP = pxdSensorInfo.pointToGlobal(TVector3(0, 0, 0), true);
      auto normalV = pxdSensorInfo.vectorToGlobal(TVector3(0, 0, 1), true);
      genfit::SharedPlanePtr sensorPlaneSptr(new genfit::DetPlane(centerP, normalV));

      // genfit track and measured state on plane
      const genfit::Track& gfTrack = RecoTrackGenfitAccess::getGenfitTrack(recoTrack);
      auto statePtr = std::make_shared<TrackState>();

      try {
        *statePtr = gfTrack.getFittedState();
        lambda = statePtr->extrapolateToPlane(sensorPlaneSptr);
      }  catch (...) {
        B2DEBUG(20, "extrapolation to plane failed! Lambda = " << lambda);
        return std::shared_ptr<TrackState>(nullptr);
      }
      auto intersec = pxdSensorInfo.pointToLocal(statePtr->getPos(), true);

      // check if the intersection is inside (no tolerance).
      double tolerance = 0.0;
      bool inside = pxdSensorInfo.inside(intersec.X(), intersec.Y(), tolerance, tolerance);
      if (!inside) return std::shared_ptr<TrackState>(nullptr);

      return statePtr;
    }

  } // end namespace PXD
} // end namespace Belle2
