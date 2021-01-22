/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/DATCON/DATCONSVDSimpleClusterizerModule.h>

#include <svd/geometry/SensorInfo.h>

using namespace std;
using namespace Belle2;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DATCONSVDSimpleClusterizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DATCONSVDSimpleClusterizerModule::DATCONSVDSimpleClusterizerModule() : Module()
{
  //Set module properties
  setDescription("Clusterize DATCONSVDDigits");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("DATCONSVDDigits", m_storeDATCONSVDDigitsListName,
           "DATCONSVDDigits collection name", string("DATCONSVDDigits"));
  addParam("DATCONSVDCluster", m_storeDATCONSVDClustersName,
           "DATCONSVDCluster collection name", string("DATCONSVDCluster"));
  addParam("SVDTrueHits", m_storeTrueHitsName,
           "TrueHit collection name", string(""));
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticles collection name", string(""));

  // 2. Clustering
  addParam("useNoiseFilter", m_useNoiseFilter,
           "Use a simple noise fiter", bool(true));
  addParam("NoiseLevelADU", m_NoiseLevelInADU,
           "Simple assumption of the noise level of the sensors in ADU's", (unsigned short)(5));
  addParam("NoiseCutADU", m_NoiseCutInADU,
           "Simple assumption of the noise level of the sensors in ADU's", (unsigned short)(5));
  addParam("useSimpleClustering", m_useSimpleClustering,
           "Use the simple clustering that is currently done on FPGA for phase 2", bool(true));
  addParam("maxClusterSizeU", m_maxClusterSizeU,
           "Maximum cluster size in count of u-strips.", (unsigned short)(8));
  addParam("maxClusterSizeV", m_maxClusterSizeV,
           "Maximum cluster size in count of v-strips.", (unsigned short)(8));
  addParam("noiseCutU", m_noiseCutU, "Save noise value for a u-strip in svdNoiseMap if it exceeds m_noiseU.", float(4.0));
  addParam("noiseCutV", m_noiseCutV, "Save noise value for a v-strip in svdNoiseMap if it exceeds m_noiseV.", float(3.0));
  addParam("requiredSNRstripU", m_requiredSNRstripU, "Required SNR for a u-strip signal.", float(3.0));
  addParam("requiredSNRstripV", m_requiredSNRstripV, "Required SNR for a v-strip signal.", float(3.0));
  addParam("requiredSNRclusterU", m_requiredSNRclusterU, "Required SNR for minimum one u-strip in cluster.", float(5.0));
  addParam("requiredSNRclusterV", m_requiredSNRclusterV, "Required SNR for minimum one v-strip in cluster.", float(5.0));
  addParam("writeNoiseMapsToFile", m_writeNoiseMapsToFile, "Write the simple SVD noise maps to files?", bool(false));

}


void DATCONSVDSimpleClusterizerModule::initialize()
{
  //Register collections
  storeDATCONSVDCluster.registerInDataStore(m_storeDATCONSVDClustersName, DataStore::c_DontWriteOut);
  m_storeDATCONSVDClustersName = storeDATCONSVDCluster.getName();

  storeDATCONSVDDigits.isRequired(m_storeDATCONSVDDigitsListName);
  m_storeDATCONSVDDigitsListName = storeDATCONSVDDigits.getName();

  storeDATCONSVDCluster.registerRelationTo(storeDATCONSVDDigits, DataStore::c_Event, DataStore::c_DontWriteOut);

  storeTrueHits.isOptional(m_storeTrueHitsName);
  if (storeTrueHits.isValid()) {
    m_storeTrueHitsName = storeTrueHits.getName();
    storeDATCONSVDCluster.registerRelationTo(storeTrueHits, DataStore::c_Event, DataStore::c_DontWriteOut);
  }

  storeMCParticles.isOptional(m_storeMCParticlesName);
  if (storeMCParticles.isValid()) {
    m_storeMCParticlesName = storeMCParticles.getName();
    storeDATCONSVDCluster.registerRelationTo(storeMCParticles, DataStore::c_Event, DataStore::c_DontWriteOut);
  }

}

