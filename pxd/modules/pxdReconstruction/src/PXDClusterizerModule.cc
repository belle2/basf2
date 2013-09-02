/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/PXDClusterizerModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>

#include <vxd/geometry/GeoCache.h>
#include <pxd/geometry/SensorInfo.h>

#include <generators/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDClusterizer);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDClusterizerModule::PXDClusterizerModule() :
  Module(), m_elNoise(200.0), m_cutSeed(5.0), m_cutAdjacent(3.0), m_cutCluster(
    8.0), m_sizeHeadTail(3)
{
  //Set module properties
  setDescription("Cluster PXDHits");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("ElectronicNoise", m_elNoise,
           "Noise added by the electronics, set in ENC", m_elNoise);
  addParam("NoiseSN", m_cutAdjacent,
           "SN for digits to be considered for clustering", m_cutAdjacent);
  addParam("SeedSN", m_cutSeed, "SN for digits to be considered as seed",
           m_cutSeed);
  addParam("ClusterSN", m_cutCluster, "Minimum SN for clusters", m_cutCluster);
  addParam("HeadTailSize", m_sizeHeadTail,
           "Minimum cluster size to switch to Analog head tail algorithm for cluster center",
           m_sizeHeadTail);

  addParam("Digits", m_storeDigitsName, "Digits collection name", string(""));
  addParam("Clusters", m_storeClustersName, "Cluster collection name",
           string(""));
  addParam("TrueHits", m_storeTrueHitsName, "TrueHit collection name",
           string(""));
  addParam("MCParticles", m_storeMCParticlesName, "MCParticles collection name",
           string(""));

  addParam("TanLorentz", m_tanLorentzAngle, "Tangent of the Lorentz angle",
           double(0.25));
  addParam("AssumeSorted", m_assumeSorted,
           "Assume Digits in Collection are orderd by sensor,row,column in ascending order",
           true);
}

