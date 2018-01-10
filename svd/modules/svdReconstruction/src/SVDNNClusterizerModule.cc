/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdReconstruction/SVDNNClusterizerModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>

#include <svd/reconstruction/NNWaveFitTool.h>

#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <functional>
#include <cassert>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDNNClusterizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDNNClusterizerModule::SVDNNClusterizerModule() : Module()
{
  B2DEBUG(200, "SVDNNClusterizerModule ctor");
  //Set module properties
  setDescription("Clusterize SVDRecoDigits and reconstruct hits");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("Digits", m_storeRecoDigitsName,
           "RecoDigits collection name", string(""));
  addParam("Clusters", m_storeClustersName,
           "Cluster collection name", string(""));
  addParam("TrueHits", m_storeTrueHitsName,
           "TrueHit collection name", string(""));
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticles collection name", string(""));

  // 2. Calibration and time fitter sources
  addParam("TimeFitterName", m_timeFitterName,
           "Name of time fitter data file", string("SVDTimeNet_6samples"));
  addParam("CalibratePeak", m_calibratePeak, "Use calibrattion (vs. default) for peak widths and positions", bool(false));

  // 3. Clustering
  // FIXME: Idiotic names of parameters kept for compatibility with the old clusterizer.
  addParam("NoiseSN", m_cutAdjacent,
           "SN for digits to be considered for clustering", m_cutAdjacent);
  addParam("SeedSN", m_cutSeed,
           "SN for digits to be considered as seed", m_cutSeed);
  addParam("ClusterSN", m_cutCluster,
           "Minimum SN for clusters", m_cutCluster);
  addParam("HeadTailSize", m_sizeHeadTail,
           "Cluster size at which to switch to Analog head tail algorithm", m_sizeHeadTail);

}

void SVDNNClusterizerModule::initialize()
{
  //Register collections
  StoreArray<SVDCluster> storeClusters(m_storeClustersName);
  StoreArray<SVDRecoDigit> storeRecoDigits(m_storeRecoDigitsName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);

  storeClusters.registerInDataStore();
  storeRecoDigits.isRequired();
  storeTrueHits.isOptional();
  storeMCParticles.isOptional();

  RelationArray relClusterRecoDigits(storeClusters, storeRecoDigits);
  RelationArray relClusterTrueHits(storeClusters, storeTrueHits);
  RelationArray relClusterMCParticles(storeClusters, storeMCParticles);
  RelationArray relRecoDigitTrueHits(storeRecoDigits, storeTrueHits);
  RelationArray relRecoDigitMCParticles(storeRecoDigits, storeMCParticles);

  relClusterRecoDigits.registerInDataStore();
  //Relations to simulation objects only if the ancestor relations exist
  if (relRecoDigitTrueHits.isOptional())
    relClusterTrueHits.registerInDataStore();
  if (relRecoDigitMCParticles.isOptional())
    relClusterMCParticles.registerInDataStore();

  //Store names to speed up creation later
  m_storeClustersName = storeClusters.getName();
  m_storeRecoDigitsName = storeRecoDigits.getName();
  m_storeTrueHitsName = storeTrueHits.getName();
  m_storeMCParticlesName = storeMCParticles.getName();

  m_relClusterRecoDigitName = relClusterRecoDigits.getName();
  m_relClusterTrueHitName = relClusterTrueHits.getName();
  m_relClusterMCParticleName = relClusterMCParticles.getName();
  m_relRecoDigitTrueHitName = relRecoDigitTrueHits.getName();
  m_relRecoDigitMCParticleName = relRecoDigitMCParticles.getName();

  B2INFO(" 1. COLLECTIONS:");
  B2INFO(" -->  MCParticles:        " << m_storeMCParticlesName);
  B2INFO(" -->  Digits:             " << m_storeRecoDigitsName);
  B2INFO(" -->  Clusters:           " << m_storeClustersName);
  B2INFO(" -->  TrueHits:           " << m_storeTrueHitsName);
  B2INFO(" -->  DigitMCRel:         " << m_relRecoDigitMCParticleName);
  B2INFO(" -->  ClusterMCRel:       " << m_relClusterMCParticleName);
  B2INFO(" -->  ClusterDigitRel:    " << m_relClusterRecoDigitName);
  B2INFO(" -->  DigitTrueRel:       " << m_relRecoDigitTrueHitName);
  B2INFO(" -->  ClusterTrueRel:     " << m_relClusterTrueHitName);
  B2INFO(" 2. CALIBRATION DATA:");
  B2INFO(" -->  Time NN:            " << m_timeFitterName);
  B2INFO(" 4. CLUSTERING:");
  B2INFO(" -->  Neighbour cut:      " << m_cutAdjacent);
  B2INFO(" -->  Seed cut:           " << m_cutSeed);
  B2INFO(" -->  Cluster charge cut: " << m_cutCluster);
  B2INFO(" -->  HT for clusters >:  " << m_sizeHeadTail);

  // Properly initialize the NN time fitter
  // FIXME: Should be moved to beginRun
  // FIXME: No support for 3/6 sample switching within a run/event
  DBObjPtr<DatabaseRepresentationOfWeightfile> dbXml(m_timeFitterName);
  m_fitter.setNetwrok(dbXml->m_data);
}

