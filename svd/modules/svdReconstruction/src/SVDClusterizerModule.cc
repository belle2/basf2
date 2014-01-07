/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Peter Kvasnicka                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdReconstruction/SVDClusterizerModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>

#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <boost/foreach.hpp>

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

SVDClusterizerModule::SVDClusterizerModule() : Module(), m_elNoise(2000.0),
  m_cutSeed(5.0), m_cutAdjacent(2.5), m_cutCluster(8.0), m_sizeHeadTail(3),
  c_minSamples(3), m_timeTolerance(30), m_shapingTimeElectrons(55),
  m_shapingTimeHoles(60), m_samplingTime(30), m_refTime(0.0), m_assumeSorted(false)
{
  //Set module properties
  setDescription("Clusterize SVDDigits and reconstruct hits");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  // 1. Collections.
  addParam("Digits", m_storeDigitsName,
           "Digits collection name", string(""));
  addParam("Clusters", m_storeClustersName,
           "Cluster collection name", string(""));
  addParam("TrueHits", m_storeTrueHitsName,
           "TrueHit collection name", string(""));
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticles collection name", string(""));

  // 2. Physics.
  // FIXME: The mean Lorentz angles should be set sensor-wise, or at least
  // separately for slanted sensors.
  addParam("TanLorentz_electrons", m_tanLorentzAngle_electrons,
           "Tangent of the Lorentz angle for electrons", double(0.0));
  addParam("TanLorentz_holes", m_tanLorentzAngle_holes,
           "Tangent of the Lorentz angle for holes", double(0.08));

  // 3. Noise
  addParam("ElectronicNoise", m_elNoise,
           "RMS signal noise, set in ENC", m_elNoise);

  // 4. Clustering
  addParam("NoiseSN", m_cutAdjacent,
           "SN for digits to be considered for clustering", m_cutAdjacent);
  addParam("SeedSN", m_cutSeed,
           "SN for digits to be considered as seed", m_cutSeed);
  addParam("ClusterSN", m_cutCluster,
           "Minimum SN for clusters", m_cutCluster);
  addParam("HeadTailSize", m_sizeHeadTail,
           "Cluster size at which to switch to Analog head tail algorithm", m_sizeHeadTail);
  addParam("TimeTolerance", m_timeTolerance,
           "Maximum allowable RMS of signal times in a cluster", m_timeTolerance);

  //5. Timing: all times are expected in ns.
  addParam("ShapingTimeElectrons", m_shapingTimeElectrons,
           "Typical decay time for signals of electrons", m_shapingTimeElectrons);
  addParam("ShapingTimeHoles", m_shapingTimeHoles,
           "Typical decay time for signals of holes", m_shapingTimeHoles);
  addParam("SamplingTime", m_samplingTime,
           "Time between two consecutive signal samples", m_samplingTime);
  /** Whether or not to apply a time window cut */
  addParam("Apply time window", m_applyWindow,
           "Whether or not to apply an acceptance window based on trigger time", bool(false));
  /** Time of the trigger. */
  addParam("TriggerTime", m_triggerTime,
           "Time of the trigger", double(0.0));
  addParam("Acceptance window size", m_acceptance,
           "Size of the acceptance window following the trigger", double(100));
}

