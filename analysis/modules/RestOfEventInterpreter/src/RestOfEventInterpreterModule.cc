/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matic Lubej                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/RestOfEventInterpreter/RestOfEventInterpreterModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <analysis/VariableManager/Variables.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/logging/Logger.h>
#include <framework/core/ModuleParam.templateDetails.h>

#include <iostream>
#include <utility>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------
  REG_MODULE(RestOfEventInterpreter)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  RestOfEventInterpreterModule::RestOfEventInterpreterModule() : Module()
  {
    // Set module properties
    setDescription("Creates a mask (vector of boolean values) for tracks and clusters in RestOfEvent.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    std::vector<std::tuple<std::string, std::string, std::string, std::vector<double>>> emptyROEMaskWithFractions;

    addParam("particleList", m_particleList, "Name of the ParticleList");

    addParam("ROEMasksWithFractions", m_ROEMasksWithFractions,
             "List of (maskName, trackSelectionCut, eclClusterSelectionCut, ChargedStable fractions) tuples that specify all ROE masks of a specific particle to be created.",
             emptyROEMaskWithFractions);

    addParam("update", m_update, "Set true for updating a-priori charged stable fractions used in calculation of ROE 4-momentum",
             false);

  }

  void RestOfEventInterpreterModule::initialize()
  {
    // input
    StoreObjPtr<ParticleList>().isRequired(m_particleList);
    StoreArray<Particle> particles;
    particles.isRequired();

    for (auto ROEMask : m_ROEMasksWithFractions) {
      // parsing of the input tuple (maskName, trackSelectionCut, eclClusterSelectionCut, fractions)
      std::string maskName = get<0>(ROEMask);
      std::string trackSelection = get<1>(ROEMask);
      std::string eclClusterSelection = get<2>(ROEMask);
      std::vector<double> fractions = get<3>(ROEMask);

      std::shared_ptr<Variable::Cut> trackCut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(trackSelection));
      std::shared_ptr<Variable::Cut> eclClusterCut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(eclClusterSelection));

      m_maskNames.push_back(maskName);
      m_trackCuts.insert(stringAndCutMap::value_type(maskName, trackCut));
      m_eclClusterCuts.insert(stringAndCutMap::value_type(maskName, eclClusterCut));
      m_setOfFractions.insert(stringAndVectorMap::value_type(maskName, fractions));

      B2INFO("RestOfEventInterpreter added ROEMask with specific fractions under name \'" << maskName << "\' with track cuts: " <<
             trackSelection << " and eclCluster cuts: " << eclClusterSelection);
    }
  }

  void RestOfEventInterpreterModule::event()
  {
    // input Particle
    StoreObjPtr<ParticleList> plist(m_particleList);

    unsigned int nParts = plist->getListSize();

    for (unsigned i = 0; i < nParts; i++) {
      const Particle* particle = plist->getParticle(i);
      RestOfEvent* roe = particle->getRelated<RestOfEvent>();

      // Get vector of all pre-existing mask names
      std::vector<std::string> allMaskNames = roe->getMaskNames();

      stringAndMapOfIntAndBoolMap trackMasks;
      stringAndMapOfIntAndBoolMap eclClusterMasks;
      stringAndVectorMap subsetOfFractions;

      // Create track and cluster masks for all interpretations
      for (auto maskName : m_maskNames) {

        intAndBoolMap trackMask;
        intAndBoolMap eclClusterMask;

        // Create track masks
        std::vector<const Track*> roeTracks = roe->getTracks();
        std::vector<double> fractionsVector = m_setOfFractions.at(maskName);

        if (fractionsVector.empty())
          if (std::find(allMaskNames.begin(), allMaskNames.end(), maskName) != allMaskNames.end())
            fractionsVector = roe->getChargedStableFractions(maskName);

        const int n = Const::ChargedStable::c_SetSize;
        double fractionsArray[n];

        if (fractionsVector.size() == n) {
          for (unsigned int j = 0; j < n; j++)
            fractionsArray[j] = fractionsVector[j];
        }

        for (unsigned j = 0; j < roeTracks.size(); j++) {
          const Track* track = roeTracks[j];
          const PIDLikelihood* pid = track->getRelatedTo<PIDLikelihood>();
          int particlePDG = Const::pion.getPDGCode();

          if (fractionsVector.size() == n)
            // if normal fractions, use most likely
            pid->getMostLikely(fractionsArray).getPDGCode();
          else if (fractionsVector.size() == 1 and fractionsVector[0] == -1) {
            // if fractions = [-1] use MC mass hypothesis, if available
            const MCParticle* mcp = track->getRelatedTo<MCParticle>();
            if (mcp) {
              int mcpdg = abs(mcp->getPDG());
              if (Const::chargedStableSet.contains(Const::ParticleType(mcpdg))) {
                particlePDG = mcpdg;
              }
            }
          } else if (fractionsVector.size() == 1 and fractionsVector[0] == -2) {
            // Belle case: if fractions = [-2], use pion always for now when filling, later use basic PID info from Belle
            particlePDG = Const::pion.getPDGCode();
          } else
            B2FATAL("Size of fractions vector not appropriate! Check the fractions in the ROEInterpreter with mask name: " << maskName);

          // Skip tracks with charge 0
          Const::ChargedStable type(particlePDG);
          const TrackFitResult* tfr = track->getTrackFitResultWithClosestMass(type);

          if (!tfr)
            continue;

          int charge = tfr->getChargeSign();

          if (charge == 0) {
            B2WARNING("Track with charge=0, this track will always be ignored in ROE!");

            // If it's a new mask, set to zero, otherwise leave unchanged
            if (std::find(allMaskNames.begin(), allMaskNames.end(), maskName) == allMaskNames.end()) {
              std::pair<intAndBoolMap::iterator, bool> ret;
              ret = trackMask.insert(intAndBoolMap::value_type(track->getArrayIndex(), false));

              if (!ret.second)
                B2FATAL("Failed to add Track with ID '" << track->getArrayIndex() << "' to this Track mask! Something is wrong.");
            }
          } else {

            Particle p(track, type);
            Particle* tempPart = &p;

            // Is it a new mask?
            if (std::find(allMaskNames.begin(), allMaskNames.end(), maskName) == allMaskNames.end()) {
              std::pair<intAndBoolMap::iterator, bool> ret;
              if (m_trackCuts.at(maskName)->check(tempPart))
                ret = trackMask.insert(intAndBoolMap::value_type(track->getArrayIndex(), true));
              else
                ret = trackMask.insert(intAndBoolMap::value_type(track->getArrayIndex(), false));

              if (!ret.second)
                B2FATAL("Failed to add Track with ID '" << track->getArrayIndex() << "' to this Track mask! Something is wrong.");
            }
            // Or does it already exist?
            else if (std::find(allMaskNames.begin(), allMaskNames.end(), maskName) != allMaskNames.end()) {
              // If updating set to true OK, otherwise fatal
              if (m_update) {
                intAndBoolMap existingTrackMask = roe->getTrackMask(maskName);
                if (!m_trackCuts.at(maskName)->check(tempPart))
                  existingTrackMask.at(track->getArrayIndex()) = false;
                roe->updateTrackMask(maskName, existingTrackMask);
              } else
                B2FATAL("ROE mask with name \'" << maskName <<
                        "\' already exists! If you want to update with new cuts, use updateROEMask- python functions!");
            }
          }
        }

        // Create cluster masks
        std::vector<const ECLCluster*> roeECLClusters = roe->getECLClusters();
        for (unsigned j = 0; j < roeECLClusters.size(); j++) {
          const ECLCluster* cluster = roeECLClusters[j];

          Particle p(cluster);
          Particle* tempPart = &p;

          // Is it a new mask?
          if (std::find(allMaskNames.begin(), allMaskNames.end(), maskName) == allMaskNames.end()) {
            std::pair<intAndBoolMap::iterator, bool> ret;
            if (m_eclClusterCuts.at(maskName)->check(tempPart))
              ret = eclClusterMask.insert(intAndBoolMap::value_type(cluster->getArrayIndex(), true));
            else
              ret = eclClusterMask.insert(intAndBoolMap::value_type(cluster->getArrayIndex(), false));

            if (!ret.second)
              B2FATAL("Failed to add ECLCluster with ID '" << cluster->getArrayIndex() << "' to this ECLCluster mask! Something is wrong.");
          }
          // Or does it already exist?
          else if (std::find(allMaskNames.begin(), allMaskNames.end(), maskName) != allMaskNames.end()) {
            // If updating set to true OK, otherwise fatal
            if (m_update) {
              intAndBoolMap existingECLClusterMask = roe->getECLClusterMask(maskName);
              if (!m_eclClusterCuts.at(maskName)->check(tempPart))
                existingECLClusterMask.at(cluster->getArrayIndex()) = false;
              roe->updateECLClusterMask(maskName, existingECLClusterMask);
            } else
              B2FATAL("ROE mask with name \'" << maskName <<
                      "\' already exists! If you want to update with new cuts, use updateROE- python functions!");
          }
        }

        // If mask already exists, no need to append it again
        if (std::find(allMaskNames.begin(), allMaskNames.end(), maskName) != allMaskNames.end()) {
          if (!m_setOfFractions.at(maskName).empty())
            roe->updateChargedStableFractions(maskName, m_setOfFractions.at(maskName));
          continue;
        }

        // Final check, just in case
        if (trackMask.size() != (unsigned) roe->getNTracks())
          B2FATAL("Track mask size does not have appropriate size!");
        if (eclClusterMask.size() != (unsigned) roe->getNECLClusters())
          B2FATAL("ECLCluster mask size does not have appropriate size!");

        // Insert mask to map of masks
        std::pair<stringAndMapOfIntAndBoolMap::iterator, bool> retT;
        std::pair<stringAndMapOfIntAndBoolMap::iterator, bool> retE;
        std::pair<stringAndVectorMap::iterator, bool> retF;

        retT = trackMasks.insert(stringAndMapOfIntAndBoolMap::value_type(maskName, trackMask));
        retE = eclClusterMasks.insert(stringAndMapOfIntAndBoolMap::value_type(maskName, eclClusterMask));
        retF = subsetOfFractions.insert(stringAndVectorMap::value_type(maskName, m_setOfFractions.at(maskName)));

        if (!retT.second or !retE.second or !retF.second)
          B2ERROR("Something wrong with collecting ROE mask with name '" << maskName << "'!");
      }

      roe->appendTrackMasks(trackMasks);
      roe->appendECLClusterMasks(eclClusterMasks);
      roe->appendChargedStableFractionsSet(subsetOfFractions);
    }
  }
}
