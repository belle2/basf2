/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/vxd/modules/telReconstruction/TelClusterizerModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>

#include <vxd/geometry/GeoCache.h>
#include <testbeam/vxd/geometry/SensorInfo.h>

#include <mdst/dataobjects/MCParticle.h>
#include <testbeam/vxd/dataobjects/TelDigit.h>

#ifdef MAKE_TELCLUSTERS
#include <testbeam/vxd/dataobjects/TelCluster.h>
#else
#include <pxd/dataobjects/PXDCluster.h>
#endif

#include <testbeam/vxd/dataobjects/TelTrueHit.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::TEL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TelClusterizer);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TelClusterizerModule::TelClusterizerModule() :
  Module(), m_sizeHeadTail(3), m_clusterCacheSize(0)
{
  //Set module properties
  setDescription("Cluster TelDigits");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("ClusterCacheSize", m_clusterCacheSize,
           "Maximum desired number of sensor rows", 1200);
  addParam("Digits", m_storeDigitsName, "Digits collection name", string(""));
  addParam("Clusters", m_storeClustersName, "Cluster collection name",
           string(""));
  // CAUTION: Be always explicit about collection name when storing PXDClusters!
  addParam("TrueHits", m_storeTrueHitsName, "TrueHit collection name",
           string(""));
  addParam("MCParticles", m_storeMCParticlesName, "MCParticles collection name",
           string(""));

  addParam("TanLorentz", m_tanLorentzAngle, "Tangent of the Lorentz angle",
           double(0.25));
  addParam("cutElectrons", m_cutElectrons, "ENC equivalent of threshold cut", double(1200));
}
void TelClusterizerModule::initialize()
{
  //Register collections
#ifdef MAKE_TELCLUSTERS
  StoreArray<TelCluster> storeClusters(m_storeClustersName);
#else
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);
#endif
  StoreArray<TelDigit>   storeDigits(m_storeDigitsName);
  StoreArray<TelTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);

  storeClusters.registerInDataStore();
  storeDigits.isRequired();
  storeTrueHits.isOptional();
  storeMCParticles.isOptional();

  RelationArray relClusterDigits(storeClusters, storeDigits);
  RelationArray relClusterTrueHits(storeClusters, storeTrueHits);
  RelationArray relClusterMCParticles(storeClusters, storeMCParticles);
  RelationArray relDigitMCParticles(storeDigits, storeMCParticles);
  RelationArray relDigitTrueHits(storeDigits, storeTrueHits);

  relClusterDigits.registerInDataStore();
  //Relations to Simulation objects are only needed if the parent one already
  //exists
  if (relDigitTrueHits.isOptional()) {
    relClusterTrueHits.registerInDataStore();
  }
  if (relDigitMCParticles.isOptional()) {
    relClusterMCParticles.registerInDataStore();
  }

  //Now save all names to speed creation later, mainly if they had default names
  m_storeClustersName = storeClusters.getName();
  m_storeDigitsName = storeDigits.getName();
  m_storeTrueHitsName = storeTrueHits.getName();
  m_storeMCParticlesName = storeMCParticles.getName();

  m_relClusterDigitName = relClusterDigits.getName();
  m_relClusterTrueHitName = relClusterTrueHits.getName();
  m_relClusterMCParticleName = relClusterMCParticles.getName();
  m_relDigitTrueHitName = relDigitTrueHits.getName();
  m_relDigitMCParticleName = relDigitMCParticles.getName();

  B2INFO(
    "TelClusterizer Parameters (in default system units, *=cannot be set directly):");
  B2INFO(
    " -->  MCParticles:        " << m_storeMCParticlesName);
  B2INFO(
    " -->  Digits:             " << m_storeDigitsName);
  B2INFO(
    " -->  Clusters:           " << m_storeClustersName);
  B2INFO(
    " -->  TrueHits:           " << m_storeTrueHitsName);
  B2INFO(" -->  DigitMCRel:         " << m_relDigitMCParticleName);
  B2INFO(" -->  ClusterMCRel:       " << m_relClusterMCParticleName);
  B2INFO(" -->  ClusterDigitRel:    " << m_relClusterDigitName);
  B2INFO(" -->  DigitTrueRel:       " << m_relDigitTrueHitName);
  B2INFO(" -->  ClusterTrueRel:     " << m_relClusterTrueHitName);
  B2INFO(" -->  TanLorentz:         " << m_tanLorentzAngle);

  if (m_clusterCacheSize > 0)
    m_cache = std::unique_ptr<ClusterCache>(new ClusterCache(m_clusterCacheSize));
  else
    m_cache = std::unique_ptr<ClusterCache>(new ClusterCache());
}

