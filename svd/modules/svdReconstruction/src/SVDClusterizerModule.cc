/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdReconstruction/SVDClusterizerModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/core/Environment.h>

#include <svd/geometry/SensorInfo.h>
#include <svd/dataobjects/SVDEventInfo.h>

#include <svd/reconstruction/SVDReconstructionBase.h>

#include <svd/reconstruction/SVDRecoTimeFactory.h>
#include <svd/reconstruction/SVDRecoChargeFactory.h>
#include <svd/reconstruction/SVDRecoPositionFactory.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDClusterizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDClusterizerModule::SVDClusterizerModule() : Module(),
  m_cutSeed(5.0), m_cutAdjacent(3.0), m_useDB(true)
{
  //Set module properties
  setDescription("This module produces SVDClusters from SVDShaperDigits, providing 1-D hit position, charge and time on SVD sensors.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("ShaperDigits", m_storeShaperDigitsName,
           "SVDShaperDigits collection name.", string(""));
  addParam("Clusters", m_storeClustersName,
           "SVDCluster collection name.", string(""));
  addParam("SVDTrueHits", m_storeTrueHitsName,
           "TrueHit collection name.", string(""));
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticles collection name.", string(""));

  // 2. Clustering
  addParam("AdjacentSN", m_cutAdjacent,
           "minimum SNR for strips to be considered for clustering.", m_cutAdjacent);
  addParam("returnClusterRawTime", m_returnRawClusterTime,
           "if True, returns the raw cluster time (to be used for time calibration).",
           m_returnRawClusterTime);
  addParam("SeedSN", m_cutSeed,
           "minimum SNR for strips to be considered as cluster seed", m_cutSeed);
  addParam("ClusterSN", m_cutCluster,
           "minimum value of the SNR of the cluster", m_cutCluster);
  addParam("timeAlgorithm6Samples", m_timeRecoWith6SamplesAlgorithm,
           "cluster-time reconstruction algorithm for the 6-sample DAQ mode:  CoG6 = 6-sample CoG (default), CoG3 = 3-sample CoG,  ELS3 = 3-sample ELS. Overwritten by the dbobject if useDB = True (default).",
           m_timeRecoWith6SamplesAlgorithm);
  addParam("timeAlgorithm3Samples", m_timeRecoWith3SamplesAlgorithm,
           "cluster-time reconstruction algorithm for the 3-sample DAQ mode:  CoG6 = 6-sample CoG, CoG3 = 3-sample CoG (default),  ELS3 = 3-sample ELS. Overwritten by the dbobject if useDB = True (default).",
           m_timeRecoWith3SamplesAlgorithm);
  addParam("chargeAlgorithm6Samples", m_chargeRecoWith6SamplesAlgorithm,
           "cluster-charge reconstruction algorithm for 6-sample DAQ mode:  MaxSample (default), SumSamples,  ELS3 = 3-sample ELS. Overwritten by the dbobject if useDB = True (default).",
           m_chargeRecoWith6SamplesAlgorithm);
  addParam("chargeAlgorithm3Samples", m_chargeRecoWith3SamplesAlgorithm,
           "cluster-charge reconstruction algorithm for 3-sample DAQ mode:  MaxSample (default), SumSamples,  ELS3 = 3-sample ELS. Overwritten by the dbobject if useDB = True (default).",
           m_chargeRecoWith3SamplesAlgorithm);
  addParam("positionAlgorithm6Samples", m_positionRecoWith6SamplesAlgorithm,
           "cluster-position reconstruction algorithm for 6-sample DAQ mode:  old (default), CoGOnly. Overwritten by the dbobject if useDB = True (default).",
           m_positionRecoWith6SamplesAlgorithm);
  addParam("positionAlgorithm3Samples", m_positionRecoWith3SamplesAlgorithm,
           "cluster-position reconstruction algorithm for 3-sample DAQ mode:  old (default), CoGOnly. Overwritten by the dbobject if useDB = True (default).",
           m_positionRecoWith3SamplesAlgorithm);

  addParam("stripTimeAlgorithm6Samples", m_stripTimeRecoWith6SamplesAlgorithm,
           "strip-time reconstruction algorithm used for cluster position reconstruction for the 6-sample DAQ mode: dontdo = not done (default), CoG6 = 6-sample CoG, CoG3 = 3-sample CoG,  ELS3 = 3-sample ELS. Overwritten by the dbobject if useDB = True (default).",
           m_stripTimeRecoWith6SamplesAlgorithm);
  addParam("stripTimeAlgorithm3Samples", m_stripTimeRecoWith3SamplesAlgorithm,
           "strip-time reconstruction algorithm used for cluster position reconstruction for the 3-sample DAQ mode: dontdo = not done (default), CoG6 = 6-sample CoG, CoG3 = 3-sample CoG,  ELS3 = 3-sample ELS. Overwritten by the dbobject if useDB = True (default).",
           m_stripTimeRecoWith3SamplesAlgorithm);
  addParam("stripChargeAlgorithm6Samples", m_stripChargeRecoWith6SamplesAlgorithm,
           "strip-charge reconstruction algorithm used for cluster position reconstruction for the 6-sample DAQ mode: dontdo = not done, MaxSample, SumSamples,  ELS3 = 3-sample ELS. Overwritten by the dbobject if useDB = True (default).",
           m_stripChargeRecoWith6SamplesAlgorithm);
  addParam("stripChargeAlgorithm3Samples", m_stripChargeRecoWith3SamplesAlgorithm,
           "strip-charge reconstruction algorithm used for cluster position reconstruction for the 3-sample DAQ mode: dontdo = not done, MaxSample, SumSamples,  ELS3 = 3-sample ELS. Overwritten by the dbobject if useDB = True (default).",
           m_stripChargeRecoWith3SamplesAlgorithm);

  addParam("useDB", m_useDB,
           "if False, use clustering and reconstruction configuration module parameters", m_useDB);

}

