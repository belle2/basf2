/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/utility/PSelectorFunctions.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <analysis/utility/VariableManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <generators/dataobjects/MCParticle.h>
#include <reconstruction/dataobjects/PIDLikelihood.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <iostream>
#include <math.h>

using namespace std;

namespace Belle2 {
  namespace analysis {

    // momentum (lab) -------------------------------------------

    double particleP(const Particle* part)
    {
      return part->getP();
    }

    double particlePx(const Particle* part)
    {
      return part->getPx();
    }

    double particlePy(const Particle* part)
    {
      return part->getPy();
    }

    double particlePz(const Particle* part)
    {
      return part->getPz();
    }

    double particlePt(const Particle* part)
    {
      float px = part->getPx();
      float py = part->getPy();
      return sqrt(px * px + py * py);
    }

    double particleCosTheta(const Particle* part)
    {
      return part->getPz() / part->getP();
    }

    double particlePhi(const Particle* part)
    {
      return atan2(part->getPy(), part->getPx()) / Unit::deg;
    }

    // momentum (CMS) -----------------------------------------------

    double particlePStar(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.P();
    }

    double particlePxStar(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Px();
    }

    double particlePyStar(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Py();
    }

    double particlePzStar(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Pz();
    }

    double particlePtStar(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Pt();
    }

    double particleCosThetaStar(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.CosTheta();
    }

    double particlePhiStar(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Phi();
    }

    // vertex or POCA in respect to IP ------------------------------

    double particleDX(const Particle* part)
    {
      return part->getX() - 0; // TODO replace with IP position
    }

    double particleDY(const Particle* part)
    {
      return part->getY() - 0; // TODO replace with IP position
    }

    double particleDZ(const Particle* part)
    {
      return part->getZ() - 0; // TODO replace with IP position
    }

    double particleDRho(const Particle* part)
    {
      float x = part->getX() - 0; // TODO replace with IP position
      float y = part->getY() - 0; // TODO replace with IP position
      return sqrt(x * x + y * y);
    }

    // mass ------------------------------------------------------------

    double particleMass(const Particle* part)
    {
      return part->getMass();
    }

    double particleDMass(const Particle* part)
    {
      return part->getMass() - part->getPDGMass();
    }

    // released energy --------------------------------------------------

    double particleQ(const Particle* part)
    {
      float m = part->getMass();
      for (unsigned i = 0; i < part->getNDaughters(); i++) {
        const Particle* child = part->getDaughter(i);
        if (child) m -= child->getMass();
      }
      return m;
    }

    double particleDQ(const Particle* part)
    {
      float m = part->getMass() - part->getPDGMass();
      for (unsigned i = 0; i < part->getNDaughters(); i++) {
        const Particle* child = part->getDaughter(i);
        if (child) m -= (child->getMass() - child->getPDGMass());
      }
      return m;
    }

    // Mbc and deltaE

    double particleMbc(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      double E = T.getCMSEnergy() / 2;
      double m2 = E * E - vec.Vect().Mag2();
      double mbc = m2 > 0 ? sqrt(m2) : 0;
      return mbc;
    }

    double particleDeltaE(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.E() - T.getCMSEnergy() / 2;
    }

    // PID ---------------------------------------------

    double particleElectronId(const Particle*)
    {
      return 0.5; //TODO when eId availabe
    }

    double particleMuonId(const Particle*)
    {
      return 0.5; //TODO when muId availabe
    }

    double particlePionId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      /*
      // temporary: use dEdx only below 2.5 GeV/c
      Const::PIDDetectorSet set = Const::TOP + Const::ARICH;
      if (part->getP() < 2.5) set += Const::CDC;
      return pid->getProbability(Const::pion, Const::kaon, set);
      */
      return pid->getProbability(Const::pion, Const::kaon);

    }

