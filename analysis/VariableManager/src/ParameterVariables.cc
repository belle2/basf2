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
#include <analysis/dataobjects/ContinuumSuppression.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ReferenceFrame.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/dataobjects/Helix.h>

#include <TLorentzVector.h>
#include <TVectorF.h>
#include <TVector3.h>

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

    double NumberOfMCParticlesInEvent(const Particle*, const std::vector<double>& pdgs)
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
        return -999;

      // If particle has no MC relation, MC chain doesn't exist
      const MCParticle* mcpart = part->getRelatedTo<MCParticle>();
      if (mcpart == nullptr)
        return -1.0;

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
          return -999;
        curParticle = curDaughter;
      }

      // Daughter obtained, get MC particle of daughter
      const MCParticle* finalMCDaughter = curParticle->getRelatedTo<MCParticle>();
      if (finalMCDaughter == nullptr)
        return -1.0;

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
        return -999;

      // If particle has no MC relation, MC chain doesn't exist
      const MCParticle* mcpart = part->getRelatedTo<MCParticle>();
      if (mcpart == nullptr)
        return -1.0;

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

    double genNthMotherPDG(const Particle* part, const std::vector<double>& args)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      unsigned int nLevels;
      if (args.empty())
        nLevels = 0;
      else
        nLevels = args[0];

      const MCParticle* curMCParticle = mcparticle;
      for (unsigned int i = 0; i <= nLevels; i++) {
        const MCParticle* curMCMother = curMCParticle->getMother();
        if (curMCMother == nullptr)
          return 0.0;
        curMCParticle = curMCMother;
      }
      int m_pdg = curMCParticle->getPDG();
      return m_pdg;
    }

    double genNthMotherIndex(const Particle* part, const std::vector<double>& args)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      unsigned int nLevels;
      if (args.empty())
        nLevels = 0;
      else
        nLevels = args[0];

      const MCParticle* curMCParticle = mcparticle;
      for (unsigned int i = 0; i <= nLevels; i++) {
        const MCParticle* curMCMother = curMCParticle->getMother();
        if (curMCMother == nullptr)
          return 0.0;
        curMCParticle = curMCMother;
      }
      int m_id = curMCParticle->getArrayIndex();
      return m_id;
    }

    double daughterInvariantMass(const Particle* particle, const std::vector<double>& daughter_indexes)
    {
      if (!particle)
        return -999;

      TLorentzVector sum;
      const auto& daughters = particle->getDaughters();
      int nDaughters = static_cast<int>(daughters.size());

      for (auto& double_daughter : daughter_indexes) {
        long daughter = std::lround(double_daughter);
        if (daughter >= nDaughters)
          return -999;

        sum += daughters[daughter]->get4Vector();
      }

      return sum.M();
    }

    double daughterMCInvariantMass(const Particle* particle, const std::vector<double>& daughter_indexes)
    {
      if (!particle)
        return -999;

      TLorentzVector sum;
      const auto& daughters = particle->getDaughters();
      int nDaughters = static_cast<int>(daughters.size());

      for (auto& double_daughter : daughter_indexes) {
        long daughter = std::lround(double_daughter);
        if (daughter >= nDaughters)
          return -999;

        const MCParticle* mcdaughter = daughters[daughter]->getRelated<MCParticle>();
        if (!mcdaughter)
          return -999;

        sum += mcdaughter->get4Vector();
      }

      return sum.M();
    }


    double massDifference(const Particle* particle, const std::vector<double>& daughters)
    {
      if (!particle)
        return -999;

      long daughter = std::lround(daughters[0]);
      if (daughter >= static_cast<int>(particle->getNDaughters()))
        return -999;

      double motherMass = particle->getMass();
      double daughterMass = particle->getDaughter(daughter)->getMass();

      return motherMass - daughterMass;
    }

    double massDifferenceError(const Particle* particle, const std::vector<double>& daughters)
    {
      if (!particle)
        return -999;

      long daughter = std::lround(daughters[0]);
      if (daughter >= static_cast<int>(particle->getNDaughters()))
        return -999;

      float result = 0.0;

      TLorentzVector thisDaughterMomentum = particle->getDaughter(daughter)->get4Vector();

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
        return -999;

      long daughter = std::lround(daughters[0]);
      if (daughter >= static_cast<int>(particle->getNDaughters()))
        return -999;

      double massDiff = massDifference(particle, daughters);
      double massDiffErr = massDifferenceError(particle, daughters);

      double massDiffNominal = particle->getPDGMass() - particle->getDaughter(daughter)->getPDGMass();

      return (massDiff - massDiffNominal) / massDiffErr;
    }

    // Decay Kinematics -------------------------------------------------------
    double particleDecayAngle(const Particle* particle, const std::vector<double>& daughters)
    {
      if (!particle)
        return -999;

      double result = 0.0;

      TLorentzVector motherMomentum = particle->get4Vector();
      TVector3       motherBoost    = -(motherMomentum.BoostVector());

      long daughter = std::lround(daughters[0]);
      if (daughter >= static_cast<int>(particle->getNDaughters()))
        return -999;

      TLorentzVector daugMomentum = particle->getDaughter(daughter)->get4Vector();
      daugMomentum.Boost(motherBoost);

      result = cos(daugMomentum.Angle(motherMomentum.Vect()));

      return result;
    }

    double particleDaughterAngle(const Particle* particle, const std::vector<double>& daughters)
    {
      if (!particle)
        return -999;

      int nDaughters = static_cast<int>(particle->getNDaughters());

      long daughter1 = std::lround(daughters[0]);
      long daughter2 = std::lround(daughters[1]);
      if (daughter1 >= nDaughters || daughter2 >= nDaughters)
        return -999;

      const auto& frame = ReferenceFrame::GetCurrent();
      TVector3 a = frame.getMomentum(particle->getDaughter(daughter1)).Vect();
      TVector3 b = frame.getMomentum(particle->getDaughter(daughter2)).Vect();
      return cos(a.Angle(b));
    }

    double pointingAngle(const Particle* particle, const std::vector<double> daughters)
    {
      if (!particle)
        return -999;

      long daughter = std::lround(daughters[0]);
      if (daughter >= static_cast<int>(particle->getNDaughters()))
        return -999;

      TVector3 productionVertex = particle->getVertex();
      TVector3 decayVertex = particle->getDaughter(daughter)->getVertex();

      TVector3 vertexDiffVector = productionVertex - decayVertex;

      const auto& frame = ReferenceFrame::GetCurrent();
      TVector3 daughterMomentumVector = frame.getMomentum(particle->getDaughter(daughter)).Vect();

      return cos(daughterMomentumVector.Angle(vertexDiffVector));
    }

    double azimuthalAngleInDecayPlane(const Particle* particle, const std::vector<double> daughters)
    {
      if (!particle)
        return -999;

      int nDaughters = static_cast<int>(particle->getNDaughters());

      long daughter1 = std::lround(daughters[0]);
      long daughter2 = std::lround(daughters[1]);
      if (daughter1 >= nDaughters || daughter2 >= nDaughters)
        return -999;

      const auto& frame = ReferenceFrame::GetCurrent();
      PCmsLabTransform T;
      TLorentzVector m = T.getBeamParams().getHER() + T.getBeamParams().getLER();
      TLorentzVector p = particle->get4Vector();
      TLorentzVector d1 = particle->getDaughter(daughter1)->get4Vector();
      TLorentzVector d2 = particle->getDaughter(daughter2)->get4Vector();

      TLorentzVector l;
      l.SetX(p.Py() * (d1.Pz() * d2.E()  - d1.E()  * d2.Pz()) + p.Pz() * (d1.E()  * d2.Py() - d1.Py() * d2.E())
             + p.E()  * (d1.Py() * d2.Pz() - d1.Pz() * d2.Py()));
      l.SetY(p.Px() * (d1.E()  * d2.Pz() - d1.Pz() * d2.E())  + p.Pz() * (d1.Px() * d2.E()  - d1.E()  * d2.Px())
             + p.E()  * (d1.Pz() * d2.Px() - d1.Px() * d2.Pz()));
      l.SetZ(p.Px() * (d1.Py() * d2.E()  - d1.E()  * d2.Py()) + p.Py() * (d1.E()  * d2.Px() - d1.Px() * d2.E())
             + p.E()  * (d1.Px() * d2.Py() - d1.Py() * d2.Px()));
      l.SetE(-(p.Px() * (d1.Pz() * d2.Py() - d1.Py() * d2.Pz()) + p.Py() * (d1.Px() * d2.Pz() - d1.Pz() * d2.Px())
               + p.Pz() * (d1.Py() * d2.Px() - d1.Px() * d2.Py())));

      double m_times_p = m * p;
      double m_times_l = m * l;
      double m_times_d1 = m * d1;
      double l_times_d1 = l * d1;
      double d1_times_p = d1 * p;
      double m_abs = TMath::Sqrt(pow(m_times_p / p.M(), 2) - m.M2());
      double d1_abs = TMath::Sqrt(pow(d1_times_p / p.M(), 2) - d1.M2());
      double cos_phi = -m_times_l / (m_abs * TMath::Sqrt(-l.M2()));
      double m_parallel_abs = m_abs * TMath::Sqrt(1 - cos_phi * cos_phi);
      double m_parallel_times_d1 = m_times_p * d1_times_p / p.M2() + m_times_l * l_times_d1 / l.M2() - m_times_d1;

      return TMath::ACos(-m_parallel_times_d1 / (m_parallel_abs * d1_abs));
    }

    double v0DaughterD0(const Particle* particle, const std::vector<double>& daughterID)
    {
      if (!particle)
        return -999;

      TVector3 v0Vertex = particle->getVertex();

      const Particle* daug = particle->getDaughter(daughterID[0]);

      const Track* track = daug->getTrack();
      if (!track) return 999.9;

      const TrackFitResult* trackFit = track->getTrackFitResultWithClosestMass(Const::ChargedStable(abs(daug->getPDGCode())));
      if (!trackFit) return 999.9;

      UncertainHelix helix = trackFit->getUncertainHelix();
      helix.passiveMoveBy(v0Vertex);

      return helix.getD0();
    }

    double v0DaughterD0Diff(const Particle* particle)
    {
      return v0DaughterD0(particle, {0}) - v0DaughterD0(particle, {1});
    }

    double v0DaughterZ0(const Particle* particle, const std::vector<double>& daughterID)
    {
      if (!particle)
        return -999;

      TVector3 v0Vertex = particle->getVertex();

      const Particle* daug = particle->getDaughter(daughterID[0]);

      const Track* track = daug->getTrack();
      if (!track) return 999.9;

      const TrackFitResult* trackFit = track->getTrackFitResultWithClosestMass(Const::ChargedStable(abs(daug->getPDGCode())));
      if (!trackFit) return 999.9;

      UncertainHelix helix = trackFit->getUncertainHelix();
      helix.passiveMoveBy(v0Vertex);

      return helix.getZ0();
    }

    double v0DaughterZ0Diff(const Particle* particle)
    {
      return v0DaughterZ0(particle, {0}) - v0DaughterZ0(particle, {1});
    }

    double Constant(const Particle*, const std::vector<double>& constant)
    {
      return constant[0];
    }



    VARIABLE_GROUP("ParameterFunctions");
    REGISTER_VARIABLE("NumberOfMCParticlesInEvent(pdgcode)", NumberOfMCParticlesInEvent ,
                      "Returns number of MC Particles (including anti-particles) with the given pdgcode in the event.\n"
                      "Used in the FEI to determine to calculate reconstruction efficiencies.\n"
                      "The variable is event-based and does not need a valid particle pointer as input.");
    REGISTER_VARIABLE("isAncestorOf(i, j, ...)", isAncestorOf,
                      "Returns a positive integer if daughter at position particle->daughter(i)->daughter(j)... is an ancestor of the related MC particle, 0 otherwise.\n"
                      "Positive integer represents the number of steps needed to get from final MC daughter to ancestor."
                      "If any particle or MCparticle is a nullptr, -999 is returned. If MC relations of any particle doesn't exist, -1.0 is returned.");
    REGISTER_VARIABLE("hasAncestor(PDG, abs)", hasAncestor,
                      "Returns a positive integer if an ancestor with the given PDG code is found, 0 otherwise.\n"
                      "The integer is the level where the ancestor was found, 1: first mother, 2: grandmother, etc.\n"
                      "Second argument is optional, 1 means that the sign of the PDG code is taken into account, default is 0.\n"
                      "If there is no MC relations found, -1 is returned. In case of nullptr particle, -999 is returned.");
    REGISTER_VARIABLE("genMotherPDG(i)", genNthMotherPDG,
                      "Check the PDG code of a particles n-th MC mother particle by providing an argument. 0 is first mother, 1 is grandmother etc.");
    REGISTER_VARIABLE("genMotherID(i)", genNthMotherIndex,
                      "Check the array index of a particle n-th MC mother particle by providing an argument. 0 is first mother, 1 is grandmother etc.");
    REGISTER_VARIABLE("daughterInvariantMass(i, j, ...)", daughterInvariantMass ,
                      "Returns invariant mass of the given daughter particles.\n"
                      "E.g. daughterInvariantMass(0, 1) returns the invariant mass of the first and second daughter.\n"
                      "     daughterInvariantMass(0, 1, 2) returns the invariant mass of the first, second and third daughter.\n"
                      "Useful to identify intermediate resonances in a decay, which weren't reconstructed explicitly.\n"
                      "Returns -999 if particle is nullptr or if the given daughter-index is out of bound (>= amount of daughters).");
    REGISTER_VARIABLE("daughterMCInvariantMass(i, j, ...)", daughterMCInvariantMass ,
                      "Returns true invariant mass of the given daughter particles, same behaviour as daughterInvariantMass variable.\n");
    REGISTER_VARIABLE("decayAngle(i)", particleDecayAngle,
                      "cosine of the angle between the mother momentum vector and the direction of the i-th daughter in the mother's rest frame");
    REGISTER_VARIABLE("daughterAngle(i,j)", particleDaughterAngle, "cosine of the angle between i-th and j-th daughters");
    REGISTER_VARIABLE("pointingAngle(i)", pointingAngle,
                      "cosine of the angle between i-th daughter momentum vector and vector connecting production and decay vertex of i-th daughter");
    REGISTER_VARIABLE("azimuthalAngleInDecayPlane(i, j)", azimuthalAngleInDecayPlane,
                      "azimuthal angle of i-th daughter in decay plane towards projection of particle momentum into decay plane");

    REGISTER_VARIABLE("massDifference(i)", massDifference, "Difference in invariant masses of this particle and its i-th daughter");
    REGISTER_VARIABLE("massDifferenceError(i)", massDifferenceError,
                      "Estimated uncertainty on difference in invariant masses of this particle and its i-th daughter");
    REGISTER_VARIABLE("massDifferenceSignificance(i)", massDifferenceSignificance,
                      "Signed significance of the deviation from the nominal mass difference of this particle and its i-th daughter [(massDiff - NOMINAL_MASS_DIFF)/ErrMassDiff]");

    REGISTER_VARIABLE("V0d0(id)", v0DaughterD0,
                      "Return the d0 impact parameter of a V0's daughter with daughterID index with the V0 vertex point as a pivot for the track.");
    REGISTER_VARIABLE("V0Deltad0", v0DaughterD0Diff,
                      "Return the difference between d0 impact parameters of V0's daughters with the V0 vertex point as a pivot for the track.");
    REGISTER_VARIABLE("V0z0(id)", v0DaughterZ0,
                      "Return the z0 impact parameter of a V0's daughter with daughterID index with the V0 vertex point as a pivot for the track.");
    REGISTER_VARIABLE("V0Deltaz0", v0DaughterZ0Diff,
                      "Return the difference between z0 impact parameters of V0's daughters with the V0 vertex point as a pivot for the track.");

    REGISTER_VARIABLE("constant(float i)", Constant,
                      "Returns i.\n"
                      "Useful for debugging purposes and in conjunction with the formula meta-variable.");

    REGISTER_VARIABLE("randomChoice(i, j, ...)", RandomChoice,
                      "Returns random element of given numbers.\n"
                      "Useful for testing purposes");


  }
}
