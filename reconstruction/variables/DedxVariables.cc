/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
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
#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <reconstruction/dataobjects/VXDDedxTrack.h>
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
  CDCDedxTrack const* getDedxFromParticle(Particle const* particle)
  {
    const Track* track = particle->getTrack();
    if (!track) {
      return nullptr;
    }

    const CDCDedxTrack* dedxTrack = track->getRelatedTo<CDCDedxTrack>();
    if (!dedxTrack) {
      return nullptr;
    }

    return dedxTrack;
  }
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

    double dedx(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getDedx();
      }
    }

    double dedxnosat(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getDedxNoSat();
      }
    }

    double pCDC(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getMomentum();
      }
    }

    double CDCdEdx_chiE(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getChi(0);
      }
    }

    double CDCdEdx_chiMu(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getChi(1);
      }
    }

    double CDCdEdx_chiPi(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getChi(2);
      }
    }

    double CDCdEdx_chiK(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getChi(3);
      }
    }

    double CDCdEdx_chiP(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getChi(4);
      }
    }


    double CDCdEdx_chiD(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getChi(5);
      }
    }

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


    VARIABLE_GROUP("Dedx");
    REGISTER_VARIABLE("dedx", dedx, "dE/dx truncated mean");
    REGISTER_VARIABLE("dedxnosat", dedxnosat, "dE/dx truncated mean without saturation correction");
    REGISTER_VARIABLE("pCDC", pCDC, "Momentum valid in the CDC");
    REGISTER_VARIABLE("CDCdEdx_chiE", CDCdEdx_chiE, "Chi value of electrons from CDC dEdx");
    REGISTER_VARIABLE("CDCdEdx_chiMu", CDCdEdx_chiMu, "Chi value of muons from CDC dEdx");
    REGISTER_VARIABLE("CDCdEdx_chiPi", CDCdEdx_chiPi, "Chi value of pions from CDC dEdx");
    REGISTER_VARIABLE("CDCdEdx_chiK", CDCdEdx_chiK, "Chi value of kaons from CDC dEdx");
    REGISTER_VARIABLE("CDCdEdx_chiP", CDCdEdx_chiP, "Chi value of protons from CDC dEdx");
    REGISTER_VARIABLE("CDCdEdx_chiD", CDCdEdx_chiD, "Chi value of duetrons from CDC dEdx");
    REGISTER_VARIABLE("SVD_p", SVD_p, "momentum valid in the SVD");
    REGISTER_VARIABLE("SVD_pTrue", SVD_pTrue, "true MC momentum valid in the SVD");
    REGISTER_VARIABLE("SVD_dedx", SVD_dedx, "SVD dE/dx truncated mean");
    REGISTER_VARIABLE("SVD_CosTheta", SVD_CosTheta, "cos(theta) of the track valid in the SVD");
    REGISTER_VARIABLE("SVD_nHits", SVD_nHits, "number of hits of the track valid in the SVD");

  }
}
