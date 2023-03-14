/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/UdstListFilter/UdstListFilterModule.h>
// framework aux
#include <framework/logging/Logger.h>
#include <unordered_set>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(UdstListFilter)

/**
 *  KLM and ECL clusters to keep/remove
 */
void storeRelatedToTrack(const Track* p,
                         std::map < Particle::EParticleSourceObject, std::unordered_set<unsigned  int>>& indicesToKeep)
{
  for (const ECLCluster& cluster : p->getRelationsTo<ECLCluster>()) {
    indicesToKeep[Particle::EParticleSourceObject::c_ECLCluster].insert(cluster.getArrayIndex());
  };
  for (const KLMCluster& cluster : p->getRelationsTo<KLMCluster>()) {
    indicesToKeep[Particle::EParticleSourceObject::c_KLMCluster].insert(cluster.getArrayIndex());
  };
}


/**
 * Keep all information about the particle
 */
void keepObject(const Particle* p, std::map < Particle::EParticleSourceObject, std::unordered_set<unsigned  int>>& indicesToKeep)
{
  auto source = p->getParticleSource();
  if (source == Particle::EParticleSourceObject::c_Track ||
      source == Particle::EParticleSourceObject::c_KLMCluster ||
      source == Particle::EParticleSourceObject::c_ECLCluster ||
      source == Particle::EParticleSourceObject::c_V0) {
    unsigned mdstIndex = p->getMdstArrayIndex();
    indicesToKeep[source].insert(mdstIndex);
    // trace relations:
    if (source == Particle::EParticleSourceObject::c_Track) {
      storeRelatedToTrack(p->getTrack(), indicesToKeep);
    }
  }


  unsigned int n = p->getNDaughters();
  for (unsigned int i = 0; i < n; i++) {
    keepObject(p->getDaughter(i), indicesToKeep);
  }
}

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

UdstListFilterModule::UdstListFilterModule() : Module()
{
  setDescription("Filter udst file content based on a particle list. As a result of the module, all object which are (not) associated with the list are removed. The module is used for signal embedding.");
  addParam("listName", m_listName, "name of particle list.", std::string(""));
  addParam("keepNotInList", m_reverse,
           "if true, keep mdst objects which are not used in the particle list, otherwise keep objects which are used in the list.", false);
}

void UdstListFilterModule::initialize()
{
  m_plist.isRequired(m_listName);

  m_tracks.isRequired();
  m_track_selector.registerSubset(m_tracks, "myudst_tracks");
  m_track_selector.inheritAllRelations();

  m_eclclusters.isRequired();
  m_ecl_selector.registerSubset(m_eclclusters, DataStore::EStoreFlags::c_WriteOut);

  m_klmclusters.isRequired();
  m_klm_selector.registerSubset(m_klmclusters);

  m_v0s.isRequired();
  m_v0_selector.registerSubset(m_v0s);

  m_selectedV0s = new StoreArray<V0>("myudst_V0s");
  m_selectedV0s->registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);

}

void UdstListFilterModule::event()
{
  std::map < Particle::EParticleSourceObject, std::unordered_set<unsigned  int>> mdstIndices;
  // Select objects to keep/remove:
  for (auto& p : *m_plist) {
    keepObject(&p, mdstIndices);
  }

  for (const auto& mI : mdstIndices) {
    // for (const auto& [source, indices] : mdstIndices) {

    const auto& source  = mI.first;
    const auto& indices = mI.second;

    // Selector
    auto selector = [indices, this](const RelationsObject * p) -> bool {
      int idx = p->getArrayIndex();
      return (indices.count(idx) == 1)^ m_reverse;
    };

    // Do removal
    if (source == Particle::EParticleSourceObject::c_Track) {
      m_track_selector.select(selector);
      // Update V0s...
      // Also overwrite the original tracks:
      swapV0s();
      m_track_selector.swapSetsAndDestroyOriginal();
    }

    else if (source == Particle::EParticleSourceObject::c_V0) {
      // already dealt with
    }

    else if (source == Particle::EParticleSourceObject::c_ECLCluster) {
      m_ecl_selector.select(selector);
    }

    else if (source == Particle::EParticleSourceObject::c_KLMCluster) {
      m_klm_selector.select(selector);
    }
  }
}

void UdstListFilterModule::swapV0s()
{
  for (auto& v0 : m_v0s) {
    const auto tracks =  v0.getTracks();
    const auto fitResults = v0.getTrackFitResults();
    const Track* r1 = tracks.first->getRelated<Track>("myudst_tracks");
    const Track* r2 = tracks.second->getRelated<Track>("myudst_tracks");
    if ((r1 != nullptr) and (r2 != nullptr)) {
      // New V0 with new track relations:
      m_selectedV0s->appendNew(std::make_pair(r1, fitResults.first)
                               , std::make_pair(r2, fitResults.second));
    }
  }

  StoreEntry* fromEntry = DataStore::Instance().getEntry(*m_selectedV0s);
  StoreEntry* toEntry = DataStore::Instance().getEntry(m_v0s);
  if (!fromEntry->ptr) {
    if (toEntry->isArray)
      toEntry->getPtrAsArray()->Delete();   // This seems to be needed too.
    toEntry->ptr = nullptr;
  } else {
    DataStore::Instance().replaceData(*m_selectedV0s, m_v0s);
  }
}

