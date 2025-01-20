#include <analysis/VariableManager/Manager.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>

namespace {
  Belle2::RecoTrack* getRecoTrack(const Belle2::Particle* particle)
  {
    const Belle2::Track* track = particle->getTrack();
    if (!track)
      return nullptr;
    return track->getRelatedTo<Belle2::RecoTrack>();
  }
}

namespace Belle2::Variable {

  Manager::FunctionPtr SVDClusterCharge(const std::vector<std::string>& arguments)
  {
    if (arguments.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required for SVDClusterCharge.");
    }
    const auto clusterIndex = std::stoi(arguments[0]);

    B2INFO("clusterIndex" << clusterIndex);

    auto func = [clusterIndex](const Particle * particle) -> double {
      B2INFO("aaaaaa");

      RecoTrack* recoTrack = getRecoTrack(particle);
      if (!recoTrack)
      {
        B2INFO("bbbbbb");
        return Const::doubleNaN;
      }
      const std::vector<Belle2::SVDCluster*> svdClusters = recoTrack->getSVDHitList();
      if (clusterIndex >= svdClusters.size())
      {
        return Const::doubleNaN;
      }
      B2INFO("ccccccc");
      return svdClusters[clusterIndex]->getCharge();
    };
    return func;
  }

  VARIABLE_GROUP("Tracking Validation");

  REGISTER_METAVARIABLE("SVDClusterCharge(i)", SVDClusterCharge,
                        "Returns the charge of the i-th SVD cluster related to the Particle.",
                        Manager::VariableDataType::c_double);

}