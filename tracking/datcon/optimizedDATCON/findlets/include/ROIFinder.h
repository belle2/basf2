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
#include <framework/geometry/B2Vector3.h>

#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/BeamSpot.h>

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
    /// Calculate ROI in this findlet?
    bool m_param_calculateROI = true;

    // StoreArrays
    /// Name of the PXDIntercepts StoreArray
    std::string m_param_storePXDInterceptsName = "DATCONPXDIntercepts";
    /// Name of the ROIs StoreArray
    std::string m_param_storeROIsName = "DATCONROIs";
    /// PXDIntercepts StoreArray
    StoreArray<PXDIntercept> m_storePXDIntercepts;
    /// ROIs StoreArray
    StoreArray<ROIid> m_storeROIs;


    //  Extrapolation parameters
    /// Allowed tolerance (in radians) phi to create intercepts per sensor
    double m_param_tolerancePhi = 0.15;
    /// Allowed tolerance (in cm) in z to create intercepts per sensor
    double m_param_toleranceZ = 0.5;

    //  The etrapolation has two charge dependent biases:
    //  a) the residuals show a ~1/R bias, that is larger for low pT = small track radii
    //  b) the residuals show a larger sin(phi) and a smaller cos(phi modulation)
    //  Both of these can be corrected for
    /// Correction factor for radial bias: factor * charge / radius
    double m_param_radiusCorrectionFactor = 3.0;
    /// Correction factor for the sin(phi) modulation
    double m_param_sinPhiCorrectionFactor = 0.05;
    /// Correction factor for the cos(phi) modulation
    double m_param_cosPhiCorrectionFactor = 0.02;


    //  ROI calculation parameters
    /// Minimum size of ROI in u-direction on L1
    double m_param_minimumROISizeUL1 = 40;
    /// Minimum size of ROI in v-direction on L1
    double m_param_minimumROISizeVL1 = 40;
    /// Minimum size of ROI in u-direction on L2
    double m_param_minimumROISizeUL2 = 40;
    /// Minimum size of ROI in v-direction on L2
    double m_param_minimumROISizeVL2 = 40;

    /// Multiplier term in ROI size estimation
    /// For u: size = multiplier * 1/R + minimumROISize
    /// For v: size = (1 + abs(tan(lambda)) * multiplier) + minimumROISize
    /// Multiplier term for u-direction on L1
    double m_param_multiplierUL1 = 100;
    /// Multiplier term for u-direction on L2
    double m_param_multiplierUL2 = 100;
    /// Multiplier term for v-direction on L1
    double m_param_multiplierVL1 = 0.5;
    /// Multiplier term for v-direction on L2
    double m_param_multiplierVL2 = 0.5;

    /// maximum ROI size in u
    unsigned short m_param_maximumROISizeU = 120;
    /// maximum ROI size in v
    unsigned short m_param_maximumROISizeV = 120;

    //  Constants used during extrapolation to PXD and ROI calculation
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
    /// PXD sensors in L1 and L2 have the same size in u direction (=width):
    const double m_const_activeSensorWidth = (m_const_sensorMaxY - m_const_sensorMinY);
    /// Shift of the center position in y if the x-axis is perpendicular to the sensor:
    const double m_const_shiftY = (m_const_sensorMaxY + m_const_sensorMinY) / 2.0;
    /// Radius of the two layers
    const double m_const_layerRadius[2] = {1.42854, 2.21218};
    /// Length of the active region for L1 and L2
    const double m_const_activeSensorLength[2] = {4.48, 6.144};
    /// Phi values of the ladders of L1
    const double m_const_ladderPhiL1[8] = {0., 0.25 * M_PI, M_PI_2, 0.75 * M_PI, M_PI, -0.75 * M_PI, -M_PI_2, -0.25 * M_PI};
    /// Phi values of the ladders of L2
    const double m_const_ladderPhiL2[12] = {0., 1. / 6. * M_PI, 1. / 3. * M_PI,  M_PI_2, 2. / 3. * M_PI, 5. / 6. * M_PI,  M_PI, -5. / 6. * M_PI, -2. / 3. * M_PI, -M_PI_2, -1. / 3. * M_PI, -1. / 6. * M_PI};

    /// BField in Tesla
    double m_bFieldZ = 1.5;


    /// BeamSpot from DB
    DBObjPtr<BeamSpot> m_BeamSpotDB;
    /// Actual BeamSpot
    BeamSpot m_BeamSpot;
    /// B2Vector3D actually contining the BeamSpot position. This will be used as the starting point of the extrapolation.
    B2Vector3D m_BeamSpotPosition;

  };
}
