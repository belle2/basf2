/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/PXDClusteringModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>

#include <pxd/vxd/GeoCache.h>
#include <pxd/geometry/SensorInfo.h>

#include <generators/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDClustering)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDClusteringModule::PXDClusteringModule() : Module(), m_elNoise(200.0),
    m_cutSeed(5.0), m_cutAdjacent(3.0), m_cutCluster(8.0), m_sizeHeadTail(3)
{
  //Set module properties
  setDescription("Cluster PXDHits");
  addParam("ElectronicNoise", m_elNoise,
           "Noise added by the electronics, set in ENC", m_elNoise);
  addParam("NoiseSN", m_cutAdjacent,
           "SN for digits to be considered for clustering", m_cutAdjacent);
  addParam("SeedSN", m_cutSeed,
           "SN for digits to be considered as seed", m_cutSeed);
  addParam("ClusterSN", m_cutCluster,
           "Minimum SN for clusters", m_cutCluster);
  addParam("HeadTailSize", m_sizeHeadTail,
           "Minimum cluster size to switch to Analog head tail algorithm for cluster center", m_sizeHeadTail);

  addParam("Digits", m_storeDigitsName,
           "Digits collection name", string(""));
  addParam("Clusters", m_storeClustersName,
           "Cluster collection name", string(""));
  addParam("TrueHits", m_storeTrueHitsName,
           "TrueHit collection name", string(""));
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticles collection name", string(""));
  addParam("DigitMCRel", m_relDigitMCParticleName,
           "Relation between digits and MCParticles", string(""));
  addParam("ClusterMCRel", m_relClusterMCParticleName,
           "Relation between clusters and MCParticles", string(""));
  addParam("ClusterDigitRel", m_relClusterDigitName,
           "Relation between clusters and Digits", string(""));
  addParam("DigitTrueRel", m_relDigitTrueHitName,
           "Relation between Digits and TrueHits", string(""));
  addParam("ClusterTrueRel", m_relDigitTrueHitName,
           "Relation between Clusters and TrueHits", string(""));

  addParam("TanLorentz", m_tanLorentzAngle,
           "Tangent of the Lorentz angle", double(0.25));
  addParam("AssumeSorted", m_assumeSorted,
           "Assume Digits in Collection are orderd by sensor,u,v in ascending order", true);
}

void PXDClusteringModule::initialize()
{
  //Register collections
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<PXDDigit>   storeDigits(m_storeDigitsName);
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);
  StoreArray<PXDTrueHit> storeTrueHits(m_storeTrueHitsName);
  RelationArray relDigitMCParticle(storeDigits, storeMCParticles, m_relDigitMCParticleName);
  RelationArray relClusterMCParticle(storeClusters, storeMCParticles, m_relClusterMCParticleName);
  RelationArray relClusterDigit(storeClusters, storeDigits, m_relClusterDigitName);
  RelationArray relDigitTrueHit(storeDigits, storeTrueHits, m_relDigitTrueHitName);
  RelationArray relClusterTrueHit(storeClusters, storeTrueHits, m_relClusterTrueHitName);

  //Set names in case default was used
  m_relDigitMCParticleName   = relDigitMCParticle.getName();
  m_relClusterMCParticleName = relClusterMCParticle.getName();
  m_relClusterDigitName      = relClusterDigit.getName();
  m_relDigitTrueHitName      = relDigitTrueHit.getName();
  m_relClusterTrueHitName    = relClusterTrueHit.getName();

  B2INFO("PXDClustering Parameters (in default system units, *=cannot be set directly):");
  B2INFO(" -->  ElectronicNoise:    " << m_elNoise);
  B2INFO(" -->  NoiseSN:            " << m_cutAdjacent);
  B2INFO(" -->  SeedSN:             " << m_cutSeed);
  B2INFO(" -->  ClusterSN:          " << m_cutCluster);
  B2INFO(" -->  MCParticles:        " << storeMCParticles.getName());
  B2INFO(" -->  Digits:             " << storeDigits.getName());
  B2INFO(" -->  Clusters:           " << storeClusters.getName());
  B2INFO(" -->  TrueHits:           " << storeTrueHits.getName());
  B2INFO(" -->  DigitMCRel:         " << m_relDigitMCParticleName);
  B2INFO(" -->  ClusterMCRel:       " << m_relClusterMCParticleName);
  B2INFO(" -->  ClusterDigitRel:    " << m_relClusterDigitName);
  B2INFO(" -->  DigitTrueRel:       " << m_relDigitTrueHitName);
  B2INFO(" -->  ClusterTrueRel:     " << m_relClusterTrueHitName);
  B2INFO(" -->  AssumeSorted:       " << (m_assumeSorted ? "true" : "false"));
  B2INFO(" -->  TanLorentz:         " << m_tanLorentzAngle);

  NoiseMap::getInstance().setCuts(m_elNoise*m_cutAdjacent, m_elNoise*m_cutSeed, m_elNoise*m_cutCluster);
}