void PXDClusterizerModule::initialize()
{
  //Register output collections
  StoreArray<PXDCluster>::registerPersistent(m_storeClustersName);
  RelationArray::registerPersistent<PXDCluster, MCParticle>(m_storeClustersName,
                                                            m_storeMCParticlesName);
  RelationArray::registerPersistent<PXDCluster, PXDDigit>(m_storeClustersName,
                                                          m_storeDigitsName);
  RelationArray::registerPersistent<PXDCluster, PXDTrueHit>(m_storeClustersName,
                                                            m_storeTrueHitsName);

  //Set names in case default was used. We need these for the RelationIndices.
  m_relDigitMCParticleName = DataStore::relationName(
                               DataStore::arrayName<PXDDigit>(m_storeDigitsName),
                               DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  m_relClusterMCParticleName = DataStore::relationName(
                                 DataStore::arrayName<PXDCluster>(m_storeClustersName),
                                 DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  m_relClusterDigitName = DataStore::relationName(
                            DataStore::arrayName<PXDCluster>(m_storeClustersName),
                            DataStore::arrayName<PXDDigit>(m_storeDigitsName));
  m_relDigitTrueHitName = DataStore::relationName(
                            DataStore::arrayName<PXDDigit>(m_storeDigitsName),
                            DataStore::arrayName<PXDTrueHit>(m_storeTrueHitsName));
  m_relClusterTrueHitName = DataStore::relationName(
                              DataStore::arrayName<PXDCluster>(m_storeClustersName),
                              DataStore::arrayName<PXDTrueHit>(m_storeTrueHitsName));

  B2INFO(
    "PXDClusterizer Parameters (in default system units, *=cannot be set directly):");
  B2INFO(" -->  ElectronicNoise:    " << m_elNoise);
  B2INFO(" -->  NoiseSN:            " << m_cutAdjacent);
  B2INFO(" -->  SeedSN:             " << m_cutSeed);
  B2INFO(" -->  ClusterSN:          " << m_cutCluster);
  B2INFO(
    " -->  MCParticles:        " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2INFO(
    " -->  Digits:             " << DataStore::arrayName<PXDDigit>(m_storeDigitsName));
  B2INFO(
    " -->  Clusters:           " << DataStore::arrayName<PXDCluster>(m_storeClustersName));
  B2INFO(
    " -->  TrueHits:           " << DataStore::arrayName<PXDTrueHit>(m_storeTrueHitsName));
  B2INFO(" -->  DigitMCRel:         " << m_relDigitMCParticleName);
  B2INFO(" -->  ClusterMCRel:       " << m_relClusterMCParticleName);
  B2INFO(" -->  ClusterDigitRel:    " << m_relClusterDigitName);
  B2INFO(" -->  DigitTrueRel:       " << m_relDigitTrueHitName);
  B2INFO(" -->  ClusterTrueRel:     " << m_relClusterTrueHitName);
  B2INFO(" -->  AssumeSorted:       " << (m_assumeSorted ? "true" : "false"));
  B2INFO(" -->  TanLorentz:         " << m_tanLorentzAngle);

  //This is still static noise for all pixels, should be done more sophisticated in the future
  m_noiseMap.setNoiseLevel(m_elNoise);
  m_cutElectrons = m_elNoise * m_cutAdjacent;
}

void PXDClusterizerModule::event()
{
  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<PXDTrueHit> storeTrueHits(m_storeTrueHitsName);
  const StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);

  if (!storeClusters.isValid())
    storeClusters.create();
  else
    storeClusters.getPtr()->Clear();

  RelationArray relClusterMCParticle(storeClusters, storeMCParticles,
                                     m_relClusterMCParticleName);
  if (relClusterMCParticle) relClusterMCParticle.clear();

  RelationArray relClusterDigit(storeClusters, storeDigits,
                                m_relClusterDigitName);
  if (relClusterDigit) relClusterDigit.clear();

  RelationArray relClusterTrueHit(storeClusters, storeTrueHits,
                                  m_relClusterTrueHitName);
  if (relClusterTrueHit) relClusterTrueHit.clear();

  int nPixels = storeDigits.getEntries();
  if (nPixels == 0)
    return;

  //Build lookup tables for relations
  RelationArray relDigitMCParticle(storeDigits, storeMCParticles, m_relDigitMCParticleName);
  RelationArray relDigitTrueHit(storeDigits, storeTrueHits, m_relDigitTrueHitName);
  createRelationLookup(relDigitMCParticle, m_mcRelation, storeDigits.getEntries());
  createRelationLookup(relDigitTrueHit, m_trueRelation, storeDigits.getEntries());

  m_cache.clear();

  if (!m_assumeSorted) {
    //If the pixels are in random order, we have to sort them first before we can cluster them
    std::map<VxdID, Sensor> sensors;
    //Fill sensors
    for (int i = 0; i < nPixels; i++) {
      const PXDDigit* const digit = storeDigits[i];
      Pixel px(digit, i);
      VxdID sensorID = digit->getSensorID();
      std::pair<Sensor::iterator, bool> it = sensors[sensorID].insert(px);
      if (!it.second)
        B2ERROR(
          "Pixel (" << px.getU() << "," << px.getV() << ") in sensor " << (string)sensorID << " is already set, ignoring second occurence");
    }

    //Now we loop over sensors and cluster each sensor in turn
    for (map<VxdID, Sensor>::iterator it = sensors.begin(); it != sensors.end();
         ++it) {
      m_noiseMap.setSensorID(it->first);
      for (const PXD::Pixel & px : it->second) {
        if (!m_noiseMap(px, m_cutAdjacent)) continue;
        m_cache.findCluster(px.getU(), px.getV()).add(px);
      }
      writeClusters(it->first);
      it->second.clear();
    }
  } else {
    //If we can assume that all pixels are already sorted we can skip the
    //reordering and directly cluster them once the sensorID changes, we write
    //out all existing clusters and continue
    VxdID sensorID;
    unsigned int lastU(0), lastV(0);
    for (int i = 0; i < nPixels; i++) {
      const PXDDigit* const digit = storeDigits[i];
      Pixel px(digit, i);
      //Load the correct noise map for the first pixel
      if (i == 0)
        m_noiseMap.setSensorID(digit->getSensorID());
      //Ignore digits with not enough signal
      if (!m_noiseMap(px, m_cutAdjacent))
        continue;

      //New sensor, write clusters
      if (sensorID != digit->getSensorID()) {
        writeClusters(sensorID);
        sensorID = digit->getSensorID();
        //Load the correct noise map for the new sensor
        m_noiseMap.setSensorID(sensorID);
      } else if (lastV > px.getV()
                 || (lastV == px.getV() && lastU > px.getU())) {
        //Check for sorting as precaution
        B2FATAL(
          "Pixels are not sorted correctly, please change the assumeSorted parameter " << "to false or fix the input to be ordered by v,u in ascending order");
      }
      lastU = px.getU();
      lastV = px.getV();
      // TODO: If we would like to cluster across dead channels we would need a
      // sorted list of dead pixels. Assuming we have such a list m_deadChannels
      // containing Pixel instances with all dead channels of this sensor and
      // m_currentDeadChannel is an iterator to that list (initialized to  we
      // would do begin(m_deadChannels) when the sensorID changes) we would do
      //
      // while(m_currentDeadChannel != end(m_deadChannels) && *m_currentDeadChannel < px){
      //   m_cache.findCluster(m_currentDeadChannel->getU(), m_currentDeadChannel->getV());
      //   m_currentDeadChannel++;
      // }
      //
      // This would have the effect of marking the pixel address as belonging
      // to a cluster but would not modify the clusters itself (except for
      // possible merging of clusters) so we would end up with clusters that
      // contain holes or are disconnected.

      // Find correct cluster and add pixel to cluster
      m_cache.findCluster(px.getU(), px.getV()).add(px);
    }
    writeClusters(sensorID);
  }
}

void PXDClusterizerModule::createRelationLookup(const RelationArray& relation, RelationLookup& lookup, size_t digits)
{
  lookup.clear();
  //If we don't have a relation we don't build a lookuptable
  if (!relation) return;
  //Resize to number of digits and set all values
  lookup.resize(digits);
  for (const RelationElement & element : relation) {
    lookup[element.getFromIndex()] = &element;
  }
}

void PXDClusterizerModule::fillRelationMap(const RelationLookup& lookup, std::map<unsigned int, float>& relation, unsigned int index)
{
  //If the lookuptable is not empty and the element is set
  if (!lookup.empty() && lookup[index]) {
    const RelationElement& element = *lookup[index];
    const unsigned int size = element.getSize();
    //Add all Relations to the map
    for (unsigned int i = 0; i < size; ++i) {
      relation[element.getToIndex(i)] += element.getWeight(i);
    }
  }
}

void PXDClusterizerModule::writeClusters(VxdID sensorID)
{
  if (m_cache.empty())
    return;

  //Get all datastore elements
  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  const StoreArray<PXDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);
  RelationArray relClusterMCParticle(storeClusters, storeMCParticles,
                                     m_relClusterMCParticleName);
  RelationArray relClusterDigit(storeClusters, storeDigits,
                                m_relClusterDigitName);
  RelationArray relClusterTrueHit(storeClusters, storeTrueHits,
                                  m_relClusterTrueHitName);

  //Get Geometry information
  const SensorInfo& info = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(
                                                             sensorID));

  map<unsigned int, float> mc_relations;
  map<unsigned int, float> truehit_relations;
  vector<pair<unsigned int, float> > digit_weights;

  for (ClusterCandidate & cls : m_cache) {
    //Check for noise cuts
    if (!(cls.size() > 0 && m_noiseMap(cls.getCharge(), m_cutCluster) && m_noiseMap(cls.getSeed(), m_cutSeed))) continue;

    double rho(0);
    ClusterProjection projU, projV;
    mc_relations.clear();
    truehit_relations.clear();
    digit_weights.clear();
    digit_weights.reserve(cls.size());

    const Pixel& seed = cls.getSeed();

    for (const PXD::Pixel & px : cls.pixels()) {
      //Add the Pixel information to the two projections
      projU.add(px.getU(), info.getUCellPosition(px.getU()), px.getCharge());
      projV.add(px.getV(), info.getVCellPosition(px.getV()), px.getCharge());

      //Obtain relations from MCParticles
      fillRelationMap(m_mcRelation, mc_relations, px.getIndex());
      //Obtain relations from PXDTrueHits
      fillRelationMap(m_trueRelation, truehit_relations, px.getIndex());
      //Save the weight of the digits for the Cluster->Digit relation
      digit_weights.emplace_back(px.getIndex(), px.getCharge());
    }
    projU.finalize();
    projV.finalize();

    const double pitchU = info.getUPitch();
    const double pitchV = info.getVPitch(projV.getPos());
    // Calculate shape correlation coefficient: only for non-trivial shapes
    if (projU.getSize() > 1 && projV.getSize() > 1) {
      // Add in-pixel position noise to smear the correlation
      double posUU = cls.getCharge() * pitchU * pitchU / 12.0;
      double posVV = cls.getCharge() * pitchV * pitchV / 12.0;
      double posUV(0);
      for (const Pixel & px : cls.pixels()) {
        const double du = info.getUCellPosition(px.getU()) - projU.getPos();
        const double dv = info.getVCellPosition(px.getV()) - projV.getPos();
        posUU += px.getCharge() * du * du;
        posVV += px.getCharge() * dv * dv;
        posUV += px.getCharge() * du * dv;
      }
      rho = posUV / sqrt(posUU * posVV);
    }

    //Calculate position and error with u as primary axis, fixed pitch size
    calculatePositionError(cls, projU, projV, pitchU, pitchU, pitchU);
    //Calculate position and error with v as primary axis, possibly different pitch sizes
    calculatePositionError(cls, projV, projU, info.getVPitch(projV.getMinPos()), pitchV, info.getVPitch(projV.getMaxPos()));

    //Lorentz shift correction FIXME: get from Bfield
    projU.setPos(projU.getPos() - 0.5 * info.getThickness() * m_tanLorentzAngle);

    //Store Cluster into Datastore ...
    int clsIndex = storeClusters.getEntries();
    storeClusters.appendNew(sensorID, projU.getPos(), projV.getPos(), projU.getError(), projV.getError(),
                            rho, cls.getCharge(), seed.getCharge(),
                            cls.size(), projU.getSize(), projV.getSize(), projU.getMinCell(), projV.getMinCell()
                           );

    //Create Relations to this Digit
    if (!mc_relations.empty()) relClusterMCParticle.add(clsIndex, mc_relations.begin(), mc_relations.end());
    if (!truehit_relations.empty()) relClusterTrueHit.add(clsIndex, truehit_relations.begin(), truehit_relations.end());
    relClusterDigit.add(clsIndex, digit_weights.begin(), digit_weights.end());
  }

  m_cache.clear();
}

