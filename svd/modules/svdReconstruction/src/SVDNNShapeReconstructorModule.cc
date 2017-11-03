/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdReconstruction/SVDNNShapeReconstructorModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDRecoDigit.h>

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
REG_MODULE(SVDNNShapeReconstructor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDNNShapeReconstructorModule::SVDNNShapeReconstructorModule() : Module()
{
  B2DEBUG(200, "Now in SVDNNShapeReconstructorModule ctor");
  //Set module properties
  setDescription("Reconstruct signals on SVD strips.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("Digits", m_storeShaperDigitsName,
           "Shaperdigits collection name", string(""));
  addParam("RecoDigits", m_storeRecoDigitsName,
           "RecoDigits collection name", string(""));
  addParam("TrueHits", m_storeTrueHitsName,
           "TrueHits collection name", string(""));
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticles collection name", string(""));
  addParam("StripMapFileName", m_stripMapXmlName,
           "Name of strip map file, TB only", string(""));
  addParam("OOMapFileName", m_ooMapXmlName,
           "Name of O-O map xml to decode strip map, TB only", string(""));
  addParam("TimeFitterFileName", m_timeFitterXmlName,
           "Name of time fitter data file", string("svd/data/SVDTimeNet.xml"));
  addParam("WriteRecoDigits", m_writeRecoDigits,
           "Write RecoDigits to output?", m_writeRecoDigits);
  addParam("ZeroSuppressionCut", m_cutAdjacent, "Zero-suppression cut on digits",
           m_cutAdjacent);
}

void SVDNNShapeReconstructorModule::initialize()
{
  //Register collections
  StoreArray<SVDRecoDigit> storeRecoDigits(m_storeRecoDigitsName);
  StoreArray<SVDShaperDigit> storeShaperDigits(m_storeShaperDigitsName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);

  if (!m_writeRecoDigits)
    storeRecoDigits.registerInDataStore(DataStore::c_DontWriteOut);
  else
    storeRecoDigits.registerInDataStore();

  storeShaperDigits.required();
  storeTrueHits.isOptional();
  storeMCParticles.isOptional();

  RelationArray relRecoDigitShaperDigits(storeRecoDigits, storeShaperDigits);
  RelationArray relRecoDigitTrueHits(storeRecoDigits, storeTrueHits);
  RelationArray relRecoDigitMCParticles(storeRecoDigits, storeMCParticles);
  RelationArray relShaperDigitTrueHits(storeShaperDigits, storeTrueHits);
  RelationArray relShaperDigitMCParticles(storeShaperDigits, storeMCParticles);

  if (!m_writeRecoDigits)
    relRecoDigitShaperDigits.registerInDataStore(DataStore::c_DontWriteOut);
  else
    relRecoDigitShaperDigits.registerInDataStore();
  //Relations to simulation objects only if the ancestor relations exist
  if (relShaperDigitTrueHits.isOptional())
    relRecoDigitTrueHits.registerInDataStore();
  if (relShaperDigitMCParticles.isOptional())
    relRecoDigitMCParticles.registerInDataStore();

  //Store names to speed up creation later
  m_storeRecoDigitsName = storeRecoDigits.getName();
  m_storeShaperDigitsName = storeShaperDigits.getName();
  m_storeTrueHitsName = storeTrueHits.getName();
  m_storeMCParticlesName = storeMCParticles.getName();

  m_relRecoDigitShaperDigitName = relRecoDigitShaperDigits.getName();
  m_relRecoDigitTrueHitName = relRecoDigitTrueHits.getName();
  m_relRecoDigitMCParticleName = relRecoDigitMCParticles.getName();
  m_relShaperDigitTrueHitName = relShaperDigitTrueHits.getName();
  m_relShaperDigitMCParticleName = relShaperDigitMCParticles.getName();

  B2INFO(" 1. COLLECTIONS:");
  B2INFO(" -->  MCParticles:        " << m_storeMCParticlesName);
  B2INFO(" -->  Digits:             " << m_storeShaperDigitsName);
  B2INFO(" -->  RecoDigits:           " << m_storeRecoDigitsName);
  B2INFO(" -->  TrueHits:           " << m_storeTrueHitsName);
  B2INFO(" -->  DigitMCRel:         " << m_relShaperDigitMCParticleName);
  B2INFO(" -->  RecoDigitMCRel:       " << m_relRecoDigitMCParticleName);
  B2INFO(" -->  RecoDigitDigitRel:    " << m_relRecoDigitShaperDigitName);
  B2INFO(" -->  DigitTrueRel:       " << m_relShaperDigitTrueHitName);
  B2INFO(" -->  RecoDigitTrueRel:     " << m_relRecoDigitTrueHitName);
  B2INFO(" -->  Save RecoDigits?    " << (m_writeRecoDigits ? "Y" : "N"));
  B2INFO(" 2. CALIBRATION:");
  B2INFO(" -->  StripMap:           " << m_stripMapXmlName);
  B2INFO(" -->  OO-Map:             " << m_ooMapXmlName);
  B2INFO(" -->  Time NN:            " << m_timeFitterXmlName);

  // Now that we have the required filenames (or don't), create the strip map object.
  m_stripMap = std::unique_ptr<StripCalibrationMap>(new StripCalibrationMap(m_stripMapXmlName, m_ooMapXmlName));
  // Properly initialize the NN time fitter
  m_fitter.setNetwrok(m_timeFitterXmlName);
}

void SVDNNShapeReconstructorModule::createRelationLookup(const RelationArray& relation,
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

void SVDNNShapeReconstructorModule::fillRelationMap(const RelationLookup& lookup,
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

void SVDNNShapeReconstructorModule::event()
{
  const StoreArray<SVDShaperDigit> storeShaperDigits(m_storeShaperDigitsName);
  // If no digits, nothing to do
  if (!storeShaperDigits || !storeShaperDigits.getEntries()) return;

  size_t nDigits = storeShaperDigits.getEntries();
  B2DEBUG(90, "Initial size of StoreDigits array: " << nDigits);

  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);

  RelationArray relShaperDigitMCParticle(storeShaperDigits, storeMCParticles, m_relShaperDigitMCParticleName);
  RelationArray relShaperDigitTrueHit(storeShaperDigits, storeTrueHits, m_relShaperDigitTrueHitName);

  StoreArray<SVDRecoDigit> storeRecoDigits(m_storeRecoDigitsName);
  storeRecoDigits.clear();

  RelationArray relRecoDigitMCParticle(storeRecoDigits, storeMCParticles,
                                       m_relRecoDigitMCParticleName);
  if (relRecoDigitMCParticle) relRecoDigitMCParticle.clear();

  RelationArray relRecoDigitShaperDigit(storeRecoDigits, storeShaperDigits,
                                        m_relRecoDigitShaperDigitName);
  if (relRecoDigitShaperDigit) relRecoDigitShaperDigit.clear();

  RelationArray relRecoDigitTrueHit(storeRecoDigits, storeTrueHits,
                                    m_relRecoDigitTrueHitName);
  if (relRecoDigitTrueHit) relRecoDigitTrueHit.clear();

  //Build lookup tables for relations
  createRelationLookup(relShaperDigitMCParticle, m_mcRelation, nDigits);
  createRelationLookup(relShaperDigitTrueHit, m_trueRelation, nDigits);

  // Create fit tool object
  NNWaveFitTool fitTool = m_fitter.getFitTool();

  // I. Group digits by sensor/side.
  vector<pair<unsigned short, unsigned short> > sensorDigits;
  VxdID lastSensorID(0);
  size_t firstSensorDigit = 0;
  for (size_t iDigit = 0; iDigit < nDigits; ++iDigit) {
    const SVDShaperDigit& digit = *storeShaperDigits[iDigit];
    VxdID sensorID = digit.getSensorID();
    sensorID.setSegmentNumber(digit.isUStrip() ? 1 : 0);
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
    // Get VXDID and side from the first digit
    const SVDShaperDigit& exampleDigit = *storeShaperDigits[firstDigit];
    VxdID sensorID = exampleDigit.getSensorID();
    bool isU = exampleDigit.isUStrip();

    // 2. Cycle through digits and form recodigits on the way.

    B2DEBUG(300, "Reconstructing digits " << firstDigit << " to " << lastDigit);
    for (size_t iDigit = firstDigit; iDigit < lastDigit; ++iDigit) {

      const SVDShaperDigit& shaperDigit = *storeShaperDigits[iDigit];
      StripCalibrationMap::StripData stripData = m_stripMap->getStripData(sensorID, isU, shaperDigit.getCellID());
      bool validDigit = stripData.m_goodStrip;
      float stripGain = stripData.m_calPeak;
      float stripNoise = stripData.m_noise;
      float stripNoiseADU = stripNoise / stripGain;
      float stripSignalWidth = stripData.m_calWidth;
      float stripT0 = stripData.m_calTimeDelay;

      // If the strip is not masked away, normalize samples (sample/stripNoise)
      // FIXME: This is excessive once we've fixed the digitizer
      apvSamples normedSamples;
      if (validDigit) {
        auto samples = shaperDigit.getSamples();
        transform(samples.begin(), samples.end(), normedSamples.begin(),
                  bind2nd(divides<float>(), stripNoiseADU));
        validDigit = validDigit && pass3Samples(normedSamples, m_cutAdjacent);
      }

      if (validDigit) {
        zeroSuppress(normedSamples, m_cutAdjacent);
      } else // only now we give up on the diigit
        continue;

      // 3. Now we create and save the RecoDigit

      ostringstream os;
      os << "Input to NNFitter: iDigit = " << iDigit << endl << "Samples: ";
      copy(normedSamples.begin(), normedSamples.end(), ostream_iterator<double>(os, " "));
      os << endl;
      std::shared_ptr<nnFitterBinData> pStrip = m_fitter.getFit(normedSamples, stripSignalWidth);
      os << "Output from NNWaveFitter: " << endl;
      copy(pStrip->begin(), pStrip->end(), ostream_iterator<double>(os, " "));
      os << endl;
      // Apply strip time shift to pdf
      fitTool.shiftInTime(*pStrip, -stripT0);
      B2DEBUG(200, os.str());
      // Calculate time and its error, amplitude and its error, and chi2
      double stripTime, stripTimeError;
      tie(stripTime, stripTimeError) = fitTool.getTimeShift(*pStrip);
      // Now we have the cluster time pdf, so we can calculate amplitudes.
      double stripAmplitude, stripAmplitudeError, stripChi2;
      tie(stripAmplitude, stripAmplitudeError, stripChi2) =
        fitTool.getAmplitudeChi2(normedSamples, stripTime, stripSignalWidth);
      //De-normalize amplitudes and convert to electrons.
      stripAmplitude *= stripNoise;
      stripAmplitudeError *= stripNoise;
      B2DEBUG(200, "RecoDigit " << iDigit << " Noise: " << stripNoise
              << "Time: " << stripTime << " +/- " << stripTimeError
              << " Amplitude: " << stripAmplitude << " +/- " << stripAmplitudeError
              << " Chi2: " << stripChi2
             );

      // Finally, we save the RecoDigit and its relations.
      map<unsigned int, float> mc_relations;
      map<unsigned int, float> truehit_relations;
      vector<pair<unsigned int, float> > digit_weights;
      digit_weights.reserve(1);

      // Store relations to MCParticles and SVDTrueHits
      fillRelationMap(m_mcRelation, mc_relations, iDigit);
      fillRelationMap(m_trueRelation, truehit_relations, iDigit);
      //Add digit to the RecoDigit->ShaperDigit relation list
      digit_weights.emplace_back(iDigit, 1.0);

      //Store the RecoDigit into Datastore ...
      int recoDigitIndex = storeRecoDigits.getEntries();
      storeRecoDigits.appendNew(
        SVDRecoDigit(sensorID, isU, shaperDigit.getCellID(), stripAmplitude,
                     stripAmplitudeError, stripTime, stripTimeError, *pStrip, stripChi2,
                     shaperDigit.getModeByte())
      );

      //Create relations to the cluster
      if (!mc_relations.empty()) {
        relRecoDigitMCParticle.add(recoDigitIndex, mc_relations.begin(), mc_relations.end());
      }
      if (!truehit_relations.empty()) {
        relRecoDigitTrueHit.add(recoDigitIndex, truehit_relations.begin(), truehit_relations.end());
      }
      relRecoDigitShaperDigit.add(recoDigitIndex, digit_weights.begin(), digit_weights.end());
    } // CYCLE OVER SHAPERDIGITS

  } // CYCLE OVER SENSORS for items in sensorDigits

  B2DEBUG(100, "Number of RecoDigits: " << storeRecoDigits.getEntries());

} // event()


