/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/vxdHoughTracking/findlets/ROIFinder.h>
#include <tracking/vxdHoughTracking/findlets/RawTrackCandCleaner.icc.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <framework/logging/Logger.h>
#include <framework/core/ModuleParamList.h>
#include <framework/geometry/BFieldManager.h>
#include <framework/geometry/B2Vector3.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace vxdHoughTracking;

ROIFinder::~ROIFinder() = default;

ROIFinder::ROIFinder()
{
}

void ROIFinder::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "calculateROI"), m_calculateROI,
                                "Calculate PXDIntercepts and ROIs in this findlet based on a simple circle extrapolation (r-phi) and straigh line extrapolation (z, theta)?",
                                m_calculateROI);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "storePXDInterceptsName"), m_storePXDInterceptsName,
                                "Name of the PXDIntercepts StoreArray produced by SVDHoughTracking using a simple circle extrapolation in r-phi and a straight line extrapolation in theta.",
                                m_storePXDInterceptsName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "storeROIsName"), m_storeROIsName,
                                "Name of the ROIs StoreArray produced by SVDHoughTracking using a simple circle extrapolation in r-phi and a straight line extrapolation in theta.",
                                m_storeROIsName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "tolerancePhi"), m_tolerancePhi,
                                "Allowed tolerance in phi (in radians) (for ROI calculation in u direction). If the intercept is within this range of the active region, an intercept is created.",
                                m_tolerancePhi);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "toleranceZ"), m_toleranceZ,
                                "Allowed tolerance in z (in cm) (for ROI calculation in v direction). If the intercept is within this range of the active region of a sensor, an intercept is created.",
                                m_toleranceZ);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "radiusCorrectionFactorL1"), m_radiusCorrectionFactorL1,
                                "Correct charge-dependent bias of the extrapolated hit on L1 with radiusCorrectionFactor * trackCharge / trackRadius.",
                                m_radiusCorrectionFactorL1);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "radiusCorrectionFactorL2"), m_radiusCorrectionFactorL2,
                                "Correct charge-dependent bias of the extrapolated hit on L2 with radiusCorrectionFactor * trackCharge / trackRadius.",
                                m_radiusCorrectionFactorL2);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "sinPhiCorrectionFactor"), m_sinPhiCorrectionFactor,
                                "Correct sin(trackPhi) dependent bias with sinPhiCorrectionFactor * sin(trackPhi).",
                                m_sinPhiCorrectionFactor);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "cosPhiCorrectionFactor"), m_cosPhiCorrectionFactor,
                                "Correct cos(trackPhi) dependent bias with cosPhiCorrectionFactor * cos(trackPhi).",
                                m_cosPhiCorrectionFactor);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "zPositionCorrectionFactor"), m_zPositionCorrectionFactor,
                                "Correction factor for the extrapolated z position.",
                                m_zPositionCorrectionFactor);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimumROISizeUL1"), m_minimumROISizeUL1,
                                "Minimum ROI size (in pixel) in u direction on L1.", m_minimumROISizeUL1);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimumROISizeVL1"), m_minimumROISizeVL1,
                                "Minimum ROI size (in pixel) in v direction on L1.", m_minimumROISizeVL1);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimumROISizeUL2"), m_minimumROISizeUL2,
                                "Minimum ROI size (in pixel) in u direction on L2.", m_minimumROISizeUL2);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimumROISizeVL2"), m_minimumROISizeVL2,
                                "Minimum ROI size (in pixel) in v direction on L2.", m_minimumROISizeVL2);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "multiplierUL1"), m_multiplierUL1,
                                "Multiplier term for ROI size estimation for L1 u direction. Usage: multiplierUL1 * 1/R + minimumROISizeUL1, with R in cm.",
                                m_multiplierUL1);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "multiplierUL2"), m_multiplierUL2,
                                "Multiplier term for ROI size estimation for L2 u direction. Usage: multiplierUL2 * 1/R + minimumROISizeUL2, with R in cm.",
                                m_multiplierUL2);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "multiplierVL1"), m_multiplierVL1,
                                "Multiplier term for ROI size estimation for L1 v direction. Usage: (1 + abs(tan(lambda)) * multiplierVL1) + minimumROISizeVL1.",
                                m_multiplierVL1);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "multiplierVL2"), m_multiplierVL2,
                                "Multiplier term for ROI size estimation for L2 v direction. Usage: (1 + abs(tan(lambda)) * multiplierVL2) + minimumROISizeVL2.",
                                m_multiplierVL2);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumROISizeU"), m_maximumROISizeU,
                                "Maximum ROI size in u.", m_maximumROISizeU);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumROISizeV"), m_maximumROISizeV,
                                "Maximum ROI size in v.", m_maximumROISizeV);


  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "refit"), m_refit,
                                "Refit the track with trackQualityEstimationMethod?", m_refit);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "addVirtualIP"), m_addVirtualIP,
                                "Add virtual IP to fit the tracks again for ROI creation?", m_addVirtualIP);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "ROIFitMethod"), m_ROIFitMethod,
                                "Identifier which fit method to use. Valid identifiers are: [circleFit, tripletFit, helixFit]",
                                m_ROIFitMethod);

}

