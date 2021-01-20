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
  class VxdID;
  class ROIid;

  class ModuleParamList;

  /**
   * Findlet to calculate ROI on the PXD sensors based on input hits
   */
  class ROICalculator : public TrackFindingCDC::Findlet<std::pair<VxdID, long>, std::pair<VxdID, long>> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<std::pair<VxdID, long>, std::pair<VxdID, long>>;

  public:
    /// Find intercepts in the 2D Hough space
    ROICalculator();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load in the prepared hits and create tracks for extrapolation to PXD
    void apply(std::vector<std::pair<VxdID, long>>& uExtrapolations, std::vector<std::pair<VxdID, long>>& vExtrapolations) override;

  private:

//     /// convert float to long int for more similarity to the FPGA implementation
//     /// @param value to be converted
//     /// @param power multiply value by 10^power
//     inline long convertToInt(double value, int power)
//     {
//       long factor = (long)pow(10, power);
//       return round(factor * value);
//     }

    /// name of the PXDIntercept StoreArray
    std::string m_param_ROIsStoreArrayName = "DATCONROIs";
    /// PXDIntercept StoreArray
    StoreArray<ROIid> m_storeDATCONROIs;

    /// ROI size in u direction on L1
    short m_param_uROIsizeL1 = 40;
    /// ROI size in u direction on L2
    short m_param_uROIsizeL2 = 40;
    /// ROI size in v direction on L1
    short m_param_vROIsizeL1 = 40;
    /// ROI size in v direction on L2
    short m_param_vROIsizeL2 = 40;

  };
}
