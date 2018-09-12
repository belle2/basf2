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
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>

#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>

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
  setDescription("Clusterize SVDRecoDigits fitted by the Center of Gravity estimator");
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
  m_storeClusters.registerInDataStore(m_storeClustersName, DataStore::c_ErrorIfAlreadyRegistered);
  m_storeDigits.isRequired(m_storeRecoDigitsName);
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
  m_storeRecoDigitsName = m_storeDigits.getName();
  m_storeTrueHitsName = m_storeTrueHits.getName();
  m_storeMCParticlesName = m_storeMCParticles.getName();

  m_relClusterRecoDigitName = relClusterDigits.getName();
  m_relClusterTrueHitName = relClusterTrueHits.getName();
  m_relClusterMCParticleName = relClusterMCParticles.getName();
  m_relRecoDigitTrueHitName = relDigitTrueHits.getName();
  m_relRecoDigitMCParticleName = relDigitMCParticles.getName();

  // Report:
  B2DEBUG(1, "SVDSimpleClusterizer Parameters (in default system unit, *=cannot be set directly):");

  B2DEBUG(1, " 1. COLLECTIONS:");
  B2DEBUG(1, " -->  MCParticles:        " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2DEBUG(1, " -->  SVDRecoDigits:      " << DataStore::arrayName<SVDRecoDigit>(m_storeRecoDigitsName));
  B2DEBUG(1, " -->  SVDClusters:        " << DataStore::arrayName<SVDCluster>(m_storeClustersName));
  B2DEBUG(1, " -->  SVDTrueHits:        " << DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName));
  B2DEBUG(1, " -->  DigitMCRel:         " << m_relRecoDigitMCParticleName);
  B2DEBUG(1, " -->  ClusterMCRel:       " << m_relClusterMCParticleName);
  B2DEBUG(1, " -->  ClusterDigitRel:    " << m_relClusterRecoDigitName);
  B2DEBUG(1, " -->  DigitTrueRel:       " << m_relRecoDigitTrueHitName);
  B2DEBUG(1, " -->  ClusterTrueRel:     " << m_relClusterTrueHitName);
  B2DEBUG(1, " 2. CLUSTERING:");
  B2DEBUG(1, " -->  Neighbour cut:      " << m_cutAdjacent);
  B2DEBUG(1, " -->  Seed cut:           " << m_cutSeed);
  B2DEBUG(1, " -->  Size HeadTail:      " << m_sizeHeadTail);
}



void SVDSimpleClusterizerModule::event()
{
  int nDigits = m_storeDigits.getEntries();
  if (nDigits == 0)
    return;

  m_storeClusters.clear();

  RelationArray relClusterMCParticle(m_storeClusters, m_storeMCParticles,
                                     m_relClusterMCParticleName);
  if (relClusterMCParticle) relClusterMCParticle.clear();

  RelationArray relClusterDigit(m_storeClusters, m_storeDigits,
                                m_relClusterRecoDigitName);
  if (relClusterDigit) relClusterDigit.clear();

  RelationArray relClusterTrueHit(m_storeClusters, m_storeTrueHits,
                                  m_relClusterTrueHitName);
  if (relClusterTrueHit) relClusterTrueHit.clear();


  //create a dummy cluster just to start
  SimpleClusterCandidate clusterCandidate(m_storeDigits[0]->getSensorID(), m_storeDigits[0]->isUStrip(),
                                          m_sizeHeadTail, m_cutSeed, m_cutAdjacent);

  //loop over the SVDRecoDigits
  int i = 0;
  while (i < nDigits) {

    //retrieve the VxdID, sensor and cellID of the current RecoDigit
    VxdID thisSensorID = m_storeDigits[i]->getSensorID();
    bool thisSide = m_storeDigits[i]->isUStrip();
    int thisCellID = m_storeDigits[i]->getCellID();

    //Ignore digits with insufficient signal
    float thisNoise = m_NoiseCal.getNoiseInElectrons(thisSensorID, thisSide, thisCellID);
    float thisCharge = m_storeDigits[i]->getCharge();
    B2DEBUG(10, "Noise = " << thisNoise << " e-, Charge = " << thisCharge);

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
    aStrip.time = m_storeDigits[i]->getTime();

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

  B2DEBUG(1, "Number of clusters: " << m_storeClusters.getEntries());
}


void SVDSimpleClusterizerModule::writeClusters(SimpleClusterCandidate cluster)
{

  RelationArray relClusterDigit(m_storeClusters, m_storeDigits, m_relClusterRecoDigitName);

  RelationArray relClusterMCParticle(m_storeClusters, m_storeMCParticles, m_relClusterMCParticleName);
  RelationArray relClusterTrueHit(m_storeClusters, m_storeTrueHits, m_relClusterTrueHitName);

  RelationIndex<SVDRecoDigit, MCParticle> relDigitMCParticle(m_storeDigits, m_storeMCParticles, m_relRecoDigitMCParticleName);
  RelationIndex<SVDRecoDigit, SVDTrueHit> relDigitTrueHit(m_storeDigits, m_storeTrueHits, m_relRecoDigitTrueHitName);


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
  m_storeClusters.appendNew(SVDCluster(
                              sensorID, isU, position, positionError, time, timeError, charge, seedCharge, size, SNR, -1
                            ));

  //register relation between RecoDigit and Cluster
  int clsIndex = m_storeClusters.getEntries() - 1;

  map<int, float> mc_relations;
  map<int, float> truehit_relations;

  vector<pair<int, float> > digit_weights;
  digit_weights.reserve(size);

  std::vector<stripInCluster> strips = cluster.getStripsInCluster();

  for (auto strip : strips) {

    //Fill map with MCParticle relations
    if (relDigitMCParticle) {
      typedef const RelationIndex<SVDRecoDigit, MCParticle>::Element relMC_type;
      for (relMC_type& mcRel : relDigitMCParticle.getElementsFrom(m_storeDigits[strip.recoDigitIndex])) {
        //negative weights are from ignored particles, we don't like them and
        //thus ignore them :D
        if (mcRel.weight < 0) continue;
        mc_relations[mcRel.indexTo] += mcRel.weight;
      };
    };
    //Fill map with SVDTrueHit relations
    if (relDigitTrueHit) {
      typedef const RelationIndex<SVDRecoDigit, SVDTrueHit>::Element relTrueHit_type;
      for (relTrueHit_type& trueRel : relDigitTrueHit.getElementsFrom(m_storeDigits[strip.recoDigitIndex])) {
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
