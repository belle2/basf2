/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// needed to build variables here
#include <cdc/variables/DedxVariables.h>
#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Conversion.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/Track.h>
#include <reconstruction/dataobjects/VXDDedxTrack.h>

#include <TString.h>

#include <cmath>

namespace Belle2 {

  /**
  * SVD dEdx value from particle
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

    //Variables for SVD dedx
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

    double SVDdedx(const Particle* part)
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

    VARIABLE_GROUP("SVD dEdx");
    //SVD variables
    REGISTER_VARIABLE("SVDdEdx", SVDdedx, "SVD dE/dx truncated mean");
    REGISTER_VARIABLE("pSVD", SVD_p, "momentum valid in the SVD");
    REGISTER_VARIABLE("SVD_pTrue", SVD_pTrue, "true MC momentum valid in the SVD");
    REGISTER_VARIABLE("SVD_CosTheta", SVD_CosTheta, "cos(theta) of the track valid in the SVD");
    REGISTER_VARIABLE("SVD_nHits", SVD_nHits, "number of hits of the track valid in the SVD");

  }
}