/// Create the store arrays
void ROIFinder::initialize()
{
  // If no ROIs shall be calculated, return rightaway and don't do anything
  if (not m_calculateROI) {
    return;
  }

  Super::initialize();

  m_storePXDIntercepts.registerInDataStore(m_storePXDInterceptsName);
  m_storeROIs.registerInDataStore(m_storeROIsName);

  if (m_minimumROISizeUL1 < 0 or m_minimumROISizeUL2 < 0 or
      m_minimumROISizeVL1 < 0 or m_minimumROISizeVL2 < 0 or
      m_multiplierUL1 < 0 or m_multiplierUL2 < 0 or
      m_multiplierVL1 < 0 or m_multiplierVL2 < 0 or
      m_tolerancePhi < 0 or m_toleranceZ < 0) {
    B2ERROR("Please check the ROI size parameters. "
            "None of minimumROISizeUL1, minimumROISizeUL2, minimumROISizeVL1, minimumROISizeVL2, "
            "multiplierUL1, multiplierUL2, multiplierVL1, multiplierVL2, overlapU, overlapV must be < 0!");
  }

  if (m_ROIFitMethod == "tripletFit") {
    m_estimator = std::make_unique<QualityEstimatorTripletFit>();
  } else if (m_ROIFitMethod == "circleFit") {
    m_estimator = std::make_unique<QualityEstimatorCircleFit>();
  } else if (m_ROIFitMethod == "helixFit") {
    m_estimator = std::make_unique<QualityEstimatorRiemannHelixFit>();
  }
  B2ASSERT("QualityEstimator could not be initialized with method: " << m_ROIFitMethod, m_estimator);

};

void ROIFinder::beginRun()
{
  // If no ROIs shall be calculated, return rightaway and don't do anything
  if (not m_calculateROI) {
    return;
  }

  Super::beginRun();

  m_bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_estimator->setMagneticFieldStrength(m_bFieldZ);

  DBObjPtr<BeamSpot> beamSpotDB;
  if (beamSpotDB.isValid()) {
    m_BeamSpotPosition = (*beamSpotDB).getIPPosition();
    const TMatrixDSym posErr = (*beamSpotDB).getIPPositionCovMatrix();
    m_BeamSpotPositionError.SetXYZ(sqrt(posErr[0][0]), sqrt(posErr[1][1]), sqrt(posErr[2][2]));
  } else {
    m_BeamSpotPosition.SetXYZ(0., 0., 0.);
    m_BeamSpotPositionError.SetXYZ(0., 0., 0.);
  }
}


void ROIFinder::beginEvent()
{
  // If no ROIs shall be calculated, return rightaway and don't do anything
  if (not m_calculateROI) {
    return;
  }

  Super::beginEvent();
}

