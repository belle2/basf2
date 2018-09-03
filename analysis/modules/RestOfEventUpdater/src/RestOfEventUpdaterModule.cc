/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matic Lubej                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/RestOfEventUpdater/RestOfEventUpdaterModule.h>


#include <analysis/VariableManager/Variables.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h> //
#include <framework/dataobjects/EventMetaData.h> //

#include <framework/logging/Logger.h>

#include <iostream>
#include <utility>

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
    std::vector<double> defaultFractionsVector = {0, 0, 1, 0, 0, 0};

    addParam("particleList", m_inputListName, "Name of the ParticleList which contains information that will be used for updating");
    addParam("updateMasks", m_maskNamesForUpdating, "List of all mask names which will be updated", emptyMaskVector);
    addParam("cutString", m_selection, "Cut string which will be used for updating masks", emptyCutString);
    addParam("discard", m_discard,
             "Update the ROE mask by passing or discarding particles in the provided particle list, default is to pass", false);
    addParam("fractions", m_fractions, "A-priori fractions used to update (default: pion always, empty vector: no change)",
             defaultFractionsVector);
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
    Particle::EParticleType listType = getListType();

    // Apply cuts on input list
    std::vector<const Particle*> particlesToUpdate;
    for (unsigned j = 0; j < m_inputList->getListSize(); j++) {
      const Particle* partWithInfo = m_inputList->getParticle(j);
      if (m_cut->check(partWithInfo)) {
        particlesToUpdate.push_back(partWithInfo);
      }
    }
    if (listType != Particle::EParticleType::c_Composite) {
      updateMasksWithParticles(roe, particlesToUpdate, listType);
    }

    if (listType == Particle::EParticleType::c_Composite) {
      updateMasksWithV0(roe, particlesToUpdate);
    }
  }
  void RestOfEventUpdaterModule::updateMasksWithV0(StoreObjPtr<RestOfEvent> roe, std::vector<const Particle*>& particlesToUpdate)
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
  void RestOfEventUpdaterModule::updateMasksWithParticles(StoreObjPtr<RestOfEvent> roe,
                                                          std::vector<const Particle*>& particlesToUpdate, Particle::EParticleType listType)
  {
    for (auto& maskToUpdate : m_maskNamesForUpdating) {
      std::string maskNameToGetParticles = maskToUpdate;
      if (maskToUpdate == "") {
        B2FATAL("Cannot update ROE mask with no name!");
      }
      if (!roe->hasMask(maskToUpdate)) {
        // Change name to get all ROE particles in case of new mask
        maskNameToGetParticles = "";
        roe->initializeMask(maskToUpdate, "ROEUpdaterModule");
      }
      roe->excludeParticlesFromMask(maskToUpdate, particlesToUpdate, listType, m_discard);

    }
  }

  Particle::EParticleType RestOfEventUpdaterModule::getListType()
  {
    int pdgCode = m_inputList->getPDGCode();
    if (pdgCode == Const::pion.getPDGCode()) {
      return Particle::EParticleType::c_Track;
    }
    if (pdgCode == Const::photon.getPDGCode()) {
      return Particle::EParticleType::c_ECLCluster;
    }
    if (pdgCode == Const::Klong.getPDGCode()) {
      return Particle::EParticleType::c_KLMCluster;
    }
    //add converted photon support
    if (pdgCode == Const::Kshort.getPDGCode() or pdgCode == Const::Lambda.getPDGCode()) {
      return Particle::EParticleType::c_Composite;
    }
    B2WARNING("Unknown PDG code of particle list!");
    return Particle::EParticleType::c_Undefined;
  }


}


