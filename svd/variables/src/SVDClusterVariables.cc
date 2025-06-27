/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>
#include <analysis/dataobjects/Particle.h>
#include <svd/variables/SVDClusterVariables.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <vxd/dataobjects/VxdID.h>

namespace {
  Belle2::RecoTrack* getRecoTrack(const Belle2::Particle* particle)
  {
    const Belle2::Track* track = particle->getTrack();
    if (!track)
      return nullptr;
    return track->getRelatedTo<Belle2::RecoTrack>();
  }

  Belle2::SVDCluster* getSVDCluster(const Belle2::RecoTrack* recoTrack, unsigned int clusterIndex)
  {
    if (!recoTrack) {
      return nullptr;
    }
    const std::vector<Belle2::SVDCluster*> svdClusters = recoTrack->getSVDHitList();
    if (clusterIndex >= svdClusters.size()) {
      return nullptr;
    }
    return svdClusters[clusterIndex];
  }

  Belle2::SVDCluster* getSVDCluster(const Belle2::Particle* particle, unsigned int clusterIndex)
  {
    const Belle2::RecoTrack* recoTrack = getRecoTrack(particle);
    return getSVDCluster(recoTrack, clusterIndex);
  }
}

namespace Belle2::Variable {

  double SVDClusterCharge(const Particle* particle, const std::vector<double>& indices)
  {
    if (!particle) {
      return Const::doubleNaN;
    }
    if (indices.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required.");
    }
    const auto clusterIndex = static_cast<unsigned int>(indices[0]);

    SVDCluster* svdCluster = getSVDCluster(particle, clusterIndex);
    return svdCluster ? svdCluster->getCharge() : Const::doubleNaN;
  }

  double SVDClusterSNR(const Particle* particle, const std::vector<double>& indices)
  {
    if (!particle) {
      return Const::doubleNaN;
    }

    if (indices.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required.");
    }
    const auto clusterIndex = static_cast<unsigned int>(indices[0]);

    SVDCluster* svdCluster = getSVDCluster(particle, clusterIndex);
    return svdCluster ? svdCluster->getSNR() : Const::doubleNaN;
  }

  int SVDClusterSize(const Particle* particle, const std::vector<double>& indices)
  {
    if (!particle) {
      return -1;
    }

    if (indices.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required.");
    }
    const auto clusterIndex = static_cast<unsigned int>(indices[0]);

    SVDCluster* svdCluster = getSVDCluster(particle, clusterIndex);
    return svdCluster ? svdCluster->getSize() : -1;
  }

  double SVDClusterTime(const Particle* particle, const std::vector<double>& indices)
  {
    if (!particle) {
      return Const::doubleNaN;
    }
    if (indices.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required.");
    }
    const auto clusterIndex = static_cast<unsigned int>(indices[0]);

    SVDCluster* svdCluster = getSVDCluster(particle, clusterIndex);
    return svdCluster ? svdCluster->getClsTime() : Const::doubleNaN;
  }

  double SVDTrackPrime(const Particle* particle, const std::vector<double>& indices)
  {
    if (!particle) {
      return Const::doubleNaN;
    }
    if (indices.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required.");
    }
    const auto clusterIndex = static_cast<unsigned int>(indices[0]);

    const RecoTrack* recoTrack = getRecoTrack(particle);
    if (!recoTrack) {
      return Const::doubleNaN;
    }
    const SVDCluster* svdCluster = getSVDCluster(recoTrack, clusterIndex);
    if (!svdCluster) {
      return Const::doubleNaN;
    }
    const RecoHitInformation* recoHitInformation = recoTrack->getRecoHitInformation(svdCluster);
    if (!recoHitInformation) {
      return Const::doubleNaN;
    }
    try {
      genfit::MeasuredStateOnPlane measuredState = recoTrack->getMeasuredStateOnPlaneFromRecoHit(recoHitInformation);
      return svdCluster->isUCluster()
             ? measuredState.getState()[1]
             : measuredState.getState()[2];
    } catch (const NoTrackFitResult&) {
      B2WARNING("No track fit result available for this hit!");
      return Const::doubleNaN;
    }
  }

