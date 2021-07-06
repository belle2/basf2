/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/RestOfEventUpdater/RestOfEventUpdaterModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h> //

#include <framework/logging/Logger.h>

#include <iostream>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------
  REG_MODULE(RestOfEventUpdater)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  RestOfEventUpdaterModule::RestOfEventUpdaterModule() : Module()
  {
    // Set module properties
    setDescription("Updates an existing mask (map of boolean values) for tracks or eclClusters in RestOfEvent with an available property (e.g. after performing training).");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    std::vector<std::string> emptyMaskVector;
    std::string emptyCutString;

    addParam("particleList", m_inputListName, "Name of the ParticleList which contains information that will be used for updating");
    addParam("updateMasks", m_maskNamesForUpdating, "List of all mask names which will be updated", emptyMaskVector);
    addParam("cutString", m_selection, "Cut string which will be used for updating masks", emptyCutString);
    addParam("discard", m_discard,
             "Update the ROE mask by passing or discarding particles in the provided particle list, default is to pass", false);
  }

  void RestOfEventUpdaterModule::initialize()
  {
    StoreArray<Particle>().isRequired();
    m_inputList.isRequired(m_inputListName);

    m_cut = Variable::Cut::compile(m_selection);

    B2INFO("RestOfEventUpdater updated track/eclCluster ROEMask(s) with infoList: " << m_inputListName << " and cut: " << m_selection);
  }

  void RestOfEventUpdaterModule::event()
  {
    if (!m_inputList) {
      B2WARNING("Input list " << m_inputList.getName() << " was not created?");
      return;
    }
    StoreObjPtr<RestOfEvent> roe("RestOfEvent");
    if (!roe.isValid()) {
      B2WARNING("ROE list is not valid somehow, ROE masks are not updated!");
      return;
    }
    std::set<Particle::EParticleSourceObject> encounteredSources;
    // Particle lists can contain Particles from different mdst sources
    // Thus, we split them based on their mdst source
    // Only particles surviving the provided cut are considered
    std::vector<const Particle*> particlesFromTracksToUpdate;
    std::vector<const Particle*> particlesFromECLClustersToUpdate;
    std::vector<const Particle*> particlesFromKLMClustersToUpdate;
    std::vector<const Particle*> compositeParticlesToUpdate;
    for (unsigned j = 0; j < m_inputList->getListSize(); j++) {
      const Particle* partWithInfo = m_inputList->getParticle(j);
      Particle::EParticleSourceObject mdstSource = partWithInfo->getParticleSource();
      encounteredSources.insert(mdstSource);
      if (m_cut->check(partWithInfo)) {
        if (mdstSource == Particle::EParticleSourceObject::c_Track) {
          particlesFromTracksToUpdate.push_back(partWithInfo);
        } else if (mdstSource == Particle::EParticleSourceObject::c_ECLCluster) {
          particlesFromECLClustersToUpdate.push_back(partWithInfo);
        } else if (mdstSource == Particle::EParticleSourceObject::c_KLMCluster) {
          particlesFromKLMClustersToUpdate.push_back(partWithInfo);
        } else if (mdstSource == Particle::EParticleSourceObject::c_Composite or
                   mdstSource == Particle::EParticleSourceObject::c_V0) {
          compositeParticlesToUpdate.push_back(partWithInfo);
        }
      }
    }
    if (encounteredSources.count(Particle::EParticleSourceObject::c_Track) > 0) {
      updateMasksWithParticles(roe, particlesFromTracksToUpdate, Particle::EParticleSourceObject::c_Track);
    } else { // If we have a track-based particle in the particle list there can not be any other mdst source
      if (encounteredSources.count(Particle::EParticleSourceObject::c_ECLCluster) > 0) {
        updateMasksWithParticles(roe, particlesFromECLClustersToUpdate, Particle::EParticleSourceObject::c_ECLCluster);
      }
      if (encounteredSources.count(Particle::EParticleSourceObject::c_KLMCluster) > 0) {
        updateMasksWithParticles(roe, particlesFromKLMClustersToUpdate, Particle::EParticleSourceObject::c_KLMCluster);
      }
      updateMasksWithV0(roe, compositeParticlesToUpdate); // in updateMasksWithV0 it is checked whether the vector is empty
    }
  }

  void RestOfEventUpdaterModule::updateMasksWithV0(const StoreObjPtr<RestOfEvent>& roe,
                                                   std::vector<const Particle*>& particlesToUpdate)
  {
    if (particlesToUpdate.size() == 0) {
      B2DEBUG(10, "No particles in list provided, nothing to do");
      return;
    }
    for (auto& maskToUpdate : m_maskNamesForUpdating) {
      if (maskToUpdate == "") {
        B2FATAL("Cannot update ROE mask with no name!");
      }
      for (auto* particleV0 : particlesToUpdate) {
        if (!roe->checkCompatibilityOfMaskAndV0(maskToUpdate, particleV0)) {
          continue;
        }
        roe->updateMaskWithV0(maskToUpdate, particleV0);
      }
    }
  }

  void RestOfEventUpdaterModule::updateMasksWithParticles(const StoreObjPtr<RestOfEvent>& roe,
                                                          std::vector<const Particle*>& particlesToUpdate, Particle::EParticleSourceObject listType)
  {
    for (auto& maskToUpdate : m_maskNamesForUpdating) {
      if (maskToUpdate == "") {
        B2FATAL("Cannot update ROE mask with no name!");
      }
      if (!roe->hasMask(maskToUpdate)) {
        // Change name to get all ROE particles in case of new mask
        roe->initializeMask(maskToUpdate, "ROEUpdaterModule");
      }
      roe->excludeParticlesFromMask(maskToUpdate, particlesToUpdate, listType, m_discard);

    }
  }
}