void SVDNNClusterizerModule::createRelationLookup(const RelationArray& relation,
                                                  RelationLookup& lookup, size_t digits)
{
  lookup.clear();
  //If we don't have a relation we don't build a lookuptable
  if (!relation) return;
  //Resize to number of digits and set all values
  lookup.resize(digits);
  for (const auto& element : relation) {
    lookup[element.getFromIndex()] = &element;
  }
}

void SVDNNClusterizerModule::fillRelationMap(const RelationLookup& lookup,
                                             std::map<unsigned int, float>& relation, unsigned int index)
{
  //If the lookup table is not empty and the element is set
  if (!lookup.empty() && lookup[index]) {
    const RelationElement& element = *lookup[index];
    const unsigned int size = element.getSize();
    //Add all Relations to the map
    for (unsigned int i = 0; i < size; ++i) {
      //negative weights are from ignored particles, we don't like them and
      //thus ignore them :D
      if (element.getWeight(i) < 0) continue;
      relation[element.getToIndex(i)] += element.getWeight(i);
    }
  }
}

void SVDNNClusterizerModule::event()
{
  const StoreArray<SVDRecoDigit> storeRecoDigits(m_storeRecoDigitsName);
  // If no recodigits, nothing to do
  if (!storeRecoDigits || !storeRecoDigits.getEntries()) return;

  size_t nDigits = storeRecoDigits.getEntries();
  B2DEBUG(90, "Initial size of RecoDigits array: " << nDigits);

  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);

  RelationArray relRecoDigitMCParticle(storeRecoDigits, storeMCParticles, m_relRecoDigitMCParticleName);
  RelationArray relRecoDigitTrueHit(storeRecoDigits, storeTrueHits, m_relRecoDigitTrueHitName);

  StoreArray<SVDCluster> storeClusters(m_storeClustersName);
  storeClusters.clear();

  RelationArray relClusterMCParticle(storeClusters, storeMCParticles,
                                     m_relClusterMCParticleName);
  if (relClusterMCParticle) relClusterMCParticle.clear();

  RelationArray relClusterRecoDigit(storeClusters, storeRecoDigits,
                                    m_relClusterRecoDigitName);
  if (relClusterRecoDigit) relClusterRecoDigit.clear();

  RelationArray relClusterTrueHit(storeClusters, storeTrueHits,
                                  m_relClusterTrueHitName);
  if (relClusterTrueHit) relClusterTrueHit.clear();

  //Build lookup tables for relations
  createRelationLookup(relRecoDigitMCParticle, m_mcRelation, nDigits);
  createRelationLookup(relRecoDigitTrueHit, m_trueRelation, nDigits);

  // Create fit tool object
  NNWaveFitTool fitTool = m_fitter.getFitTool();

  // I. Group digits by sensor/side.
  vector<pair<unsigned short, unsigned short> > sensorDigits;
  VxdID lastSensorID(0);
  size_t firstSensorDigit = 0;
  for (size_t iDigit = 0; iDigit < nDigits; ++iDigit) {
    const SVDRecoDigit& adigit = *storeRecoDigits[iDigit];
    VxdID sensorID = adigit.getSensorID();
    sensorID.setSegmentNumber(adigit.isUStrip() ? 1 : 0);
    if (sensorID != lastSensorID) { // we have a new sensor side
      sensorDigits.push_back(make_pair(firstSensorDigit, iDigit));
      firstSensorDigit = iDigit;
      lastSensorID = sensorID;
    }
  }
  // save last VxdID
  sensorDigits.push_back(make_pair(firstSensorDigit, nDigits));

  // ICYCLE OVER SENSORS
  for (auto id_indices : sensorDigits) {
    // Retrieve parameters from sensorDigits
    unsigned int firstDigit = id_indices.first;
    unsigned int lastDigit = id_indices.second;
    // Get VXDID from the first digit
    const SVDRecoDigit& sampleRecoDigit = *storeRecoDigits[firstDigit];
    VxdID sensorID = sampleRecoDigit.getSensorID();
    bool isU = sampleRecoDigit.isUStrip();

    // Retrieve sensor parameters from GeoCache
    const SensorInfo& sensorInfo = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));

    // 4. Cycle through digits and form clusters on the way.

    // Identify contiguous groups of strips
    // We maintain two pointers: firstDigit and lastDigit. We also keep track of last seen
    // strip number.
    // We start with firstDigit = lastDigit = 0
    // If there is a gap in strip numbers or the current digit/strip is invalid, we save the
    // current group (fd, cd), unless empty, and set fd = cd.
    // If current strip is valid, we include it: ld = cd+1.

    vector<pair<size_t, size_t> > stripGroups;
    size_t firstClusterDigit = firstDigit;
    size_t lastClusterDigit = firstDigit;
    short lastStrip = -2;

    B2DEBUG(300, "Clustering digits " << firstDigit << " to " << lastDigit);
    for (size_t iDigit = firstDigit; iDigit < lastDigit; ++iDigit) {

      const SVDRecoDigit& recoDigit = *storeRecoDigits[iDigit];
      unsigned short currentStrip = recoDigit.getCellID();
      B2DEBUG(300, "Digit " << iDigit << ", strip: " << currentStrip << ", lastStrip: " << lastStrip);
      B2DEBUG(300, "First CD: " << firstClusterDigit << " Last CD: " << lastClusterDigit);

      // See if we have a gap in strip numbers.
      bool consecutive = ((currentStrip - lastStrip) == 1);
      lastStrip = currentStrip;

      B2DEBUG(300, (consecutive ? "consecutive" : "gap"));

      // If we have a gap, we save if there is something to save
      if (!consecutive && (firstClusterDigit < lastClusterDigit)) {
        B2DEBUG(300, "Saving (" << firstClusterDigit << ", " << lastClusterDigit << ")");
        stripGroups.emplace_back(firstClusterDigit, lastClusterDigit);
      }

      // Update counters
      if (consecutive) {// consecutive : include in cluster
        lastClusterDigit = iDigit + 1;
      } else { // valid after a gap = new cluster
        firstClusterDigit = iDigit;
        lastClusterDigit = iDigit + 1;
      }
    } // for digits
    // save last if any
    if (firstClusterDigit < lastClusterDigit) {
      B2DEBUG(300, "Saving (" << firstClusterDigit << ", " << lastDigit << ")");
      stripGroups.emplace_back(firstClusterDigit, lastDigit);
    }

    ostringstream os;
    os << "StripGroups: " << endl;
    for (auto item : stripGroups) {
      os << "(" << item.first << ", " << item.second << "), ";
    }
    B2DEBUG(300, os.str());

    // 5. Now we loop through stripGroups and form clusters.
    // We are still keeping digit indices, so we are keeping hold on the relations as wwell.
    double pitch = isU ? sensorInfo.getUPitch() : sensorInfo.getVPitch();

    // FIXME: Pretty stupid. There should be ClusterCandidate type to hold all this, as
    // well as normed samples. It could calculate a lot of things by itself and make the
    // code cleaner.
    vector<unsigned short> stripNumbers;
    vector<float> stripPositions;
    vector<float> stripNoises;
    vector<float> stripGains;
    vector<float> timeShifts;
    vector<float> waveWidths;
    vector<apvSamples> storedNormedSamples;
    vector<SVDRecoDigit::OutputProbArray> storedPDFs;

    // Now reconstruct clusters
    for (auto clusterBounds : stripGroups) {

      unsigned short clusterSize = clusterBounds.second - clusterBounds.first;
      assert(clusterSize > 0);

      stripNumbers.clear();
      stripPositions.clear();
      stripNoises.clear();
      stripGains.clear();
      timeShifts.clear();
      waveWidths.clear();

      for (size_t iDigit = clusterBounds.first; iDigit < clusterBounds.second; ++iDigit) {
        // Now we take the normed samples and calculate the common
        // cluster probability distribution and, from it, the time and its error, and
        // the probability of the hit being from beam background.
        const SVDRecoDigit& recoDigit = *storeRecoDigits[iDigit];

        unsigned short stripNo = recoDigit.getCellID();
        stripNumbers.push_back(stripNo);
        // Is the calibrations interface sensible?
        double stripNoiseADU = m_noiseCal.getNoise(sensorID, isU, stripNo);
        stripNoises.push_back(
          m_pulseShapeCal.getChargeFromADC(sensorID, isU, stripNo, stripNoiseADU)
        );
        // Some calibrations magic.
        // FIXME: Only use calibration on real data. Until simulations correspond to
        // default calibrtion, we cannot use it.
        double peakWidth = 270;
        double timeShift = isU ? 4.0 : 0.0;
        if (m_calibratePeak) {
          peakWidth = 1.988 * m_pulseShapeCal.getWidth(sensorID, isU, stripNo);
          timeShift = m_pulseShapeCal.getPeakTime(sensorID, isU, stripNo)
                      - 0.25 * peakWidth;
        }
        waveWidths.push_back(peakWidth);
        timeShifts.push_back(timeShift);
        stripPositions.push_back(
          isU ? sensorInfo.getUCellPosition(stripNo) : sensorInfo.getVCellPosition(stripNo)
        );
        // Recover samples from ShaperDigits and store them.
        apvSamples normedSamples;
        const SVDShaperDigit* shaperDigit = recoDigit.getRelatedTo<SVDShaperDigit>();
        if (!shaperDigit) // PANIC, this should not happen.
          B2FATAL("Missing SVDRecoDigits->SVDShaperDigits relation. This should not happen.");
        auto samples = shaperDigit->getSamples();
        transform(samples.begin(), samples.end(), normedSamples.begin(),
                  bind2nd(divides<float>(), stripNoiseADU));

        // These are from ShaperDigit, we need to zeroSuppress again,
        // just in case.
        zeroSuppress(normedSamples, m_cutAdjacent);
        storedNormedSamples.emplace_back(normedSamples);
        storedPDFs.emplace_back(recoDigit.getProbabilities());
      }
      // The formulas don't take into account differences in strip noises. So we take RMS
      // strip noise as cluster noise.
      // clusterNoise is noise in electrons.
      float clusterNoise = sqrt(
                             1.0 / clusterSize
                             * inner_product(stripNoises.begin(), stripNoises.end(), stripNoises.begin(), 0.0)
                           );
      B2DEBUG(200, "RMS cluster noise: " << clusterNoise);

      // This will hold component pdfs. We may want to rememeber them to study
      // homogeneity of cluster times.
      shared_ptr<nnFitterBinData> pStrip;
      // This will aggregate the components pdfs to get cluster time pdf
      nnFitterBinData pCluster(m_fitter.getBinCenters().size());
      fill(pCluster.begin(), pCluster.end(), double(1.0));

      for (size_t iClusterStrip = 0; iClusterStrip < clusterSize; ++iClusterStrip) {
        size_t iDigit = clusterBounds.first + iClusterStrip;
        ostringstream os1;
        os1 << "Input to NNFitter: iDigit = " << iDigit << endl << "Samples: ";
        copy(storedNormedSamples[iClusterStrip].begin(), storedNormedSamples[iClusterStrip].end(),
             ostream_iterator<double>(os1, " "));
        os1 << endl;
        os1 << "PDF from RecoDigit: " << endl;
        copy(storedPDFs[iClusterStrip].begin(), storedPDFs[iClusterStrip].end(), ostream_iterator<double>(os1, " "));
        os1 << endl;
        fitTool.multiply(pCluster, storedPDFs[iClusterStrip]);
        os1 << "Accummulated: " << endl;
        copy(pCluster.begin(), pCluster.end(), ostream_iterator<double>(os1, " "));
        B2DEBUG(200, os1.str());
      }
      // We can now calculate clsuter time and its error.
      double clusterTime, clusterTimeErr;
      tie(clusterTime, clusterTimeErr) = fitTool.getTimeShift(pCluster);
      B2DEBUG(200, "Time: " << clusterTime << " +/- " << clusterTimeErr);
      // Now we have the cluster time pdf, so we can calculate amplitudes.
      // In the next cycle thrugh cluster's digits, we calculate ampltidues and their
      // errors.
      vector<double> stripAmplitudes(stripNoises.size());
      vector<double> stripAmplitudeErrors(stripNoises.size());
      double clusterChi2 = 0.0;
      for (size_t iClusterStrip = 0; iClusterStrip < clusterSize; ++iClusterStrip) {
        size_t iDigit = clusterBounds.first + iClusterStrip;
        double snAmp, snAmpError, chi2;
        tie(snAmp, snAmpError, chi2) =
          fitTool.getAmplitudeChi2(storedNormedSamples[iClusterStrip], clusterTime, waveWidths[iClusterStrip]);
        // We have to de-normalize: Multiply amplitudes in SN units by noises in electrons.
        stripAmplitudes[iClusterStrip] = stripNoises[iClusterStrip] * snAmp;
        stripAmplitudeErrors[iClusterStrip] = stripNoises[iClusterStrip] * snAmpError;
        clusterChi2 += chi2;
        B2DEBUG(200, "Digit " << iDigit << " Noise: " << stripNoises[iClusterStrip]
                << " Amplitude: " << stripAmplitudes[iClusterStrip]
                << " +/- " << stripAmplitudeErrors[iClusterStrip]
                << " Chi2: " << chi2
               );
      }
      // Cluster charge, S/N, chi2
      float clusterCharge = accumulate(stripAmplitudes.begin(), stripAmplitudes.end(), 0.0);
      float clusterChargeError = sqrt(
                                   inner_product(stripAmplitudeErrors.begin(), stripAmplitudeErrors.end(),
                                                 stripAmplitudeErrors.begin(), 0.0)
                                 );
      float clusterSN = (clusterChargeError > 0) ? clusterCharge / clusterChargeError : clusterCharge;
      // FIXME: Return chi2 and ndf from GetApmplitude, this is ugly.
      clusterChi2 /= clusterSize;
      // Cluster seed
      size_t seedIndex = distance(stripAmplitudes.begin(), max_element(
                                    stripAmplitudes.begin(), stripAmplitudes.end()));
      float clusterSeedCharge = stripAmplitudes[seedIndex];
      B2DEBUG(200, "Cluster parameters:");
      B2DEBUG(200, "Charge: " << clusterCharge << " +/- " << clusterChargeError);
      B2DEBUG(200, "Seed: " << clusterSeedCharge << " +/- " << stripAmplitudeErrors[seedIndex]);
      B2DEBUG(200, "S/N: " <<  clusterSN);
      B2DEBUG(200, "chi2: " <<  clusterChi2);

      // Hit position
      float clusterPosition, clusterPositionError;

      // Now censoring:
      // NB: We don't censor strip charges again. The requirement of 3 consecutive samples
      // above threshold is a good enough safeguard.
      if ((clusterCharge < clusterNoise * m_cutCluster) || (clusterSeedCharge < clusterNoise * m_cutSeed))
        continue;

      if (clusterSize < m_sizeHeadTail) { // centre of gravity
        clusterPosition = 1.0 / clusterCharge * inner_product(
                            stripAmplitudes.begin(), stripAmplitudes.end(), stripPositions.begin(), 0.0
                          );
        // Strip charge equal to zero-suppression threshold
        float phantomCharge = m_cutAdjacent * clusterNoise;
        if (clusterSize == 1) {
          clusterPositionError = pitch * phantomCharge / (clusterCharge + phantomCharge);
        } else {
          clusterPositionError = pitch * phantomCharge / clusterCharge;
        }
      } else { // Head-tail
        float leftStripCharge = stripAmplitudes.front();
        float leftPos = stripPositions.front();
        float rightStripCharge = stripAmplitudes.back();
        float rightPos = stripPositions.back();
        float centreCharge = (clusterCharge - leftStripCharge - rightStripCharge) / (clusterSize - 2);
        leftStripCharge = (leftStripCharge < centreCharge) ? leftStripCharge : centreCharge;
        rightStripCharge = (rightStripCharge < centreCharge) ? rightStripCharge : centreCharge;
        clusterPosition = 0.5 * (leftPos + rightPos)
                          + 0.5 * (rightStripCharge - leftStripCharge) / centreCharge * pitch;
        float sn = centreCharge / m_cutAdjacent / clusterNoise;
        // Rough estimates of Landau noise
        float landauHead = leftStripCharge / centreCharge;
        double landauTail = rightStripCharge / centreCharge;
        clusterPositionError = 0.5 * pitch * sqrt(1.0 / sn / sn
                                                  + 0.5 * landauHead * landauHead + 0.5 * landauTail * landauTail);
      }
      //Lorentz shift correction
      clusterPosition -= sensorInfo.getLorentzShift(isU, clusterPosition);

      // Finally, we save the cluster and its relations.
      map<unsigned int, float> mc_relations;
      map<unsigned int, float> truehit_relations;
      vector<pair<unsigned int, float> > digit_weights;
      digit_weights.reserve(clusterSize);

      for (size_t iDigit = clusterBounds.first; iDigit < clusterBounds.second; ++iDigit) {
        // Store relations to MCParticles and SVDTrueHits
        fillRelationMap(m_mcRelation, mc_relations, iDigit);
        fillRelationMap(m_trueRelation, truehit_relations, iDigit);
        //Add digit to the Cluster->Digit relation list
        digit_weights.emplace_back(iDigit, stripAmplitudes[iDigit - clusterBounds.first]);
      }

      //Store the cluster into Datastore ...
      int clsIndex = storeClusters.getEntries();
      VxdID clusterSensorID = sensorID;
      clusterSensorID.setSegmentNumber(0);
      storeClusters.appendNew(
        SVDCluster(sensorID, isU, clusterPosition, clusterPositionError, clusterTime,
                   clusterTimeErr, clusterCharge, clusterSeedCharge, clusterSize, clusterSN, clusterChi2)
      );

      //Create relations to the cluster
      if (!mc_relations.empty()) {
        relClusterMCParticle.add(clsIndex, mc_relations.begin(), mc_relations.end());
      }
      if (!truehit_relations.empty()) {
        relClusterTrueHit.add(clsIndex, truehit_relations.begin(), truehit_relations.end());
      }
      relClusterRecoDigit.add(clsIndex, digit_weights.begin(), digit_weights.end());
    } // CYCLE OVER CLUSTERS for stripGroups

  } // CYCLE OVER SENSORS for items in sensorDigits

  B2DEBUG(100, "Number of clusters: " << storeClusters.getEntries());

} // event()