void SVDClusterizerModule::initialize()
{
  //Register collections
  StoreArray<SVDCluster> storeClusters(m_storeClustersName);
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);

  storeClusters.registerAsPersistent();
  storeDigits.required();
  storeTrueHits.isOptional();
  storeMCParticles.isOptional();

  RelationArray relClusterDigits(storeClusters, storeDigits);
  RelationArray relClusterTrueHits(storeClusters, storeTrueHits);
  RelationArray relClusterMCParticles(storeClusters, storeMCParticles);
  RelationArray relDigitTrueHits(storeDigits, storeTrueHits);
  RelationArray relDigitMCParticles(storeDigits, storeMCParticles);

  relClusterDigits.registerAsPersistent();
  //Relations to simulation objects only if the ancestor relations exist
  if (relDigitTrueHits.isOptional())
    relClusterTrueHits.registerAsPersistent();
  if (relDigitMCParticles.isOptional())
    relClusterMCParticles.registerAsPersistent();

  //Store names to speed up creation later
  m_storeClustersName = storeClusters.getName();
  m_storeDigitsName = storeDigits.getName();
  m_storeTrueHitsName = storeTrueHits.getName();
  m_storeMCParticlesName = storeMCParticles.getName();

  m_relClusterDigitName = relClusterDigits.getName();
  m_relClusterTrueHitName = relClusterTrueHits.getName();
  m_relClusterMCParticleName = relClusterMCParticles.getName();
  m_relDigitTrueHitName = relDigitTrueHits.getName();
  m_relDigitMCParticleName = relDigitMCParticles.getName();

  // Report:
  B2INFO("SVDClusterizer Parameters (in default system unit, *=cannot be set directly):");

  B2INFO(" 1. COLLECTIONS:");
  B2INFO(" -->  MCParticles:        " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2INFO(" -->  Digits:             " << DataStore::arrayName<SVDDigit>(m_storeDigitsName));
  B2INFO(" -->  Clusters:           " << DataStore::arrayName<SVDCluster>(m_storeClustersName));
  B2INFO(" -->  TrueHits:           " << DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName));
  B2INFO(" -->  DigitMCRel:         " << m_relDigitMCParticleName);
  B2INFO(" -->  ClusterMCRel:       " << m_relClusterMCParticleName);
  B2INFO(" -->  ClusterDigitRel:    " << m_relClusterDigitName);
  B2INFO(" -->  DigitTrueRel:       " << m_relDigitTrueHitName);
  B2INFO(" -->  ClusterTrueRel:     " << m_relClusterTrueHitName);
  B2INFO(" -->  AssumeSorted:       " << (m_assumeSorted ? "true" : "false"));
  B2INFO(" -->  TanLorentz (e-):    " << m_tanLorentzAngle_electrons);
  B2INFO(" -->  TanLorentz (h+):    " << m_tanLorentzAngle_holes);

  B2INFO(" 3. NOISE:");
  B2INFO(" -->  RMS signal noise:   " << m_elNoise);

  B2INFO(" 4. CLUSTERING:");
  B2INFO(" -->  Neighbour cut:      " << m_cutAdjacent);
  B2INFO(" -->  Seed cut:           " << m_cutSeed);
  B2INFO(" -->  Cluster charge cut: " << m_cutCluster);
  B2INFO(" -->  Min. samples/strip: " << c_minSamples);
  B2INFO(" -->  Time tolerance:     " << m_timeTolerance);

  B2INFO(" TIMING: ");
  B2INFO(" -->  Decay time (e-):    " << m_shapingTimeElectrons);
  B2INFO(" -->  Decay time (h-):    " << m_shapingTimeHoles);
  B2INFO(" -->  Sampling time:      " << m_samplingTime);
  B2INFO(" -->  Acceptance window:  " << (m_applyWindow ? "true" : "false"));
  B2INFO(" -->  Trigger time:       " << m_triggerTime);
  B2INFO(" -->  Acceptance w. size: " << m_acceptance);


  //FIXME: This is still static noise for all pixels, can be elaborated on in the
  // right time. E.g., do we have different noise levels on the two sides of a
  // a sensor? on different sensor types?
  m_noiseMap.setNoiseLevel(m_elNoise);
}

inline void SVDClusterizerModule::findCluster(const Sample& sample)
{
  ClusterCandidate* prev = m_cache.findCluster(sample.getTime(), sample.getCellID());
  if (!prev) {
    m_clusters.push_back(ClusterCandidate());
    prev = &m_clusters.back();
  }
  prev->add(sample);
  m_cache.setLast(sample.getTime(), sample.getCellID(), prev);
}