void SVDClusterizerModule::beginRun()
{
  if (m_useDB) {
    if (!m_recoConfig.isValid())
      B2FATAL("no valid configuration found for SVD reconstruction");
    else
      B2INFO("SVDRecoConfiguration: from now on we are using " << m_recoConfig->get_uniqueID());

    m_timeRecoWith6SamplesAlgorithm = m_recoConfig->getTimeRecoWith6Samples();
    m_timeRecoWith3SamplesAlgorithm = m_recoConfig->getTimeRecoWith3Samples();
    m_chargeRecoWith6SamplesAlgorithm = m_recoConfig->getChargeRecoWith6Samples();
    m_chargeRecoWith3SamplesAlgorithm = m_recoConfig->getChargeRecoWith3Samples();
    m_positionRecoWith6SamplesAlgorithm = m_recoConfig->getPositionRecoWith6Samples();
    m_positionRecoWith3SamplesAlgorithm = m_recoConfig->getPositionRecoWith3Samples();

    //strip algorithms
    m_stripTimeRecoWith6SamplesAlgorithm = m_recoConfig->getStripTimeRecoWith6Samples();
    m_stripTimeRecoWith3SamplesAlgorithm = m_recoConfig->getStripTimeRecoWith3Samples();
    m_stripChargeRecoWith6SamplesAlgorithm = m_recoConfig->getStripChargeRecoWith6Samples();
    m_stripChargeRecoWith3SamplesAlgorithm = m_recoConfig->getStripChargeRecoWith3Samples();

  }
  //check that all algorithms are available, otherwise use the default one
  SVDReconstructionBase recoBase;

  if (!recoBase.isTimeAlgorithmAvailable(m_timeRecoWith6SamplesAlgorithm)) {
    B2WARNING("cluster time algorithm " << m_timeRecoWith6SamplesAlgorithm << " is NOT available, using CoG3");
    m_timeRecoWith6SamplesAlgorithm = "CoG3";
  };

  if (!recoBase.isTimeAlgorithmAvailable(m_timeRecoWith3SamplesAlgorithm)) {
    B2WARNING("cluster time algorithm " << m_timeRecoWith3SamplesAlgorithm << " is NOT available, using CoG3");
    m_timeRecoWith3SamplesAlgorithm = "CoG3";
  };
  if (!recoBase.isChargeAlgorithmAvailable(m_chargeRecoWith6SamplesAlgorithm)) {
    B2WARNING("cluster charge algorithm " << m_chargeRecoWith6SamplesAlgorithm << " is NOT available, using MaxSample");
    m_chargeRecoWith6SamplesAlgorithm = "MaxSample";
  };
  if (!recoBase.isChargeAlgorithmAvailable(m_chargeRecoWith3SamplesAlgorithm)) {
    B2WARNING("cluster charge algorithm " << m_chargeRecoWith3SamplesAlgorithm << " is NOT available, using MaxSample");
    m_chargeRecoWith3SamplesAlgorithm = "MaxSample";
  };
  if (!recoBase.isPositionAlgorithmAvailable(m_positionRecoWith6SamplesAlgorithm)) {
    B2WARNING("cluster position algorithm " << m_positionRecoWith6SamplesAlgorithm << " is NOT available, using oldDefault");
    m_positionRecoWith6SamplesAlgorithm = "oldDefault";
  };
  if (!recoBase.isPositionAlgorithmAvailable(m_positionRecoWith3SamplesAlgorithm)) {
    B2WARNING("cluster position algorithm " << m_positionRecoWith3SamplesAlgorithm << " is NOT available, using oldDefault");
    m_positionRecoWith3SamplesAlgorithm = "oldDefault";
  };


  m_time6SampleClass = SVDRecoTimeFactory::NewTime(m_timeRecoWith6SamplesAlgorithm, m_returnRawClusterTime);
  m_time3SampleClass = SVDRecoTimeFactory::NewTime(m_timeRecoWith3SamplesAlgorithm, m_returnRawClusterTime);
  m_charge6SampleClass = SVDRecoChargeFactory::NewCharge(m_chargeRecoWith6SamplesAlgorithm);
  m_charge3SampleClass = SVDRecoChargeFactory::NewCharge(m_chargeRecoWith3SamplesAlgorithm);
  m_position6SampleClass = SVDRecoPositionFactory::NewPosition(m_positionRecoWith6SamplesAlgorithm);
  m_position6SampleClass->set_stripChargeAlgo(m_stripChargeRecoWith6SamplesAlgorithm);
  m_position6SampleClass->set_stripTimeAlgo(m_stripTimeRecoWith6SamplesAlgorithm);
  m_position3SampleClass = SVDRecoPositionFactory::NewPosition(m_positionRecoWith3SamplesAlgorithm);
  m_position3SampleClass->set_stripChargeAlgo(m_stripChargeRecoWith3SamplesAlgorithm);
  m_position3SampleClass->set_stripTimeAlgo(m_stripTimeRecoWith3SamplesAlgorithm);

  string israwtime = "";
  if (m_returnRawClusterTime) israwtime = " (raw)";
  B2INFO("SVD  6-sample DAQ, cluster time algorithm: " << m_timeRecoWith6SamplesAlgorithm << israwtime <<
         ", cluster charge algorithm: " <<
         m_chargeRecoWith6SamplesAlgorithm << ", cluster position algorithm: " << m_positionRecoWith6SamplesAlgorithm);
  B2INFO(" with strip charge reconstructed with " << m_stripChargeRecoWith6SamplesAlgorithm << " and strip time reconstructed with "
         <<
         m_stripTimeRecoWith6SamplesAlgorithm);

  B2INFO("SVD  3-sample DAQ, cluster time algorithm: " << m_timeRecoWith3SamplesAlgorithm << israwtime <<
         ", cluster charge algorithm: " <<
         m_chargeRecoWith3SamplesAlgorithm << ", cluster position algorithm: " << m_positionRecoWith3SamplesAlgorithm);
  B2INFO(" with strip charge reconstructed with " << m_stripChargeRecoWith3SamplesAlgorithm << " and strip time reconstructed with "
         <<
         m_stripTimeRecoWith3SamplesAlgorithm);
}

