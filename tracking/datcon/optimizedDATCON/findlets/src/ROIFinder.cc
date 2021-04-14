/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Christian Wessel                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/findlets/ROIFinder.h>
#include <tracking/datcon/optimizedDATCON/findlets/RawTrackCandCleaner.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/logging/Logger.h>
#include <framework/core/ModuleParamList.h>
#include <framework/geometry/BFieldManager.h>

#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

using namespace Belle2;
using namespace TrackFindingCDC;

ROIFinder::~ROIFinder() = default;

ROIFinder::ROIFinder()
{
}

void ROIFinder::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "calculateROI"), m_param_calculateROI,
                                "Calculate PXDIntercepts and ROIs based on a simple circle extrapolation (r-phi) and straigh line extrapolation (z, theta)?",
                                m_param_calculateROI);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "storePXDInterceptsName"),
                                m_param_storePXDInterceptsName,
                                "Name of the PXDIntercepts StoreArray produced by DATCON using a simple circle extrapolation in r-phi and a straight line extrapolation in theta.",
                                m_param_storePXDInterceptsName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "storeROIsName"),
                                m_param_storeROIsName,
                                "Name of the ROIs StoreArray produced by DATCON using a simple circle extrapolation in r-phi and a straight line extrapolation in theta.",
                                m_param_storeROIsName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "tolerancePhi"), m_param_tolerancePhi,
                                "Allowed tolerance in phi (in radians) (for ROI calculation in u direction). If the intercept is within this range of the active region, an intercept is created.",
                                m_param_tolerancePhi);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "toleranceZ"), m_param_toleranceZ,
                                "Allowed tolerance in z (in cm) (for ROI calculation in v direction). If the intercept is within this range of the active region of a sensor, an intercept is created.",
                                m_param_toleranceZ);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimumROISizeUL1"), m_param_minimumROISizeUL1,
                                "Minimum ROI size (in pixel) in u direction on L1.", m_param_minimumROISizeUL1);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimumROISizeVL1"), m_param_minimumROISizeVL1,
                                "Minimum ROI size (in pixel) in v direction on L1.", m_param_minimumROISizeVL1);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimumROISizeUL2"), m_param_minimumROISizeUL2,
                                "Minimum ROI size (in pixel) in u direction on L2.", m_param_minimumROISizeUL2);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimumROISizeVL2"), m_param_minimumROISizeVL2,
                                "Minimum ROI size (in pixel) in v direction on L2.", m_param_minimumROISizeVL2);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "multiplierUL1"), m_param_multiplierUL1,
                                "Multiplier term for ROI size estimation for L1 u direction. Usage: multiplierUL1 * 1/R + minimumROISizeUL1, with R in cm.",
                                m_param_multiplierUL1);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "multiplierUL2"), m_param_multiplierUL2,
                                "Multiplier term for ROI size estimation for L2 u direction. Usage: multiplierUL2 * 1/R + minimumROISizeUL2, with R in cm.",
                                m_param_multiplierUL2);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "multiplierVL1"), m_param_multiplierVL1,
                                "Multiplier term for ROI size estimation for L1 v direction. Usage: (1 + abs(tan(lambda)) * multiplierVL1) + minimumROISizeVL1.",
                                m_param_multiplierVL1);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "multiplierVL2"), m_param_multiplierVL2,
                                "Multiplier term for ROI size estimation for L2 v direction. Usage: (1 + abs(tan(lambda)) * multiplierVL2) + minimumROISizeVL2.",
                                m_param_multiplierVL2);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumROISizeU"), m_param_maximumROISizeU,
                                "Maximum ROI size in u.", m_param_maximumROISizeU);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumROISizeV"), m_param_maximumROISizeV,
                                "Maximum ROI size in v.", m_param_maximumROISizeV);

}

