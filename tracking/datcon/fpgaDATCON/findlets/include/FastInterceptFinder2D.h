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
  class FastInterceptFinder2D : public TrackFindingCDC::Findlet<std::pair<VxdID, std::pair<long, long>>> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<std::pair<VxdID, std::pair<long, long>>>;

  public:
    /// Add the subfindlets
    FastInterceptFinder2D();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load in the reco tracks and the hits
    void apply(std::vector<std::pair<VxdID, std::pair<long, long>>>& hits) override;

  private:

    inline long convertToInt(double value, int power)
    {
      long factor = (long)pow(10, power);
      return round(factor * value);
    };

    inline unsigned short layerFilter(bool* layer)
    {
      unsigned short layercount = 0;
      /* Count number of found layers */
      for (unsigned short i = 0; i < 4; ++i) {
        if (layer[i] == true)  layercount++;
      }
      // return true, if lines in sector are from at least minLines different layers
      if (layercount >= 3) {
        return layercount;
      }
      return 0;
    }

    // Parameters

    void fastInterceptFinder2d(std::vector<std::pair<VxdID, std::pair<long, long>>>& hits,
                               uint xmin, uint xmax, uint ymin, uint ymax, uint iterations);

    uint m_maxRecursionLevel = 6;

    uint m_nAngleSectors = 256;

    uint m_nVerticalSectors = 256;

    long m_verticalHoughSpaceSize = convertToInt(1, 6);

    double m_minimumX = -3.168;
    double m_maximumX = 3.168;

    std::array<long, 16385> m_HSSinValuesLUT = {0};
    std::array<long, 16385> m_HSCosValuesLUT = {0};
    std::array<long, 16384> m_HSCenterSinValuesLUT = {0};
    std::array<long, 16384> m_HSCenterCosValuesLUT = {0};
    std::array<long, 16385> m_HSYLUT = {0};
    std::array<long, 16384> m_HSYCenterLUT = {0};
    std::array<double, 16385> m_HSXLUT = {0};
    std::array<double, 16384> m_HSXCenterLUT = {0};

  };
}
