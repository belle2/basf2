/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/RestOfEventInterpreter/RestOfEventInterpreterModule.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>

#include <framework/logging/Logger.h>
#include <framework/core/ModuleParam.templateDetails.h>

#include <iostream>

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
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> emptyROEMask;

    addParam("particleList", m_particleListName, "Name of the ParticleList");

    addParam("ROEMasks", m_ROEMasks,
             "List of (maskName, trackSelectionCut, eclClusterSelectionCut) tuples that specify all ROE masks of a specific particle to be created.",
             emptyROEMask);

    addParam("update", m_update, "Set true for updating a-priori charged stable fractions used in calculation of ROE 4-momentum",
             false);

  }

  void RestOfEventInterpreterModule::initialize()
  {
    // input
    m_plist.isRequired(m_particleListName);

    for (auto ROEMask : m_ROEMasks) {
      // parsing of the input tuple (maskName, trackSelectionCut, eclClusterSelectionCut, fractions)

      std::string maskName = get<0>(ROEMask);
      std::string trackSelection = get<1>(ROEMask);
      std::string eclClusterSelection = get<2>(ROEMask);
      std::string klmClusterSelection = get<3>(ROEMask);

      std::shared_ptr<Variable::Cut> trackCut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(trackSelection));
      std::shared_ptr<Variable::Cut> eclClusterCut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(eclClusterSelection));
      std::shared_ptr<Variable::Cut> klmClusterCut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(klmClusterSelection));

      m_maskNames.push_back(maskName);
      m_trackCuts.insert(stringAndCutMap::value_type(maskName, trackCut));
      m_eclClusterCuts.insert(stringAndCutMap::value_type(maskName, eclClusterCut));
      m_klmClusterCuts.insert(stringAndCutMap::value_type(maskName, klmClusterCut));

      B2INFO("RestOfEventInterpreter added ROEMask with specific fractions under name \'" << maskName << "\' with track cuts: " <<
             trackSelection << " and eclCluster cuts: " << eclClusterSelection);
    }
  }

  void RestOfEventInterpreterModule::event()
  {
    unsigned int nParts = m_plist->getListSize();

    for (unsigned i = 0; i < nParts; i++) {
      const Particle* particle = m_plist->getParticle(i);
      auto* roe = particle->getRelatedTo<RestOfEvent>("ALL");
      for (auto& maskName : m_maskNames) {
        if (!m_update) {
          roe->initializeMask(maskName, "ROEInterpreterModule");
        }
        roe->updateMaskWithCuts(maskName, m_trackCuts.at(maskName), m_eclClusterCuts.at(maskName), m_klmClusterCuts.at(maskName), m_update);
      }
    }
  }

}
