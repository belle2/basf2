#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Manager.h>

namespace {
  Belle2::RecoTrack* getRecoTrack(const Belle2::Particle* particle)
  {
    const Belle2::Track* track = particle->getTrack();
    if (!track)
      return nullptr;
    return track->getRelatedTo<Belle2::RecoTrack>();
  }

  Belle2::SVDCluster* getSVDCluster(const Belle2::Particle* particle, unsigned int clusterIndex)
  {
    const Belle2::RecoTrack* recoTrack = getRecoTrack(particle);
    if (!recoTrack) {
      return nullptr;
    }

    const std::vector<Belle2::SVDCluster*> svdClusters = recoTrack->getSVDHitList();
    if (clusterIndex >= svdClusters.size()) {
      return nullptr;
    }
    return svdClusters[clusterIndex];
  }
}

namespace Belle2::Variable {

  Manager::FunctionPtr SVDClusterCharge(const std::vector<std::string>& arguments)
  {
    if (arguments.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required.");
    }
    const auto clusterIndex = std::stoi(arguments[0]);

    return [clusterIndex](const Particle * particle) -> double {
      SVDCluster* svdCluster = getSVDCluster(particle, clusterIndex);
      return svdCluster ? svdCluster->getCharge() : Const::doubleNaN;
    };
  }

  Manager::FunctionPtr SVDClusterSNR(const std::vector<std::string>& arguments)
  {
    if (arguments.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required.");
    }
    const auto clusterIndex = std::stoi(arguments[0]);

    return [clusterIndex](const Particle * particle) -> double {
      SVDCluster* svdCluster = getSVDCluster(particle, clusterIndex);
      return svdCluster ? svdCluster->getSNR() : Const::doubleNaN;
    };
  }

  Manager::FunctionPtr SVDClusterSize(const std::vector<std::string>& arguments)
  {
    if (arguments.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required.");
    }
    const auto clusterIndex = std::stoi(arguments[0]);

    return [clusterIndex](const Particle * particle) -> int {
      SVDCluster* svdCluster = getSVDCluster(particle, clusterIndex);
      return svdCluster ? svdCluster->getSize() : -1;
    };
  }

  VARIABLE_GROUP("SVD Validation");

  REGISTER_METAVARIABLE("SVDClusterCharge(i)", SVDClusterCharge,
                        "Returns the charge of the i-th SVD cluster related to the Particle.",
                        Manager::VariableDataType::c_double);

  REGISTER_METAVARIABLE("SVDClusterSNR(i)", SVDClusterSNR,
                        "Returns the SNR of the i-th SVD cluster related to the Particle.",
                        Manager::VariableDataType::c_double);

  REGISTER_METAVARIABLE("SVDClusterSize(i)", SVDClusterSize,
                        "Returns the size of the i-th SVD cluster related to the Particle.",
                        Manager::VariableDataType::c_int);
}