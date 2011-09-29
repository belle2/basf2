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
#include <boost/foreach.hpp>



#ifdef DUMP_CLUSTERS
#include <pxd/dataobjects/PXDTrueHit.h>
#include <framework/dataobjects/EventMetaData.h>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>
using namespace boost::iostreams;
#endif

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
    m_cutSeed(5.0), m_cutAdjacent(3.0), m_cutCluster(8.0), m_sizeHeadTail(3.0)
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
  addParam("Digits", m_digitColName,
           "Digits collection name", string(""));
  addParam("Clusters", m_clusterColName,
           "Cluster collection name", string(""));
  addParam("MCParticles", m_mcColName,
           "MCParticles collection name", string(""));

  addParam("DigitMCRel", m_digitMCRelName,
           "Relation between digits and MCParticles", string(""));
  addParam("ClusterMCRel", m_clusterMCRelName,
           "Relation between clusters and MCParticles", string(""));
  addParam("ClusterDigitRel", m_clusterDigitRelName,
           "Relation between clusters and Digits", string(""));

#ifdef DUMP_CLUSTERS
  addParam("DumpClusters", m_dumpFileName,
           "Dump digits and clusters into file", string(""));
#endif
  addParam("TanLorentz", m_tanLorentzAngle,
           "Tangent of the Lorentz angle", double(0.25));
  addParam("AssumeSorted", m_assumeSorted,
           "Assume Digits in Collection are orderd by sensor,u,v in ascending order", true);
}

void PXDClusteringModule::initialize()
{

  //Register collections
  StoreArray<MCParticle> storeMC(m_mcColName);
  StoreArray<PXDDigit>   storeDigits(m_digitColName);
  StoreArray<PXDCluster> storeClusters(m_clusterColName);
  RelationArray relDigitMC(storeDigits, storeMC, m_digitMCRelName);
  RelationArray relClusterMC(storeClusters, storeMC, m_clusterMCRelName);
  RelationArray relClusterDigit(storeClusters, storeDigits, m_clusterDigitRelName);

  NoiseMap::getInstance().setCuts(m_elNoise*m_cutAdjacent, m_elNoise*m_cutSeed, m_elNoise*m_cutCluster);
#ifdef DUMP_CLUSTERS
  if (m_dumpFileName != "") {
    m_dump.push(boost::iostreams::gzip_compressor());
    m_dump.push(boost::iostreams::file_sink(m_dumpFileName));
  }
#endif
}

void PXDClusteringModule::event()
{
  StoreArray<MCParticle> storeMC(m_mcColName);
  StoreArray<PXDDigit>   storeDigits(m_digitColName);
  StoreArray<PXDCluster> storeClusters(m_clusterColName);
  RelationArray relDigitMC(storeDigits, storeMC, m_digitMCRelName);
  RelationArray relClusterMC(storeClusters, storeMC, m_clusterMCRelName);
  RelationArray relClusterDigit(storeClusters, storeDigits, m_clusterDigitRelName);

  storeClusters->Clear();
  relClusterMC.clear();
  relClusterDigit.clear();


#ifdef DUMP_CLUSTERS
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  m_dump << "Event " << eventMetaDataPtr->getEvent() << endl;
  StoreArray<PXDTrueHit> storeTrueHits;
  int nTrueHits = storeTrueHits.getEntries();
  if (nTrueHits) {
    m_dump << "TrueHits" << endl;
    for (int i = 0; i < nTrueHits; ++i) {
      PXDTrueHit &hit = *storeTrueHits[i];
      const SensorInfo &info = dynamic_cast<const SensorInfo&> VXD::GeoCache::get(hit.getSensorID());
      double posU = (0.5 * info.getUSize() + hit.getU()) / info.getUPitch();
      double posV = (0.5 * info.getVSize() + hit.getV()) / info.getVPitch(hit.getV());
      m_dump << (string)hit.getSensorID() << " " << posU << " " << posV << endl;
    }
  }
  m_assumeSorted = false;
#endif

  if (!m_assumeSorted) {
    std::map<VxdID, Sensor> sensors;

    int nPixels = storeDigits.getEntries();
    //Fill sensors
#ifdef DUMP_CLUSTERS
    m_dump << "Digits" << endl;
#endif
    for (int i = 0; i < nPixels; i++) {
      Pixel px(storeDigits[i], i);
#ifdef DUMP_CLUSTERS
      m_dump << px.get()->getSensorID() << " " << px.getU() << " " << px.getV() << " " << px.getCharge() << endl;
#endif
      if (!NoiseMap::getInstance().adjacent(px.getU(), px.getV(), px.getCharge())) continue;
      VxdID sensorID = px.get()->getSensorID();
      std::pair<Sensor::iterator, bool> it = sensors[sensorID].insert(px);
      if (!it.second) B2ERROR("Pixel (" << px.getU() << "," << px.getV() << ") in sensor "
                                << (string)sensorID << " is already set, ignoring second occurence");
    }

    for (map<VxdID, Sensor>::iterator it = sensors.begin(); it != sensors.end(); it++) {
      findClusters(it->second);
      writeClusters(it->first);
      it->second.clear();
    }
  } else {
    VxdID sensorID;
    int nPixels = storeDigits.getEntries();
    for (int i = 0; i < nPixels; i++) {
      Pixel px(storeDigits[i], i);
      //Ignore digits with not enough signal
      if (!NoiseMap::getInstance().adjacent(px.getU(), px.getV(), px.getCharge())) continue;

      //New sensor, write clusters
      if (sensorID != px.get()->getSensorID()) {
        if (sensorID) writeClusters(sensorID);
        sensorID = px.get()->getSensorID();
      }
      //Find correct cluster and add pixel to cluster
      ClusterCandidate* prev = m_cache.findCluster(px.getU(), px.getV());
      if (!prev) {
        m_clusters.push_back(ClusterCandidate());
        prev = &m_clusters.back();
      }
      prev->add(px);
      m_cache.setLast(px.getU(), px.getV(), prev);
    }
    writeClusters(sensorID);
  }
}