void ROIFinder::apply(const std::vector<SpacePointTrackCand>& finalTracks)
{
  // If no ROIs shall be calculated, return rightaway and don't do anything
  if (not m_calculateROI) {
    return;
  }

  for (auto& track : finalTracks) {
    // do nothing if the SpacePointTrackCand is not active
    if (!track.hasRefereeStatus(SpacePointTrackCand::c_isActive)) {
      continue;
    }

    QualityEstimationResults refittedTrackEstimate;
    if (m_refit) {
      auto sortedHits = track.getSortedHits();

      if (m_addVirtualIP) {
        SpacePoint virtualIPSpacePoint = SpacePoint(m_BeamSpotPosition, m_BeamSpotPositionError, {0.5, 0.5}, {false, false},
                                                    VxdID(0), Belle2::VXD::SensorInfoBase::VXD);
        sortedHits.push_back(&virtualIPSpacePoint);
      }

      std::sort(sortedHits.begin(), sortedHits.end(),
      [](const SpacePoint * a, const SpacePoint * b) {return a->getPosition().Perp() < b->getPosition().Perp(); });

      refittedTrackEstimate = m_estimator->estimateQualityAndProperties(sortedHits);
    }

    std::vector<PXDIntercept> thisTracksIntercepts;
    thisTracksIntercepts.reserve(8);

    B2Vector3D momentumEstimate = track.getMomSeed();
    if (m_refit) {
      momentumEstimate = *refittedTrackEstimate.p;
    }

    const double trackPhi       = momentumEstimate.Phi();
    const double trackTheta     = momentumEstimate.Theta();
    const double tanTrackLambda = tan(M_PI_2 - trackTheta);
    const double trackRadius    = momentumEstimate.Perp() / (0.00299792458 * m_bFieldZ) ;
    const double trackCharge    = track.getChargeSeed();

    /** Loop over both PXD layer */
    for (int layer = 1; layer <= 2; layer++) {
      const double sensorPerpRadius = c_layerRadius[layer - 1];

      /** Loop over all ladders of layer */
      for (int ladder = 1; ladder <= (layer == 1 ? 8 : 12); ladder++) {
        const double sensorPhi = (layer == 1) ? c_ladderPhiL1[ladder - 1] : c_ladderPhiL2[ladder - 1];
        const double rotatedBeamSpotX =  m_BeamSpotPosition.X() * cos(sensorPhi) + m_BeamSpotPosition.Y() * sin(sensorPhi);
        const double rotatedBeamSpotY = -m_BeamSpotPosition.X() * sin(sensorPhi) + m_BeamSpotPosition.Y() * cos(sensorPhi);

        // in the rotated coordinate the sensor is perpendicular to the x axis at position sensorLocalX
        const double sensorLocalX = sensorPerpRadius - rotatedBeamSpotX;

        double phiDiff = trackPhi - sensorPhi;
        if (phiDiff < -M_PI) {
          phiDiff += 2. * M_PI;
        }
        if (phiDiff > M_PI) {
          phiDiff -= 2. * M_PI;
        }

        // Don't try to extrapolate if track direction and sensor are too different (in phi)
        // this should speed up the calculation as wrong combinations are not checked at all.
        // Two values to check both the outgoing (0.3) and incoming (0.8) arm.
        if (fabs(phiDiff) > 0.25 * M_PI and fabs(phiDiff) < 0.75 * M_PI) {
          continue;
        }
        if (fabs(phiDiff) > 0.75 * M_PI and fabs(tanTrackLambda) > 0.2 /*and fabs(trackRadius) > 20*/) {
          continue;
        }


        // relative phi value of the track center compared to the rotated
        // coordinate system defined by the (rotated) line perpendicular
        // to the sensor (with length sensorPerpRadius)
        double relTrackCenterPhi = 0;
        if (trackCharge < 0) {
          relTrackCenterPhi = trackPhi - M_PI_2 - sensorPhi;
        } else if (trackCharge > 0) {
          relTrackCenterPhi = trackPhi + M_PI_2 - sensorPhi;
        }
        const double xCenter  = trackRadius * cos(relTrackCenterPhi);
        const double yCenter  = trackRadius * sin(relTrackCenterPhi);

        // continue if radicant of sqrt is negative
        if ((trackRadius * trackRadius - (sensorLocalX - xCenter) * (sensorLocalX - xCenter)) < 0) {
          continue;
        }
        const double ytmp     = sqrt(trackRadius * trackRadius - (sensorLocalX - xCenter) * (sensorLocalX - xCenter));
        const double yplus    = yCenter + ytmp + rotatedBeamSpotY;
        const double yminus   = yCenter - ytmp + rotatedBeamSpotY;

        const double correctionterm = ((layer == 1 ? m_radiusCorrectionFactorL1 : m_radiusCorrectionFactorL2) * trackCharge /
                                       trackRadius) +
                                      m_sinPhiCorrectionFactor * sin(trackPhi) +
                                      m_cosPhiCorrectionFactor * cos(trackPhi);

        const double localUPositionPlus   = yplus  - c_shiftY + correctionterm;
        const double localUPositionMinus  = yminus - c_shiftY + correctionterm;
        const double toleranceRPhi        = m_tolerancePhi * sensorLocalX;

        // if the hit for sure is out of reach of the current ladder, continue
        if (not(yplus  >= c_sensorMinY - toleranceRPhi and yplus  <= c_sensorMaxY + toleranceRPhi) and
            not(yminus >= c_sensorMinY - toleranceRPhi and yminus <= c_sensorMaxY + toleranceRPhi)) {
          continue;
        }

        // estimate the z coordinate of the extrapolation on this layer
        const double z      = sensorLocalX * tanTrackLambda - m_BeamSpotPosition.Z();
        const double zPlus  = sqrt(sensorLocalX * sensorLocalX + yplus * yplus)   * tanTrackLambda * m_zPositionCorrectionFactor -
                              m_BeamSpotPosition.Z();
        const double zMinus = sqrt(sensorLocalX * sensorLocalX + yminus * yminus) * tanTrackLambda * m_zPositionCorrectionFactor -
                              m_BeamSpotPosition.Z();

        /** Loop over both modules of a ladder */
        for (int sensor = 1; sensor <= 2; sensor++) {

          const double shiftZ = (layer == 1) ? c_centerZShiftLayer1[sensor - 1] : c_centerZShiftLayer2[sensor - 1];

          double localVPosition       = z - shiftZ;
          double localVPositionPlus   = zPlus - shiftZ;
          double localVPositionMinus  = zMinus - shiftZ;
          // check whether z intersection possibly is on sensor to be checked, only continue with the rest of calculations if that's the case
          if (localVPosition >= ((-c_activeSensorLength[layer - 1] / 2.0) - m_toleranceZ) and
              localVPosition <= ((c_activeSensorLength[layer - 1] / 2.0) + m_toleranceZ)) {

            const VxdID sensorID = VxdID(layer, ladder, sensor);

            // check for first option of the intersection
            if (localUPositionPlus >= -c_activeSensorWidth / 2.0 - toleranceRPhi and
                localUPositionPlus <=  c_activeSensorWidth / 2.0 + toleranceRPhi) {
              PXDIntercept intercept;
              intercept.setCoorU(localUPositionPlus);
              intercept.setCoorV(localVPositionPlus);
              intercept.setVxdID(sensorID);
              m_storePXDIntercepts.appendNew(intercept);
              thisTracksIntercepts.push_back(intercept);
            }
            // check for second option of the intersection
            if (localUPositionMinus >= -c_activeSensorWidth / 2.0 - toleranceRPhi and
                localUPositionMinus <=  c_activeSensorWidth / 2.0 + toleranceRPhi) {
              PXDIntercept intercept;
              intercept.setCoorU(localUPositionMinus);
              intercept.setCoorV(localVPositionMinus);
              intercept.setVxdID(sensorID);
              m_storePXDIntercepts.appendNew(intercept);
              thisTracksIntercepts.push_back(intercept);
            }
          }
        }
      }
    }

    const double omega = 1. / trackRadius;
    unsigned short uSizeL1 = (unsigned short)(m_multiplierUL1 * omega + m_minimumROISizeUL1);
    unsigned short uSizeL2 = (unsigned short)(m_multiplierUL2 * omega + m_minimumROISizeUL2);
    unsigned short vSizeL1 = (unsigned short)((1. + fabs(tanTrackLambda) * m_multiplierVL1) * m_minimumROISizeVL1);
    unsigned short vSizeL2 = (unsigned short)((1. + fabs(tanTrackLambda) * m_multiplierVL2) * m_minimumROISizeVL2);
    for (auto& intercept : thisTracksIntercepts) {
      const VxdID& interceptSensorID = intercept.getSensorID();

      double uCoordinate = intercept.getCoorU();
      double vCoordinate = intercept.getCoorV();

      const PXD::SensorInfo* currentSensor = dynamic_cast<const PXD::SensorInfo*>(&VXD::GeoCache::get(interceptSensorID));

      int interceptUCell = currentSensor->getUCellID(uCoordinate, vCoordinate, false);
      int interceptVCell = currentSensor->getVCellID(vCoordinate, false);
      int nUCells = currentSensor->getUCells();
      int nVCells = currentSensor->getVCells();

      unsigned short uSize = (interceptSensorID.getLayerNumber() == 1 ? uSizeL1 : uSizeL2);
      unsigned short vSize = (interceptSensorID.getLayerNumber() == 1 ? vSizeL1 : vSizeL2);
      if (uSize > m_maximumROISizeU) {
        uSize = m_maximumROISizeU;
      }
      if (vSize > m_maximumROISizeV) {
        vSize = m_maximumROISizeV;
      }

      /** Lower left corner */
      short uCellDownLeft = interceptUCell - uSize / 2;
      short vCellDownLeft = interceptVCell - vSize / 2;

      /** Upper right corner */
      short uCellUpRight = interceptUCell + uSize / 2;
      short vCellUpRight = interceptVCell + vSize / 2;

      if (uCellDownLeft >= nUCells or vCellDownLeft >= nVCells or uCellUpRight < 0 or vCellUpRight < 0) {
        continue;
      }

      if (uCellDownLeft < 0) {
        uCellDownLeft = 0;
      }
      if (vCellDownLeft < 0) {
        vCellDownLeft = 0;
      }

      // minimum cell id is 0, maximum is nCells - 1
      if (uCellUpRight >= nUCells) {
        uCellUpRight = nUCells - 1;
      }
      if (vCellUpRight >= nVCells) {
        vCellUpRight = nVCells - 1;
      }

      m_storeROIs.appendNew(ROIid(uCellDownLeft, uCellUpRight, vCellDownLeft, vCellUpRight, interceptSensorID));
    }

  }
}

