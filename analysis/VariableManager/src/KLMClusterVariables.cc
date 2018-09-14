/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Torben Ferber                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/KLMClusterVariables.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/utility/PCmsLabTransform.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/Track.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

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

    double maximumKLMAngleCMS(const Particle* particle)
    {
      StoreArray<KLMCluster> clusters;
      if (clusters.getEntries() == 0) {
        return std::numeric_limits<double>::quiet_NaN();
      }

      PCmsLabTransform T;
      const TVector3 pCms = (T.rotateLabToCms() * particle->get4Vector()).Vect();

      double maxAngle = 0.0;
      for (int iKLM = 0; iKLM < clusters.getEntries(); iKLM++) {
        const TVector3 clusterMomentumCms = (T.rotateLabToCms() * clusters[iKLM]->getMomentum()).Vect();
        double angle = pCms.Angle(clusterMomentumCms);
        if (angle > maxAngle) {
          maxAngle = angle;
        }
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
      const Track* track = particle->getTrack();
      if (!track)
        return std::numeric_limits<double>::quiet_NaN();
      size_t out = track->getRelationsTo<KLMCluster>().size();
      return double(out);
    }

    VARIABLE_GROUP("KLM Cluster and KlongID");

    REGISTER_VARIABLE("klmClusterTiming", klmClusterTiming, "Returns KLMCluster's timing info.");
    REGISTER_VARIABLE("klmClusterPositionX", klmClusterPositionX, "Returns KLMCluster's x position.");
    REGISTER_VARIABLE("klmClusterPositionY", klmClusterPositionY, "Returns KLMCluster's y position.");
    REGISTER_VARIABLE("klmClusterPositionZ", klmClusterPositionZ, "Returns KLMCluster's z position.");
    REGISTER_VARIABLE("klmClusterInnermostLayer", klmClusterInnermostLayer,
                      "Returns KLM cluster's number of the innermost layer with hits.");
    REGISTER_VARIABLE("klmClusterLayers", klmClusterLayers, "Returns KLM cluster's number of layers with hits.");
    REGISTER_VARIABLE("klmClusterEnergy", klmClusterEnergy, "Returns KLMCluster's energy (assuming K_L0 hypothesis).");
    REGISTER_VARIABLE("klmClusterMomentum", klmClusterMomentum, "Returns KLMCluster's momentum magnitude.")
    REGISTER_VARIABLE("maximumKLMAngleCMS", maximumKLMAngleCMS ,
                      "Returns the maximum angle in the CMS between the Particle and all KLM clusters in the event.");
    REGISTER_VARIABLE("nKLMClusterTrackMatches", nKLMClusterTrackMatches,
                      "Returns the number of Tracks matched to the KLMCluster associated to this Particle (0 for K_L0, >0 for matched Tracks, NaN for not-matched Tracks).");
    REGISTER_VARIABLE("nMatchedKLMClusters", nMatchedKLMClusters,
                      "Returns the number of KLMClusters matched to the Track associated to this Particle. This variable returns NaN for K_L0 (they have no Tracks associated). It can return >1");

  }
}