void PXDClusterizerModule::calculatePositionError(const ClusterCandidate& cls, ClusterProjection& primary, const ClusterProjection& secondary, double minPitch, double centerPitch, double maxPitch)
{
  if (primary.getSize() >= m_sizeHeadTail) { //Analog head tail
    //Average charge in the central area
    const double centerCharge = primary.getCenterCharge() / (primary.getSize() - 2);
    const double minCharge = (primary.getMinCharge() < centerCharge) ? primary.getMinCharge() : centerCharge;
    const double maxCharge = (primary.getMaxCharge() < centerCharge) ? primary.getMaxCharge() : centerCharge;
    primary.setPos(0.5 * (primary.getMinPos() + primary.getMaxPos()
                          + (maxCharge * maxPitch - minCharge * minPitch) / centerCharge));
    const double snHead = centerCharge / m_cutElectrons / minPitch;
    const double snTail = centerCharge / m_cutElectrons / maxPitch;
    const double landauHead = minCharge / centerCharge * minPitch;
    const double landauTail = maxCharge / centerCharge * maxPitch;
    primary.setError(0.5 * sqrt(1.0 / snHead / snHead + 1.0 / snTail / snTail
                                + 0.5 * landauHead * landauHead + 0.5 * landauTail * landauTail));
  } else if (primary.getSize() <= 2) { // Add a phantom charge to second strip
    primary.setError(centerPitch * (secondary.getSize() + 2) * m_cutElectrons / (primary.getCharge() + (secondary.getSize() + 3) * m_cutElectrons));
  } else {
    const double sn = cls.getSeedCharge() / m_elNoise;
    primary.setError(2.0 * centerPitch / sn);
  }
}
