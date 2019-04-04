/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Torben Ferber, Giacomo De Pietro,           *
 *               Sam Cunliffe                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/variables/KLMClusterVariables.h>
#include <analysis/utility/PCmsLabTransform.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// mdst dataobjects
#include <mdst/dataobjects/KlId.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/ECLCluster.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// std
#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

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
      return cluster->getMomentum().Theta();
    }

    double klmClusterPhi(const Particle* particle)
    {
      const KLMCluster* cluster = particle->getKLMCluster();
      if (!cluster) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return cluster->getMomentum().Phi();
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

    double minimumKLMAngleCMS(const Particle* particle)
    {
      // check there actually are KLM clusters in the event
      StoreArray<KLMCluster> clusters;
      if (clusters.getEntries() == 0) return std::numeric_limits<double>::quiet_NaN();

      // get the input particle's vector momentum in the CMS frame
      PCmsLabTransform T;
      const TVector3 pCms = (T.rotateLabToCms() * particle->get4Vector()).Vect();

      // find the KLM cluster with the smallest angle
      double minAngle = M_PI;
      for (int iKLM = 0; iKLM < clusters.getEntries(); iKLM++) {
        const TVector3 clusterMomentumCms = (T.rotateLabToCms() * clusters[iKLM]->getMomentum()).Vect();
        double angle = pCms.Angle(clusterMomentumCms);
        if (angle < minAngle) minAngle = angle;
      }
      return minAngle;
    }

    Manager::FunctionPtr closestKLMCMS(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments for meta function closestKLMCMS");

      // get the variable wanted for the cluster particle
      const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);

      auto func = [var](const Particle * particle) -> double {

        // check there actually are KLM clusters in the event
        StoreArray<KLMCluster> clusters;
        if (clusters.getEntries() == 0) return std::numeric_limits<double>::quiet_NaN();

        // get the input particle's vector momentum in the CMS frame
        PCmsLabTransform T;
        const TVector3 pCms = (T.rotateLabToCms() * particle->get4Vector()).Vect();

        // find the KLM cluster with the smallest angle
        double minAngle = M_PI;
        int iBest = -1;
        for (int iKLM = 0; iKLM < clusters.getEntries(); iKLM++)
        {
          const TVector3 clusterMomentumCms = (T.rotateLabToCms() * clusters[iKLM]->getMomentum()).Vect();
          double angle = pCms.Angle(clusterMomentumCms);
          if (angle < minAngle) {
            minAngle = angle;
            iBest = iKLM; // update the array index of the closest
          }
        } // loop over KLM clusters
        Particle tmpPart(clusters[iBest]);
        return var->function(&tmpPart);
      };
      return func;
    }

    Manager::FunctionPtr furthestKLMCMS(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments for meta function furthestKLMCMS");

      // get the variable wanted for the cluster particle
      const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);

      auto func = [var](const Particle * particle) -> double {

        // check there actually are KLM clusters in the event
        StoreArray<KLMCluster> clusters;
        if (clusters.getEntries() == 0) return std::numeric_limits<double>::quiet_NaN();

        // get the input particle's vector momentum in the CMS frame
        PCmsLabTransform T;
        const TVector3 pCms = (T.rotateLabToCms() * particle->get4Vector()).Vect();

        // find the KLM cluster with the largest angle
        double maxAngle = 0;
        int iBest = -1;
        for (int iKLM = 0; iKLM < clusters.getEntries(); iKLM++)
        {
          const TVector3 clusterMomentumCms = (T.rotateLabToCms() * clusters[iKLM]->getMomentum()).Vect();
          double angle = pCms.Angle(clusterMomentumCms);
          if (angle > maxAngle) {
            maxAngle = angle;
            iBest = iKLM; // update the array index of the furthest
          }
        } // loop over KLM clusters
        Particle tmpPart(clusters[iBest]);
        return var->function(&tmpPart);
      };
      return func;
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
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<double>::quiet_NaN();
      size_t out = track->getRelationsTo<KLMCluster>().size();
      return double(out);
    }

    VARIABLE_GROUP("KLM Cluster and KlongID");

    REGISTER_VARIABLE("klmClusterKlId", klmClusterKlId, "Returns the KlId associated to the KLMCluster.");
    REGISTER_VARIABLE("klmClusterBelleTrackFlag", klmClusterBelleTrackFlag, "Returns the Belle-style Track flag.");
    REGISTER_VARIABLE("klmClusterBelleECLFlag", klmClusterBelleECLFlag, "Returns the Belle-style ECL flag.");
    REGISTER_VARIABLE("klmClusterTiming", klmClusterTiming, "Returns KLMCluster's timing info.");
    REGISTER_VARIABLE("klmClusterPositionX", klmClusterPositionX, "Returns KLMCluster's x position.");
    REGISTER_VARIABLE("klmClusterPositionY", klmClusterPositionY, "Returns KLMCluster's y position.");
    REGISTER_VARIABLE("klmClusterPositionZ", klmClusterPositionZ, "Returns KLMCluster's z position.");
    REGISTER_VARIABLE("klmClusterInnermostLayer", klmClusterInnermostLayer,
                      "Returns KLM cluster's number of the innermost layer with hits.");
    REGISTER_VARIABLE("klmClusterLayers", klmClusterLayers, "Returns KLM cluster's number of layers with hits.");
    REGISTER_VARIABLE("klmClusterEnergy", klmClusterEnergy, "Returns KLMCluster's energy (assuming K_L0 hypothesis).");
    REGISTER_VARIABLE("klmClusterMomentum", klmClusterMomentum, "Returns KLMCluster's momentum magnitude.")
    REGISTER_VARIABLE("klmClusterIsBKLM", klmClusterIsBKLM, "Returns 1 if the associated KLMCluster is in BKLM.");
    REGISTER_VARIABLE("klmClusterIsEKLM", klmClusterIsEKLM, "Returns 1 if the associated KLMCluster is in EKLM.");
    REGISTER_VARIABLE("klmClusterIsForwardEKLM", klmClusterIsForwardEKLM, "Returns 1 if the associated KLMCluster is in forward EKLM.");
    REGISTER_VARIABLE("klmClusterIsBackwardEKLM", klmClusterIsBackwardEKLM,
                      "Returns 1 if the associated KLMCluster is in backward EKLM.");
    REGISTER_VARIABLE("klmClusterTheta", klmClusterTheta, "Returns KLMCluster's theta.");
    REGISTER_VARIABLE("klmClusterPhi", klmClusterPhi, "Returns KLMCluster's phi.");
    REGISTER_VARIABLE("maximumKLMAngleCMS", maximumKLMAngleCMS ,
                      "Returns the maximum angle in the CMS between the Particle and all KLM clusters in the event.");
    REGISTER_VARIABLE("minimumKLMAngleCMS", minimumKLMAngleCMS ,
                      "Returns the minimum angle in the CMS between the Particle and all KLM clusters in the event.");
    REGISTER_VARIABLE("closestKLMCMS", closestKLMCMS ,
                      "Returns the value of the variable provided for the closest KLM cluster particle where \"close\" is the smallest angle in the CMS. For example `closestKLMCMS(klmClusterE)` returns the energy. See also `minimumKLMAngleCMS`.");
    REGISTER_VARIABLE("furthestKLMCMS", furthestKLMCMS ,
                      "Returns the value of the variable provided for the furthest KLM cluster particle where \"far\" is the largest angle in the CMS. For example `furthestKLMCMS(klmClusterE)` returns the energy. See also `maximumKLMAngleCMS`.");
    REGISTER_VARIABLE("nKLMClusterTrackMatches", nKLMClusterTrackMatches,
                      "Returns the number of Tracks matched to the KLMCluster associated to this Particle (0 for K_L0, >0 for matched Tracks, NaN for not-matched Tracks).");
    REGISTER_VARIABLE("nMatchedKLMClusters", nMatchedKLMClusters,
                      "Returns the number of KLMClusters matched to the Track associated to this Particle. This variable returns NaN for K_L0 (they have no Tracks associated). It can return >1");

  }
}