  double SVDTrackPositionErrorUnbiased(const Particle* particle, const std::vector<double>& indices)
  {
    if (!particle) {
      return Const::doubleNaN;
    }
    if (indices.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required.");
    }
    const auto clusterIndex = static_cast<unsigned int>(indices[0]);

    const RecoTrack* recoTrack = getRecoTrack(particle);
    if (!recoTrack) {
      return Const::doubleNaN;
    }
    const SVDCluster* svdCluster = getSVDCluster(recoTrack, clusterIndex);
    if (!svdCluster) {
      return Const::doubleNaN;
    }
    const RecoHitInformation* recoHitInformation = recoTrack->getRecoHitInformation(svdCluster);
    if (!recoHitInformation) {
      return Const::doubleNaN;
    }
    const genfit::TrackPoint* trackPoint = recoTrack->getCreatedTrackPoint(recoHitInformation);
    if (!trackPoint) {
      return Const::doubleNaN;
    }
    const genfit::AbsFitterInfo* fitterInfo = trackPoint->getFitterInfo();
    if (!fitterInfo) {
      return Const::doubleNaN;
    }
    try {
      genfit::MeasuredStateOnPlane unbiasedState = fitterInfo->getFittedState(false);
      return svdCluster->isUCluster()
             ? sqrt(unbiasedState.getCov()[3][3])
             : sqrt(unbiasedState.getCov()[4][4]);
    } catch (...) {
      B2WARNING("Could not compute SVDTrackPositionErrorUnbiased.");
      return Const::doubleNaN;
    }
  }

  double SVDTruePosition(const Particle* particle, const std::vector<double>& indices)
  {
    if (!particle) {
      return Const::doubleNaN;
    }
    if (indices.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required.");
    }
    const auto clusterIndex = static_cast<unsigned int>(indices[0]);

    SVDCluster* svdCluster = getSVDCluster(particle, clusterIndex);
    if (!svdCluster) {
      return Const::doubleNaN;
    }

    const auto trueHit = svdCluster->getRelatedTo<Belle2::SVDTrueHit>();

    if (!trueHit) {
      return Const::doubleNaN;
    }
    return svdCluster->isUCluster()
           ? trueHit->getU()
           : trueHit->getV();
  }

  double SVDResidual(const Particle* particle, const std::vector<double>& indices)
  {
    if (!particle) {
      return Const::doubleNaN;
    }
    if (indices.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required.");
    }
    const auto clusterIndex = static_cast<unsigned int>(indices[0]);

    const RecoTrack* recoTrack = getRecoTrack(particle);
    if (!recoTrack) {
      return Const::doubleNaN;
    }
    const SVDCluster* svdCluster = getSVDCluster(recoTrack, clusterIndex);
    if (!svdCluster) {
      return Const::doubleNaN;
    }
    const RecoHitInformation* recoHitInformation = recoTrack->getRecoHitInformation(svdCluster);
    if (!recoHitInformation) {
      return Const::doubleNaN;
    }
    const genfit::TrackPoint* trackPoint = recoTrack->getCreatedTrackPoint(recoHitInformation);
    if (!trackPoint) {
      return Const::doubleNaN;
    }
    const genfit::AbsFitterInfo* fitterInfo = trackPoint->getFitterInfo();
    if (!fitterInfo) {
      return Const::doubleNaN;
    }
    try {
      const TVectorD residualMeasurement = fitterInfo->getResidual(0, false).getState();
      return residualMeasurement.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
    } catch (...) {
      B2WARNING("Could not get track residual.");
      return Const::doubleNaN;
    }
  }

