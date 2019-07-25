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
#include <analysis/variables/Variables.h>
#include <analysis/variables/EventVariables.h>
#include <analysis/variables/VertexVariables.h>
#include <analysis/variables/TrackVariables.h>
#include <analysis/variables/ParameterVariables.h>
#include <analysis/variables/FlightInfoVariables.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ReferenceFrame.h>

#include <analysis/utility/MCMatching.h>
#include <analysis/ClusterUtility/ClusterUtils.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ContinuumSuppression.h>
#include <analysis/dataobjects/EventShapeContainer.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/utilities/Conversion.h>
#include <framework/logging/Logger.h>
#include <framework/core/InputController.h>

#include <TLorentzVector.h>
#include <TRandom.h>
#include <TVectorF.h>
#include <TVector3.h>

#include <boost/lexical_cast.hpp>

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

    double particleClusterEUncertainty(const Particle* part)
    {
      const ECLCluster* cluster = part->getECLCluster();
      const auto EPhiThetaCov = cluster->getCovarianceMatrix3x3();
      if (cluster) {
        return std::sqrt(EPhiThetaCov[0][0]);
      }
      return std::nan("");
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

    double particleEUncertainty(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();

      double errorSquared = frame.getMomentumErrorMatrix(part)(3, 3);

      if (errorSquared > 0.0)
        return std::sqrt(errorSquared);
      else
        return 0.0;
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


    double particleXp(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector p4 = part -> get4Vector();
      TLorentzVector p4CMS = T.rotateLabToCms() * p4;
      float s = T.getCMSEnergy();
      float M = part->getMass();
      return p4CMS.P() / TMath::Sqrt(s * s / 4 - M * M);
    }

    double particlePDGCode(const Particle* part)
    {
      return part->getPDGCode();
    }

    double cosAngleBetweenMomentumAndVertexVectorInXYPlane(const Particle* part)
    {
      PCmsLabTransform T;
      double px = part->getMomentum().Px();
      double py = part->getMomentum().Py();

      double xV = part->getVertex().X();
      double yV = part->getVertex().Y();
      double xIP = T.getBeamParams().getVertex().X();
      double yIP = T.getBeamParams().getVertex().Y();

      double x = xV - xIP;
      double y = yV - yIP;

      double cosangle = (px * x + py * y) / (sqrt(px * px + py * py) * sqrt(x * x + y * y));
      return cosangle;
    }

    double cosAngleBetweenMomentumAndVertexVector(const Particle* part)
    {
      PCmsLabTransform T;
      return std::cos((part->getVertex() - T.getBeamParams().getVertex()).Angle(part->getMomentum()));
    }

    double cosThetaBetweenParticleAndNominalB(const Particle* part)
    {

      int particlePDG = abs(part->getPDGCode());
      if (particlePDG != 511 and particlePDG != 521)
        B2FATAL("The Variables cosThetaBetweenParticleAndNominalB is only meant to be used on B mesons!");

      PCmsLabTransform T;
      // Hardcoded value, how to bypass this?
      double e_Beam = 1.0579400E+1 / 2.0; // GeV
      double m_B = part->getPDGMass();

      double p_B = std::sqrt(e_Beam * e_Beam - m_B * m_B);

      TLorentzVector p = T.rotateLabToCms() * part->get4Vector();
      double e_d = p.E();
      double m_d = p.M();
      double p_d = p.Rho();

      double theta_BY = (2 * e_Beam * e_d - m_B * m_B - m_d * m_d)
                        / (2 * p_B * p_d);
      return theta_BY;
    }

    Manager::FunctionPtr cosHelicityAngleIfCMSIsTheMother(const std::vector<std::string>& arguments)
    {
      int idau = 0;
      if (arguments.size() == 1) {
        try {
          idau = Belle2::convertString<int>(arguments[0]);
        } catch (boost::bad_lexical_cast&) {
          B2FATAL("The argument of cosHelicityAngleWrtCMSFrame must be an integer!");
          return nullptr;
        }
      } else {
        B2FATAL("Wrong number of arguments for cosHelicityAngleIfCMSIsTheMother");
      }
      auto func = [idau](const Particle * mother) -> double {
        const Particle* part = mother->getDaughter(idau);
        if (!part)
        {
          B2FATAL("Couldn't find the " << idau << "th daughter");
          return -999.0;
        }

        TLorentzVector beam4Vector(getBeamPx(nullptr), getBeamPy(nullptr), getBeamPz(nullptr), getBeamE(nullptr));
        TLorentzVector part4Vector = part->get4Vector();
        TLorentzVector mother4Vector = mother->get4Vector();

        TVector3 motherBoost = -(mother4Vector.BoostVector());

        TLorentzVector beam4Vector_motherFrame, part4Vector_motherFrame;
        beam4Vector_motherFrame = beam4Vector;
        part4Vector_motherFrame = part4Vector;

        beam4Vector_motherFrame.Boost(motherBoost);
        part4Vector_motherFrame.Boost(motherBoost);

        return std::cos(beam4Vector_motherFrame.Angle(part4Vector_motherFrame.Vect()));
      };
      return func;
    }


    double cosToThrustOfEvent(const Particle* part)
    {
      StoreObjPtr<EventShapeContainer> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find thrust of event information, did you forget to load the event shape calculation?");
        return std::numeric_limits<float>::quiet_NaN();
      }
      PCmsLabTransform T;
      TVector3 th = evtShape->getThrustAxis();
      TVector3 particleMomentum = (T.rotateLabToCms() * part -> get4Vector()).Vect();
      return std::cos(th.Angle(particleMomentum));
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
        for (auto daughter : daughters)
          sum += daughter->get4Vector();

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

    double particleMassSquared(const Particle* part)
    {
      TLorentzVector p4 = part->get4Vector();
      return p4.M2();
    }

    double b2bTheta(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector pcms = T.rotateLabToCms() * part->get4Vector();
      TLorentzVector b2bcms(-pcms.Px(), -pcms.Py(), -pcms.Pz(), pcms.E());
      TLorentzVector b2blab = T.rotateCmsToLab() * b2bcms;
      return b2blab.Vect().Theta();
    }

    double b2bPhi(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector pcms = T.rotateLabToCms() * part->get4Vector();
      TLorentzVector b2bcms(-pcms.Px(), -pcms.Py(), -pcms.Pz(), pcms.E());
      TLorentzVector b2blab = T.rotateCmsToLab() * b2bcms;
      return b2blab.Vect().Phi();
    }

    double b2bClusterTheta(const Particle* part)
    {
      // get associated ECLCluster
      const ECLCluster* cluster = part->getECLCluster();
      const ECLCluster::EHypothesisBit clusterHypothesis = part->getECLClusterEHypothesisBit();
      if (!cluster) return std::numeric_limits<float>::quiet_NaN();

      // get 4 momentum from cluster
      ClusterUtils clutls;
      TLorentzVector p4Cluster = clutls.Get4MomentumFromCluster(cluster, clusterHypothesis);

      // find the vector that balances this in the CMS
      PCmsLabTransform T;
      TLorentzVector pcms = T.rotateLabToCms() * p4Cluster;
      TLorentzVector b2bcms(-pcms.Px(), -pcms.Py(), -pcms.Pz(), pcms.E());
      TLorentzVector b2blab = T.rotateCmsToLab() * b2bcms;
      return b2blab.Vect().Theta();
    }

    double b2bClusterPhi(const Particle* part)
    {
      // get associated ECLCluster
      const ECLCluster* cluster = part->getECLCluster();
      const ECLCluster::EHypothesisBit clusterHypothesis = part->getECLClusterEHypothesisBit();
      if (!cluster) return std::numeric_limits<float>::quiet_NaN();

      // get 4 momentum from cluster
      ClusterUtils clutls;
      TLorentzVector p4Cluster = clutls.Get4MomentumFromCluster(cluster, clusterHypothesis);

      // find the vector that balances this in the CMS
      PCmsLabTransform T;
      TLorentzVector pcms = T.rotateLabToCms() * p4Cluster;
      TLorentzVector b2bcms(-pcms.Px(), -pcms.Py(), -pcms.Pz(), pcms.E());
      TLorentzVector b2blab = T.rotateCmsToLab() * b2bcms;
      return b2blab.Vect().Phi();
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

      for (auto mcTemp : mcDaug) {
        if (abs(mcTemp->getPDG()) <= 16)
          continue;

        pX += mcTemp->get4Vector();
      }

      TLorentzVector q = pB - pX;

      return q.Mag2();
    }

// Recoil Kinematics related ---------------------------------------------
    double recoilPx(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN = T.getBoostVector();

      return (pIN - particle->get4Vector()).Px();
    }

    double recoilPy(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN = T.getBoostVector();

      return (pIN - particle->get4Vector()).Py();
    }

    double recoilPz(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN = T.getBoostVector();

      return (pIN - particle->get4Vector()).Pz();
    }


    double recoilMomentum(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN = T.getBoostVector();

      return (pIN - particle->get4Vector()).P();
    }

    double recoilMomentumTheta(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN = T.getBoostVector();

      return (pIN - particle->get4Vector()).Vect().Theta();
    }

    double recoilMomentumPhi(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN = T.getBoostVector();

      return (pIN - particle->get4Vector()).Vect().Phi();
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

    double m2RecoilSignalSide(const Particle* part)
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

    double recoilMCDecayType(const Particle* particle)
    {
      auto* mcp = particle->getRelatedTo<MCParticle>();

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
      for (auto& daughter : daughters) {
        if (!daughter->hasStatus(MCParticle::c_PrimaryParticle))
          continue;

        nPrimaryParticleDaughters++;
        if (abs(daughter->getPDG()) > 22)
          nHadronicParticles++;
      }

      if (nPrimaryParticleDaughters > 1) {
        for (auto& daughter : daughters) {
          if (!daughter->hasStatus(MCParticle::c_PrimaryParticle))
            continue;

          if (abs(daughter->getPDG()) == 12 or abs(daughter->getPDG()) == 14 or abs(daughter->getPDG()) == 16) {
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
            checkMCParticleDecay(daughter, decayType, recursive);
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

    double eventRandom(const Particle*)
    {
      std::string key = "__eventRandom";
      StoreObjPtr<EventExtraInfo> eventExtraInfo;
      if (not eventExtraInfo.isValid())
        eventExtraInfo.create();
      if (eventExtraInfo->hasExtraInfo(key)) {
        return eventExtraInfo->getExtraInfo(key);
      } else {
        double value = gRandom->Uniform(0, 1);
        eventExtraInfo->addExtraInfo(key, value);
        return value;
      }
    }

    VARIABLE_GROUP("Kinematics");
    REGISTER_VARIABLE("p", particleP, "momentum magnitude");
    REGISTER_VARIABLE("E", particleE, "energy");

    REGISTER_VARIABLE("E_uncertainty", particleEUncertainty, "energy uncertainty (sqrt(sigma2))");
    REGISTER_VARIABLE("ECLClusterE_uncertainty", particleClusterEUncertainty,
                      "energy uncertainty as given by the underlying ECL cluster.");
    REGISTER_VARIABLE("px", particlePx, "momentum component x");
    REGISTER_VARIABLE("py", particlePy, "momentum component y");
    REGISTER_VARIABLE("pz", particlePz, "momentum component z");
    REGISTER_VARIABLE("pt", particlePt, "transverse momentum");
    REGISTER_VARIABLE("xp", particleXp,
                      "scaled momentum: the momentum of the particle in the CMS as a fraction of its maximum available momentum in the collision");
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
    REGISTER_VARIABLE("theta", particleTheta, "polar angle in radians");
    REGISTER_VARIABLE("thetaErr", particleThetaErr, "error of polar angle in radians");
    REGISTER_VARIABLE("cosTheta", particleCosTheta, "momentum cosine of polar angle");
    REGISTER_VARIABLE("cosThetaErr", particleCosThetaErr, "error of momentum cosine of polar angle");
    REGISTER_VARIABLE("phi", particlePhi, "momentum azimuthal angle in radians");
    REGISTER_VARIABLE("phiErr", particlePhiErr, "error of momentum azimuthal angle in radians");
    REGISTER_VARIABLE("PDG", particlePDGCode, "PDG code");

    REGISTER_VARIABLE("cosAngleBetweenMomentumAndVertexVectorInXYPlane",
                      cosAngleBetweenMomentumAndVertexVectorInXYPlane,
                      "cosine of the angle between momentum and vertex vector (vector connecting ip and fitted vertex) of this particle in xy-plane");
    REGISTER_VARIABLE("cosAngleBetweenMomentumAndVertexVector",
                      cosAngleBetweenMomentumAndVertexVector,
                      "cosine of the angle between momentum and vertex vector (vector connecting ip and fitted vertex) of this particle");
    REGISTER_VARIABLE("cosThetaBetweenParticleAndNominalB",
                      cosThetaBetweenParticleAndNominalB,
                      "cosine of the angle in CMS between momentum the particle and a nominal B particle. It is somewhere between -1 and 1 if only a massless particle like a neutrino is missing in the reconstruction.");
    REGISTER_VARIABLE("cosHelicityAngleIfCMSIsTheMother", cosHelicityAngleIfCMSIsTheMother,
                      "Cosine of the helicity angle of the i-th (where 'i' is the parameter passed to the function) daughter of the particle provided,\n"
                      "assuming that the mother of the provided particle correspond to the Centre of Mass System, whose parameters are\n"
                      "automatically loaded by the function, given the accelerators conditions.");

    REGISTER_VARIABLE("cosToThrustOfEvent", cosToThrustOfEvent,
                      "Returns the cosine of the angle between the particle and the thrust axis of the event, as calculate by the EventShapeCalculator module. buildEventShape() must be run before calling this variable")

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


    REGISTER_VARIABLE("M", particleMass,
                      "invariant mass (determined from particle's 4-momentum vector)");
    REGISTER_VARIABLE("dM", particleDMass, "mass minus nominal mass");
    REGISTER_VARIABLE("Q", particleQ, "released energy in decay");
    REGISTER_VARIABLE("dQ", particleDQ,
                      "released energy in decay minus nominal one");
    REGISTER_VARIABLE("Mbc", particleMbc, "beam constrained mass");
    REGISTER_VARIABLE("deltaE", particleDeltaE, "energy difference");
    REGISTER_VARIABLE("M2", particleMassSquared,
                      "invariant mass squared (determined from particle's 4-momentum vector)");

    REGISTER_VARIABLE("InvM", particleInvariantMass,
                      "invariant mass (determined from particle's daughter 4-momentum vectors)");
    REGISTER_VARIABLE("InvMLambda", particleInvariantMassLambda,
                      "invariant mass (determined from particle's daughter 4-momentum vectors)");

    REGISTER_VARIABLE("ErrM", particleInvariantMassError,
                      "uncertainty of invariant mass (determined from particle's daughter 4 - momentum vectors)");
    REGISTER_VARIABLE("SigM", particleInvariantMassSignificance,
                      "signed deviation of particle's invariant mass from its nominal mass");
    REGISTER_VARIABLE("SigMBF", particleInvariantMassBeforeFitSignificance,
                      "signed deviation of particle's invariant mass(determined from particle's daughter 4-momentum vectors) from its nominal mass");

    REGISTER_VARIABLE("pxRecoil", recoilPx,
                      "component x of 3-momentum recoiling against given Particle");
    REGISTER_VARIABLE("pyRecoil", recoilPy,
                      "component y of 3-momentum recoiling against given Particle");
    REGISTER_VARIABLE("pzRecoil", recoilPz,
                      "component z of 3-momentum recoiling against given Particle");

    REGISTER_VARIABLE("pRecoil", recoilMomentum,
                      "magnitude of 3 - momentum recoiling against given Particle");
    REGISTER_VARIABLE("pRecoilTheta", recoilMomentumTheta,
                      "Polar angle of a particle's missing momentum in the lab system");
    REGISTER_VARIABLE("pRecoilPhi", recoilMomentumPhi,
                      "Azimutal angle of a particle's missing momentum in the lab system");
    REGISTER_VARIABLE("eRecoil", recoilEnergy,
                      "energy recoiling against given Particle");
    REGISTER_VARIABLE("mRecoil", recoilMass,
                      "Invariant mass of the system recoiling against given Particle");
    REGISTER_VARIABLE("m2Recoil", recoilMassSquared,
                      "invariant mass squared of the system recoiling against given Particle");
    REGISTER_VARIABLE("m2RecoilSignalSide", m2RecoilSignalSide,
                      "Squared recoil mass of the signal side which is calculated in the CMS frame under the assumption that the signal and tag side are produced back to back and the tag side energy equals the beam energy. The variable must be applied to the Upsilon and the tag side must be the first, the signal side the second daughter ");


    REGISTER_VARIABLE("b2bTheta", b2bTheta,
                      "Polar angle in the lab system that is back-to-back to the particle in the CMS. Useful for low multiplicity studies.")
    REGISTER_VARIABLE("b2bPhi", b2bPhi,
                      "Azimuthal angle in the lab system that is back-to-back to the particle in the CMS. Useful for low multiplicity studies.")
    REGISTER_VARIABLE("b2bClusterTheta", b2bClusterTheta,
                      "Polar angle in the lab system that is back-to-back to the particle's associated ECLCluster in the CMS. Returns NAN if no cluster is found. Useful for low multiplicity studies.")
    REGISTER_VARIABLE("b2bClusterPhi", b2bClusterPhi,
                      "Azimuthal angle in the lab system that is back-to-back to the particle's associated ECLCluster in the CMS. Returns NAN if no cluster is found. Useful for low multiplicity studies.")

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

    REGISTER_VARIABLE("decayTypeRecoil", recoilMCDecayType,
                      "type of the particle decay(no related mcparticle = -1, hadronic = 0, direct leptonic = 1, direct semileptonic = 2,"
                      "lower level leptonic = 3.");

    REGISTER_VARIABLE("printParticle", printParticle,
                      "For debugging, print Particle and daughter PDG codes, plus MC match. Returns 0.");
    REGISTER_VARIABLE("mcMomTransfer2", particleMCMomentumTransfer2,
                      "Return the true momentum transfer to lepton pair in a B(semi -) leptonic B meson decay.");
    REGISTER_VARIABLE("False", False,
                      "returns always 0, used for testing and debugging.");
    REGISTER_VARIABLE("True", True,
                      "returns always 1, used for testing and debugging.");
    REGISTER_VARIABLE("infinity", infinity,
                      "returns std::numeric_limits<double>::infinity()");
    REGISTER_VARIABLE("random", random,
                      "return a random number between 0 and 1 for each candidate. Can be used, e.g. for picking a random"
                      "candidate in the best candidate selection.");
    REGISTER_VARIABLE("eventRandom", eventRandom,
                      "[Eventbased] Returns a random number between 0 and 1 for this event. Can be used, e.g. for applying an event prescale.");

  }
}