void PXDClusteringModule::event()
{
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<PXDDigit>   storeDigits(m_storeDigitsName);
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);
  StoreArray<PXDTrueHit> storeTrueHits(m_storeTrueHitsName);
  RelationArray relDigitMCParticle(storeDigits, storeMCParticles, m_relDigitMCParticleName);
  RelationArray relClusterMCParticle(storeClusters, storeMCParticles, m_relClusterMCParticleName);
  RelationArray relClusterDigit(storeClusters, storeDigits, m_relClusterDigitName);
  RelationArray relDigitTrueHit(storeDigits, storeTrueHits, m_relDigitTrueHitName);
  RelationArray relClusterTrueHit(storeClusters, storeTrueHits, m_relClusterTrueHitName);

  storeClusters->Clear();
  relClusterMCParticle.clear();
  relClusterDigit.clear();
  relClusterTrueHit.clear();
  int nPixels = storeDigits.getEntries();
  if (nPixels == 0) return;

  m_clusters.clear();
  m_cache.clear();

  if (!m_assumeSorted) {
    //If the pixels are in random order, we have to sort them first before we can cluster them
    std::map<VxdID, Sensor> sensors;
    //Fill sensors
    for (int i = 0; i < nPixels; i++) {
      Pixel px(storeDigits[i], i);
      if (!NoiseMap::getInstance().adjacent(px.getU(), px.getV(), px.getCharge())) continue;
      VxdID sensorID = px.get()->getSensorID();
      std::pair<Sensor::iterator, bool> it = sensors[sensorID].insert(px);
      if (!it.second) B2ERROR("Pixel (" << px.getU() << "," << px.getV() << ") in sensor "
                                << (string)sensorID << " is already set, ignoring second occurence");
    }

    //Now we loop over sensors and cluster each sensor in turn
    for (map<VxdID, Sensor>::iterator it = sensors.begin(); it != sensors.end(); it++) {
      BOOST_FOREACH(const PXD::Pixel &px, it->second) {
        findCluster(px);
      }
      writeClusters(it->first);
      it->second.clear();
    }
  } else {
    //If we can assume that all pixels are already sorted we can skip the
    //reordering and directly cluster them once the sensorID changes, we write
    //out all existing clusters and continue
    VxdID sensorID;
    for (int i = 0; i < nPixels; i++) {
      Pixel px(storeDigits[i], i);
      //Ignore digits with not enough signal
      if (!NoiseMap::getInstance().adjacent(px.getU(), px.getV(), px.getCharge())) continue;

      //New sensor, write clusters
      if (sensorID != px.get()->getSensorID()) {
        writeClusters(sensorID);
        sensorID = px.get()->getSensorID();
      }
      //Find correct cluster and add pixel to cluster
      findCluster(px);
    }
    writeClusters(sensorID);
  }
}

inline void PXDClusteringModule::findCluster(const Pixel &px)
{
  ClusterCandidate* prev = m_cache.findCluster(px.getU(), px.getV());
  if (!prev) {
    m_clusters.push_back(ClusterCandidate());
    prev = &m_clusters.back();
  }
  prev->add(px);
  m_cache.setLast(px.getU(), px.getV(), prev);
}

