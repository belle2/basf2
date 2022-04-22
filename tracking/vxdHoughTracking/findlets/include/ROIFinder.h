/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>
#include <framework/datastore/StoreArray.h>
#include <framework/geometry/B2Vector3.h>

#include <string>
#include <vector>

namespace Belle2 {
  class ModuleParamList;
  class SpacePointTrackCand;

  namespace vxdHoughTracking {

    /**
    * Findlet for performing the simple SVDHoughTracking ROI calculation.
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
      bool m_calculateROI = false;

      // StoreArrays
      /// Name of the PXDIntercepts StoreArray
      std::string m_storePXDInterceptsName = "SVDHoughPXDIntercepts";
      /// Name of the ROIs StoreArray
      std::string m_storeROIsName = "SVDHoughROIs";
      /// PXDIntercepts StoreArray
      StoreArray<PXDIntercept> m_storePXDIntercepts;
      /// ROIs StoreArray
      StoreArray<ROIid> m_storeROIs;


      /// Refit the tracks with m_ROIFitMethod
      bool m_refit = true;
      /// Add a virtual IP for the refit?
      bool m_addVirtualIP = true;
      /// Refit with this estimator, options are circleFit, tripletFit, helixFit
      std::string m_ROIFitMethod = "helixFit";

      /// pointer to the selected QualityEstimator
      std::unique_ptr<QualityEstimatorBase> m_estimator;


      //  Extrapolation parameters
      /// Allowed tolerance (in radians) phi to create intercepts per sensor
      double m_tolerancePhi = 0.15;
      /// Allowed tolerance (in cm) in z to create intercepts per sensor
      double m_toleranceZ = 0.5;

      //  The extrapolation has two charge dependent biases:
      //  a) the residuals show a ~1/R bias, that is larger for low pT = small track radii
      //  b) the residuals show a larger sin(phi) and a smaller cos(phi modulation)
      //  Both of these can be corrected for
      /// Correction factor for radial bias for L1: factor * charge / radius
      double m_radiusCorrectionFactorL1 = -2.0;
      /// Correction factor for radial bias for L2: factor * charge / radius
      double m_radiusCorrectionFactorL2 = -5.0;
      /// Correction factor for the sin(phi) modulation
      double m_sinPhiCorrectionFactor = 0.0;
      /// Correction factor for the cos(phi) modulation
      double m_cosPhiCorrectionFactor = 0.0;

      /// Correction factor for the z position
      double m_zPositionCorrectionFactor = 1.0;


      //  ROI calculation parameters
      /// Minimum size of ROI in u-direction on L1 in pixel
      double m_minimumROISizeUL1 = 40;
      /// Minimum size of ROI in v-direction on L1 in pixel
      double m_minimumROISizeVL1 = 40;
      /// Minimum size of ROI in u-direction on L2 in pixel
      double m_minimumROISizeUL2 = 35;
      /// Minimum size of ROI in v-direction on L2 in pixel
      double m_minimumROISizeVL2 = 30;

      /// Multiplier term in ROI size estimation
      /// For u: size = multiplier * 1/R + minimumROISize
      /// For v: size = (1 + abs(tan(lambda)) * multiplier) + minimumROISize
      /// Multiplier term for u-direction on L1
      double m_multiplierUL1 = 500;
      /// Multiplier term for u-direction on L2
      double m_multiplierUL2 = 600;
      /// Multiplier term for v-direction on L1
      double m_multiplierVL1 = 0.8;
      /// Multiplier term for v-direction on L2
      double m_multiplierVL2 = 0.8;

      /// maximum ROI size in u in pixel
      unsigned short m_maximumROISizeU = 100;
      /// maximum ROI size in v in pixel
      unsigned short m_maximumROISizeV = 100;

      //  Constants used during extrapolation to PXD and ROI calculation
      // ATTENTION: hard coded values taken and derived from pxd/data/PXD-Components.xml
      /// Shift of the center of the active area of each sensor in a ladder of layer 1
      /// For use of mhp_z > (lengh/-2)+shiftZ &&  mhp_z < (lengh/2)+shiftZ
      const double c_centerZShiftLayer1[2] = {3.68255, -0.88255};
      /// Shift of the center of the active area of each sensor in a ladder of layer 2
      /// For use of mhp_z > (lengh/-2)+shiftZ &&  mhp_z < (lengh/2)+shiftZ
      const double c_centerZShiftLayer2[2] = {5.01455, -1.21455};
      /// PXD is shifted to create the windmill structure.
      /// Minimum y coordinate if x-axis is perpendicular to the sensor:
      const double c_sensorMinY = -0.36;
      /// Maximum y coordinate if x-axis is perpendicular to the sensor:
      const double c_sensorMaxY =  0.89;
      /// PXD sensors in L1 and L2 have the same size in u direction (=width):
      const double c_activeSensorWidth = (c_sensorMaxY - c_sensorMinY);
      /// Shift of the center position in y if the x-axis is perpendicular to the sensor:
      const double c_shiftY = (c_sensorMaxY + c_sensorMinY) / 2.0;
      /// Radius of the two layers
      const double c_layerRadius[2] = {1.42854, 2.21218};
      /// Length of the active region for L1 and L2
      const double c_activeSensorLength[2] = {4.48, 6.144};
      /// Phi values of the ladders of L1
      const double c_ladderPhiL1[8] = {0., 0.25 * M_PI, M_PI_2, 0.75 * M_PI, M_PI, -0.75 * M_PI, -M_PI_2, -0.25 * M_PI};
      /// Phi values of the ladders of L2
      const double c_ladderPhiL2[12] = {0., 1. / 6. * M_PI, 1. / 3. * M_PI,  M_PI_2, 2. / 3. * M_PI, 5. / 6. * M_PI,  M_PI, -5. / 6. * M_PI, -2. / 3. * M_PI, -M_PI_2, -1. / 3. * M_PI, -1. / 6. * M_PI};

      /// BField in Tesla
      double m_bFieldZ = 1.5;


      /// B2Vector3D actually contining the BeamSpot position. This will be used as the starting point of the extrapolation.
      B2Vector3D m_BeamSpotPosition;
      /// B2Vector3D actually contining the BeamSpot position error.
      B2Vector3D m_BeamSpotPositionError;

    };

  }
}
