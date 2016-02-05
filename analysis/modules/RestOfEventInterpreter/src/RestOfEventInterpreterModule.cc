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
    setDescription("Creates a mask (vector of boolean values) for tracks and clusters in RestOfEvent regarding their usage.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    std::vector<std::tuple<std::string, std::string, std::string>> emptyROEMask;
    std::vector<std::tuple<std::string, std::string, std::string, std::vector<double>>> emptyROEMaskWithFractions;

    addParam("particleList", m_particleList, "Name of the ParticleList");

    addParam("ROEMasks", m_ROEMasks,
             "List of (maskName, trackSelectionCut, eclClusterSelectionCut) tuples that specify all ROE masks of a specific particle to be created.",
             emptyROEMask);

    addParam("ROEMasksWithFractions", m_ROEMasksWithFractions,
             "List of (maskName, trackSelectionCut, eclClusterSelectionCut, ChargedStable fractions) tuples that specify all ROE masks of a specific particle to be created.",
             emptyROEMaskWithFractions);
  }

  void RestOfEventInterpreterModule::initialize()
  {
    // input
    StoreObjPtr<ParticleList>::required(m_particleList);
    StoreArray<Particle> particles;
    particles.isRequired();

    for (auto ROEMask : m_ROEMasks) {
      // parsing of the input tuple (maskName, trackSelectionCut, eclClusterSelectionCut, fractions)
      std::string maskName = get<0>(ROEMask);
      std::string trackSelection = get<1>(ROEMask);
      std::string eclClusterSelection = get<2>(ROEMask);
      std::vector<double> fractions = {0, 0, 1, 0, 0, 0};

      std::shared_ptr<Variable::Cut> trackCut = std::shared_ptr<Variable::Cut>(Variable::Cut::Compile(trackSelection));
      std::shared_ptr<Variable::Cut> eclClusterCut = std::shared_ptr<Variable::Cut>(Variable::Cut::Compile(eclClusterSelection));

      if (std::find(m_allMaskNames.begin(), m_allMaskNames.end(), maskName) != m_allMaskNames.end())
        B2FATAL("ROE mask with name \'" << maskName << "\' already exists!");

      m_allMaskNames.push_back(maskName);
      m_trackCuts[maskName] = trackCut;
      m_eclClusterCuts[maskName] = eclClusterCut;
      m_setOfFractions[maskName] = fractions;

      B2INFO("RestOfEventInterpreter added ROEMask with default fractions under name \'" << maskName << "\' with track cuts: " <<
             trackSelection << " and eclCluster cuts: " << eclClusterSelection);
    }

    for (auto ROEMask : m_ROEMasksWithFractions) {
      // parsing of the input tuple (maskName, trackSelectionCut, eclClusterSelectionCut, fractions)
      std::string maskName = get<0>(ROEMask);
      std::string trackSelection = get<1>(ROEMask);
      std::string eclClusterSelection = get<2>(ROEMask);
      std::vector<double> fractions = get<3>(ROEMask);

      std::shared_ptr<Variable::Cut> trackCut = std::shared_ptr<Variable::Cut>(Variable::Cut::Compile(trackSelection));
      std::shared_ptr<Variable::Cut> eclClusterCut = std::shared_ptr<Variable::Cut>(Variable::Cut::Compile(eclClusterSelection));

      if (std::find(m_allMaskNames.begin(), m_allMaskNames.end(), maskName) != m_allMaskNames.end())
        B2FATAL("ROE mask with name \'" << maskName << "\' already exists!");

      m_allMaskNames.push_back(maskName);
      m_trackCuts[maskName] = trackCut;
      m_eclClusterCuts[maskName] = eclClusterCut;
      m_setOfFractions[maskName] = fractions;

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

      std::map<std::string, std::map<unsigned int, bool>> trackMasks;
      std::map<std::string, std::map<unsigned int, bool>> eclClusterMasks;

      // Create track and cluster masks for all interpretations
      for (auto maskName : m_allMaskNames) {

        std::map<unsigned int, bool> trackMask;
        std::map<unsigned int, bool> eclClusterMask;

        // Create track masks
        std::vector<const Track*> roeTracks = roe->getTracks();
        for (unsigned i = 0; i < roeTracks.size(); i++) {
          const Track* track = roeTracks[i];
          const PIDLikelihood* pid = track->getRelatedTo<PIDLikelihood>();
          int chargedStablePDG;

          std::vector<double> tempVector = m_setOfFractions[maskName];
          const int n = Const::ChargedStable::c_SetSize;

          if (tempVector.size() == n) {
            double tempFractionsArray[n];
            for (unsigned int i = 0; i < n; i++)
              tempFractionsArray[i] = tempVector[i];
            chargedStablePDG = pid->getMostLikely(tempFractionsArray).getPDGCode();
          } else if (tempVector.size() == 1 and tempVector[0] == -1) {
            const MCParticle* mcp = track->getRelatedTo<MCParticle>();
            if (!mcp) {
              B2WARNING("No related MCParticle found! Default will be used.");
              chargedStablePDG = Const::pion.getPDGCode();
            } else
              chargedStablePDG = abs(mcp->getPDG());
          } else
            B2FATAL("Size of fractions vector not appropriate! Check the fractions in the ROEInterpreter with mask name: " << maskName);

          Particle p(track, Const::ChargedStable(chargedStablePDG));
          Particle* tempPart = &p;

          if (m_trackCuts[maskName]->check(tempPart))
            trackMask[track->getArrayIndex()] = true;
          else
            trackMask[track->getArrayIndex()] = false;
        }

        // Create cluster masks
        std::vector<const ECLCluster*> roeECLClusters = roe->getECLClusters();
        for (unsigned i = 0; i < roeECLClusters.size(); i++) {
          const ECLCluster* cluster = roeECLClusters[i];
          Particle p(cluster);
          Particle* tempPart = &p;
          if (m_eclClusterCuts[maskName]->check(tempPart))
            eclClusterMask[cluster->getArrayIndex()] = true;
          else
            eclClusterMask[cluster->getArrayIndex()] = false;
        }

        // Final check, just in case
        if (trackMask.size() != (unsigned) roe->getNTracks())
          B2FATAL("Track mask size does not have appropriate size!");
        if (eclClusterMask.size() != (unsigned) roe->getNECLClusters())
          B2FATAL("ECLCluster mask size does not have appropriate size!");

        trackMasks[maskName] = trackMask;
        eclClusterMasks[maskName] = eclClusterMask;
      }

      roe->appendTrackMasks(trackMasks);
      roe->appendECLClusterMasks(eclClusterMasks);
      roe->appendChargedStableFractionsSet(m_setOfFractions);
    }
  }
}
