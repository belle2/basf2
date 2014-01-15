/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/vxd/modules/telReconstruction/TelDigitSorterModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/MCParticle.h>
#include <testbeam/vxd/dataobjects/TelTrueHit.h>
#include <testbeam/vxd/reconstruction/Pixel.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::TEL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TelDigitSorter);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TelDigitSorterModule::TelDigitSorterModule() : Module()
{
  //Set module properties
  setDescription("This module sorts the existing TelDigits collection and also "
                 "updates the corresponding Relation to MCParticles and TrueHits. This is "
                 "needed for unsorted pixel data as the Clusterizer expects sorted input.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("digits", m_storeDigitsName, "TelDigit collection name", string(""));
  addParam("truehits", m_storeTrueHitsName, "TelTrueHit collection name", string(""));
  addParam("particles", m_storeMCParticlesName, "MCParticle collection name", string(""));
  addParam("digitsToTrueHits", m_relDigitTrueHitName, "Digits to TrueHit relation name",
           string(""));
  addParam("digitsToMCParticles", m_relDigitMCParticleName, "Digits to MCParticles relation name",
           string(""));
}

void TelDigitSorterModule::initialize()
{
  //Register collections
  StoreArray<TelDigit> storeDigits(m_storeDigitsName);
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<TelTrueHit> storeTrueHits(m_storeTrueHitsName);
  storeDigits.required();
  storeMCParticles.isOptional();
  storeTrueHits.isOptional();

  RelationArray relDigitMCParticles(storeDigits, storeMCParticles, m_relDigitMCParticleName);
  RelationArray relDigitTrueHits(storeDigits, storeTrueHits, m_relDigitTrueHitName);
  relDigitMCParticles.isOptional();
  relDigitTrueHits.isOptional();

  m_storeDigitsName = storeDigits.getName();
  m_storeTrueHitsName = storeTrueHits.getName();
  m_storeMCParticlesName = storeMCParticles.getName();

  m_relDigitTrueHitName = relDigitTrueHits.getName();
  m_relDigitMCParticleName = relDigitMCParticles.getName();
}

void TelDigitSorterModule::event()
{
  StoreArray<TelDigit> storeDigits(m_storeDigitsName);
  // If not digits, nothing to do
  if (!storeDigits || !storeDigits.getEntries()) return;

  RelationArray relDigitMCParticle(m_relDigitMCParticleName);
  RelationArray relDigitTrueHit(m_relDigitTrueHitName);

  //List of mappings from old indices to new indices
  std::vector<RelationArray::consolidation_type> relationIndices(storeDigits.getEntries());
  //Mapping of Pixel information to sort according to VxdID, row, column
  std::map<VxdID, std::multiset<Pixel>> sensors;

  // Fill sensor information to get sorted TelDigit indices
  const int nPixels = storeDigits.getEntries();
  for (int i = 0; i < nPixels; i++) {
    const TelDigit* const digit = storeDigits[i];
    Pixel px(digit, i);
    VxdID sensorID = digit->getSensorID();
    sensors[sensorID].insert(px);
  }

  // Now we loop over sensors and reorder the digits list
  // To do this, we create a copy of the existing digits
  m_digitcopy.clear();
  m_digitcopy.insert(end(m_digitcopy), begin(storeDigits), end(storeDigits));
  // and a vector to remember which index changed into what
  unsigned int index(0);
  // And just loop over the sensors and assign the digits at the correct position
  for (const auto & sensor : sensors) {
    const TEL::Pixel* lastpx(0);
    for (const TEL::Pixel & px : sensor.second) {
      //Normal case: pixel has different address
      if (!lastpx || px > *lastpx) {
        //Overwrite the digit
        *storeDigits[index] = m_digitcopy[px.getIndex()];
        //Remember old and new index
        relationIndices[px.getIndex()] = std::make_pair(index, false);
        //increment next index
        ++index;
      } else {
        //We already have a pixel at this address, drop the new one
        relationIndices[px.getIndex()] = std::make_pair(0, true);
      }
      lastpx = &px;
    }
  }
  //Resize if we omitted/merged one or more elements
  storeDigits.getPtr()->ExpandCreate(index);

  // Finally we just need to reorder the RelationArrays and delete elements
  // referencing pixels which were ignored because that adress was set more
  // than once
  RelationArray::ReplaceVec<> from(relationIndices);
  RelationArray::Identity to;
  if (relDigitMCParticle) relDigitMCParticle.consolidate(from, to, RelationArray::c_deleteElement);
  if (relDigitTrueHit) relDigitTrueHit.consolidate(from, to, RelationArray::c_deleteElement);
}