  int SVDLayer(const Particle* particle, const std::vector<double>& indices)
  {
    if (!particle) {
      return -1;
    }
    if (indices.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required.");
    }
    const auto clusterIndex = static_cast<unsigned int>(indices[0]);

    SVDCluster* svdCluster = getSVDCluster(particle, clusterIndex);
    if (!svdCluster) {
      return -1;
    }
    const VxdID vxdId = svdCluster->getSensorID();
    return vxdId ? vxdId.getLayerNumber() : -1;
  }

  int SVDLadder(const Particle* particle, const std::vector<double>& indices)
  {
    if (!particle) {
      return -1;
    }
    if (indices.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required.");
    }
    const auto clusterIndex = static_cast<unsigned int>(indices[0]);

    SVDCluster* svdCluster = getSVDCluster(particle, clusterIndex);
    if (!svdCluster) {
      return -1;
    }
    const VxdID vxdId = svdCluster->getSensorID();
    return vxdId ? vxdId.getLadderNumber() : -1;
  }

  int SVDSensor(const Particle* particle, const std::vector<double>& indices)
  {
    if (!particle) {
      return -1;
    }
    if (indices.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required.");
    }
    const auto clusterIndex = static_cast<unsigned int>(indices[0]);

    SVDCluster* svdCluster = getSVDCluster(particle, clusterIndex);
    if (!svdCluster) {
      return -1;
    }
    const VxdID vxdId = svdCluster->getSensorID();
    return vxdId ? vxdId.getSensorNumber() : -1;
  }

  bool SVDSide(const Particle* particle, const std::vector<double>& indices)
  {
    if (!particle) {
      return false;
    }
    if (indices.size() != 1) {
      B2FATAL("Exactly one parameter (cluster index) is required.");
    }
    const auto clusterIndex = static_cast<unsigned int>(indices[0]);

    SVDCluster* svdCluster = getSVDCluster(particle, clusterIndex);
    return svdCluster ? svdCluster->isUCluster() : false;
  }

  VARIABLE_GROUP("SVD Validation");

  REGISTER_VARIABLE("SVDClusterCharge(i)", SVDClusterCharge,
                    "Returns the charge of the i-th SVD cluster related to the Particle.");
  REGISTER_VARIABLE("SVDClusterSNR(i)", SVDClusterSNR,
                    "Returns the SNR of the i-th SVD cluster related to the Particle.");
  REGISTER_VARIABLE("SVDClusterSize(i)", SVDClusterSize,
                    "Returns the size of the i-th SVD cluster related to the Particle.");
  REGISTER_VARIABLE("SVDClusterTime(i)", SVDClusterTime,
                    "Returns the time of the i-th SVD cluster related to the Particle.");
  REGISTER_VARIABLE("SVDTrackPrime(i)", SVDTrackPrime,
                    "Returns the tan of the incident angle projected on U/V of the i-th SVD cluster related to the Particle.");
  REGISTER_VARIABLE("SVDResidual(i)", SVDResidual,
                    "Returns the track residual of the i-th SVD cluster related to the Particle.");
  REGISTER_VARIABLE("SVDTrackPositionErrorUnbiased(i)", SVDTrackPositionErrorUnbiased,
                    "Returns the unbiased track position error of the i-th SVD cluster related to the Particle.");
  REGISTER_VARIABLE("SVDTruePosition(i)", SVDTruePosition,
                    "Returns the true position of the i-th SVD cluster related to the Particle.");
  REGISTER_VARIABLE("SVDLayer(i)", SVDLayer,
                    "Returns the layer number of the i-th SVD cluster related to the Particle. If no SVD cluster is found, returns -1.");
  REGISTER_VARIABLE("SVDLadder(i)", SVDLadder,
                    "Returns the ladder number of the i-th SVD cluster related to the Particle. If no SVD cluster is found, returns -1.");
  REGISTER_VARIABLE("SVDSensor(i)", SVDSensor,
                    "Returns the sensor number of the i-th SVD cluster related to the Particle. If no SVD cluster is found, returns -1.");
  REGISTER_VARIABLE("SVDSide(i)", SVDSide,
                    "Returns true if the i-th SVD cluster related to the Particle is a U cluster.");
}