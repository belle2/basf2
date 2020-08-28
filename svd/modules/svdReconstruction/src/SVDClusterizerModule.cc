/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
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

#include <svd/geometry/SensorInfo.h>
#include <svd/dataobjects/SVDEventInfo.h>

#include <svd/reconstruction/SVDRecoTimeFactory.h>
//#include <svd/reconstruction/SVDRecoChargeFactory.h>

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
  setDescription("Clusterize SVDShaperDigits");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("ShaperDigits", m_storeShaperDigitsName,
           "SVDShaperDigits collection name", string(""));
  addParam("Clusters", m_storeClustersName,
           "SVDCluster collection name", string(""));
  addParam("SVDTrueHits", m_storeTrueHitsName,
           "TrueHit collection name", string(""));
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticles collection name", string(""));

  // 2. Clustering
  addParam("AdjacentSN", m_cutAdjacent,
           "SN for digits to be considered for clustering", m_cutAdjacent);
  addParam("SeedSN", m_cutSeed,
           "SN for digits to be considered as seed", m_cutSeed);
  addParam("ClusterSN", m_cutCluster,
           "minimum value of the SNR of the cluster", m_cutCluster);
  addParam("timeAlgorithm6Samples", m_timeRecoWith6SamplesAlgorithm,
           " choose time algorithm for the 6-sample DAQ mode:  CoG6 = 6-sample CoG (default), CoG3 = 3-sample CoG,  ELS3 = 3-sample ELS",
           m_timeRecoWith6SamplesAlgorithm);
  addParam("timeAlgorithm3Samples", m_timeRecoWith3SamplesAlgorithm,
           " choose time algorithm for the 3-sample DAQ mode:  CoG6 = 6-sample CoG, CoG3 = 3-sample CoG (default),  ELS3 = 3-sample ELS",
           m_timeRecoWith3SamplesAlgorithm);
  addParam("chargeAlgorithm6Samples", m_chargeRecoWith6SamplesAlgorithm,
           " choose charge algorithm for 6-sample DAQ mode:  MaxSample (default), SumSamples,  ELS3 = 3-sample ELS",
           m_chargeRecoWith6SamplesAlgorithm);
  addParam("chargeAlgorithm3Samples", m_chargeRecoWith3SamplesAlgorithm,
           " choose charge algorithm for 3-sample DAQ mode:  MaxSample (default), SumSamples,  ELS3 = 3-sample ELS",
           m_chargeRecoWith3SamplesAlgorithm);

  addParam("useDB", m_useDB,
           "if false use clustering and reconstruction configuration module parameters", m_useDB);

}

void SVDClusterizerModule::beginRun()
{

  if (m_useDB) {
    if (!m_recoConfig.isValid())
      B2ERROR("no valid configuration found for SVD reconstruction");
    else
      B2INFO("SVDRecoConfiguration: from now on we are using " << m_recoConfig->get_uniqueID());

    m_timeRecoWith6SamplesAlgorithm = m_recoConfig->getTimeRecoWith6Samples();
    m_timeRecoWith3SamplesAlgorithm = m_recoConfig->getTimeRecoWith3Samples();
    m_chargeRecoWith6SamplesAlgorithm = m_recoConfig->getChargeRecoWith6Samples();
    m_chargeRecoWith3SamplesAlgorithm = m_recoConfig->getChargeRecoWith3Samples();

  }

  m_time6SampleClass = SVDRecoTimeFactory::NewTime(m_timeRecoWith6SamplesAlgorithm);
  m_time3SampleClass = SVDRecoTimeFactory::NewTime(m_timeRecoWith3SamplesAlgorithm);
  //  m_charge6SampleClass = SVDRecoChargeFactory::NewCharge(m_chargeRecoWith6SamplesAlgorithm);
  //  m_charge3SampleClass = SVDRecoChargeFactory::NewCharge(m_chargeRecoWith3SamplesAlgorithm);


  B2INFO("SVD  6-sample DAQ, time algorithm:" << m_timeRecoWith6SamplesAlgorithm <<  ", charge algorithm: " <<
         m_chargeRecoWith6SamplesAlgorithm);

  B2INFO("SVD  3-sample DAQ, time algorithm:" << m_timeRecoWith3SamplesAlgorithm <<  ", charge algorithm: " <<
         m_chargeRecoWith3SamplesAlgorithm);
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
  B2DEBUG(1, "SVDClusterizer Parameters (in default system unit, *=cannot be set directly):");

  B2DEBUG(1, " 1. COLLECTIONS:");
  B2DEBUG(1, " -->  MCParticles:        " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2DEBUG(1, " -->  SVDShaperDigits:      " << DataStore::arrayName<SVDShaperDigit>(m_storeShaperDigitsName));
  B2DEBUG(1, " -->  SVDClusters:        " << DataStore::arrayName<SVDCluster>(m_storeClustersName));
  B2DEBUG(1, " -->  SVDTrueHits:        " << DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName));
  B2DEBUG(1, " -->  DigitMCRel:         " << m_relShaperDigitMCParticleName);
  B2DEBUG(1, " -->  ClusterMCRel:       " << m_relClusterMCParticleName);
  B2DEBUG(1, " -->  ClusterDigitRel:    " << m_relClusterShaperDigitName);
  B2DEBUG(1, " -->  DigitTrueRel:       " << m_relShaperDigitTrueHitName);
  B2DEBUG(1, " -->  ClusterTrueRel:     " << m_relClusterTrueHitName);
  B2DEBUG(1, " 2. CLUSTERING:");
  B2DEBUG(1, " -->  Neighbour cut:      " << m_cutAdjacent);
  B2DEBUG(1, " -->  Seed cut:           " << m_cutSeed);
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
  int i = 0;
  while (i < nDigits) {

    //retrieve the VxdID, sensor and cellID of the current ShaperDigit
    VxdID thisSensorID = m_storeDigits[i]->getSensorID();
    bool thisSide = m_storeDigits[i]->isUStrip();
    int thisCellID = m_storeDigits[i]->getCellID();

    if (m_useDB) {
      m_cutSeed = m_ClusterCal.getMinSeedSNR(thisSensorID, thisSide);
      m_cutAdjacent = m_ClusterCal.getMinAdjSNR(thisSensorID, thisSide);
      m_cutCluster = m_ClusterCal.getMinClusterSNR(thisSensorID, thisSide);
    }

    //Ignore digits with insufficient signal
    float thisNoise = m_NoiseCal.getNoise(thisSensorID, thisSide, thisCellID);
    float thisCharge = m_storeDigits[i]->getMaxADCCounts();
    B2DEBUG(10, "Noise = " << thisNoise << " ADC, MaxSample = " << thisCharge << " ADC");

    if ((float)thisCharge / thisNoise < m_cutAdjacent) {
      i++;
      continue;
    }

    //this strip has a sufficient S/N
    stripInRawCluster aStrip;
    aStrip.shaperDigitIndex = i;
    aStrip.maxSample = thisCharge;
    aStrip.cellID = thisCellID;
    aStrip.noise = thisNoise;
    aStrip.samples = m_storeDigits[i]->getSamples();

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
    i++;
  } //exit loop on ShaperDigits

  //write the last cluster, if good
  if ((rawCluster.getSize() > 0) && (rawCluster.isGoodRawCluster()))
    finalizeCluster(rawCluster);

  B2DEBUG(1, "Number of clusters: " << m_storeClusters.getEntries());
}


