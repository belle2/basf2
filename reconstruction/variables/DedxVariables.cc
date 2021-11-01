/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// needed to build variables here
#include <reconstruction/variables/DedxVariables.h>
#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Conversion.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/Track.h>
#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <reconstruction/dataobjects/VXDDedxTrack.h>

#include <TString.h>

#include <cmath>

namespace Belle2 {

  /**
  * CDC dEdx value from particle
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

    double CDCdedx(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getDedx();
      }
    }

    double CDCdedxnosat(const Particle* part)
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

    double costhCDC(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getCosTheta();
      }
    }


    double CDCdEdx_nhits(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->size();
      }
    }

    double CDCdEdx_lnhits(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getNLayerHits();
      }
    }

    double CDCdEdx_lnhitsused(const Particle* part)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getNLayerHitsUsed();
      }
    }

    double CDCdEdx_llog(const Particle* part, const int index)
    {
      const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
      if (!dedxTrack) {
        return -999.0;
      } else {
        return dedxTrack->getLogl(index);
      }
    }

    double CDCdEdx_llogE(const Particle* part)
    {
      return CDCdEdx_llog(part, 0);
    }

    double CDCdEdx_llogMu(const Particle* part)
    {
      return CDCdEdx_llog(part, 1);
    }

    double CDCdEdx_llogPi(const Particle* part)
    {
      return CDCdEdx_llog(part, 2);
    }

    double CDCdEdx_llogK(const Particle* part)
    {
      return CDCdEdx_llog(part, 3);
    }

    double CDCdEdx_llogP(const Particle* part)
    {
      return CDCdEdx_llog(part, 4);
    }

    double CDCdEdx_llogD(const Particle* part)
    {
      return CDCdEdx_llog(part, 5);
    }



    Manager::FunctionPtr CDCdEdx_PIDvars(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 2) {
        B2ERROR("Min two arguments required to get variables related chi, predicted mean and reso");
        return nullptr;
      }

      TString var = "";
      try {
        var = arguments[0];
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of variable must be a variable name (chi or pmean or preso)");
        return nullptr;
      }

      int pdgCode;
      try {
        pdgCode = Belle2::convertString<int>(arguments[1]);
      } catch (std::invalid_argument& e) {
        B2ERROR("Second argument of variable must be a PDG code");
        return nullptr;
      }

      int index  = -999;
      if (abs(pdgCode) == Const::electron.getPDGCode())index = 0;
      else if (abs(pdgCode) == Const::muon.getPDGCode())index = 1;
      else if (abs(pdgCode) == Const::pion.getPDGCode())index = 2;
      else if (abs(pdgCode) == Const::kaon.getPDGCode())index = 3;
      else if (abs(pdgCode) == Const::proton.getPDGCode())index = 4;
      else if (abs(pdgCode) == Const::deuteron.getPDGCode())index = 5;

      auto func = [index, var](const Particle * part) -> double {
        const CDCDedxTrack* dedxTrack = getDedxFromParticle(part);
        if (!dedxTrack)
        {
          return std::numeric_limits<float>::quiet_NaN();
        } else {
          if (var == "chi") return dedxTrack->getChi(index);
          else if (var == "pmean") return dedxTrack->getPmean(index);
          else if (var == "preso") return dedxTrack->getPreso(index);
          else return std::numeric_limits<float>::quiet_NaN();
        }
      };
      return func;
    }

    double CDCdEdx_chiE(const Particle* part)
    {
      return std::get<double>(Manager::Instance().getVariable("CDCdEdx_PIDvars(chi, 11)")->function(part));
    }

    double CDCdEdx_chiMu(const Particle* part)
    {
      return std::get<double>(Manager::Instance().getVariable("CDCdEdx_PIDvars(chi, 13)")->function(part));
    }

    double CDCdEdx_chiPi(const Particle* part)
    {
      return std::get<double>(Manager::Instance().getVariable("CDCdEdx_PIDvars(chi, 211)")->function(part));
    }

    double CDCdEdx_chiK(const Particle* part)
    {
      return std::get<double>(Manager::Instance().getVariable("CDCdEdx_PIDvars(chi, 321)")->function(part));
    }

    double CDCdEdx_chiP(const Particle* part)
    {
      return std::get<double>(Manager::Instance().getVariable("CDCdEdx_PIDvars(chi, 2212)")->function(part));
    }

    double CDCdEdx_chiD(const Particle* part)
    {
      return std::get<double>(Manager::Instance().getVariable("CDCdEdx_PIDvars(chi, 1000010020)")->function(part));
    }

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

    VARIABLE_GROUP("Dedx");
    //CDC variables
    REGISTER_VARIABLE("CDCdEdx", CDCdedx, "CDC dE/dx truncated mean");
    REGISTER_VARIABLE("CDCdEdxnosat", CDCdedxnosat,
                      "CDC dE/dx truncated mean without saturation correction (NA for current track level MC)");
    REGISTER_VARIABLE("pCDC", pCDC, "Momentum valid in the CDC");
    REGISTER_VARIABLE("costhCDC", costhCDC, "costheta valid in the CDC");
    REGISTER_VARIABLE("CDCdEdx_nhits", CDCdEdx_nhits, "total hits of dedx track");
    REGISTER_VARIABLE("CDCdEdx_lnhits", CDCdEdx_lnhits, "layer hits for dedx track");
    REGISTER_VARIABLE("CDCdEdx_lnhitsused", CDCdEdx_lnhitsused, "truncated hits of dedx track");

    REGISTER_METAVARIABLE("CDCdEdx_PIDvars(var,PDG) var (= chi or pmean or preso) and PDG is of charged particles", CDCdEdx_PIDvars,
                          "advance CDC dEdx PID related variables for charged particle", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("CDCdEdx_chiE", CDCdEdx_chiE, "Chi value of electrons from CDC dEdx");
    REGISTER_VARIABLE("CDCdEdx_chiMu", CDCdEdx_chiMu, "Chi value of muons from CDC dEdx");
    REGISTER_VARIABLE("CDCdEdx_chiPi", CDCdEdx_chiPi, "Chi value of pions from CDC dEdx");
    REGISTER_VARIABLE("CDCdEdx_chiK", CDCdEdx_chiK, "Chi value of kaons from CDC dEdx");
    REGISTER_VARIABLE("CDCdEdx_chiP", CDCdEdx_chiP, "Chi value of protons from CDC dEdx");
    REGISTER_VARIABLE("CDCdEdx_chiD", CDCdEdx_chiD, "Chi value of duetrons from CDC dEdx");

    REGISTER_VARIABLE("CDCdEdx_llogE", CDCdEdx_llogE, "Log likelihood value of electrons from CDC dEdx");
    REGISTER_VARIABLE("CDCdEdx_llogMu", CDCdEdx_llogMu, "Log likelihood value of muons from CDC dEdx");
    REGISTER_VARIABLE("CDCdEdx_llogPi", CDCdEdx_llogPi, "Log likelihood value of pions from CDC dEdx");
    REGISTER_VARIABLE("CDCdEdx_llogK", CDCdEdx_llogK, "Log likelihood value of kaons from CDC dEdx");
    REGISTER_VARIABLE("CDCdEdx_llogP", CDCdEdx_llogP, "Log likelihood value of protons from CDC dEdx");
    REGISTER_VARIABLE("CDCdEdx_llogD", CDCdEdx_llogD, "Log likelihood value of duetrons from CDC dEdx");

    //SVD variables
    REGISTER_VARIABLE("SVDdEdx", SVDdedx, "SVD dE/dx truncated mean");
    REGISTER_VARIABLE("pSVD", SVD_p, "momentum valid in the SVD");
    REGISTER_VARIABLE("SVD_pTrue", SVD_pTrue, "true MC momentum valid in the SVD");
    REGISTER_VARIABLE("SVD_CosTheta", SVD_CosTheta, "cos(theta) of the track valid in the SVD");
    REGISTER_VARIABLE("SVD_nHits", SVD_nHits, "number of hits of the track valid in the SVD");

  }
}
