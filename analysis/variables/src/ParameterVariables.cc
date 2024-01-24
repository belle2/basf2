/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/variables/ParameterVariables.h>

// include VariableManager
#include <analysis/VariableManager/Manager.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ReferenceFrame.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/MCParticle.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <Math/Boost.h>
#include <Math/Vector4D.h>
#include <TVectorF.h>

#include <cmath>


namespace Belle2 {
  namespace Variable {

    bool almostContains(const std::vector<double>& vector, int value)
    {
      for (const auto& item : vector)
        if (std::abs(value - item) < 1e-3)
          return true;
      return false;
    }

    double RandomChoice(const Particle*, const std::vector<double>& choices)
    {
      int r = std::rand() % choices.size() + 1;
      auto it = choices.begin();
      std::advance(it, r);
      return *it;
    }

    int NumberOfMCParticlesInEvent(const Particle*, const std::vector<double>& pdgs)
    {
      StoreArray<MCParticle> mcParticles;
      int counter = 0;
      for (int i = 0; i < mcParticles.getEntries(); ++i) {
        if (mcParticles[i]->getStatus(MCParticle::c_PrimaryParticle) and almostContains(pdgs, std::abs(mcParticles[i]->getPDG())))
          counter++;
      }
      return counter;
    }

    double isAncestorOf(const Particle* part, const std::vector<double>& daughterIDs)
    {
      if (part == nullptr)
        return Const::doubleNaN;

      // If particle has no MC relation, MC chain doesn't exist
      const MCParticle* mcpart = part->getMCParticle();
      if (mcpart == nullptr)
        return Const::doubleNaN;

      if (daughterIDs.empty())
        B2FATAL("Wrong number of arguments for parameter function isAncestorOf. At least one needed!");

      // Get to the daughter of interest
      const Particle* curParticle = part;
      double isAncestor = 0.0;

      for (unsigned int i = 0; i < daughterIDs.size(); i++) {
        int nCurDaughters = curParticle->getNDaughters();
        if (nCurDaughters == 0)
          B2FATAL("Assumed mother of particle at argument " << i << " has no daughters!");
        if (daughterIDs[i] >= nCurDaughters)
          B2FATAL("Assumed mother of particle at argument " << i << " has only " << nCurDaughters
                  << " daughters, but daughter at position " << daughterIDs[i] << " expected!");
        const Particle* curDaughter = curParticle->getDaughter(daughterIDs[i]);
        if (curDaughter == nullptr)
          return Const::doubleNaN;
        curParticle = curDaughter;
      }

      // Daughter obtained, get MC particle of daughter
      const MCParticle* finalMCDaughter = curParticle->getMCParticle();
      if (finalMCDaughter == nullptr)
        return Const::doubleNaN;

      // Go up the MC chain, check for ancestor
      const MCParticle* curMCParticle = finalMCDaughter;

      while (curMCParticle != nullptr) {
        const MCParticle* curMCMother = curMCParticle->getMother();
        if (curMCMother == nullptr)
          return 0.0;
        else {
          if (curMCMother->getArrayIndex() == mcpart->getArrayIndex()) {
            isAncestor++;
            break;
          } else {
            curMCParticle = curMCMother;
            isAncestor++;
          }
        }
      }
      return isAncestor;
    }

    double hasAncestor(const Particle* part, const std::vector<double>& args)
    {
      if (part == nullptr)
        return Const::doubleNaN;

      // If particle has no MC relation, MC chain doesn't exist
      const MCParticle* mcpart = part->getMCParticle();
      if (mcpart == nullptr)
        return Const::doubleNaN;

      int m_PDG, m_sign = 0;

      if (args.empty())
        B2FATAL("Wrong number of arguments for variable hasAncestor!");
      else if (args.size() == 1) {
        if (args[0] == 0)
          B2FATAL("PDG code in variable hasAncestor is 0!");
        else
          m_PDG = args[0];
      } else if (args.size() == 2) {
        if (args[0] == 0 or (args[1] != 0 and args[1] != 1))
          B2FATAL("PDG code in variable hasAncestor is 0 or second argument is not 0 or 1!");
        else {
          m_PDG = args[0];
          m_sign = args[1];
        }
      } else {
        B2FATAL("Too many arguments for variable hasAncestor!");
      }

      unsigned int nLevels = 0;

      const MCParticle* curMCParticle = mcpart;

      while (curMCParticle != nullptr) {
        const MCParticle* curMCMother = curMCParticle->getMother();
        if (curMCMother == nullptr)
          return 0;

        int pdg = curMCMother->getPDG();
        if (m_sign == 0)
          pdg = abs(pdg);

        if (pdg == m_PDG) {
          ++nLevels;
          break;
        } else {
          ++nLevels;
          curMCParticle = curMCMother;
        }
      }
      return nLevels;
    }


