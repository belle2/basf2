/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdDigitMerging/SVDDigitMergerModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <functional>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDigitMerger)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDigitMergerModule::SVDDigitMergerModule() : Module()
{
  //Set module properties
  setDescription("Merge SVDDigits into SVDShaperDigits");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Collections
  addParam("Digits", m_storeDigitsName,
           "SVDDigits collection name", string(""));
  addParam("ShaperDigits", m_storeShaperDigitsName,
           "SVDShaperDigits collection name", string(""));
  addParam("TrueHits", m_storeTrueHitsName,
           "TrueHit collection name", string(""));
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticles collection name", string(""));
}

void SVDDigitMergerModule::initialize()
{
  //Register collections
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  StoreArray<SVDShaperDigit> storeShaperDigits(m_storeShaperDigitsName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);

  storeDigits.isRequired();
  storeTrueHits.isOptional();
  storeMCParticles.isOptional();

  storeShaperDigits.registerInDataStore();

  RelationArray relDigitTrueHits(storeDigits, storeTrueHits);
  RelationArray relDigitMCParticles(storeDigits, storeMCParticles);
  RelationArray relShaperDigitTrueHits(storeShaperDigits, storeTrueHits);
  RelationArray relShaperDigitMCParticles(storeShaperDigits, storeMCParticles);
  RelationArray relShaperDigitDigits(storeShaperDigits, storeDigits);

  //Relations to simulation objects only if the ancestor relations exist
  if (relDigitTrueHits.isOptional())
    relShaperDigitTrueHits.registerInDataStore();
  if (relDigitMCParticles.isOptional())
    relShaperDigitMCParticles.registerInDataStore();
  relShaperDigitDigits.registerInDataStore();

  //Store names to speed up creation later
  m_storeDigitsName = storeDigits.getName();
  m_storeShaperDigitsName = storeShaperDigits.getName();
  m_storeTrueHitsName = storeTrueHits.getName();
  m_storeMCParticlesName = storeMCParticles.getName();

  m_relDigitTrueHitName = relDigitTrueHits.getName();
  m_relDigitMCParticleName = relDigitMCParticles.getName();
  m_relShaperDigitTrueHitName = relShaperDigitTrueHits.getName();
  m_relShaperDigitMCParticleName = relShaperDigitMCParticles.getName();
  m_relShaperDigitDigitName = relShaperDigitDigits.getName();

  B2INFO("SVD Digit Merger settings: ");
  B2INFO(" -->  MCParticles:   " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2INFO(" -->  Digits:        " << DataStore::arrayName<SVDDigit>(m_storeDigitsName));
  B2INFO(" -->  ShaperDigits:       " << DataStore::arrayName<SVDShaperDigit>(m_storeShaperDigitsName));
  B2INFO(" -->  TrueHits:      " << DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName));
  B2INFO(" -->  DigitMCRel:    " << m_relDigitMCParticleName);
  B2INFO(" -->  DigitTrueRel:  " << m_relDigitTrueHitName);
  B2INFO(" -->  ShaperDigitMCRel:   " << m_relShaperDigitMCParticleName);
  B2INFO(" -->  ShaperDigitTrueRel: " << m_relShaperDigitTrueHitName);
  B2INFO(" -->  ShaperDigitDigitRel:" << m_relShaperDigitDigitName);
}

