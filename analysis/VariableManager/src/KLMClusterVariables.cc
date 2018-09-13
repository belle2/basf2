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
      double result = 0.0;

      const KLMCluster* cluster = particle->getKLMCluster();
      if (cluster) {
        result = cluster->getTime();
      }
      return result;
    }


    double klmClusterPositionX(const Particle* particle)
    {
      double result = 0.0;

      const KLMCluster* cluster = particle->getKLMCluster();
      if (cluster) {
        result = cluster->getClusterPosition().x();
      }
      return result;
    }


    double klmClusterPositionY(const Particle* particle)
    {
      double result = 0.0;

      const KLMCluster* cluster = particle->getKLMCluster();
      if (cluster) {
        result = cluster->getClusterPosition().y();
      }
      return result;
    }


    double klmClusterPositionZ(const Particle* particle)
    {
      double result = 0.0;

      const KLMCluster* cluster = particle->getKLMCluster();
      if (cluster) {
        result = cluster->getClusterPosition().z();
      }
      return result;
    }


    double klmClusterInnermostLayer(const Particle* particle)
    {
      double result = 0.0;

      const KLMCluster* cluster = particle->getKLMCluster();
      if (cluster) {
        result = cluster->getInnermostLayer();
      }
      return result;
    }


    double klmClusterLayers(const Particle* particle)
    {
      double result = 0.0;

      const KLMCluster* cluster = particle->getKLMCluster();
      if (cluster) {
        result = cluster->getLayers();
      }
      return result;
    }

    double klmClusterEnergy(const Particle* particle)
    {
      double result = 0.0;

      const KLMCluster* cluster = particle->getKLMCluster();
      if (cluster) {
        result = cluster->getEnergy();
      }
      return result;
    }

    double klmClusterMomentum(const Particle* particle)
    {
      double result = 0.0;

      const KLMCluster* cluster = particle->getKLMCluster();
      if (cluster) {
        result = cluster->getMomentumMag();
      }
      return result;
    }

    double maximumKLMAngleCMS(const Particle* part)
    {
      PCmsLabTransform T;
      const TVector3 pcms = (T.rotateLabToCms() * part->get4Vector()).Vect();
      double maxangle = -999.0;

      StoreArray<KLMCluster> klmClusters;
      for (int iKLM = 0; iKLM < klmClusters.getEntries(); iKLM++) {
        const TVector3 klmmomcms = (T.rotateLabToCms() * klmClusters[iKLM]->getMomentum()).Vect();
        double angle = pcms.Angle(klmmomcms);

        if (angle > maxangle) {
          maxangle = angle;
        }
      }

      return maxangle;
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

    VARIABLE_GROUP("KLM Cluster");

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
