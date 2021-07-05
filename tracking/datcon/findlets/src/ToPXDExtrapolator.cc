/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/datcon/findlets/ToPXDExtrapolator.h>
#include <tracking/datcon/utilities/DATCONHelpers.h>
#include <tracking/dataobjects/PXDIntercept.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>
#include <framework/gearbox/Unit.h>
#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

ToPXDExtrapolator::ToPXDExtrapolator() : Super()
{
}

void ToPXDExtrapolator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "extrapolationPhiCutL1"), m_param_phiCutL1,
                                "Only extrapolate to PXD sensors within this value away from the track phi value, L1.",
                                m_param_phiCutL1);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "extrapolationPhiCutL2"), m_param_phiCutL2,
                                "Only extrapolate to PXD sensors within this value away from the track phi value, L2.",
                                m_param_phiCutL2);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "createPXDIntercepts"), m_param_createPXDIntercepts,
                                "Store PXDIntercepts to StoreArray?", m_param_createPXDIntercepts);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "storePXDInterceptsName"), m_param_PXDInterceptStoreArrayName,
                                "Name of the PXDIntercepts StoreArray?", m_param_PXDInterceptStoreArrayName);

}

void ToPXDExtrapolator::initialize()
{
  Super::initialize();

  if (m_param_createPXDIntercepts) {
    m_pxdIntercepts.registerInDataStore(m_param_PXDInterceptStoreArrayName);
  }

}

void ToPXDExtrapolator::apply(const std::vector<std::pair<double, double>>& uTracks,
                              const std::vector<std::pair<double, double>>& vTracks,
                              std::vector<std::pair<VxdID, long>>& uExtrapolations, std::vector<std::pair<VxdID, long>>& vExtrapolations)
{
  VxdID sensorID;

  for (auto& uTrack : uTracks) {
    const double trackPhi = uTrack.first;
    const double trackRadius = uTrack.second;

    extrapolateUTrack(trackPhi, trackRadius, 1, uExtrapolations);
    extrapolateUTrack(trackPhi, trackRadius, 2, uExtrapolations);
  }

  for (auto& vTrack : vTracks) {
    const double& trackLambda = -vTrack.first;
    const long tanLambda = convertFloatToInt(tan(trackLambda), 3);

    extrapolateVTrack(tanLambda, 1, vExtrapolations);
    extrapolateVTrack(tanLambda, 2, vExtrapolations);
  }

  if (m_param_createPXDIntercepts) {
    for (auto& uExtrapolatedHit : uExtrapolations) {
      const VxdID& uHitSensorID = uExtrapolatedHit.first;

      for (auto& vExtrapolatedHit : vExtrapolations) {
        const VxdID& vHitSensorID = vExtrapolatedHit.first;

        if (uHitSensorID != vHitSensorID) {
          continue;
        }

        // convert back from nm to cm
        double uCoordinateInCM = uExtrapolatedHit.second * Unit::nm;
        double vCoordinateInCM = vExtrapolatedHit.second * Unit::nm;

        PXDIntercept intercept;
        intercept.setCoorU(uCoordinateInCM);
        intercept.setCoorV(vCoordinateInCM);
        intercept.setVxdID(uHitSensorID);
        m_pxdIntercepts.appendNew(intercept);
      }
    }
  }

  B2DEBUG(29, "uExtrapolations.size: " << uExtrapolations.size() << " vExtrapolations.size: " << vExtrapolations.size());
}

void ToPXDExtrapolator::extrapolateUTrack(const double trackPhi, const double trackRadius, const uint layer,
                                          std::vector<std::pair<VxdID, long>>& uExtrapolations)
{
  long sensorPerpRadius = layerRadius[layer - 1];
  for (uint ladder = 1; ladder <= laddersPerLayer[layer - 1]; ladder++) {
    double sensorPhi = M_PI / (laddersPerLayer[layer - 1] / 2) * (ladder - 1);
    if (sensorPhi > M_PI) {
      sensorPhi -= 2. * M_PI;
    }

    double angleDiff = trackPhi - sensorPhi;
    if (angleDiff > M_PI) {
      angleDiff -= 2. * M_PI;
    }
    if (angleDiff < -M_PI) {
      angleDiff += 2. * M_PI;
    }
    if (fabs(angleDiff) >= (layer == 1 ? m_param_phiCutL1 : m_param_phiCutL2)) continue;

    // additional factor of 10^3, as the sine and cosine values are also multiplied by 1000
    long trackRadiusSquared = convertFloatToInt(trackRadius, 3) * convertFloatToInt(trackRadius, 3);
    // additional factor of 10^3, as the sine and cosine values are also multiplied by 1000
    long b = convertFloatToInt(sensorPerpRadius, 3) - trackRadius * convertFloatToInt(sin(angleDiff), 3);
    double y = -trackRadius * convertFloatToInt(cos(angleDiff), 3) + sqrt(trackRadiusSquared - b * b);

    if (y >= sensorMinY && y <= sensorMaxY) {
      long localUPosition = y - shiftY;

      // store extrapolated hit for first sensor in ladder
      VxdID sensorID = VxdID(layer, ladder, 1);
      uExtrapolations.emplace_back(sensorID, localUPosition);

      // store extrapolated hit for second sensor in ladder
      sensorID = VxdID(layer, ladder, 2);
      uExtrapolations.emplace_back(sensorID, localUPosition);
    }
  }
}

void ToPXDExtrapolator::extrapolateVTrack(const long tanLambda, const uint layer,
                                          std::vector<std::pair<VxdID, long>>& vExtrapolations)
{
  for (uint sensor = 1; sensor <= 2; sensor++) {
    const long& sensorPerpRadius = layerRadius[layer - 1];
    const long& lengthOfSensor = sensorLength[layer - 1];
    const long& shiftZ = (layer == 1 ? centerZShiftLayer1[sensor - 1] : centerZShiftLayer2[sensor - 1]);
    // sensorPerpRadius is in µm, inverseTanTheta is multiplied by 1000, so this is basically nm
    const long globalz = sensorPerpRadius * tanLambda;
    // shift globalz into local coordinate system by subtracting the z-shift of this sensor
    const long localVPosition = globalz - shiftZ;
    if (localVPosition >= -lengthOfSensor / 2. && localVPosition <= lengthOfSensor / 2.) {

      for (uint ladder = 1; ladder <= laddersPerLayer[layer - 1]; ladder++) {
        VxdID sensorID = VxdID(layer, ladder, sensor);
        vExtrapolations.emplace_back(sensorID, localVPosition);
      }
    }
  }
}
