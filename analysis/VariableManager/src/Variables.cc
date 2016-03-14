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

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>


// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <geometry/bfieldmap/BFieldMap.h>

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

    double particleCosTheta(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(part).CosTheta();
    }

    double particlePhi(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getMomentum(part).Phi();
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
      double m_B = 5.2794; //Only true for B^+ or B^0
      double p_B = std::sqrt(e_Beam * e_Beam - m_B * m_B);

      TLorentzVector p = T.rotateLabToCms() * part->get4Vector();
      double e_d = p.E();
      double m_d = p.M();
      double p_d = p.Rho();

      double theta_Bd = (2 * e_Beam * e_d - m_B * m_B - m_d * m_d) / (2 * p_B * p_d);
      return theta_Bd;
    }

    double VertexZDist(const Particle* part)
    {
      double z0_daughters[2] = { -99., -99.};
      const double alpha = 1.0 / (1.5 * TMath::C()) * 1E11;
      const std::vector<Particle*> daughters = part->getDaughters();
      for (unsigned i = 0; i <= 1; i++) {
        TLorentzVector dt;
        dt = daughters[i]-> get4Vector();
        double charge = daughters[i] -> getCharge();

        double x = dt.X(); double y = dt.Y(); double z = dt.Z();
        double px = dt.Px(); double py = dt.Py(); double pz = dt.Pz();

        // We find the perigee parameters by inverting this system of
        // equations and solving for the six variables d0, phi, omega, z0,
        // cotTheta, chi.

        const double ptinv = 1 / hypot(px, py);
        const double omega = charge * ptinv / alpha;
        const double cotTheta = ptinv * pz;

        const double cosphichi = charge * ptinv * px;  // cos(phi + chi)
        const double sinphichi = charge * ptinv * py;  // sin(phi + chi)

        // Helix center in the (x, y) plane:
        const double helX = x + charge * py * alpha;
        const double helY = y - charge * px * alpha;
        const double phi = atan2(helY, helX) + charge * M_PI / 2;
        const double sinchi = sinphichi * cos(phi) - cosphichi * sin(phi);
        const double chi = asin(sinchi);
        z0_daughters[i] = z + charge / omega * cotTheta * chi;
      }

      return abs(z0_daughters[1] - z0_daughters[0]);
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

    double particleDRho(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).Perp();
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

      TVectorF    jacobian(Particle::c_DimMomentum);
      jacobian[0] = -1.0 * part->getPx() / invMass;
      jacobian[1] = -1.0 * part->getPy() / invMass;
      jacobian[2] = -1.0 * part->getPz() / invMass;
      jacobian[3] =  1.0 * part->getEnergy() / invMass;

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

    // other ------------------------------------------------------------

    double particleMdstArrayIndex(const Particle* part)
    {
      return part->getMdstArrayIndex();
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
      s  << p->getPDGCode();
      const MCParticle* mcp = p->getRelated<MCParticle>();
      if (mcp) {
        s << " -> MC: " << mcp->getPDG() << ", mcErrors: " << MCMatching::getMCErrors(p, mcp);
        s << ", mc-index " << mcp->getIndex();
        s << ", mc-pdg " << mcp->getPDG();
      } else {
        s << " (no MC match)";
      }
      s << ", mdst-source " << p->getMdstSource();
      B2INFO(s.str())
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

      //abort early if PDG codes are different
      if (abs(mcparticle->getPDG()) != abs(part->getPDGCode()))
        return 0.0;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits, these are usually ok
      status &= (~MCMatching::c_MissFSR);
      status &= (~MCMatching::c_MissingResonance);
      //status &= (~MCMatching::c_DecayInFlight);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }

    double genMotherPDG(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr) return 0.0;

      const MCParticle* mcmother = mcparticle->getMother();
      if (mcmother == nullptr) return 0.0;

      int m_pdg = mcmother->getPDG();
      return m_pdg;
    }

    double genMotherIndex(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (!mcparticle) return -1.0;

      const MCParticle* mcmother = mcparticle->getMother();
      if (!mcmother) return -2.0;

      double m_ID = mcmother->getArrayIndex();
      return m_ID;
    }

    double genParticleIndex(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (!mcparticle) return -1.0;

      double m_ID = mcparticle->getArrayIndex();
      return m_ID;
    }

    double isSignalAcceptMissingNeutrino(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      //abort early if PDG codes are different
      if (abs(mcparticle->getPDG()) != abs(part->getPDGCode()))
        return 0.0;

      int status = MCMatching::getMCErrors(part, mcparticle);
      //remove the following bits, these are usually ok
      status &= (~MCMatching::c_MissFSR);
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
      RelationVector<MCParticle> mcRelations = particle->getRelationsTo<MCParticle>();
      return double(mcRelations.size());
    }

    double particleMCMatchWeight(const Particle* particle)
    {
      std::pair<MCParticle*, double> relation = particle->getRelatedToWithWeight<MCParticle>();

      if (relation.first) {
        return relation.second;
      } else {
        return 0.0;
      }
    }

    // RestOfEvent related --------------------------------------------------

    double nRemainingTracksInRestOfEvent(const Particle* particle)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (not roe.isValid())
        return 0.0;
      int roe_tracks = roe->getTracks().size();
      int par_tracks = 0;
      const auto& daughters = particle->getFinalStateDaughters();
      for (const auto& daughter : daughters) {
        int pdg = abs(daughter->getPDGCode());
        if (pdg == 11 or pdg == 13 or pdg == 211 or pdg == 321 or pdg == 2212)
          par_tracks++;
      }
      return roe_tracks - par_tracks;
    }

    double isInRestOfEvent(const Particle* particle)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (not roe.isValid())
        return 1.0;
      const auto& tracks = roe->getTracks();
      if (std::find(tracks.begin(), tracks.end(), particle->getTrack()) != tracks.end()) {
        return 1.0;
      }
      const auto& klm = roe->getKLMClusters();
      if (std::find(klm.begin(), klm.end(), particle->getKLMCluster()) != klm.end()) {
        return 1.0;
      }
      const auto& ecl = roe->getECLClusters();
      if (std::find(ecl.begin(), ecl.end(), particle->getECLCluster()) != ecl.end()) {
        return 1.0;
      }
      return 0;
    }

    double nROETracks(const Particle* particle)
    {
      double result = -1.0;

      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (roe)
        result = roe->getNTracks();

      return result;
    }

    double nROEECLClusters(const Particle* particle)
    {
      double result = -1.0;

      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (roe)
        result = roe->getNECLClusters();

      return result;
    }

    double nROEKLMClusters(const Particle* particle)
    {
      double result = -1.0;

      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (roe)
        result = roe->getNKLMClusters();
      return result;
    }

    double pionVeto(const Particle* particle)
    {
      double pion0Mass = 0.135;           // neutral pion mass from PDG
      double deltaE = 0.03;               // mass range around pion0Mass that will be accepted

      StoreObjPtr<ParticleList> PhotonList("gamma");

      const Particle* sig_Photon = particle->getDaughter(1)->getDaughter(0);
      TLorentzVector vec = sig_Photon->get4Vector();

      for (unsigned int i = 0; i < PhotonList->getListSize(); i++) {
        Particle* p_Photon = PhotonList->getParticle(i);
        if ((p_Photon->getEnergy() >= 0.1) && (p_Photon->getMdstArrayIndex() != sig_Photon->getMdstArrayIndex())) {
          double tempCombination = (p_Photon->get4Vector() + vec).M();
          if (abs(tempCombination - pion0Mass) <= deltaE) {
            return 1;
          }
        }
      }

      return 0;
    }

    // TDCPV related ---------------------------------------------------------

    double particleTagVx(const Particle* particle)
    {
      double result = -1111.0;

      Vertex* vert = particle->getRelatedTo<Vertex>();

      if (vert)
        result = vert->getTagVertex().X();

      return result;
    }

    double particleTagVy(const Particle* particle)
    {
      double result = -1111.0;

      Vertex* vert = particle->getRelatedTo<Vertex>();

      if (vert)
        result = vert->getTagVertex().Y();

      return result;
    }

    double particleTagVz(const Particle* particle)
    {
      double result = -1111.0;

      Vertex* vert = particle->getRelatedTo<Vertex>();

      if (vert)
        result = vert->getTagVertex().Z();

      return result;
    }

    double particleDeltaT(const Particle* particle)
    {
      double result = -1111.0;

      Vertex* vert = particle->getRelatedTo<Vertex>();

      if (vert)
        result = vert->getDeltaT();

      return result;
    }

    double particleMCDeltaT(const Particle* particle)
    {
      double result = -1111.0;

      Vertex* vert = particle->getRelatedTo<Vertex>();

      if (vert)
        result = vert->getMCDeltaT();

      return result;
    }

    double particleDeltaZ(const Particle* particle)
    {
      double result = -1111.0;

      Vertex* vert = particle->getRelatedTo<Vertex>();

      if (vert)
        result = particle->getZ() - vert->getTagVertex().Z();

      return result;
    }

    double particleDeltaB(const Particle* particle)
    {
      double result = -1111.0;

      Vertex* vert = particle->getRelatedTo<Vertex>();

      if (vert) {
        PCmsLabTransform T;
        TVector3 boost = T.getBoostVector().BoostVector();
        double bg = boost.Mag() / TMath::Sqrt(1 - boost.Mag2());
        double c = Const::speedOfLight / 1000.; // cm ps-1
        result = vert->getDeltaT() * bg * c;
      }
      return result;
    }

    // Recoil Kinematics related ---------------------------------------------

    double recoilMomentum(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN  = T.getBoostVector();

      return (pIN - particle->get4Vector()).P();
    }

    double recoilEnergy(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN  = T.getBoostVector();

      return (pIN - particle->get4Vector()).E();
    }

    double recoilMass(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN  = T.getBoostVector();

      return (pIN - particle->get4Vector()).M();
    }

    double recoilMassSquared(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN  = T.getBoostVector();

      return (pIN - particle->get4Vector()).M2();
    }

    // Extra energy --------------------------------------------------------

    double extraEnergy(const Particle* particle)
    {
      double result = -1.0;

      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();
      if (!roe)
        return result;

      const std::vector<ECLCluster*> remainECLClusters = roe->getECLClusters();
      result = 0.0;
      for (unsigned i = 0; i < remainECLClusters.size(); i++)
        result += remainECLClusters[i]->getEnergy();

      return result;
    }

    double extraEnergyFromGoodGamma(const Particle* particle)
    {
      double result = -1.0;

      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();
      if (!roe)
        return result;

      const std::vector<ECLCluster*> remainECLClusters = roe->getECLClusters();
      result = 0.0;
      for (unsigned i = 0; i < remainECLClusters.size(); i++) {
        Particle gamma(remainECLClusters[i]);
        if (goodGamma(&gamma) > 0)
          result += remainECLClusters[i]->getEnergy();
      }

      return result;
    }

    // ECLCluster related variables -----------------------------------------

    double eclClusterDetectionRegion(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* shower = particle->getECLCluster();
      if (shower) {
        /// TODO: check if ECLCluster will provide this on its own
        float theta = shower->getMomentum().Theta();
        if (theta < 0.555) {
          result = 1.0;
        } else if (theta < 2.26) {
          result = 2.0;
        } else {
          result = 3.0;
        }
      }

      return result;
    }

    bool isGoodGamma(int region, double energy, bool calibrated)
    {
      bool goodGammaRegion1, goodGammaRegion2, goodGammaRegion3;
      if (!calibrated) {
        goodGammaRegion1 = region == 1 && energy > 0.140;
        goodGammaRegion2 = region == 2 && energy > 0.130;
        goodGammaRegion3 = region == 3 && energy > 0.200;
      } else {
        goodGammaRegion1 = region == 1 && energy > 0.100;
        goodGammaRegion2 = region == 2 && energy > 0.090;
        goodGammaRegion3 = region == 3 && energy > 0.160;
      }

      return goodGammaRegion1 || goodGammaRegion2 || goodGammaRegion3;
    }

    double goodGammaUncalibrated(const Particle* particle)
    {
      double energy = particle->getEnergy();
      int    region = eclClusterDetectionRegion(particle);

      return (double)isGoodGamma(region, energy, false);
    }

    double goodGamma(const Particle* particle)
    {
      double energy = particle->getEnergy();
      int    region = eclClusterDetectionRegion(particle);

      return (double)isGoodGamma(region, energy, true);
    }

    double eclClusterUncorrectedE(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* shower = particle->getECLCluster();
      if (shower) {
        result = shower->getEnedepSum();
      }
      return result;
    }

    double eclClusterHighestE(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* shower = particle->getECLCluster();
      if (shower) {
        result = shower->getHighestE();
      }
      return result;
    }

    double eclClusterTiming(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* shower = particle->getECLCluster();
      if (shower) {
        result = shower->getTiming();
      }
      return result;
    }

    double eclClusterTheta(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* shower = particle->getECLCluster();
      if (shower) {
        result = shower->getTheta();
      }
      return result;
    }

    double eclClusterPhi(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* shower = particle->getECLCluster();
      if (shower) {
        result = shower->getPhi();
      }
      return result;
    }

    double eclClusterR(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* shower = particle->getECLCluster();
      if (shower) {
        result = shower->getR();
      }
      return result;
    }



    double eclClusterE9E25(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* shower = particle->getECLCluster();
      if (shower) {
        result = shower->getE9oE25();
      }
      return result;
    }

    double eclClusterNHits(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* shower = particle->getECLCluster();
      if (shower) {
        result = shower->getNofCrystals();
      }
      return result;
    }

    double eclClusterTrackMatched(const Particle* particle)
    {
      double result = 0.0;

      const ECLCluster* shower = particle->getECLCluster();
      if (shower) {
        const Track* track = shower->getRelated<Track>();

        if (track)
          result = 1.0;
      }
      return result;

    }


    // Event ------------------------------------------------
    double eventType(const Particle*)
    {
      StoreArray<MCParticle> mcparticles;
      return (mcparticles.getEntries()) > 0 ? 0 : 1;
    }

    double nTracks(const Particle*)
    {
      StoreArray<Track> tracks;
      return tracks.getEntries();
    }

    double nECLClusters(const Particle*)
    {
      StoreArray<ECLCluster> eclClusters;
      return eclClusters.getEntries();
    }

    double nKLMClusters(const Particle*)
    {
      StoreArray<KLMCluster> klmClusters;
      return klmClusters.getEntries();
    }

    double ECLEnergy(const Particle*)
    {
      StoreArray<ECLCluster> eclClusters;
      double result = 0;
      for (int i = 0; i < eclClusters.getEntries(); ++i) {
        result += eclClusters[i]->getEnergy();
      }
      return result;
    }

    double KLMEnergy(const Particle*)
    {
      StoreArray<KLMCluster> klmClusters;
      double result = 0;
      for (int i = 0; i < klmClusters.getEntries(); ++i) {
        result += klmClusters[i]->getMomentum().Energy();
      }
      return result;
    }

    double isContinuumEvent(const Particle*)
    {
      StoreArray<MCParticle> mcParticles;
      for (int i = 0; i < mcParticles.getEntries(); ++i) {
        if (mcParticles[i]->getPDG() == 300553)
          return 0.0;
      }
      return 1.0;
    }

    // Continuum Suppression related ------------------------

    double thrustBm(const Particle* particle)
    {
      double result = -1.0;

      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return result;

      result = qq->getThrustBm();

      return result;
    }

    double thrustOm(const Particle* particle)
    {
      double result = -1.0;

      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return result;

      result = qq->getThrustOm();

      return result;
    }

    double cosTBTO(const Particle* particle)
    {
      double result = -1.0;

      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return result;

      result = qq->getCosTBTO();

      return result;
    }

    double cosTBz(const Particle* particle)
    {
      double result = -1.0;

      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return result;

      result = qq->getCosTBz();

      return result;
    }

    double R2(const Particle* particle)
    {
      double result = -1.0;

      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return result;

      result = qq->getR2();

      return result;
    }

    double False(const Particle*)
    {
      return 0;
    }

    VARIABLE_GROUP("Kinematics");
    REGISTER_VARIABLE("p", particleP, "momentum magnitude");
    REGISTER_VARIABLE("E", particleE, "energy");
    REGISTER_VARIABLE("px", particlePx, "momentum component x");
    REGISTER_VARIABLE("py", particlePy, "momentum component y");
    REGISTER_VARIABLE("pz", particlePz, "momentum component z");
    REGISTER_VARIABLE("pt", particlePt, "transverse momentum");
    REGISTER_VARIABLE("cosTheta", particleCosTheta, "momentum cosine of polar angle");
    REGISTER_VARIABLE("phi", particlePhi, "momentum azimuthal angle in degrees");

    REGISTER_VARIABLE("cosThetaBetweenParticleAndTrueB", cosThetaBetweenParticleAndTrueB,
                      "cosine of angle between momentum the particle and a true B particle. Is somewhere between -1 and 1 if only a massless particle like a neutrino is missing in the reconstruction.");
    REGISTER_VARIABLE("cosAngleBetweenMomentumAndVertexVector", cosAngleBetweenMomentumAndVertexVector,
                      "cosine of angle between momentum and vertex vector (vector connecting ip and fitted vertex) of this particle");
    REGISTER_VARIABLE("VertexZDist"      , VertexZDist    , "Z-distance of two daughter tracks at vertex point");

    REGISTER_VARIABLE("distance", particleDistance, "3D distance relative to interaction point");
    REGISTER_VARIABLE("significanceOfDistance", particleDistanceSignificance,
                      "significance of distance relative to interaction point (-1 in case of numerical problems)");
    REGISTER_VARIABLE("dx", particleDX, "x in respect to IP");
    REGISTER_VARIABLE("dy", particleDY, "y in respect to IP");
    REGISTER_VARIABLE("dz", particleDZ, "z in respect to IP");
    REGISTER_VARIABLE("dr", particleDRho, "transverse distance in respect to IP");

    REGISTER_VARIABLE("M", particleMass, "invariant mass (determined from particle's 4-momentum vector)");
    REGISTER_VARIABLE("dM", particleDMass, "mass minus nominal mass");
    REGISTER_VARIABLE("Q", particleQ, "released energy in decay");
    REGISTER_VARIABLE("dQ", particleDQ, "released energy in decay minus nominal one");
    REGISTER_VARIABLE("Mbc", particleMbc, "beam constrained mass");
    REGISTER_VARIABLE("deltaE", particleDeltaE, "energy difference");

    REGISTER_VARIABLE("InvM", particleInvariantMass, "invariant mass (determined from particle's daughter 4-momentum vectors)");
    REGISTER_VARIABLE("InvMLambda", particleInvariantMassLambda,
                      "invariant mass (determined from particle's daughter 4-momentum vectors)");

    REGISTER_VARIABLE("ErrM", particleInvariantMassError,
                      "uncertainty of invariant mass (determined from particle's daughter 4-momentum vectors)");
    REGISTER_VARIABLE("SigM", particleInvariantMassSignificance, "signed deviation of particle's invariant mass from its nominal mass");
    REGISTER_VARIABLE("SigMBF", particleInvariantMassBeforeFitSignificance,
                      "signed deviation of particle's invariant mass (determined from particle's daughter 4-momentum vectors) from its nominal mass");

    VARIABLE_GROUP("MC Matching");
    REGISTER_VARIABLE("isSignal", isSignal,               "1.0 if Particle is correctly reconstructed (SIGNAL), 0.0 otherwise");
    REGISTER_VARIABLE("genMotherPDG", genMotherPDG,               "Check the PDG code of a particles MC mother particle");
    REGISTER_VARIABLE("genMotherID", genMotherIndex,               "Check the array index of a particle's generated mother");
    REGISTER_VARIABLE("genParticleID", genParticleIndex,               "Check the array index of a particle's related MCParticle");
    REGISTER_VARIABLE("isSignalAcceptMissingNeutrino", isSignalAcceptMissingNeutrino,
                      "same as isSignal, but also accept missing neutrino");
    REGISTER_VARIABLE("mcPDG",    particleMCMatchPDGCode,
                      "The PDG code of matched MCParticle, 0 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("mcErrors", particleMCErrors,
                      "The bit pattern indicating the quality of MC match (see MCMatching::MCErrorFlags)");
    REGISTER_VARIABLE("mcMatchWeight", particleMCMatchWeight,
                      "The weight of the Particle -> MCParticle relation (only for the first Relation = largest weight).");
    REGISTER_VARIABLE("nMCMatches", particleNumberOfMCMatch, "The number of relations of this Particle to MCParticle.");

    REGISTER_VARIABLE("mcVirtual", particleMCVirtualParticle,
                      "Returns 1 if Particle is related to virtual MCParticle, 0 if Particle is related to non-virtual MCParticle, -1 if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcInitial", particleMCInitialParticle,
                      "Returns 1 if Particle is related to initial MCParticle, 0 if Particle is related to non-initial MCParticle, -1 if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcISR", particleMCISRParticle,
                      "Returns 1 if Particle is related to ISR MCParticle, 0 if Particle is related to non-ISR MCParticle, -1 if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcFSR", particleMCFSRParticle,
                      "Returns 1 if Particle is related to FSR MCParticle, 0 if Particle is related to non-FSR MCParticle, -1 if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("mcPhotos", particleMCPhotosParticle,
                      "Returns 1 if Particle is related to Photos MCParticle, 0 if Particle is related to non-Photos MCParticle, -1 if Particle is not related to MCParticle.")

    VARIABLE_GROUP("Event");
    REGISTER_VARIABLE("EventType", eventType, "EventType (0 MC, 1 Data)");
    REGISTER_VARIABLE("isContinuumEvent",  isContinuumEvent,  "[Eventbased] true if event doesn't contain an Y(4S)");
    REGISTER_VARIABLE("nTracks",  nTracks,  "[Eventbased] number of tracks in the event");
    REGISTER_VARIABLE("nECLClusters", nECLClusters, "[Eventbased] number of ECL in the event");
    REGISTER_VARIABLE("nKLMClusters", nKLMClusters, "[Eventbased] number of KLM in the event");
    REGISTER_VARIABLE("ECLEnergy", ECLEnergy, "[Eventbased] total energy in ECL in the event");
    REGISTER_VARIABLE("KLMEnergy", KLMEnergy, "[Eventbased] total energy in KLM in the event");

    VARIABLE_GROUP("Rest Of Event");
    REGISTER_VARIABLE("nROETracks",  nROETracks,  "number of remaining tracks as given by the related RestOfEvent object");
    REGISTER_VARIABLE("nROEECLClusters", nROEECLClusters,
                      "number of remaining ECL clusters as given by the related RestOfEvent object");
    REGISTER_VARIABLE("nROEKLMClusters", nROEKLMClusters,
                      "number of remaining KLM clusters as given by the related RestOfEvent object");
    REGISTER_VARIABLE("nRemainingTracksInRestOfEvent", nRemainingTracksInRestOfEvent,
                      "Returns number of tracks in ROE - number of tracks of given particle");

    REGISTER_VARIABLE("pionVeto", pionVeto, "Returns the Flag 1 if a combination of photons has the invariant mass of a neutral pion");

    VARIABLE_GROUP("TDCPV");
    REGISTER_VARIABLE("TagVx", particleTagVx, "Tag vertex X");
    REGISTER_VARIABLE("TagVy", particleTagVy, "Tag vertex Y");
    REGISTER_VARIABLE("TagVz", particleTagVz, "Tag vertex Z");
    REGISTER_VARIABLE("DeltaT", particleDeltaT, "Delta T (Brec - Btag) in ps");
    REGISTER_VARIABLE("MCDeltaT", particleMCDeltaT, "Generated Delta T (Brec - Btag) in ps");
    REGISTER_VARIABLE("DeltaZ", particleDeltaZ, "Z(Brec) - Z(Btag)");
    REGISTER_VARIABLE("DeltaB", particleDeltaB, "Boost direction: Brec - Btag");

    VARIABLE_GROUP("Miscellaneous");
    REGISTER_VARIABLE("chiProb", particlePvalue, "chi^2 probability of the fit");
    REGISTER_VARIABLE("nDaughters", particleNDaughters, "number of daughter particles");
    REGISTER_VARIABLE("flavor", particleFlavorType, "flavor type of decay (0=unflavored, 1=flavored)");
    REGISTER_VARIABLE("charge", particleCharge, "charge of particle");
    REGISTER_VARIABLE("mdstIndex", particleMdstArrayIndex,
                      "StoreArray index (0-based) of the MDST object from which the Particle was created");

    REGISTER_VARIABLE("pRecoil",  recoilMomentum,    "magnitude of 3-momentum recoiling against given Particle");
    REGISTER_VARIABLE("eRecoil",  recoilEnergy,   "energy recoiling against given Particle");
    REGISTER_VARIABLE("mRecoil",  recoilMass,        "invariant mass of the system recoiling against given Particle");
    REGISTER_VARIABLE("m2Recoil", recoilMassSquared, "invariant mass squared of the system recoiling against given Particle");

    REGISTER_VARIABLE("isInRestOfEvent", isInRestOfEvent,
                      "1.0 of track, cluster of given particle is found in rest of event. 0 otherwise.");

    REGISTER_VARIABLE("eextra", extraEnergy, "extra energy in the calorimeter that is not associated to the given Particle");

    REGISTER_VARIABLE("printParticle", printParticle,
                      "For debugging, print Particle and daughter PDG codes, plus MC match. Returns 0.");
    REGISTER_VARIABLE("mcSecPhysProc", mcParticleSecondaryPhysicsProcess,
                      "Returns the secondary physics process flag.");
    REGISTER_VARIABLE("mcParticleStatus", mcParticleStatus,
                      "Returns status bits of related MCParticle or -1 if MCParticle relation is not set.");
    REGISTER_VARIABLE("mcPrimary", particleMCPrimaryParticle,
                      "Returns 1 if Particle is related to primary MCParticle, 0 if Particle is related to non-primary MCParticle, -1 if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("False", False, "returns always 0, used for testing and debugging.");

    VARIABLE_GROUP("ECL Cluster related");
    REGISTER_VARIABLE("goodGamma",         goodGamma, "1.0 if photon candidate passes good photon selection criteria");
    REGISTER_VARIABLE("goodGammaUnCal",    goodGammaUncalibrated,
                      "1.0 if photon candidate passes good photon selection criteria (to be used if photon's energy is not calibrated)");
    REGISTER_VARIABLE("clusterReg",        eclClusterDetectionRegion,
                      "detection region in the ECL [1 - forward, 2 - barrel, 3 - backward]");
    REGISTER_VARIABLE("clusterE9E25",      eclClusterE9E25,           "ratio of energies in inner 3x3 and 5x5 cells");
    REGISTER_VARIABLE("clusterTiming",     eclClusterTiming,           "timing");
    REGISTER_VARIABLE("clusterNHits",      eclClusterNHits,           "number of hits associated to this cluster");
    REGISTER_VARIABLE("clusterTrackMatch", eclClusterTrackMatched,    "number of charged track matched to this cluster");

    VARIABLE_GROUP("Continuum Suppression");
    REGISTER_VARIABLE("cosTBTO"  , cosTBTO , "cosine of angle between thrust axis of B and thrust axis of ROE");
    REGISTER_VARIABLE("cosTBz"   , cosTBz  , "cosine of angle between thrust axis of B and z-axis");
    REGISTER_VARIABLE("thrustBm" , thrustBm, "magnitude of the B thrust axis");
    REGISTER_VARIABLE("thrustOm" , thrustOm, "magnitude of the ROE thrust axis");
    REGISTER_VARIABLE("R2"       , R2      , "reduced Fox-Wolfram moment R2");
  }
}
