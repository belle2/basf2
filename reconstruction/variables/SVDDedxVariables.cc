/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett, Sagar Hazra
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// needed to build variables here
#include <reconstruction/variables/DedxVariables.h>
#include <framework/core/Module.h>
#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <reconstruction/dataobjects/VXDDedxTrack.h>
#include <mdst/dataobjects/PIDLikelihood.h>
// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <iostream>
#include <algorithm>
#include <cmath>

namespace Belle2 {

  /**
  * dEdx value from particle
  */
  VXDDedxTrack const* getSVDDedxFromParticle(Particle const* particle)
  {
    const Track* track = particle->getTrack();
    if (!track) {
      return nullptr;
    }

    const VXDDedxTrack* dedxTrack = track->getRelatedTo<VXDDedxTrack>();
    if (!dedxTrack) {
      return nullptr;
    }
    return dedxTrack;
  }

  namespace Variable {

    double SVD_p(const Particle* part)
    {
      const VXDDedxTrack* dedxTrack = getSVDDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getMomentum();
      }
    }
    double SVD_pTrue(const Particle* part)
    {
      const VXDDedxTrack* dedxTrack = getSVDDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getTrueMomentum();
      }
    }

    double SVD_dedx(const Particle* part)
    {
      const VXDDedxTrack* dedxTrack = getSVDDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getDedx(Const::EDetector::SVD);
      }
    }

    double SVD_CosTheta(const Particle* part)
    {
      const VXDDedxTrack* dedxTrack = getSVDDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getCosTheta();
      }
    }
    double SVD_nHits(const Particle* part)
    {
      const VXDDedxTrack* dedxTrack = getSVDDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->size();
      }
    }


    VARIABLE_GROUP("SVDDedx");
    REGISTER_VARIABLE("SVD_p", SVD_p, "momentum valid in the SVD");
    REGISTER_VARIABLE("SVD_pTrue", SVD_pTrue, "true MC momentum valid in the SVD");
    REGISTER_VARIABLE("SVD_dedx", SVD_dedx, "SVD dE/dx truncated mean");
    REGISTER_VARIABLE("SVD_CosTheta", SVD_CosTheta, "cos(theta) of the track valid in the SVD");
    REGISTER_VARIABLE("SVD_nHits", SVD_nHits, "number of hits of the track valid in the SVD");


  }
}
