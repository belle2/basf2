/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/TrackVariables.h>

#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternCDC.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <iostream>
#include <algorithm>
#include <cmath>

class getRelatedTo;
using namespace std;

namespace Belle2 {
  namespace Variable {

    double trackNHits(const Particle* part, const Const::EDetector& det)
    {
      const Track* track = part->getTrack();
      if (!track) {
        return 0.0;
      }

      const TrackFitResult* trackFit = track->getTrackFitResult(Const::ChargedStable(abs(part->getPDGCode())));
      if (!trackFit) {
        return 0.0;
      }

      if (det == Const::EDetector::CDC) {
        return trackFit->getHitPatternCDC().getNHits();
      } else if (det == Const::EDetector::CDC) {
        // TODO: implement when available
        return 0.0;
      } else if (det == Const::EDetector::CDC) {
        // TODO: implement when available
        return 0.0;
      } else
        return 0.0;
    }

    double trackNCDCHits(const Particle* part)
    {
      return trackNHits(part, Const::EDetector::CDC);
    }

    double trackNSVDHits(const Particle* part)
    {
      return trackNHits(part, Const::EDetector::SVD);
    }

    double trackNPXDHits(const Particle* part)
    {
      return trackNHits(part, Const::EDetector::PXD);
    }

    double trackD0(const Particle* part)
    {
      const Track* track = part->getTrack();
      if (!track) return 0.0;

      const TrackFitResult* trackFit = track->getTrackFitResult(Const::ChargedStable(abs(part->getPDGCode())));
      if (!trackFit) return 0.0;

      return trackFit->getD0();
    }

    double trackZ0(const Particle* part)
    {
      const Track* track = part->getTrack();
      if (!track) return 0.0;

      const TrackFitResult* trackFit = track->getTrackFitResult(Const::ChargedStable(abs(part->getPDGCode())));
      if (!trackFit) return 0.0;

      return trackFit->getZ0();
    }

    double trackPValue(const Particle* part)
    {
      const Track* track = part->getTrack();
      if (!track) return 0.0;

      const TrackFitResult* trackFit = track->getTrackFitResult(Const::ChargedStable(abs(part->getPDGCode())));
      if (!trackFit) return 0.0;

      return trackFit->getPValue();
    }


    VARIABLE_GROUP("PID");
    REGISTER_VARIABLE("nCDCHits", trackNCDCHits,     "Number of CDC hits associated to the track");
    REGISTER_VARIABLE("nSVDHits", trackNSVDHits,     "Number of SVD hits associated to the track");
    REGISTER_VARIABLE("nPXDHits", trackNPXDHits,     "Number of PXD hits associated to the track");

    REGISTER_VARIABLE("d0",     trackD0,     "Signed distance to the POCA in the r-phi plane");
    REGISTER_VARIABLE("z0",     trackZ0,     "z coordinate of the POCA");
    REGISTER_VARIABLE("pValue", trackPValue, "chi2 probalility of the track fit");

  }
}
