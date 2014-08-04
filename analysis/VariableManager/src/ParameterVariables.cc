/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VariableManager/ParameterVariables.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <TLorentzVector.h>
#include <TVectorF.h>
#include <TVector3.h>

namespace Belle2 {
  namespace Variable {

    double NumberOfMCParticlesInEvent(const Particle*, const std::vector<double>& pdg)
    {
      StoreArray<MCParticle> mcParticles;
      int counter = 0;
      for (int i = 0; i < mcParticles.getEntries(); ++i) {
        if (mcParticles[i]->getPDG() == pdg[0])
          counter++;
      }
      return counter;
    }

    double daughterInvariantMass(const Particle* particle, const std::vector<double>& daughter_indexes)
    {
      TLorentzVector sum;
      const std::vector<Particle*> daughters = particle->getDaughters();
      for (auto & double_daughter : daughter_indexes) {
        int daughter = static_cast<int>(double_daughter + 0.5);
        sum += daughters[daughter]->get4Vector();
      }

      return sum.M();
    }

    // Decay Kinematics -------------------------------------------------------
    double particleDecayAngle(const Particle* particle, const std::vector<double>& daughters)
    {
      double result = 0.0;

      TLorentzVector motherMomentum = particle->get4Vector();
      TVector3       motherBoost    = -(motherMomentum.BoostVector());

      int daughter = static_cast<int>(daughters[0] + 0.5);
      TLorentzVector daugMomentum = particle->getDaughter(daughter)->get4Vector();
      daugMomentum.Boost(motherBoost);

      result = cos(daugMomentum.Angle(motherMomentum.Vect()));

      return result;
    }

    double particleDaughterAngle(const Particle* particle, const std::vector<double>& daughters)
    {
      if (particle->getNDaughters() != 2)
        return 0.0;

      int daughter1 = static_cast<int>(daughters[0] + 0.5);
      int daughter2 = static_cast<int>(daughters[1] + 0.5);
      const TVector3 a = particle->getDaughter(daughter1)->getMomentum();
      const TVector3 b = particle->getDaughter(daughter2)->getMomentum();
      return cos(a.Angle(b));
    }

    VARIABLE_GROUP("ParameterFunctions");
    REGISTER_VARIABLE("NumberOfMCParticlesInEvent(pdg)", NumberOfMCParticlesInEvent , "Returns number of MC Particles with the given pdg in the event.");
    REGISTER_VARIABLE("daughterInvariantMass(i, j, ...)", daughterInvariantMass , "Returns invariant mass of the given daughter particles.");
    REGISTER_VARIABLE("decayAngle(i)", particleDecayAngle, "cosine of the angle between the mother momentum vector and the direction of the i-th daughter in the mother's rest frame");
    REGISTER_VARIABLE("daughterAngle(i,j)", particleDaughterAngle, "cosine of the angle between i-th and j-th daughters, in lab frame");

  }
}
