/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdPerformance/SVDVariablesToStorageModule.h>
#include <reconstruction/persistenceManager/PersistenceManagerFactory.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <analysis/dataobjects/ParticleList.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <analysis/VariableManager/Manager.h>
namespace {
  namespace VPM = Belle2::VariablePersistenceManager;

  std::string getIndexedVariableName(const std::string& variableName, unsigned int iCluster)
  {
    return variableName + "(" + std::to_string(iCluster) + ")";
  }

  VPM::Variables createVariablesToStore(const std::vector<std::string>& variablesToNtuple)
  {
    VPM::Variables variables;
    for (const auto& variableName : variablesToNtuple) {
      const auto variableDataType = static_cast<VPM::VariableDataType>(Belle2::Variable::Manager::Instance().getVariable(
                                      getIndexedVariableName(variableName, 0))->variabletype);
      variables.push_back(VPM::TypedVariable(variableName, variableDataType));
    }
    return variables;
  }

  VPM::Variables createVariablesToStore(const std::vector<std::tuple<std::string, int, float, float>>& variablesToHistogram)
  {
    VPM::Variables variables;
    for (const auto& [varName, nbins, minVal, maxVal] : variablesToHistogram) {
      variables.push_back(VPM::BinnedVariable(varName, nbins, minVal, maxVal));
    }
    return variables;
  }

  std::vector<std::string> extractVariableNames(const std::vector<std::tuple<std::string, int, float, float>>& variablesToHistogram)
  {
    std::vector<std::string> variableNames;
    std::transform(variablesToHistogram.begin(), variablesToHistogram.end(), std::back_inserter(variableNames),
    [](const auto & variable) {
      return std::get<0>(variable);
    });
    return variableNames;
  }
}

namespace Belle2::SVD {

  namespace VPM = Belle2::VariablePersistenceManager;

  /** Register the module. */
  REG_MODULE(SVDVariablesToStorage);

  SVDVariablesToStorageModule::SVDVariablesToStorageModule() : Module()
  {
    addParam("outputFileName", m_fileName, "", m_fileName);
    addParam("containerName", m_containerName, "", m_containerName);
    addParam("particleListName", m_particleListName, "", m_particleListName);
    addParam("variablesToNtuple", m_variablesToNtuple, "Variables to store in the ntuple.", m_variablesToNtuple);
    addParam("variablesToHistogram", m_variablesToHistogram, "Variables to store in the histogram.", m_variablesToHistogram);
  }

  void SVDVariablesToStorageModule::initialize()
  {
    VPM::Variables variablesToStore;
    if (not m_variablesToNtuple.empty() and not m_variablesToHistogram.empty()) {
      B2FATAL("Cannot have both variablesToNtuple and variablesToHistogram set.");
    } else if (not m_variablesToNtuple.empty()) {
      m_variableNames = m_variablesToNtuple;
      variablesToStore = createVariablesToStore(m_variablesToNtuple);
      persistenceManager = VPM::PersistenceManagerFactory::create("ntuple");
    } else if (not m_variablesToHistogram.empty()) {
      m_variableNames = extractVariableNames(m_variablesToHistogram);
      variablesToStore = createVariablesToStore(m_variablesToHistogram);
      persistenceManager = VPM::PersistenceManagerFactory::create("histogram");
    }
    persistenceManager->initialize(m_fileName, m_containerName, variablesToStore);
  }

  void SVDVariablesToStorageModule::event()
  {
    StoreObjPtr<ParticleList> particlelist(m_particleListName);
    const auto ncandidates = particlelist->getListSize();
    for (unsigned int iPart = 0; iPart < ncandidates; iPart++) {
      const Particle* particle = particlelist->getParticle(iPart);

      const auto trackFitResult = particle->getTrackFitResult();

      if (!trackFitResult) {
        continue;
      }

      const auto nSVDClusters = trackFitResult->getHitPatternVXD().getNSVDHits();

      for (unsigned int iCluster = 0; iCluster < nSVDClusters; iCluster++) {

        VPM::EvaluatedVariables evaluatedVariables{};
        for (const auto& variableName : m_variableNames) {
          const std::string indexedVariableName = getIndexedVariableName(variableName, iCluster);
          const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(indexedVariableName);
          const auto value = var->function(particle);
          evaluatedVariables[variableName] = value;
        }
        persistenceManager->addEntry(evaluatedVariables);
      }
    }
  }

  void SVDVariablesToStorageModule::terminate()
  {
    persistenceManager->store();
  }

}