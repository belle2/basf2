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
  class ROIFinder : public TrackFindingCDC::Findlet<const SpacePointTrackCand> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<const SpacePointTrackCand>;

  public:
    /// Constructor for adding the subfindlets
    ROIFinder();

    /// Default desctructor
    ~ROIFinder();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Function to call all the sub-findlets
    void apply(const std::vector<SpacePointTrackCand>& finalTracks) override;

    /// Initialize the StoreArrays
    void initialize() override;

    /// Initialize the BField
    void beginRun() override;

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


    /// Minimum size of ROI in u-direction on L1
    unsigned short m_param_minimumROISizeUL1 = 40;
    /// Minimum size of ROI in v-direction on L1
    unsigned short m_param_minimumROISizeVL1 = 40;
    /// Minimum size of ROI in u-direction on L2
    unsigned short m_param_minimumROISizeUL2 = 40;
    /// Minimum size of ROI in v-direction on L2
    unsigned short m_param_minimumROISizeVL2 = 40;

    /// Multiplier term in ROI size estimation
    /// size = multiplier * 1/R + minimumROISize
    /// Multiplier term for u-direction on L1
    double m_param_multiplierUL1 = 100;
    /// Multiplier term for v-direction on L1
    double m_param_multiplierVL1 = 100;
    /// Multiplier term for u-direction on L2
    double m_param_multiplierUL2 = 100;
    /// Multiplier term for v-direction on L2
    double m_param_multiplierVL2 = 100;

    /// allowed overlap (in cm) in u (=r-phi)
    double m_param_overlapU = 0.2;
    /// allowed overlap (in cm) in v (= z / theta)
    double m_param_overlapV = 0.2;

    // ATTENTION: hard coded values taken and derived from pxd/data/PXD-Components.xml
    /// Shift of the center of the active area of each sensor in a ladder of layer 1
    /// For use of mhp_z > (lengh/-2)+shiftZ &&  mhp_z < (lengh/2)+shiftZ
    const double m_const_centerZShiftLayer1[2] = {3.68255, -0.88255};
    /// Shift of the center of the active area of each sensor in a ladder of layer 2
    /// For use of mhp_z > (lengh/-2)+shiftZ &&  mhp_z < (lengh/2)+shiftZ
    const double m_const_centerZShiftLayer2[2] = {5.01455, -1.21455};
    /// PXD is shifted to create the windmill structure.
    /// Minimum y coordinate if x-axis is perpendicular to the sensor:
    const double m_const_sensorMinY = -0.36;
    /// Maximum y coordinate if x-axis is perpendicular to the sensor:
    const double m_const_sensorMaxY =  0.89;
    /// Shift of the center position in y if the x-axis is perpendicular to the sensor:
    const double m_const_shiftY = (m_const_sensorMaxY + m_const_sensorMinY) / 2.0;
    /// Radius of the two layers
    const double m_const_layerRadius[2] = {1.42854, 2.21218};
    /// Length of the active region for L1 and L2
    const double m_const_activeSensorLength[2] = {4.48, 6.144};

    /// BField in Tesla
    double m_bFieldZ = 1.5;

  };
}