/// Create the store arrays
void ROIFinder::initialize()
{
  // If no ROIs shall be calculated, return rightaway and don't do anything
  if (not m_param_calculateROI) {
    return;
  }

  Super::initialize();

  m_storePXDIntercepts.registerInDataStore(m_param_storePXDInterceptsName);
  m_storeROIs.registerInDataStore(m_param_storeROIsName);

  if (m_param_minimumROISizeUL1 < 0 or m_param_minimumROISizeUL2 < 0 or
      m_param_minimumROISizeVL1 < 0 or m_param_minimumROISizeVL2 < 0 or
      m_param_multiplierUL1 < 0 or m_param_multiplierUL2 < 0 or
      m_param_multiplierVL1 < 0 or m_param_multiplierVL2 < 0 or
      m_param_tolerancePhi < 0 or m_param_toleranceZ < 0) {
    B2ERROR("Please check the ROI size parameters. "
            "None of minimumROISizeUL1, minimumROISizeUL2, minimumROISizeVL1, minimumROISizeVL2, "
            "multiplierUL1, multiplierUL2, multiplierVL1, multiplierVL2, overlapU, overlapV must be < 0!");
  }
};

void ROIFinder::beginRun()
{
  // If no ROIs shall be calculated, return rightaway and don't do anything
  if (not m_param_calculateROI) {
    return;
  }

  Super::beginRun();

  m_bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
}


void ROIFinder::beginEvent()
{
  // If no ROIs shall be calculated, return rightaway and don't do anything
  if (not m_param_calculateROI) {
    return;
  }

  Super::beginEvent();
}

