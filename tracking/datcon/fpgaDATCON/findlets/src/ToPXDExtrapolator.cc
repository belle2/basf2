/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/fpgaDATCON/findlets/ToPXDExtrapolator.h>

#include <tracking/dataobjects/PXDIntercept.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <vxd/dataobjects/VxdID.h>
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

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "extrapolationPhiCutL1"),
                                m_param_phiCutL1,
                                "Only extrapolate to PXD sensors within this value away from the track phi value, L1.",
                                m_param_phiCutL1);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "extrapolationPhiCutL2"),
                                m_param_phiCutL2,
                                "Only extrapolate to PXD sensors within this value away from the track phi value, L2.",
                                m_param_phiCutL2);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "storePXDIntercepts"),
                                m_param_storePXDIntercepts,
                                "Store PXDIntercepts to StoreArray?",
                                m_param_storePXDIntercepts);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "storePXDInterceptsName"),
                                m_param_PXDInterceptStoreArrayName,
                                "Name of the PXDIntercepts StoreArray?",
                                m_param_PXDInterceptStoreArrayName);

}

void ToPXDExtrapolator::initialize()
{
  Super::initialize();

  if (m_param_storePXDIntercepts) {
    m_pxdIntercepts.registerInDataStore(m_param_PXDInterceptStoreArrayName);
    m_param_PXDInterceptStoreArrayName = m_pxdIntercepts.getName();
  }

}

