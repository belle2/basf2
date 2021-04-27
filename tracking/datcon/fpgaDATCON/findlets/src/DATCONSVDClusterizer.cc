/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/fpgaDATCON/findlets/DATCONSVDClusterizer.h>
#include <tracking/datcon/fpgaDATCON/entities/DATCONSVDDigit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

DATCONSVDClusterizer::DATCONSVDClusterizer() : Super()
{
}

void DATCONSVDClusterizer::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "noiseMap"), m_param_noiseMapfileName,
                                "Name of the text file containing strip noise information.", m_param_noiseMapfileName);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "writeNoiseMapsToFile"), m_param_writeNoiseMapsToFile,
                                "Write noise information to text file?", m_param_writeNoiseMapsToFile);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumClusterSize"), m_param_maxiClusterSize,
                                "Maximum cluster size.", m_param_maxiClusterSize);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "noiseCut"), m_param_noiseCut,
                                "Cut for using default noise (noise < this value), or actual noise (noise > this value).",
                                m_param_noiseCut);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "requiredSNRstrip"),
                                m_param_requiredSNRstrip, "Required SNR per strip.", m_param_requiredSNRstrip);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "requiredSNRcluster"),
                                m_param_requiredSNRcluster, "Required SNR for the cluster.", m_param_requiredSNRcluster);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "isUClusterizer"),
                                m_param_isU, "Is this u or v side?", m_param_isU);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "saveClusterToDataStore"),
                                m_param_saveClusterToDataStore, "Save SVDClusters for analysis?", m_param_saveClusterToDataStore);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "storeSVDClustersName"),
                                m_param_storeSVDClustersName, "Name of the SVDClusters StoreArray", m_param_storeSVDClustersName);

}

void DATCONSVDClusterizer::beginRun()
{
  svdNoiseMap.clear();
  fillDATCONSVDNoiseMap();
}

void DATCONSVDClusterizer::initialize()
{
  Super::initialize();

  if (m_param_saveClusterToDataStore) {
    m_storeSVDClusters.registerInDataStore(m_param_storeSVDClustersName);
    m_param_storeSVDClustersName = m_storeSVDClusters.getName();
  }
}

void DATCONSVDClusterizer::apply(std::vector<DATCONSVDDigit>& digits, std::vector<SVDCluster>& clusters)
{
  if (digits.size() == 0) return;

  clusterCandidate clusterCand;

  // create a dummy cluster just to start
  clusterCand.vxdID = digits.at(0).getSensorID();

  for (auto& digit : digits) {

    float stripSNR = calculateSNR(digit);
    if (stripSNR < m_param_requiredSNRstrip) {
      continue;
    }

    //retrieve the VxdID, sensor and cellID of the current DATCONSVDDigit
    VxdID thisSensorID = digit.getSensorID();
    unsigned short thisCellID = digit.getCellID();
    unsigned short thisCharge = digit.getMaxSampleCharge();

    //try to add the strip to the existing cluster
    if (! clusterCand.add(thisSensorID, thisCharge, thisCellID, stripSNR, m_param_maxiClusterSize)) {
      //if the strip is not added, write the cluster, if present and good:
      if (clusterCand.strips.size() > 0) {
        const VXD::SensorInfoBase& info = geoCache.getSensorInfo(clusterCand.vxdID);
        double pitch = m_param_isU ? info.getUPitch() : info.getVPitch();
        unsigned short numberofStrips = m_param_isU ? info.getUCells() : info.getVCells();

        clusterCand.finalizeCluster(pitch, numberofStrips);
        if (clusterCand.maxSNRinClusterCandidate >= m_param_requiredSNRcluster) {
          double clusterPositionError = pitch;
          if (clusterCand.strips.size() == 1) {
            clusterPositionError = pitch / sqrt(12.);
          } else if (clusterCand.strips.size() == 2) {
            clusterPositionError = pitch / 2.;
          }

          clusters.emplace_back(SVDCluster(clusterCand.vxdID, m_param_isU, clusterCand.clusterPosition, clusterPositionError,
                                           0.0, 0.0, clusterCand.charge, clusterCand.seedCharge, clusterCand.strips.size(), 0.0, 0.0));
        }
      }

      // start the next cluster
      clusterCand.strips.clear();
      clusterCand.charges.clear();
      clusterCand.vxdID = thisSensorID;
      clusterCand.strips.emplace_back(thisCellID);
      clusterCand.charges.emplace_back(thisCharge);
      clusterCand.maxSNRinClusterCandidate = stripSNR;
    }
  }

  //write the last cluster, if good
  if (clusterCand.strips.size() > 0) {

    const VXD::SensorInfoBase& info = geoCache.getSensorInfo(clusterCand.vxdID);
    double pitch = m_param_isU ? info.getUPitch() : info.getVPitch();
    unsigned short numberofStrips = m_param_isU ? info.getUCells() : info.getVCells();

    clusterCand.finalizeCluster(pitch, numberofStrips);
    if (clusterCand.maxSNRinClusterCandidate >= m_param_requiredSNRcluster) {

      double clusterPositionError = pitch;
      if (clusterCand.strips.size() == 1) {
        clusterPositionError = pitch / sqrt(12.);
      } else if (clusterCand.strips.size() == 2) {
        clusterPositionError = pitch / 2.;
      }

      clusters.emplace_back(SVDCluster(clusterCand.vxdID, m_param_isU, clusterCand.clusterPosition, clusterPositionError,
                                       0.0, 0.0, clusterCand.charge, clusterCand.seedCharge, clusterCand.strips.size(), 0.0, 0.0));
    }
  }

  if (m_param_saveClusterToDataStore) {
    for (const auto& cluster : clusters) {
      m_storeSVDClusters.appendNew(cluster);
    }
  }

}

