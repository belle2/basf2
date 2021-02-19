/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
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
        const unsigned short hitLayer = hitData.layer;
        const double hitZPosition = hitData.z;

        // The hitZPosition cuts are based on simple geometrical calculations.
        // Take the gap between two L6 sensors, draw a straight line to the origin, check the intercepts with the other layers,
        // and add / subtract {5, 7.5, 10} mm for L3, L4, L5 to account for a straight line not perfectly representing the boarders,
        // slight misalignment, boost, etc so that there is some overlap
        if (hitLayer < 6) {
          switch (sensorInLayerSixLadder) {
            case 1:
              if (hitLayer == 3 && hitZPosition > 6.56) {
                selectedHits.emplace_back(&hit);
              } else if (hitLayer == 4 && hitZPosition > 13.74) {
                selectedHits.emplace_back(&hit);
              } else if (hitLayer == 5 && hitZPosition > 17.84) {
                selectedHits.emplace_back(&hit);
              }
              break;
            case 2:
              if (hitLayer == 3 && hitZPosition > 2.95 && hitZPosition < 7.56) {
                selectedHits.emplace_back(&hit);
              } else if (hitLayer == 4 && hitZPosition > 6.33 && hitZPosition < 15.24) {
                selectedHits.emplace_back(&hit);
              } else if (hitLayer == 5 && hitZPosition > 8.21 && hitZPosition < 19.84) {
                selectedHits.emplace_back(&hit);
              }
              break;
            case 3:
              if (hitLayer == 3 && hitZPosition > -0.66 && hitZPosition < 3.95) {
                selectedHits.emplace_back(&hit);
              } else if (hitLayer == 4 && hitZPosition > -1.08 && hitZPosition < 7.83) {
                selectedHits.emplace_back(&hit);
              } else if (hitLayer == 5 && hitZPosition > -1.42 && hitZPosition < 10.21) {
                selectedHits.emplace_back(&hit);
              }
              break;
            case 4:
              if (hitLayer == 3 && hitZPosition > -4.27 && hitZPosition < 0.34) {
                selectedHits.emplace_back(&hit);
              } else if (hitLayer == 4 && hitZPosition > -8.48 && hitZPosition < 0.42) {
                selectedHits.emplace_back(&hit);
              } else if (hitLayer == 5 && hitZPosition > -11.05 && hitZPosition < 0.58) {
                selectedHits.emplace_back(&hit);
              }
              break;
            case 5:
              if (hitLayer == 3  && hitZPosition < -3.27) {
                selectedHits.emplace_back(&hit);
              } else if (hitLayer == 4  && hitZPosition < -6.98) {
                selectedHits.emplace_back(&hit);
              } else if (hitLayer == 5 && hitZPosition < -9.05) {
                selectedHits.emplace_back(&hit);
              }
              break;
          }
        } else {
          // we already know that this is the correct L6 hit because of the std::find above
          selectedHits.emplace_back(&hit);
        }
      }
    }

  };
}