void PXDClusteringModule::writeClusters(VxdID sensorID)
{
  if (m_clusters.empty()) return;

  //Get all datastore elements
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<PXDDigit>   storeDigits(m_storeDigitsName);
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);
  StoreArray<PXDTrueHit> storeTrueHits(m_storeTrueHitsName);
  RelationArray relClusterMCParticle(storeClusters, storeMCParticles, m_relClusterMCParticleName);
  RelationArray relClusterDigit(storeClusters, storeDigits, m_relClusterDigitName);
  RelationArray relClusterTrueHit(storeClusters, storeTrueHits, m_relClusterTrueHitName);
  RelationIndex<PXDDigit, MCParticle> relDigitMCParticle(storeDigits, storeMCParticles, m_relDigitMCParticleName);
  RelationIndex<PXDDigit, PXDTrueHit> relDigitTrueHit(storeDigits, storeTrueHits, m_relDigitTrueHitName);

  //Get Geometry information
  const SensorInfo &info = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));

  BOOST_FOREACH(ClusterCandidate &cls, m_clusters) {
    //Check for noise cut
    if (!cls.valid()) continue;

    const Pixel &seed = cls.getSeed();
    unsigned int maxU(0), minU(info.getUCells() - 1) , maxV(0), minV(info.getVCells() - 1);
    double minUCharge(0), maxUCharge(0), minVCharge(0), maxVCharge(0);
    double posU(0), posV(0);

// For head tail, we need the min/max pixels in both directions. To avoid copy
// pasting, we define a macro for checking if the ID is at the border
#define SET_CLUSTER_LIMIT(var,op,cell,charge)\
  if(var op cell){\
    var = cell;\
    var##Charge = charge;\
  }else if(var == cell){\
    var##Charge += charge;\
  }

    map<int, float> mc_relations;
    map<int, float> truehit_relations;
    vector<pair<int, float> > digit_weights;
    digit_weights.reserve(cls.size());
    BOOST_FOREACH(const PXD::Pixel &px, cls.pixels()) {
      SET_CLUSTER_LIMIT(minU, > , px.getU(), px.getCharge());
      SET_CLUSTER_LIMIT(maxU, < , px.getU(), px.getCharge());
      SET_CLUSTER_LIMIT(minV, > , px.getV(), px.getCharge());
      SET_CLUSTER_LIMIT(maxV, < , px.getV(), px.getCharge());

      posU += px.getCharge() * info.getUCellPosition(px.getU());
      posV += px.getCharge() * info.getVCellPosition(px.getV());

      typedef const RelationIndex<PXDDigit, MCParticle>::Element relMC_type;
      typedef const RelationIndex<PXDDigit, PXDTrueHit>::Element relTrueHit_type;

      //Fill map with MCParticle relations
      BOOST_FOREACH(relMC_type &mcRel, relDigitMCParticle.getFrom(px.get())) {
        mc_relations[mcRel.indexTo] += mcRel.weight;
      };
      //Fill map with PXDTrueHit relations
      BOOST_FOREACH(relTrueHit_type &trueRel, relDigitTrueHit.getFrom(px.get())) {
        truehit_relations[trueRel.indexTo] += trueRel.weight;
      };
      //Add digit to the Cluster->Digit relation list
      digit_weights.push_back(make_pair(px.getIndex(), px.getCharge()));
    }
    posU /= cls.getCharge();
    posV /= cls.getCharge();
    const int sizeU = maxU - minU + 1;
    const int sizeV = maxV - minV + 1;
    if (sizeU >= m_sizeHeadTail) {
      //Average charge in the central area
      double centreCharge = (cls.getCharge() - minUCharge - maxUCharge) / (sizeU - 2);
      double minUPos = info.getUCellPosition(minU);
      double maxUPos = info.getUCellPosition(maxU);
      posU = 0.5 * (minUPos + maxUPos + (maxUCharge - minUCharge) / centreCharge * info.getUPitch());
    }
    if (sizeV >= m_sizeHeadTail) {
      //Average charge in the central area
      double centreCharge = (cls.getCharge() - minVCharge - maxVCharge) / (sizeV - 2);
      double minVPos = info.getVCellPosition(minV);
      double maxVPos = info.getVCellPosition(maxV);
      posV = 0.5 * (minVPos + maxVPos + (maxVCharge * info.getVPitch(maxVPos) -
                                         minVCharge * info.getVPitch(minVPos)) / centreCharge);
    }

    //Lorentz shift correction FIXME: get from Bfield
    posU -= 0.5 * info.getThickness() * m_tanLorentzAngle;

    //Store Cluster into Datastore ...
    int clsIndex = storeClusters->GetLast() + 1;
    new(storeClusters->AddrAt(clsIndex)) PXDCluster(
      seed.get()->getSensorID(), posU, posV,
      cls.getCharge(), seed.getCharge(),
      cls.size(), sizeU, sizeV
    );

    //Create Relations to this Digit
    relClusterMCParticle.add(clsIndex, mc_relations.begin(), mc_relations.end());
    relClusterTrueHit.add(clsIndex, truehit_relations.begin(), truehit_relations.end());
    relClusterDigit.add(clsIndex, digit_weights.begin(), digit_weights.end());
  }

  m_clusters.clear();
  m_cache.clear();
}
