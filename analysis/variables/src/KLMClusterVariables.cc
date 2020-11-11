/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Torben Ferber, Giacomo De Pietro            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <analysis/variables/KLMClusterVariables.h>

/* Analysis headers. */
#include <analysis/dataobjects/Particle.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/VariableManager/Manager.h>

/* Belle 2 headers. */
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KlId.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

using namespace std;

namespace Belle2::Variable {

  double klmClusterKlId(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    const KlId* klid = cluster->getRelatedTo<KlId>();
    if (!klid) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return klid->getKlId();
  }

  int klmClusterBelleTrackFlag(const Particle* particle)
  {
    const float angle = 0.24;
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<int>::quiet_NaN();
    }
    const TVector3& pos = cluster->getClusterPosition();
    StoreArray<TrackFitResult> tracks;
    for (const TrackFitResult& track : tracks) {
      const TVector3& trackPos = track.getPosition();
      if (trackPos.Angle(pos) < angle) {
        return 1;
      }
    }
    return 0;
  }

  int klmClusterBelleECLFlag(const Particle* particle)
  {
    const float angle = 0.24;
    const KLMCluster* klmCluster = particle->getKLMCluster();
    if (!klmCluster) {
      return std::numeric_limits<int>::quiet_NaN();
    }
    const TVector3& klmClusterPos = klmCluster->getClusterPosition();
    StoreArray<ECLCluster> eclClusters;
    for (const ECLCluster& eclCluster : eclClusters) {
      const TVector3& eclClusterPos = eclCluster.getClusterPosition();
      if (eclClusterPos.Angle(klmClusterPos) < angle) {
        return 1;
      }
    }
    return 0;
  }

  double klmClusterTiming(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return cluster->getTime();
  }


  double klmClusterPositionX(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return cluster->getClusterPosition().x();
  }


  double klmClusterPositionY(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return cluster->getClusterPosition().y();
  }


  double klmClusterPositionZ(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return cluster->getClusterPosition().z();
  }


  double klmClusterInnermostLayer(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return cluster->getInnermostLayer();
  }


  double klmClusterLayers(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return cluster->getLayers();
  }

  double klmClusterEnergy(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return cluster->getEnergy();
  }

  double klmClusterMomentum(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return cluster->getMomentumMag();
  }

  double klmClusterIsBKLM(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    float clusterZ = cluster->getClusterPosition().Z();
    if ((clusterZ > -180) && (clusterZ < 275)) {
      return 1;
    }
    return 0;
  }

  double klmClusterIsEKLM(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    float clusterZ = cluster->getClusterPosition().Z();
    if ((clusterZ < -180) || (clusterZ > 275)) {
      return 1;
    }
    return 0;
  }

  double klmClusterIsForwardEKLM(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    float clusterZ = cluster->getClusterPosition().Z();
    if (clusterZ > 275) {
      return 1;
    }
    return 0;
  }

  double klmClusterIsBackwardEKLM(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    float clusterZ = cluster->getClusterPosition().Z();
    if (clusterZ < -180) {
      return 1;
    }
    return 0;
  }

  double klmClusterTheta(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return cluster->getClusterPosition().Theta();
  }

  double klmClusterPhi(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return cluster->getClusterPosition().Phi();
  }

  double maximumKLMAngleCMS(const Particle* particle)
  {
    // check there actually are KLM clusters in the event
    StoreArray<KLMCluster> clusters;
    if (clusters.getEntries() == 0) return std::numeric_limits<double>::quiet_NaN();

    // get the input particle's vector momentum in the CMS frame
    PCmsLabTransform T;
    const TVector3 pCms = (T.rotateLabToCms() * particle->get4Vector()).Vect();

    // find the KLM cluster with the largest angle
    double maxAngle = 0.0;
    for (int iKLM = 0; iKLM < clusters.getEntries(); iKLM++) {
      const TVector3 clusterMomentumCms = (T.rotateLabToCms() * clusters[iKLM]->getMomentum()).Vect();
      double angle = pCms.Angle(clusterMomentumCms);
      if (angle > maxAngle) maxAngle = angle;
    }
    return maxAngle;
  }

  double nKLMClusterTrackMatches(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster)
      return std::numeric_limits<double>::quiet_NaN();
    size_t out = cluster->getRelationsFrom<Track>().size();
    return double(out);
  }

  double nMatchedKLMClusters(const Particle* particle)
  {
    Belle2::Particle::EParticleSourceObject particleSource = particle->getParticleSource();
    if (particleSource == Particle::EParticleSourceObject::c_Track) {
      return particle->getTrack()->getRelationsTo<KLMCluster>().size();
    } else if (particleSource == Particle::EParticleSourceObject::c_ECLCluster) {
      return particle->getECLCluster()->getRelationsTo<KLMCluster>().size();
    } else {
      return std::numeric_limits<double>::quiet_NaN();
    }
  }

  double nKLMClusterECLClusterMatches(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster)
      return std::numeric_limits<double>::quiet_NaN();
    size_t out = cluster->getRelationsFrom<ECLCluster>().size();
    return double(out);
  }

  double klmClusterTrackDistance(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster)
      return std::numeric_limits<double>::quiet_NaN();
    auto trackWithWeight = cluster->getRelatedFromWithWeight<Track>();
    if (!trackWithWeight.first)
      return std::numeric_limits<double>::quiet_NaN();
    return 1. / trackWithWeight.second;
  }

  VARIABLE_GROUP("KLM Cluster and KlongID");

  REGISTER_VARIABLE("klmClusterKlId", klmClusterKlId,
                    "Returns the KlId classifier output associated to the KLMCluster.");
  REGISTER_VARIABLE("klmClusterBelleTrackFlag", klmClusterBelleTrackFlag,
                    "Returns the Belle-style Track flag.");
  REGISTER_VARIABLE("klmClusterBelleECLFlag", klmClusterBelleECLFlag,
                    "Returns the Belle-style ECL flag.");
  REGISTER_VARIABLE("klmClusterTiming", klmClusterTiming, R"DOC(
Returns the timing informationf of the associated KLMCluster.

.. warning::
  Currently the KLM has no time calibration. This leads to a huge discrepancy for the variable :b2:var:`klmClusterTiming` if one compares collisions and simulated data. Moreover, the distribution of the variable on collisions data has a very complicated structure, due to the different timing shifts of different KLM components.
  It is recommended to not use it in any case until the KLM is calibrated, even for simulated data.

)DOC");
  REGISTER_VARIABLE("klmClusterPositionX", klmClusterPositionX, R"DOC(
Returns the :math:`x` position of the associated KLMCluster.
)DOC");
  REGISTER_VARIABLE("klmClusterPositionY", klmClusterPositionY, R"DOC(
Returns the :math:`y` position of the associated KLMCluster.
)DOC");
  REGISTER_VARIABLE("klmClusterPositionZ", klmClusterPositionZ, R"DOC(
Returns the :math:`z` position of the associated KLMCluster.
)DOC");
  REGISTER_VARIABLE("klmClusterInnermostLayer", klmClusterInnermostLayer,
                    "Returns the number of the innermost KLM layer with a 2-dimensional hit of the associated KLMCluster.");
  REGISTER_VARIABLE("klmClusterLayers", klmClusterLayers,
                    "Returns the number of KLM layers with 2-dimensional hits of the associated KLMCluster.");
  REGISTER_VARIABLE("klmClusterEnergy", klmClusterEnergy, R"DOC(
Returns the energy of the associated KLMCluster. 

.. warning::
  This variable returns an approximation of the energy: it uses :b2:var:`klmClusterMomentum` as momentum and the hypothesis that the KLMCluster is originated by a :math:`K_{L}^0` 
  (:math:`E_{\text{KLM}} = \sqrt{M_{K^0_L}^2 + p_{\text{KLM}}^2}`, where :math:`E_{\text{KLM}}` is this variable, :math:`M_{K^0_L}` is the :math:`K^0_L` mass and :math:`p_{\text{KLM}}` is :b2:var:`klmClusterMomentum`).
  It should be used with caution, and may not be physically meaningful, especially for :math:`n` candidates.

)DOC");
  REGISTER_VARIABLE("klmClusterMomentum", klmClusterMomentum, R"DOC(
Returns the momentum magnitude of the associated KLMCluster. 

.. warning::
  This variable returns an approximation of the momentum, since it is proportional to :b2:var:`klmClusterLayers` 
  (:math:`p_{\text{KLM}} = 0.215 \cdot N_{\text{layers}}`, where :math:`p_{\text{KLM}}` is this variable and :math:`N_{\text{layers}}` is :b2:var:`klmClusterLayers`).
  It should be used with caution, and may not be physically meaningful.

)DOC");
  REGISTER_VARIABLE("klmClusterIsBKLM", klmClusterIsBKLM,
                    "Returns 1 if the associated KLMCluster is in barrel KLM.");
  REGISTER_VARIABLE("klmClusterIsEKLM", klmClusterIsEKLM,
                    "Returns 1 if the associated KLMCluster is in endcap KLM.");
  REGISTER_VARIABLE("klmClusterIsForwardEKLM", klmClusterIsForwardEKLM,
                    "Returns 1 if the associated KLMCluster is in forward endcap KLM.");
  REGISTER_VARIABLE("klmClusterIsBackwardEKLM", klmClusterIsBackwardEKLM,
                    "Returns 1 if the associated KLMCluster is in backward endcap KLM.");
  REGISTER_VARIABLE("klmClusterTheta", klmClusterTheta, R"DOC(
Returns the polar (:math:`\theta`) angle of the associated KLMCluster.
)DOC");
  REGISTER_VARIABLE("klmClusterPhi", klmClusterPhi, R"DOC(
Returns the azimuthal (:math:`\phi`) angle of the associated KLMCluster.
)DOC");
  REGISTER_VARIABLE("maximumKLMAngleCMS", maximumKLMAngleCMS ,
                    "Returns the maximum angle in the CMS frame between the Particle and all KLMClusters in the event.");
  REGISTER_VARIABLE("nKLMClusterTrackMatches", nKLMClusterTrackMatches, R"DOC(
Returns the number of Tracks matched to the KLMCluster associated to this Particle. This variable can return a number greater than 0 for :math:`K_{L}^0` or :math:`n` candidates originating from KLMClusters and returns NaN for Particles with no KLMClusters associated.
)DOC");
  REGISTER_VARIABLE("nMatchedKLMClusters", nMatchedKLMClusters, R"DOC(
                     Returns the number of KLMClusters matched to the particle. It only works for
                     Particles created either from Tracks or from ECLCluster, while it returns NaN
                     for :math:`K_{L}^0` or :math:`n` candidates originating from KLMClusters.
              )DOC");
  REGISTER_VARIABLE("nKLMClusterECLClusterMatches", nKLMClusterECLClusterMatches, R"DOC(
                     Returns the number of ECLClusters matched to the KLMCluster associated to this Particle.
              )DOC");
  REGISTER_VARIABLE("klmClusterTrackDistance", klmClusterTrackDistance,
                    "Returns the distance between the Track and the KLMCluster associated to this Particle. This variable returns NaN if there is no Track-to-KLMCluster relationship.");

}
