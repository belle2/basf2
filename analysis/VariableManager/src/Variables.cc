/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/Variables.h>
#include <analysis/utility/PCmsLabTransform.h>

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

    // momentum (lab) -------------------------------------------

    double particleP(const Particle* part)
    {
      return part->getP();
    }

    double particleE(const Particle* part)
    {
      return part->getEnergy();
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
      return part->get4Vector().CosTheta();
    }

    double particlePhi(const Particle* part)
    {
      return part->get4Vector().Phi();
    }

    // momentum (CMS) -----------------------------------------------

    double particleP_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();

      return vec.P();
    }

    double particleE_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.E();
    }

    double particlePx_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Px();
    }

    double particlePy_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Py();
    }

    double particlePz_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Pz();
    }

    double particlePt_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Pt();
    }

    double particleCosTheta_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.CosTheta();
    }

    double particlePhi_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Phi();
    }

    double cosAngleBetweenMomentumAndVertexVector(const Particle* part)
    {
      return std::cos(part->getVertex().Angle(part->getMomentum()));
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

    double particleDistance(const Particle* part)
    {
      return part->getVertex().Mag();
    }

    double particleDistanceSignificance(const Particle* part)
    {
      // significance is defined as s = r/sigma_r, therefore:
      // s &= \frac{r}{\sqrt{ \sum_{ij} \frac{\partial r}{x_i} V_{ij} \frac{\partial r}{x_j}}}
      //   &= \frac{r^2}{\sqrt{\vec{x}V\vec{x}}}
      // where:
      // r &= \sqrt{\vec{x}*\vec{x}}
      // and V_{ij} is the covariance matrix
      const auto& vertex = part->getVertex();
      auto denominator = vertex * (part->getVertexErrorMatrix() * vertex);
      if (denominator <= 0)
        return -1;
      return vertex.Mag2() / sqrt(denominator);
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
      for (const auto * daughter : p->getDaughters()) {
        printParticleInternal(daughter, depth + 1);
      }
    }

    double printParticle(const Particle* p)
    {
      printParticleInternal(p, 0);
      return 0.0;
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

    double particleAbsMCMatchPDGCode(const Particle* part)
    {
      return std::abs(particleMCMatchPDGCode(part));
    }

    double particleMCErrors(const Particle* part)
    {
      return MCMatching::getMCErrors(part);
    }

    // Flavour tagging variables

    double isMajorityInRestOfEventFromB0(const Particle*)
    {
      int vote = 0;
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      for (auto & track : roe->getTracks()) {
        const MCParticle* mcParticle = track->getRelated<MCParticle>();
        while (mcParticle != nullptr) {
          if (mcParticle->getPDG() == 511) {
            vote++;
            break;
          }
          if (mcParticle->getPDG() == -511) {
            vote--;
            break;
          }
          mcParticle = mcParticle->getMother();
        }
      }
      return vote > 0;
    }

    double isMajorityInRestOfEventFromB0bar(const Particle*)
    {
      int vote = 0;
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      for (auto & track : roe->getTracks()) {
        const MCParticle* mcParticle = track->getRelated<MCParticle>();
        while (mcParticle != nullptr) {
          if (mcParticle->getPDG() == 511) {
            vote++;
          }
          if (mcParticle->getPDG() == -511) {
            vote--;
          }
          mcParticle = mcParticle->getMother();
        }
      }
      return vote < 0;
    }

    double isRestOfEventOfB0(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      Particle* part = roe->getRelated<Particle>();
      const MCParticle* mcParticle = part->getRelated<MCParticle>();
      if (mcParticle == nullptr) {return -999.0;} //if there is no mcparticle (e.g. not in training modus)
      else if (mcParticle->getPDG() == 511) {
        return 1.0;
      }
      return 0.0;
    }

    double isRestOfEventOfB0bar(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      Particle* part = roe->getRelated<Particle>();
      const MCParticle* mcParticle = part->getRelated<MCParticle>();
      if (mcParticle == nullptr) {return -999.0;} //if there is no mcparticle (e.g. not in training modus)
      else if (mcParticle->getPDG() == -511) {
        return 1.0;
      }
      return 0.0;
    }

    double isRestOfEventEmpty(const Particle* part)
    {
      const RestOfEvent* roe = part->getRelatedTo<RestOfEvent>();
      float ObjectsInROE = 0; //Flavor of B
      if (roe-> getNTracks() != 0) {
        ObjectsInROE++;
      } else if (roe-> getNECLClusters() != 0) {
        ObjectsInROE++;
      } else if (roe-> getNKLMClusters() != 0) {
        ObjectsInROE++;
      }
      if (ObjectsInROE > 0) {
        return ObjectsInROE;
      } else return -2;
    }

    double isRestOfEventB0Flavor(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      float q_MC = 0; //Flavor of B
      if (roe-> getNTracks() != 0) {
        for (auto & track : roe->getTracks()) {
          const MCParticle* mcParticle = track->getRelated<MCParticle>();
          while (mcParticle != nullptr) {
            if (mcParticle->getPDG() == 511) {
              q_MC++;
              break;
            }
            if (mcParticle->getPDG() == -511) {
              q_MC--;
              break;
            }
            mcParticle = mcParticle->getMother();
          }
        }
      } else if (roe-> getNECLClusters() != 0) {
        for (auto & cluster : roe-> getECLClusters()) {
          const MCParticle* mcParticle = cluster->getRelated<MCParticle>();
          while (mcParticle != nullptr) {
            if (mcParticle->getPDG() == 511) {
              q_MC++;
              break;
            }
            if (mcParticle->getPDG() == -511) {
              q_MC--;
              break;
            }
            mcParticle = mcParticle->getMother();
          }
        }
      } else if (roe-> getNKLMClusters() != 0) {
        for (auto & klmcluster : roe-> getKLMClusters()) {
          const MCParticle* mcParticle = klmcluster->getRelated<MCParticle>();
          while (mcParticle != nullptr) {
            if (mcParticle->getPDG() == 511) {
              q_MC++;
              break;
            }
            if (mcParticle->getPDG() == -511) {
              q_MC--;
              break;
            }
            mcParticle = mcParticle->getMother();
          }
        }
      }
      if (q_MC > 0) {
        return 1;
      } else if (q_MC < 0) {
        return -1;
      } else return -2;
    }


    double isRestOfEventB0Flavor_Norm(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      float q_MC = 0; //Flavor of B
      if (roe-> getNTracks() != 0) {
        for (auto & track : roe->getTracks()) {
          const MCParticle* mcParticle = track->getRelated<MCParticle>();
          while (mcParticle != nullptr) {
            if (mcParticle->getPDG() == 511) {
              q_MC++;
              break;
            }
            if (mcParticle->getPDG() == -511) {
              q_MC--;
              break;
            }
            mcParticle = mcParticle->getMother();
          }
        }
      } else if (roe-> getNECLClusters() != 0) {
        for (auto & cluster : roe-> getECLClusters()) {
          const MCParticle* mcParticle = cluster->getRelated<MCParticle>();
          while (mcParticle != nullptr) {
            if (mcParticle->getPDG() == 511) {
              q_MC++;
              break;
            }
            if (mcParticle->getPDG() == -511) {
              q_MC--;
              break;
            }
            mcParticle = mcParticle->getMother();
          }
        }
      } else if (roe-> getNKLMClusters() != 0) {
        for (auto & klmcluster : roe-> getKLMClusters()) {
          const MCParticle* mcParticle = klmcluster->getRelated<MCParticle>();
          while (mcParticle != nullptr) {
            if (mcParticle->getPDG() == 511) {
              q_MC++;
              break;
            }
            if (mcParticle->getPDG() == -511) {
              q_MC--;
              break;
            }
            mcParticle = mcParticle->getMother();
          }
        }
      }
      if (q_MC > 0) {
        return 1;
      } else if (q_MC < 0) {
        return 0;
      } else return -2;//gRandom->Uniform(0, 1);
    }

    double p_miss(const Particle*)
    {
      TLorentzVector trackiCMSVec;
      TLorentzVector roeCMSVec;
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      const auto& tracks = roe->getTracks();
      for (unsigned int i = 0; i < tracks.size(); ++i) {
        const PIDLikelihood* trackiPidLikelihood = tracks[i]->getRelated<PIDLikelihood>();
        const Const::ChargedStable trackiChargedStable = trackiPidLikelihood->getMostLikely();
        double trackiMassHypothesis = trackiChargedStable.getMass();
        const TrackFitResult* tracki = tracks[i]->getTrackFitResult(trackiChargedStable);
        if (tracki == nullptr) continue;
        double energy = sqrt(trackiMassHypothesis * trackiMassHypothesis + (tracki->getMomentum()).Dot(tracki->getMomentum()));
        TLorentzVector trackiVec(tracki->getMomentum(), energy);
        trackiCMSVec = PCmsLabTransform::labToCms(trackiVec);
        roeCMSVec += trackiCMSVec;
      }
      double missMom = -roeCMSVec.P();
      return missMom ;
    }

    double NumberOfKShortinRemainingROEKaon(const Particle* particle)
    {
      StoreObjPtr<ParticleList> KShorts("K_S0:ROEKaon");
      int flag = 0;
      if (KShorts->getListSize() == 0) {return flag;}
      for (unsigned int i = 0; i < KShorts->getListSize(); i++) {
        if (particle->overlapsWith(KShorts->getParticle(i))) {continue;}
        flag++;
      }
      return flag;
    }

    double NumberOfKShortinRemainingROELambda(const Particle* particle)
    {
      StoreObjPtr<ParticleList> KShorts("K_S0:ROELambda");
      int flag = 0;
      if (KShorts->getListSize() == 0) {return flag;}
      for (unsigned int i = 0; i < KShorts->getListSize(); i++) {
        if (particle->overlapsWith(KShorts->getParticle(i))) {continue;}
        flag++;
      }
      return flag;
    }

    double lambdaFlavor(const Particle* particle)
    {
      if (particle->getPDGCode() == 3122) return 1.0; //Lambda0
      else if (particle->getPDGCode() == -3122) return -1.0; //Anti-Lambda0
      else return 0.0;
    }

    double lambdaZError(const Particle* particle)
    {
      //This is a simplisitc hack. But I see no other way to get that information.
      //Should be removed if worthless
      TMatrixFSym ErrorPositionMatrix = particle->getVertexErrorMatrix();
      double zError = ErrorPositionMatrix[2][2];
      return zError;
    }

    double MomentumOfSecondDaughter(const Particle* part)
    {
      if (part->getDaughter(1) == nullptr) return 0.0;
      else {
        return part->getDaughter(1)->getP();
      }
    }

    double MomentumOfSecondDaughter_CMS(const Particle* part)
    {
      if (part->getDaughter(1) == nullptr) return 0.0;
      else {
        PCmsLabTransform T;
        TLorentzVector vec = T.rotateLabToCms() * part->getDaughter(1)->get4Vector();
        return vec.P();
      }
    }

    double chargeTimesKaonLiklihood(const Particle*)
    {
      StoreObjPtr<ParticleList> kaons("K+:ROE");
      if (kaons->getListSize() == 0) return 0.0;
      double maximum_Kid = 0;
      double maximum_charge = 0;
      for (unsigned int i = 0; i < kaons->getListSize(); ++i) {
        Particle* p = kaons->getParticle(i);
        double Kid = p->getRelatedTo<PIDLikelihood>()->getProbability(Const::kaon, Const::pion);
        if (Kid > maximum_Kid) {
          maximum_Kid = Kid;
          maximum_charge = p->getCharge();
        }
      }
      return maximum_Kid * maximum_charge;
    }

    double transverseMomentumOfChargeTracksInRoe(const Particle* part)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (not roe.isValid())
        return 0.0;

      double sum = 0.0;
      double pt = 0.0;
      for (const auto & track : roe->getTracks()) {
        if (part->getTrack() == track)
          continue;
        if (track->getTrackFitResult(Const::pion) == nullptr)
          continue;
        pt = track->getTrackFitResult(Const::pion)->getTransverseMomentum();

        sum += sqrt(pt * pt);
      }
      return sum;

    }

    double McFlavorOfTagSide(const Particle* part)
    {
      const RestOfEvent* roe = part->getRelatedTo<RestOfEvent>();
      for (auto & track : roe->getTracks()) {
        const MCParticle* mcParticle = track->getRelated<MCParticle>();
        while (mcParticle != nullptr) {
          if (mcParticle->getPDG() == 511) return 511;
          else if (mcParticle->getPDG() == -511) return -511;
          mcParticle = mcParticle->getMother();
        }
      }
      return 0;
    }

    double cosTPTO(const Particle* part)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      const ContinuumSuppression* cs = roe->getRelated<Particle>()->getRelated<ContinuumSuppression>();
      const TVector3 thrustAxisO = cs->getThrustO(); //thrust is already in cms
      const TVector3 pAxis = PCmsLabTransform::labToCms(part->get4Vector()).Vect();
      double result = fabs(cos(pAxis.Angle(thrustAxisO)));

      //const ContinuumSuppression* qq = p->getRelated<ContinuumSuppression>();
      //const TVector3 thrustAxisO = qq->getThrustO();
      //cout << "thrustAxisO" << thrustAxisO << endl;
      //const TVector3 pAxis = PCmsLabTransform::labToCms(p->get4Vector()).Vect();
      //cout << "pAxis" << pAxis << endl;
      //double result = fabs(cos(pAxis.Angle(thrustAxisO)));
      return result;
    }

    double isInElectronOrMuonCat(const Particle* particle)
    {

      StoreObjPtr<ParticleList> MuonList("mu+:ROE");
      StoreObjPtr<ParticleList> ElectronList("e+:ROE");
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");

      double maximum_prob_el = 0;
      double maximum_prob_mu = 0;

      const Track* track_target_mu = nullptr;
      const Track* track_target_el = nullptr;

      for (unsigned int i = 0; i < MuonList->getListSize(); ++i) {
        Particle* p_mu = MuonList->getParticle(i);
        double prob_mu = p_mu->getExtraInfo("IsFromB(Muon)");
        if (prob_mu > maximum_prob_mu) {
          maximum_prob_mu = prob_mu;
          track_target_mu = p_mu -> getTrack();
        }
      }
      for (unsigned int i = 0; i < ElectronList->getListSize(); ++i) {
        Particle* p_el = ElectronList->getParticle(i);
//             const Particle* p_el = p ->getRelated<PartList>();
        double prob_el = p_el->getExtraInfo("IsFromB(Electron)");
        if (prob_el > maximum_prob_el) {
          maximum_prob_el = prob_el;
          track_target_el = p_el -> getTrack();
        }
      }
      if (particle->getTrack() == track_target_mu || particle->getTrack() == track_target_el) {
        return 1.0;
      } else return 0.0;
    }

    double cosKaonPion(const Particle* particle)
    {

//       StoreObjPtr<ParticleList> KaonList("K+:ROE");
      StoreObjPtr<ParticleList> SlowPionList("pi+:ROE");
//       StoreObjPtr<RestOfEvent> roe("RestOfEvent");

//       double maximum_prob_K = 0;
      double maximum_prob_pi = 0;

      PCmsLabTransform T;
      TLorentzVector momTarget_K = T.rotateLabToCms() * particle -> get4Vector();
      TLorentzVector momTarget_pi;

//       for (unsigned int i = 0; i < KaonList->getListSize(); ++i) {
//         Particle* p_K = KaonList->getParticle(i);
//         double prob_K = p_K->getExtraInfo("IsFromB(Kaon)");
//         if (prob_K > maximum_prob_K) {
//           maximum_prob_K = prob_K;
//           momTarget_K = T.rotateLabToCms() * p_K -> get4Vector();
//         }
//       }
      for (unsigned int i = 0; i < SlowPionList->getListSize(); ++i) {
        Particle* p_pi = SlowPionList->getParticle(i);
        double prob_pi = p_pi->getExtraInfo("IsFromB(SlowPion)");
        if (prob_pi > maximum_prob_pi) {
          maximum_prob_pi = prob_pi;
          momTarget_pi = T.rotateLabToCms() * p_pi -> get4Vector();
        }
      }
      return TMath::Cos(momTarget_K.Angle(momTarget_pi.Vect()));
    }

    double ImpactXY(const Particle* particle)
    {

      double x = particle->getX() - 0;
      double y = particle->getY() - 0;

      double px = particle->getPx();
      double py = particle->getPy();
      double pt = sqrt(px * px + py * py);

//       const TVector3 m_BeamSpotCenter = TVector3(0., 0., 0.);
//       TVector3 Bfield= BFieldMap::Instance().getBField(m_BeamSpotCenter); # TODO check why this produces a linking bug

      double a = -0.2998 * 1.5 * particle->getCharge(); //Curvature of the track,

      double T = TMath::Sqrt(pt * pt - 2 * a * (x * py - y * px) + a * a * (x * x + y * y));

      return TMath::Abs((-2 * (x * py - y * px) + a * (x * x + y * y)) / (T + pt));

    }

    double KaonPionHaveOpositeCharges(const Particle* particle)
    {

//       StoreObjPtr<ParticleList> KaonList("K+:ROE");
      StoreObjPtr<ParticleList> SlowPionList("pi+:ROE");
//       StoreObjPtr<RestOfEvent> roe("RestOfEvent");

//       double maximum_prob_K = 0;
      double maximum_prob_pi = 0;

      float chargeTarget_K = particle -> getCharge();
      float chargeTarget_pi = 0;

      for (unsigned int i = 0; i < SlowPionList->getListSize(); ++i) {
        Particle* p_pi = SlowPionList->getParticle(i);
        double prob_pi = p_pi->getExtraInfo("IsFromB(SlowPion)");
        if (prob_pi > maximum_prob_pi) {
          maximum_prob_pi = prob_pi;
          chargeTarget_pi =  p_pi -> getCharge();
        }
      }
      if (chargeTarget_K * chargeTarget_pi == -1) {
        return 1;
      } else return 0;
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
      for (const auto & daughter : daughters) {
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

    double particleClassifiedFlavor(const Particle* particle)
    {
      double result = -1.0;

      if (!(particle->getExtraInfo("Class_Flavor"))) return result;

      return particle->getExtraInfo("Class_Flavor");
    }

    double particleMCFlavor(const Particle* particle)
    {
      double result = -0.0;

      if (!(particle->getExtraInfo("MC_Flavor"))) return result;

      return particle->getExtraInfo("MC_Flavor");
    }

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

    bool isGoodGamma(int region, double energy, double e9e25, bool calibrated)
    {
      bool goodGammaRegion1, goodGammaRegion2, goodGammaRegion3;
      if (!calibrated) {
        goodGammaRegion1 = region == 1 && energy > 0.125 && e9e25 > 0.7;
        goodGammaRegion2 = region == 2 && energy > 0.100;
        goodGammaRegion3 = region == 3 && energy > 0.150;
      } else {
        goodGammaRegion1 = region == 1 && energy > 0.085 && e9e25 > 0.7;
        goodGammaRegion2 = region == 2 && energy > 0.060;
        goodGammaRegion3 = region == 3 && energy > 0.110;
      }
      //bool goodTiming       = timing > 800 && timing < 2400;

      //((goodGammaRegion1 || goodGammaRegion2 || goodGammaRegion3) && goodTiming)
      return goodGammaRegion1 || goodGammaRegion2 || goodGammaRegion3;
    }

    double goodGammaUncalibrated(const Particle* particle)
    {
      double energy = particle->getEnergy();
      double e9e25  = eclClusterE9E25(particle);
      int    region = eclClusterDetectionRegion(particle);

      return (double)isGoodGamma(region, energy, e9e25, false);
    }

    double goodGamma(const Particle* particle)
    {
      //double timing = eclClusterTiming(particle);
      double energy = particle->getEnergy();
      double e9e25  = eclClusterE9E25(particle);
      int    region = eclClusterDetectionRegion(particle);

      return (double)isGoodGamma(region, energy, e9e25, true);
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

    REGISTER_VARIABLE("p_CMS", particleP_CMS, "CMS momentum magnitude");
    REGISTER_VARIABLE("E_CMS", particleE_CMS, "CMS energy");
    REGISTER_VARIABLE("px_CMS", particlePx_CMS, "CMS momentum component x");
    REGISTER_VARIABLE("py_CMS", particlePy_CMS, "CMS momentum component y");
    REGISTER_VARIABLE("pz_CMS", particlePz_CMS, "CMS momentum component z");
    REGISTER_VARIABLE("pt_CMS", particlePt_CMS, "CMS transverse momentum");
    REGISTER_VARIABLE("cosTheta_CMS", particleCosTheta_CMS, "CMS momentum cosine of polar angle");
    REGISTER_VARIABLE("phi_CMS", particlePhi_CMS, "CMS momentum azimuthal angle in degrees");

    REGISTER_VARIABLE("cosThetaBetweenParticleAndTrueB", cosThetaBetweenParticleAndTrueB, "cosine of angle between momentum the particle and a true B particle. Is somewhere between -1 and 1 if only a massless particle like a neutrino is missing in the reconstruction.");
    REGISTER_VARIABLE("cosAngleBetweenMomentumAndVertexVector", cosAngleBetweenMomentumAndVertexVector, "cosine of angle between momentum and vertex vector (vector connecting ip and fitted vertex) of this particle");
    REGISTER_VARIABLE("distance", particleDistance, "distance relative to interaction point");
    REGISTER_VARIABLE("significanceOfDistance", particleDistanceSignificance, "significance of distance relative to interaction point (-1 in case of numerical problems)");
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
    REGISTER_VARIABLE("ErrM", particleInvariantMassError, "uncertainty of invariant mass (determined from particle's daughter 4-momentum vectors)");
    REGISTER_VARIABLE("SigM", particleInvariantMassSignificance, "signed deviation of particle's invariant mass from its nominal mass");

    VARIABLE_GROUP("MC Matching");
    REGISTER_VARIABLE("isSignal", isSignal,               "1.0 if Particle is correctly reconstructed (SIGNAL), 0.0 otherwise");
    REGISTER_VARIABLE("genMotherPDG", genMotherPDG,               "Check the PDG code of a particles MC mother particle");
    REGISTER_VARIABLE("isSignalAcceptMissingNeutrino", isSignalAcceptMissingNeutrino, "same as isSignal, but also accept missing neutrino");
    REGISTER_VARIABLE("mcPDG",    particleMCMatchPDGCode, "The PDG code of matched MCParticle, 0 if no match. Requires running matchMCTruth() on the particles first.");
    REGISTER_VARIABLE("abs_mcPDG", particleAbsMCMatchPDGCode, "The absolute PDG code of matched MCParticle");
    REGISTER_VARIABLE("mcErrors", particleMCErrors,  "The bit pattern indicating the quality of MC match (see MCMatching::MCErrorFlags)");

    VARIABLE_GROUP("Flavour tagging");
    REGISTER_VARIABLE("isMajorityInRestOfEventFromB0", isMajorityInRestOfEventFromB0, "[Eventbased] Check if the majority of the tracks in the current RestOfEvent are from a B0.");
    REGISTER_VARIABLE("isMajorityInRestOfEventFromB0bar", isMajorityInRestOfEventFromB0bar, "[Eventbased] Check if the majority of the tracks in the current RestOfEvent are from a B0bar.");
    REGISTER_VARIABLE("isRestOfEventOfB0", isRestOfEventOfB0,  "[Eventbased] Check if current RestOfEvent is related to a B0.");
    REGISTER_VARIABLE("isRestOfEventOfB0bar", isRestOfEventOfB0bar,  "[Eventbased] Check if current RestOfEvent is related to a B0 B0bar.");
    REGISTER_VARIABLE("isRestOfEventEmpty", isRestOfEventEmpty,  "-1 (1), -2 if current RestOfEvent is related to a B0bar (B0). But is used for checking if RoE empty.");
    REGISTER_VARIABLE("isRestOfEventB0Flavor", isRestOfEventB0Flavor,  "-1 (1) if current RestOfEvent is related to a B0bar (B0).");
    REGISTER_VARIABLE("qrCombined", isRestOfEventB0Flavor_Norm,  "0 (1) if current RestOfEvent is related to a B0bar (B0).");
    REGISTER_VARIABLE("p_miss", p_miss,  "Calculates the missing Momentum for a given particle on the tag side.");
    REGISTER_VARIABLE("isInRestOfEvent", isInRestOfEvent,  "1.0 of track, cluster of given particle is found in rest of event. 0 otherwise.");
    REGISTER_VARIABLE("NumberOfKShortinRemainingROEKaon", NumberOfKShortinRemainingROEKaon,  "Returns the number of K_S0 in the remainging Kaon ROE.");
    REGISTER_VARIABLE("NumberOfKShortinRemainingROELambda", NumberOfKShortinRemainingROELambda,  "Returns the number of K_S0 in the remainging Lambda ROE.");
    REGISTER_VARIABLE("lambdaFlavor", lambdaFlavor,  "1.0 if Lambda0, -1.0 if Anti-Lambda0, 0.0 else.");
    REGISTER_VARIABLE("lambdaZError", lambdaZError,  "Returns the Matrixelement[2][2] of the PositionErrorMatrix of the Vertex fit.");
    REGISTER_VARIABLE("MomentumOfSecondDaughter", MomentumOfSecondDaughter,  "Returns the Momentum of second daughter if existing, else 0.");
    REGISTER_VARIABLE("MomentumOfSecondDaughter_CMS", MomentumOfSecondDaughter_CMS,  "Returns the Momentum of second daughter if existing in CMS, else 0.");
    REGISTER_VARIABLE("chargeTimesKaonLiklihood", chargeTimesKaonLiklihood,  "Returns the q*(highest PID_Likelihood for Kaons), else 0.");
    REGISTER_VARIABLE("ptTracksRoe", transverseMomentumOfChargeTracksInRoe,  "Returns the transverse momentum of all charged tracks if there exists a ROE for the given particle, else 0.");
    REGISTER_VARIABLE("McFlavorOfTagSide",  McFlavorOfTagSide, "Flavour of tag side from MC extracted from the RoE");
    REGISTER_VARIABLE("BtagClassFlavor",  particleClassifiedFlavor,    "Flavour of Btag from trained Method");
    REGISTER_VARIABLE("BtagMCFlavor",  particleMCFlavor,    "Flavour of Btag from MC");
    REGISTER_VARIABLE("isInElectronOrMuonCat", isInElectronOrMuonCat,  "Returns 1.0 if the particle has been selected as target in the Muon or Electron Category, 0.0 else.");
    REGISTER_VARIABLE("cosKaonPion"  , cosKaonPion , "cosine of angle between kaon and slow pion momenta, i.e. between the momenta of the particles selected as target kaon and slow pion");
    REGISTER_VARIABLE("ImpactXY"  , ImpactXY , "The impact parameter of the given particle in the xy plane");
    REGISTER_VARIABLE("KaonPionHaveOpositeCharges", KaonPionHaveOpositeCharges, "Returns 1 if the particles selected as target kaon and slow pion have oposite charges, 0 else")

    VARIABLE_GROUP("Event");
    REGISTER_VARIABLE("isContinuumEvent",  isContinuumEvent,  "[Eventbased] true if event doesn't contain an Y(4S)");
    REGISTER_VARIABLE("nTracks",  nTracks,  "[Eventbased] number of tracks in the event");
    REGISTER_VARIABLE("nECLClusters", nECLClusters, "[Eventbased] number of ECL in the event");
    REGISTER_VARIABLE("nKLMClusters", nKLMClusters, "[Eventbased] number of KLM in the event");
    REGISTER_VARIABLE("ECLEnergy", ECLEnergy, "[Eventbased] total energy in ECL in the event");
    REGISTER_VARIABLE("KLMEnergy", KLMEnergy, "[Eventbased] total energy in KLM in the event");

    VARIABLE_GROUP("Rest Of Event");
    REGISTER_VARIABLE("nROETracks",  nROETracks,  "number of remaining tracks as given by the related RestOfEvent object");
    REGISTER_VARIABLE("nROEECLClusters", nROEECLClusters, "number of remaining ECL clusters as given by the related RestOfEvent object");
    REGISTER_VARIABLE("nROEKLMClusters", nROEKLMClusters, "number of remaining KLM clusters as given by the related RestOfEvent object");
    REGISTER_VARIABLE("nRemainingTracksInRestOfEvent", nRemainingTracksInRestOfEvent, "Returns number of tracks in ROE - number of tracks of given particle");

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
    REGISTER_VARIABLE("mdstIndex", particleMdstArrayIndex, "StoreArray index (0-based) of the MDST object from which the Particle was created");

    REGISTER_VARIABLE("pRecoil",  recoilMomentum,    "magnitude of 3-momentum recoiling against given Particle");
    REGISTER_VARIABLE("eRecoil",  recoilEnergy,   "energy recoiling against given Particle");
    REGISTER_VARIABLE("mRecoil",  recoilMass,        "invariant mass of the system recoiling against given Particle");
    REGISTER_VARIABLE("m2Recoil", recoilMassSquared, "invariant mass squared of the system recoiling against given Particle");

    REGISTER_VARIABLE("eextra", extraEnergy, "extra energy in the calorimeter that is not associated to the given Particle");

    REGISTER_VARIABLE("printParticle", printParticle, "For debugging, print Particle and daughter PDG codes, plus MC match. Returns 0.");
    REGISTER_VARIABLE("mcParticleStatus", mcParticleStatus, "Returns status bits of related MCParticle or -1 if MCParticle relation is not set.");
    REGISTER_VARIABLE("mcPrimary", particleMCPrimaryParticle, "Returns 1 if Particle is related to primary MCParticle, 0 if Particle is related to non-primary MCParticle, -1 if Particle is not related to MCParticle.")
    REGISTER_VARIABLE("False", False, "returns always 0, used for testing and debugging.");

    VARIABLE_GROUP("ECL Cluster related");
    REGISTER_VARIABLE("goodGamma",         goodGamma, "1.0 if photon candidate passes good photon selection criteria");
    REGISTER_VARIABLE("goodGammaUnCal",    goodGammaUncalibrated, "1.0 if photon candidate passes good photon selection criteria (to be used if photon's energy is not calibrated)");
    REGISTER_VARIABLE("clusterReg",        eclClusterDetectionRegion, "detection region in the ECL [1 - forward, 2 - barrel, 3 - backward]");
    REGISTER_VARIABLE("clusterE9E25",      eclClusterE9E25,           "ratio of energies in inner 3x3 and 5x5 cells");
    REGISTER_VARIABLE("clusterNHits",      eclClusterNHits,           "number of hits associated to this cluster");
    REGISTER_VARIABLE("clusterTrackMatch", eclClusterTrackMatched,    "number of charged track matched to this cluster");

    VARIABLE_GROUP("Continuum Suppression");
    REGISTER_VARIABLE("cosTBTO"  , cosTBTO , "cosine of angle between thrust axis of B and thrust axis of ROE");
    REGISTER_VARIABLE("cosTBz"   , cosTBz  , "cosine of angle between thrust axis of B and z-axis");
    REGISTER_VARIABLE("cosTPTO"  , cosTPTO , "cosine of angle between thrust axis of given particle and thrust axis of ROE");
    REGISTER_VARIABLE("thrustBm" , thrustBm, "magnitude of the B thrust axis");
    REGISTER_VARIABLE("thrustOm" , thrustOm, "magnitude of the ROE thrust axis");
    REGISTER_VARIABLE("R2"       , R2      , "reduced Fox-Wolfram moment R2");
  }
}
