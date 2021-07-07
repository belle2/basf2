/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/vxdHoughTracking/entities/VXDHoughState.h>
#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <vxd/dataobjects/VxdID.h>

#include <string>
#include <vector>

namespace Belle2 {
  class SpacePoint;

  namespace vxdHoughTracking {

    /**
    * Select hits to be analysed in the Hough Space intercept finder for a given layer 6 sensor based on the simple sensor friend map.
    */
    class HitSelector : public TrackFindingCDC::Findlet<VXDHoughState, const VxdID, VXDHoughState*> {

    public:
      /// Load the hits in a sensor friend list for a given L6 sensor from hits and store them in selectedHits, which then are used for the Hough trafo and intercept finding
      void apply(std::vector<VXDHoughState>& hits, const std::vector<VxdID>& friendSensorList,
                 std::vector<VXDHoughState*>& selectedHits) override
      {
        const unsigned short sensorInLayerSixLadder = friendSensorList.back().getSensorNumber();

        for (auto& hit : hits) {

          const VXDHoughState::DataCache& hitData = hit.getDataCache();
          const VxdID& currentHitSensorID = hitData.sensorID;

          if (std::find(friendSensorList.begin(), friendSensorList.end(), currentHitSensorID) == friendSensorList.end()) {
            continue;
          }

          const double hitZPosition = hitData.z;

          if (sensorInLayerSixLadder == 1 and hitZPosition >= -1.0) {
            selectedHits.emplace_back(&hit);
          } else if (sensorInLayerSixLadder == 5 and hitZPosition <= 1.0) {
            selectedHits.emplace_back(&hit);
          }
        }
      }
    };

  }
}
