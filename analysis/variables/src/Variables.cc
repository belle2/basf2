/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/variables/Variables.h>

// include VariableManager
#include <analysis/VariableManager/Manager.h>

#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ReferenceFrame.h>
#include <analysis/utility/MCMatching.h>
#include <analysis/ClusterUtility/ClusterUtils.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <framework/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/EventShapeContainer.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/V0.h>

#include <mdst/dbobjects/BeamSpot.h>

// framework aux
#include <framework/logging/Logger.h>
#include <framework/geometry/B2Vector3.h>
#include <framework/geometry/BFieldManager.h>
#include <framework/gearbox/Const.h>
#include <framework/utilities/Conversion.h>

#include <Math/Vector4D.h>
#include <TRandom.h>
#include <TVectorF.h>

#include <iostream>
#include <cmath>
#include <boost/algorithm/string.hpp>


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
      if (cluster) {
        ClusterUtils clutls;
        const auto EPhiThetaCov = clutls.GetCovarianceMatrix3x3FromCluster(cluster);
        return sqrt(fabs(EPhiThetaCov[0][0]));
      }
      return Const::doubleNaN;
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
        B2WARNING("Requested particle's momentumVertex covariance matrix element is out of boundaries [0 - 6]:" << LogVar("i", elementI));
        return Const::doubleNaN;
      }
      if (elementJ < 0 || elementJ > 6) {
        B2WARNING("Requested particle's momentumVertex covariance matrix element is out of boundaries [0 - 6]:" << LogVar("j", elementJ));
        return Const::doubleNaN;
      }

      return part->getMomentumVertexErrorMatrix()(elementI, elementJ);
    }

    double particleEUncertainty(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();

      double errorSquared = frame.getMomentumErrorMatrix(part)(3, 3);

      if (errorSquared >= 0.0)
        return sqrt(errorSquared);
      else
        return Const::doubleNaN;
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

      if (errorSquared >= 0.0)
        return sqrt(errorSquared);
      else
        return Const::doubleNaN;
    }

    double particlePxErr(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();

      double errorSquared = frame.getMomentumErrorMatrix(part)(0, 0);

      if (errorSquared >= 0.0)
        return sqrt(errorSquared);
      else
        return Const::doubleNaN;
    }

    double particlePyErr(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      double errorSquared = frame.getMomentumErrorMatrix(part)(1, 1);

      if (errorSquared >= 0.0)
        return sqrt(errorSquared);
      else
        return Const::doubleNaN;
    }

    double particlePzErr(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      double errorSquared = frame.getMomentumErrorMatrix(part)(2, 2);

      if (errorSquared >= 0.0)
        return sqrt(errorSquared);
      else
        return Const::doubleNaN;
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

      if (errorSquared >= 0.0)
        return sqrt(errorSquared);
      else
        return Const::doubleNaN;
    }

    double momentumDeviationChi2(const Particle* part)
    {
      double result = Const::doubleNaN;

      // check if error matrix is set
      if (part->getPValue() < 0.0)
        return result;

      // check if mc match exists
      const MCParticle* mcp = part->getMCParticle();
      if (mcp == nullptr)
        return result;

      result = 0.0;
      result += TMath::Power(part->getPx() - mcp->getMomentum().X(), 2.0) / part->getMomentumVertexErrorMatrix()(0, 0);
      result += TMath::Power(part->getPy() - mcp->getMomentum().Y(), 2.0) / part->getMomentumVertexErrorMatrix()(1, 1);
      result += TMath::Power(part->getPz() - mcp->getMomentum().Z(), 2.0) / part->getMomentumVertexErrorMatrix()(2, 2);

      return result;
    }

    double particleTheta(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(part).Theta();
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

      if (errorSquared >= 0.0)
        return sqrt(errorSquared);
      else
        return Const::doubleNaN;
    }

    double particleCosTheta(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return cos(frame.getMomentum(part).Theta());
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

      if (errorSquared >= 0.0)
        return sqrt(errorSquared);
      else
        return Const::doubleNaN;
    }

    double particleXp(const Particle* part)
    {
      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector p4 = part -> get4Vector();
      ROOT::Math::PxPyPzEVector p4CMS = T.rotateLabToCms() * p4;
      float s = T.getCMSEnergy();
      float M = part->getMass();
      return p4CMS.P() / TMath::Sqrt(s * s / 4 - M * M);
    }

    int particlePDGCode(const Particle* part)
    {
      return part->getPDGCode();
    }

    double cosAngleBetweenMomentumAndVertexVectorInXYPlane(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      double px = part->getPx();
      double py = part->getPy();

      double xV = part->getX();
      double yV = part->getY();

      double xIP = (beamSpotDB->getIPPosition()).X();
      double yIP = (beamSpotDB->getIPPosition()).Y();

      double x = xV - xIP;
      double y = yV - yIP;

      double cosangle = (px * x + py * y) / (sqrt(px * px + py * py) * sqrt(x * x + y * y));
      return cosangle;
    }

    double cosAngleBetweenMomentumAndVertexVector(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      return cos((B2Vector3D(part->getVertex()) - beamSpotDB->getIPPosition()).Angle(B2Vector3D(part->getMomentum())));
    }

    double cosThetaBetweenParticleAndNominalB(const Particle* part)
    {

      int particlePDG = abs(part->getPDGCode());
      if (particlePDG != 511 and particlePDG != 521)
        B2FATAL("The Variables cosThetaBetweenParticleAndNominalB is only meant to be used on B mesons!");

      PCmsLabTransform T;
      double e_Beam = T.getCMSEnergy() / 2.0; // GeV
      double m_B = part->getPDGMass();
      // if this is a continuum run, use an approximate Y(4S) CMS energy
      if (e_Beam * e_Beam - m_B * m_B < 0) {
        e_Beam = 1.0579400E+1 / 2.0; // GeV
      }
      double p_B = std::sqrt(e_Beam * e_Beam - m_B * m_B);

      ROOT::Math::PxPyPzEVector p = T.rotateLabToCms() * part->get4Vector();
      double e_d = p.E();
      double m_d = p.M();
      double p_d = p.P();

      double theta_BY = (2 * e_Beam * e_d - m_B * m_B - m_d * m_d)
                        / (2 * p_B * p_d);
      return theta_BY;
    }

    double cosToThrustOfEvent(const Particle* part)
    {
      StoreObjPtr<EventShapeContainer> evtShape;
      if (!evtShape) {
        B2WARNING("Cannot find thrust of event information, did you forget to load the event shape calculation?");
        return Const::doubleNaN;
      }
      PCmsLabTransform T;
      B2Vector3D th = evtShape->getThrustAxis();
      B2Vector3D particleMomentum = (T.rotateLabToCms() * part -> get4Vector()).Vect();
      return std::cos(th.Angle(particleMomentum));
    }

    double ImpactXY(const Particle* particle)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;

      ROOT::Math::XYZVector mom = particle->getMomentum();

      ROOT::Math::XYZVector r = particle->getVertex() - ROOT::Math::XYZVector(beamSpotDB->getIPPosition());

      ROOT::Math::XYZVector Bfield = BFieldManager::getInstance().getFieldInTesla(ROOT::Math::XYZVector(beamSpotDB->getIPPosition()));

      ROOT::Math::XYZVector curvature = - Bfield * Const::speedOfLight * particle->getCharge(); //Curvature of the track
      double T = TMath::Sqrt(mom.Perp2() - 2.0 * curvature.Dot(r.Cross(mom)) + curvature.Mag2() * r.Perp2());

      return TMath::Abs((-2 * r.Cross(mom).Z() + curvature.R() * r.Perp2()) / (T + mom.Rho()));
    }

    double ArmenterosLongitudinalMomentumAsymmetry(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      int nDaughters = part -> getNDaughters();
      if (nDaughters != 2)
        B2FATAL("You are trying to use an Armenteros variable. The mother particle is required to have exactly two daughters");

      const auto& daughters = part -> getDaughters();
      B2Vector3D motherMomentum = frame.getMomentum(part).Vect();
      B2Vector3D daughter1Momentum = frame.getMomentum(daughters[0]).Vect();
      B2Vector3D daughter2Momentum = frame.getMomentum(daughters[1]).Vect();

      int daughter1Charge = daughters[0] -> getCharge();
      int daughter2Charge = daughters[1] -> getCharge();
      double daughter1Ql = daughter1Momentum.Dot(motherMomentum) / motherMomentum.Mag();
      double daughter2Ql = daughter2Momentum.Dot(motherMomentum) / motherMomentum.Mag();

      double Arm_alpha;
      if (daughter2Charge > daughter1Charge)
        Arm_alpha = (daughter2Ql - daughter1Ql) / (daughter2Ql + daughter1Ql);
      else
        Arm_alpha = (daughter1Ql - daughter2Ql) / (daughter1Ql + daughter2Ql);

      return Arm_alpha;
    }

    double ArmenterosDaughter1Qt(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      int nDaughters = part -> getNDaughters();
      if (nDaughters != 2)
        B2FATAL("You are trying to use an Armenteros variable. The mother particle is required to have exactly two daughters.");

      const auto& daughters = part -> getDaughters();
      B2Vector3D motherMomentum = frame.getMomentum(part).Vect();
      B2Vector3D daughter1Momentum = frame.getMomentum(daughters[0]).Vect();
      double qt = daughter1Momentum.Perp(motherMomentum);

      return qt;
    }

    double ArmenterosDaughter2Qt(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      int nDaughters = part -> getNDaughters();
      if (nDaughters != 2)
        B2FATAL("You are trying to use an Armenteros variable. The mother particle is required to have exactly two daughters.");

      const auto& daughters = part -> getDaughters();
      B2Vector3D motherMomentum = frame.getMomentum(part).Vect();
      B2Vector3D daughter2Momentum = frame.getMomentum(daughters[1]).Vect();
      double qt = daughter2Momentum.Perp(motherMomentum);

      return qt;
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

    double particleInvariantMassFromDaughters(const Particle* part)
    {
      const std::vector<Particle*> daughters = part->getDaughters();
      if (daughters.size() > 0) {
        ROOT::Math::PxPyPzEVector sum;
        for (auto daughter : daughters)
          sum += daughter->get4Vector();

        return sum.M();
      } else {
        return part->getMass(); // !
      }
    }

    double particleInvariantMassFromDaughtersDisplaced(const Particle* part)
    {
      ROOT::Math::XYZVector vertex = part->getVertex();
      if (part->getParticleSource() != Particle::EParticleSourceObject::c_V0
          && vertex.Rho() < 0.5) return particleInvariantMassFromDaughters(part);

      const std::vector<Particle*> daughters = part->getDaughters();
      if (daughters.size() == 0) return particleInvariantMassFromDaughters(part);

      const double bField = BFieldManager::getFieldInTesla(vertex).Z();
      ROOT::Math::PxPyPzMVector sum;
      for (auto daughter : daughters) {
        const TrackFitResult* tfr = daughter->getTrackFitResult();
        if (!tfr) {
          sum += daughter->get4Vector();
          continue;
        }
        Helix helix = tfr->getHelix();
        helix.passiveMoveBy(vertex);
        double scalingFactor = daughter->getEffectiveMomentumScale();
        double momX = scalingFactor * helix.getMomentumX(bField);
        double momY = scalingFactor * helix.getMomentumY(bField);
        double momZ = scalingFactor * helix.getMomentumZ(bField);
        float mPDG = daughter->getPDGMass();
        sum += ROOT::Math::PxPyPzMVector(momX, momY, momZ, mPDG);
      }
      return sum.M();
    }

    double particleInvariantMassLambda(const Particle* part)
    {
      const std::vector<Particle*> daughters = part->getDaughters();
      if (daughters.size() == 2) {
        ROOT::Math::PxPyPzEVector dt1;
        ROOT::Math::PxPyPzEVector dt2;
        ROOT::Math::PxPyPzEVector dtsum;
        double mpi = Const::pionMass;
        double mpr = Const::protonMass;
        dt1 = daughters[0]->get4Vector();
        dt2 = daughters[1]->get4Vector();
        double E1 = hypot(mpi, dt1.P());
        double E2 = hypot(mpr, dt2.P());
        dtsum = dt1 + dt2;
        return sqrt((E1 + E2) * (E1 + E2) - dtsum.P() * dtsum.P());

      } else {
        return part->getMass();
      }
    }

    double particleInvariantMassError(const Particle* part)
    {
      float invMass = part->getMass();
      TMatrixFSym covarianceMatrix = part->getMomentumErrorMatrix();

      TVectorF jacobian(Particle::c_DimMomentum);
      jacobian[0] = -1.0 * part->getPx() / invMass;
      jacobian[1] = -1.0 * part->getPy() / invMass;
      jacobian[2] = -1.0 * part->getPz() / invMass;
      jacobian[3] = 1.0 * part->getEnergy() / invMass;

      double result = jacobian * (covarianceMatrix * jacobian);

      if (result < 0.0)
        return Const::doubleNaN;

      return TMath::Sqrt(result);
    }

    double particleInvariantMassSignificance(const Particle* part)
    {
      return particleDMass(part) / particleInvariantMassError(part);
    }

    double particleMassSquared(const Particle* part)
    {
      ROOT::Math::PxPyPzEVector p4 = part->get4Vector();
      return p4.M2();
    }

    double b2bTheta(const Particle* part)
    {
      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector pcms = T.rotateLabToCms() * part->get4Vector();
      ROOT::Math::PxPyPzEVector b2bcms(-pcms.Px(), -pcms.Py(), -pcms.Pz(), pcms.E());
      ROOT::Math::PxPyPzEVector b2blab = T.rotateCmsToLab() * b2bcms;
      return b2blab.Theta();
    }

    double b2bPhi(const Particle* part)
    {
      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector pcms = T.rotateLabToCms() * part->get4Vector();
      ROOT::Math::PxPyPzEVector b2bcms(-pcms.Px(), -pcms.Py(), -pcms.Pz(), pcms.E());
      ROOT::Math::PxPyPzEVector b2blab = T.rotateCmsToLab() * b2bcms;
      return b2blab.Phi();
    }

    double b2bClusterTheta(const Particle* part)
    {
      // get associated ECLCluster
      const ECLCluster* cluster = part->getECLCluster();
      if (!cluster) return Const::doubleNaN;
      const ECLCluster::EHypothesisBit clusterHypothesis = part->getECLClusterEHypothesisBit();

      // get 4 momentum from cluster
      ClusterUtils clutls;
      ROOT::Math::PxPyPzEVector p4Cluster = clutls.Get4MomentumFromCluster(cluster, clusterHypothesis);

      // find the vector that balances this in the CMS
      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector pcms = T.rotateLabToCms() * p4Cluster;
      ROOT::Math::PxPyPzEVector b2bcms(-pcms.Px(), -pcms.Py(), -pcms.Pz(), pcms.E());
      ROOT::Math::PxPyPzEVector b2blab = T.rotateCmsToLab() * b2bcms;
      return b2blab.Theta();
    }

    double b2bClusterPhi(const Particle* part)
    {
      // get associated ECLCluster
      const ECLCluster* cluster = part->getECLCluster();
      if (!cluster) return Const::doubleNaN;
      const ECLCluster::EHypothesisBit clusterHypothesis = part->getECLClusterEHypothesisBit();

      // get 4 momentum from cluster
      ClusterUtils clutls;
      ROOT::Math::PxPyPzEVector p4Cluster = clutls.Get4MomentumFromCluster(cluster, clusterHypothesis);

      // find the vector that balances this in the CMS
      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector pcms = T.rotateLabToCms() * p4Cluster;
      ROOT::Math::PxPyPzEVector b2bcms(-pcms.Px(), -pcms.Py(), -pcms.Pz(), pcms.E());
      ROOT::Math::PxPyPzEVector b2blab = T.rotateCmsToLab() * b2bcms;
      return b2blab.Phi();
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
      ROOT::Math::PxPyPzEVector vec = T.rotateLabToCms() * part->get4Vector();
      double E = T.getCMSEnergy() / 2;
      double m2 = E * E - vec.P2();
      double mbc = m2 >= 0 ? sqrt(m2) : Const::doubleNaN;
      return mbc;
    }

    double particleDeltaE(const Particle* part)
    {
      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.E() - T.getCMSEnergy() / 2;
    }

// other ------------------------------------------------------------


    void printParticleInternal(const Particle* p, int depth)
    {
      stringstream s("");
      for (int i = 0; i < depth; i++) {
        s << "    ";
      }
      s << p->getPDGCode();
      const MCParticle* mcp = p->getMCParticle();
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

    bool printParticle(const Particle* p)
    {
      printParticleInternal(p, 0);
      return 0;
    }


    double particleMCMomentumTransfer2(const Particle* part)
    {
      // for B meson MC particles only
      const MCParticle* mcB = part->getMCParticle();

      if (!mcB)
        return Const::doubleNaN;

      ROOT::Math::PxPyPzEVector pB = mcB->get4Vector();

      std::vector<MCParticle*> mcDaug = mcB->getDaughters();

      if (mcDaug.empty())
        return Const::doubleNaN;

      // B -> X l nu
      // q = pB - pX
      ROOT::Math::PxPyPzEVector pX;

      for (auto mcTemp : mcDaug) {
        if (abs(mcTemp->getPDG()) <= 16)
          continue;

        pX += mcTemp->get4Vector();
      }

      ROOT::Math::PxPyPzEVector q = pB - pX;

      return q.M2();
    }

// Recoil Kinematics related ---------------------------------------------
    double recoilPx(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      ROOT::Math::PxPyPzEVector pIN = T.getBeamFourMomentum();

      // Use requested frame for final calculation
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(pIN - particle->get4Vector()).Px();
    }

    double recoilPy(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      ROOT::Math::PxPyPzEVector pIN = T.getBeamFourMomentum();

      // Use requested frame for final calculation
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(pIN - particle->get4Vector()).Py();
    }

    double recoilPz(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      ROOT::Math::PxPyPzEVector pIN = T.getBeamFourMomentum();

      // Use requested frame for final calculation
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(pIN - particle->get4Vector()).Pz();
    }

    double recoilMomentum(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      ROOT::Math::PxPyPzEVector pIN = T.getBeamFourMomentum();

      // Use requested frame for final calculation
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(pIN - particle->get4Vector()).P();
    }

    double recoilMomentumTheta(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      ROOT::Math::PxPyPzEVector pIN = T.getBeamFourMomentum();

      // Use requested frame for final calculation
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(pIN - particle->get4Vector()).Theta();
    }

    double recoilMomentumPhi(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      ROOT::Math::PxPyPzEVector pIN = T.getBeamFourMomentum();

      // Use requested frame for final calculation
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(pIN - particle->get4Vector()).Phi();
    }

    double recoilEnergy(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      ROOT::Math::PxPyPzEVector pIN = T.getBeamFourMomentum();

      // Use requested frame for final calculation
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(pIN - particle->get4Vector()).E();
    }

    double recoilMass(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      ROOT::Math::PxPyPzEVector pIN = T.getBeamFourMomentum();

      // Use requested frame for final calculation
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(pIN - particle->get4Vector()).M();
    }

    double recoilMassSquared(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      ROOT::Math::PxPyPzEVector pIN = T.getBeamFourMomentum();

      // Use requested frame for final calculation
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(pIN - particle->get4Vector()).M2();
    }

    double m2RecoilSignalSide(const Particle* part)
    {
      PCmsLabTransform T;
      double beamEnergy = T.getCMSEnergy() / 2.;
      if (part->getNDaughters() != 2) return Const::doubleNaN;
      ROOT::Math::PxPyPzEVector tagVec = T.rotateLabToCms() * part->getDaughter(0)->get4Vector();
      ROOT::Math::PxPyPzEVector sigVec = T.rotateLabToCms() * part->getDaughter(1)->get4Vector();
      tagVec.SetE(-beamEnergy);
      return (-tagVec - sigVec).M2();
    }

    double recoilMCDecayType(const Particle* particle)
    {
      auto* mcp = particle->getMCParticle();

      if (!mcp)
        return Const::doubleNaN;

      MCParticle* mcMother = mcp->getMother();

      if (!mcMother)
        return Const::doubleNaN;

      std::vector<MCParticle*> daughters = mcMother->getDaughters();

      if (daughters.size() != 2)
        return Const::doubleNaN;

      MCParticle* recoilMC = nullptr;
      if (daughters[0]->getArrayIndex() == mcp->getArrayIndex())
        recoilMC = daughters[1];
      else
        recoilMC = daughters[0];

      if (!recoilMC->hasStatus(MCParticle::c_PrimaryParticle))
        return Const::doubleNaN;

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
        if (abs(daughter->getPDG()) > Const::photon.getPDGCode())
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

    int nRemainingTracksInEvent(const Particle* particle)
    {

      StoreArray<Track> tracks;
      int event_tracks = tracks.getEntries();

      int par_tracks = 0;
      const auto& daughters = particle->getFinalStateDaughters();
      for (const auto& daughter : daughters) {
        int pdg = abs(daughter->getPDGCode());
        if (pdg == Const::electron.getPDGCode() or pdg == Const::muon.getPDGCode() or pdg == Const::pion.getPDGCode()
            or pdg == Const::kaon.getPDGCode() or pdg == Const::proton.getPDGCode())
          par_tracks++;
      }
      return event_tracks - par_tracks;
    }

    double trackMatchType(const Particle* particle)
    {
      // Particle does not contain a ECL Cluster
      double result = Const::doubleNaN;

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


    bool False(const Particle*)
    {
      return 0;
    }

    bool True(const Particle*)
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

    Manager::FunctionPtr particleDistToClosestExtTrk(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 3 && arguments.size() != 4) {
        B2ERROR("Wrong number of arguments (3 or 4 required) for meta variable minET2ETDist");
        return nullptr;
      }
      bool useHighestProbMassForExt(true);
      if (arguments.size() == 4) {
        try {
          useHighestProbMassForExt = static_cast<bool>(Belle2::convertString<int>(arguments[3]));
        } catch (std::invalid_argument& e) {
          B2ERROR("Fourth (optional) argument of minET2ETDist must be an integer flag.");
          return nullptr;
        }
      }

      std::string detName = arguments[0];
      std::string detLayer = arguments[1];
      std::string referenceListName = arguments[2];
      std::string extraSuffix = (useHighestProbMassForExt) ? "__useHighestProbMassForExt" : "";
      // Distance to closets neighbour at this detector layer.
      std::string extraInfo = "distToClosestTrkAt" + detName + detLayer + "_VS_" + referenceListName + extraSuffix;

      auto func = [ = ](const Particle * part) -> double {
        auto dist = (part->hasExtraInfo(extraInfo)) ? part->getExtraInfo(extraInfo) : Const::doubleNaN;
        return dist;
      };

      return func;
    }

// Track helix extrapolation-based isolation --------------------------------------------------

    Manager::FunctionPtr particleDistToClosestExtTrkVar(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 4) {
        B2ERROR("Wrong number of arguments (4 required) for meta variable minET2ETDistVar");
        return nullptr;
      }

      std::string detName = arguments[0];
      std::string detLayer = arguments[1];
      std::string referenceListName = arguments[2];
      std::string variableName = arguments[3];
      // Mdst array index of the particle that is closest to the particle in question.
      std::string extraInfo = "idxOfClosestPartAt" + detName + detLayer + "In_" + referenceListName;

      auto func = [ = ](const Particle * part) -> double {

        StoreObjPtr<ParticleList> refPartList(referenceListName);
        if (!refPartList.isValid())
        {
          B2FATAL("Invalid Listname " << referenceListName << " given to minET2ETDistVar!");
        }

        if (!part->hasExtraInfo(extraInfo))
        {
          return Const::doubleNaN;
        }

        const Variable::Manager::Var* var = Manager::Instance().getVariable(variableName);
        auto refPart = refPartList->getParticleWithMdstIdx(part->getExtraInfo(extraInfo));

        return std::get<double>(var->function(refPart));
      };

      return func;
    }


    Manager::FunctionPtr particleExtTrkIsoScoreVar(const std::vector<std::string>& arguments)
    {

      if (arguments.size() < 3) {
        B2ERROR("Wrong number of arguments (at least 3 required) for meta variable minET2ETIsoScore");
        return nullptr;
      }

      std::string referenceListName = arguments[0];
      bool useHighestProbMassForExt;
      try {
        useHighestProbMassForExt = static_cast<bool>(Belle2::convertString<int>(arguments[1]));
      } catch (std::invalid_argument& e) {
        B2ERROR("Second argument must be an integer flag.");
        return nullptr;
      }
      std::string extraSuffix = (useHighestProbMassForExt) ? "__useHighestProbMassForExt" : "";

      std::vector<std::string> detectorNames(arguments.begin() + 2, arguments.end());

      std::string detNamesConcat("");
      for (auto& detName : detectorNames) {
        boost::to_upper(detName);
        detNamesConcat += "_" + detName;
      }

      std::string extraInfo = "trkIsoScore" + detNamesConcat + "_VS_" + referenceListName + extraSuffix;

      auto func = [ = ](const Particle * part) -> double {

        StoreObjPtr<ParticleList> refPartList(referenceListName);
        if (!refPartList.isValid())
        {
          B2FATAL("Invalid Listname " << referenceListName << " given to minET2ETIsoScore!");
        }

        if (!part->hasExtraInfo(extraInfo))
        {
          return Const::doubleNaN;
        }
        auto scoreDet = part->getExtraInfo(extraInfo);
        if (std::isnan(scoreDet))
        {
          return Const::doubleNaN;
        }

        return scoreDet;

      };

      return func;
    }


    Manager::FunctionPtr particleExtTrkIsoScoreVarAsWeightedAvg(const std::vector<std::string>& arguments)
    {

      if (arguments.size() < 3) {
        B2ERROR("Wrong number of arguments (at least 3 required) for meta variable minET2ETIsoScoreAsWeightedAvg");
        return nullptr;
      }

      std::string referenceListName = arguments[0];
      bool useHighestProbMassForExt;
      try {
        useHighestProbMassForExt = static_cast<bool>(Belle2::convertString<int>(arguments[1]));
      } catch (std::invalid_argument& e) {
        B2ERROR("Second argument must be an integer flag.");
        return nullptr;
      }
      std::string extraSuffix = (useHighestProbMassForExt) ? "__useHighestProbMassForExt" : "";

      std::vector<std::string> detectorNames(arguments.begin() + 2, arguments.end());

      std::string detNamesConcat("");
      for (auto& detName : detectorNames) {
        boost::to_upper(detName);
        detNamesConcat += "_" + detName;
      }

      std::string extraInfo = "trkIsoScoreAsWeightedAvg" + detNamesConcat + "_VS_" + referenceListName + extraSuffix;

      auto func = [ = ](const Particle * part) -> double {

        StoreObjPtr<ParticleList> refPartList(referenceListName);
        if (!refPartList.isValid())
        {
          B2FATAL("Invalid Listname " << referenceListName << " given to minET2ETIsoScoreAsWeightedAvg!");
        }

        if (!part->hasExtraInfo(extraInfo))
        {
          return Const::doubleNaN;
        }
        auto scoreDet = part->getExtraInfo(extraInfo);
        if (std::isnan(scoreDet))
        {
          return Const::doubleNaN;
        }

        return scoreDet;

      };

      return func;
    }

    VARIABLE_GROUP("Kinematics");
    REGISTER_VARIABLE("p", particleP, "momentum magnitude\n\n", "GeV/c");
    REGISTER_VARIABLE("E", particleE, "energy\n\n", "GeV");

    REGISTER_VARIABLE("E_uncertainty", particleEUncertainty, R"DOC(
                      energy uncertainty (:math:`\sqrt{\sigma^2}`)

                      )DOC", "GeV");
    REGISTER_VARIABLE("ECLClusterE_uncertainty", particleClusterEUncertainty,
                      "energy uncertainty as given by the underlying ECL cluster\n\n", "GeV");
    REGISTER_VARIABLE("px", particlePx, "momentum component x\n\n", "GeV/c");
    REGISTER_VARIABLE("py", particlePy, "momentum component y\n\n", "GeV/c");
    REGISTER_VARIABLE("pz", particlePz, "momentum component z\n\n", "GeV/c");
    REGISTER_VARIABLE("pt", particlePt, "transverse momentum\n\n", "GeV/c");
    REGISTER_VARIABLE("xp", particleXp,
                      "scaled momentum: the momentum of the particle in the CMS as a fraction of its maximum available momentum in the collision");
    REGISTER_VARIABLE("pErr", particlePErr, "error of momentum magnitude\n\n", "GeV/c");
    REGISTER_VARIABLE("pxErr", particlePxErr, "error of momentum component x\n\n", "GeV/c");
    REGISTER_VARIABLE("pyErr", particlePyErr, "error of momentum component y\n\n", "GeV/c");
    REGISTER_VARIABLE("pzErr", particlePzErr, "error of momentum component z\n\n", "GeV/c");
    REGISTER_VARIABLE("ptErr", particlePtErr, "error of transverse momentum\n\n", "GeV/c");
    REGISTER_VARIABLE("momVertCovM(i,j)", covMatrixElement,
                      "returns the (i,j)-th element of the MomentumVertex Covariance Matrix (7x7).\n"
                      "Order of elements in the covariance matrix is: px, py, pz, E, x, y, z.\n\n", "GeV/c, GeV/c, GeV/c, GeV, cm, cm, cm");
    REGISTER_VARIABLE("momDevChi2", momentumDeviationChi2, R"DOC(
momentum deviation :math:`\chi^2` value calculated as :math:`\chi^2 = \sum_i (p_i - mc(p_i))^2/\sigma(p_i)^2`,
where :math:`\sum` runs over i = px, py, pz and :math:`mc(p_i)` is the mc truth value and :math:`\sigma(p_i)` is the estimated error of i-th component of momentum vector
)DOC");
    REGISTER_VARIABLE("theta", particleTheta, "polar angle\n\n", "rad");
    REGISTER_VARIABLE("thetaErr", particleThetaErr, "error of polar angle\n\n", "rad");
    REGISTER_VARIABLE("cosTheta", particleCosTheta, "momentum cosine of polar angle");
    REGISTER_VARIABLE("cosThetaErr", particleCosThetaErr, "error of momentum cosine of polar angle");
    REGISTER_VARIABLE("phi", particlePhi, "momentum azimuthal angle\n\n", "rad");
    REGISTER_VARIABLE("phiErr", particlePhiErr, "error of momentum azimuthal angle\n\n", "rad");
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
    REGISTER_VARIABLE("cosToThrustOfEvent", cosToThrustOfEvent,
                      "Returns the cosine of the angle between the particle and the thrust axis of the event, as calculate by the EventShapeCalculator module. buildEventShape() must be run before calling this variable");

    REGISTER_VARIABLE("ImpactXY"  , ImpactXY , "The impact parameter of the given particle in the xy plane\n\n", "cm");

    REGISTER_VARIABLE("M", particleMass, R"DOC(
The particle's mass.

Note that this is context-dependent variable and can take different values depending on the situation. This should be the "best"
value possible with the information provided.

- If this particle is track- or cluster- based, then this is the value of the mass hypothesis.
- If this particle is an MC particle then this is the mass of that particle.
- If this particle is composite, then *initially* this takes the value of the invariant mass of the daughters.
- If this particle is composite and a *mass or vertex fit* has been performed then this may be updated by the fit.

* You will see a difference between this mass and the :b2:var:`InvM`.

)DOC", "GeV/:math:`\\text{c}^2`");
    REGISTER_VARIABLE("dM", particleDMass, "mass minus nominal mass\n\n", "GeV/:math:`\\text{c}^2`");
    REGISTER_VARIABLE("Q", particleQ, "energy released in decay\n\n", "GeV");
    REGISTER_VARIABLE("dQ", particleDQ, ":b2:var:`Q` minus nominal energy released in decay\n\n", "GeV");
    REGISTER_VARIABLE("Mbc", particleMbc, "beam constrained mass\n\n", "GeV/:math:`\\text{c}^2`");
    REGISTER_VARIABLE("deltaE", particleDeltaE, "difference between :b2:var:`E` and half the center of mass energy\n\n", "GeV");
    REGISTER_VARIABLE("M2", particleMassSquared, "The particle's mass squared.\n\n", ":math:`[\\text{GeV}/\\text{c}^2]^2`");

    REGISTER_VARIABLE("InvM", particleInvariantMassFromDaughtersDisplaced,
                      "invariant mass (determined from particle's daughter 4-momentum vectors). If this particle is V0 or decays at rho > 5 mm, its daughter 4-momentum vectors at fitted vertex are taken.\n"
                      "If this particle has no daughters, defaults to :b2:var:`M`.\n\n", "GeV/:math:`\\text{c}^2`");
    REGISTER_VARIABLE("InvMLambda", particleInvariantMassLambda,
                      "Invariant mass (determined from particle's daughter 4-momentum vectors), assuming the first daughter is a pion and the second daughter is a proton.\n"
                      "If the particle has not 2 daughters, it returns just the mass value.\n\n", "GeV/:math:`\\text{c}^2`");

    REGISTER_VARIABLE("ErrM", particleInvariantMassError,
                      "uncertainty of invariant mass\n\n", "GeV/:math:`\\text{c}^2`");
    REGISTER_VARIABLE("SigM", particleInvariantMassSignificance,
                      "signed deviation of particle's invariant mass from its nominal mass in units of the uncertainty on the invariant mass (:b2:var:`dM`/:b2:var:`ErrM`)");

    REGISTER_VARIABLE("pxRecoil", recoilPx,
                      "component x of 3-momentum recoiling against given Particle\n\n", "GeV/c");
    REGISTER_VARIABLE("pyRecoil", recoilPy,
                      "component y of 3-momentum recoiling against given Particle\n\n", "GeV/c");
    REGISTER_VARIABLE("pzRecoil", recoilPz,
                      "component z of 3-momentum recoiling against given Particle\n\n", "GeV/c");

    REGISTER_VARIABLE("pRecoil", recoilMomentum,
                      "magnitude of 3 - momentum recoiling against given Particle\n\n", "GeV/c");
    REGISTER_VARIABLE("pRecoilTheta", recoilMomentumTheta,
                      "Polar angle of a particle's missing momentum\n\n", "rad");
    REGISTER_VARIABLE("pRecoilPhi", recoilMomentumPhi,
                      "Azimuthal angle of a particle's missing momentum\n\n", "rad");
    REGISTER_VARIABLE("eRecoil", recoilEnergy,
                      "energy recoiling against given Particle\n\n", "GeV");
    REGISTER_VARIABLE("mRecoil", recoilMass,
                      "Invariant mass of the system recoiling against given Particle\n\n", "GeV/:math:`\\text{c}^2`");
    REGISTER_VARIABLE("m2Recoil", recoilMassSquared,
                      "invariant mass squared of the system recoiling against given Particle\n\n", ":math:`[\\text{GeV}/\\text{c}^2]^2`");
    REGISTER_VARIABLE("m2RecoilSignalSide", m2RecoilSignalSide, R"DOC(
                       Squared recoil mass of the signal side which is calculated in the CMS frame under the assumption that the
                       signal and tag side are produced back to back and the tag side energy equals the beam energy. The variable
                       must be applied to the Upsilon and the tag side must be the first, the signal side the second daughter

                       )DOC", ":math:`[\\text{GeV}/\\text{c}^2]^2`");

    REGISTER_VARIABLE("b2bTheta", b2bTheta,
                      "Polar angle in the lab system that is back-to-back to the particle in the CMS. Useful for low multiplicity studies.\n\n", "rad");
    REGISTER_VARIABLE("b2bPhi", b2bPhi,
                      "Azimuthal angle in the lab system that is back-to-back to the particle in the CMS. Useful for low multiplicity studies.\n\n",
                      "rad");
    REGISTER_VARIABLE("b2bClusterTheta", b2bClusterTheta,
                      "Polar angle in the lab system that is back-to-back to the particle's associated ECLCluster in the CMS. Returns NAN if no cluster is found. Useful for low multiplicity studies.\n\n",
                      "rad");
    REGISTER_VARIABLE("b2bClusterPhi", b2bClusterPhi,
                      "Azimuthal angle in the lab system that is back-to-back to the particle's associated ECLCluster in the CMS. Returns NAN if no cluster is found. Useful for low multiplicity studies.\n\n",
                      "rad");
    REGISTER_VARIABLE("ArmenterosLongitudinalMomentumAsymmetry", ArmenterosLongitudinalMomentumAsymmetry,
                      "Longitudinal momentum asymmetry of V0's daughters.\n"
                      "The mother (V0) is required to have exactly two daughters");
    REGISTER_VARIABLE("ArmenterosDaughter1Qt", ArmenterosDaughter1Qt, R"DOC(
                       Transverse momentum of the first daughter with respect to the V0 mother.
                       The mother is required to have exactly two daughters

                       )DOC", "GeV/c");
    REGISTER_VARIABLE("ArmenterosDaughter2Qt", ArmenterosDaughter2Qt, R"DOC(
                       Transverse momentum of the second daughter with respect to the V0 mother.
                       The mother is required to have exactly two daughters

                       )DOC", "GeV/c");

    VARIABLE_GROUP("Miscellaneous");
    REGISTER_VARIABLE("nRemainingTracksInEvent",  nRemainingTracksInEvent,
                      "Number of tracks in the event - Number of tracks( = charged FSPs) of particle.");
    REGISTER_VARIABLE("trackMatchType", trackMatchType, R"DOC(

* -1 particle has no ECL cluster
*  0 particle has no associated track
*  1 there is a matched track called connected - region(CR) track match

                      )DOC");
    MAKE_DEPRECATED("trackMatchType", true, "light-2012-minos", R"DOC(
                     Use better variables like `trackNECLClusters`, `clusterTrackMatch`, and `nECLClusterTrackMatches`.)DOC");

    REGISTER_VARIABLE("decayTypeRecoil", recoilMCDecayType,
                      "type of the particle decay(no related mcparticle = -1, hadronic = 0, direct leptonic = 1, direct semileptonic = 2,"
                      "lower level leptonic = 3.");

    REGISTER_VARIABLE("printParticle", printParticle,
                      "For debugging, print Particle and daughter PDG codes, plus MC match. Returns 0.");
    REGISTER_VARIABLE("mcMomTransfer2", particleMCMomentumTransfer2,
                      "Return the true momentum transfer to lepton pair in a B(semi -) leptonic B meson decay.\n\n", "GeV/c");
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
    REGISTER_METAVARIABLE("minET2ETDist(detName, detLayer, referenceListName, useHighestProbMassForExt=1)", particleDistToClosestExtTrk,
                          R"DOC(Returns the distance :math:`d_{\mathrm{i}}` in [cm] between the particle and the nearest particle in the reference list at the given detector :math:`i`-th layer surface.
The definition is based on the track helices extrapolation.

* The first argument is the name of the detector to consider.
* The second argument is the detector layer on whose surface we search for the nearest neighbour.
* The third argument is the reference particle list name used to search for the nearest neighbour.
* The fourth (optional) argument is an integer ("boolean") flag: if 1 (the default, if nothing is set), it is assumed the extrapolation was done with the most probable mass hypothesis for the track fit;
  if 0, it is assumed the mass hypothesis matching the particle lists' PDG was used.

.. note::
    This variable requires to run the ``TrackIsoCalculator`` module first.
    Note that the choice of input parameters of this metafunction must correspond to the settings used to configure the module!
)DOC",
			  Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("minET2ETDistVar(detName, detLayer, referenceListName, variableName)", particleDistToClosestExtTrkVar,
			  R"DOC(Returns the value of the variable for the nearest neighbour to this particle as taken from the reference list at the given detector :math:`i`-th layer surface
, according to the distance definition of `minET2ETDist`.

* The first argument is the name of the detector to consider.
* The second argument is the detector layer on whose surface we search for the nearest neighbour.
* The third argument is the reference particle list name used to search for the nearest neighbour.
* The fourth argument is a variable name, e.g. `nCDCHits`.

.. note::
    This variable requires to run the ``TrackIsoCalculator`` module first.
    Note that the choice of input parameters of this metafunction must correspond to the settings used to configure the module!
)DOC",
			  Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("minET2ETIsoScore(referenceListName, useHighestProbMassForExt, detectorList)", particleExtTrkIsoScoreVar,
			  R"DOC(Returns a particle's isolation score :math:`s` defined as:

.. math::
   :nowrap:

   \begin{split}

     s &= \sum_{\mathrm{det}} 1 - \left(-w_{\mathrm{det}} \cdot \frac{\sum_{i}^{N_{\mathrm{det}}^{\mathrm{layers}}} H(i)}{N_{\mathrm{det}}^{\mathrm{layers}}}\right), \\

     H(i) &=
       \begin{cases}
         0 & d_{\mathrm{i}} > D_{\mathrm{det}}^{\mathrm{thresh}} \\
         1 & d_{\mathrm{i}} <= D_{\mathrm{det}}^{\mathrm{thresh}}, \\
       \end{cases}

   \end{split}

where :math:`d_{\mathrm{i}}` is the distance to the closest neighbour at the :math:`i`-th layer of the given detector (c.f., `minET2ETDist`), :math:`N_{\mathrm{det}}^{\mathrm{layers}}` is the
number of layers of the detector, :math:`D_{\mathrm{det}}^{\mathrm{thresh}}` is a threshold length related to the detector's granularity defined in the ``TrackIsoCalculator`` module,
and :math:`w_{\mathrm{det}}` are (negative) weights associated to the detector's impact on PID for this particle type, read from a CDB payload.

The score is normalised in [0, 1], where values closer to 1 indicates a well-isolated particle.

* The first argument is the reference particle list name used to search for the nearest neighbour.
* The second argument is an integer ("boolean") flag: if 1, it is assumed the extrapolation was done with the most probable mass hypothesis for the track fit;
  if 0, it is assumed the mass hypothesis matching the particle lists' PDG was used.
* The remaining arguments are a comma-separated list of detector names, which must correspond to the one given to the `TrackIsoCalculator` module.

.. note::
    The PID detector weights :math:`w_{\mathrm{det}}` are non-trivial only if ``excludePIDDetWeights=false`` in the ``TrackIsoCalculator`` module configuration.
    Otherwise :math:`w_{\mathrm{det}} = -1`.

.. note::
    This variable requires to run the `TrackIsoCalculator` module first.
    Note that the choice of input parameters of this metafunction must correspond to the settings used to configure the module!

)DOC",
			  Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("minET2ETIsoScoreAsWeightedAvg(referenceListName, useHighestProbMassForExt, detectorList)", particleExtTrkIsoScoreVarAsWeightedAvg,
			  R"DOC(Returns a particle's isolation score :math:`s` based on the weighted average:

.. math::

   s = 1 - \frac{\sum_{\mathrm{det}} \sum_{i}^{N_{\mathrm{det}}^{\mathrm{layers}}} w_{\mathrm{det}} \cdot \frac{D_{\mathrm{det}}^{\mathrm{thresh}}}{d_{\mathrm{i}}} }{ \sum_{\mathrm{det}} w_{\mathrm{det}}},

where :math:`d_{\mathrm{i}}` is the distance to the closest neighbour at the :math:`i`-th layer of the given detector (c.f., `minET2ETDist`), :math:`N_{\mathrm{det}}^{\mathrm{layers}}` is the
number of layers of the detector, :math:`D_{\mathrm{det}}^{\mathrm{thresh}}` is a threshold length related to the detector's granularity defined in the ``TrackIsoCalculator`` module,
and :math:`w_{\mathrm{det}}` are (negative) weights associated to the detector's impact on PID for this particle type, read from a CDB payload.

The score is normalised in [0, 1], where values closer to 1 indicates a well-isolated particle.

* The first argument is the reference particle list name used to search for the nearest neighbour.
* The second argument is an integer ("boolean") flag: if 1, it is assumed the extrapolation was done with the most probable mass hypothesis for the track fit;
  if 0, it is assumed the mass hypothesis matching the particle lists' PDG was used.
* The remaining arguments are a comma-separated list of detector names, which must correspond to the one given to the `TrackIsoCalculator` module.

.. note::
    The PID detector weights :math:`w_{\mathrm{det}}` are non-trivial only if ``excludePIDDetWeights=false`` in the ``TrackIsoCalculator`` module configuration.
    Otherwise :math:`\lvert w_{\mathrm{det}} \rvert = 1`.

.. note::
    This variable requires to run the `TrackIsoCalculator` module first.
    Note that the choice of input parameters of this metafunction must correspond to the settings used to configure the module!

)DOC",
			  Manager::VariableDataType::c_double);

  }
}
