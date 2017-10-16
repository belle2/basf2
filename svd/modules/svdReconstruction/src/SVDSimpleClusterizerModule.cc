/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Peter Kvasnicka                           *
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

#include <boost/math/distributions/chi_squared.hpp>

#include <algorithm>
#include <numeric>
#include <functional>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

using boost::math::chi_squared;
using boost::math::quantile;
using boost::math::complement;

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
  setDescription("Clusterize SVDRecoDigits fitted by the basic time estimator");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("RecoDigits", m_storeRecoDigitsName,
           "RecoDigits collection name", string(""));
  addParam("Clusters", m_storeClustersName,
           "Cluster collection name", string(""));
  addParam("TrueHits", m_storeTrueHitsName,
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
  storeDigits.required();
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
  B2INFO(" -->  Digits:             " << DataStore::arrayName<SVDRecoDigit>(m_storeRecoDigitsName));
  B2INFO(" -->  Clusters:           " << DataStore::arrayName<SVDCluster>(m_storeClustersName));
  B2INFO(" -->  TrueHits:           " << DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName));
  B2INFO(" -->  DigitMCRel:         " << m_relRecoDigitMCParticleName);
  B2INFO(" -->  ClusterMCRel:       " << m_relClusterMCParticleName);
  B2INFO(" -->  ClusterDigitRel:    " << m_relClusterRecoDigitName);
  B2INFO(" -->  DigitTrueRel:       " << m_relRecoDigitTrueHitName);
  B2INFO(" -->  ClusterTrueRel:     " << m_relClusterTrueHitName);
  B2INFO(" 4. CLUSTERING:");
  B2INFO(" -->  Neighbour cut:      " << m_cutAdjacent);
  B2INFO(" -->  Seed cut:           " << m_cutSeed);

}



void SVDSimpleClusterizerModule::event()
{
  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  const StoreArray<SVDRecoDigit>   storeDigits(m_storeRecoDigitsName);
  StoreArray<SVDCluster> storeClusters(m_storeClustersName);

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

  int nDigits = storeDigits.getEntries();
  if (nDigits == 0)
    return;


  int i = 0;
  VxdID prevSensorID = storeDigits[0]->getSensorID();
  bool prevSide = storeDigits[0]->isUStrip();
  int prevCellID = storeDigits[0]->getCellID();

  SimpleClusterCandidate* clusterCandidate = new SimpleClusterCandidate(prevSensorID, prevSide, m_sizeHeadTail, m_cutSeed,
      m_cutAdjacent);

  bool skip = false;
  while (i < nDigits) {

    VxdID thisSensorID = storeDigits[i]->getSensorID();
    bool thisSide = storeDigits[i]->isUStrip();
    int thisCellID = storeDigits[i]->getCellID();

    //    B2INFO("looking for a strip to ADD, index "<<i<<", sensor, side, cellID = "<<thisSensorID<<","<<thisSide<<", "<<thisCellID);
    //    B2INFO("PREVIOUS: sensor, side, cellID = "<<prevSensorID<<","<<prevSide<<", "<<prevCellID);

    if (! skip)
      if ((thisSide != prevSide) || (thisSensorID != prevSensorID) ||
          (thisCellID != prevCellID + 1)) {
        //  B2INFO("side: "<<thisSide<<" VS "<< prevSide);
        //  B2INFO("sensor: "<<thisSensorID<<" VS "<< prevSensorID);
        //  B2INFO("cell: "<<thisCellID<<" VS "<< prevCellID);
        if (clusterCandidate->size() > 0) {
          clusterCandidate->finalizeCluster();
          if (clusterCandidate->isGoodCluster())
            writeClusters(*clusterCandidate);
        }

        clusterCandidate->clear();
        delete clusterCandidate;

        clusterCandidate = new SimpleClusterCandidate(thisSensorID, thisSide, m_sizeHeadTail, m_cutSeed, m_cutAdjacent);

        prevSide = thisSide;
        prevSensorID = thisSensorID;
        prevCellID = thisCellID;
        skip = true;

        continue;
      }

    skip = false;

    //     calibration
    //Ignore digits with insufficient signal
    //    float ADCnoise = m_NoiseCal->getNoise(thisSensorID, thisSide, thisCellID);
    //    float thisNoise = m_PulseShapeCal->getChargeFromADC(thisSensorID, thisSide, thisCellID, ADCnoise);

    float thisNoise = 50; //to be removed
    float thisCharge = storeDigits[i]->getCharge();

    //    B2INFO( "signal over noise = " << thisCharge / thisNoise);
    if ((float)thisCharge / thisNoise < m_cutAdjacent) {
      i++;

      prevSide = thisSide;
      prevSensorID = thisSensorID;
      prevCellID = thisCellID;

      continue;
    }

    stripInCluster aStrip;
    aStrip.recoDigitIndex = i;
    aStrip.charge = thisCharge;
    aStrip.cellID = thisCellID;
    aStrip.noise = thisNoise;
    aStrip.time = storeDigits[i]->getTime();

    //    B2INFO("addind a strip, index "<<i<<", sensor, side, cellID = "<<thisSensorID<<","<<thisSide<<", "<<thisCellID);
    clusterCandidate->add(aStrip);


    prevSide = thisSide;
    prevSensorID = thisSensorID;
    prevCellID = thisCellID;

    i++;

  }

  if (clusterCandidate->size() > 0) {
    clusterCandidate->finalizeCluster();
    if (clusterCandidate->isGoodCluster())
      writeClusters(*clusterCandidate);
  }

  B2DEBUG(1, "Number of clusters: " << storeClusters.getEntries());
}


void SVDSimpleClusterizerModule::writeClusters(SimpleClusterCandidate cluster)
{

  StoreArray<SVDCluster> storeClusters(m_storeClustersName);

  VxdID sensorID = cluster.getSensorID();
  bool isU = cluster.isUSide();
  float position = cluster.getPosition();
  float positionError = cluster.getPositionError();
  float time = cluster.getTime();
  float timeError = cluster.getTimeError();
  float seedCharge = cluster.getSeedCharge();
  float charge = cluster.getCharge();
  float size = cluster.size();
  float SNR = cluster.getSNR();

  //  Store Cluster into Datastore
  storeClusters.appendNew(SVDCluster(
                            sensorID, isU, position, positionError, time, timeError, seedCharge, charge, size, SNR
                          ));


}