void DATCONSVDSimpleClusterizerModule::beginRun()
{
  svdNoiseMapU.clear();
  svdNoiseMapV.clear();
  fillDATCONSVDNoiseMap();
}



void DATCONSVDSimpleClusterizerModule::event()
{
  int nDigits = storeDATCONSVDDigits.getEntries();
  if (nDigits == 0)
    return;

  storeDATCONSVDCluster.clear();
  clusterCandidates.clear();


  //create a dummy cluster just to start
  bool uStripFirstDigit = storeDATCONSVDDigits[0]->isUStrip();
  unsigned short maxClusterSize = uStripFirstDigit ? m_maxClusterSizeU : m_maxClusterSizeV;
  float requiredSNRcluster      = uStripFirstDigit ? m_requiredSNRclusterU : m_requiredSNRclusterV;
  DATCONSVDSimpleClusterCandidate clusterCandidate(storeDATCONSVDDigits[0]->getSensorID(), uStripFirstDigit,
                                                   maxClusterSize, requiredSNRcluster);

  unsigned short digitindex = 0;

  //loop over the DATCONSVDDigits
  for (auto& datconsvddigit : storeDATCONSVDDigits) {

    float stripSNR = calculateSNR(datconsvddigit);
    if (stripSNR < (datconsvddigit.isUStrip() ? m_requiredSNRstripU : m_requiredSNRstripV)) {
      digitindex++;
      continue;
    }

    //retrieve the VxdID, sensor and cellID of the current DATCONSVDDigit
    VxdID thisSensorID = datconsvddigit.getSensorID();
    bool thisSide = datconsvddigit.isUStrip();
    unsigned short thisCellID = datconsvddigit.getCellID();
    unsigned short thisCharge = datconsvddigit.getMaxSampleCharge();

    //try to add the strip to the existing cluster
    if (! clusterCandidate.add(thisSensorID, thisSide, digitindex, thisCharge, thisCellID, stripSNR)) {
      //if the strip is not added, write the cluster, if present and good:
      if (clusterCandidate.size() > 0) {
        if (m_useSimpleClustering) {
          clusterCandidate.finalizeSimpleCluster();
        } else {
          B2WARNING("This one is not yet implemented, so no DATCONSVDCluster will be created! Skipping...");
        }
        if (clusterCandidate.isGoodCluster()) {
          clusterCandidates.push_back(clusterCandidate);
        }
      }

      //prepare for the next cluster:
      maxClusterSize      = thisSide ? m_maxClusterSizeU : m_maxClusterSizeV;
      requiredSNRcluster  = thisSide ? m_requiredSNRclusterU : m_requiredSNRclusterV;
      clusterCandidate = DATCONSVDSimpleClusterCandidate(thisSensorID, thisSide, maxClusterSize, requiredSNRcluster);

      //start another cluster:
      if (! clusterCandidate.add(thisSensorID, thisSide, digitindex, thisCharge, thisCellID, stripSNR))
        B2WARNING("this state is forbidden!!");
    }
    digitindex++;
  } //exit loop on ShaperDigits

  //write the last cluster, if good
  if (clusterCandidate.size() > 0) {
    if (m_useSimpleClustering) {
      clusterCandidate.finalizeSimpleCluster();
    } else {
      B2WARNING("This one is not yet implemented, so no DATCONSVDCluster will be created! Skipping...");
    }
    if (clusterCandidate.isGoodCluster())
      clusterCandidates.push_back(clusterCandidate);
  }

  saveClusters();

  B2DEBUG(1, "Number of clusters: " << storeDATCONSVDCluster.getEntries());
}


