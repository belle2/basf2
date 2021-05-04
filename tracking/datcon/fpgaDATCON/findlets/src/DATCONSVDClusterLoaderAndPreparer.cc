/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/fpgaDATCON/findlets/DATCONSVDClusterLoaderAndPreparer.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <svd/dataobjects/SVDCluster.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

DATCONSVDClusterLoaderAndPreparer::DATCONSVDClusterLoaderAndPreparer() : Super()
{
}

void DATCONSVDClusterLoaderAndPreparer::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maxClustersPerLayer"), m_param_maxClustersPerLayer,
                                "Maximum number of clusters on one layer before aborting tracking.", m_param_maxClustersPerLayer);
}

void DATCONSVDClusterLoaderAndPreparer::initialize()
{
  Super::initialize();
}

void DATCONSVDClusterLoaderAndPreparer::apply(std::vector<SVDCluster>& uClusters, std::vector<SVDCluster>& vClusters,
                                              std::vector<std::pair<VxdID, std::pair<long, long>>>& uHits,
                                              std::vector<std::pair<VxdID, std::pair<long, long>>>& vHits)
{
  if (uClusters.size() == 0  or vClusters.size() == 0) {
    return;
  }

//   for (uint i = 0; i < 8; i++) m_nClusterPerLayer.at(i) = 0;
  m_nClusterPerLayer.fill(0);

  for (auto& cluster : uClusters) {
    const VxdID& sensorID = cluster.getSensorID();
    const uint& layerNumber = sensorID.getLayerNumber();
    const uint& ladderNumber = sensorID.getLadderNumber();
    const long localPosition = convertToInt(cluster.getPosition(), 4); // convert the cluster position from cm to µm

    m_nClusterPerLayer.at(layerNumber - 3)++;
    const double rotangle = m_const_InitialAngle[layerNumber - 3] + (ladderNumber - 1) * m_const_AngleStep[layerNumber - 3];
    const double cosRotAngle = cos(rotangle);
    const double sinRotAngle = sin(rotangle);
    const int    sensorRadius = m_const_SVDRadii[layerNumber - 3];
    const int    ytmp = localPosition + m_const_RPhiShiftsOfLayers[layerNumber - 3];

    // perform 2D rotation
    const long x = round(sensorRadius * cosRotAngle - ytmp * sinRotAngle); // x is in µm
    const long y = round(sensorRadius * sinRotAngle + ytmp * cosRotAngle); // y is in µm
    const long radiusSquared = x * x + y * y; // therefore radius is in µm as well, like all other length values
    // x or y divided by r^2 can create values very close to 0. To cope for this and still
    // have non-zero numbers after conversion and rounding, the conversion to int is made with 10^10
    std::pair<long, long> pos2D = std::make_pair(convertToInt(2. * (double)x / (double)radiusSquared, 10),
                                                 convertToInt(2. * (double)y / (double)radiusSquared, 10));
    B2DEBUG(29, "x, y: " << x << " " << y << " Hough-values: " << pos2D.first << " " << pos2D.second);
    uHits.emplace_back(std::make_pair(sensorID, pos2D));
  }

  /* First convert to absolute hits and save into a map */
  for (auto& cluster : vClusters) {
    const VxdID& sensorID = cluster.getSensorID();
    const uint& layerNumber = sensorID.getLayerNumber();
    const uint& sensorNumber = sensorID.getSensorNumber();
    const long localPosition = convertToInt(cluster.getPosition(), 4); // convert the cluster position from cm to µm

    m_nClusterPerLayer.at(4 + layerNumber - 3)++;
    const int radius = m_const_SVDRadii[layerNumber - 3];
    int z = 0;
    if (layerNumber == 3) {
      z = localPosition + m_const_ZShiftL3[sensorNumber - 1];
    } else {
      switch (layerNumber) {
        case 4:
          if (sensorNumber == 1) {
            z = localPosition * m_const_CosSlantedAngles[layerNumber - 3] + m_const_ZShiftL4[sensorNumber - 1];
          } else {
            z = localPosition + m_const_ZShiftL4[sensorNumber - 1];
          }
          break;
        case 5:
          if (sensorNumber == 1) {
            z = localPosition * m_const_CosSlantedAngles[layerNumber - 3] + m_const_ZShiftL5[sensorNumber - 1];
          } else {
            z = localPosition + m_const_ZShiftL5[sensorNumber - 1];
          }
          break;
        case 6:
          if (sensorNumber == 1) {
            z = localPosition * m_const_CosSlantedAngles[layerNumber - 3] + m_const_ZShiftL6[sensorNumber - 1];
          } else {
            z = localPosition + m_const_ZShiftL6[sensorNumber - 1];
          }
          break;
      }
    }
    vHits.emplace_back(std::make_pair(sensorID, std::make_pair(z, radius)));
  }

  const uint* maxOfClustersPerLayer = std::max_element(m_nClusterPerLayer.begin(), m_nClusterPerLayer.end());
  if (*maxOfClustersPerLayer > m_param_maxClustersPerLayer) {
    B2WARNING("High occupancy in SVD, aborting DATCON...");
    uHits.clear();
    vHits.clear();
  }

}
