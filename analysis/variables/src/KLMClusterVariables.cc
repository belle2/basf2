/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <analysis/variables/KLMClusterVariables.h>

// include VariableManager
#include <analysis/VariableManager/Manager.h>

/* Analysis headers. */
#include <analysis/dataobjects/Particle.h>
#include <analysis/utility/PCmsLabTransform.h>

/* Basf2 headers. */
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Helix.h>
#include <framework/gearbox/Const.h>
#include <framework/geometry/VectorUtil.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KlId.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

/* ROOT headers. */
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <Math/VectorUtil.h>

using namespace std;

namespace Belle2::Variable {

  double klmClusterKlId(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return Const::doubleNaN;
    }
    const KlId* klid = cluster->getRelatedTo<KlId>();
    if (!klid) {
      return Const::doubleNaN;
    }
    return klid->getKlId();
  }

  int klmClusterBelleTrackFlag(const Particle* particle)
  {
    const float angle = 0.24;
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return 0;
    }
    const ROOT::Math::XYZVector& pos = cluster->getClusterPosition();
    StoreArray<TrackFitResult> tracks;
    for (const TrackFitResult& track : tracks) {
      const ROOT::Math::XYZVector& trackPos = track.getPosition();
      if (ROOT::Math::VectorUtil::Angle(trackPos, pos) < angle) {
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
      return 0;
    }
    const ROOT::Math::XYZVector& klmClusterPos = klmCluster->getClusterPosition();
    StoreArray<ECLCluster> eclClusters;
    for (const ECLCluster& eclCluster : eclClusters) {
      const ROOT::Math::XYZVector& eclClusterPos = eclCluster.getClusterPosition();
      if (ROOT::Math::VectorUtil::Angle(eclClusterPos, klmClusterPos) < angle) {
        return 1;
      }
    }
    return 0;
  }

  double klmClusterTiming(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return Const::doubleNaN;
    }
    return cluster->getTime();
  }


  double klmClusterPositionX(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return Const::doubleNaN;
    }
    return cluster->getClusterPosition().X();
  }


  double klmClusterPositionY(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return Const::doubleNaN;
    }
    return cluster->getClusterPosition().Y();
  }


  double klmClusterPositionZ(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return Const::doubleNaN;
    }
    return cluster->getClusterPosition().Z();
  }


  double klmClusterInnermostLayer(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return Const::doubleNaN;
    }
    return cluster->getInnermostLayer();
  }


  double klmClusterLayers(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return Const::doubleNaN;
    }
    return cluster->getLayers();
  }

  double klmClusterEnergy(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return Const::doubleNaN;
    }
    return cluster->getEnergy();
  }

  double klmClusterMomentum(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return Const::doubleNaN;
    }
    return cluster->getMomentumMag();
  }

  double klmClusterIsBKLM(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return Const::doubleNaN;
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
      return Const::doubleNaN;
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
      return Const::doubleNaN;
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
      return Const::doubleNaN;
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
      return Const::doubleNaN;
    }
    return cluster->getClusterPosition().Theta();
  }

  double klmClusterPhi(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster) {
      return Const::doubleNaN;
    }
    return cluster->getClusterPosition().Phi();
  }

  double maximumKLMAngleCMS(const Particle* particle)
  {
    // check there actually are KLM clusters in the event
    StoreArray<KLMCluster> clusters;
    if (clusters.getEntries() == 0) return Const::doubleNaN;

    // get the input particle's vector momentum in the CMS frame
    PCmsLabTransform T;
    const ROOT::Math::PxPyPzEVector pCms = T.rotateLabToCms() * particle->get4Vector();

    // find the KLM cluster with the largest angle
    double maxAngle = 0.0;
    for (int iKLM = 0; iKLM < clusters.getEntries(); iKLM++) {
      const ROOT::Math::PxPyPzEVector clusterMomentumCms = T.rotateLabToCms() * clusters[iKLM]->getMomentum();
      double angle = ROOT::Math::VectorUtil::Angle(pCms, clusterMomentumCms);
      if (angle > maxAngle) maxAngle = angle;
    }
    return maxAngle;
  }

  double nKLMClusterTrackMatches(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster)
      return Const::doubleNaN;
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
      return Const::doubleNaN;
    }
  }

  double nKLMClusterECLClusterMatches(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster)
      return Const::doubleNaN;
    size_t out = cluster->getRelationsFrom<ECLCluster>().size();
    return double(out);
  }

  double klmClusterTrackDistance(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster)
      return Const::doubleNaN;
    return cluster->getClusterTrackSeparation();
  }

  ROOT::Math::XYZVector getPositionOnHelix_trackfit(const TrackFitResult* trackFit)
  {
    double r_BKLM =  201.6;
    double z_EFWD =  283.9;
    double z_EBWD = -189.9;

    // get helix and parameters
    const double z0 = trackFit->getZ0();
    const double tanlambda = trackFit->getTanLambda();
    const Helix h = trackFit->getHelix();

    // extrapolate to radius
    const double arcLength = h.getArcLength2DAtCylindricalR(r_BKLM);
    const double lHelixRadius = arcLength > 0 ? arcLength : std::numeric_limits<double>::max();

    // extrapolate to FWD z
    const double lFWD = (z_EFWD - z0) / tanlambda > 0 ? (z_EFWD - z0) / tanlambda : std::numeric_limits<double>::max();

    // extrapolate to BWD z
    const double lBWD = (z_EBWD - z0) / tanlambda > 0 ? (z_EBWD - z0) / tanlambda : std::numeric_limits<double>::max();

    // pick smallest arclength
    const double l = std::min({lHelixRadius, lFWD, lBWD});

    ROOT::Math::XYZVector ext_helix = h.getPositionAtArcLength2D(l);
    double helixExtR_surface = r_BKLM / sin(ext_helix.Theta());
    if (l == lFWD) { helixExtR_surface = z_EFWD / cos(ext_helix.Theta());}
    else if (l == lBWD) { helixExtR_surface = z_EBWD / cos(ext_helix.Theta());}

    ROOT::Math::XYZVector helixExt_surface_position(0, 0, 0);
    VectorUtil::setMagThetaPhi(helixExt_surface_position, helixExtR_surface, ext_helix.Theta(), ext_helix.Phi());

    return helixExt_surface_position;
  }

  double klmClusterTrackDistance_helix_extrapolation(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    static const ROOT::Math::XYZVector vecNaN(Const::doubleNaN, Const::doubleNaN, Const::doubleNaN);
    double r_BKLM =  201.6;
    double z_EFWD =  283.9;
    double z_EBWD = -189.9;

    bool isEFWD = false;
    bool isEBWD = false;
    isEFWD = (cluster->getClusterPosition().Z() > 275);
    isEBWD = (cluster->getClusterPosition().Z() < -180);

    double klmClusterR_surface = r_BKLM / sin(cluster->getClusterPosition().Theta());
    if (isEFWD) {klmClusterR_surface = z_EFWD / cos(cluster->getClusterPosition().Theta());}
    else if (isEBWD) {klmClusterR_surface = z_EBWD / cos(cluster->getClusterPosition().Theta());}

    ROOT::Math::XYZVector klmCluster_surface_position(0, 0, 0);
    VectorUtil::setMagThetaPhi(klmCluster_surface_position, klmClusterR_surface,  cluster->getClusterPosition().Theta(),
                               cluster->getClusterPosition().Phi());

    double min_dist = 1e10, dist = 1e10;

    StoreArray<Track> tracks;
    for (const Track& track : tracks) { // loop on all tracks here, and select the one with the minimum distance
      const TrackFitResult* trackfit = track.getTrackFitResultWithBestPValue();
      if (!trackfit) {continue;}
      ROOT::Math::XYZVector helixExt_surface_position = getPositionOnHelix_trackfit(trackfit);
      if (helixExt_surface_position == vecNaN) return Const::doubleNaN;

      dist = (klmCluster_surface_position - helixExt_surface_position).R();
      if (dist < min_dist) min_dist = dist;
    }
    if (min_dist > 9999) min_dist = -1;
    return min_dist;
  }


  double klmClusterTrackSeparationAngle(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster)
      return Const::doubleNaN;
    return cluster->getClusterTrackSeparationAngle();
  }

  double klmClusterTrackRotationAngle(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster)
      return Const::doubleNaN;
    return cluster->getClusterTrackRotationAngle();
  }

  double klmClusterShapeStdDev1(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster)
      return Const::doubleNaN;
    return cluster->getShapeStdDev1();
  }

  double klmClusterShapeStdDev2(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster)
      return Const::doubleNaN;
    return cluster->getShapeStdDev2();
  }

  double klmClusterShapeStdDev3(const Particle* particle)
  {
    const KLMCluster* cluster = particle->getKLMCluster();
    if (!cluster)
      return Const::doubleNaN;
    return cluster->getShapeStdDev3();
  }

  VARIABLE_GROUP("KLM Cluster and KlongID");

  REGISTER_VARIABLE("klmClusterKlId", klmClusterKlId,
                    "Returns the KlId classifier output associated to the KLMCluster.");
  REGISTER_VARIABLE("klmClusterBelleTrackFlag", klmClusterBelleTrackFlag,
                    "Returns the Belle-style Track flag.");
  REGISTER_VARIABLE("klmClusterBelleECLFlag", klmClusterBelleECLFlag,
                    "Returns the Belle-style ECL flag.");
  REGISTER_VARIABLE("klmClusterTiming", klmClusterTiming, R"DOC(
Returns the timing information of the associated KLMCluster.

)DOC","ns");
  REGISTER_VARIABLE("klmClusterPositionX", klmClusterPositionX, R"DOC(
Returns the :math:`x` position of the associated KLMCluster.

)DOC","cm");
  REGISTER_VARIABLE("klmClusterPositionY", klmClusterPositionY, R"DOC(
Returns the :math:`y` position of the associated KLMCluster.

)DOC","cm");
  REGISTER_VARIABLE("klmClusterPositionZ", klmClusterPositionZ, R"DOC(
Returns the :math:`z` position of the associated KLMCluster.

)DOC","cm");
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

)DOC","GeV");
  REGISTER_VARIABLE("klmClusterMomentum", klmClusterMomentum, R"DOC(
Returns the momentum magnitude of the associated KLMCluster. 

.. warning::
  This variable returns an approximation of the momentum, since it is proportional to :b2:var:`klmClusterLayers` 
  (:math:`p_{\text{KLM}} = 0.215 \cdot N_{\text{layers}}`, where :math:`p_{\text{KLM}}` is this variable and :math:`N_{\text{layers}}` is :b2:var:`klmClusterLayers`).
  It should be used with caution, and may not be physically meaningful.

)DOC","GeV/c");
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

)DOC","rad");
  REGISTER_VARIABLE("klmClusterPhi", klmClusterPhi, R"DOC(
Returns the azimuthal (:math:`\phi`) angle of the associated KLMCluster.

)DOC","rad");
  REGISTER_VARIABLE("maximumKLMAngleCMS", maximumKLMAngleCMS ,
                    "Returns the maximum angle in the CMS frame between the Particle and all KLMClusters in the event.\n\n","rad");
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
                    "Returns the distance between KLMCluster associated to this Particle and the closest track. This variable returns NaN if there is no Track-to-KLMCluster relationship.\n\n",
                    "cm");
  REGISTER_VARIABLE("klmClusterTrackDistance_helix_extrapolation", klmClusterTrackDistance_helix_extrapolation,
                    "This also returns the distance between KLM cluster and its closes track, but calculated using helix extrapolation. This variable returns NaN if there is no Track-to-KLMCluster relationship.\n\n",
                    "cm");
  REGISTER_VARIABLE("klmClusterTrackRotationAngle", klmClusterTrackRotationAngle,
                    "Returns the angle between the direction at the IP and at the POCA to the KLMCluster associated to this Particle for the closest track. This variable returns NaN if there is no Track-to-KLMCluster relationship.\n\n",
                    "rad");
  REGISTER_VARIABLE("klmClusterTrackSeparationAngle", klmClusterTrackSeparationAngle,
                    "Returns the angle between the KLMCluster associated to this Particle and the closest track. This variable returns NaN if there is no Track-to-KLMCluster relationship"
                    "rad");

  REGISTER_VARIABLE("klmClusterShapeStdDev1", klmClusterShapeStdDev1,
                    "Returns the std deviation of the 1st axis from a PCA of the KLMCluster associated to this Particle. This variable returns 0 if this KLMCluster contains only one KLMHit2d cluster."
                    "cm");
  REGISTER_VARIABLE("klmClusterShapeStdDev2", klmClusterShapeStdDev2,
                    "Returns the std deviation of the 2nd axis from a PCA of the KLMCluster associated to this Particle. This variable returns 0 if this KLMCluster contains only one KLMHit2d cluster."
                    "cm");
  REGISTER_VARIABLE("klmClusterShapeStdDev3", klmClusterShapeStdDev3,
                    "Returns the std deviation of the 3rd axis from a PCA of the KLMCluster associated to this Particle. This variable returns 0 if this KLMCluster contains only one KLMHit2d cluster."
                    "cm");
}
