/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Peter Kvasnicka                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdReconstruction/SVDShaperDigitSorterModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/reconstruction/Sample.h>
#include <utility>
#include <vector>
#include <algorithm>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDShaperDigitSorter);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDShaperDigitSorterModule::SVDShaperDigitSorterModule() : Module()
{
  //
  //Set module properties
  setDescription("This module sorts an existing SVDShaperDigits collection and also "
                 "updates the corresponding relations to MCParticles and TrueHits. This is "
                 "needed for background overlay as the Clusterizer expects sorted input. "
                 "The module is otherwise not required for simulation or reconstruction, "
                 "as both SVDDigitzer ond SVDUnpacker output sorted digit collections.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("digits", m_storeShaperDigitsName, "SVDShaperDigit collection name", string(""));
  addParam("truehits", m_storeTrueHitsName, "SVDTrueHit collection name", string(""));
  addParam("particles", m_storeMCParticlesName, "MCParticle collection name", string(""));
  addParam("digitsToTrueHits", m_relShaperDigitTrueHitName, "ShaperDigits to TrueHit relation name",
           string(""));
  addParam("digitsToMCParticles", m_relShaperDigitMCParticleName, "ShaperDigits to MCParticles relation name",
           string(""));
}

void SVDShaperDigitSorterModule::initialize()
{
  //Register collections
  StoreArray<SVDShaperDigit> storeShaperDigits(m_storeShaperDigitsName);
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<SVDTrueHit> storeTrueHits(m_storeTrueHitsName);
  storeShaperDigits.isRequired();
  storeMCParticles.isOptional();
  storeTrueHits.isOptional();

  RelationArray relShaperDigitMCParticles(storeShaperDigits, storeMCParticles, m_relShaperDigitMCParticleName);
  RelationArray relShaperDigitTrueHits(storeShaperDigits, storeTrueHits, m_relShaperDigitTrueHitName);
  relShaperDigitMCParticles.isOptional();
  relShaperDigitTrueHits.isOptional();

  m_storeShaperDigitsName = storeShaperDigits.getName();
  m_storeTrueHitsName = storeTrueHits.getName();
  m_storeMCParticlesName = storeMCParticles.getName();

  m_relShaperDigitTrueHitName = relShaperDigitTrueHits.getName();
  m_relShaperDigitMCParticleName = relShaperDigitMCParticles.getName();

}

void SVDShaperDigitSorterModule::event()
{
  StoreArray<SVDShaperDigit> storeShaperDigits(m_storeShaperDigitsName);
  // If not digits, nothing to do
  if (!storeShaperDigits || !storeShaperDigits.getEntries()) return;
  B2DEBUG(89, "Initial size of StoreShaperDigits array: " << storeShaperDigits.getEntries());

  RelationArray relShaperDigitMCParticle(m_relShaperDigitMCParticleName);
  RelationArray relShaperDigitTrueHit(m_relShaperDigitTrueHitName);

  //List of mappings from old indices to new indices
  std::vector<RelationArray::consolidation_type> relationIndices(storeShaperDigits.getEntries());

  // Sort digits by channelID
  typedef std::pair<size_t, unsigned int> indexElement;

  std::vector<indexElement> digitIndex;

  const unsigned int nSamples = storeShaperDigits.getEntries();
  for (size_t i = 0; i < nSamples; i++)
    digitIndex.emplace_back(i, storeShaperDigits[i]->getUniqueChannelID());

  std::sort(digitIndex.begin(), digitIndex.end(),
            [](const indexElement & a, const indexElement & b)->bool { return (a.second < b.second);}
           );

  // Now we loop over sensors and reorder the digits list
  // To do this, we create a copy of the existing digits
  m_digitcopy.clear();
  m_digitcopy.insert(end(m_digitcopy), begin(storeShaperDigits), end(storeShaperDigits));

  for (size_t i = 0; i < nSamples; ++i) {
    *storeShaperDigits[i] = m_digitcopy[digitIndex[i].first];
    relationIndices[digitIndex[i].first] = std::make_pair(i, false);
  }

  // Finally we just need to reorder the RelationArrays
  RelationArray::ReplaceVec<> from(relationIndices);
  RelationArray::Identity to;
  if (relShaperDigitMCParticle) relShaperDigitMCParticle.consolidate(from, to, RelationArray::c_deleteElement);
  if (relShaperDigitTrueHit) relShaperDigitTrueHit.consolidate(from, to, RelationArray::c_deleteElement);
  B2DEBUG(89, "Final size of StoreShaperDigits store array: " << storeShaperDigits.getEntries());

}