    double daughterInvariantMass(const Particle* particle, const std::vector<double>& daughter_indexes)
    {
      if (!particle)
        return Const::doubleNaN;

      ROOT::Math::PxPyPzEVector sum;
      const auto& daughters = particle->getDaughters();
      int nDaughters = static_cast<int>(daughters.size());

      for (auto& double_daughter : daughter_indexes) {
        long daughter = std::lround(double_daughter);
        if (daughter >= nDaughters)
          return Const::doubleNaN;

        sum += daughters[daughter]->get4Vector();
      }

      return sum.M();
    }

    double daughterMCInvariantMass(const Particle* particle, const std::vector<double>& daughter_indexes)
    {
      if (!particle)
        return Const::doubleNaN;

      ROOT::Math::PxPyPzEVector sum;
      const auto& daughters = particle->getDaughters();
      int nDaughters = static_cast<int>(daughters.size());

      for (auto& double_daughter : daughter_indexes) {
        long daughter = std::lround(double_daughter);
        if (daughter >= nDaughters)
          return Const::doubleNaN;

        const MCParticle* mcdaughter = daughters[daughter]->getMCParticle();
        if (!mcdaughter)
          return Const::doubleNaN;

        sum += mcdaughter->get4Vector();
      }

      return sum.M();
    }


    double massDifference(const Particle* particle, const std::vector<double>& daughters)
    {
      if (!particle)
        return Const::doubleNaN;

      long daughter = std::lround(daughters[0]);
      if (daughter >= static_cast<int>(particle->getNDaughters()))
        return Const::doubleNaN;

      double motherMass = particle->getMass();
      double daughterMass = particle->getDaughter(daughter)->getMass();

      return motherMass - daughterMass;
    }

    double massDifferenceError(const Particle* particle, const std::vector<double>& daughters)
    {
      if (!particle)
        return Const::doubleNaN;

      long daughter = std::lround(daughters[0]);
      if (daughter >= static_cast<int>(particle->getNDaughters()))
        return Const::doubleNaN;

      float result = 0.0;

      ROOT::Math::PxPyPzEVector thisDaughterMomentum = particle->getDaughter(daughter)->get4Vector();

      TMatrixFSym thisDaughterCovM(Particle::c_DimMomentum);
      thisDaughterCovM = particle->getDaughter(daughter)->getMomentumErrorMatrix();
      TMatrixFSym othrDaughterCovM(Particle::c_DimMomentum);

      for (int j = 0; j < int(particle->getNDaughters()); ++j) {
        if (j == daughter)
          continue;

        othrDaughterCovM += particle->getDaughter(j)->getMomentumErrorMatrix();
      }

      TMatrixFSym covarianceMatrix(2 * Particle::c_DimMomentum);
      covarianceMatrix.SetSub(0, thisDaughterCovM);
      covarianceMatrix.SetSub(4, othrDaughterCovM);

      double motherMass = particle->getMass();
      double daughterMass = particle->getDaughter(daughter)->getMass();

      TVectorF    jacobian(2 * Particle::c_DimMomentum);
      jacobian[0] =  thisDaughterMomentum.Px() / daughterMass - particle->getPx() / motherMass;
      jacobian[1] =  thisDaughterMomentum.Py() / daughterMass - particle->getPy() / motherMass;
      jacobian[2] =  thisDaughterMomentum.Pz() / daughterMass - particle->getPz() / motherMass;
      jacobian[3] =  particle->getEnergy() / motherMass - thisDaughterMomentum.E() / daughterMass;
      jacobian[4] = -1.0 * particle->getPx() / motherMass;
      jacobian[5] = -1.0 * particle->getPy() / motherMass;
      jacobian[6] = -1.0 * particle->getPz() / motherMass;
      jacobian[7] =  1.0 * particle->getEnergy() / motherMass;

      result = jacobian * (covarianceMatrix * jacobian);

      if (result < 0.0)
        result = 0.0;

      return TMath::Sqrt(result);
    }

