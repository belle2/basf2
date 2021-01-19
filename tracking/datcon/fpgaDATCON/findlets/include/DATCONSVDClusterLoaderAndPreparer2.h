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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>

namespace Belle2 {
  class SVDCluster;
  class VxdID;

  class ModuleParamList;

  /**
   * Findlet for loading the seeds from the data store.
   * Also, the tracks are fitted and only the fittable tracks are passed on.
   *
   * If a direction != "invalid" is given, the relations of the tracks to the given store array are checked.
   * If there is a relation with the weight equal to the given direction (meaning there is already a
   * partner for this direction), the track is not passed on.
   */
  class DATCONSVDClusterLoaderAndPreparer2 : public
    TrackFindingCDC::Findlet<SVDCluster, SVDCluster, std::pair<VxdID, std::pair<long, long>>,
        std::pair<VxdID, std::pair<long, long>>> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<SVDCluster, SVDCluster, std::pair<VxdID, std::pair<long, long>>,
          std::pair<VxdID, std::pair<long, long>>>;

  public:
    /// Add the subfindlets
    DATCONSVDClusterLoaderAndPreparer2();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load in the reco tracks and the hits
    void apply(std::vector<SVDCluster>& uClusters, std::vector<SVDCluster>& vClusters,
               std::vector<std::pair<VxdID, std::pair<long, long>>>& uHits,
               std::vector<std::pair<VxdID, std::pair<long, long>>>& vHits) override;

  private:
    // Parameters

    std::array<int, 8> nClusterPerLayer = {0};

    // ATTENTION: all the values below are hardcoded and taken from svd/data/SVD-Components.xml
    const std::array<int, 4> svdRadii = {38990, 80000, 104000, 135150};    // in µm
    const std::array<int, 4> rPhiShiftsOfLayers = { -4680, -10780, -11036, -19076};    // in µm
//     const float initialAngle[4] = { -0.310755873318, 0.1396263402, -0.1396263402, -0.0698131701};    // in rad
    const std::array<float, 4> initialAngle = {(342.195 - 360.) / 180. * M_PI, 8. / 180. * M_PI, -8. / 180. * M_PI, -4. / 180. * M_PI};    // in rad
//     const float angleStep[4] = {0.897597901026, 0.628318530718, 0.523598775598, 0.392699081699};    // in rad
    const std::array<float, 4> angleStep = {2. * M_PI / 7., M_PI / 5., M_PI / 6., M_PI / 8.};    // in rad
    const std::array<float, 3> cosSlantedAngles = {cos(0.207694180987), cos(0.279252680319), cos(0.368264472171)};
    const std::array<float, 3> sinSlantedAngles = {sin(0.207694180987), sin(0.279252680319), sin(0.368264472171)};
//     const float slantedRadius[4] = {38.99, 66890, 86500, 112100};    // in µm

    const std::array<int, 2> zShiftL3 = {92350, -32650};    // in µm
    const std::array<int, 3> zShiftL4 = {149042, 24760, -100240};    // in µm
    const std::array<int, 4> zShiftL5 = {233754, 110560, -14440, -139440};    // in µm
    const std::array<int, 5> zShiftL6 = {303471, 182060, 57060, -67940, -192940};    // in µm

    inline long convertToInt(float value, int power)
    {
      long factor = (long)pow(10, power);
      return round(factor * value);
    };

  };
}
