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

    if (roe.isValid()) {

      std::vector<std::string> allMaskNames = roe->getMaskNames();

      std::vector<const Track*> roeAllTracks = roe->getTracks();
      std::vector<const ECLCluster*> roeAllECLClusters = roe->getECLClusters();

      for (auto& roeMask : m_maskNamesForUpdating) {
        // Check if mask exists. If not, create a default mask with all true.
        if (std::find(allMaskNames.begin(), allMaskNames.end(), roeMask) == allMaskNames.end()) {

          intAndBoolMap newTrackMask;
          intAndBoolMap newECLClusterMask;

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
          if (m_fractions.empty()) {
            std::vector<double> defaultFractionsVector = {0, 0, 1, 0, 0, 0};
            roe->updateChargedStableFractions(roeMask, defaultFractionsVector);
          } else
            roe->updateChargedStableFractions(roeMask, m_fractions);
        }

        std::map<unsigned int, bool> trackMask = roe->getTrackMask(roeMask);
        std::map<unsigned int, bool> eclClusterMask = roe->getECLClusterMask(roeMask);
        std::vector<const Track*> roeTracks = roe->getTracks(roeMask);
        std::vector<const ECLCluster*> roeECLClusters = roe->getECLClusters(roeMask);

        double pdgList = abs(m_inputList->getPDGCode());

        // Update a-priori chargedStable fractions
        if (!m_fractions.empty())
          roe->updateChargedStableFractions(roeMask, m_fractions);

        // Update track mask
        if (pdgList == Const::pion.getPDGCode()) {
          intAndIntMap trackIDAndParticlePositionMap;
          for (unsigned j = 0; j < m_inputList->getListSize(); j++) {
            const Particle* partWithInfo = m_inputList->getParticle(j);
            const Track* track = partWithInfo->getTrack();
            trackIDAndParticlePositionMap.insert(intAndIntMap::value_type(track->getArrayIndex(), j));
          }
          for (unsigned j = 0; j < roeTracks.size(); j++) {
            unsigned int trackID = roeTracks[j]->getArrayIndex();

            if (!m_discard) { // Pass selection
              // Is this track in the input particle list? If yes, check new cut
              if (trackIDAndParticlePositionMap.find(trackID) != trackIDAndParticlePositionMap.end()) {
                const Particle* partWithInfo = m_inputList->getParticle(trackIDAndParticlePositionMap.at(trackID));
                bool passNewCut = m_cut->check(partWithInfo);
                if (trackMask.at(trackID) == true && !passNewCut)
                  trackMask[trackID] = false;
              }
              // If not in list, set to false
              else
                trackMask[trackID] = false;
            }

            else { // Discard selection
              // Is this track in the input particle list? If yes, check new cut. If not in list, leave as is.
              if (trackIDAndParticlePositionMap.find(trackID) != trackIDAndParticlePositionMap.end()) {
                const Particle* partWithInfo = m_inputList->getParticle(trackIDAndParticlePositionMap.at(trackID));
                bool passNewCut = m_cut->check(partWithInfo);
                if (trackMask.at(trackID) == true && passNewCut)
                  trackMask[trackID] = false;
              }
            }
          }

          roe->updateTrackMask(roeMask, trackMask);

        }

        // Update eclCluster mask
        else if (pdgList == Const::photon.getPDGCode()) {
          intAndIntMap eclClusterIDAndParticlePositionMap;
          for (unsigned j = 0; j < m_inputList->getListSize(); j++) {
            const Particle* partWithInfo = m_inputList->getParticle(j);
            const ECLCluster* ecl = partWithInfo->getECLCluster();
            eclClusterIDAndParticlePositionMap.insert(intAndIntMap::value_type(ecl->getArrayIndex(), j));
          }
          for (unsigned j = 0; j < roeECLClusters.size(); j++) {
            unsigned int eclID = roeECLClusters[j]->getArrayIndex();

            if (!m_discard) { // Pass selection
              // Is this track in the input particle list? If yes, check new cut
              if (eclClusterIDAndParticlePositionMap.find(eclID) != eclClusterIDAndParticlePositionMap.end()) {
                const Particle* partWithInfo = m_inputList->getParticle(eclClusterIDAndParticlePositionMap.at(eclID));
                bool passNewCut = m_cut->check(partWithInfo);
                if (eclClusterMask.at(eclID) == true && !passNewCut)
                  eclClusterMask[eclID] = false;
              }
              // If not in list, set to false
              else
                eclClusterMask[eclID] = false;
            }

            else { // Discard selection
              // Is this track in the input particle list? If yes, check new cut. If not in list, leave as is.
              if (eclClusterIDAndParticlePositionMap.find(eclID) != eclClusterIDAndParticlePositionMap.end()) {
                const Particle* partWithInfo = m_inputList->getParticle(eclClusterIDAndParticlePositionMap.at(eclID));
                bool passNewCut = m_cut->check(partWithInfo);
                if (eclClusterMask.at(eclID) == true && passNewCut)
                  eclClusterMask[eclID] = false;
              }
            }
          }

          roe->updateECLClusterMask(roeMask, eclClusterMask);
        }

        // Update track mask with info from V0 list, V0 list should be sorted from best to worst candidate
        // TODO: add support for lambda and converted photons
        else if (pdgList == Const::Kshort.getPDGCode() or pdgList == Const::Lambda.getPDGCode() or pdgList == Const::photon.getPDGCode()) {

          TLorentzVector track4momentum;

          std::vector<unsigned int> v0ID;
          std::vector<unsigned int> tracksFromV0;
          for (unsigned j = 0; j < m_inputList->getListSize(); j++) {
            const Particle* v0part = m_inputList->getParticle(j);
            const Particle* d0 = v0part->getDaughter(0);
            const Particle* d1 = v0part->getDaughter(1);

            // Check if composite particles
            if (v0part->getParticleType() != Particle::c_Composite)
              B2FATAL("Current V0 particle with pdg code: " << v0part->getPDGCode() << "is not a composite particle!");

            // Check if V0 already in list
            std::vector<unsigned int> usedV0ID = roe->getV0IDList(roeMask);
            if (std::find(usedV0ID.begin(), usedV0ID.end(), v0part->getArrayIndex()) != usedV0ID.end())
              continue;

            // Check if V0 passes selection cut
            bool passNewCut = m_cut->check(v0part);
            if (!passNewCut)
              continue;

            // Check if V0 daughter tracks are in ROE
            bool track0InROE = std::find(roeTracks.begin(), roeTracks.end(), d0->getTrack()) != roeTracks.end();
            bool track1InROE = std::find(roeTracks.begin(), roeTracks.end(), d1->getTrack()) != roeTracks.end();

            if (!track0InROE or !track1InROE) {
              B2WARNING("V0 particle not in ROE! Skipping particle!");
              continue;
            }

            // Check if V0 daughter tracks pass current ROE mask
            bool track0MaskTrue = trackMask.at(d0->getTrack()->getArrayIndex());
            bool track1MaskTrue = trackMask.at(d1->getTrack()->getArrayIndex());

            if (!track0MaskTrue or !track1MaskTrue)
              continue;

            // Check if V0 daughter tracks already taken into account, skip candidate if yes
            bool track0InList = std::find(tracksFromV0.begin(), tracksFromV0.end(), d0->getTrack()->getArrayIndex()) != tracksFromV0.end();
            bool track1InList = std::find(tracksFromV0.begin(), tracksFromV0.end(), d1->getTrack()->getArrayIndex()) != tracksFromV0.end();

            if (track0InList or track1InList)
              continue;

            v0ID.push_back(v0part->getArrayIndex());
            tracksFromV0.push_back(d0->getTrack()->getArrayIndex());
            tracksFromV0.push_back(d1->getTrack()->getArrayIndex());
          }

          for (unsigned j = 0; j < roeTracks.size(); j++) {
            unsigned int trackID = roeTracks[j]->getArrayIndex();

            // Is this ROE track used in V0? If yes, discard the track from the mask
            if (std::find(tracksFromV0.begin(), tracksFromV0.end(), trackID) != tracksFromV0.end())
              trackMask[trackID] = false;
          }

          roe->appendV0IDList(roeMask, v0ID);
          roe->updateTrackMask(roeMask, trackMask);

        } else
          B2FATAL("ParticleList with PDG: " << m_inputList->getPDGCode() <<
                  " was inserted. Only pion/photon/V0 particle list for tracks/eclClusters possible!");
      }
    }
  }
}


