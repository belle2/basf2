#include <svd/modules/svdPerformance/SVDValidationModule.h>
#include <svd/persistenceManager/PersistenceManagerFactory.h>
#include <svd/variables/VariableFactory.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <analysis/dataobjects/ParticleList.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <analysis/VariableManager/Manager.h>
namespace {
  namespace Variables = Belle2::SVD::Variables;

  Variables::Variables createVariables(std::vector<std::string> variablesToNtuple)
  {
    Variables::Variables variables;
    for (const auto& variableName : variablesToNtuple) {
      variables.push_back(Variables::TypedVariable(variableName, Variables::VariableDataType::c_double));
    }
    return variables;
  }

  Variables::Variables createVariables(std::vector<std::tuple<std::string, int, float, float>> variablesToHistogram)
  {
    Variables::Variables variables;
    for (const auto& [varName, nbins, minVal, maxVal] : variablesToHistogram) {
      B2INFO("I'm here");
      variables.push_back(Variables::BinnedVariable(varName, nbins, minVal, maxVal));
    }
    return variables;
  }
}

namespace Belle2::SVD {

  REG_MODULE(SVDValidation);

  SVDValidationModule::SVDValidationModule() : Module()
  {
    addParam("outputFileName", m_fileName, "", m_fileName);
    addParam("containerName", m_containerName, "", m_containerName);
    addParam("variablesToNtuple", m_variablesToNtuple, "Variables to store in the ntuple", m_variablesToNtuple);
    addParam("variablesToHistogram", m_variablesToHistogram, "Variables to store in the histogram", m_variablesToHistogram);
  }

  void SVDValidationModule::initialize()
  {
    Variables::Variables variables;
    if (not m_variablesToNtuple.empty() and not m_variablesToHistogram.empty()) {
      B2FATAL("Cannot have both variablesToNtuple and variablesToHistogram set.");
    } else if (not m_variablesToNtuple.empty()) {
      variables = createVariables(m_variablesToNtuple);
      persistenceManager = PersistenceManagerFactory::create("ntuple");
    } else if (not m_variablesToHistogram.empty()) {
      variables = createVariables(m_variablesToHistogram);
      persistenceManager = PersistenceManagerFactory::create("histogram");
    }
    persistenceManager->initialize(m_fileName, m_containerName, variables);
  }

  void SVDValidationModule::event()
  {
    StoreObjPtr<ParticleList> particlelist("pi+:all");
    const auto ncandidates = particlelist->getListSize();
    B2INFO("listSize: " << ncandidates);
    for (unsigned int iPart = 0; iPart < ncandidates; iPart++) {
      const Particle* particle = particlelist->getParticle(iPart);

      const auto trackFitResult = particle->getTrackFitResult();

      if (!trackFitResult) {
        continue;
      }

      const auto nSVDClusters = trackFitResult->getHitPatternVXD().getNSVDHits();
      B2INFO("clusters: " << nSVDClusters);

      for (unsigned int iCluster = 0; iCluster < nSVDClusters; iCluster++) {

        std::string varName = "SVDClusterCharge(" + std::to_string(iCluster) + ")";

        const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(varName);
        Belle2::SVD::Variables::ReturnType charge = var->function(particle);

        Variables::EvaluatedVariables evaluatedVariables{};
        evaluatedVariables["clusterCharge"] = charge;
        persistenceManager->addEntry(evaluatedVariables);
      }

    }

  }

  void SVDValidationModule::terminate()
  {
    persistenceManager->store();
  }

}