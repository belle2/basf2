/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matic Lubej, Sviatoslav Bilokin                          *
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
    StoreObjPtr<ParticleList> plist(m_particleList);

    unsigned int nParts = plist->getListSize();

    for (unsigned i = 0; i < nParts; i++) {
      const Particle* particle = plist->getParticle(i);
      RestOfEvent* roe = particle->getRelated<RestOfEvent>();
      for (auto& maskName : m_maskNames) {
        if (!m_update) {
          roe->initializeMask(maskName, "ROEInterpreterModule");
        }
        roe->updateMaskWithCuts(maskName, m_trackCuts.at(maskName), m_eclClusterCuts.at(maskName), nullptr, m_update);
        roe->updateChargedStableFractions(maskName, m_setOfFractions.at(maskName));
      }
    }
  }

}