    double massDifferenceSignificance(const Particle* particle, const std::vector<double>& daughters)
    {
      if (!particle)
        return Const::doubleNaN;

      long daughter = std::lround(daughters[0]);
      if (daughter >= static_cast<int>(particle->getNDaughters()))
        return Const::doubleNaN;

      double massDiff = massDifference(particle, daughters);
      double massDiffErr = massDifferenceError(particle, daughters);

      double massDiffNominal = particle->getPDGMass() - particle->getDaughter(daughter)->getPDGMass();

      return (massDiff - massDiffNominal) / massDiffErr;
    }

    // Decay Kinematics -------------------------------------------------------
    double particleDecayAngle(const Particle* particle, const std::vector<double>& daughters)
    {
      if (!particle)
        return Const::doubleNaN;

      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector m = - T.getBeamFourMomentum();

      ROOT::Math::PxPyPzEVector motherMomentum = particle->get4Vector();
      B2Vector3D                motherBoost    = motherMomentum.BoostToCM();

      long daughter = std::lround(daughters[0]);
      if (daughter >= static_cast<int>(particle->getNDaughters()))
        return Const::doubleNaN;

      ROOT::Math::PxPyPzEVector daugMomentum = particle->getDaughter(daughter)->get4Vector();
      daugMomentum = ROOT::Math::Boost(motherBoost) * daugMomentum;

      m = ROOT::Math::Boost(motherBoost) * m;

      return B2Vector3D(daugMomentum.Vect()).Angle(B2Vector3D(m.Vect()));
    }

    double pointingAngle(const Particle* particle, const std::vector<double>& daughters)
    {
      if (!particle)
        return Const::doubleNaN;

      long daughter = std::lround(daughters[0]);
      if (daughter >= static_cast<int>(particle->getNDaughters()))
        return Const::doubleNaN;

      if (particle->getDaughter(daughter)->getNDaughters() < 2)
        return Const::doubleNaN;

      B2Vector3D productionVertex = particle->getVertex();
      B2Vector3D decayVertex = particle->getDaughter(daughter)->getVertex();

      B2Vector3D vertexDiffVector = decayVertex - productionVertex;

      const auto& frame = ReferenceFrame::GetCurrent();
      B2Vector3D daughterMomentumVector = frame.getMomentum(particle->getDaughter(daughter)).Vect();

      return daughterMomentumVector.Angle(vertexDiffVector);
    }

