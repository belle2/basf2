/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdReconstruction/SVDSimpleClusterizerModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>

#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDSimpleClusterizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDSimpleClusterizerModule::SVDSimpleClusterizerModule() : Module(),
  m_cutSeed(5.0), m_cutAdjacent(3.0), m_sizeHeadTail(3)
{
  //Set module properties
  setDescription("Clusterize SVDRecoDigits fitted by the bCenter of Gravity estimator");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("RecoDigits", m_storeRecoDigitsName,
           "SVDRecoDigits collection name", string(""));
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
  addParam("HeadTailSize", m_sizeHeadTail,
           "Cluster size at which to switch to Analog head tail algorithm", m_sizeHeadTail);


}

void SVDSimpleClusterizerModule::initialize()
{
  //Register collections
  StoreArray<SVDCluster> storeClusters(m_storeClustersName);
  StoreArray<SVDRecoDigit> storeDigits(m_storeRecoDigitsName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);

  storeClusters.registerInDataStore();
  storeDigits.isRequired();
  storeTrueHits.isOptional();
  storeMCParticles.isOptional();

  RelationArray relClusterDigits(storeClusters, storeDigits);
  RelationArray relClusterTrueHits(storeClusters, storeTrueHits);
  RelationArray relClusterMCParticles(storeClusters, storeMCParticles);
  RelationArray relDigitTrueHits(storeDigits, storeTrueHits);
  RelationArray relDigitMCParticles(storeDigits, storeMCParticles);

  relClusterDigits.registerInDataStore();
  //Relations to simulation objects only if the ancestor relations exist
  if (relDigitTrueHits.isOptional())
    relClusterTrueHits.registerInDataStore();
  if (relDigitMCParticles.isOptional())
    relClusterMCParticles.registerInDataStore();

  //Store names to speed up creation later
  m_storeClustersName = storeClusters.getName();
  m_storeRecoDigitsName = storeDigits.getName();
  m_storeTrueHitsName = storeTrueHits.getName();
  m_storeMCParticlesName = storeMCParticles.getName();

  m_relClusterRecoDigitName = relClusterDigits.getName();
  m_relClusterTrueHitName = relClusterTrueHits.getName();
  m_relClusterMCParticleName = relClusterMCParticles.getName();
  m_relRecoDigitTrueHitName = relDigitTrueHits.getName();
  m_relRecoDigitMCParticleName = relDigitMCParticles.getName();

  // Report:
  B2INFO("SVDSimpleClusterizer Parameters (in default system unit, *=cannot be set directly):");

  B2INFO(" 1. COLLECTIONS:");
  B2INFO(" -->  MCParticles:        " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2INFO(" -->  SVDRecoDigits:      " << DataStore::arrayName<SVDRecoDigit>(m_storeRecoDigitsName));
  B2INFO(" -->  SVDClusters:        " << DataStore::arrayName<SVDCluster>(m_storeClustersName));
  B2INFO(" -->  SVDTrueHits:        " << DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName));
  B2INFO(" -->  DigitMCRel:         " << m_relRecoDigitMCParticleName);
  B2INFO(" -->  ClusterMCRel:       " << m_relClusterMCParticleName);
  B2INFO(" -->  ClusterDigitRel:    " << m_relClusterRecoDigitName);
  B2INFO(" -->  DigitTrueRel:       " << m_relRecoDigitTrueHitName);
  B2INFO(" -->  ClusterTrueRel:     " << m_relClusterTrueHitName);
  B2INFO(" 2. CLUSTERING:");
  B2INFO(" -->  Neighbour cut:      " << m_cutAdjacent);
  B2INFO(" -->  Seed cut:           " << m_cutSeed);
  B2INFO(" -->  Size HeadTail:      " << m_sizeHeadTail);
}