// /*
// * Simple noise filter.
// * Run the noise filter over the given numbers of samples.
// * If it fulfills the requirements true is returned.
// */
float DATCONSVDClusterizer::calculateSNR(DATCONSVDDigit digit)
{
  unsigned short maxSampleIndex = digit.getMaxSampleIndex();
  const DATCONSVDDigit::APVRawSamples& sample = digit.getRawSamples();
  unsigned short stripID = digit.getCellID();
  VxdID sensorID = digit.getSensorID();
  int simpleVXDID = 131072 * (sensorID.getLayerNumber() - 3) + 8192 * (sensorID.getLadderNumber() - 1)
                    + 1024 * (sensorID.getSensorNumber() - 1) + stripID;

  // set noise value to default value and only use actual noise if it is too large (> m_param_noiseCut)
  float stripNoise = m_param_noiseCut;
  if (svdNoiseMap.find(simpleVXDID) != svdNoiseMap.end()) {
    stripNoise = svdNoiseMap.at(simpleVXDID);
  }

  float currentSNR = (float)sample[maxSampleIndex] / stripNoise;
  return currentSNR;
}



void DATCONSVDClusterizer::fillDATCONSVDNoiseMap()
{
  //call for a geometry instance
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> svdLayers = aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();

  std::ofstream noiseMap;
  if (m_param_writeNoiseMapsToFile) {
    noiseMap.open(m_param_noiseMapfileName, std::ios::trunc);
  }

  while ((itSvdLayers != svdLayers.end()) && (itSvdLayers->getLayerNumber() != 7)) { //loop on Layers

    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) { //loop on Ladders

      std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();
      B2DEBUG(29, "    svd sensor info " << * (svdSensors.begin()));

      while (itSvdSensors != svdSensors.end()) { //loop on sensors
        B2DEBUG(29, "    svd sensor info " << *itSvdSensors);

        int layer = itSvdSensors->getLayerNumber();
        int ladder =  itSvdSensors->getLadderNumber();
        int sensor = itSvdSensors->getSensorNumber();
        Belle2::VxdID theVxdID(layer, ladder, sensor);
        const SVD::SensorInfo* currentSensorInfo = dynamic_cast<const SVD::SensorInfo*>(&VXD::GeoCache::get(theVxdID));

        int simpleVXDID = 131072 * (layer - 3) + 8192 * (ladder - 1) + 1024 * (sensor - 1);

        int Ncells = m_param_isU ? currentSensorInfo->getUCells() : currentSensorInfo->getVCells();

        for (int strip = 0; strip < Ncells; strip++) {

          float noise = -1;
          if (m_NoiseCal.isValid()) {
            noise = m_NoiseCal.getNoise(theVxdID, m_param_isU, strip);
          }

          if (noise > m_param_noiseCut) {
            svdNoiseMap.insert(std::make_pair(simpleVXDID + strip, noise));
            if (m_param_writeNoiseMapsToFile) {
              noiseMap << 4096 * (layer - 3) + 16 * (ladder - 1) + (sensor - 1) << " " << strip << " " << noise << std::endl;
            }
          }
        }
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }
  if (m_param_writeNoiseMapsToFile) {
    noiseMap.close();
  }

}
