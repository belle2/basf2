#include <svd/modules/svdPerformance/SVDValidationModule.h>
#include <svd/persistenceManager/PersistenceManagerFactory.h>
#include <svd/variables/VariableFactory.h>

#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>


namespace Belle2::SVD {

  REG_MODULE(SVDValidation);

  SVDValidationModule::SVDValidationModule() : Module()
  {
    addParam("outputFileName", m_fileName, "", m_fileName);
    addParam("treeName", m_treeName, "", m_treeName);
    addParam("storageType", m_storageType, "Type of storage to store the variables in.", m_storageType);
    addParam("variables", m_variableNames, "Variables used for the valiadation plots", m_variableNames);
  }

  void SVDValidationModule::initialize()
  {
    persistenceManager = PersistenceManagerFactory::create(m_storageType);
    m_computableVariables = Variables::VariableFactory::create(m_variableNames);
    persistenceManager->initialize(m_fileName, m_treeName, m_computableVariables);
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