void DATCONSVDSimpleClusterizerModule::saveClusters()
{

  for (auto clustercandit = clusterCandidates.begin(); clustercandit != clusterCandidates.end(); clustercandit++) {

    DATCONSVDSimpleClusterCandidate clustercand = *clustercandit;

    VxdID sensorID = clustercand.getSensorID();
    bool isU = clustercand.isUSide();
    float clusterPosition = clustercand.getPosition();
    unsigned short clusterSeedCharge = clustercand.getSeedCharge();
    unsigned short clusterCharge = clustercand.getCharge();
    unsigned short clusterSize = clustercand.size();
    float pitch;
    float clusterPositionError = 0;

    const SVD::SensorInfo* aSensorInfo = dynamic_cast<const SVD::SensorInfo*>(&VXD::GeoCache::get(sensorID));

    if (isU) {
      pitch = aSensorInfo->getUPitch();
    } else {
      pitch = aSensorInfo->getVPitch();
    }

    if (clusterSize == 1) {
      clusterPositionError = pitch / sqrt(12.);
    } else if (clusterSize == 2) {
      clusterPositionError = pitch / 2.;
    } else if (clusterSize > 2) {
      clusterPositionError = pitch;
    }

    SVDCluster* datconSVDCluster =
      storeDATCONSVDCluster.appendNew(SVDCluster(sensorID, isU, clusterPosition, clusterPositionError,
                                                 0.0, 0.0, clusterCharge, clusterSeedCharge, clusterSize, 0.0, 0.0));

    vector<unsigned short> indices = clustercand.getIndexVector();

    for (auto digitindexit = indices.begin(); digitindexit != indices.end(); digitindexit++) {

      DATCONSVDDigit2* datconsvddigit = storeDATCONSVDDigits[*digitindexit];
      RelationVector<MCParticle> relatedMC = datconsvddigit->getRelationsTo<MCParticle>();
      RelationVector<SVDTrueHit> relatedSVDTrue = datconsvddigit->getRelationsTo<SVDTrueHit>();

      // Register relation to the DATCONSVDDigits this cluster belongs to
      datconSVDCluster->addRelationTo(datconsvddigit);

      // Register relations to the MCParticles and SVDTrueHits
      if (relatedMC.size() > 0) {
        for (unsigned int relmcindex = 0; relmcindex < relatedMC.size(); relmcindex++) {
          datconSVDCluster->addRelationTo(relatedMC[relmcindex], relatedMC.weight(relmcindex));
        }
      }
      if (relatedSVDTrue.size() > 0) {
        for (unsigned int reltruehitindex = 0; reltruehitindex < relatedSVDTrue.size(); reltruehitindex++) {
          datconSVDCluster->addRelationTo(relatedSVDTrue[reltruehitindex], relatedSVDTrue.weight(reltruehitindex));
        }
      }
    }
  }
}

// /*
// * Simple noise filter.
// * Run the noise filter over the given numbers of samples.
// * If it fulfills the requirements true is returned.
// */
float DATCONSVDSimpleClusterizerModule::calculateSNR(DATCONSVDDigit2 datconsvddigit)
{
  unsigned short maxSampleIndex = datconsvddigit.getMaxSampleIndex();
  DATCONSVDDigit2::APVRawSamples sample = datconsvddigit.getRawSamples();
  unsigned short stripID = datconsvddigit.getCellID();
  bool side = datconsvddigit.isUStrip();
  VxdID sensorID = datconsvddigit.getSensorID();
  int simpleVXDID = 131072 * (sensorID.getLayerNumber() - 3) + 8192 * (sensorID.getLadderNumber() - 1)
                    + 1024 * (sensorID.getSensorNumber() - 1) + stripID;

  float stripNoise = side ? m_noiseCutU : m_noiseCutV;
  if (side == true && svdNoiseMapU.find(simpleVXDID) != svdNoiseMapU.end()) {
    stripNoise = svdNoiseMapU.at(simpleVXDID);
  } else if (side == false && svdNoiseMapV.find(simpleVXDID) != svdNoiseMapV.end()) {
    stripNoise = svdNoiseMapV.at(simpleVXDID);
  }

  float currentSNR = (float)sample[maxSampleIndex] / stripNoise;

  return currentSNR;

}