void SVDClusterizerModule::event()
{
  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  const StoreArray<SVDDigit>   storeDigits(m_storeDigitsName);
  StoreArray<SVDCluster> storeClusters(m_storeClustersName);

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

  int nDigits = storeDigits.getEntries();
  if (nDigits == 0)
    return;

  m_clusters.clear();
  m_cache.clear();

  if (!m_assumeSorted) {
    //If the pixels are in random order, we have to sort them first before we can cluster them
    Sensors sensors;
    //Fill sensors
    for (int i = 0; i < nDigits; i++) {
      Sample sample(storeDigits[i], i);
      VxdID sensorID = sample.getDigit()->getSensorID();
      int side = sample.getDigit()->isUStrip() ? 0 : 1;
      std::pair<SensorSide::iterator, bool> it = sensors[sensorID][side].insert(sample);
      if (!it.second) B2ERROR("Sample (" << sample.getTime() << "," << sample.getCellID() << "/" << (sample.isUStrip() ? 0 : 1) << ") in sensor "
                                << (string)sensorID << " is already set, ignoring second occurrence.");
    }

    //Now we loop over sensors and cluster each sensor in turn
    for (SensorIterator it = sensors.begin(); it != sensors.end(); it++) {
      for (int iSide = 0; iSide < 2; ++iSide) {
        m_noiseMap.setSensorID(it->first);
        BOOST_FOREACH(const SVD::Sample & sample, it->second[iSide]) {
          if (!m_noiseMap(sample, m_cutAdjacent)) continue;
          findCluster(sample);
        }
        writeClusters(it->first, iSide);
        it->second[iSide].clear();
      }
    }
  } else {
    //If we can assume that all pixels are already sorted we can skip the
    //reordering and directly cluster them; once the side or sensorID changes,
    //we write out all existing clusters and continue
    VxdID sensorID;
    bool uSide(true);
    unsigned int lastTime(0), lastStrip(0);
    for (int i = 0; i < nDigits; i++) {
      Sample sample(storeDigits[i], i);
      //Load the correct noise map for the first pixel
      if (i == 0) m_noiseMap.setSensorID(sample.getSensorID());
      //Ignore digits with insufficient signal
      if (!m_noiseMap(sample, m_cutAdjacent)) continue;

      //Check for sorting as precaution
      if (lastStrip > sample.getCellID() || (lastStrip == sample.getCellID() && lastTime > sample.getTime())) {
        B2FATAL("Digits are not sorted correctly, please change the assumeSorted parameter "
                "to false or fix the input to be ordered by strip number and time in ascending order");
      }
      lastTime = sample.getTime();
      lastStrip = sample.getCellID();

      //Other side or new sensor: write clusters
      if ((uSide != sample.isUStrip()) || (sensorID != sample.getSensorID())) {
        writeClusters(sensorID, uSide ? 0 : 1);
        // Reset the guards
        uSide = sample.isUStrip();
        sensorID = sample.getSensorID();
        //Load the correct noise map for the new sensor
        m_noiseMap.setSensorID(sensorID);
      }
      //Find the correct cluster and add sample to cluster
      findCluster(sample);
    }
    writeClusters(sensorID, uSide ? 0 : 1);
  }
}

