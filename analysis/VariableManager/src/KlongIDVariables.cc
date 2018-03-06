/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include

#include <analysis/VariableManager/KlongIDVariables.h>
#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/KlId.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/TrackFitResult.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

    double particleKLMKlongID(const Particle* particle)
    {
      double result = -999;
      const KlId* klid = particle->getKLMCluster()->getRelatedTo<KlId>();
      if (klid) {
        result = klid->getKlId();
      }
      return result;
    }

    int particleKLMBelleTrackFlag(const Particle* particle)
    {
      const float angle = 0.24;
      const KLMCluster* cluster = particle->getKLMCluster();
      if (!cluster) {return -999;}
      const TVector3& pos = cluster->getClusterPosition();
      Belle2::StoreArray<Belle2::TrackFitResult> tracks;
      for (const Belle2::TrackFitResult& track : tracks) {
        const TVector3& trackPos = track.getPosition();
        if (trackPos.Angle(pos) < angle) {
          return 1;
        }
      }
      return 0;
    }

    int particleKLMBelleECLFlag(const Particle* particle)
    {
      const float angle = 0.24;
      const KLMCluster* klmcluster = particle->getKLMCluster();
      if (!klmcluster) {return -999;}
      const TVector3& pos = klmcluster->getClusterPosition();
      StoreArray<Belle2::ECLCluster> clusters;
      for (const Belle2::ECLCluster& cluster : clusters) {
        const TVector3& clusterPos = cluster.getClusterPosition();
        if (clusterPos.Angle(pos) < angle) {
          return 1;
        }
      }
      return 0;
    }

    int particleKLMisForwardEKLM(const Particle* particle)
    {
      const KLMCluster* cluster = particle->getKLMCluster();
      if (!cluster) {return -999;}
      float clusterZ = cluster->getClusterPosition().Z();
      if (clusterZ > 275) {return 1;}
      return 0;
    }

    int particleKLMisBackwardEKLM(const Particle* particle)
    {
      const KLMCluster* cluster = particle->getKLMCluster();
      if (!cluster) {return -999;}
      float clusterZ = cluster->getClusterPosition().Z();
      if (clusterZ < -180) {return 1;}
      return 0;
    }

    int particleKLMisBKLM(const Particle* particle)
    {
      const KLMCluster* cluster = particle->getKLMCluster();
      if (!cluster) {return -999;}
      float clusterZ = cluster->getClusterPosition().Z();
      if ((clusterZ < 275) && (clusterZ > -180)) {return 1;}
      return 0;
    }

    float particleKLMgetTime(const Particle* particle)
    {
      const KLMCluster* cluster = particle->getKLMCluster();
      if (!cluster) {return -999;}
      return cluster->getTime();
    }

    float particleKLMgetEnergy(const Particle* particle)
    {
      const KLMCluster* cluster = particle->getKLMCluster();
      if (!cluster) {return -999;}
      return cluster->getEnergy();
    }

    float particleKLMgetInnermostLayer(const Particle* particle)
    {
      const KLMCluster* cluster = particle->getKLMCluster();
      if (!cluster) {return -999;}
      return cluster->getInnermostLayer();
    }

    float particleKLMgetNLayers(const Particle* particle)
    {
      const KLMCluster* cluster = particle->getKLMCluster();
      if (!cluster) {return -999;}
      return cluster->getLayers();
    }


    VARIABLE_GROUP("klong-ID");
    REGISTER_VARIABLE("klongID_KLM"          , particleKLMKlongID              , "KlongID from KLMcluster classifier.");
    REGISTER_VARIABLE("klong_BelleTrackFlag" , particleKLMBelleTrackFlag    ,
                      "Does the corresponding Cluster carry a Belle style Track Flag.");
    REGISTER_VARIABLE("klong_BelleECLFlag"   , particleKLMBelleECLFlag      ,
                      "Does the corresponding Cluster carry a Belle style ECL Flag.");
    REGISTER_VARIABLE("klong_isForwardEKLM"  , particleKLMisForwardEKLM     , "Is the corresponding KLM cluster from Forward EKLM.");
    REGISTER_VARIABLE("klong_isBackwardEKLM" , particleKLMisBackwardEKLM    , "Is the corresponding KLM cluster from Backward EKLM.");
    REGISTER_VARIABLE("klong_isBKLM"         , particleKLMisBKLM            , "Is the corresponding KLM cluster from BKLM.");
    REGISTER_VARIABLE("klong_Nlayers"        , particleKLMgetNLayers        , "Number of Layers in the cluster.");
    REGISTER_VARIABLE("klong_InnermostLayer" , particleKLMgetInnermostLayer , "Number of the first hit layer");
    REGISTER_VARIABLE("klong_Time"           , particleKLMgetTime           ,
                      "Timing of Corresponding KLMcluster (corrected for muon? time of flight)");
    REGISTER_VARIABLE("klong_Energy"         , particleKLMgetEnergy         ,
                      "Energy of corresponding KLMcluster (0.215GeV times n_hitCells)");

  }
}