void PXDClusteringModule::findClusters(const Sensor &sensor)
{
  BOOST_FOREACH(const PXD::Pixel &px, sensor) {
    ClusterCandidate* prev = m_cache.findCluster(px.getU(), px.getV());
    if (!prev) {
      m_clusters.push_back(ClusterCandidate());
      prev = &m_clusters.back();
    }
    prev->add(px);
    m_cache.setLast(px.getU(), px.getV(), prev);
  }
}

void PXDClusteringModule::writeClusters(VxdID sensorID)
{
  //Get all datastore elements
  StoreArray<MCParticle> storeMC(m_mcColName);
  StoreArray<PXDDigit>   storeDigits(m_digitColName);
  StoreArray<PXDCluster> storeClusters(m_clusterColName);
  RelationArray relClusterMC(storeClusters, storeMC, m_clusterMCRelName);
  RelationArray relClusterDigit(storeClusters, storeDigits, m_clusterDigitRelName);
  RelationIndex<PXDDigit, MCParticle> digitRel(storeDigits, storeMC, m_digitMCRelName);

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
    vector<pair<int, float> > digit_weights;
    digit_weights.reserve(cls.size());
    BOOST_FOREACH(const PXD::Pixel &px, cls.pixels()) {
      SET_CLUSTER_LIMIT(minU, > , px.getU(), px.getCharge());
      SET_CLUSTER_LIMIT(maxU, < , px.getU(), px.getCharge());
      SET_CLUSTER_LIMIT(minV, > , px.getV(), px.getCharge());
      SET_CLUSTER_LIMIT(maxV, < , px.getV(), px.getCharge());

      posU += px.getCharge() * info.getUCellPosition(px.getU());
      posV += px.getCharge() * info.getVCellPosition(px.getV());

      typedef const RelationIndex<PXDDigit, MCParticle>::Element rel_type;

      //Fill map with MCParticle relations
      BOOST_FOREACH(rel_type &mcRel, digitRel.getFrom(px.get())) {
        mc_relations[mcRel.indexTo] += mcRel.weight;
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
    relClusterMC.add(clsIndex, mc_relations.begin(), mc_relations.end());
    relClusterDigit.add(clsIndex, digit_weights.begin(), digit_weights.end());


#ifdef DUMP_CLUSTERS
    posU = (0.5 * info.getUSize() + posU) / info.getUPitch();
    posV = (0.5 * info.getVSize() + posV) / info.getVPitch(posV);

    m_dump << "Cluster " << uid << " "
    << posU << " " << posV << " "
    << sizeU << " " << sizeV << " "
    << cls.charge() << " " << seed.charge() << " " << endl;
#endif
  }

  m_clusters.clear();
  m_cache.clear();
}
