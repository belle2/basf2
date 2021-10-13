/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/utilities/PXDUtilities.h>
#include <framework/logging/Logger.h>
#include <pxd/reconstruction/PXDPixelMasker.h>

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

    bool isCloseToBorder(int u, int v, int checkDistance)
    {

      if (u - checkDistance < 0 || u + checkDistance >= 250 ||
          v - checkDistance < 0 || v + checkDistance >= 768) {
        return true;
      }
      return false;
    }

    bool isDefectivePixelClose(int u, int v, int checkDistance, const VxdID& moduleID)
    {

      //Iterate over square around the intersection to see if any close pixel is dead
      for (int u_iter = u - checkDistance; u_iter <= u + checkDistance ; ++u_iter) {
        for (int v_iter = v - checkDistance; v_iter <= v + checkDistance ; ++v_iter) {
          if (PXDPixelMasker::getInstance().pixelDead(moduleID, u_iter, v_iter)
              || !PXDPixelMasker::getInstance().pixelOK(moduleID, u_iter, v_iter)) {
            return true;
          }
        }
      }
      return false;
    }
  } // end namespace PXD
} // end namespace Belle2
