/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/datcon/optimizedDATCON/entities/HitData.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <vxd/dataobjects/VxdID.h>

#include <string>
#include <vector>

namespace Belle2 {
  class SpacePoint;
  /**
   * Select hits to be analysed in the Hough Space intercept finder for a given layer 6 sensor based on the simple sensor friend map.
   */
  class HitSelector : public TrackFindingCDC::Findlet<HitData, VxdID, HitData*> {

  public:
    /// Load the hits in a sensor friend list for a given L6 sensor from hits and store them in selectedHits, which then are used for the Hough trafo and intercept finding
    void apply(std::vector<HitData>& hits, std::vector<VxdID>& friendSensorList,
               std::vector<HitData*>& selectedHits) override
    {
      const unsigned short sensorInLayerSixLadder = friendSensorList.back().getSensorNumber();

      for (auto& hit : hits) {

        const HitData::DataCache& hitData = hit.getDataCache();
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