void SVDClusterizerModule::finalizeCluster(Belle2::SVD::RawCluster& rawCluster)
{

  VxdID sensorID = rawCluster.getSensorID();
  bool isU = rawCluster.isUSide();
  int size = rawCluster.getSize();

  //first take Event Informations:
  // 1. acquisition mode (6 or 3 sample)

  StoreObjPtr<SVDEventInfo> temp_eventinfo("SVDEventInfo");
  std::string m_svdEventInfoName = "SVDEventInfo";
  if (!temp_eventinfo.isValid())
    m_svdEventInfoName = "SVDEventInfoSim";
  StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
  if (!eventinfo) B2ERROR("No SVDEventInfo!");

  int daqMode = eventinfo->getModeByte().getDAQMode();
  if (daqMode == 2) m_numberOfAcquiredSamples = 6;
  if (daqMode == 1) m_numberOfAcquiredSamples = 3;
  if (daqMode == 0) m_numberOfAcquiredSamples = 1;

  //--------------
  // CLUSTER TIME
  //--------------
  double time = std::numeric_limits<double>::quiet_NaN();
  double timeError = std::numeric_limits<double>::quiet_NaN();
  int firstFrame = std::numeric_limits<int>::quiet_NaN();

  // cluster time computation
  if (m_numberOfAcquiredSamples == 6) {
    m_time6SampleClass->setRawCluster(rawCluster);
    time = m_time6SampleClass->getClusterTime();
    timeError = m_time6SampleClass->getClusterTimeError();
    firstFrame = m_time6SampleClass->getFirstFrame();
  } else if (m_numberOfAcquiredSamples == 3) {
    m_time3SampleClass->setRawCluster(rawCluster);
    time = m_time3SampleClass->getClusterTime();
    timeError = m_time3SampleClass->getClusterTimeError();
    firstFrame = m_time3SampleClass->getFirstFrame();
  } else
    B2ERROR("SVD Reconstruction not available for this cluster (unrecognized or not supported  number of acquired APV samples!!");

  // now go into FTSW reference frame
  time = time + eventinfo->getSVD2FTSWTimeShift(firstFrame);


  //-----------------
  // CLUSTER POSITION
  //-----------------

  float position = std::numeric_limits<float>::quiet_NaN();
  float positionError = std::numeric_limits<float>::quiet_NaN();
  //  float positionError = m_ClusterCal.getCorrectedClusterPositionError(sensorID, isU, size, cluster.getPositionError());  double time = 0;


  //----------------
  // CLUSTER CHARGE
  //----------------
  float charge = std::numeric_limits<float>::quiet_NaN();
  float seedCharge = std::numeric_limits<float>::quiet_NaN();

  /*
  // cluster charge computation
  if(m_numberOfAcquiredSamples == 6)
    {
      m_charge6SampleClass->setRawCluster(rawCluster);
      charge = m_charge6SampleClass->getClusterCharge();
      seedCharge = m_charge6SampleClass->getClusterSeedCharge();
    }
  else
    if(m_numberOfAcquiredSamples == 3)
      {
  m_charge3SampleClass->setRawCluster(rawCluster);
  charge = m_charge3SampleClass->getClusterCharge();
  seedCharge = m_charge3SampleClass->getClusterSeedCharge();
      } else
      B2ERROR("SVD Reconstruction not available for this cluster (unrecognized or not supported  number of acquired APV samples!!");
  */

  float SNR = 0;

  m_storeClusters.appendNew(SVDCluster(sensorID, isU, position, positionError, time, timeError, charge, seedCharge, size, SNR, -1,
                                       firstFrame));

  writeClusterRelations(rawCluster);
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

  std::vector<stripInRawCluster> strips = rawCluster.getStripsInRawCluster();

  for (auto strip : strips) {

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

