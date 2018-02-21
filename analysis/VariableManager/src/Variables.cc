/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc, Thomas Keck                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/ParameterVariables.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ReferenceFrame.h>

#include <analysis/VariableManager/Manager.h>
#include <analysis/utility/MCMatching.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ContinuumSuppression.h>
#include <analysis/dataobjects/Vertex.h>
#include <analysis/dataobjects/ThrustOfEvent.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/core/InputController.h>

// utility
#include <analysis/utility/C2TDistanceUtility.h>

#include <TLorentzVector.h>
#include <TRandom.h>
#include <TVectorF.h>
#include <TVector3.h>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

// momentum -------------------------------------------

    double particleP(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(part).P();
    }

    double particleE(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(part).E();
    }

    double particlePx(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(part).Px();
    }

    double particlePy(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(part).Py();
    }

    double particlePz(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(part).Pz();
    }

    double particlePt(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(part).Pt();
    }

    double covMatrixElement(const Particle* part, const std::vector<double>& element)
    {
      int elementI = int(std::lround(element[0]));
      int elementJ = int(std::lround(element[1]));

      if (elementI < 0 || elementI > 6) {
        B2WARNING("Requested particle's momentumVertex covariance matrix element is out of boundaries [0 - 6]: i = " << elementI);
        return 0;
      }
      if (elementJ < 0 || elementJ > 6) {
        B2WARNING("Requested particle's momentumVertex covariance matrix element is out of boundaries [0 - 6]: j = " << elementJ);
        return 0;
      }

      return part->getMomentumVertexErrorMatrix()(elementI, elementJ);
    }

    double particlePErr(const Particle* part)
    {
      TMatrixD jacobianRot(3, 3);
      jacobianRot.Zero();

      double cosPhi = cos(particlePhi(part));
      double sinPhi = sin(particlePhi(part));
      double cosTheta = particleCosTheta(part);
      double sinTheta = sin(acos(cosTheta));
      double p = particleP(part);

      jacobianRot(0, 0) = sinTheta * cosPhi;
      jacobianRot(0, 1) = sinTheta * sinPhi;
      jacobianRot(1, 0) = cosTheta * cosPhi / p;
      jacobianRot(1, 1) = cosTheta * sinPhi / p;
      jacobianRot(0, 2) = cosTheta;
      jacobianRot(2, 0) = -sinPhi / sinTheta / p;
      jacobianRot(1, 2) = -sinTheta / p;
      jacobianRot(2, 1) = cosPhi / sinTheta / p;

      const auto& frame = ReferenceFrame::GetCurrent();

      double errorSquared = frame.getMomentumErrorMatrix(part).GetSub(0, 2, 0, 2, " ").Similarity(jacobianRot)(0, 0);

      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return 0.0;
    }

    double particlePxErr(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();

      double errorSquared = frame.getMomentumErrorMatrix(part)(0, 0);

      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return 0.0;
    }

    double particlePyErr(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      double errorSquared = frame.getMomentumErrorMatrix(part)(1, 1);

      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return 0.0;
    }

    double particlePzErr(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      double errorSquared = frame.getMomentumErrorMatrix(part)(2, 2);

      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return 0.0;
    }

    double particlePtErr(const Particle* part)
    {
      TMatrixD jacobianRot(3, 3);
      jacobianRot.Zero();

      double px = particlePx(part);
      double py = particlePy(part);
      double pt = particlePt(part);

      jacobianRot(0, 0) = px / pt;
      jacobianRot(0, 1) = py / pt;
      jacobianRot(1, 0) = -py / (pt * pt);
      jacobianRot(1, 1) = px / (pt * pt);
      jacobianRot(2, 2) = 1;

      const auto& frame = ReferenceFrame::GetCurrent();
      double errorSquared = frame.getMomentumErrorMatrix(part).GetSub(0, 2, 0, 2, " ").Similarity(jacobianRot)(0, 0);

      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return 0.0;

    }

    double momentumDeviationChi2(const Particle* part)
    {
      double result = 1e6;

      // check if error matrix is set
      if (part->getPValue() < 0.0)
        return result;

      // check if mc match exists
      const MCParticle* mcp = part->getRelated<MCParticle>();
      if (mcp == nullptr)
        return result;

      result = 0.0;
      result += TMath::Power(part->getPx() - mcp->getMomentum().Px(), 2.0) / part->getMomentumVertexErrorMatrix()(0, 0);
      result += TMath::Power(part->getPy() - mcp->getMomentum().Py(), 2.0) / part->getMomentumVertexErrorMatrix()(1, 1);
      result += TMath::Power(part->getPz() - mcp->getMomentum().Pz(), 2.0) / part->getMomentumVertexErrorMatrix()(2, 2);

      return result;
    }

    double particleTheta(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return acos(frame.getMomentum(part).CosTheta());
    }

    double particleThetaErr(const Particle* part)
    {
      TMatrixD jacobianRot(3, 3);
      jacobianRot.Zero();

      double cosPhi = cos(particlePhi(part));
      double sinPhi = sin(particlePhi(part));
      double cosTheta = particleCosTheta(part);
      double sinTheta = sin(acos(cosTheta));
      double p = particleP(part);

      jacobianRot(0, 0) = sinTheta * cosPhi;
      jacobianRot(0, 1) = sinTheta * sinPhi;
      jacobianRot(1, 0) = cosTheta * cosPhi / p;
      jacobianRot(1, 1) = cosTheta * sinPhi / p;
      jacobianRot(0, 2) = cosTheta;
      jacobianRot(2, 0) = -sinPhi / sinTheta / p;
      jacobianRot(1, 2) = -sinTheta / p;
      jacobianRot(2, 1) = cosPhi / sinTheta / p;

      const auto& frame = ReferenceFrame::GetCurrent();
      double errorSquared = frame.getMomentumErrorMatrix(part).GetSub(0, 2, 0, 2, " ").Similarity(jacobianRot)(1, 1);

      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return 0.0;
    }

    double particleCosTheta(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(part).CosTheta();
    }

    double particleCosThetaErr(const Particle* part)
    {
      return fabs(particleThetaErr(part) * sin(particleTheta(part)));
    }

    double particlePhi(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(part).Phi();
    }

    double particlePhiErr(const Particle* part)
    {
      TMatrixD jacobianRot(3, 3);
      jacobianRot.Zero();

      double px = particlePx(part);
      double py = particlePy(part);
      double pt = particlePt(part);

      jacobianRot(0, 0) = px / pt;
      jacobianRot(0, 1) = py / pt;
      jacobianRot(1, 0) = -py / (pt * pt);
      jacobianRot(1, 1) = px / (pt * pt);
      jacobianRot(2, 2) = 1;

      const auto& frame = ReferenceFrame::GetCurrent();
      double errorSquared = frame.getMomentumErrorMatrix(part).GetSub(0, 2, 0, 2, " ").Similarity(jacobianRot)(1, 1);

      if (errorSquared > 0.0)
        return sqrt(errorSquared);
      else
        return 0.0;
    }

    double particlePDGCode(const Particle* part)
    {
      return part->getPDGCode();
    }

    double cosAngleBetweenMomentumAndVertexVector(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return std::cos(frame.getVertex(part).Angle(frame.getMomentum(part).Vect()));
    }

    double cosThetaBetweenParticleAndTrueB(const Particle* part)
    {
      PCmsLabTransform T;
      double e_Beam = T.getCMSEnergy() / 2;
      double m_B = part->getPDGMass();
      double p_B = std::sqrt(e_Beam * e_Beam - m_B * m_B);

      TLorentzVector p = T.rotateLabToCms() * part->get4Vector();
      double e_d = p.E();
      double m_d = p.M();
      double p_d = p.Rho();

      double theta_Bd = (2 * e_Beam * e_d - m_B * m_B - m_d * m_d)
                        / (2 * p_B * p_d);
      return theta_Bd;
    }

    double cosHelicityAngle(const Particle* part)
    {

      const auto& frame = ReferenceFrame::GetCurrent();
      TVector3 motherBoost = - frame.getMomentum(part).BoostVector();
      TVector3 motherMomentum = frame.getMomentum(part).Vect();
      const auto& daughters = part -> getDaughters() ;

      if (daughters.size() == 2) {

        bool isOneConversion = false;

        for (auto& idaughter : daughters) {
          if (idaughter -> getNDaughters() == 2) {
            if (std::abs(idaughter -> getDaughters()[0]-> getPDGCode()) == 11) isOneConversion = true;
          }
        }

        if (isOneConversion) {
          //only for pi0 decay where one gamma converts

          TLorentzVector pGamma;

          for (auto& idaughter : daughters) {
            if (idaughter -> getNDaughters() == 2) continue;
            else pGamma = frame.getMomentum(idaughter);
          }

          pGamma.Boost(motherBoost);

          return std::cos(motherMomentum.Angle(pGamma.Vect()));

        } else {
          TLorentzVector pDaughter1 = frame.getMomentum(daughters[0]);
          TLorentzVector pDaughter2 = frame.getMomentum(daughters[1]);

          pDaughter1.Boost(motherBoost);
          pDaughter2.Boost(motherBoost);

          TVector3 p12 = (pDaughter2 - pDaughter1).Vect();

          return std::cos(motherMomentum.Angle(p12));
        }

      } else if (daughters.size() == 3) {

        TLorentzVector pDaughter1 = frame.getMomentum(daughters[0]);
        TLorentzVector pDaughter2 = frame.getMomentum(daughters[1]);
        TLorentzVector pDaughter3 = frame.getMomentum(daughters[2]);

        pDaughter1.Boost(motherBoost);
        pDaughter2.Boost(motherBoost);
        pDaughter3.Boost(motherBoost);

        TVector3 p12 = (pDaughter2 - pDaughter1).Vect();
        TVector3 p13 = (pDaughter3 - pDaughter1).Vect();

        TVector3 n = p12.Cross(p13);

        return std::cos(motherMomentum.Angle(n));

      }  else return 0;

    }

    double cosHelicityAnglePi0Dalitz(const Particle* part)
    {

      const auto& frame = ReferenceFrame::GetCurrent();
      TVector3 motherBoost = - frame.getMomentum(part).BoostVector();
      TVector3 motherMomentum = frame.getMomentum(part).Vect();
      const auto& daughters = part -> getDaughters() ;


      if (daughters.size() == 3) {

        TLorentzVector pGamma;

        for (auto& idaughter : daughters) {
          if (std::abs(idaughter -> getPDGCode()) == 22) pGamma = frame.getMomentum(idaughter);
        }

        pGamma.Boost(motherBoost);

        return std::cos(motherMomentum.Angle(pGamma.Vect()));

      }  else return 0;

    }

    double ImpactXY(const Particle* particle)
    {
      double px = particle->getPx();
      double py = particle->getPy();

      if (py == py && px == px) {

        double x = particle->getX() - 0;
        double y = particle->getY() - 0;

        double pt = sqrt(px * px + py * py);

//       const TVector3 m_BeamSpotCenter = TVector3(0., 0., 0.);
//       TVector3 Bfield= BFieldMap::Instance().getBField(m_BeamSpotCenter); # TODO check why this produces a linking bug

        double a = -0.2998 * 1.5 * particle->getCharge(); //Curvature of the track,
        double T = TMath::Sqrt(pt * pt - 2 * a * (x * py - y * px) + a * a * (x * x + y * y));

        return TMath::Abs((-2 * (x * py - y * px) + a * (x * x + y * y)) / (T + pt));
      } else return 0;
    }

