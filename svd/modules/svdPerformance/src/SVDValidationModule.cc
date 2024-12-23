#include <svd/modules/svdPerformance/SVDValidationModule.h>
#include <svd/persistenceManager/PersistenceManagerFactory.h>
#include <svd/variables/VariableFactory.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>


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

    // Po staremu
    addParam("variables", m_variableNames, "Variables used for the valiadation plots", m_variableNames);

    // Po nowemu
    addParam("variablesToNtuple", m_variablesToNtuple, "Variables to store in the ntuple", m_variablesToNtuple);
    addParam("variablesToHistogram", m_variablesToHistogram, "Variables to store in the histogram", m_variablesToHistogram);
  }

  void SVDValidationModule::initialize()
  {
    // To bedzie do wywalenia ale na razie potrzebne
    m_computableVariables = Variables::VariableFactory::create(m_variableNames);

    if (not m_variablesToNtuple.empty() and not m_variablesToHistogram.empty()) {
      B2FATAL("Cannot have both variablesToNtuple and variablesToHistogram set");
    } else if (not m_variablesToNtuple.empty()) {
      m_variables = createVariables(m_variablesToNtuple);
      persistenceManager = PersistenceManagerFactory::create("ntuple");
    } else if (not m_variablesToHistogram.empty()) {
      B2INFO("I'm here");
      m_variables = createVariables(m_variablesToHistogram);
      persistenceManager = PersistenceManagerFactory::create("histogram");
    }
    persistenceManager->initialize(m_fileName, m_containerName, m_variables);
  }

  void SVDValidationModule::event()
  {
    // Can be moved as a class field
    StoreArray<RecoTrack> recoTracks{m_recoTracksStoreArrayName};

    for (const auto& recoTrack : recoTracks) {
      if (not recoTrack.wasFitSuccessful()) {
        continue;
      }
      // Do we need a vector? consider getRelationWith...
      RelationVector<Track> track = DataStore::getRelationsWithObj<Track>(&recoTrack);
      if (0 == track.size()) {
        continue;
      }
      const TrackFitResult* trackFitResult = track[0]->getTrackFitResultWithClosestMass(Const::pion);

      if (trackFitResult) {
        // Calculate track related stuff
      }

      const std::vector<Belle2::SVDCluster*> svdClusters = recoTrack.getSVDHitList();

      for (const auto& svdCluster : svdClusters) {
        Variables::EvaluatedVariables evaluatedVariables{};

        for (const auto& computableVariable : m_computableVariables) {
          evaluatedVariables[computableVariable.getName()] = computableVariable(svdCluster);
        }
        persistenceManager->addEntry(evaluatedVariables);
      }

    }
  }

  void SVDValidationModule::terminate()
  {
    persistenceManager->store();
  }

}