    double azimuthalAngleInDecayPlane(const Particle* particle, const std::vector<double>& daughters)
    {
      if (!particle)
        return Const::doubleNaN;

      int nDaughters = static_cast<int>(particle->getNDaughters());

      long daughter1 = std::lround(daughters[0]);
      long daughter2 = std::lround(daughters[1]);
      if (daughter1 >= nDaughters || daughter2 >= nDaughters)
        return Const::doubleNaN;

      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector m = T.getBeamFourMomentum();
      ROOT::Math::PxPyPzEVector p = particle->get4Vector();
      ROOT::Math::PxPyPzEVector d1 = particle->getDaughter(daughter1)->get4Vector();
      ROOT::Math::PxPyPzEVector d2 = particle->getDaughter(daughter2)->get4Vector();

      ROOT::Math::PxPyPzEVector l;
      l.SetPx(p.Py() * (d1.Pz() * d2.E()  - d1.E()  * d2.Pz()) + p.Pz() * (d1.E()  * d2.Py() - d1.Py() * d2.E())
              + p.E()  * (d1.Py() * d2.Pz() - d1.Pz() * d2.Py()));
      l.SetPy(p.Px() * (d1.E()  * d2.Pz() - d1.Pz() * d2.E())  + p.Pz() * (d1.Px() * d2.E()  - d1.E()  * d2.Px())
              + p.E()  * (d1.Pz() * d2.Px() - d1.Px() * d2.Pz()));
      l.SetPz(p.Px() * (d1.Py() * d2.E()  - d1.E()  * d2.Py()) + p.Py() * (d1.E()  * d2.Px() - d1.Px() * d2.E())
              + p.E()  * (d1.Px() * d2.Py() - d1.Py() * d2.Px()));
      l.SetE(-(p.Px() * (d1.Pz() * d2.Py() - d1.Py() * d2.Pz()) + p.Py() * (d1.Px() * d2.Pz() - d1.Pz() * d2.Px())
               + p.Pz() * (d1.Py() * d2.Px() - d1.Px() * d2.Py())));

      double m_times_p = m.Dot(p);
      double m_times_l = m.Dot(l);
      double m_times_d1 = m.Dot(d1);
      double l_times_d1 = l.Dot(d1);
      double d1_times_p = d1.Dot(p);
      double m_abs = TMath::Sqrt(pow(m_times_p / p.M(), 2) - m.M2());
      double d1_abs = TMath::Sqrt(pow(d1_times_p / p.M(), 2) - d1.M2());
      double cos_phi = -m_times_l / (m_abs * TMath::Sqrt(-l.M2()));
      double m_parallel_abs = m_abs * TMath::Sqrt(1 - cos_phi * cos_phi);
      double m_parallel_times_d1 = m_times_p * d1_times_p / p.M2() + m_times_l * l_times_d1 / l.M2() - m_times_d1;

      return TMath::ACos(-m_parallel_times_d1 / (m_parallel_abs * d1_abs));
    }

    double Constant(const Particle*, const std::vector<double>& constant)
    {
      return constant[0];
    }