void ToPXDExtrapolator::apply(std::vector<std::pair<double, double>>& uTracks, std::vector<std::pair<double, double>>& vTracks,
                              std::vector<std::pair<VxdID, long>>& uExtrapolations, std::vector<std::pair<VxdID, long>>& vExtrapolations)
{
  VxdID sensorID;

  double angleDiff;

  for (auto& uTrack : uTracks) {
    double trackPhi = uTrack.first + M_PI_2;
    if (trackPhi < -M_PI) trackPhi += 2 * M_PI;
    if (trackPhi >  M_PI) trackPhi -= 2 * M_PI;
//     const double trackOmega = uTrack.second * 1e-6;
//     const double trackRadius = 1./trackOmega;

    // 1./uTrack.second * 1e10 yields trackRadius in cm. To convert to µm, which all other values are in,
    // multiplication by another 1e4 is required -> total of 1e14
    double trackRadius = convertToInt(1. / uTrack.second, 14);
    B2INFO("trackPhi: " << trackPhi << " omega: " << uTrack.second << " trackRadius: " <<
           trackRadius);// << " trackRadius * 1e4 -> um: " << convertToInt(trackRadius, 4));

    // layer 1 extrapolation
    uint layer = 1;
    long sensorPerpRadius = layerRadius[layer - 1];
    for (uint ladder = 1; ladder <= 8; ladder++) {
      const double sensorPhi = M_PI / 4. * (ladder - 1);

      angleDiff = trackPhi - sensorPhi;
      if (angleDiff > M_PI) {
        angleDiff -= 2 * M_PI;
      }
      if (angleDiff < -M_PI) {
        angleDiff += 2 * M_PI;
      }
      if (fabs(angleDiff) >= m_param_phiCutL1) continue;
      B2INFO("angleDiff: " << angleDiff);
      B2INFO("trackRadius * trackRadius: " << trackRadius * trackRadius);
      B2INFO("trackRadius * cos(angleDiff): " << trackRadius * cos(angleDiff));
      B2INFO("trackRadius * sin(angleDiff): " << trackRadius * sin(angleDiff));
      B2INFO("sensorPerpRadius - trackRadius * sin(angleDiff): " << sensorPerpRadius - trackRadius * sin(angleDiff));

      // sensorPerpRadius is in µm tan is multiplied by 1000, so this is basically nm
//       long y = sensorPerpRadius * convertToInt(tan(angleDiff), 3);
//       long y = (long)(-trackRadius * cos(angleDiff) + sqrt(trackRadius * trackRadius - (sensorPerpRadius - trackRadius * sin(angleDiff)) * (sensorPerpRadius - trackRadius * sin(angleDiff)) ));
      double y = (-trackRadius * cos(angleDiff) + sqrt(trackRadius * trackRadius - (sensorPerpRadius - trackRadius * sin(angleDiff)) *
                                                       (sensorPerpRadius - trackRadius * sin(angleDiff))));
      B2INFO("y from extrapolation: " << y);

      if (y >= sensorMinY && y <= sensorMaxY) {
        B2INFO("I hit a sensor!!! " << y << " on sensor: " << layer << "." << ladder << ".X");

        long localUPosition = y - shiftY;
        B2INFO("localUPosition: " << localUPosition);

        // store extrapolated hit for first sensor in ladder
        sensorID = VxdID(layer, ladder, 1);
        uExtrapolations.emplace_back(sensorID, localUPosition);
        if (m_param_storePXDIntercepts) {
          PXDIntercept intercept;
          intercept.setCoorU((double)localUPosition * Unit::nm);  // convert the "nm" like values back to cm (basf2 std unit)
          intercept.setCoorV(-std::numeric_limits<long>::max());
          intercept.setVxdID(sensorID);
          m_pxdIntercepts.appendNew(intercept);
        }

        // store extrapolated hit for second sensor in ladder
        sensorID = VxdID(layer, ladder, 2);
        uExtrapolations.emplace_back(sensorID, localUPosition);
        if (m_param_storePXDIntercepts) {
          PXDIntercept intercept;
          intercept.setCoorU((double)localUPosition * Unit::nm); // convert the "nm" like values back to cm (basf2 std unit)
          intercept.setCoorV(-std::numeric_limits<long>::max());
          intercept.setVxdID(sensorID);
          m_pxdIntercepts.appendNew(intercept);
        }
      }
    }

    // layer 2 extrapolation
    layer = 2;
    sensorPerpRadius = layerRadius[layer - 1];
    for (uint ladder = 1; ladder <= 12; ladder++) {
      const double sensorPhi = M_PI / 6. * (ladder - 1);

      angleDiff = trackPhi - sensorPhi;
      if (angleDiff > M_PI) {
        angleDiff -= 2 * M_PI;
      }
      if (angleDiff < -M_PI) {
        angleDiff += 2 * M_PI;
      }
      if (fabs(angleDiff) >= m_param_phiCutL2) continue;

      // sensorPerpRadius is in µm, tan is multiplied by 1000, so this is basically nm
      long y = sensorPerpRadius * convertToInt(tan(angleDiff), 3);

      if (y >= sensorMinY && y <= sensorMaxY) {

        long localUPosition = y - shiftY;

        // store extrapolated hit for first sensor in ladder
        sensorID = VxdID(layer, ladder, 1);
        uExtrapolations.emplace_back(sensorID, localUPosition);
        if (m_param_storePXDIntercepts) {
          PXDIntercept intercept;
          intercept.setCoorU((double)localUPosition * Unit::nm); // convert the "nm" like values back to cm (basf2 std unit)
          intercept.setCoorV(-std::numeric_limits<long>::max());
          intercept.setVxdID(sensorID);
          m_pxdIntercepts.appendNew(intercept);
        }

        // store extrapolated hit for second sensor in ladder
        sensorID = VxdID(layer, ladder, 2);
        uExtrapolations.emplace_back(sensorID, localUPosition);
        if (m_param_storePXDIntercepts) {
          PXDIntercept intercept;
          intercept.setCoorU((double)localUPosition * Unit::nm); // convert the "nm" like values back to cm (basf2 std unit)
          intercept.setCoorV(-std::numeric_limits<long>::max());
          intercept.setVxdID(sensorID);
          m_pxdIntercepts.appendNew(intercept);
        }
      }
    }
  }

  for (auto& vTrack : vTracks) {
    // layer 1 extrapolation
    uint layer = 1;
    const double& trackLambda = vTrack.first;
    const long tanLambda = convertToInt(tan(trackLambda), 3);
    for (uint sensor = 1; sensor <= 2; sensor++) {
      const long& sensorPerpRadius = layerRadius[layer - 1];
      const long& lengthOfSensor = sensorLength[layer - 1];
      const long& shiftZ = centerZShiftLayer1[sensor - 1];
      // sensorPerpRadius is in µm, inverseTanTheta is multiplied by 1000, so this is basically nm
      const long globalz = sensorPerpRadius * tanLambda;
      // shift globalz into local coordinate system by subtracting the z-shift of this sensor
      const long localVPosition = globalz - shiftZ;
      if (localVPosition >= -lengthOfSensor / 2 && localVPosition <= lengthOfSensor / 2) {

        for (uint ladder = 1; ladder <= 8; ladder++) {
          sensorID = VxdID(layer, ladder, sensor);

          vExtrapolations.emplace_back(sensorID, localVPosition);
          if (m_param_storePXDIntercepts) {
            PXDIntercept intercept;
            intercept.setCoorU(-std::numeric_limits<long>::max());
            intercept.setCoorV((double)localVPosition * Unit::nm); // convert the "nm" like values back to cm (basf2 std unit)
            intercept.setVxdID(sensorID);
            m_pxdIntercepts.appendNew(intercept);
          }
        }
      }
    }

    // layer 2 extrapolation
    layer = 2;
    for (uint sensor = 1; sensor <= 2; sensor++) {
      const long& sensorPerpRadius = layerRadius[layer - 1]; // convert from mm to µm as integer
      const long& lengthOfSensor = sensorLength[layer - 1]; // convert from mm to µm as integer
      const long& shiftZ = centerZShiftLayer2[sensor - 1];
      const long globalz = sensorPerpRadius * tanLambda;

      // shift globalz into local coordinate system by subtracting the z-shift of this sensor
      const long localVPosition = globalz - shiftZ;
      if (localVPosition >= -lengthOfSensor / 2 && localVPosition <= lengthOfSensor / 2) {
        for (uint ladder = 1; ladder <= 12; ladder++) {
          sensorID = VxdID(layer, ladder, sensor);

          vExtrapolations.emplace_back(sensorID, localVPosition);
          if (m_param_storePXDIntercepts) {
            PXDIntercept intercept;
            intercept.setCoorU(-std::numeric_limits<long>::max());
            intercept.setCoorV((double)localVPosition * Unit::nm); // convert the "nm" like values back to cm (basf2 std unit)
            intercept.setVxdID(sensorID);
            m_pxdIntercepts.appendNew(intercept);
          }
        }
      }
    }
  }

//   B2DEBUG(29, "uExtrapolations.size: " << uExtrapolations.size() << " vExtrapolations.size: " << vExtrapolations.size());
}