    double particleKaonId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      /*
      // temporary: use dEdx only below 2.5 GeV/c
      Const::PIDDetectorSet set = Const::TOP + Const::ARICH;
      if (part->getP() < 2.5) set += Const::CDC;
      return pid->getProbability(Const::kaon, Const::pion, set);
      */
      return pid->getProbability(Const::kaon, Const::pion);

    }

    double particleProtonId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      /*
      // temporary: use dEdx only below 2.5 GeV/c
      Const::PIDDetectorSet set = Const::TOP + Const::ARICH;
      if (part->getP() < 2.5) set += Const::CDC;
      return pid->getProbability(Const::proton, Const::pion, set);
      */
      return pid->getProbability(Const::proton, Const::pion);
    }

    // other ------------------------------------------------------------

    double particlePvalue(const Particle* part)
    {
      return part->getPValue();
    }

    double particleNDaughters(const Particle* part)
    {
      return part->getNDaughters();
    }

    double particleFlavorType(const Particle* part)
    {
      return part->getFlavorType();
    }

    REGISTER_VARIABLE("p", particleP, "momentum magnitude");
    REGISTER_VARIABLE("px", particlePx, "momentum component x");
    REGISTER_VARIABLE("py", particlePy, "momentum component y");
    REGISTER_VARIABLE("pz", particlePz, "momentum component z");
    REGISTER_VARIABLE("pt", particlePt, "transverse momentum");
    REGISTER_VARIABLE("cosTheta", particleCosTheta, "momentum cosine of polar angle");
    REGISTER_VARIABLE("cth", particleCosTheta, "momentum cosine of polar angle");
    REGISTER_VARIABLE("phi", particlePhi, "momentum azimuthal angle in degrees");

    REGISTER_VARIABLE("p*", particlePStar, "CMS momentum magnitude");
    REGISTER_VARIABLE("px*", particlePxStar, "CMS momentum component x");
    REGISTER_VARIABLE("py*", particlePyStar, "CMS momentum component y");
    REGISTER_VARIABLE("pz*", particlePzStar, "CMS momentum component z");
    REGISTER_VARIABLE("pt*", particlePtStar, "CMS transverse momentum");
    REGISTER_VARIABLE("cosTheta*", particleCosThetaStar, "CMS momentum cosine of polar angle");
    REGISTER_VARIABLE("cth*", particleCosThetaStar, "CMS momentum cosine of polar angle");
    REGISTER_VARIABLE("phi*", particlePhiStar, "CMS momentum azimuthal angle in degrees");

    REGISTER_VARIABLE("dx", particleDX, "x in respect to IP");
    REGISTER_VARIABLE("dy", particleDY, "y in respect to IP");
    REGISTER_VARIABLE("dz", particleDZ, "z in respect to IP");
    REGISTER_VARIABLE("dr", particleDRho, "transverse distance in respect to IP");

    REGISTER_VARIABLE("M", particleMass, "mass");
    REGISTER_VARIABLE("dM", particleDMass, "mass minus nominal mass");
    REGISTER_VARIABLE("Q", particleQ, "released energy in decay");
    REGISTER_VARIABLE("dQ", particleDQ, "released energy in decay minus nominal one");
    REGISTER_VARIABLE("Mbc", particleMbc, "beam constrained mass");
    REGISTER_VARIABLE("deltaE", particleDeltaE, "energy difference");

    REGISTER_VARIABLE("eid", particleElectronId, "electron identification probability");
    REGISTER_VARIABLE("muid", particleMuonId, "muon identification probability");
    REGISTER_VARIABLE("piid", particlePionId, "pion identification probability");
    REGISTER_VARIABLE("Kid", particleKaonId, "kaon identification probability");
    REGISTER_VARIABLE("prid", particleProtonId, "proton identification probability");

    REGISTER_VARIABLE("chiProb", particlePvalue, "chi^2 probability of the fit");
    REGISTER_VARIABLE("nDaughters", particleNDaughters, "number of daughter particles");
    REGISTER_VARIABLE("flavor", particleFlavorType, "flavor type of decay (0=unflavored, 1=flavored)");

  }
}