void SVDSimpleClusterizerModule::event()
{
  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  const StoreArray<SVDRecoDigit> storeDigits(m_storeRecoDigitsName);
  StoreArray<SVDCluster> storeClusters(m_storeClustersName);

  int nDigits = storeDigits.getEntries();
  if (nDigits == 0)
    return;

  storeClusters.clear();

  RelationArray relClusterMCParticle(storeClusters, storeMCParticles,
                                     m_relClusterMCParticleName);
  if (relClusterMCParticle) relClusterMCParticle.clear();

  RelationArray relClusterDigit(storeClusters, storeDigits,
                                m_relClusterRecoDigitName);
  if (relClusterDigit) relClusterDigit.clear();

  RelationArray relClusterTrueHit(storeClusters, storeTrueHits,
                                  m_relClusterTrueHitName);
  if (relClusterTrueHit) relClusterTrueHit.clear();


  //create a dummy cluster just to start
  SimpleClusterCandidate clusterCandidate(storeDigits[0]->getSensorID(), storeDigits[0]->isUStrip(),
                                          m_sizeHeadTail, m_cutSeed, m_cutAdjacent);

  //loop over the SVDRecoDigits
  int i = 0;
  while (i < nDigits) {

    //retrieve the VxdID, sensor and cellID of the current RecoDigit
    VxdID thisSensorID = storeDigits[i]->getSensorID();
    bool thisSide = storeDigits[i]->isUStrip();
    int thisCellID = storeDigits[i]->getCellID();

    //Ignore digits with insufficient signal
    float ADCnoise = m_NoiseCal.getNoise(thisSensorID, thisSide, thisCellID);
    float thisNoise = m_PulseShapeCal.getChargeFromADC(thisSensorID, thisSide, thisCellID, ADCnoise);
    float thisCharge = storeDigits[i]->getCharge();
    if ((float)thisCharge / thisNoise < m_cutAdjacent) {
      i++;
      continue;
    }

    //this strip has a sufficient S/N
    stripInCluster aStrip;
    aStrip.recoDigitIndex = i;
    aStrip.charge = thisCharge;
    aStrip.cellID = thisCellID;
    aStrip.noise = thisNoise;
    aStrip.time = storeDigits[i]->getTime();

    //try to add the strip to the existing cluster
    if (! clusterCandidate.add(thisSensorID, thisSide, aStrip)) {

      //if the strip is not added, write the cluster, if present and good:
      if (clusterCandidate.size() > 0) {
        clusterCandidate.finalizeCluster();
        if (clusterCandidate.isGoodCluster()) {
          writeClusters(clusterCandidate);
        }
      }

      //prepare for the next cluster:
      clusterCandidate = SimpleClusterCandidate(thisSensorID, thisSide, m_sizeHeadTail, m_cutSeed, m_cutAdjacent);

      //start another cluster:
      if (! clusterCandidate.add(thisSensorID, thisSide, aStrip))
        B2WARNING("this state is forbidden!!");

    }
    i++;
  } //exit loop on RecoDigits

  //write the last cluster, if good
  if (clusterCandidate.size() > 0) {
    clusterCandidate.finalizeCluster();
    if (clusterCandidate.isGoodCluster())
      writeClusters(clusterCandidate);
  }

  B2DEBUG(1, "Number of clusters: " << storeClusters.getEntries());
}


void SVDSimpleClusterizerModule::writeClusters(SimpleClusterCandidate cluster)
{

  StoreArray<SVDCluster> storeClusters(m_storeClustersName);
  const StoreArray<SVDRecoDigit> storeDigits(m_storeRecoDigitsName);
  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);

  RelationArray relClusterDigit(storeClusters, storeDigits, m_relClusterRecoDigitName);

  RelationArray relClusterMCParticle(storeClusters, storeMCParticles, m_relClusterMCParticleName);
  RelationArray relClusterTrueHit(storeClusters, storeTrueHits, m_relClusterTrueHitName);

  RelationIndex<SVDRecoDigit, MCParticle> relDigitMCParticle(storeDigits, storeMCParticles, m_relRecoDigitMCParticleName);
  RelationIndex<SVDRecoDigit, SVDTrueHit> relDigitTrueHit(storeDigits, storeTrueHits, m_relRecoDigitTrueHitName);


  VxdID sensorID = cluster.getSensorID();
  bool isU = cluster.isUSide();
  float position = cluster.getPosition();
  float positionError = cluster.getPositionError();
  float time = cluster.getTime();
  float timeError = cluster.getTimeError(); //not implemented yet
  float seedCharge = cluster.getSeedCharge();
  float charge = cluster.getCharge();
  float size = cluster.size();
  float SNR = cluster.getSNR();


  //  Store Cluster into Datastore
  storeClusters.appendNew(SVDCluster(
                            sensorID, isU, position, positionError, time, timeError, charge, seedCharge, size, SNR, -1
                          ));

  //register relation between RecoDigit and Cluster
  int clsIndex = storeClusters.getEntries() - 1;

  map<int, float> mc_relations;
  map<int, float> truehit_relations;

  vector<pair<int, float> > digit_weights;
  digit_weights.reserve(size);

  std::vector<stripInCluster> strips = cluster.getStripsInCluster();

  for (auto strip : strips) {

    //Fill map with MCParticle relations
    if (relDigitMCParticle) {
      typedef const RelationIndex<SVDRecoDigit, MCParticle>::Element relMC_type;
      for (relMC_type& mcRel : relDigitMCParticle.getElementsFrom(storeDigits[strip.recoDigitIndex])) {
        //negative weights are from ignored particles, we don't like them and
        //thus ignore them :D
        if (mcRel.weight < 0) continue;
        mc_relations[mcRel.indexTo] += mcRel.weight;
      };
    };
    //Fill map with SVDTrueHit relations
    if (relDigitTrueHit) {
      typedef const RelationIndex<SVDRecoDigit, SVDTrueHit>::Element relTrueHit_type;
      for (relTrueHit_type& trueRel : relDigitTrueHit.getElementsFrom(storeDigits[strip.recoDigitIndex])) {
        //negative weights are from ignored particles, we don't like them and
        //thus ignore them :D
        if (trueRel.weight < 0) continue;
        truehit_relations[trueRel.indexTo] += trueRel.weight;
      };
    };

    digit_weights.push_back(make_pair(strip.recoDigitIndex, strip.charge));
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