    VARIABLE_GROUP("ParameterFunctions");
    REGISTER_VARIABLE("NumberOfMCParticlesInEvent(pdgcode)", NumberOfMCParticlesInEvent, R"DOC(
                      [Eventbased] Returns number of MC Particles (including anti-particles) with the given pdgcode in the event.

                      Used in the FEI to determine to calculate reconstruction efficiencies.

                      The variable is event-based and does not need a valid particle pointer as input.)DOC");
    REGISTER_VARIABLE("isAncestorOf(i, j, ...)", isAncestorOf, R"DOC(
                      Returns a positive integer if daughter at position particle->daughter(i)->daughter(j)... is an ancestor of the related MC particle, 0 otherwise.

                      Positive integer represents the number of steps needed to get from final MC daughter to ancestor.
                      If any particle or MCparticle is a nullptr, NaN is returned. If MC relations of any particle doesn't exist, -1.0 is returned.)DOC");
    REGISTER_VARIABLE("hasAncestor(PDG, abs)", hasAncestor, R"DOC(

                      Returns a positive integer if an ancestor with the given PDG code is found, 0 otherwise.

                      The integer is the level where the ancestor was found, 1: first mother, 2: grandmother, etc.

                      Second argument is optional, 1 means that the sign of the PDG code is taken into account, default is 0.

                      If there is no MC relations found, -1 is returned. In case of nullptr particle, NaN is returned.)DOC");
    REGISTER_VARIABLE("daughterInvariantMass(i, j, ...)", daughterInvariantMass, R"DOC(
Returns invariant mass of the given daughter particles. E.g.:

* daughterInvariantMass(0, 1) returns the invariant mass of the first and second daughter.
* daughterInvariantMass(0, 1, 2) returns the invariant mass of the first, second and third daughter.

Useful to identify intermediate resonances in a decay, which weren't reconstructed explicitly.

Returns NaN if particle is nullptr or if the given daughter-index is out of bound (>= number of daughters).

)DOC", "GeV/:math:`\\text{c}^2`");
    MAKE_DEPRECATED("daughterInvariantMass", false, "light-2203-zeus", R"DOC(
                     The variable `daughterInvM` provides exactly the same functionality.)DOC");
    REGISTER_VARIABLE("daughterMCInvariantMass(i, j, ...)", daughterMCInvariantMass, R"DOC(
Returns true invariant mass of the given daughter particles, same behaviour as daughterInvariantMass variable.

)DOC", "GeV/:math:`\\text{c}^2`");
    REGISTER_VARIABLE("decayAngle(i)", particleDecayAngle, R"DOC(
Angle in the mother's rest frame between the reverted CMS momentum vector and the direction of the i-th daughter

)DOC", "rad");
    REGISTER_VARIABLE("pointingAngle(i)", pointingAngle, R"DOC(
Angle between i-th daughter's momentum vector and vector connecting production and decay vertex of i-th daughter.
This makes only sense if the i-th daughter has itself daughter particles and therefore a properly defined vertex.

)DOC", "rad");
    REGISTER_VARIABLE("azimuthalAngleInDecayPlane(i, j)", azimuthalAngleInDecayPlane, R"DOC(
Azimuthal angle of i-th daughter in decay plane towards projection of particle momentum into decay plane.

First we define the following symbols:

* P: four-momentum vector of decaying particle in whose decay plane the azimuthal angle is measured
* M: "mother" of p, however not necessarily the direct mother but any higher state, here the CMS itself is chosen
* D1: daughter for which the azimuthal angle is supposed to be calculated
* D2: another daughter needed to span the decay plane
* L: normal to the decay plane (four-component vector)

L can be defined via the following relation:

.. math:: L^{\sigma} = \delta^{\sigma\nu} \epsilon_{\mu\nu\alpha\beta} P^{\mu}D1^{\alpha}D2^{\beta}

The azimuthal angle is given by

.. math:: \phi \equiv \cos^{-1} \left(\frac{-\vec{M_{\parallel}} \cdot \vec{D1}}{|\vec{M_{\parallel}}| \cdot |\vec{D1}|}\right)

For a frame independent formulation the three component vectors need to be written via invariant four-momentum vectors.

.. math::

  -\vec{M_{\parallel}} \cdot \vec{D1} &= \biggl[M - \frac{(M \cdot L)L}{L^2}\biggr] \cdot D1 - \frac{(M \cdot P)(D1 \cdot P)}{m^2_P}\\
  |\vec{M_{\parallel}}| &= |\vec{M}| \sqrt{1 - \cos^2 \psi}\\
  |\vec{M}| &= \sqrt{\frac{(M \cdot P)^2}{m^2_P} - m^2_M}\\
  \cos \psi &= \frac{\vec{M} \cdot \vec{L}}{|\vec{M}| \cdot |\vec{L}|} = \frac{-M \cdot L}{|\vec{M}| \cdot \sqrt{-L^2}}\\
  |\vec{D1}| &= \sqrt{\frac{(D1 \cdot P)^2}{m^2_P} - m^2_{D1}}

)DOC", "rad");

    REGISTER_VARIABLE("massDifference(i)", massDifference, "Difference in invariant masses of this particle and its i-th daughter\n\n",
                      "GeV/:math:`\\text{c}^2`");
    REGISTER_VARIABLE("massDifferenceError(i)", massDifferenceError,
                      "Estimated uncertainty on difference in invariant masses of this particle and its i-th daughter\n\n", "GeV/:math:`\\text{c}^2`");
    REGISTER_VARIABLE("massDifferenceSignificance(i)", massDifferenceSignificance,
                      "Signed significance of the deviation from the nominal mass difference of this particle and its i-th daughter [(massDiff - NOMINAL_MASS_DIFF)/ErrMassDiff]");

    REGISTER_VARIABLE("constant(float i)", Constant, R"DOC(
                      Returns i.

                      Useful for debugging purposes and in conjunction with the formula meta-variable.)DOC");

    REGISTER_VARIABLE("randomChoice(i, j, ...)", RandomChoice, R"DOC(
                      Returns random element of given numbers.

                      Useful for testing purposes.)DOC");


  }
}