void SVDClusterizerModule::writeClusters(VxdID sensorID, int side)
{
  if (m_clusters.empty()) return;

  //Get all datastore elements
  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<SVDDigit>   storeDigits(m_storeDigitsName);
  const StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<SVDCluster> storeClusters(m_storeClustersName);

  RelationArray relClusterMCParticle(storeClusters, storeMCParticles, m_relClusterMCParticleName);
  RelationArray relClusterDigit(storeClusters, storeDigits, m_relClusterDigitName);
  RelationArray relClusterTrueHit(storeClusters, storeTrueHits, m_relClusterTrueHitName);

  RelationIndex<SVDDigit, MCParticle> relDigitMCParticle(storeDigits, storeMCParticles, m_relDigitMCParticleName);
  RelationIndex<SVDDigit, SVDTrueHit> relDigitTrueHit(storeDigits, storeTrueHits, m_relDigitTrueHitName);

  //Get Geometry information
  const SensorInfo& info = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));
  bool isU = (side == 0);
  double pitch = isU ? info.getUPitch() : info.getVPitch();

  for (ClusterCandidate & cls : m_clusters) {
    //Check for noise cuts
    if (!(cls.size() > 0 && m_noiseMap(cls.getCharge(), m_cutCluster) && m_noiseMap(cls.getSeed(), m_cutSeed))) continue;

    const Sample& clusterSeed = cls.getSeed();
    double clusterCharge = cls.getCharge();

    const std::map<unsigned int, float> stripCharges = cls.getStripCharges();
    unsigned int maxStrip = stripCharges.rbegin()->first;
    double maxStripCharge = stripCharges.rbegin()->second;
    unsigned int minStrip = stripCharges.begin()->first;
    double minStripCharge = stripCharges.begin()->second;
    int clusterSize = stripCharges.size();
    double clusterPosition = 0.0;
    double clusterPositionError = 0.0;
    if (clusterSize < m_sizeHeadTail) { // COG, size = 1 or 2
      std::map<unsigned int, float>::const_iterator strip_charge = stripCharges.begin();
      for (; strip_charge != stripCharges.end(); ++strip_charge) {
        double stripPos = isU ? info.getUCellPosition(strip_charge->first)
                          : info.getVCellPosition(strip_charge->first);
        clusterPosition += stripPos * strip_charge->second;
      }
      clusterPosition /= clusterCharge;
      // Compute position error
      if (clusterSize == 1) {
        // Add a strip charge equal to the zero-suppression threshold to compute the error
        double phantomCharge = m_cutAdjacent * m_elNoise;
        clusterPositionError = pitch * phantomCharge / (clusterCharge + phantomCharge);
      } else {
        double a = (clusterSize == 2) ? 1.414 : (clusterSize - 1);
        double sn = clusterCharge / m_elNoise;
        clusterPositionError = a * pitch / sn;
      }
    } else { // Head-tail
      double centreCharge = (clusterCharge - minStripCharge - maxStripCharge) / (clusterSize - 2);
      minStripCharge = (minStripCharge < centreCharge) ? minStripCharge : centreCharge;
      maxStripCharge = (maxStripCharge < centreCharge) ? maxStripCharge : centreCharge;
      double minPos = isU ? info.getUCellPosition(minStrip) : info.getVCellPosition(minStrip);
      double maxPos = isU ? info.getUCellPosition(maxStrip) : info.getVCellPosition(maxStrip);
      clusterPosition = 0.5 * (minPos + maxPos + (maxStripCharge - minStripCharge) / centreCharge * pitch);
      double sn = centreCharge / m_cutAdjacent / m_elNoise;
      // Rough estimates of Landau noise
      double landauHead = minStripCharge / centreCharge;
      double landauTail = maxStripCharge / centreCharge;
      clusterPositionError = 0.5 * pitch * sqrt(2.0 / sn / sn +
                                                0.5 * landauHead * landauHead +
                                                0.5 * landauTail * landauTail);
    }

    // Estimate time - this is currently very crude
    const std::map<unsigned int, unsigned int> stripMaxima = cls.getMaxima();
    const std::map<unsigned int, unsigned int> stripCounts = cls.getCounts();
    // Check that we have enough data in each strip.
    unsigned int maxCount = 0;
    std::map<unsigned int, unsigned int>::const_iterator strip_count = stripCounts.begin();
    for (; strip_count != stripCounts.end(); ++strip_count)
      if (strip_count->second > maxCount) maxCount = strip_count->second;
    if (maxCount < c_minSamples) continue;
    // Constrain time calculation to strips with at least c_minSamples samples.
    std::map<unsigned int, unsigned int>::const_iterator strip_low = stripCounts.begin();
    while (strip_low->second < c_minSamples) ++strip_low;
    unsigned int stripLow = strip_low->first;
    std::map<unsigned int, unsigned int>::const_reverse_iterator strip_high = stripCounts.rbegin();
    while (strip_high->second < c_minSamples) ++strip_high;
    unsigned int stripHigh = strip_high->first;
    double clusterTime = 0.0;
    double restrictedCharge = 0.0;
    for (unsigned int strip = stripLow; strip <= stripHigh; ++strip) {
      double charge = stripCharges.find(strip)->second; // safe
      clusterTime += m_samplingTime * charge * stripMaxima.find(strip)->second;
      restrictedCharge += charge;
    }
    clusterTime /= restrictedCharge;
    double clusterTiimeStd = 0.0;
    if (clusterSize > 1) {
      for (unsigned int strip = stripLow; strip <= stripHigh; ++strip) {
        double charge = stripCharges.find(strip)->second; // safe
        double diff = m_samplingTime * stripMaxima.find(strip)->second - clusterTime;
        clusterTiimeStd += charge * diff * diff;
      }
    }
    clusterTiimeStd = sqrt(clusterTiimeStd / restrictedCharge);
    clusterTime += m_refTime;
    // discard if not within acceptance
    if (m_applyWindow && ((clusterTime < m_triggerTime) || (clusterTime > m_triggerTime + m_acceptance)))
      continue;

    //Lorentz shift correction FIXME: get from Bfield
    if (isU)
      clusterPosition -= 0.5 * info.getThickness() * m_tanLorentzAngle_holes;
    else
      clusterPosition -= 0.5 * info.getThickness() * m_tanLorentzAngle_electrons;


    map<int, float> mc_relations;
    map<int, float> truehit_relations;
    vector<pair<int, float> > digit_weights;
    digit_weights.reserve(cls.size());

    for (const SVD::Sample & sample : cls.samples()) {

      //Fill map with MCParticle relations
      if (relDigitMCParticle) {
        typedef const RelationIndex<SVDDigit, MCParticle>::Element relMC_type;
        for (relMC_type & mcRel : relDigitMCParticle.getElementsFrom(sample.getDigit())) {
          mc_relations[mcRel.indexTo] += mcRel.weight;
        };
      };
      //Fill map with SVDTrueHit relations
      if (relDigitTrueHit) {
        typedef const RelationIndex<SVDDigit, SVDTrueHit>::Element relTrueHit_type;
        for (relTrueHit_type & trueRel : relDigitTrueHit.getElementsFrom(sample.getDigit())) {
          truehit_relations[trueRel.indexTo] += trueRel.weight;
        };
      };
      //Add digit to the Cluster->Digit relation list
      digit_weights.push_back(make_pair(sample.getIndex(), sample.getCharge()));
    }

    //Store Cluster into Datastore ...
    int clsIndex = storeClusters.getEntries();
    storeClusters.appendNew(SVDCluster(
                              clusterSeed.getDigit()->getSensorID(), isU, clusterPosition, clusterPositionError, clusterTime, clusterTiimeStd,
                              clusterSeed.getCharge(), clusterCharge, clusterSize
                            ));

    //Create Relations to this Digit
    if (!mc_relations.empty()) {
      relClusterMCParticle.add(clsIndex, mc_relations.begin(), mc_relations.end());
    }
    if (!truehit_relations.empty()) {
      relClusterTrueHit.add(clsIndex, truehit_relations.begin(), truehit_relations.end());
    }
    relClusterDigit.add(clsIndex, digit_weights.begin(), digit_weights.end());
  }

  m_clusters.clear();
  m_cache.clear();
}