void SVDClusterizerModule::initialize()
{
  //Register collections
  m_storeClusters.registerInDataStore(m_storeClustersName, DataStore::c_ErrorIfAlreadyRegistered);
  m_storeDigits.isRequired(m_storeShaperDigitsName);
  m_storeTrueHits.isOptional(m_storeTrueHitsName);
  m_storeMCParticles.isOptional(m_storeMCParticlesName);

  RelationArray relClusterDigits(m_storeClusters, m_storeDigits);
  RelationArray relClusterTrueHits(m_storeClusters, m_storeTrueHits);
  RelationArray relClusterMCParticles(m_storeClusters, m_storeMCParticles);
  RelationArray relDigitTrueHits(m_storeDigits, m_storeTrueHits);
  RelationArray relDigitMCParticles(m_storeDigits, m_storeMCParticles);

  relClusterDigits.registerInDataStore();

  //Relations to simulation objects only if the ancestor relations exist
  if (relDigitTrueHits.isOptional())
    relClusterTrueHits.registerInDataStore();

  if (relDigitMCParticles.isOptional())
    relClusterMCParticles.registerInDataStore();

  //Store names to speed up creation later
  m_storeClustersName = m_storeClusters.getName();
  m_storeShaperDigitsName = m_storeDigits.getName();
  m_storeTrueHitsName = m_storeTrueHits.getName();
  m_storeMCParticlesName = m_storeMCParticles.getName();

  m_relClusterShaperDigitName = relClusterDigits.getName();
  m_relClusterTrueHitName = relClusterTrueHits.getName();
  m_relClusterMCParticleName = relClusterMCParticles.getName();
  m_relShaperDigitTrueHitName = relDigitTrueHits.getName();
  m_relShaperDigitMCParticleName = relDigitMCParticles.getName();

  // Report:
  B2DEBUG(20, "SVDClusterizer Parameters (in default system unit, *=cannot be set directly):");

  B2DEBUG(20, " 1. COLLECTIONS:");
  B2DEBUG(20, " -->  MCParticles:        " << m_storeMCParticlesName);
  B2DEBUG(20, " -->  SVDShaperDigits:      " << m_storeShaperDigitsName);
  B2DEBUG(20, " -->  SVDClusters:        " << m_storeClustersName);
  B2DEBUG(20, " -->  SVDTrueHits:        " << m_storeTrueHitsName);
  B2DEBUG(20, " 2. RELATIONS:");
  B2DEBUG(20, " -->  DigitMCRel:         " << m_relShaperDigitMCParticleName);
  B2DEBUG(20, " -->  ClusterMCRel:       " << m_relClusterMCParticleName);
  B2DEBUG(20, " -->  ClusterDigitRel:    " << m_relClusterShaperDigitName);
  B2DEBUG(20, " -->  DigitTrueRel:       " << m_relShaperDigitTrueHitName);
  B2DEBUG(20, " -->  ClusterTrueRel:     " << m_relClusterTrueHitName);
  B2DEBUG(20, " 3. CLUSTERING:");
  B2DEBUG(20, " -->  Neighbour cut:      " << m_cutAdjacent);
  B2DEBUG(20, " -->  Seed cut:           " << m_cutSeed);
}



