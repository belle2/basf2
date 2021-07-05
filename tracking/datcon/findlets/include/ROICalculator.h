/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>

namespace Belle2 {
  class ModuleParamList;
  class VxdID;
  class ROIid;

  /**
   * Findlet to calculate ROI on the PXD sensors based on input hits
   */
  class ROICalculator : public TrackFindingCDC::Findlet<const std::pair<VxdID, long>, const std::pair<VxdID, long>> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const std::pair<VxdID, long>, const std::pair<VxdID, long>>;

  public:
    /// Find intercepts in the 2D Hough space
    ROICalculator();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load in the prepared hits and create tracks for extrapolation to PXD
    void apply(const std::vector<std::pair<VxdID, long>>& uExtrapolations,
               const std::vector<std::pair<VxdID, long>>& vExtrapolations) override;

  private:

    /// name of the PXDIntercept StoreArray
    std::string m_param_ROIsStoreArrayName = "DATCONFPGAROIs";
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
