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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

// #include <tracking/spacePointCreation/SpacePoint.h>
// #include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>

namespace Belle2 {
  class ModuleParamList;
  class SpacePointTrackCand;

  /**
   * Findlet for performing the DATCON ROI calculation.
   */
  class DATCONROIFinder : public TrackFindingCDC::Findlet<const SpacePointTrackCand> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const SpacePointTrackCand>;

  public:
    /// Constructor for adding the subfindlets
    DATCONROIFinder();

    /// Default desctructor
    ~DATCONROIFinder();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Function to call all the sub-findlets
    void apply(const std::vector<SpacePointTrackCand>& finalTracks) override;

    /// Initialize the StoreArrays
    void initialize() override;

    /// Clear the object pools
    void beginEvent() override;

  private:
    // StoreArrays
    /// Name of the PXDIntercepts StoreArray
    std::string m_param_storePXDInterceptsName = "DATCONPXDIntercepts";
    /// Name of the ROIs StoreArray
    std::string m_param_storeROIsName = "DATCONROIs";
    /// PXDIntercepts StoreArray
    StoreArray<PXDIntercept> m_storePXDIntercepts;
    /// ROIs StoreArray
    StoreArray<ROIid> m_storeROIs;

    const double m_const_centerZShiftLayer1[2] = {3.68255, -0.88255};    // for use of mhp_z > (lengh/-2)+shiftZ &&  mhp_z < (lengh/2)+shiftZ      // ATTENTION: hard coded values taken and derived from pxd/data/PXD-Components.xml
    const double m_const_centerZShiftLayer2[2] = {5.01455, -1.21455};    // for use of mhp_z > (lengh/-2)+shiftZ &&  mhp_z < (lengh/2)+shiftZ      // ATTENTION: hard coded values taken and derived from pxd/data/PXD-Components.xml
    const double m_const_sensorMinY =
      -0.36;              // ATTENTION: hard coded values taken and derived from pxd/data/PXD-Components.xml
    const double m_const_sensorMaxY =
      0.89;                 // ATTENTION: hard coded values taken and derived from pxd/data/PXD-Components.xml
    const double m_const_shiftY = (m_const_sensorMaxY + m_const_sensorMinY) / 2.0;
    const double m_const_layerRadius[2] = {1.42854, 2.21218};      // ATTENTION: hard coded values from pxd/data/PXD-Components.xml
    const double m_const_sensorLength[2] = {4.48, 6.144};

  };
}