void SVDClusterizerModule::event()
{
  int nDigits = m_storeDigits.getEntries();
  if (nDigits == 0)
    return;

  m_storeClusters.clear();


  RelationArray relClusterMCParticle(m_storeClusters, m_storeMCParticles,
                                     m_relClusterMCParticleName);
  if (relClusterMCParticle) relClusterMCParticle.clear();

  RelationArray relClusterDigit(m_storeClusters, m_storeDigits,
                                m_relClusterShaperDigitName);
  if (relClusterDigit) relClusterDigit.clear();

  RelationArray relClusterTrueHit(m_storeClusters, m_storeTrueHits,
                                  m_relClusterTrueHitName);
  if (relClusterTrueHit) relClusterTrueHit.clear();

  if (m_useDB) {
    m_cutSeed = m_ClusterCal.getMinSeedSNR(m_storeDigits[0]->getSensorID(), m_storeDigits[0]->isUStrip());
    m_cutAdjacent = m_ClusterCal.getMinAdjSNR(m_storeDigits[0]->getSensorID(), m_storeDigits[0]->isUStrip());
    m_cutCluster = m_ClusterCal.getMinClusterSNR(m_storeDigits[0]->getSensorID(), m_storeDigits[0]->isUStrip());
  }

  //create a dummy cluster just to start
  RawCluster rawCluster(m_storeDigits[0]->getSensorID(), m_storeDigits[0]->isUStrip(), m_cutSeed, m_cutAdjacent,
                        m_storeShaperDigitsName);

  //loop over the SVDShaperDigits
  for (const SVDShaperDigit& currentDigit : m_storeDigits) {

    //retrieve the VxdID, sensor and cellID of the current ShaperDigit
    VxdID thisSensorID = currentDigit.getSensorID();
    bool thisSide = currentDigit.isUStrip();
    int thisCellID = currentDigit.getCellID();

    if (m_useDB) {
      m_cutSeed = m_ClusterCal.getMinSeedSNR(thisSensorID, thisSide);
      m_cutAdjacent = m_ClusterCal.getMinAdjSNR(thisSensorID, thisSide);
      m_cutCluster = m_ClusterCal.getMinClusterSNR(thisSensorID, thisSide);
    }

    //Ignore digits with insufficient signal
    float thisNoise = m_NoiseCal.getNoise(thisSensorID, thisSide, thisCellID);
    int thisCharge = currentDigit.getMaxADCCounts();
    B2DEBUG(20, "Noise = " << thisNoise << " ADC, MaxSample = " << thisCharge << " ADC");

    if ((float)thisCharge / thisNoise < m_cutAdjacent)
      continue;

    //this strip has a sufficient S/N
    StripInRawCluster aStrip;
    aStrip.shaperDigitIndex = currentDigit.getArrayIndex();
    aStrip.maxSample = thisCharge;
    aStrip.cellID = thisCellID;
    aStrip.noise = thisNoise;
    aStrip.samples = currentDigit.getSamples();

    //try to add the strip to the existing cluster
    if (! rawCluster.add(thisSensorID, thisSide, aStrip)) {

      //if the strip is not added, write the cluster, if present and good:
      if ((rawCluster.getSize() > 0) && (rawCluster.isGoodRawCluster()))
        finalizeCluster(rawCluster);

      //prepare for the next cluster:
      rawCluster = RawCluster(thisSensorID, thisSide, m_cutSeed, m_cutAdjacent, m_storeShaperDigitsName);

      //start another cluster:
      if (! rawCluster.add(thisSensorID, thisSide, aStrip))
        B2WARNING("this state is forbidden!!");

    }
  } //exit loop on ShaperDigits

  //write the last cluster, if good
  if ((rawCluster.getSize() > 0) && (rawCluster.isGoodRawCluster()))
    finalizeCluster(rawCluster);

  B2DEBUG(20, "Number of clusters: " << m_storeClusters.getEntries());
}


