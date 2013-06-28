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
      if (!part) return 0;
      return part->getP();
    }

    double particlePx(const Particle* part)
    {
      if (!part) return 0;
      return part->getPx();
    }

    double particlePy(const Particle* part)
    {
      if (!part) return 0;
      return part->getPy();
    }

    double particlePz(const Particle* part)
    {
      if (!part) return 0;
      return part->getPz();
    }

    double particlePt(const Particle* part)
    {
      if (!part) return 0;
      float px = part->getPx();
      float py = part->getPy();
      return sqrt(px * px + py * py);
    }

    double particleCosTheta(const Particle* part)
    {
      if (!part) return 0;
      return part->getPz() / part->getP();
    }

    double particlePhi(const Particle* part)
    {
      if (!part) return 0;
      return atan2(part->getPy(), part->getPx()) / Unit::deg;
    }

    // momentum (CMS) -----------------------------------------------

    double particlePStar(const Particle* part)
    {
      if (!part) return 0;
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.P();
    }

    double particlePxStar(const Particle* part)
    {
      if (!part) return 0;
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Px();
    }

    double particlePyStar(const Particle* part)
    {
      if (!part) return 0;
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Py();
    }

    double particlePzStar(const Particle* part)
    {
      if (!part) return 0;
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Pz();
    }

    double particlePtStar(const Particle* part)
    {
      if (!part) return 0;
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Pt();
    }

    double particleCosThetaStar(const Particle* part)
    {
      if (!part) return 0;
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.CosTheta();
    }

    double particlePhiStar(const Particle* part)
    {
      if (!part) return 0;
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Phi();
    }

    // vertex or POCA in respect to IP ------------------------------

    double particleDX(const Particle* part)
    {
      if (!part) return 0;
      return part->getX() - 0; // TODO replace with IP position
    }

    double particleDY(const Particle* part)
    {
      if (!part) return 0;
      return part->getY() - 0; // TODO replace with IP position
    }

    double particleDZ(const Particle* part)
    {
      if (!part) return 0;
      return part->getZ() - 0; // TODO replace with IP position
    }

    double particleDRho(const Particle* part)
    {
      if (!part) return 0;
      float x = part->getX() - 0; // TODO replace with IP position
      float y = part->getY() - 0; // TODO replace with IP position
      return sqrt(x * x + y * y);
    }

    // mass ------------------------------------------------------------

    double particleMass(const Particle* part)
    {
      if (!part) return 0;
      return part->getMass();
    }

    double particleDMass(const Particle* part)
    {
      if (!part) return 0;
      return part->getMass() - part->getPDGMass();
    }

    // released energy --------------------------------------------------

    double particleQ(const Particle* part)
    {
      if (!part) return 0;
      float m = part->getMass();
      for (unsigned i = 0; i < part->getNDaughters(); i++) {
        const Particle* child = part->getDaughter(i);
        if (child) m -= child->getMass();
      }
      return m;
    }

    double particleDQ(const Particle* part)
    {
      if (!part) return 0;
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
      if (!part) return 0;
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      double E = T.getCMSEnergy() / 2;
      double m2 = E * E - vec.Vect().Mag2();
      double mbc = m2 > 0 ? sqrt(m2) : 0;
      return mbc;
    }

    double particleDeltaE(const Particle* part)
    {
      if (!part) return 0;
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.E() - T.getCMSEnergy() / 2;
    }

    // PID ---------------------------------------------

    double particleElectronId(const Particle* part)
    {
      if (!part) return 0;
      return 0.5; //TODO when eId availabe
    }

    double particleMuonId(const Particle* part)
    {
      if (!part) return 0;
      return 0.5; //TODO when muId availabe
    }

    double particlePionId(const Particle* part)
    {
      if (!part) return 0;
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      // temporary: use dEdx only below 2.5 GeV/c
      Const::PIDDetectorSet set = Const::TOP + Const::ARICH;
      if (part->getP() < 2.5) set += Const::CDC;
      return pid->getProbability(Const::pion, Const::kaon, set);

    }

    double particleKaonId(const Particle* part)
    {
      if (!part) return 0;
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      // temporary: use dEdx only below 2.5 GeV/c
      Const::PIDDetectorSet set = Const::TOP + Const::ARICH;
      if (part->getP() < 2.5) set += Const::CDC;
      return pid->getProbability(Const::kaon, Const::pion, set);
    }

    double particleProtonId(const Particle* part)
    {
      if (!part) return 0;
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      // temporary: use dEdx only below 2.5 GeV/c
      Const::PIDDetectorSet set = Const::TOP + Const::ARICH;
      if (part->getP() < 2.5) set += Const::CDC;
      return pid->getProbability(Const::proton, Const::pion, set);
    }

    // other ------------------------------------------------------------

    double particlePvalue(const Particle* part)
    {
      if (!part) return 0;
      return part->getPValue();
    }

    double particleNchilds(const Particle* part)
    {
      if (!part) return 0;
      return part->getNDaughters();
    }

    double particleFlavorType(const Particle* part)
    {
      if (!part) return 0;
      return part->getFlavorType();
    }


  }
}