void TelClusterizerModule::event()
{
  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<TelTrueHit> storeTrueHits(m_storeTrueHitsName);
  const StoreArray<TelDigit> storeDigits(m_storeDigitsName);
#ifdef MAKE_TELCLUSTERS
  StoreArray<TelCluster> storeClusters(m_storeClustersName);
#else
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);
#endif

  storeClusters.clear();

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

  m_cache->clear();

  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //We require all pixels are already sorted and directly cluster them. Once
  //the sensorID changes, we write out all existing clusters and continue.
  VxdID sensorID(0);
  //To check sorting
  Pixel lastPixel;
  for (int i = 0; i < nPixels; i++) {
    const TelDigit* const digit = storeDigits[i];
    // If malformed data, drop the digit.
    if (!geo.validSensorID(digit->getSensorID())) {
      B2WARNING("Malformed TelDigit, sensorID " << digit->getSensorID() << ". Dropping.");
      continue;
    }
    Pixel px(digit, i);
    //New sensor, write clusters
    if (sensorID != digit->getSensorID()) {
      writeClusters(sensorID);
      sensorID = digit->getSensorID();
      //Load the correct noise map for the new sensor
      m_noiseMap.setSensorID(sensorID);
    } else if (px <= lastPixel) {
      //Check for sorting as precaution; but is sometimes happens to fail, so don't panic.
      B2WARNING("Pixels are not sorted correctly, please include the "
                "TelDigitSorter module before running the Clusterizer or fix "
                "the input to be ordered by v,u in ascending order");
      return;
    }
    //Remember last pixel to check sorting
    lastPixel = px;

    //Ignore digits with not enough signal. Has to be done after the check of
    //the SensorID to make sure we compare to the correct noise level
    if (!m_noiseMap(px, 0.1)) continue;

    // Find correct cluster and add pixel to cluster
    m_cache->findCluster(px.getU(), px.getV()).add(px);
  }
  writeClusters(sensorID);
}

void TelClusterizerModule::createRelationLookup(const RelationArray& relation, RelationLookup& lookup, size_t digits)
{
  lookup.clear();
  //If we don't have a relation we don't build a lookuptable
  if (!relation) return;
  //Resize to number of digits and set all values
  lookup.resize(digits);
  for (const RelationElement& element : relation) {
    lookup[element.getFromIndex()] = &element;
  }
}

void TelClusterizerModule::fillRelationMap(const RelationLookup& lookup, std::map<unsigned int, float>& relation,
                                           unsigned int index)
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

void TelClusterizerModule::writeClusters(VxdID sensorID)
{
  if (m_cache->empty())
    return;

  //Get all datastore elements
  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<TelDigit> storeDigits(m_storeDigitsName);
  const StoreArray<TelTrueHit> storeTrueHits(m_storeTrueHitsName);
#ifdef MAKE_TELCLUSTERS
  StoreArray<TelCluster> storeClusters(m_storeClustersName);
#else
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);
#endif
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

  for (ClusterCandidate& cls : *m_cache) {

    double rho(0);
    ClusterProjection projU, projV;
    mc_relations.clear();
    truehit_relations.clear();
    digit_weights.clear();
    digit_weights.reserve(cls.size());

    for (const Pixel& px : cls.pixels()) {
      //Add the Pixel information to the two projections
      projU.add(px.getU(), info.getUCellPosition(px.getU()));
      projV.add(px.getV(), info.getVCellPosition(px.getV()));

      //Obtain relations from MCParticles
      fillRelationMap(m_mcRelation, mc_relations, px.getIndex());
      //Obtain relations from TelTrueHits
      fillRelationMap(m_trueRelation, truehit_relations, px.getIndex());
      //Save the weight of the digits for the Cluster->Digit relation
      digit_weights.emplace_back(px.getIndex(), 1.0);
    }
    projU.finalize();
    projV.finalize();

    const double pitchU = info.getUPitch();
    const double pitchV = info.getVPitch(projV.getPos());
    // Calculate shape correlation coefficient: only for non-trivial shapes
    if (projU.getSize() > 1 && projV.getSize() > 1) {
      // Add in-pixel position noise to smear the correlation
      double posUU = cls.size() * pitchU * pitchU / 12.0;
      double posVV = cls.size() * pitchV * pitchV / 12.0;
      double posUV(0);
      for (const Pixel& px : cls.pixels()) {
        const double du = info.getUCellPosition(px.getU()) - projU.getPos();
        const double dv = info.getVCellPosition(px.getV()) - projV.getPos();
        posUU += du * du;
        posVV += dv * dv;
        posUV += du * dv;
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
    storeClusters.appendNew(sensorID, projU.getPos(), projV.getPos(),
                            projU.getError(), projV.getError(), rho, cls.size(), 1, cls.size(),
                            projU.getSize(), projV.getSize(), projU.getMinCell(), projV.getMinCell()
                           );

    //Create Relations to this Digit
    if (!mc_relations.empty()) relClusterMCParticle.add(clsIndex, mc_relations.begin(), mc_relations.end());
    if (!truehit_relations.empty()) relClusterTrueHit.add(clsIndex, truehit_relations.begin(), truehit_relations.end());
    relClusterDigit.add(clsIndex, digit_weights.begin(), digit_weights.end());
  }

  m_cache->clear();
}

void TelClusterizerModule::calculatePositionError(const ClusterCandidate&, ClusterProjection& primary,
                                                  const ClusterProjection& secondary, double minPitch, double centerPitch, double maxPitch)
{
  if (primary.getSize() >= static_cast<unsigned int>(m_sizeHeadTail)) { //Analog head tail
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
    primary.setError(centerPitch * (secondary.getSize() + 2) * m_cutElectrons / (primary.getCharge() +
                     (secondary.getSize() + 3) * m_cutElectrons));
  } else {
    const double sn = 10.0;
    primary.setError(2.0 * centerPitch / sn);
  }
}