void ROIFinder::apply(const std::vector<SpacePointTrackCand>& finalTracks)
{
  // If no ROIs shall be calculated, return rightaway and don't do anything
  if (not m_param_calculateROI) {
    return;
  }

  for (auto& track : finalTracks) {
    // do nothing if the SpacePointTrackCand is not active
    if (!track.hasRefereeStatus(SpacePointTrackCand::c_isActive)) {
      continue;
    }

    std::vector<PXDIntercept> thisTracksIntercepts;
    thisTracksIntercepts.reserve(8);

    const B2Vector3D& momentumEstimate = track.getMomSeed();

    const double trackPhi       = momentumEstimate.Phi();
    const double trackTheta     = momentumEstimate.Theta();
    const double tanTrackLambda = tan(M_PI_2 - trackTheta);
    const double trackRadius    = momentumEstimate.Perp() / (0.00299792458 * m_bFieldZ) ;
    const double trackCharge    = track.getChargeSeed();

    /** Loop over both PXD layer */
    for (int layer = 1; layer <= 2; layer++) {
      const double sensorPerpRadius = m_const_layerRadius[layer - 1];
      const double x = sensorPerpRadius;

      /** Loop over all ladders of layer */
      for (int ladder = 1; ladder <= (layer == 1 ? 8 : 12); ladder++) {
        const double sensorPhi = (layer == 1) ? m_const_ladderPhiL1[ladder - 1] : m_const_ladderPhiL2[ladder - 1];
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
        if (fabs(phiDiff) > 0.3 * M_PI /*and fabs(phiDiff) < 0.8 * M_PI*/) {
          continue;
        }

        // relative phi value of the track center compared to the rotated
        // coordinate system defined by the (rotated) line perpendicular
        // to the sensor (with length sensorPerpRadius)
        double relTrackCenterPhi = 0;
//         if (trackCharge == +1) {
//           relTrackCenterPhi = trackPhi - M_PI_2 - sensorPhi;
//         } else if (trackCharge == -1) {
//           relTrackCenterPhi = trackPhi + M_PI_2 - sensorPhi;
//         }
        if (trackCharge == -1) {
          relTrackCenterPhi = trackPhi - M_PI_2 - sensorPhi;
        } else if (trackCharge == +1) {
          relTrackCenterPhi = trackPhi + M_PI_2 - sensorPhi;
        }
        const double xCenter  = trackRadius * cos(relTrackCenterPhi);
        const double yCenter  = trackRadius * sin(relTrackCenterPhi);
        if ((trackRadius * trackRadius - (x - xCenter) * (x - xCenter)) < 0) {
          continue;
        }
        const double ytmp     = sqrt(trackRadius * trackRadius - (x - xCenter) * (x - xCenter));
        const double yplus    = yCenter + ytmp;
        const double yminus   = yCenter - ytmp;
        const double localUPositionPlus   = yplus  - m_const_shiftY;
        const double localUPositionMinus  = yminus - m_const_shiftY;
        const double toleranceRPhi = m_param_tolerancePhi * sensorPerpRadius;

        // if the hit for sure is out of reach of the current ladder, continue
        if (not(yplus  >= m_const_sensorMinY - toleranceRPhi and yplus  <= m_const_sensorMaxY + toleranceRPhi) and
            not(yminus >= m_const_sensorMinY - toleranceRPhi and yminus <= m_const_sensorMaxY + toleranceRPhi)) {
          continue;
        }

        // estimate the z coordinate of the extrapolation on this layer
        const double z = sensorPerpRadius * tanTrackLambda; // tan(trackLambda);

        /** Loop over both modules of a ladder */
        for (int sensor = 1; sensor <= 2; sensor++) {

          const double shiftZ = (layer == 1) ? m_const_centerZShiftLayer1[sensor - 1] : m_const_centerZShiftLayer2[sensor - 1];

          double localVPosition = z - shiftZ;
          // check whether z intersection possibly is on sensor to be checked, only continue with the rest of calculations if that's the case
          if (localVPosition >= ((m_const_activeSensorLength[layer - 1] / -2.0) - m_param_toleranceZ) and
              localVPosition <= ((m_const_activeSensorLength[layer - 1] /  2.0) + m_param_toleranceZ)) {
            // // check whether z intersection possibly is on sensor to be checked, only continue with the rest of calculations if that's the case
            // if (z >= ((m_const_activeSensorLength[layer - 1] / -2.0) - m_param_toleranceZ + shiftZ) and
            //     z <= ((m_const_activeSensorLength[layer - 1] /  2.0) + m_param_toleranceZ + shiftZ)) {

            const VxdID sensorID = VxdID(layer, ladder, sensor);

            // double localVPosition = z - shiftZ;
            // check for first option of the intersection
            if (yplus >= m_const_sensorMinY - toleranceRPhi and yplus <= m_const_sensorMaxY + toleranceRPhi) {
              PXDIntercept intercept;
              intercept.setCoorU(localUPositionPlus);
              intercept.setCoorV(localVPosition);
              intercept.setVxdID(sensorID);
              m_storePXDIntercepts.appendNew(intercept);
              thisTracksIntercepts.push_back(intercept);
            }
            // check for second option of the intersection
            if (yminus >= m_const_sensorMinY - toleranceRPhi and yminus <= m_const_sensorMaxY + toleranceRPhi) {
              PXDIntercept intercept;
              intercept.setCoorU(localUPositionMinus);
              intercept.setCoorV(localVPosition);
              intercept.setVxdID(sensorID);
              m_storePXDIntercepts.appendNew(intercept);
              thisTracksIntercepts.push_back(intercept);
            }
          }
        }
      }
    }

    const double omega = 1. / trackRadius;
    unsigned short uSizeL1 = (unsigned short)(m_param_multiplierUL1 * omega + m_param_minimumROISizeUL1);
    unsigned short uSizeL2 = (unsigned short)(m_param_multiplierUL2 * omega + m_param_minimumROISizeUL2);
    unsigned short vSizeL1 = (unsigned short)((1. + fabs(tanTrackLambda) * m_param_multiplierVL1) * m_param_minimumROISizeVL1);
    unsigned short vSizeL2 = (unsigned short)((1. + fabs(tanTrackLambda) * m_param_multiplierVL2) * m_param_minimumROISizeVL2);
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
      if (uSize > m_param_maximumROISizeU) {
        uSize = m_param_maximumROISizeU;
      }
      if (vSize > m_param_maximumROISizeV) {
        vSize = m_param_maximumROISizeV;
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