void SVDClusterizerModule::finalizeCluster(Belle2::SVD::RawCluster& rawCluster)
{

  VxdID sensorID = rawCluster.getSensorID();
  bool isU = rawCluster.isUSide();
  int size = rawCluster.getSize();

  //first take Event Informations:
  StoreObjPtr<SVDEventInfo> temp_eventinfo("SVDEventInfo");
  std::string m_svdEventInfoName = "SVDEventInfo";
  if (!temp_eventinfo.isValid())
    m_svdEventInfoName = "SVDEventInfoSim";
  StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
  if (!eventinfo) B2ERROR("No SVDEventInfo!");

  m_numberOfAcquiredSamples = eventinfo->getNSamples();

  //--------------
  // CLUSTER RECO
  //--------------
  double time = std::numeric_limits<double>::quiet_NaN();
  double timeError = std::numeric_limits<double>::quiet_NaN();
  int firstFrame = std::numeric_limits<int>::quiet_NaN();

  double charge = std::numeric_limits<double>::quiet_NaN();
  double seedCharge = std::numeric_limits<float>::quiet_NaN();
  double SNR = std::numeric_limits<double>::quiet_NaN();

  double position = std::numeric_limits<float>::quiet_NaN();
  double positionError = std::numeric_limits<float>::quiet_NaN();


  if (m_numberOfAcquiredSamples == 6) {

    //time
    m_time6SampleClass->computeClusterTime(rawCluster, time, timeError, firstFrame);
    //charge
    m_charge6SampleClass->computeClusterCharge(rawCluster, charge, SNR, seedCharge);

    //position
    m_position6SampleClass->computeClusterPosition(rawCluster, position, positionError);
  } else if (m_numberOfAcquiredSamples == 3) {
    //time
    m_time3SampleClass->computeClusterTime(rawCluster, time, timeError, firstFrame);

    //charge
    m_charge3SampleClass->computeClusterCharge(rawCluster, charge, SNR, seedCharge);

    //position
    m_position3SampleClass->computeClusterPosition(rawCluster, position, positionError);

  } else //we should never get here!
    B2FATAL("SVD Reconstruction not available for this cluster (unrecognized or not supported  number of acquired APV samples!!");

  // now go into FTSW time reference frame
  time = eventinfo->getTimeInFTSWReference(time, firstFrame);

  //apply the Lorentz Shift Correction
  position = applyLorentzShiftCorrection(position, sensorID, isU);

  //append the new cluster to the StoreArray...
  if (SNR > m_cutCluster) {
    m_storeClusters.appendNew(sensorID, isU, position, positionError, time, timeError, charge, seedCharge, size, SNR, -1,
                              firstFrame);

    B2DEBUG(20, "CLUSTER SIZE = " << size);
    B2DEBUG(20, "        time = " << time << ", timeError = " << timeError << ", firstframe = " << firstFrame);
    B2DEBUG(20, "        charge = " << charge << ", SNR = " << SNR << ", seedCharge = " << seedCharge);
    B2DEBUG(20, "        position = " << position << ", positionError = " << positionError);

    //..and write relations
    writeClusterRelations(rawCluster);
  }
}

