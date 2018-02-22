/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/PXDClusterFilterModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <set>
#include <algorithm>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDClusterFilter);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDClusterFilterModule::PXDClusterFilterModule() : Module()
{
  //Set module properties
  setDescription("This module filters PXDClusters StoreArray"
                 " and updates the corresponding relations to"
                 " MCParticles, PXDTrueHits and PXDDigits.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("clusters", m_storeClustersName, "PXDCluster collection name", string(""));
  addParam("digits", m_storeDigitsName, "PXDDigit collection name", string(""));
  addParam("truehits", m_storeTrueHitsName, "PXDTrueHit collection name", string(""));
  addParam("particles", m_storeMCParticlesName, "MCParticle collection name", string(""));
  addParam("minCharge", m_minCharge, "Minimum cluster charge to pass", double(0.0));
  addParam("minSize", m_minSize, "Minimum cluster size to pass", (unsigned int)(0));
}

void PXDClusterFilterModule::initialize()
{
  //Register collections
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<PXDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  storeClusters.isRequired();
  storeDigits.isRequired();
  storeMCParticles.isOptional();
  storeTrueHits.isOptional();

  RelationArray relClusterMCParticles(storeClusters, storeMCParticles, m_relClusterMCParticleName);
  RelationArray relClusterTrueHits(storeClusters, storeTrueHits, m_relClusterTrueHitName);
  RelationArray relClusterDigits(storeClusters, storeDigits, m_relClusterDigitName);
  relClusterDigits.isRequired();
  relClusterMCParticles.isOptional();
  relClusterTrueHits.isOptional();

  m_storeClustersName = storeClusters.getName();
  m_storeDigitsName = storeDigits.getName();
  m_storeTrueHitsName = storeTrueHits.getName();
  m_storeMCParticlesName = storeMCParticles.getName();

  m_relClusterDigitName = relClusterDigits.getName();
  m_relClusterTrueHitName = relClusterTrueHits.getName();
  m_relClusterMCParticleName = relClusterMCParticles.getName();
}

void PXDClusterFilterModule::event()
{
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);
  // If no clusters, nothing to do
  if (!storeClusters || !storeClusters.getEntries()) return;
  B2DEBUG(89, "Initial size of StoreClusters array: " << storeClusters.getEntries());

  RelationArray relClusterDigits(m_relClusterDigitName);
  RelationArray relClusterMCParticles(m_relClusterMCParticleName);
  RelationArray relClusterTrueHits(m_relClusterTrueHitName);

  //List of mappings from old indices to new indices
  std::vector<RelationArray::consolidation_type> relationIndices(storeClusters.getEntries());

  unsigned int index(0);
  // And just loop over the clusters and assign the clusters at the correct position
  for (unsigned int i = 0; i < storeClusters.getEntries(); ++i) {
    if (goodCluster(*storeClusters[i])) {
      if (i > index) //No need to copy on itself
        *storeClusters[index] = *storeClusters[i];
      relationIndices[i] = std::make_pair(index, false);
      ++index;
    } else {
      relationIndices[i] = std::make_pair(0, true);
    }
  }

  //Resize if we omitted one or more elements
  storeClusters.getPtr()->ExpandCreate(index);

  // Finally we just need to reorder the RelationArrays and delete elements
  // referencing clusters which were ignored.
  RelationArray::ReplaceVec<> from(relationIndices);
  RelationArray::Identity to;
  if (relClusterDigits) relClusterDigits.consolidate(from, to, RelationArray::c_deleteElement);
  if (relClusterMCParticles) relClusterMCParticles.consolidate(from, to, RelationArray::c_deleteElement);
  if (relClusterTrueHits) relClusterTrueHits.consolidate(from, to, RelationArray::c_deleteElement);
  B2DEBUG(89, "Final size of StoreClusters store array: " << storeClusters.getEntries());
}