void SVDDigitMergerModule::createRelationLookup(const RelationArray& relation,
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

void SVDDigitMergerModule::fillRelationMap(const RelationLookup& lookup,
                                           std::map<unsigned int, float>& relation, unsigned int index)
{
  //If the lookup table is not empty and the element is set
  if (!lookup.empty() && lookup[index]) {
    const RelationElement& element = *lookup[index];
    const unsigned int size = element.getSize();
    //Add all Relations to the map
    for (unsigned int i = 0; i < size; ++i) {
      //negative weights are from ignored particles, we don't like them and
      //thus ignore them. We replace rather than add weights, since the
      //relation content of digits on the same strip is the same.
      if (element.getWeight(i) < 0) continue;
      relation[element.getToIndex(i)] = element.getWeight(i);
    }
  }
}


void SVDDigitMergerModule::event()
{
  const StoreArray<SVDDigit> storeDigits(m_storeDigitsName);

  // If no digits, nothing to do
  if (!storeDigits || !storeDigits.getEntries()) return;
  size_t nDigits = storeDigits.getEntries();
  B2DEBUG(90, "Initial size of StoreDigits array: " << nDigits);

  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<SVDShaperDigit> storeShaperDigits(m_storeShaperDigitsName);

  storeShaperDigits.clear();

  RelationArray relDigitMCParticles(storeDigits, storeMCParticles,
                                    m_relDigitMCParticleName);
  RelationArray relDigitTrueHits(storeDigits, storeTrueHits,
                                 m_relDigitTrueHitName);

  RelationArray relShaperDigitMCParticles(storeShaperDigits, storeMCParticles,
                                          m_relShaperDigitMCParticleName);
  if (relShaperDigitMCParticles) relShaperDigitMCParticles.clear();

  RelationArray relShaperDigitTrueHits(storeShaperDigits, storeTrueHits,
                                       m_relShaperDigitTrueHitName);
  if (relShaperDigitTrueHits) relShaperDigitTrueHits.clear();

  RelationArray relShaperDigitDigits(storeShaperDigits, storeDigits, m_relShaperDigitDigitName);
  if (relShaperDigitDigits) relShaperDigitDigits.clear();

  //Build lookup tables for relations
  createRelationLookup(relDigitMCParticles, m_mcRelation, storeDigits.getEntries());
  createRelationLookup(relDigitTrueHits, m_trueRelation, storeDigits.getEntries());

  // We assume that all digits are already sorted, so we directly merge them
  // and write shaper digits as we go.
  VxdID lastSensorID;
  bool lastUSide(true);
  size_t lastStripNo(0);

  size_t nMerged(0);
  size_t nSamples(0);
  SVDShaperDigit::APVRawSamples samples({{0, 0, 0, 0, 0, 0}});

  map<unsigned int, float> mc_relations;
  map<unsigned int, float> truehit_relations;
  vector<pair<unsigned int, float> > digit_weights;

  for (size_t i = 0; i < nDigits; i++) {

    B2DEBUG(90, "Processing digit " << i);
    B2DEBUG(90, storeDigits[i]->toString());

    VxdID thisSensorID = storeDigits[i]->getSensorID();
    bool thisUSide = storeDigits[i]->isUStrip();
    size_t thisStripNo = storeDigits[i]->getCellID();
    size_t thisSampleNo = storeDigits[i]->getIndex();
    SVDShaperDigit::APVRawSampleType sample =
      static_cast<SVDShaperDigit::APVRawSampleType>(storeDigits[i]->getCharge());

    //Other strip or side or sensor, or last digit: save the 6-digit and start a new one.
    if ((lastStripNo != thisStripNo)
        || (lastUSide != thisUSide)
        || (lastSensorID != thisSensorID)
       ) {
      // Check ordering
      if ((lastStripNo > thisStripNo)
          && (lastUSide == thisUSide)
          && (lastSensorID == thisSensorID)
         ) {
        B2DEBUG(3, "last strip: " << lastStripNo << ", current strip: " << thisStripNo);
        B2FATAL("Digits are not sorted correctly. Use DigitSorter. ");
      } // ordering
      // Don't save if this is the first 1-digit.
      if (i > 0) {
        auto shaperDigit = storeShaperDigits.appendNew(
                             SVDShaperDigit(lastSensorID, lastUSide, lastStripNo, samples)
                           );
        // Set relations
        if (!mc_relations.empty()) relShaperDigitMCParticles.add(
            nMerged, mc_relations.begin(), mc_relations.end()
          );
        if (!truehit_relations.empty()) relShaperDigitTrueHits.add(
            nMerged, truehit_relations.begin(), truehit_relations.end()
          );
        relShaperDigitDigits.add(nMerged, digit_weights.begin(), digit_weights.end());
        B2DEBUG(90, "Saved ShaperDigit " << nMerged);
        B2DEBUG(90, shaperDigit->toString());
        nMerged++;
      } // save if i > 0
      // Reset samples
      samples.fill(0);
      nSamples = 0;
      // Reset the guards
      lastSensorID = thisSensorID;
      lastUSide = thisUSide;
      lastStripNo = thisStripNo;
      // Reset the relation maps
      mc_relations.clear();
      truehit_relations.clear();
      digit_weights.clear();
      digit_weights.reserve(SVDShaperDigit::c_nAPVSamples);
    } // save and reset for new strip

    samples[thisSampleNo] = sample;
    nSamples++;
    //Obtain relations from MCParticles and TrueHits
    fillRelationMap(m_mcRelation, mc_relations, i);
    fillRelationMap(m_trueRelation, truehit_relations, i);
    //Save digit charge as weight for the ShaperDigit->Digit relation
    digit_weights.emplace_back(i, sample);

    // If this is the last 1-digit, save
    if (i == nDigits - 1) {
      auto shaperDigit =
        storeShaperDigits.appendNew(
          SVDShaperDigit(lastSensorID, lastUSide, lastStripNo, samples)
        );
      // Set relations
      if (!mc_relations.empty()) relShaperDigitMCParticles.add(
          nMerged, mc_relations.begin(), mc_relations.end()
        );
      if (!truehit_relations.empty()) relShaperDigitTrueHits.add(
          nMerged, truehit_relations.begin(), truehit_relations.end()
        );
      relShaperDigitDigits.add(nMerged, digit_weights.begin(), digit_weights.end());
      B2DEBUG(90, "Saved 6-digit " << nMerged);
      B2DEBUG(90, shaperDigit->toString());
      nMerged++;
    }
  } // for i in 0 to nDigits

  B2DEBUG(90, "Processed " << nDigits << " digits, created : " << nMerged - 1 << " ShaperDigits.");
}