void SVDClusterizerModule::writeClusterRelations(Belle2::SVD::RawCluster& rawCluster)
{
  RelationArray relClusterDigit(m_storeClusters, m_storeDigits, m_relClusterShaperDigitName);

  RelationArray relClusterMCParticle(m_storeClusters, m_storeMCParticles, m_relClusterMCParticleName);
  RelationArray relClusterTrueHit(m_storeClusters, m_storeTrueHits, m_relClusterTrueHitName);

  RelationIndex<SVDShaperDigit, MCParticle> relDigitMCParticle(m_storeDigits, m_storeMCParticles, m_relShaperDigitMCParticleName);
  RelationIndex<SVDShaperDigit, SVDTrueHit> relDigitTrueHit(m_storeDigits, m_storeTrueHits, m_relShaperDigitTrueHitName);


  //register relation between ShaperDigit and Cluster
  int clsIndex = m_storeClusters.getEntries() - 1;

  map<int, float> mc_relations;
  map<int, float> truehit_relations;

  vector<pair<int, float> > digit_weights;
  digit_weights.reserve(m_storeClusters[clsIndex]->getSize());

  std::vector<StripInRawCluster> strips = rawCluster.getStripsInRawCluster();

  for (const auto& strip : strips) {

    //Fill map with MCParticle relations
    if (relDigitMCParticle) {
      typedef const RelationIndex<SVDShaperDigit, MCParticle>::Element relMC_type;
      for (relMC_type& mcRel : relDigitMCParticle.getElementsFrom(m_storeDigits[strip.shaperDigitIndex])) {
        //negative weights are from ignored particles, we don't like them and
        //thus ignore them :D
        if (mcRel.weight < 0) continue;
        mc_relations[mcRel.indexTo] += mcRel.weight;
      };
    };
    //Fill map with SVDTrueHit relations
    if (relDigitTrueHit) {
      typedef const RelationIndex<SVDShaperDigit, SVDTrueHit>::Element relTrueHit_type;
      for (relTrueHit_type& trueRel : relDigitTrueHit.getElementsFrom(m_storeDigits[strip.shaperDigitIndex])) {
        //negative weights are from ignored particles, we don't like them and
        //thus ignore them :D
        if (trueRel.weight < 0) continue;
        truehit_relations[trueRel.indexTo] += trueRel.weight;
      };
    };

    digit_weights.push_back(make_pair(strip.shaperDigitIndex, strip.maxSample));
  }

  //Create Relations to this Digit
  if (!mc_relations.empty()) {
    relClusterMCParticle.add(clsIndex, mc_relations.begin(), mc_relations.end());
  }
  if (!truehit_relations.empty()) {
    relClusterTrueHit.add(clsIndex, truehit_relations.begin(), truehit_relations.end());
  }

  relClusterDigit.add(clsIndex, digit_weights.begin(), digit_weights.end());

}

double SVDClusterizerModule::applyLorentzShiftCorrection(double position, VxdID vxdID, bool isU)
{

  //Lorentz shift correction - PATCHED
  //NOTE: layer 3 is upside down with respect to L4,5,6 in the real data (real SVD), but _not_ in the simulation. We need to change the sign of the Lorentz correction on L3 only if reconstructing data, i.e. if Environment::Instance().isMC() is FALSE.

  const SensorInfo& sensorInfo = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(vxdID));

  bool isMC = Environment::Instance().isMC();

  if ((vxdID.getLayerNumber() == 3) && ! isMC)
    position += sensorInfo.getLorentzShift(isU, position);
  else
    position -= sensorInfo.getLorentzShift(isU, position);

  return position;
}
void SVDClusterizerModule::endRun()
{

  delete m_time6SampleClass;
  delete m_time3SampleClass;
  delete m_charge6SampleClass;
  delete m_charge3SampleClass;
  delete m_position6SampleClass;
  delete m_position3SampleClass;

}
