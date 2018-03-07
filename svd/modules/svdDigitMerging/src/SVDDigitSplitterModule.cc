/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdDigitMerging/SVDDigitSplitterModule.h>

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
#include <functional>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDDigitSplitter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDDigitSplitterModule::SVDDigitSplitterModule() : Module()
{
  //Set module properties
  setDescription("Split SVDShaperDigits into SVDDigits");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("Digits", m_storeDigitsName,
           "SVDDigits collection name", string(""));
  addParam("ShaperDigits", m_storeShaperDigitsName,
           "SVDShaperDigits collection name", string(""));
  addParam("TrueHits", m_storeTrueHitsName,
           "TrueHit collection name", string(""));
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticles collection name", string(""));
}

void SVDDigitSplitterModule::initialize()
{
  //Register collections
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);
  StoreArray<SVDShaperDigit> storeShaperDigits(m_storeShaperDigitsName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);

  storeShaperDigits.isRequired();
  storeTrueHits.isOptional();
  storeMCParticles.isOptional();

  storeDigits.registerInDataStore();

  // Required relations
  RelationArray relShaperDigitTrueHits(storeShaperDigits, storeTrueHits);
  RelationArray relShaperDigitMCParticles(storeShaperDigits, storeMCParticles);
  // New relations
  RelationArray relShaperDigitDigits(storeShaperDigits, storeDigits);
  RelationArray relDigitTrueHits(storeDigits, storeTrueHits);
  RelationArray relDigitMCParticles(storeDigits, storeMCParticles);

  //Relations to simulation objects only if the ancestor relations exist
  if (relShaperDigitTrueHits.isOptional())
    relDigitTrueHits.registerInDataStore();
  if (relShaperDigitMCParticles.isOptional())
    relDigitMCParticles.registerInDataStore();
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

  B2INFO("SVD Digit Splitter settings: ");
  B2INFO(" -->  MCParticles:   " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2INFO(" -->  ShaperDigits:       " << DataStore::arrayName<SVDShaperDigit>(m_storeShaperDigitsName));
  B2INFO(" -->  TrueHits:      " << DataStore::arrayName<SVDTrueHit>(m_storeTrueHitsName));
  B2INFO(" -->  ShaperDigitMCRel:   " << m_relShaperDigitMCParticleName);
  B2INFO(" -->  ShaperDigitTrueRel: " << m_relShaperDigitTrueHitName);
  B2INFO(" -->  ShaperDigitDigitRel:" << m_relShaperDigitDigitName);
  B2INFO(" -->  Digits:        " << DataStore::arrayName<SVDDigit>(m_storeDigitsName));
  B2INFO(" -->  DigitMCRel:    " << m_relDigitMCParticleName);
  B2INFO(" -->  DigitTrueRel:  " << m_relDigitTrueHitName);
}

void SVDDigitSplitterModule::createRelationLookup(const RelationArray& relation,
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

void SVDDigitSplitterModule::fillRelationMap(const RelationLookup& lookup,
                                             std::map<unsigned int, float>& relation, unsigned int index)
{
  //If the lookup table is not empty and the element is set
  if (!lookup.empty() && lookup[index]) {
    const RelationElement& element = *lookup[index];
    const unsigned int size = element.getSize();
    //Add all Relations to the map
    for (unsigned int i = 0; i < size; ++i) {
      //negative weights are from ignored particles, we don't like them and
      //thus ignore them. We replace weights rather than add, since the
      //relation content of SVDDigits from the same strip is the same.
      if (element.getWeight(i) < 0) continue;
      relation[element.getToIndex(i)] = element.getWeight(i);
    }
  }
}


void SVDDigitSplitterModule::event()
{
  const StoreArray<SVDShaperDigit> storeShaperDigits(m_storeShaperDigitsName);

  // If no digits, nothing to do
  if (!storeShaperDigits || !storeShaperDigits.getEntries()) return;
  size_t nShaperDigits = storeShaperDigits.getEntries();
  B2DEBUG(90, "Initial size of StoreShaperDigits array: " << nShaperDigits);

  const StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  const StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<SVDDigit> storeDigits(m_storeDigitsName);

  storeDigits.clear();

  RelationArray relShaperDigitMCParticles(storeShaperDigits, storeMCParticles,
                                          m_relShaperDigitMCParticleName);

  RelationArray relShaperDigitTrueHits(storeShaperDigits, storeTrueHits,
                                       m_relShaperDigitTrueHitName);

  RelationArray relShaperDigitDigits(storeShaperDigits, storeDigits, m_relShaperDigitDigitName);
  if (relShaperDigitDigits) relShaperDigitDigits.clear();

  RelationArray relDigitMCParticles(storeDigits, storeMCParticles,
                                    m_relDigitMCParticleName);
  if (relDigitMCParticles) relDigitMCParticles.clear();
  RelationArray relDigitTrueHits(storeDigits, storeTrueHits,
                                 m_relDigitTrueHitName);
  if (relDigitTrueHits) relDigitTrueHits.clear();

  //Build lookup tables for relations
  createRelationLookup(relShaperDigitMCParticles, m_mcRelation, nShaperDigits);
  createRelationLookup(relShaperDigitTrueHits, m_trueRelation, nShaperDigits);

  size_t nDigits(0);

  for (const SVDShaperDigit& shaperDigit : storeShaperDigits) {

    size_t sixIndex = shaperDigit.getArrayIndex();
    B2DEBUG(90, "Processing shaper digit " << sixIndex);
    B2DEBUG(90, shaperDigit.toString());

    VxdID sensorID = shaperDigit.getSensorID();
    bool uSide = shaperDigit.isUStrip();
    size_t stripNo = shaperDigit.getCellID();
    float position = 0;
    const SVDShaperDigit::APVFloatSamples& samples = shaperDigit.getSamples();
    //Obtain relations from MCParticles and TrueHits
    map<unsigned int, float> mc_relations;
    map<unsigned int, float> truehit_relations;
    fillRelationMap(m_mcRelation, mc_relations, sixIndex);
    fillRelationMap(m_trueRelation, truehit_relations, sixIndex);
    vector<pair<unsigned int, float> > digit_weights;
    digit_weights.reserve(SVDShaperDigit::c_nAPVSamples);

    // Now save a 1-digit for each sample
    for (size_t iSample = 0; iSample < SVDShaperDigit::c_nAPVSamples; ++iSample) {
      auto onedigit = storeDigits.appendNew(
                        SVDDigit(sensorID, uSide, stripNo, position, samples[iSample], iSample)
                      );
      // Store relation data to the 1-digit
      digit_weights.emplace_back(nDigits, samples[iSample]);
      // Store 1-digit's realtions to MCParticles and SVDTrueHits
      if (!mc_relations.empty()) relDigitMCParticles.add(
          nDigits, mc_relations.begin(), mc_relations.end()
        );
      if (!truehit_relations.empty()) relDigitTrueHits.add(
          nDigits, truehit_relations.begin(), truehit_relations.end()
        );
      B2DEBUG(90, "Saved digit " << nDigits);
      B2DEBUG(90, onedigit->toString());
      nDigits++;
    } // for positive samples
    // Add 6-digit -> 1-digit relation
    relShaperDigitDigits.add(sixIndex, digit_weights.begin(), digit_weights.end());
  } // for 6-digits

  B2DEBUG(90, "Processed " << nShaperDigits << " shaper digits, created : " << nDigits << " 1-digits.");
}