void DATCONSVDSimpleClusterizerModule::fillDATCONSVDNoiseMap()
{
  //call for a geometry instance
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> svdLayers = aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();

  std::ofstream noiseMapU;
  std::ofstream noiseMapV;
  if (m_writeNoiseMapsToFile) {
    noiseMapU.open("svdNoiseMapU.txt", std::ios::trunc);
    noiseMapV.open("svdNoiseMapV.txt", std::ios::trunc);
  }

  while ((itSvdLayers != svdLayers.end()) && (itSvdLayers->getLayerNumber() != 7)) { //loop on Layers

    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) { //loop on Ladders

      std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();
      B2DEBUG(1, "    svd sensor info " << * (svdSensors.begin()));

      while (itSvdSensors != svdSensors.end()) { //loop on sensors
        B2DEBUG(1, "    svd sensor info " << *itSvdSensors);

        int layer = itSvdSensors->getLayerNumber();
        int ladder =  itSvdSensors->getLadderNumber();
        int sensor = itSvdSensors->getSensorNumber();
        Belle2::VxdID theVxdID(layer, ladder, sensor);
        const SVD::SensorInfo* currentSensorInfo = dynamic_cast<const SVD::SensorInfo*>(&VXD::GeoCache::get(theVxdID));

        int simpleVXDID = 131072 * (layer - 3) + 8192 * (ladder - 1) + 1024 * (sensor - 1);

        for (int side = 0; side < 2; side++) {

          int Ncells = currentSensorInfo->getUCells();
          if (side == 0)
            Ncells = currentSensorInfo->getVCells();

          for (int strip = 0; strip < Ncells; strip++) {

//             m_mask = -1;
//             if (m_MaskedStr.isValid())
//               m_mask = m_MaskedStr.isMasked(theVxdID, side, strip);

            float noise = -1;
//             m_noiseEl = -1;
            if (m_NoiseCal.isValid()) {
              noise = m_NoiseCal.getNoise(theVxdID, side, strip);
//               m_noiseEl = m_NoiseCal.getNoiseInElectrons(theVxdID, side, strip);
            }

            if (side == 0 && noise > m_noiseCutV) {
              svdNoiseMapV.insert(std::make_pair(simpleVXDID + strip, noise));
              if (m_writeNoiseMapsToFile) {
//                 noiseMapV << layer << " " << ladder << " " << sensor << " " << strip << " " << noise << endl;
                noiseMapV << 4096 * (layer - 3) + 16 * (ladder - 1) + (sensor - 1) << " " << strip << " " << noise << endl;
              }
            } else if (side == 1 && noise > m_noiseCutU) {
              svdNoiseMapU.insert(std::make_pair(simpleVXDID + strip, noise));
              if (m_writeNoiseMapsToFile) {
//                 noiseMapU << layer << " " << ladder << " " << sensor << " " << strip << " " << noise << endl;
                noiseMapU << 4096 * (layer - 3) + 16 * (ladder - 1) + (sensor - 1) << " " << strip << " " << noise << endl;
              }
            }

//             m_pedestal = -1;
//             if (m_PedestalCal.isValid())
//               m_pedestal = m_PedestalCal.getPedestal(theVxdID, side, strip);
//
//             m_gain = -1;
//             if (m_PulseShapeCal.isValid()) {
//               m_gain = m_PulseShapeCal.getChargeFromADC(theVxdID, side, strip, 1/*ADC*/);
//               m_calPeakADC = 22500. / m_PulseShapeCal.getChargeFromADC(theVxdID, side, strip, 1/*ADC*/);
//               m_calPeakTime = m_PulseShapeCal.getPeakTime(theVxdID, side, strip);
//               m_pulseWidth = m_PulseShapeCal.getWidth(theVxdID, side, strip);
//             }
          }
        }
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }

  if (m_writeNoiseMapsToFile) {
    noiseMapU.close();
    noiseMapV.close();
  }

}

