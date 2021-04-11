/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Christian Wessel                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/findlets/DATCONROIFinder.h>
#include <tracking/datcon/optimizedDATCON/findlets/RawTrackCandCleaner.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/logging/Logger.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

DATCONROIFinder::~DATCONROIFinder() = default;

DATCONROIFinder::DATCONROIFinder()
{
//   addProcessingSignalListener(&m_spacePointLoaderAndPreparer);

}

void DATCONROIFinder::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "storePXDInterceptsName"),
                                m_param_storePXDInterceptsName,
                                "Name of the PXDIntercepts StoreArray produced by DATCON using a simple circle extrapolation in r-phi and a straight line extrapolation in theta.",
                                m_param_storePXDInterceptsName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "storeROIsName"),
                                m_param_storeROIsName,
                                "Name of the ROIs StoreArray produced by DATCON using a simple circle extrapolation in r-phi and a straight line extrapolation in theta.",
                                m_param_storeROIsName);

//   m_spacePointLoaderAndPreparer.exposeParameters(moduleParamList, prefix);
}

/// Create the store arrays
void DATCONROIFinder::initialize()
{
  Super::initialize();
  m_storePXDIntercepts.registerInDataStore(m_param_storePXDInterceptsName);
  m_storeROIs.registerInDataStore(m_param_storeROIsName);
};


void DATCONROIFinder::beginEvent()
{
  Super::beginEvent();
}

void DATCONROIFinder::apply(const std::vector<SpacePointTrackCand>& finalTracks)
{
  for (auto& track : finalTracks) {
    const B2Vector3D& momentumEstimate = track.getMomSeed();

    const double trackPhi     = momentumEstimate.Phi();
    const double trackTheta   = momentumEstimate.Theta();
    const double trackRadius  = momentumEstimate.Perp();
    const double trackCharge  = track.getChargeSeed();

    /** Loop over both PXD layer */
    for (int layer = 1; layer <= 2; layer++) {
      const double sensorPerpRadius = m_const_layerRadius[layer - 1];
      const double x = sensorPerpRadius;
      /** Loop over all ladders of layer */
      for (int ladder = 1; ladder <= (layer == 1 ? 8 : 12); ladder++) {
        const double sensorPhi = (layer == 1) ? M_PI / 4. * (ladder - 1) : M_PI / 6. * (ladder - 1);
        const double phiDiff = trackPhi - sensorPhi;

        // Don't try to extrapolate if track direction and sensor are too different (in phi)
        // this should speed up the calculation as wrong combinations are not checked at all.
        // Two values to check both the outgoing (0.2) and incoming (0.8) arm.
        if (fabs(phiDiff) > 0.2 * M_PI and fabs(phiDiff) < 0.8 * M_PI)
          continue;

        // relative phi value of the track center compared to the rotated
        // coordinate system defined by the (rotated) line perpendicular
        // to the sensor (with length sensorPerpRadius)
        double relTrackCenterPhi = 0;
        if (trackCharge == +1) {
          relTrackCenterPhi = trackPhi - M_PI_2 - sensorPhi;
        } else if (trackCharge == -1) {
          relTrackCenterPhi = trackPhi + M_PI_2 - sensorPhi;
        }
        const double xCenter  = trackRadius * cos(relTrackCenterPhi);
        const double yCenter  = trackRadius * sin(relTrackCenterPhi);
        const double ytmp     = sqrt(trackRadius * trackRadius - (x - xCenter) * (x - xCenter));
        const double yplus    = yCenter + ytmp;
        const double yminus   = yCenter - ytmp;
        const double localUPositionPlus   = yplus  - m_const_shiftY;
        const double localUPositionMinus  = yminus - m_const_shiftY;

        // if the hit for sure is out of reach of the current ladder, continue
        if (not(yplus  >= m_const_sensorMinY - 0.5 and yplus  <= m_const_sensorMaxY + 0.5) and
            not(yminus >= m_const_sensorMinY - 0.5 and yminus <= m_const_sensorMaxY + 0.5)) {
          continue;
        }

        /** Loop over both modules of a ladder */
        for (int sensor = 1; sensor <= 2; sensor++) {

          const double shiftZ = (layer == 1) ? m_const_centerZShiftLayer1[sensor - 1] : m_const_centerZShiftLayer2[sensor - 1];
          const double z = (fabs(trackTheta - M_PI_2) > 1e-3) ? sensorPerpRadius / tan(trackTheta) : 0.0;

          // check whether z intersection possibly is on sensor to be checked, only continue with the rest of calculations if that's the case
          if (z >= ((m_const_sensorLength[layer - 1] / -2.0) - 1.0 + shiftZ)
              && z <= ((m_const_sensorLength[layer - 1] / 2.0) + 1.0 + shiftZ)) {


            const VxdID sensorID = VxdID(layer, ladder, sensor);

            double localVPosition = z - shiftZ;
            // check for first option of the intersection
            if (yplus >= m_const_sensorMinY - 0.5 && yplus <= m_const_sensorMaxY + 0.5) {
              PXDIntercept intercept;
              intercept.setCoorU(localUPositionPlus);
              intercept.setCoorV(localVPosition);
              intercept.setVxdID(sensorID);
              m_storePXDIntercepts.appendNew(intercept);
              //break;
            }
            // check for second option of the intersection
            if (yminus >= m_const_sensorMinY - 0.5 && yminus <= m_const_sensorMaxY + 0.5) {
              PXDIntercept intercept;
              intercept.setCoorU(localUPositionMinus);
              intercept.setCoorV(localVPosition);
              intercept.setVxdID(sensorID);
              m_storePXDIntercepts.appendNew(intercept);
              //break;
            }
          }
        }
      }
    }

  }
}