// vertex or POCA in respect to IP ------------------------------

    double particleDX(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).X();
    }

    double particleDY(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).Y();
    }

    double particleDZ(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).Z();
    }

    inline double getParticleUncertaintyByIndex(const Particle* part, unsigned int index)
    {
      if (!part) {
        B2FATAL("The particle provide does not exist.");
      }
      const auto& errMatrix = part->getVertexErrorMatrix();
      return std::sqrt(errMatrix(index, index));
    }

    double particleDXUncertainty(const Particle* part)
    {
      return getParticleUncertaintyByIndex(part, 0);
    }

    double particleDYUncertainty(const Particle* part)
    {
      return getParticleUncertaintyByIndex(part, 1);
    }

    double particleDZUncertainty(const Particle* part)
    {
      return getParticleUncertaintyByIndex(part, 2);
    }

    double particleDRho(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).Perp();
    }

    double particleDPhi(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).Phi();
    }

    double particleDCosTheta(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).CosTheta();
    }

    double particleDistance(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).Mag();
    }

    double particleDistanceSignificance(const Particle* part)
    {
      // significance is defined as s = r/sigma_r, therefore:
      // s &= \frac{r}{\sqrt{ \sum_{ij} \frac{\partial r}{x_i} V_{ij} \frac{\partial r}{x_j}}}
      //   &= \frac{r^2}{\sqrt{\vec{x}V\vec{x}}}
      // where:
      // r &= \sqrt{\vec{x}*\vec{x}}
      // and V_{ij} is the covariance matrix
      const auto& frame = ReferenceFrame::GetCurrent();
      const auto& vertex = frame.getVertex(part);
      auto denominator = vertex * (part->getVertexErrorMatrix() * vertex);
      if (denominator <= 0)
        return -1;
      return vertex.Mag2() / sqrt(denominator);
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

    double particleInvariantMass(const Particle* part)
    {
      double result = 0.0;

      const std::vector<Particle*> daughters = part->getDaughters();
      if (daughters.size() > 0) {
        TLorentzVector sum;
        for (unsigned i = 0; i < daughters.size(); i++)
          sum += daughters[i]->get4Vector();

        result = sum.M();
      } else {
        result = part->getMass();
      }

      return result;
    }

    double particleInvariantMassLambda(const Particle* part)
    {
      double result = 0.0;

      const std::vector<Particle*> daughters = part->getDaughters();
      if (daughters.size() == 2) {
        TLorentzVector dt1;
        TLorentzVector dt2;
        TLorentzVector dtsum;
        double mpi = 0.1396;
        double mpr = 0.9383;
        dt1 = daughters[0]->get4Vector();
        dt2 = daughters[1]->get4Vector();
        double E1 = hypot(mpi, dt1.P());
        double E2 = hypot(mpr, dt2.P());
        dtsum = dt1 + dt2;
        return sqrt((E1 + E2) * (E1 + E2) - dtsum.P() * dtsum.P());

      } else {
        result = part->getMass();
      }

      return result;
    }

    double particleInvariantMassError(const Particle* part)
    {
      float result = 0.0;

      float invMass = part->getMass();

      TMatrixFSym covarianceMatrix(Particle::c_DimMomentum);
      for (unsigned i = 0; i < part->getNDaughters(); i++) {
        covarianceMatrix += part->getDaughter(i)->getMomentumErrorMatrix();
      }

      TVectorF jacobian(Particle::c_DimMomentum);
      jacobian[0] = -1.0 * part->getPx() / invMass;
      jacobian[1] = -1.0 * part->getPy() / invMass;
      jacobian[2] = -1.0 * part->getPz() / invMass;
      jacobian[3] = 1.0 * part->getEnergy() / invMass;

      result = jacobian * (covarianceMatrix * jacobian);

      if (result < 0.0)
        result = 0.0;

      return TMath::Sqrt(result);
    }

    double particleInvariantMassSignificance(const Particle* part)
    {
      float invMass = part->getMass();
      float nomMass = part->getPDGMass();
      float massErr = particleInvariantMassError(part);

      return (invMass - nomMass) / massErr;
    }

    double particleInvariantMassBeforeFitSignificance(const Particle* part)
    {
      float invMass = particleInvariantMass(part);
      float nomMass = part->getPDGMass();
      float massErr = particleInvariantMassError(part);

      return (invMass - nomMass) / massErr;
    }

    double missingMass(const Particle* part)
    {
      PCmsLabTransform T;
      double beamEnergy = T.getCMSEnergy() / 2.;
      TLorentzVector tagVec = T.rotateLabToCms()
                              * part->getDaughter(0)->get4Vector();
      TLorentzVector sigVec = T.rotateLabToCms()
                              * part->getDaughter(1)->get4Vector();
      tagVec.SetE(-beamEnergy);
      return (-tagVec - sigVec).M2();
    }

    double missingMomentum(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector beam = T.getBeamParams().getHER() + T.getBeamParams().getLER();

      return (beam - part->get4Vector()).Vect().Mag();
    }

    double missingMomentumTheta(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector beam = T.getBeamParams().getHER() + T.getBeamParams().getLER();

      return (beam - part->get4Vector()).Vect().Theta();
    }

    double missingMomentumPhi(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector beam = T.getBeamParams().getHER() + T.getBeamParams().getLER();

      return (beam - part->get4Vector()).Vect().Phi();
    }

    double cosToThrustOfEvent(const Particle* part)
    {
      StoreObjPtr<ThrustOfEvent> thrust;
      if (!thrust) {
        B2WARNING("Cannot find thrust of event information, did you forget to run ThrustOfEventModule?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      PCmsLabTransform T;
      TVector3 th = thrust->getThrustAxis();
      TVector3 particleMomentum = (T.rotateLabToCms() * part -> get4Vector()).Vect();
      return std::cos(th.Angle(particleMomentum));
    }

// released energy --------------------------------------------------

    double particleQ(const Particle* part)
    {
      float m = part->getMass();
      for (unsigned i = 0; i < part->getNDaughters(); i++) {
        const Particle* child = part->getDaughter(i);
        if (child)
          m -= child->getMass();
      }
      return m;
    }

    double particleDQ(const Particle* part)
    {
      float m = part->getMass() - part->getPDGMass();
      for (unsigned i = 0; i < part->getNDaughters(); i++) {
        const Particle* child = part->getDaughter(i);
        if (child)
          m -= (child->getMass() - child->getPDGMass());
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

// other ------------------------------------------------------------

    double particleMdstArrayIndex(const Particle* part)
    {
      return part->getMdstArrayIndex();
    }

    double particleMdstSource(const Particle* part)
    {
      return part->getMdstSource();
    }

    double particleCosMdstArrayIndex(const Particle* part)
    {
      return std::cos(part->getMdstArrayIndex());
    }

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

    double particleCharge(const Particle* part)
    {
      return part->getCharge();
    }

    void printParticleInternal(const Particle* p, int depth)
    {
      stringstream s("");
      for (int i = 0; i < depth; i++) {
        s << "    ";
      }
      s << p->getPDGCode();
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (mcp) {
        unsigned int flags = MCMatching::getMCErrors(p, mcp);
        s << " -> MC: " << mcp->getPDG() << ", mcErrors: " << flags << " ("
          << MCMatching::explainFlags(flags) << ")";
        s << ", mc-index " << mcp->getIndex();
      } else {
        s << " (no MC match)";
      }
      s << ", mdst-source " << p->getMdstSource();
      B2INFO(s.str());
      for (const auto* daughter : p->getDaughters()) {
        printParticleInternal(daughter, depth + 1);
      }
    }

    double printParticle(const Particle* p)
    {
      printParticleInternal(p, 0);
      return 0.0;
    }

    double mcParticleSecondaryPhysicsProcess(const Particle* p)
    {
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (mcp) {
        return mcp->getSecondaryPhysicsProcess();
      } else {
        return -1;
      }
    }

    double mcParticleStatus(const Particle* p)
    {
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (mcp) {
        return mcp->getStatus();
      } else {
        return -1;
      }
    }

    double particleMCPrimaryParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_PrimaryParticle;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return -1;
      }
    }

    double particleMCMomentumTransfer2(const Particle* part)
    {
      // for B meson MC particles only
      const MCParticle* mcB = part->getRelated<MCParticle>();

      if (!mcB)
        return -999.9;

      TLorentzVector pB = mcB->get4Vector();

      std::vector<MCParticle*> mcDaug = mcB->getDaughters();

      if (mcDaug.empty())
        return -999.9;

      // B -> X l nu
      // q = pB - pX
      TLorentzVector pX;

      for (unsigned i = 0; i < mcDaug.size(); i++) {
        const MCParticle* mcTemp = mcDaug[i];

        if (abs(mcTemp->getPDG()) <= 16)
          continue;

        pX += mcTemp->get4Vector();
      }

      TLorentzVector q = pB - pX;

      return q.Mag2();
    }

    // MC related ------------------------------------------------------------
    double particleMCVirtualParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_IsVirtual;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return -1;
      }
    }

    double particleMCInitialParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_Initial;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return -1;
      }
    }

    double particleMCISRParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_IsISRPhoton;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return -1;
      }
    }

    double particleMCFSRParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_IsFSRPhoton;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return -1;
      }
    }

    double particleMCPhotosParticle(const Particle* p)
    {
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (mcp) {
        unsigned int bitmask = MCParticle::c_IsPHOTOSPhoton;
        if (mcp->hasStatus(bitmask))
          return 1;
        else
          return 0;
      } else {
        return -1;
      }
    }

    double isSignal(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits, these are usually ok
      status &= (~MCMatching::c_MissFSR);
      status &= (~MCMatching::c_MissPHOTOS);
      status &= (~MCMatching::c_MissingResonance);
      //status &= (~MCMatching::c_DecayInFlight);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }


    double isExtendedSignal(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits, these are usually ok
      status &= (~MCMatching::c_MissFSR);
      status &= (~MCMatching::c_MissPHOTOS);
      status &= (~MCMatching::c_MissingResonance);
      status &= (~MCMatching::c_MisID);
      status &= (~MCMatching::c_AddedWrongParticle);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }

    double genMotherPDG(const Particle* part)
    {
      const std::vector<double> args = {};
      return genNthMotherPDG(part, args);
    }

    double genMotherP(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      const MCParticle* mcmother = mcparticle->getMother();
      if (mcmother == nullptr)
        return 0.0;

      double p = mcmother->getMomentum().Mag();
      return p;
    }

    double genMotherIndex(const Particle* part)
    {
      const std::vector<double> args = {};
      return genNthMotherIndex(part, args);
    }

    double genParticleIndex(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (!mcparticle)
        return -1.0;

      double m_ID = mcparticle->getArrayIndex();
      return m_ID;
    }

    double isSignalAcceptMissingNeutrino(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits, these are usually ok
      status &= (~MCMatching::c_MissFSR);
      status &= (~MCMatching::c_MissPHOTOS);
      status &= (~MCMatching::c_MissingResonance);
      //status &= (~MCMatching::c_DecayInFlight);
      status &= (~MCMatching::c_MissNeutrino);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }

    double particleMCMatchPDGCode(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      return mcparticle->getPDG();
    }

    double particleMCErrors(const Particle* part)
    {
      return MCMatching::getMCErrors(part);
    }

    double particleNumberOfMCMatch(const Particle* particle)
    {
      RelationVector<MCParticle> mcRelations =
        particle->getRelationsTo<MCParticle>();
      return double(mcRelations.size());
    }

    double particleMCMatchWeight(const Particle* particle)
    {
      auto relWithWeight = particle->getRelatedToWithWeight<MCParticle>();

      if (relWithWeight.first) {
        return relWithWeight.second;
      } else {
        return 0.0;
      }
    }

    double particleMCMatchDecayTime(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getDecayTime();
    }

    double particleMCMatchPX(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getMomentum().Px();
    }

    double particleMCMatchPY(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getMomentum().Py();
    }

    double particleMCMatchPZ(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getMomentum().Pz();
    }

    double particleMCMatchDX(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getDecayVertex().Px();
    }

    double particleMCMatchDY(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getDecayVertex().Py();
    }

    double particleMCMatchDZ(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getDecayVertex().Pz();
    }

    double particleMCMatchE(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getEnergy();
    }

    double particleMCMatchP(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return -999.0;

      return mcparticle->getMomentum().Mag();
    }

    double particleMCRecoilMass(const Particle* part)
    {
      StoreArray<MCParticle> mcparticles;
      if (mcparticles.getEntries() < 1)
        return -999;

      TLorentzVector pInitial = mcparticles[0]->get4Vector();
      TLorentzVector pDaughters;
      const std::vector<Particle*> daughters = part->getDaughters();
      for (unsigned i = 0; i < daughters.size(); i++) {
        const MCParticle* mcD = daughters[i]->getRelatedTo<MCParticle>();
        if (mcD == nullptr)
          return -999;

        pDaughters += mcD->get4Vector();
      }

      return (pInitial - pDaughters).M();
    }


// Recoil Kinematics related ---------------------------------------------

    double recoilMomentum(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN = T.getBoostVector();

      return (pIN - particle->get4Vector()).P();
    }

    double recoilEnergy(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN = T.getBoostVector();

      return (pIN - particle->get4Vector()).E();
    }

    double recoilMass(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN = T.getBoostVector();

      return (pIN - particle->get4Vector()).M();
    }

    double recoilMassSquared(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN = T.getBoostVector();

      return (pIN - particle->get4Vector()).M2();
    }

    double recoilMCDecayType(const Particle* particle)
    {
      MCParticle* mcp = particle->getRelatedTo<MCParticle>();

      if (!mcp)
        return -1.0;

      MCParticle* mcMother = mcp->getMother();

      if (!mcMother)
        return -1.0;

      std::vector<MCParticle*> daughters = mcMother->getDaughters();

      if (daughters.size() != 2)
        return -1.0;

      MCParticle* recoilMC = nullptr;
      if (daughters[0]->getArrayIndex() == mcp->getArrayIndex())
        recoilMC = daughters[1];
      else
        recoilMC = daughters[0];

      if (!recoilMC->hasStatus(MCParticle::c_PrimaryParticle))
        return -1.0;

      int decayType = 0;
      checkMCParticleDecay(recoilMC, decayType, false);

      if (decayType == 0)
        checkMCParticleDecay(recoilMC, decayType, true);

      return decayType;
    }

    void checkMCParticleDecay(MCParticle* mcp, int& decayType, bool recursive)
    {
      int nHadronicParticles = 0;
      int nPrimaryParticleDaughters = 0;
      std::vector<MCParticle*> daughters = mcp->getDaughters();

      // Are any of the daughters primary particles? How many of them are hadrons?
      for (unsigned i = 0; i < daughters.size(); i++) {
        if (!daughters[i]->hasStatus(MCParticle::c_PrimaryParticle))
          continue;

        nPrimaryParticleDaughters++;
        if (abs(daughters[i]->getPDG()) > 22)
          nHadronicParticles++;
      }

      if (nPrimaryParticleDaughters > 1) {
        for (unsigned i = 0; i < daughters.size(); i++) {
          if (!daughters[i]->hasStatus(MCParticle::c_PrimaryParticle))
            continue;

          if (abs(daughters[i]->getPDG()) == 12 or abs(daughters[i]->getPDG()) == 14 or abs(daughters[i]->getPDG()) == 16) {
            if (!recursive) {
              if (nHadronicParticles == 0) {
                decayType = 1.0;
                break;
              } else {
                decayType = 2.0;
                break;
              }
            } else {
              decayType = 3.0;
              break;
            }
          }

          else if (recursive)
            checkMCParticleDecay(daughters[i], decayType, recursive);
        }
      }
    }



    double nRemainingTracksInEvent(const Particle* particle)
    {

      StoreArray<Track> tracks;
      int event_tracks = tracks.getEntries();

      int par_tracks = 0;
      const auto& daughters = particle->getFinalStateDaughters();
      for (const auto& daughter : daughters) {
        int pdg = abs(daughter->getPDGCode());
        if (pdg == 11 or pdg == 13 or pdg == 211 or pdg == 321 or pdg == 2212)
          par_tracks++;
      }
      return event_tracks - par_tracks;
    }

    double trackMatchType(const Particle* particle)
    {
      // Particle does not contain a ECL Cluster
      double result = -1.0;

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        // No associated track is default
        result = 0;
        if (cluster->isTrack()) {
          // There is a track match
          result = 1.0;
        }
      }
      return result;
    }

    double isPrimarySignal(const Particle* part)
    {
      if (isSignal(part) > 0.5 and particleMCPrimaryParticle(part) > 0.5)
        return 1.0;
      else
        return 0.0;
    }

    double False(const Particle*)
    {
      return 0;
    }

    double True(const Particle*)
    {
      return 1;
    }

    double infinity(const Particle*)
    {
      double inf = std::numeric_limits<double>::infinity();
      return inf;
    }

    double random(const Particle*)
    {
      return gRandom->Uniform(0, 1);
    }




    VARIABLE_GROUP("Kinematics");
    REGISTER_VARIABLE("p", particleP, "momentum magnitude");
    REGISTER_VARIABLE("E", particleE, "energy");
    REGISTER_VARIABLE("px", particlePx, "momentum component x");
    REGISTER_VARIABLE("py", particlePy, "momentum component y");
    REGISTER_VARIABLE("pz", particlePz, "momentum component z");
    REGISTER_VARIABLE("pt", particlePt, "transverse momentum");
    REGISTER_VARIABLE("pErr", particlePErr, "error of momentum magnitude");
    REGISTER_VARIABLE("pxErr", particlePxErr, "error of momentum component x");
    REGISTER_VARIABLE("pyErr", particlePyErr, "error of momentum component y");
    REGISTER_VARIABLE("pzErr", particlePzErr, "error of momentum component z");
    REGISTER_VARIABLE("ptErr", particlePtErr, "error of transverse momentum");
    REGISTER_VARIABLE("momVertCovM(i,j)", covMatrixElement,
                      "returns the (i,j)-th element of the MomentumVertex Covariance Matrix (7x7).\n"
                      "Order of elements in the covariance matrix is: px, py, pz, E, x, y, z.");
    REGISTER_VARIABLE("momDevChi2", momentumDeviationChi2,
                      "momentum deviation chi^2 value calculated as"
                      "chi^2 = sum_i (p_i - mc(p_i))^2/sigma(p_i)^2, where sum runs over i = px, py, pz and"
                      "mc(p_i) is the mc truth value and sigma(p_i) is the estimated error of i-th component of momentum vector")
    REGISTER_VARIABLE("Theta", particleTheta, "polar angle");
    REGISTER_VARIABLE("ThetaErr", particleThetaErr, "error of polar angle");
    REGISTER_VARIABLE("cosTheta", particleCosTheta, "momentum cosine of polar angle");
    REGISTER_VARIABLE("cosThetaErr", particleCosThetaErr, "error of momentum cosine of polar angle");
    REGISTER_VARIABLE("phi", particlePhi, "momentum azimuthal angle in degrees");
    REGISTER_VARIABLE("phiErr", particlePhiErr, "error of momentum azimuthal angle in degrees");
    REGISTER_VARIABLE("PDG", particlePDGCode, "PDG code");

    REGISTER_VARIABLE("cosAngleBetweenMomentumAndVertexVector",
                      cosAngleBetweenMomentumAndVertexVector,
                      "cosine of the angle between momentum and vertex vector (vector connecting ip and fitted vertex) of this particle");
    REGISTER_VARIABLE("cosThetaBetweenParticleAndTrueB",
                      cosThetaBetweenParticleAndTrueB,
                      "cosine of the angle between momentum the particle and a true B particle. Is somewhere between -1 and 1 if only a massless particle like a neutrino is missing in the reconstruction.");
    REGISTER_VARIABLE("cosHelicityAngle",
                      cosHelicityAngle,
                      "If the given particle has two daughters: cosine of the angle between the line defined by the momentum difference of the two daughters in the frame of the given particle (mother)"
                      "and the momentum of the given particle in the lab frame\n"
                      "If the given particle has three daughters: cosine of the angle between the normal vector of the plane defined by the momenta of the three daughters in the frame of the given particle (mother)"
                      "and the momentum of the given particle in the lab frame.\n"
                      "Else: 0.");
    REGISTER_VARIABLE("cosHelicityAnglePi0Dalitz",
                      cosHelicityAnglePi0Dalitz,
                      "To be used for the decay pi0 -> e+ e- gamma: cosine of the angle between the momentum of the gamma in the frame of the given particle (mother)"
                      "and the momentum of the given particle in the lab frame.\n"
                      "Else: 0.");

    REGISTER_VARIABLE("ImpactXY"  , ImpactXY , "The impact parameter of the given particle in the xy plane");

    REGISTER_VARIABLE("distance", particleDistance,
                      "3D distance relative to interaction point");
    REGISTER_VARIABLE("significanceOfDistance", particleDistanceSignificance,
                      "significance of distance relative to interaction point(-1 in case of numerical problems)");
    REGISTER_VARIABLE("dx", particleDX, "x in respect to IP");
    REGISTER_VARIABLE("dy", particleDY, "y in respect to IP");
    REGISTER_VARIABLE("dz", particleDZ, "z in respect to IP");
    REGISTER_VARIABLE("x", particleDX, "x coordinate of vertex");
    REGISTER_VARIABLE("y", particleDY, "y coordinate of vertex");
    REGISTER_VARIABLE("z", particleDZ, "z coordinate of vertex");
    REGISTER_VARIABLE("x_uncertainty", particleDXUncertainty, "uncertainty on x");
    REGISTER_VARIABLE("y_uncertainty", particleDYUncertainty, "uncertainty on y");
    REGISTER_VARIABLE("z_uncertainty", particleDZUncertainty, "uncertainty on z");
    REGISTER_VARIABLE("dr", particleDRho, "transverse distance in respect to IP");
    REGISTER_VARIABLE("dphi", particleDPhi, "vertex azimuthal angle in degrees in respect to IP");
    REGISTER_VARIABLE("dcosTheta", particleDCosTheta, "vertex polar angle in respect to IP");

    REGISTER_VARIABLE("M", particleMass,
                      "invariant mass(determined from particle's 4-momentum vector)");
    REGISTER_VARIABLE("dM", particleDMass, "mass minus nominal mass");
    REGISTER_VARIABLE("Q", particleQ, "released energy in decay");
    REGISTER_VARIABLE("dQ", particleDQ,
                      "released energy in decay minus nominal one");
    REGISTER_VARIABLE("Mbc", particleMbc, "beam constrained mass");
    REGISTER_VARIABLE("deltaE", particleDeltaE, "energy difference");

    REGISTER_VARIABLE("InvM", particleInvariantMass,
                      "invariant mass (determined from particle's daughter 4 - momentum vectors)");
    REGISTER_VARIABLE("InvMLambda", particleInvariantMassLambda,
                      "invariant mass(determined from particle's daughter 4-momentum vectors)");

    REGISTER_VARIABLE("ErrM", particleInvariantMassError,
                      "uncertainty of invariant mass (determined from particle's daughter 4 - momentum vectors)");
    REGISTER_VARIABLE("SigM", particleInvariantMassSignificance,
                      "signed deviation of particle's invariant mass from its nominal mass");
    REGISTER_VARIABLE("SigMBF", particleInvariantMassBeforeFitSignificance,
                      "signed deviation of particle's invariant mass(determined from particle's daughter 4-momentum vectors) from its nominal mass");
    REGISTER_VARIABLE("missingMass", missingMass,
                      "missing mass squared of second daughter of a Upsilon calculated under the assumption that the first daughter of the Upsilon is the tag side and the energy of the tag side is equal to the beam energy");
    REGISTER_VARIABLE("missingMomentum", missingMomentum,
                      "Missing momentum (magnitude of three-vector) of the particle with respect to the nominal beam momentum in the lab system, pmiss = pbeam - pparticle");
    REGISTER_VARIABLE("missingMomentumTheta", missingMomentumTheta,
                      "Missing momentum polar angle of the particle with respect to the nominal beam momentum in the lab system");
    REGISTER_VARIABLE("missingMomentumPhi", missingMomentumPhi,
                      "Missing azimuthal polar angle of the particle with respect to the nominal beam momentum in the lab system");
    REGISTER_VARIABLE("cosToEvtThrust", cosToThrustOfEvent,
                      "Cosine of the angle between the momentum of the particle and the Thrust of the event in the CM system");

    VARIABLE_GROUP("MC Matching");
    REGISTER_VARIABLE("isSignal", isSignal,
                      "1.0 if Particle is correctly reconstructed (SIGNAL), 0.0 otherwise");
    REGISTER_VARIABLE("isExtendedSignal", isExtendedSignal,
                      "1.0 if Particle is almost correctly reconstructed (SIGNAL), 0.0 otherwise.\n"
                      "Misidentification of charged FSP is allowed.");
    REGISTER_VARIABLE("isPrimarySignal", isPrimarySignal,
                      "1.0 if Particle is correctly reconstructed (SIGNAL) and primary, 0.0 otherwise");
    REGISTER_VARIABLE("genMotherPDG", genMotherPDG,
                      "Check the PDG code of a particles MC mother particle");
    REGISTER_VARIABLE("genMotherID", genMotherIndex,
                      "Check the array index of a particles generated mother");
    REGISTER_VARIABLE("genMotherP", genMotherP,
                      "Generated momentum of a particles MC mother particle");
    REGISTER_VARIABLE("genParticleID", genParticleIndex,
                      "Check the array index of a particle's related MCParticle");
    REGISTER_VARIABLE("isSignalAcceptMissingNeutrino",
                      isSignalAcceptMissingNeutrino,
                      "same as isSignal, but also accept missing neutrino");
    REGISTER_VARIABLE("mcPDG", particleMCMatchPDGCode,
                      "The PDG code of matched MCParticle, 0 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcErrors", particleMCErrors,
                      "The bit pattern indicating the quality of MC match (see MCMatching::MCErrorFlags)");
    REGISTER_VARIABLE("mcMatchWeight", particleMCMatchWeight,
                      "The weight of the Particle -> MCParticle relation (only for the first Relation = largest weight).");
    REGISTER_VARIABLE("nMCMatches", particleNumberOfMCMatch,
                      "The number of relations of this Particle to MCParticle.");
    REGISTER_VARIABLE("mcDecayTime", particleMCMatchDecayTime,
                      "The decay time of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.")
    REGISTER_VARIABLE("mcPX", particleMCMatchPX,
                      "The px of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcPY", particleMCMatchPY,
                      "The py of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcPZ", particleMCMatchPZ,
                      "The pz of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcDX", particleMCMatchDX,
                      "The decay x-Vertex of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcDY", particleMCMatchDY,
                      "The decay y-Vertex of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcDZ", particleMCMatchDZ,
                      "The decay z-Vertex of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcE", particleMCMatchE,
                      "The energy of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcP", particleMCMatchP,
                      "The total momentum of matched MCParticle, -999 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcRecoilMass", particleMCRecoilMass,
                      "The mass recoiling against the particles attached as particle's daughters calculated using MC truth values.");


    REGISTER_VARIABLE("mcVirtual", particleMCVirtualParticle,
                      "Returns 1 if Particle is related to virtual MCParticle, 0 if Particle is related to non - virtual MCParticle,"
                      "-1 if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcInitial", particleMCInitialParticle,
                      "Returns 1 if Particle is related to initial MCParticle, 0 if Particle is related to non - initial MCParticle,"
                      "-1 if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcISR", particleMCISRParticle,
                      "Returns 1 if Particle is related to ISR MCParticle, 0 if Particle is related to non - ISR MCParticle,"
                      "-1 if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcFSR", particleMCFSRParticle,
                      "Returns 1 if Particle is related to FSR MCParticle, 0 if Particle is related to non - FSR MCParticle,"
                      "-1 if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcPhotos", particleMCPhotosParticle,
                      "Returns 1 if Particle is related to Photos MCParticle, 0 if Particle is related to non - Photos MCParticle,"
                      "-1 if Particle is not related to MCParticle.")

    VARIABLE_GROUP("Miscellaneous");
    REGISTER_VARIABLE("nRemainingTracksInEvent",  nRemainingTracksInEvent,
                      "Number of tracks in the event - Number of tracks( = charged FSPs) of particle.");
    REGISTER_VARIABLE("chiProb", particlePvalue, "chi ^ 2 probability of the fit");
    REGISTER_VARIABLE("nDaughters", particleNDaughters,
                      "number of daughter particles");
    REGISTER_VARIABLE("flavor", particleFlavorType,
                      "flavor type of decay(0 = unflavored, 1 = flavored)");
    REGISTER_VARIABLE("charge", particleCharge, "charge of particle");
    REGISTER_VARIABLE("trackMatchType", trackMatchType,
                      "-1 particle has no ECL cluster, 0 particle has no associated track, 1 there is a matched track"
                      "called connected - region(CR) track match");
    REGISTER_VARIABLE("mdstIndex", particleMdstArrayIndex,
                      "StoreArray index(0 - based) of the MDST object from which the Particle was created");
    REGISTER_VARIABLE("mdstSource", particleMdstSource,
                      "mdstSource - unique identifier for identification of Particles that are constructed from the same object in the detector (Track, energy deposit, ...)");
    REGISTER_VARIABLE("CosMdstIndex", particleCosMdstArrayIndex,
                      " Cosinus of StoreArray index(0 - based) of the MDST object from which the Particle was created. To be used for random ranking.");
    REGISTER_VARIABLE("pRecoil", recoilMomentum,
                      "magnitude of 3 - momentum recoiling against given Particle");
    REGISTER_VARIABLE("eRecoil", recoilEnergy,
                      "energy recoiling against given Particle");
    REGISTER_VARIABLE("mRecoil", recoilMass,
                      "invariant mass of the system recoiling against given Particle");
    REGISTER_VARIABLE("m2Recoil", recoilMassSquared,
                      "invariant mass squared of the system recoiling against given Particle");
    REGISTER_VARIABLE("decayTypeRecoil", recoilMCDecayType,
                      "type of the particle decay(no related mcparticle = -1, hadronic = 0, direct leptonic = 1, direct semileptonic = 2,"
                      "lower level leptonic = 3.");

    REGISTER_VARIABLE("printParticle", printParticle,
                      "For debugging, print Particle and daughter PDG codes, plus MC match. Returns 0.");
    REGISTER_VARIABLE("mcSecPhysProc", mcParticleSecondaryPhysicsProcess,
                      "Returns the secondary physics process flag.");
    REGISTER_VARIABLE("mcParticleStatus", mcParticleStatus,
                      "Returns status bits of related MCParticle or - 1 if MCParticle relation is not set.");
    REGISTER_VARIABLE("mcPrimary", particleMCPrimaryParticle,
                      "Returns 1 if Particle is related to primary MCParticle, 0 if Particle is related to non - primary MCParticle,"
                      "-1 if Particle is not related to MCParticle.");
    REGISTER_VARIABLE("mcMomTransfer2", particleMCMomentumTransfer2,
                      "Return the true momentum transfer to lepton pair in a B(semi -) leptonic B meson decay.");
    REGISTER_VARIABLE("False", False,
                      "returns always 0, used for testing and debugging.");
    REGISTER_VARIABLE("True", True,
                      "returns always 1, used for testing and debugging.");



    VARIABLE_GROUP("Other");
    REGISTER_VARIABLE("infinity", infinity,
                      "returns std::numeric_limits<double>::infinity()");
    REGISTER_VARIABLE("random", random, "return a random number between 0 and 1. Can be used, e.g. for picking a random"
                      "candidate in the best candidate selection.");

  }
}
