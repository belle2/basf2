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
#include <analysis/dataobjects/RestOfEvent.h>

#include <analysis/dataobjects/Particle.h>

#include <analysis/VariableManager/Variables.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/StoreArray.h>

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
    std::vector<double> defaultFractionsVector;

    for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {
      if (i == 2)
        defaultFractionsVector.push_back(1);
      else
        defaultFractionsVector.push_back(0);
    }

    addParam("particleList", m_inputListName, "Name of the ParticleList which contains information that will be used for updating");
    addParam("updateMasks", m_maskNamesForUpdating, "List of all mask names which will be updated", emptyMaskVector);
    addParam("cutString", m_selection, "Cut string which will be used for updating masks", emptyCutString);
    addParam("fractions", m_fractions,
             "A priori fractions used for newly added masks. Fractions of existing masks will not be changed (default: pion always)",
             defaultFractionsVector);
  }

  void RestOfEventUpdaterModule::initialize()
  {
    StoreArray<Particle>::required();
    m_inputList.isRequired(m_inputListName);

    m_cut = Variable::Cut::Compile(m_selection);

    B2INFO("RestOfEventUpdater updated track/eclCluster ROEMask(s) with infoList: " << m_inputListName << " and cut: " << m_selection);

  }

  void RestOfEventUpdaterModule::event()
  {
    if (!m_inputList) {
      B2WARNING("Input list " << m_inputList.getName() << " was not created?");
      return;
    }

    StoreObjPtr<RestOfEvent> roe("RestOfEvent");

    if (roe.isValid()) {

      std::vector<std::string> allMaskNames = roe->getMaskNames();

      for (auto& roeMask : m_maskNamesForUpdating) {

        // Check if mask exists. If not, create a default mask.
        if (std::find(allMaskNames.begin(), allMaskNames.end(), roeMask) == allMaskNames.end()) {

          intAndBoolMap newTrackMask;
          intAndBoolMap newECLClusterMask;

          std::vector<const Track*> roeAllTracks = roe->getTracks();
          std::vector<const ECLCluster*> roeAllECLClusters = roe->getECLClusters();

          // Fill track mask
          for (unsigned i = 0; i < roeAllTracks.size(); i++) {
            std::pair<intAndBoolMap::iterator, bool> ret;
            ret = newTrackMask.insert(intAndBoolMap::value_type(roeAllTracks[i]->getArrayIndex(), true));

            if (!ret.second)
              B2FATAL("Failed to add Track with ID '" << roeAllTracks[i]->getArrayIndex() << "' to this Track mask! Something is wrong.");
          }

          // Fill eclCluster mask
          for (unsigned i = 0; i < roeAllECLClusters.size(); i++) {
            std::pair<intAndBoolMap::iterator, bool> ret;
            ret = newECLClusterMask.insert(intAndBoolMap::value_type(roeAllECLClusters[i]->getArrayIndex(), true));

            if (!ret.second)
              B2FATAL("Failed to add ECLCluster with ID '" << roeAllECLClusters[i]->getArrayIndex() <<
                      "' to this ECLCluster mask! Something is wrong.");
          }

          // Append new mask (update function can update old keys or add new ones)
          roe->updateTrackMask(roeMask, newTrackMask);
          roe->updateECLClusterMask(roeMask, newECLClusterMask);
          roe->updateChargedStableFractions(roeMask, m_fractions);
        }

        std::map<unsigned int, bool> trackMask = roe->getTrackMask(roeMask);
        std::map<unsigned int, bool> eclClusterMask = roe->getECLClusterMask(roeMask);
        std::vector<const Track*> roeTracks = roe->getTracks(roeMask);
        std::vector<const ECLCluster*> roeECLClusters = roe->getECLClusters(roeMask);

        // Update existing eclCluster or track mask
        if (abs(m_inputList->getPDGCode()) == Const::pion.getPDGCode()) {
          intAndIntMap trackIDAndParticlePositionMap;
          for (unsigned j = 0; j < m_inputList->getListSize(); j++) {
            const Particle* partWithInfo = m_inputList->getParticle(j);
            const Track* track = partWithInfo->getTrack();
            trackIDAndParticlePositionMap.insert(intAndIntMap::value_type(track->getArrayIndex(), j));
          }
          for (unsigned j = 0; j < roeTracks.size(); j++) {
            unsigned int trackID = roeTracks[j]->getArrayIndex();

            // Is this ROE track in the input particle list? If yes, check new cut. If not, set to false.
            if (trackIDAndParticlePositionMap.find(trackID) != trackIDAndParticlePositionMap.end()) {
              const Particle* partWithInfo = m_inputList->getParticle(trackIDAndParticlePositionMap.at(trackID));
              bool passNewCut = m_cut->check(partWithInfo);
              if (trackMask.at(trackID) == true && !passNewCut)
                trackMask[trackID] = false;
            } else
              trackMask[trackID] = false;
          }
          roe->updateTrackMask(roeMask, trackMask);
        } else if (abs(m_inputList->getPDGCode()) == Const::photon.getPDGCode()) {
          intAndIntMap eclClusterIDAndParticlePositionMap;
          for (unsigned j = 0; j < m_inputList->getListSize(); j++) {
            const Particle* partWithInfo = m_inputList->getParticle(j);
            const ECLCluster* ecl = partWithInfo->getECLCluster();
            eclClusterIDAndParticlePositionMap.insert(intAndIntMap::value_type(ecl->getArrayIndex(), j));
          }
          for (unsigned j = 0; j < roeECLClusters.size(); j++) {
            unsigned int eclID = roeECLClusters[j]->getArrayIndex();

            // Is this ROE ECLCluster in the input particle list? If yes, check new cut. If not, set to false.
            if (eclClusterIDAndParticlePositionMap.find(eclID) != eclClusterIDAndParticlePositionMap.end()) {
              const Particle* partWithInfo = m_inputList->getParticle(eclClusterIDAndParticlePositionMap.at(eclID));
              bool passNewCut = m_cut->check(partWithInfo);
              if (eclClusterMask.at(eclID) == true && !passNewCut)
                eclClusterMask[eclID] = false;
            } else
              eclClusterMask[eclID] = false;
          }
          roe->updateECLClusterMask(roeMask, eclClusterMask);
        } else
          B2FATAL("ParticleList with PDG: " << m_inputList->getPDGCode() <<
                  " was inserted. Only pion/photon infoList for tracks/eclClusters possible!");

      }
    }
  }
}


