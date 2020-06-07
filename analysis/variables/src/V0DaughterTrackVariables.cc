/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Sam Cunliffe, Martin Heck, Torben Ferber    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own includes
#include <analysis/variables/V0DaughterTrackVariables.h>
#include <analysis/VariableManager/Manager.h>

#include <analysis/variables/TrackVariables.h>

// framework - DataStore
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Helix.h>

// dataobjects from the MDST
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/V0.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/ECLCluster.h>

// framework aux
#include <framework/logging/Logger.h>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

    // An helper function to get track fit results
    // Not registered in variable mananger
    TrackFitResult const* getTrackFitResultFromV0DaughterParticle(Particle const* particle, const double daughterID)
    {
      const int dID = int(std::lround(daughterID));
      if (not(dID == 0 || dID == 1))
        return nullptr;

      if (particle->getParticleSource() == Particle::EParticleSourceObject::c_V0) {
        StoreArray<V0> V0s;
        const TrackFitResult* trackFit = (dID == 0) ?
                                         V0s[ particle->getMdstArrayIndex() ]->getTrackFitResults().first :
                                         V0s[ particle->getMdstArrayIndex() ]->getTrackFitResults().second;
        return trackFit;
      } else {
        const TrackFitResult* trackFit = getTrackFitResultFromParticle(particle->getDaughter(dID));
        return trackFit;
      }
    }

    double v0DaughterTrackNHits(const Particle* part, const std::vector<double>& daughterID, const Const::EDetector& det)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(part, daughterID[0]);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }

      if (det == Const::EDetector::CDC) {
        return trackFit->getHitPatternCDC().getNHits();
      } else if (det == Const::EDetector::SVD) {
        return trackFit->getHitPatternVXD().getNSVDHits();
      } else if (det == Const::EDetector::PXD) {
        return trackFit->getHitPatternVXD().getNPXDHits();
      } else {
        return std::numeric_limits<double>::quiet_NaN();
      }
    }

    double v0DaughterTrackNCDCHits(const Particle* part, const std::vector<double>& daughterID)
    {
      return v0DaughterTrackNHits(part, daughterID, Const::EDetector::CDC);
    }

    double v0DaughterTrackNSVDHits(const Particle* part, const std::vector<double>& daughterID)
    {
      return v0DaughterTrackNHits(part, daughterID, Const::EDetector::SVD);
    }

    double v0DaughterTrackNPXDHits(const Particle* part, const std::vector<double>& daughterID)
    {
      return v0DaughterTrackNHits(part, daughterID, Const::EDetector::PXD);
    }

    double v0DaughterTrackNVXDHits(const Particle* part, const std::vector<double>& daughterID)
    {
      return v0DaughterTrackNPXDHits(part, daughterID) + v0DaughterTrackNSVDHits(part, daughterID);
    }

    double v0DaughterTrackFirstSVDLayer(const Particle* part, const std::vector<double>& daughterID)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(part, daughterID[0]);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getHitPatternVXD().getFirstSVDLayer();
    }

    double v0DaughterTrackFirstPXDLayer(const Particle* part, const std::vector<double>& daughterID)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(part, daughterID[0]);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getHitPatternVXD().getFirstPXDLayer(HitPatternVXD::PXDMode::normal);
    }

    double v0DaughterTrackFirstCDCLayer(const Particle* part, const std::vector<double>& daughterID)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(part, daughterID[0]);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getHitPatternCDC().getFirstLayer();
    }

    double v0DaughterTrackLastCDCLayer(const Particle* part, const std::vector<double>& daughterID)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(part, daughterID[0]);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getHitPatternCDC().getLastLayer();
    }

    double v0DaughterTrackParam5AtIPPerigee(const Particle* part, const std::vector<double>& params)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(part, params[0]);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }

      const int paramID = (int)params[1];
      if (not(0 <= paramID && paramID < 5))
        return std::numeric_limits<double>::quiet_NaN();

      std::vector<float> tau = trackFit->getTau();
      return tau[paramID];
    }

    double v0DaughterTrackParamCov5x5AtIPPerigee(const Particle* part, const std::vector<double>& params)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(part, params[0]);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }

      const int paramID = (int)params[1];
      if (not(0 <= paramID && paramID < 15))
        return std::numeric_limits<double>::quiet_NaN();

      std::vector<float> cov = trackFit->getCov();
      return cov[paramID];
    }


    double v0DaughterTrackPValue(const Particle* part, const std::vector<double>& daughterID)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(part, daughterID[0]);
      if (!trackFit) {
        return std::numeric_limits<float>::quiet_NaN();
      }

      return trackFit->getPValue();
    }




    VARIABLE_GROUP("V0Daughter");

    REGISTER_VARIABLE("v0DaughterNCDCHits(i)", v0DaughterTrackNCDCHits,     "Number of CDC hits associated to the i-th daughter track");
    REGISTER_VARIABLE("v0DaughterNSVDHits(i)", v0DaughterTrackNSVDHits,     "Number of SVD hits associated to the i-th daughter track");
    REGISTER_VARIABLE("v0DaughterNPXDHits(i)", v0DaughterTrackNPXDHits,     "Number of PXD hits associated to the i-th daughter track");
    REGISTER_VARIABLE("v0DaughterNVXDHits(i)", v0DaughterTrackNVXDHits,
                      "Number of PXD and SVD hits associated to the i-th daughter track");
    REGISTER_VARIABLE("v0DaughterFirstSVDLayer(i)", v0DaughterTrackFirstSVDLayer,
                      "First activated SVD layer associated to the i-th daughter track");
    REGISTER_VARIABLE("v0DaughterFirstPXDLayer(i)", v0DaughterTrackFirstPXDLayer,
                      "First activated PXD layer associated to the i-th daughter track");
    REGISTER_VARIABLE("v0DaughterFirstCDCLayer(i)", v0DaughterTrackFirstCDCLayer,
                      "First activated CDC layer associated to the i-th daughter track");
    REGISTER_VARIABLE("v0DaughterLastCDCLayer(i)",  v0DaughterTrackLastCDCLayer,
                      "Last CDC layer associated to the i-th daughter track");
    REGISTER_VARIABLE("v0DaughterPValue(i)",        v0DaughterTrackPValue,
                      "chi2 probalility of the i-th daughter track fit");

    REGISTER_VARIABLE("v0DaughterTau(i,j)",        v0DaughterTrackParam5AtIPPerigee,
                      "j-th track parameter (at IP perigee) of the i-th daughter track. "
                      "j:  0:d0, 1:phi0, 2:omega, 3:z0, 4:tanLambda");
    REGISTER_VARIABLE("v0DaughterCov(i,j)",        v0DaughterTrackParamCov5x5AtIPPerigee,
                      "j-th element of the 15 covariance matrix elements (at IP perigee) of the i-th daughter track. "
                      "(0,0), (0,1) ... (1,1), (1,2) ... (2,2) ...");
  }
}
