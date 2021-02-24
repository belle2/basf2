/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Fernando Abudinen, Moritz Gelb                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/variables/FlavorTaggingVariables.h>
#include <analysis/variables/MCTruthVariables.h>
#include <analysis/variables/KLMClusterVariables.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <analysis/ClusterUtility/ClusterUtils.h>

#include <analysis/utility/MCMatching.h>

// framework - DataStore
#include <framework/datastore/StoreObjPtr.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/FlavorTaggerInfo.h>
#include <analysis/ContinuumSuppression/Thrust.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>


// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TLorentzVector.h>
#include <TVector3.h>

#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

    auto& labToCms = PCmsLabTransform::labToCms;
    static const double realNaN = std::numeric_limits<double>::quiet_NaN();
    //   ############################################## FlavorTagger Variables   ###############################################

    // Track Level Variables ---------------------------------------------------------------------------------------------------

    double momentumMissingTagSide(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (!roe.isValid()) return 0;

      TLorentzVector roeCMSVec;

      const auto& roeChargedParticles = roe->getChargedParticles();
      for (auto roeChargedParticle : roeChargedParticles) {
        if (not roeChargedParticle->isMostLikely()) continue;
        roeCMSVec += labToCms(roeChargedParticle->get4Vector());
      }

      double missMom = -roeCMSVec.P();
      return missMom ;
    }

    double cosTPTO(const Particle* part)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (!roe.isValid()) return 0;

      std::vector<TVector3> p3_cms_roe;
      static const double P_MAX(3.2);

      // Charged tracks
      //
      const auto& roeTracks = roe->getChargedParticles();
      for (auto& roeChargedParticle : roeTracks) {
        // TODO: Add helix and KVF with IpProfile once available. Port from L163-199 of:
        // /belle/b20090127_0910/src/anal/ekpcontsuppress/src/ksfwmoments.cc
        if (roeChargedParticle->isMostLikely()) {
          TLorentzVector p_cms = labToCms(roeChargedParticle->get4Vector());
          if (p_cms != p_cms) continue;
          if (p_cms.Rho() > P_MAX) continue;
          p3_cms_roe.push_back(p_cms.Vect());
        }
      }

      // ECLCluster -> Gamma
      const auto& roePhotons = roe->getPhotons();
      for (auto& roePhoton : roePhotons) {
        if (roePhoton->getECLClusterEHypothesisBit() == ECLCluster::EHypothesisBit::c_nPhotons) {
          TLorentzVector p_lab = roePhoton->get4Vector();
          if (p_lab != p_lab) continue;
          if (p_lab.Rho() < 0.05) continue;
          TLorentzVector p_cms = labToCms(p_lab);
          if (p_cms != p_cms) continue;
          if (p_cms.Rho() > P_MAX) continue;
          p3_cms_roe.push_back(p_cms.Vect());
        }
      }

      const auto& roeKlongs = roe->getHadrons();
      for (auto& roeKlong : roeKlongs) {
        if (nKLMClusterTrackMatches(roeKlong) == 0 && !(roeKlong->getKLMCluster()->getAssociatedEclClusterFlag())) {
          TLorentzVector p_lab = roeKlong->get4Vector();
          if (p_lab != p_lab) continue;
          if (p_lab.Rho() < 0.05) continue;
          TLorentzVector p_cms = labToCms(p_lab);
          if (p_cms != p_cms) continue;
          if (p_cms.Rho() > P_MAX) continue;
          p3_cms_roe.push_back(p_cms.Vect());
        }
      }

      const TVector3 thrustO  = Thrust::calculateThrust(p3_cms_roe);
      const TVector3 pAxis = labToCms(part->get4Vector()).Vect();

      double result = 0 ;
      if (pAxis == pAxis) result = abs(cos(pAxis.Angle(thrustO)));

      return result;
    }

    double lambdaFlavor(const Particle* particle)
    {
      if (particle->getPDGCode() == Const::Lambda.getPDGCode()) return 1.0; //Lambda0
      else if (particle->getPDGCode() == Const::antiLambda.getPDGCode()) return -1.0; //Anti-Lambda0
      else return 0.0;
    }

    double isLambda(const Particle* particle)
    {
      const MCParticle* mcparticle = particle->getMCParticle();
      if (!mcparticle) return 0.0;
      return (abs(mcparticle->getPDG()) == Const::Lambda.getPDGCode());
    }

    double lambdaZError(const Particle* particle)
    {
      //This is a simplisitc hack. But I see no other way to get that information.
      //Should be removed if worthless
      TMatrixFSym ErrorPositionMatrix = particle->getVertexErrorMatrix();
      return ErrorPositionMatrix[2][2];
    }

    double momentumOfSecondDaughter(const Particle* part)
    {
      if (!part->getDaughter(1)) return 0.0;
      return part->getDaughter(1)->getP();
    }

    double momentumOfSecondDaughterCMS(const Particle* part)
    {
      if (!part->getDaughter(1)) return 0.0;
      TLorentzVector vec = labToCms(part->getDaughter(1)->get4Vector());
      return vec.P();
    }

    double chargeTimesKaonLiklihood(const Particle*)
    {
      StoreObjPtr<ParticleList> KaonList("K+:inRoe");
      if (!KaonList.isValid()) return 0;

      double maximumKaonid = 0;
      double maximum_charge = 0;
      for (unsigned int i = 0; i < KaonList->getListSize(); ++i) {
        const Particle* p = KaonList->getParticle(i);
        double Kid = p->getRelatedTo<PIDLikelihood>()->getProbability(Const::kaon, Const::pion);
        if (Kid > maximumKaonid) {
          maximumKaonid = Kid;
          maximum_charge = p->getCharge();
        }
      }
      return maximumKaonid * maximum_charge;
    }

    double transverseMomentumOfChargeTracksInRoe(const Particle* part)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (!roe.isValid()) return 0;

      double sum = 0.0;

      for (const auto& track : roe->getChargedParticles()) {
        if (part->isCopyOf(track, true)) continue;
        if (!track->isMostLikely()) continue;
        sum += track->getMomentum().Perp2();
      }

      return sum;

    }

    double NumberOfKShortsInRoe(const Particle* particle)
    {
      StoreObjPtr<ParticleList> KShortList("K_S0:inRoe");
      if (!KShortList.isValid())
        B2FATAL("NumberOfKShortsInRoe cannot be calculated because the required particleList K_S0:inRoe could not be found or is not valid");


      int flag = 0;
      for (unsigned int i = 0; i < KShortList->getListSize(); ++i) {
        if (!particle->overlapsWith(KShortList->getParticle(i)))
          ++flag;
      }
      return flag;
    }

//     Event Level Variables --------------------------------------------------------------------------------------------

    double isInElectronOrMuonCat(const Particle* particle)
    {
      // check muons
      StoreObjPtr<ParticleList> MuonList("mu+:inRoe");
      const Track* trackTargetMuon = nullptr;
      if (MuonList.isValid()) {
        double maximumProbMuon = 0;
        for (unsigned int i = 0; i < MuonList->getListSize(); ++i) {
          Particle* pMuon = MuonList->getParticle(i);
          double probMuon = pMuon->getExtraInfo("isRightTrack(Muon)");
          if (probMuon > maximumProbMuon) {
            maximumProbMuon = probMuon;
            trackTargetMuon = pMuon -> getTrack();
          }
        }
      }
      if (particle->getTrack() == trackTargetMuon)
        return true;


      // check electrons
      StoreObjPtr<ParticleList> ElectronList("e+:inRoe");
      const Track* trackTargetElectron = nullptr;
      if (ElectronList.isValid()) {
        double maximumProbElectron = 0;
        for (unsigned int i = 0; i < ElectronList->getListSize(); ++i) {
          Particle* pElectron = ElectronList->getParticle(i);
          double probElectron = pElectron->getExtraInfo("isRightTrack(Electron)");
          if (probElectron > maximumProbElectron) {
            maximumProbElectron = probElectron;
            trackTargetElectron = pElectron -> getTrack();
          }
        }
      }
      if (particle->getTrack() == trackTargetElectron)
        return true;

      return false;
    }

    // helper function to get flavour of MC B0
    static int getB0flavourMC(const MCParticle* mcParticle)
    {
      while (mcParticle) {
        if (mcParticle->getPDG() == 511) {
          return 1;
        } else if (mcParticle->getPDG() == -511) {
          return -1;
        }
        mcParticle = mcParticle->getMother();
      }
      return 0; //no B found
    }

//     Target Variables --------------------------------------------------------------------------------------------------

    double isMajorityInRestOfEventFromB0(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (!roe.isValid()) return 0;

      int vote = 0;
      for (auto& track : roe->getChargedParticles()) {
        const MCParticle* mcParticle = track->getMCParticle();
        vote += getB0flavourMC(mcParticle);
      }

      return vote > 0;
    }

    double isMajorityInRestOfEventFromB0bar(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (!roe.isValid()) return 0;

      int vote = 0;
      for (auto& track : roe->getChargedParticles()) {
        const MCParticle* mcParticle = track->getMCParticle();
        vote += getB0flavourMC(mcParticle);
      }

      return vote < 0;
    }

    double hasRestOfEventTracks(const Particle* part)
    {
      const RestOfEvent* roe = part->getRelatedTo<RestOfEvent>();
      return (roe && roe-> getNTracks() > 0);
    }

    double isRelatedRestOfEventB0Flavor(const Particle* particle)
    {
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();
      if (!roe) return 0;

      const MCParticle* BcpMC = particle->getMCParticle();
      if (!BcpMC) return 0;
      if (Variable::isSignal(particle) <= 0) return 0;

      const MCParticle* Y4S = BcpMC->getMother();
      if (!Y4S) return 0;

      int BtagFlavor = 0;
      int BcpFlavor = 0;

      for (auto& roeChargedParticle : roe->getChargedParticles()) {
        const MCParticle* mcParticle = roeChargedParticle->getMCParticle();
        while (mcParticle) {
          if (mcParticle->getMother() == Y4S) {
            if (mcParticle == BcpMC) {
              if (mcParticle->getPDG() > 0) BcpFlavor = 2;
              else BcpFlavor = -2;
            } else if (BtagFlavor == 0) {
              if (abs(mcParticle->getPDG()) == 511 || abs(mcParticle->getPDG()) == 521) {
                if (mcParticle->getPDG() > 0) BtagFlavor = 1;
                else BtagFlavor = -1;
              } else BtagFlavor = 5;
            }
            break;
          }
          mcParticle = mcParticle->getMother();
        }
        if (BcpFlavor != 0 || BtagFlavor == 5) break;
      }


      return (BcpFlavor != 0) ? BcpFlavor : BtagFlavor;
    }

    double isRestOfEventB0Flavor(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (!roe.isValid()) return 0;

      const Particle* Bcp = roe->getRelated<Particle>();
      return Variable::isRelatedRestOfEventB0Flavor(Bcp);
    }

    double ancestorHasWhichFlavor(const Particle* particle)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (!roe.isValid()) return 0;

      const MCParticle* BcpMC = roe->getRelated<Particle>()->getMCParticle();
      const MCParticle* Y4S = BcpMC->getMother();
      const MCParticle* mcParticle = particle->getMCParticle();

      int outputB0tagQ = 0;
      while (mcParticle) {
        if (mcParticle->getMother() == Y4S) {
          if (mcParticle != BcpMC && abs(mcParticle -> getPDG()) == 511) {
            if (mcParticle -> getPDG() == 511) outputB0tagQ = 1;
            else outputB0tagQ = -1;
          } else if (mcParticle == BcpMC) {
            if (mcParticle -> getPDG() == 511) outputB0tagQ = 2;
            else outputB0tagQ = -2;
          } else outputB0tagQ = 5;
          break;
        }
        mcParticle = mcParticle->getMother();
      }

      return outputB0tagQ;
    }

    double B0mcErrors(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (!roe.isValid()) return -1;

      const Particle* Bcp = roe->getRelated<Particle>();
      const MCParticle* BcpMC = roe->getRelated<Particle>()->getMCParticle();
      return MCMatching::getMCErrors(Bcp, BcpMC);
    }


    double isRelatedRestOfEventMajorityB0Flavor(const Particle* part)
    {
      const RestOfEvent* roe = part->getRelatedTo<RestOfEvent>();
      if (!roe) return -2;

      int q_MC = 0; //Flavor of B

      if (roe->getNTracks() > 0) {
        for (auto& track : roe->getChargedParticles()) {
          const MCParticle* mcParticle = track->getMCParticle();
          q_MC += getB0flavourMC(mcParticle);
        }
      } else if (roe->getNECLClusters() > 0) {
        for (auto& cluster : roe->getPhotons()) {
          if (cluster->getECLClusterEHypothesisBit() != ECLCluster::EHypothesisBit::c_nPhotons) continue;
          const MCParticle* mcParticle = cluster->getMCParticle();
          q_MC += getB0flavourMC(mcParticle);
        }
      } else if (roe->getNKLMClusters() > 0) {
        for (auto& klmcluster : roe->getHadrons()) {
          const MCParticle* mcParticle = klmcluster->getMCParticle();
          q_MC += getB0flavourMC(mcParticle);
        }
      }

      if (q_MC == 0)
        return -2;
      else
        return (q_MC > 0);
    }

    double isRestOfEventMajorityB0Flavor(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (!roe.isValid()) return -2;//gRandom->Uniform(0, 1);

      const Particle* Bcp = roe->getRelated<Particle>();
      return Variable::isRelatedRestOfEventMajorityB0Flavor(Bcp);
    }

    double mcFlavorOfOtherB(const Particle* particle)
    {

      if (std::abs(particle->getPDGCode()) != 511 && std::abs(particle->getPDGCode()) != 521) {
        B2ERROR("MCFlavorOfOtherB: this variable works only for B mesons.\n"
                "The given particle with PDG code " << particle->getPDGCode() <<
                " is not a B-meson candidate (PDG code 511 or 521). ");
        return realNaN;
      }

      const MCParticle* mcParticle = particle->getMCParticle();
      if (!mcParticle) return realNaN;

      const MCParticle* mcMother = mcParticle->getMother();
      if (!mcMother) return realNaN;

      if (Variable::isSignal(particle) < 1.0) return 0;

      for (auto& upsilon4SDaughter : mcMother->getDaughters()) {
        if (upsilon4SDaughter == mcParticle) continue;
        return (upsilon4SDaughter->getPDG() > 0) ? 1 : -1;
      }

      return 0;

    };

//  ######################################### Meta Variables ##############################################

//  Track and Event Level variables ------------------------------------------------------------------------

    Manager::FunctionPtr BtagToWBosonVariables(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto requestedVariable = arguments[0];
        auto func = [requestedVariable](const Particle * particle) -> double {
          StoreObjPtr<RestOfEvent> roe("RestOfEvent");
          if (!roe.isValid()) return 0;

          TLorentzVector momXChargedTracks; //Momentum of charged X tracks in CMS-System

          const auto& roeChargedParticles = roe->getChargedParticles();
          for (auto& roeChargedParticle : roeChargedParticles)
          {
            if (roeChargedParticle->isCopyOf(particle, true)) continue;
            momXChargedTracks += roeChargedParticle->get4Vector();
          }

          TLorentzVector momXNeutralClusters = roe->get4VectorNeutralECLClusters(); //Momentum of neutral X clusters in CMS-System

          const auto& klongs = roe->getHadrons();
          for (auto& klong : klongs)
          {
            if (nKLMClusterTrackMatches(klong) == 0 && !(klong->getKLMCluster()->getAssociatedEclClusterFlag())) {
              momXNeutralClusters += klong->get4Vector();
            }
          }

          TLorentzVector momX = PCmsLabTransform::labToCms(momXChargedTracks + momXNeutralClusters); //Total Momentum of the recoiling X in CMS-System
          TLorentzVector momTarget = PCmsLabTransform::labToCms(particle->get4Vector());  //Momentum of Mu in CMS-System
          TLorentzVector momMiss = -(momX + momTarget); //Momentum of Anti-v  in CMS-System

          double output = 0.0;
          if (requestedVariable == "recoilMass") output = momX.M();
          else if (requestedVariable == "recoilMassSqrd") output = momX.M2();
          else if (requestedVariable == "pMissCMS") output = momMiss.Vect().Mag();
          else if (requestedVariable == "cosThetaMissCMS") output = TMath::Cos(momTarget.Angle(momMiss.Vect()));
          else if (requestedVariable == "EW90")
          {

            TLorentzVector momW = momTarget + momMiss; //Momentum of the W-Boson in CMS
            float E_W_90 = 0 ; // Energy of all charged and neutral clusters in the hemisphere of the W-Boson

            const auto& photons = roe->getPhotons();
            for (auto& photon : photons) {
              if (PCmsLabTransform::labToCms(photon->get4Vector()).Vect().Dot(momW.Vect()) > 0) {
                E_W_90 += photon->getECLClusterEnergy();
              }
            }
            for (auto& roeChargedParticle : roeChargedParticles) {
              if (!roeChargedParticle->isCopyOf(particle, true)) {
                for (const ECLCluster& chargedCluster : roeChargedParticle->getTrack()->getRelationsWith<ECLCluster>()) {
                  // ignore everything except the nPhotons hypothesis
                  if (!chargedCluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
                    continue;
                  float iEnergy = chargedCluster.getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
                  if (iEnergy == iEnergy) {
                    if (PCmsLabTransform::labToCms(ClusterUtils().Get4MomentumFromCluster(&chargedCluster,
                                                   ECLCluster::EHypothesisBit::c_nPhotons)).Vect().Dot(momW.Vect()) > 0)
                      E_W_90 += iEnergy;
                  }
                }
              }
            }

            output = E_W_90;
          } else {
            B2FATAL("Wrong variable " << requestedVariable <<
            " requested. The possibilities are recoilMass, recoilMassSqrd, pMissCMS, cosThetaMissCMS or EW90");
          }

          return output;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (1 required) for meta function BtagToWBosonVariables");
      }
    }

    Manager::FunctionPtr KaonPionVariables(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments (1 required) for meta function KaonPionVariables");


      auto requestedVariable = arguments[0];
      auto func = [requestedVariable](const Particle * particle) -> double {
        //       StoreObjPtr<ParticleList> KaonList("K+:ROE");
        StoreObjPtr<ParticleList> SlowPionList("pi+:inRoe");


        if ((requestedVariable != "HaveOpositeCharges") && (requestedVariable != "cosKaonPion"))
          B2FATAL("Wrong variable  " << requestedVariable << " requested. The possibilities are cosKaonPion or HaveOpositeCharges");


        TLorentzVector momTargetSlowPion;
        double chargeTargetSlowPion = 0;
        if (SlowPionList.isValid())
        {
          double maximumProbSlowPion = 0;
          for (unsigned int i = 0; i < SlowPionList->getListSize(); ++i) {
            Particle* pSlowPion = SlowPionList->getParticle(i);
            if (!pSlowPion) continue;
            if (!pSlowPion->hasExtraInfo("isRightCategory(SlowPion)")) continue;

            double probSlowPion = pSlowPion->getExtraInfo("isRightCategory(SlowPion)");
            if (probSlowPion > maximumProbSlowPion) {
              maximumProbSlowPion = probSlowPion;
              chargeTargetSlowPion =  pSlowPion->getCharge();
              momTargetSlowPion = labToCms(pSlowPion->get4Vector());
            }
          }
        }

        double output = 0.0;

        double chargeTargetKaon = particle->getCharge();
        if (requestedVariable == "HaveOpositeCharges")
        {
          if (chargeTargetKaon * chargeTargetSlowPion == -1)
            output = 1;
        }
        //TODO: when momTargetSlowPion == momTargetSlowPion fail?
        else if (requestedVariable == "cosKaonPion")
        {
          TLorentzVector momTargetKaon = labToCms(particle->get4Vector());
          if (momTargetKaon == momTargetKaon && momTargetSlowPion == momTargetSlowPion)
            output = cos(momTargetKaon.Angle(momTargetSlowPion.Vect()));
        }

        return output;
      };
      return func;
    }

    Manager::FunctionPtr FSCVariables(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments (1 required) for meta function FSCVariables");


      auto requestedVariable = arguments[0];
      auto func = [requestedVariable](const Particle * particle) -> double {
        StoreObjPtr<ParticleList> FastParticleList("pi+:inRoe");
        if (!FastParticleList.isValid()) return 0;


        if ((requestedVariable != "pFastCMS") && (requestedVariable != "cosSlowFast") && (requestedVariable != "cosTPTOFast") && (requestedVariable != "SlowFastHaveOpositeCharges"))
          B2FATAL("Wrong variable " << requestedVariable << " requested. The possibilities are pFastCMS, cosSlowFast, cosTPTOFast or SlowFastHaveOpositeCharges");


        double maximumProbFastest = 0;
        TLorentzVector momFastParticle;  //Momentum of Fast Pion in CMS-System
        Particle* TargetFastParticle = nullptr;
        for (unsigned int i = 0; i < FastParticleList->getListSize(); ++i)
        {
          Particle* particlei = FastParticleList->getParticle(i);
          if (!particlei) continue;

          TLorentzVector momParticlei = labToCms(particlei->get4Vector());
          if (momParticlei != momParticlei) continue;

          double probFastest = momParticlei.P();
          if (probFastest > maximumProbFastest) {
            maximumProbFastest = probFastest;
            TargetFastParticle = particlei;
            momFastParticle = momParticlei;
          }
        }

        // if nothing found
        if (!TargetFastParticle) return 0;


        double output = 0.0;

        if (requestedVariable == "cosTPTOFast")
          output = Variable::Manager::Instance().getVariable("cosTPTO")->function(TargetFastParticle);

        TLorentzVector momSlowPion = labToCms(particle->get4Vector());  //Momentum of Slow Pion in CMS-System
        if (momSlowPion == momSlowPion)   // FIXME
        {
          if (requestedVariable == "cosSlowFast") {
            output = cos(momSlowPion.Angle(momFastParticle.Vect()));
          } else if (requestedVariable == "SlowFastHaveOpositeCharges") {
            if (particle->getCharge()*TargetFastParticle->getCharge() == -1) {
              output = 1;
            }
          } else {
            output = momFastParticle.P();
          }
        }

        return output;
      };
      return func;
    }

    Manager::FunctionPtr hasHighestProbInCat(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 2) {
        B2FATAL("Wrong number of arguments (2 required) for meta function hasHighestProbInCat");
      }

      auto particleListName = arguments[0];
      auto extraInfoName = arguments[1];
      auto func = [particleListName, extraInfoName](const Particle * particle) -> double {
        if (!(extraInfoName == "isRightTrack(Electron)" || extraInfoName == "isRightTrack(IntermediateElectron)" || extraInfoName == "isRightTrack(Muon)" || extraInfoName == "isRightTrack(IntermediateMuon)"
        || extraInfoName == "isRightTrack(KinLepton)" || extraInfoName == "isRightTrack(IntermediateKinLepton)" || extraInfoName == "isRightTrack(Kaon)"
        || extraInfoName == "isRightTrack(SlowPion)" || extraInfoName == "isRightTrack(FastHadron)" || extraInfoName == "isRightTrack(MaximumPstar)" || extraInfoName == "isRightTrack(Lambda)"
        || extraInfoName == "isRightCategory(Electron)" || extraInfoName == "isRightCategory(IntermediateElectron)" || extraInfoName == "isRightCategory(Muon)" || extraInfoName == "isRightCategory(IntermediateMuon)"
        || extraInfoName == "isRightCategory(KinLepton)" || extraInfoName == "isRightCategory(IntermediateKinLepton)" || extraInfoName == "isRightCategory(Kaon)"
        || extraInfoName == "isRightCategory(SlowPion)" || extraInfoName == "isRightCategory(FastHadron)" || extraInfoName == "isRightCategory(KaonPion)" || extraInfoName == "isRightCategory(Lambda)"
        || extraInfoName == "isRightCategory(MaximumPstar)" || extraInfoName == "isRightCategory(FSC)"))
        {
          B2FATAL("hasHighestProbInCat: Not available category" << extraInfoName <<
          ". The possibilities for isRightTrack() are \nElectron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, MaximumPstar, and Lambda."
          << endl <<
          "The possibilities for isRightCategory() are \nElectron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, KaonPion, MaximumPstar, FSC and Lambda");
          return 0.0;
        }

        StoreObjPtr<ParticleList> ListOfParticles(particleListName);
        if (!ListOfParticles.isValid()) return 0;

        double maximumProb = 0;
        for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i)
        {
          const Particle* particlei = ListOfParticles->getParticle(i);
          if (!particlei) continue;

          double prob = 0;
          if (extraInfoName == "isRightTrack(MaximumPstar)") {
            TLorentzVector momParticlei = labToCms(particlei -> get4Vector());
            if (momParticlei == momParticlei) {
              prob = momParticlei.P();
            }
          } else {
            if (particlei->hasExtraInfo(extraInfoName)) {
              prob = particlei->getExtraInfo(extraInfoName);
            }
          }
          if (prob > maximumProb) {
            maximumProb = prob;
          }

        }

        double output = 0.0;
        if ((extraInfoName == "isRightTrack(MaximumPstar)") && (labToCms(particle->get4Vector()).P() == maximumProb))
        {
          output = 1.0;
        } else if (extraInfoName != "isRightTrack(MaximumPstar)" && particle->hasExtraInfo(extraInfoName))
        {
          if (particle->getExtraInfo(extraInfoName) == maximumProb) output = 1.0;
        }

        return output;
      };
      return func;
    }

    Manager::FunctionPtr HighestProbInCat(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 2) {
        B2FATAL("Wrong number of arguments (2 required) for meta function HighestProbInCat");
      }

      auto particleListName = arguments[0];
      auto extraInfoName = arguments[1];
      auto func = [particleListName, extraInfoName](const Particle*) -> double {
        if (!(extraInfoName == "isRightTrack(Electron)" || extraInfoName == "isRightTrack(IntermediateElectron)" || extraInfoName == "isRightTrack(Muon)" || extraInfoName == "isRightTrack(IntermediateMuon)"
        || extraInfoName == "isRightTrack(KinLepton)" || extraInfoName == "isRightTrack(IntermediateKinLepton)" || extraInfoName == "isRightTrack(Kaon)"
        || extraInfoName == "isRightTrack(SlowPion)" || extraInfoName == "isRightTrack(FastHadron)" || extraInfoName == "isRightTrack(MaximumPstar)" || extraInfoName == "isRightTrack(Lambda)"
        || extraInfoName == "isRightCategory(Electron)" || extraInfoName == "isRightCategory(IntermediateElectron" || extraInfoName == "isRightCategory(Muon)" || extraInfoName == "isRightCategory(IntermediateMuon)"
        || extraInfoName == "isRightCategory(KinLepton)" || extraInfoName == "isRightCategory(IntermediateKinLepton)" || extraInfoName == "isRightCategory(Kaon)"
        || extraInfoName == "isRightCategory(SlowPion)" || extraInfoName == "isRightCategory(FastHadron)" || extraInfoName == "isRightCategory(KaonPion)" || extraInfoName == "isRightCategory(Lambda)"
        || extraInfoName == "isRightCategory(MaximumPstar)" || extraInfoName == "isRightCategory(FSC)"))
        {
          B2FATAL("HighestProbInCat: Not available category" << extraInfoName <<
          ". The possibilities for isRightTrack() are \nElectron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, MaximumPstar, and Lambda."
          << endl <<
          "The possibilities for isRightCategory() are \nElectron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, KaonPion, MaximumPstar, FSC and Lambda");
          return 0.0;
        }

        StoreObjPtr<ParticleList> ListOfParticles(particleListName);
        if (!ListOfParticles.isValid()) return 0;

        double maximumProb = 0;
        for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i)
        {
          const Particle* particlei = ListOfParticles->getParticle(i);
          if (!particlei) continue;

          double prob = 0;
          if (extraInfoName == "isRightTrack(MaximumPstar)") {
            TLorentzVector momParticlei = labToCms(particlei -> get4Vector());
            if (momParticlei == momParticlei) {
              prob = momParticlei.P();
            }
          } else {
            if (particlei->hasExtraInfo(extraInfoName)) {
              prob = particlei->getExtraInfo(extraInfoName);
            }
          }
          maximumProb = max(maximumProb, prob);
        }

        return maximumProb;
      };
      return func;
    }

//  Target Variables ----------------------------------------------------------------------------------------------

    // Lists used in target variables
    const std::vector<int> charmMesons = { 411, 421, 10411, 10421, 413, 423, 10413, 10423, 20413, 20423, 415, 425, 431, 10431, 433, 10433, 20433, 435};

    const std::vector<int> charmBaryons = { 4122, 4222, 4212, 4112, 4224, 4214, 4114, 4232, 4132, 4322, 4312, 4324, 4314, 4332, 4334, 4412, 4422,
                                            4414, 4424, 4432, 4434, 4444
                                          };

    const std::vector<int> qqbarMesons = {// light qqbar
      111, 9000111, 100111, 10111, 200111, 113, 10113, 20113, 9000113, 100113, 9010113, 9020113, 30113, 9030113, 9040113,
      115, 10115, 100115, 9000115, 117, 9000117, 9010117, 119,
      // ssbar Mesons
      221, 331, 9000221, 9010221, 100221, 10221, 100331, 9020221, 10331, 200221, 9030221, 9040221, 9050221, 9060221, 9070221, 223, 333, 10223, 20223,
      10333, 20333, 100223, 9000223, 9010223, 30223, 100333, 225, 9000225, 335, 9010225, 9020225, 10225, 9030225, 10335, 9040225, 100225, 100335,
      9050225, 9060225, 9070225, 227, 337, 229, 9000339, 9000229,
      // ccbar Mesons
      441, 10441, 100441, 443, 10443, 20443, 100443, 30443, 9000443, 9010443, 9020443, 445, 9000445
    };

    const std::vector<int> flavorConservingMesons = {// Excited light mesons that can decay into hadrons conserving flavor
      9000211, 100211, 10211, 200211, 213, 10213, 20213, 9000213, 100213, 9010213, 9020213, 30213, 9030213, 9040213,
      215, 10215, 100215, 9000215, 217, 9000217, 9010217, 219,
      // Excited K Mesons that hadronize conserving flavor
      30343, 10311, 10321, 100311, 100321, 200311, 200321, 9000311, 9000321, 313, 323, 10313, 10323, 20313, 20323, 100313, 100323,
      9000313, 9000323, 30313, 30323, 315, 325, 9000315, 9000325, 10315, 10325, 20315, 20325, 100315, 100325, 9010315,
      9010325, 317, 327, 9010317, 9010327, 319, 329, 9000319, 9000329
    };

    Manager::FunctionPtr isRightTrack(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1) {
        B2FATAL("Wrong number of arguments (1 required) for meta function isRightTrack");
      }


      auto particleName = arguments[0];

      std::vector<std::string> names = {     "Electron",             // 0
                                             "IntermediateElectron", // 1
                                             "Muon",                 // 2
                                             "IntermediateMuon",     // 3
                                             "KinLepton",            // 4
                                             "IntermediateKinLepton",// 5
                                             "Kaon",                 // 6
                                             "SlowPion",             // 7
                                             "FastHadron",             // 8
                                             "Lambda",               // 9
                                             "mcAssociated"          // 10
                                       };

      unsigned index = std::find(names.begin(), names.end(), particleName) - names.begin();
      if (index == names.size()) {
        B2FATAL("isRightTrack: Not available category " << particleName <<
                ". The possibilities are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron and Lambda");
      }

      auto func = [index](const Particle * particle) -> double {

        const MCParticle* mcParticle = particle->getMCParticle();
        if (!mcParticle) return -2.0;

        int mcPDG = abs(mcParticle->getPDG());

        // ---------------------------- Mothers and Grandmothers ----------------------------------
        std::vector<int> mothersPDG;
        std::vector<const MCParticle*> mothersPointers;

        const MCParticle* mcMother = mcParticle->getMother();
        while (mcMother)
        {
          mothersPDG.push_back(abs(mcMother->getPDG()));
          if (index == 8) mothersPointers.push_back(mcMother);
          if (abs(mcMother->getPDG()) == 511) break;
          mcMother = mcMother -> getMother();
        }

        if (mothersPDG.size() == 0) return -2.0;

        //has associated mothers up to a B meson
        if (index == 10) return 1.0;

        // ----------------  Is D Meson in the decay chain  --------------------------------------

        bool isCharmedMesonInChain = false;
        if ((index == 6) && mothersPDG.size() > 1)
        {
          for (auto& iMCMotherPDG : mothersPDG) {
            if (std::find(charmMesons.begin(), charmMesons.end(), iMCMotherPDG) != charmMesons.end()) {
              isCharmedMesonInChain = true;
              break;
            }
          }
        }

        // ----------------  Is Charmed Baryon in the decay chain  --------------------------------

        bool isCharmedBaryonInChain = false;
        if ((index == 6 || index == 9) && mothersPDG.size() > 1)
        {
          for (auto& iMCMotherPDG : mothersPDG) {
            if (std::find(charmBaryons.begin(), charmBaryons.end(), iMCMotherPDG) != charmBaryons.end()) {
              isCharmedBaryonInChain = true;
              break;
            }
          }
        }

        // ----------------  Is neutral qqbar Meson in the decay chain  --------------------------------

        bool isQQbarMesonInChain = false;
        if ((index == 1 || index == 3 || index == 5 || index == 6 || index == 8) && mothersPDG.size() > 1)
        {
          for (auto& iMCMotherPDG : mothersPDG) {
            if (std::find(qqbarMesons.begin(), qqbarMesons.end(), iMCMotherPDG) != qqbarMesons.end()) {
              isQQbarMesonInChain = true;
              break;
            }
          }
        }

        // --------------  Is the Hadron a descendent of a Meson that conserves flavor  --------------------------

        bool isB0DaughterConservingFlavor = false;
        if ((index == 8) && mothersPDG.size() > 1)
        {
          if (std::find(flavorConservingMesons.begin(), flavorConservingMesons.end(),
                        mothersPDG.rbegin()[1]) != flavorConservingMesons.end()) {
            isB0DaughterConservingFlavor = true;
          }
        }

        // -----------------------------  Is the Hadron a single daugther of a tau ----- --------------------------

        bool isHadronSingleTauDaughter = false;
        if (index == 8 && mothersPDG.size() > 1 && mothersPDG.rbegin()[1] == 15)
        {
          int numberOfChargedDaughters = 0;
          for (auto& tauDaughter : mothersPointers.rbegin()[1] -> getDaughters()) {
            if (tauDaughter -> getCharge() != 0) numberOfChargedDaughters += 1;
          }
          if (numberOfChargedDaughters == 1) isHadronSingleTauDaughter = true;
        }

        //direct electron
        if (index == 0
            && mcPDG == Const::electron.getPDGCode()
            && mothersPDG[0] == 511)
        {
          return 1.0;
          //intermediate electron
        } else if (index == 1
                   && mcPDG == Const::electron.getPDGCode() && mothersPDG.size() > 1
                   && isQQbarMesonInChain == false)
        {
          return 1.0;
          //direct muon
        } else if (index == 2
                   && mcPDG == Const::muon.getPDGCode() && mothersPDG[0] == 511)
        {
          return 1.0;
          //intermediate muon
        } else if (index == 3
                   && mcPDG == Const::muon.getPDGCode() && mothersPDG.size() > 1
                   && isQQbarMesonInChain == false)
        {
          return 1.0;
          //KinLepton
        } else if (index == 4
                   && (mcPDG == Const::muon.getPDGCode() || mcPDG == Const::electron.getPDGCode()) && mothersPDG[0] == 511)
        {
          return 1.0;
          //IntermediateKinLepton
        } else if (index == 5
                   && (mcPDG == Const::muon.getPDGCode() || mcPDG == Const::electron.getPDGCode()) && mothersPDG.size() > 1
                   && isQQbarMesonInChain == false)
        {
          return 1.0;
          //kaon
        } else if (index == 6
                   && mcPDG == Const::kaon.getPDGCode() && isQQbarMesonInChain == false && (isCharmedMesonInChain == true || isCharmedBaryonInChain == true))
        {
          return 1.0;
          //slow pion
        } else if (index == 7
                   && mcPDG == Const::pion.getPDGCode() && mothersPDG.size() > 1 && mothersPDG[0] == 413 && mothersPDG[1] == 511)
        {
          return 1.0;
          //high momentum hadrons
        } else if (index == 8
                   && (mcPDG == Const::pion.getPDGCode() || mcPDG == Const::kaon.getPDGCode()) && isQQbarMesonInChain == false && (mothersPDG[0] == 511 || (mothersPDG.rbegin()[0] == 511
                       && (isB0DaughterConservingFlavor == true || isHadronSingleTauDaughter == true))))
        {
          return 1.0;
          //lambdas
        } else if (index == 9 && mcPDG == Const::Lambda.getPDGCode() && isCharmedBaryonInChain == true)
        {
          return 1.0;
        } else return 0.0;
      };
      return func;
    }

    Manager::FunctionPtr isRightCategory(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1) {
        B2FATAL("Wrong number of arguments (1 required) for meta function isRightCategory");
      }


      auto particleName = arguments[0];

      const std::vector<std::string> names = {    "Electron",             // 0
                                                  "IntermediateElectron", // 1
                                                  "Muon",                 // 2
                                                  "IntermediateMuon",     // 3
                                                  "KinLepton",            // 4
                                                  "IntermediateKinLepton",// 5
                                                  "Kaon",                 // 6
                                                  "SlowPion",             // 7
                                                  "FastHadron",             // 8
                                                  "KaonPion",             // 9
                                                  "MaximumPstar",         // 10
                                                  "FSC",                  // 11
                                                  "Lambda",               // 12
                                                  "mcAssociated"          // 13
                                             };

      unsigned index = find(names.begin(), names.end(), particleName) - names.begin();
      if (index == names.size()) {
        B2FATAL("isRightCategory: Not available category " << particleName <<
                ". The possibilities are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, KaonPion, MaximumPstar, FSC and Lambda");
      }

      auto func = [index](const Particle * particle) -> double {

        Particle* nullParticle = nullptr;
        double qTarget = particle -> getCharge();
        double qMC = Variable::isRestOfEventB0Flavor(nullParticle);

        const MCParticle* mcParticle = particle->getMCParticle();
        if (!mcParticle) return -2.0;

        int mcPDG = abs(mcParticle->getPDG());

        // ---------------------------- Mothers and Grandmothers ---------------------------------
        std::vector<int> mothersPDG;
        std::vector<const MCParticle*> mothersPointers;

        const MCParticle* mcMother = mcParticle->getMother();
        while (mcMother)
        {
          mothersPDG.push_back(abs(mcMother->getPDG()));
          if (index == 8 || index == 9) mothersPointers.push_back(mcMother);
          if (abs(mcMother->getPDG()) == 511) break;
          mcMother = mcMother->getMother();
        }

        if (mothersPDG.size() == 0) return -2.0;
        //has associated mothers up to a B meson
        if (index == 13) return 1.0;

        // ----------------  Is D Meson in the decay chain  --------------------------------------

        bool isCharmedMesonInChain = false;
        if ((index == 6) && mothersPDG.size() > 1)
        {
          for (auto& iMCMotherPDG : mothersPDG) {
            if (std::find(charmMesons.begin(), charmMesons.end(), iMCMotherPDG) != charmMesons.end()) {
              isCharmedMesonInChain = true;
              break;
            }
          }
        }

        // ----------------  Is Charmed Baryon in the decay chain  --------------------------------

        bool isCharmedBaryonInChain = false;
        if ((index == 6 || index == 12) && mothersPDG.size() > 1)
        {
          for (auto& iMCMotherPDG : mothersPDG) {
            if (std::find(charmBaryons.begin(), charmBaryons.end(), iMCMotherPDG) != charmBaryons.end()) {
              isCharmedBaryonInChain = true;
              break;
            }
          }
        }

        // ----------------  Is neutral qqbar Meson in the decay chain  --------------------------------

        bool isQQbarMesonInChain = false;
        if ((index == 1 || index == 3 || index == 5 || index == 6 || index == 8 || index == 11) && mothersPDG.size() > 1)
        {
          for (auto& iMCMotherPDG : mothersPDG) {
            if (std::find(qqbarMesons.begin(), qqbarMesons.end(), iMCMotherPDG) != qqbarMesons.end()) {
              isQQbarMesonInChain = true;
              break;
            }
          }
        }

        // --------------  Is the Hadron a descendent of a Meson that conserves flavor  --------------------------

        bool isB0DaughterConservingFlavor = false;
        if ((index == 8) && mothersPDG.size() > 1)
        {
          if (std::find(flavorConservingMesons.begin(), flavorConservingMesons.end(),
                        mothersPDG.rbegin()[1]) != flavorConservingMesons.end()) {
            isB0DaughterConservingFlavor = true;
          }

        }

        // -----------------------------  Is the Hadron a single daugther of a tau ----- --------------------------

        bool isHadronSingleTauDaughter = false;
        if (index == 8 && mothersPDG.size() > 1 && mothersPDG.rbegin()[1] == 15)
        {
          int numberOfChargedDaughters = 0;
          for (auto& tauDaughter : mothersPointers.rbegin()[1] -> getDaughters()) {
            if (tauDaughter -> getCharge() != 0) numberOfChargedDaughters += 1;
          }
          if (numberOfChargedDaughters == 1) isHadronSingleTauDaughter = true;
        }

        // ----------------------------  For KaonPion Category ------------------------------------
        bool haveKaonPionSameMother = false;
        // if KaonPion
        if (index == 9)
        {
          const MCParticle* mcSlowPionMother = nullptr;
          StoreObjPtr<ParticleList> SlowPionList("pi+:inRoe");
          Particle* targetSlowPion = nullptr;
          if (SlowPionList.isValid()) {
            double mcProbSlowPion = 0;
            for (unsigned int i = 0; i < SlowPionList->getListSize(); ++i) {
              Particle* pSlowPion = SlowPionList->getParticle(i);
              if (!pSlowPion) continue;
              if (pSlowPion -> hasExtraInfo("isRightCategory(SlowPion)")) {
                double probSlowPion = pSlowPion->getExtraInfo("isRightCategory(SlowPion)");
                if (probSlowPion > mcProbSlowPion) {
                  mcProbSlowPion = probSlowPion;
                  targetSlowPion = pSlowPion;
                }
              }
            }
            if (targetSlowPion != nullptr) {
              const MCParticle* mcSlowPion = targetSlowPion ->getMCParticle();
              //               SlowPion_q = targetSlowPion -> getCharge();
              if (mcSlowPion != nullptr && mcSlowPion->getMother() != nullptr
                  && abs(mcSlowPion->getPDG()) == Const::pion.getPDGCode() && abs(mcSlowPion->getMother()->getPDG()) == 413) {
                mcSlowPionMother = mcSlowPion->getMother();
              }
            }
          }

          if (std::find(mothersPointers.begin(), mothersPointers.end(), mcSlowPionMother) != mothersPointers.end())
            haveKaonPionSameMother = true;

        }

        // ----------------------------  For FastSlowCorrelated Category ----------------------------
        int FastParticlePDGMother = 0;
        double qFSC = 0;
        // FSC",
        if (index == 11)
        {
          StoreObjPtr<ParticleList> FastParticleList("pi+:inRoe");
          Particle* targetFastParticle = nullptr;
          if (FastParticleList.isValid()) {
            double mcProbFastest = 0;
            for (unsigned int i = 0; i < FastParticleList->getListSize(); ++i) {
              Particle* particlei = FastParticleList->getParticle(i);
              if (!particlei) continue;

              TLorentzVector momParticlei = labToCms(particlei -> get4Vector());
              if (momParticlei == momParticlei) {
                double probFastest = momParticlei.P();
                if (probFastest > mcProbFastest) {
                  mcProbFastest = probFastest;
                  targetFastParticle = particlei;
                }
              }
            }
            if (targetFastParticle != nullptr) {
              const MCParticle* mcFastParticle = targetFastParticle ->getMCParticle();
              //               FastParticle_q = targetFastParticle -> getCharge();
              if (mcFastParticle != nullptr && mcFastParticle->getMother() != nullptr) {
                FastParticlePDGMother = abs(mcFastParticle->getMother()->getPDG());
                qFSC = mcFastParticle->getCharge();
              }
            }
          }
        }

        // ------------------------------  Outputs  -----------------------------------
        if (index == 0 // Electron
            && qTarget == qMC && mcPDG == Const::electron.getPDGCode() && mothersPDG[0] == 511)
        {
          return 1.0;
        } else if (index == 1 // IntermediateElectron
                   && qTarget != qMC && mcPDG == Const::electron.getPDGCode() && mothersPDG.size() > 1
                   && isQQbarMesonInChain == false)
        {
          return 1.0;
        } else if (index == 2 // Muon
                   && qTarget == qMC && mcPDG == Const::muon.getPDGCode() && mothersPDG[0] == 511)
        {
          return 1.0;
        } else if (index == 3 // IntermediateMuon
                   && qTarget != qMC && mcPDG == Const::muon.getPDGCode() && mothersPDG.size() > 1
                   && isQQbarMesonInChain == false)
        {
          return 1.0;
        }  else if (index == 4 // KinLepton
                    && qTarget == qMC && (mcPDG == Const::electron.getPDGCode() || mcPDG == Const::muon.getPDGCode()) && mothersPDG[0] == 511)
        {
          return 1.0;
        }  else if (index == 5 // IntermediateKinLepton
                    && qTarget != qMC && (mcPDG == Const::electron.getPDGCode() || mcPDG == Const::muon.getPDGCode()) && mothersPDG.size() > 1
                    && isQQbarMesonInChain == false)
        {
          return 1.0;
        } else if (index == 6 && qTarget == qMC // Kaon
                   && mcPDG == Const::kaon.getPDGCode() && isQQbarMesonInChain == false && (isCharmedMesonInChain == true || isCharmedBaryonInChain == true))
        {
          return 1.0;
        } else if (index == 7 && qTarget != qMC // SlowPion
                   && mcPDG == Const::pion.getPDGCode() && mothersPDG.size() > 1 && mothersPDG[0] == 413 && mothersPDG[1] == 511)
        {
          return 1.0;
        } else if (index == 8 && qTarget == qMC // FastHadron
                   && (mcPDG == Const::pion.getPDGCode() || mcPDG == Const::kaon.getPDGCode()) && isQQbarMesonInChain == false && (mothersPDG[0] == 511 || (mothersPDG.rbegin()[0] == 511
                       && (isB0DaughterConservingFlavor == true || isHadronSingleTauDaughter == true))))
        {
          return 1.0;
        } else if (index == 9  && qTarget == qMC // KaonPion
                   && mcPDG == Const::kaon.getPDGCode() && haveKaonPionSameMother == true)
        {
          return 1.0;
        } else if (index == 10 && qTarget == qMC)   // MaximumPstar
        {
          return 1.0;
        } else if (index == Const::electron.getPDGCode() && qTarget != qMC && mothersPDG.size() > 1 && qFSC == qMC // "FSC"
                   && mcPDG == Const::pion.getPDGCode() && FastParticlePDGMother == 511 && isQQbarMesonInChain == false)
        {
          return 1.0;
        } else if (index == 12 && (particle->getPDGCode() / abs(particle->getPDGCode())) != qMC // Lambda
                   && mcPDG == Const::Lambda.getPDGCode() && isCharmedBaryonInChain == true)
        {
          return 1.0;
        } else {
          return 0.0;
        }
      };

      return func;
    }


    // List of available extrainfos used in QpOf, weightedQpOf and variableOfTarget.
    std::vector<std::string> availableExtraInfos = {     "isRightTrack(Electron)",             // 0
                                                         "isRightTrack(IntermediateElectron)", // 1
                                                         "isRightTrack(Muon)",                 // 2
                                                         "isRightTrack(IntermediateMuon)",     // 3
                                                         "isRightTrack(KinLepton)",            // 4
                                                         "isRightTrack(IntermediateKinLepton)",// 5
                                                         "isRightTrack(Kaon)",                 // 6
                                                         "isRightTrack(SlowPion)",             // 7
                                                         "isRightTrack(FastHadron)",             // 8
                                                         "isRightTrack(MaximumPstar)",         // 9
                                                         "isRightTrack(Lambda)",                // 10
                                                         "isRightCategory(Electron)",             // 11
                                                         "isRightCategory(IntermediateElectron)", // 12
                                                         "isRightCategory(Muon)",                 // 13
                                                         "isRightCategory(IntermediateMuon)",     // 14
                                                         "isRightCategory(KinLepton)",            // 15
                                                         "isRightCategory(IntermediateKinLepton)",// 16
                                                         "isRightCategory(Kaon)",                 // 17
                                                         "isRightCategory(SlowPion)",             // 18
                                                         "isRightCategory(FastHadron)",             // 19
                                                         "isRightCategory(MaximumPstar)",         // 20
                                                         "isRightCategory(Lambda)",                // 21
                                                         "isRightCategory(KaonPion)",             // 22
                                                         "isRightCategory(FSC)",                  // 23
                                                   };

    Manager::FunctionPtr QpOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 3) {
        B2FATAL("Wrong number of arguments (3 required) for meta function QpOf");
      }


      auto particleListName = arguments[0];
      auto outputExtraInfo = arguments[1];
      auto rankingExtraInfo = arguments[2];

      unsigned indexRanking = find(availableExtraInfos.begin(), availableExtraInfos.end(),
                                   rankingExtraInfo) - availableExtraInfos.begin();
      unsigned indexOutput  = find(availableExtraInfos.begin(), availableExtraInfos.end(),
                                   outputExtraInfo)  - availableExtraInfos.begin();

      if (indexRanking == availableExtraInfos.size()) {
        B2FATAL("QpOf: Not available category " << rankingExtraInfo <<
                ". The possibilities for isRightTrack() are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, MaximumPstar, and Lambda"
                <<
                ". The possibilities for isRightCategory() are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, KaonPion, MaximumPstar, FSC and Lambda");
      }

      if (indexOutput == availableExtraInfos.size()) {
        B2FATAL("QpOf: Not available category " << outputExtraInfo <<
                ". The possibilities for isRightTrack() are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, MaximumPstar, and Lambda"
                <<
                ". The possibilities for isRightCategory() are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, KaonPion, MaximumPstar, FSC and Lambda");
      }


      auto func = [particleListName, indexOutput, indexRanking](const Particle*) -> double {
        StoreObjPtr<ParticleList> ListOfParticles(particleListName);
        if (!ListOfParticles.isValid()) return 0;

        Particle* target = nullptr; //Particle selected as target
        double maximumTargetProb = 0; //Probability of being the target track from the track level
        for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i)
        {
          Particle* particlei = ListOfParticles->getParticle(i);
          if (!particlei) continue;

          double target_prob = 0;
          if (indexRanking == 9 || indexRanking == 20) { // MaximumPstar
            TLorentzVector momParticlei = labToCms(particlei -> get4Vector());
            if (momParticlei == momParticlei) {
              target_prob = momParticlei.P();
            }
          } else {
            if (particlei->hasExtraInfo(availableExtraInfos[indexRanking])) {
              target_prob = particlei->getExtraInfo(availableExtraInfos[indexRanking]);
            }
          }

          if (target_prob > maximumTargetProb) {
            maximumTargetProb = target_prob;
            target = particlei;
          }
        }

        // nothing found
        if (!target) return 0;

        double qTarget = 0; //Flavor of the track selected as target
        // Get the flavor of the track selected as target
        if (indexRanking == 10 || indexRanking == 21)   // Lambda
        {
          qTarget = (-1) * target->getPDGCode() / abs(target->getPDGCode());
          //     IntermediateElectron    IntermediateMuon        IntermediateKinLepton   SlowPion
        } else if (indexRanking == 1 || indexRanking == 3 || indexRanking == 5 || indexRanking == 7 ||
                   indexRanking == 12 || indexRanking == 14 || indexRanking == 16 || indexRanking == 18)
        {
          qTarget = (-1) * target -> getCharge();
        } else {
          qTarget = target -> getCharge();
        }

        //Get the probability of being right classified flavor from event level
        double prob = target -> getExtraInfo(availableExtraInfos[indexOutput]);

        //float r = abs(2 * prob - 1); //Definition of the dilution factor  */
        //return 0.5 * (qTarget * r + 1);
        return qTarget * prob;
      };
      return func;
    }

    Manager::FunctionPtr weightedQpOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 3) {
        B2FATAL("Wrong number of arguments (3 required) for meta function weightedQpOf");
      }

      //used by simple_flavor_tagger

      auto particleListName = arguments[0];
      auto outputExtraInfo  = arguments[1];
      auto rankingExtraInfo = arguments[2];


      unsigned indexRanking = find(availableExtraInfos.begin(), availableExtraInfos.end(),
                                   rankingExtraInfo) - availableExtraInfos.begin();
      unsigned indexOutput  = find(availableExtraInfos.begin(), availableExtraInfos.end(),
                                   outputExtraInfo)  - availableExtraInfos.begin();


      if (indexRanking == availableExtraInfos.size()) {
        B2FATAL("weightedQpOf: Not available category " << rankingExtraInfo <<
                ". The possibilities for isRightTrack() are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, MaximumPstar, and Lambda"
                <<
                ". The possibilities for isRightCategory() are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, KaonPion, MaximumPstar, FSC and Lambda");
      }

      if (indexOutput == availableExtraInfos.size()) {
        B2FATAL("weightedQpOf: Not available category " << outputExtraInfo <<
                ". The possibilities for isRightTrack() are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, MaximumPstar, and Lambda"
                <<
                ". The possibilities for isRightCategory() are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, KaonPion, MaximumPstar, FSC and Lambda");
      }

      auto func = [particleListName, indexOutput, indexRanking, rankingExtraInfo](const Particle*) -> double {

        StoreObjPtr<ParticleList> ListOfParticles(particleListName);
        if (!ListOfParticles) return 0;
        if (ListOfParticles->getListSize() == 0) return 0;


        auto compare = [rankingExtraInfo](const Particle * part1, const Particle * part2)-> bool {
          double info1 = 0;
          double info2 = 0;
          if (part1->hasExtraInfo(rankingExtraInfo)) info1 = part1->getExtraInfo(rankingExtraInfo);
          if (part2->hasExtraInfo(rankingExtraInfo)) info2 = part2->getExtraInfo(rankingExtraInfo);
          return (info1 > info2);
        };

        auto compareMomentum = [rankingExtraInfo](const Particle * part1, const Particle * part2)-> bool {
          double info1 = labToCms(part1 -> get4Vector()).P();
          double info2 = labToCms(part2 -> get4Vector()).P();
          return (info1 > info2);
        };

        std::vector<const Particle*> ParticleVector(ListOfParticles->getListSize());
        for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i)
        {
          ParticleVector[i] = ListOfParticles->getParticle(i);
        }

        if (indexRanking == 9 || indexRanking == 20)
          std::sort(ParticleVector.begin(), ParticleVector.end(), compareMomentum);  // MaximumPstar
        else
          std::sort(ParticleVector.begin(), ParticleVector.end(), compare);


        double final_value = 0.0;
        if (ParticleVector.size() != 0) final_value = 1.0;

        //Loop over K+ vector until 3 or empty
        int Limit = min(3, int(ParticleVector.size()));
        double val1 = 1.0;
        double val2 = 1.0;
        for (int i = 0; i < Limit; ++i)
        {
          if (ParticleVector[i]->hasExtraInfo(availableExtraInfos[indexOutput])) {
            double flavor = 0.0;
            if (indexRanking == 10 || indexRanking == 21) { // Lambda
              flavor = - copysign(1, ParticleVector[i]->getPDGCode());
              //     IntermediateElectron    IntermediateMuon        IntermediateKinLepton   SlowPion
            } else if (indexRanking == 1 || indexRanking == 3 || indexRanking == 5 || indexRanking == 7 ||
                       indexRanking == 12 || indexRanking == 14 || indexRanking == 16 || indexRanking == 18) {
              flavor = - ParticleVector[i]->getCharge();
            } else {
              flavor = + ParticleVector[i]->getCharge();
            }

            double p = ParticleVector[i]->getExtraInfo(availableExtraInfos[indexOutput]);
            //                 B2INFO("Right Track:" << ParticleVector[i]->getExtraInfo(availableExtraInfos[indexRanking]));
            //                 B2INFO("Right Cat:" << ParticleVector[i]->getExtraInfo(availableExtraInfos[indexOutput]));
            double qp = (flavor * p);
            val1 *= 1 + qp;
            val2 *= 1 - qp;
          }
        }
        final_value = (val1 - val2) / (val1 + val2);

        return final_value;
      };
      return func;
    }

    Manager::FunctionPtr variableOfTarget(const std::vector<std::string>& arguments)
    {

      if (arguments.size() != 3)
        B2FATAL("Wrong number of arguments (3 required) for meta function variableOfTarget");

      std::string particleListName = arguments[0];
      std::string inputVariable = arguments[1];
      std::string rankingExtraInfo = arguments[2];

      int indexRanking = -1;

      for (unsigned i = 0; i < availableExtraInfos.size(); ++i) {
        if (rankingExtraInfo == availableExtraInfos[i]) {indexRanking = i; break;}
      }

      if (indexRanking == -1) {
        B2FATAL("variableOfTarget: category " << rankingExtraInfo << "not available" <<
                ". The possibilities for isRightTrack() are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, MaximumPstar, and Lambda"
                <<
                ". The possibilities for isRightCategory() are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, KaonPion, MaximumPstar, FSC and Lambda");
      }


      auto func = [particleListName, inputVariable, indexRanking](const Particle*) -> double {
        StoreObjPtr<ParticleList> ListOfParticles(particleListName);
        if (!ListOfParticles.isValid()) return realNaN;

        Particle* target = nullptr; //Particle selected as target

        double maximumTargetProb = 0; //Probability of being the target track from the track level
        for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i)
        {
          Particle* particlei = ListOfParticles->getParticle(i);
          if (!particlei) continue;

          double target_prob = 0;
          if (indexRanking == 9 || indexRanking == 20) { // MaximumPstar
            TLorentzVector momParticlei = labToCms(particlei->get4Vector());
            if (momParticlei == momParticlei) {
              target_prob = momParticlei.P();
            }
          } else {
            if (particlei->hasExtraInfo(availableExtraInfos[indexRanking])) {
              target_prob = particlei->getExtraInfo(availableExtraInfos[indexRanking]);
            }
          }
          if (target_prob > maximumTargetProb) {
            maximumTargetProb = target_prob;
            target = particlei;
          }
        }

        // no target found
        if (!target) return realNaN;

        Variable::Manager& manager = Variable::Manager::Instance();
        double output = manager.getVariable(inputVariable)->function(target);
        return output;
      };
      return func;
    }

    Manager::FunctionPtr hasTrueTarget(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1) {
        B2FATAL("Wrong number of arguments (1 required) for meta function hasTrueTarget");
      }

      auto categoryName = arguments[0];
      auto func = [categoryName](const Particle*) -> double {
        if (!(categoryName == "Electron" || categoryName == "IntermediateElectron" || categoryName == "Muon" ||  categoryName == "IntermediateMuon" || categoryName == "KinLepton" || categoryName == "IntermediateKinLepton" || categoryName == "Kaon"
        || categoryName == "SlowPion" ||  categoryName == "FastHadron" || categoryName == "KaonPion" || categoryName == "Lambda" || categoryName == "MaximumPstar" ||  categoryName == "FSC"))
        {
          B2FATAL("hasTrueTarget: Not available category" << categoryName <<
          ". The possibilities for the category name are \nElectron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, KaonPion, MaximumPstar, FSC and Lambda");
          return 0.0;
        }

        std::string particleListName;
        std::string trackTargetName = categoryName;

        if (categoryName == "Electron" || categoryName == "IntermediateElectron") particleListName = "e+:inRoe";
        else if (categoryName == "Muon" ||  categoryName ==  "IntermediateMuon" || categoryName ==  "KinLepton" || categoryName ==  "IntermediateKinLepton") particleListName = "mu+:inRoe";
        else if (categoryName == "Kaon" || categoryName ==  "KaonPion") {particleListName = "K+:inRoe"; trackTargetName = "Kaon";}
        else if (categoryName == "Lambda") particleListName = "Lambda0:inRoe";
        else particleListName = "pi+:inRoe";

        if (categoryName == "FSC") trackTargetName = "SlowPion";

        StoreObjPtr<ParticleList> ListOfParticles(particleListName);
        if (!ListOfParticles.isValid()) return realNaN;

        Variable::Manager& manager = Variable::Manager::Instance();

        double output = 0;
        bool particlesHaveMCAssociated = false;
        int nTargets = 0;
        for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i)
        {
          Particle* iParticle = ListOfParticles->getParticle(i);
          if (!iParticle) continue;

          double targetFlag = 0;
          if (categoryName == "MaximumPstar") {
            targetFlag = manager.getVariable("hasHighestProbInCat(pi+:inRoe, isRightTrack(MaximumPstar))")-> function(iParticle);
          } else {
            targetFlag = manager.getVariable("isRightTrack(" + trackTargetName + ")")-> function(iParticle);
          }
          if (targetFlag != -2) particlesHaveMCAssociated = true;
          if (targetFlag == 1) {
            ++nTargets;
          }
        }

        if (!particlesHaveMCAssociated) output = realNaN;
        if (nTargets > 0) output = 1;

        // if (nTargets > 1); B2INFO("The Category " << categoryName << " has " <<  std::to_string(nTargets) << " target tracks.");

        return output;
      };
      return func;
    }

    Manager::FunctionPtr isTrueCategory(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1) {
        B2FATAL("Wrong number of arguments (1 required) for meta function isTrueCategory");
      }

      auto categoryName = arguments[0];
      auto func = [categoryName](const Particle*) -> double {
        if (!(categoryName == "Electron" || categoryName == "IntermediateElectron" || categoryName == "Muon" ||  categoryName == "IntermediateMuon" || categoryName == "KinLepton" || categoryName == "IntermediateKinLepton" || categoryName == "Kaon"
        || categoryName == "SlowPion" ||  categoryName == "FastHadron" || categoryName == "KaonPion" || categoryName == "Lambda" || categoryName == "MaximumPstar" ||  categoryName == "FSC"))
        {
          B2FATAL("isTrueCategory: Not available category" << categoryName <<
          ". The possibilities for the category name are \nElectron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, KaonPion, MaximumPstar, FSC and Lambda");
          return 0.0;
        }

        std::string particleListName;
        std::string trackTargetName = categoryName;

        if (categoryName == "Electron" || categoryName == "IntermediateElectron") particleListName = "e+:inRoe";
        else if (categoryName == "Muon" ||  categoryName ==  "IntermediateMuon" || categoryName ==  "KinLepton" || categoryName ==  "IntermediateKinLepton") particleListName = "mu+:inRoe";
        else if (categoryName == "Kaon" || categoryName ==  "KaonPion") {particleListName = "K+:inRoe"; trackTargetName = "Kaon";}
        else if (categoryName == "Lambda") particleListName = "Lambda0:inRoe";
        else particleListName = "pi+:inRoe";

        if (categoryName == "FSC") trackTargetName = "SlowPion";

        StoreObjPtr<ParticleList> ListOfParticles(particleListName);
        if (!ListOfParticles.isValid()) return realNaN;

        std::vector<Particle*> targetParticles;
        std::vector<Particle*> targetParticlesCategory;
        Variable::Manager& manager = Variable::Manager::Instance();


        double output = 0;
        int nTargets = 0;
        for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i)
        {
          Particle* iParticle = ListOfParticles->getParticle(i);
          if (!iParticle) continue;

          double targetFlag = 0;
          if (categoryName == "MaximumPstar") {
            targetFlag = manager.getVariable("hasHighestProbInCat(pi+:inRoe, isRightTrack(MaximumPstar))")-> function(iParticle);
          } else {
            targetFlag = manager.getVariable("isRightTrack(" + trackTargetName + ")")-> function(iParticle);
          }
          if (targetFlag == 1) {
            targetParticles.push_back(iParticle);
          }
        }

        for (const auto& targetParticle : targetParticles)
        {
          double isTargetOfRightCategory = manager.getVariable("isRightCategory(" +  categoryName + ")")-> function(targetParticle);
          if (isTargetOfRightCategory == 1) {
            output = 1;
            nTargets += 1; targetParticlesCategory.push_back(targetParticle);
          } else if (isTargetOfRightCategory == -2 && output != 1)
            output = realNaN;
        }

        /*            if (nTargets > 1) {
                      B2INFO("The Category " << categoryName << " has " <<  std::to_string(nTargets) << " target tracks.");
                      for (auto& iTargetParticlesCategory : targetParticlesCategory) {
                      const MCParticle* MCp = iTargetParticlesCategory -> getMCParticle();

                      RelationVector<Particle> mcRelations = MCp->getRelationsFrom<Particle>();
                      if (mcRelations.size() > 1) B2WARNING("MCparticle is related to two particles");

                      B2INFO("MCParticle has pdgCode = " << MCp -> getPDG() << ", MCMother has pdgCode = " << MCp-> getMother() -> getPDG() << " and " <<
                      MCp-> getMother() -> getNDaughters() << " daughters.");

                      for (auto& iDaughter : MCp->getMother() -> getDaughters()) B2INFO("iDaughter PDGCode = " << iDaughter -> getPDG());
                      }
                      }*/

        return output;
      };
      return func;
    }

    Manager::FunctionPtr qrOutput(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments for meta function qrOutput");

      std::string combinerMethod = arguments[0];
      auto func = [combinerMethod](const Particle * particle) -> double {

        double output = realNaN;
        auto* flavorTaggerInfo = particle->getRelatedTo<FlavorTaggerInfo>();

        if (flavorTaggerInfo && flavorTaggerInfo->getUseModeFlavorTagger() == "Expert")
          output = flavorTaggerInfo->getMethodMap(combinerMethod)->getQrCombined();

        return output;
      };
      return func;
    }

    Manager::FunctionPtr qOutput(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments for meta function qOutput");

      std::string combinerMethod = arguments[0];
      auto func = [combinerMethod](const Particle * particle) -> double {

        double output = realNaN;
        auto* flavorTaggerInfo = particle->getRelatedTo<FlavorTaggerInfo>();

        if (flavorTaggerInfo && flavorTaggerInfo->getUseModeFlavorTagger() == "Expert")
          output = TMath::Sign(1, flavorTaggerInfo->getMethodMap(combinerMethod)->getQrCombined());

        return output;
      };
      return func;
    }

    Manager::FunctionPtr rBinBelle(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments for meta function rBinBelle");


      std::string combinerMethod = arguments[0];
      auto func = [combinerMethod](const Particle * particle) -> double {

        int output = std::numeric_limits<int>::quiet_NaN();
        auto* flavorTaggerInfo = particle->getRelatedTo<FlavorTaggerInfo>();

        if (flavorTaggerInfo && flavorTaggerInfo->getUseModeFlavorTagger() == "Expert")
        {
          double r = std::abs(flavorTaggerInfo->getMethodMap(combinerMethod)->getQrCombined());
          if (r < 0.1) output = 0;
          if (r > 0.1 && r < 0.25) output = 1;
          if (r > 0.25 && r < 0.5) output = 2;
          if (r > 0.5 && r < 0.625) output = 3;
          if (r > 0.625 && r < 0.75) output = 4;
          if (r > 0.75 && r < 0.875) output = 5;
          if (r > 0.875 && r < 1.10) output = 6;
        }

        return output;
      };
      return func;
    }

    Manager::FunctionPtr qpCategory(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments for meta function qpCategory");

      std::string categoryName = arguments[0];
      auto func = [categoryName](const Particle * particle) -> double {

        double output = realNaN;
        auto* flavorTaggerInfo = particle->getRelatedTo<FlavorTaggerInfo>();

        if (flavorTaggerInfo && flavorTaggerInfo->getUseModeFlavorTagger() == "Expert")
        {
          std::map<std::string, float> iQpCategories = flavorTaggerInfo->getMethodMap("FBDT")->getQpCategory();
          if (iQpCategories.find(categoryName) != iQpCategories.end()) output = iQpCategories.at(categoryName);
          else if (iQpCategories.size() != 0) B2FATAL("qpCategory: Category with name " << categoryName
            << " not found. Check the official category names or if this category is included in the flavor tagger categories list.");
        }
        return output;
      };
      return func;
    }

    Manager::FunctionPtr isTrueFTCategory(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments for meta function isTrueFTCategory");

      std::string categoryName = arguments[0];
      auto func = [categoryName](const Particle * particle) -> double {

        double output = realNaN;
        auto* flavorTaggerInfo = particle->getRelatedTo<FlavorTaggerInfo>();

        if (flavorTaggerInfo && flavorTaggerInfo->getUseModeFlavorTagger() == "Expert")
        {
          std::map<std::string, float> iIsTrueCategories = flavorTaggerInfo->getMethodMap("FBDT")->getIsTrueCategory();
          if (iIsTrueCategories.find(categoryName) != iIsTrueCategories.end()) output = iIsTrueCategories.at(categoryName);
          else if (iIsTrueCategories.size() != 0) B2FATAL("isTrueFTCategory: Category with name " << categoryName
            << " not found. Check the official category names or if this category is included in the flavor tagger categories list.");
        }

        return output;
      };
      return func;
    }

    Manager::FunctionPtr hasTrueTargets(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments for meta function hasTrueTargets");

      std::string categoryName = arguments[0];
      auto func = [categoryName](const Particle * particle) -> double {

        double output = realNaN;
        auto* flavorTaggerInfo = particle->getRelatedTo<FlavorTaggerInfo>();

        if (flavorTaggerInfo &&  flavorTaggerInfo->getUseModeFlavorTagger() == "Expert")
        {
          std::map<std::string, float> iHasTrueTargets = flavorTaggerInfo->getMethodMap("FBDT")->getHasTrueTarget();
          if (iHasTrueTargets.find(categoryName) != iHasTrueTargets.end()) output = iHasTrueTargets.at(categoryName);
          else if (iHasTrueTargets.size() != 0) B2FATAL("hasTrueTargets: Category with name " << categoryName
            << " not found. Check the official category names or if this category is included in the flavor tagger categories list.");
        }

        return output;
      };
      return func;
    }

    VARIABLE_GROUP("Flavor Tagger Variables");

    REGISTER_VARIABLE("pMissTag", momentumMissingTagSide,  "Calculates the missing Momentum for a given particle on the tag side.");
    REGISTER_VARIABLE("cosTPTO"  , cosTPTO , "cosine of angle between thrust axis of given particle and thrust axis of ROE");
    REGISTER_VARIABLE("lambdaFlavor", lambdaFlavor,  "1.0 if Lambda0, -1.0 if Anti-Lambda0, 0.0 else.");
    REGISTER_VARIABLE("isLambda", isLambda,  "1.0 if MCLambda0, 0.0 else.");
    REGISTER_VARIABLE("lambdaZError", lambdaZError,  "Returns the variance of the z-component of the decay vertex.");
    REGISTER_VARIABLE("momentumOfSecondDaughter", momentumOfSecondDaughter,
                      "Returns the Momentum of second daughter if existing, else 0.");
    REGISTER_VARIABLE("momentumOfSecondDaughterCMS", momentumOfSecondDaughterCMS,
                      "Returns the Momentum of second daughter if existing in CMS, else 0.");
    REGISTER_VARIABLE("chargeTimesKaonLiklihood", chargeTimesKaonLiklihood,
                      "Returns the q*(highest PID_Likelihood for Kaons), else 0.");
    REGISTER_VARIABLE("ptTracksRoe", transverseMomentumOfChargeTracksInRoe,
                      "Returns the transverse momentum of all charged tracks if there exists a ROE for the given particle, else 0.");
    REGISTER_VARIABLE("NumberOfKShortsInRoe", NumberOfKShortsInRoe,
                      "Returns the number of K_S0 in the rest of event. The particleList K_S0:inRoe is required");

    REGISTER_VARIABLE("isInElectronOrMuonCat", isInElectronOrMuonCat,
                      "Returns 1.0 if the particle has been selected as target in the Muon or Electron Category, 0.0 else.");

    REGISTER_VARIABLE("isMajorityInRestOfEventFromB0", isMajorityInRestOfEventFromB0,
                      "[Eventbased] Check if the majority of the tracks in the current RestOfEvent are from a B0.");
    REGISTER_VARIABLE("isMajorityInRestOfEventFromB0bar", isMajorityInRestOfEventFromB0bar,
                      "[Eventbased] Check if the majority of the tracks in the current RestOfEvent are from a B0bar.");
    REGISTER_VARIABLE("hasRestOfEventTracks", hasRestOfEventTracks,
                      "Returns the amount of tracks in the RestOfEvent related to the given Particle. -2 If ROE is empty.");
    REGISTER_VARIABLE("isRelatedRestOfEventB0Flavor", isRelatedRestOfEventB0Flavor,
                      "-1 (1) if the RestOfEvent related to the given Particle is related to a B0bar (B0). The MCError bit of Breco has to be 0, 1, 2, 16 or 1024. The output of the variable is 0 otherwise. If one Particle in the Rest of Event is found to belong the reconstructed B0, the output is -2(2) for a B0bar (B0) on the reco side.");
    REGISTER_VARIABLE("qrCombined", isRestOfEventB0Flavor,
                      "[Eventbased] -1 (1) if current RestOfEvent is related to a B0bar (B0). The MCError bit of Breco has to be 0, 1, 2, 16 or 1024. The output of the variable is 0 otherwise. If one Particle in the Rest of Event is found to belong the reconstructed B0, the output is -2(2) for a B0bar (B0) on the reco side.");
    REGISTER_VARIABLE("ancestorHasWhichFlavor", ancestorHasWhichFlavor,
                      "checks the decay chain of the given particle upwards up to the Y(4S) resonance.Output is 0 (1) if an ancestor is found to be a B0bar (B0), if not -2.");
    REGISTER_VARIABLE("B0mcErrors", B0mcErrors, "mcErrors MCMatching Flag on the reconstructed B0_cp.");
    REGISTER_VARIABLE("isRelatedRestOfEventMajorityB0Flavor", isRelatedRestOfEventMajorityB0Flavor,
                      " 0 (1) if the majority of tracks and clusters of the RestOfEvent related to the given Particle are related to a B0bar (B0).");
    REGISTER_VARIABLE("isRestOfEventMajorityB0Flavor", isRestOfEventMajorityB0Flavor,
                      "0 (1) if the majority of tracks and clusters of the current RestOfEvent are related to a B0bar (B0).");
    REGISTER_VARIABLE("mcFlavorOfOtherB", mcFlavorOfOtherB,
                      "Returns the MC flavor (+-1) of the accompaning tag-side B meson if the given particle is a correctly MC-matched B candidate. It returns 0 else. \n"
                      "In other words, this variable checks the generated flavor of the other MC Upsilon(4S) daughter.");

    VARIABLE_GROUP("Flavor Tagger MetaFunctions")

    REGISTER_VARIABLE("BtagToWBosonVariables(requestedVariable)", BtagToWBosonVariables,
                      "FlavorTagging:[Eventbased] Kinematical variables (recoilMass, pMissCMS, cosThetaMissCMS or EW90) assuming a semileptonic decay with the given particle as target.");
    REGISTER_VARIABLE("KaonPionVariables(requestedVariable)"  , KaonPionVariables ,
                      " Kinematical variables for KaonPion category (cosKaonPion or HaveOpositeCharges)");
    REGISTER_VARIABLE("FSCVariables(requestedVariable)", FSCVariables,
                      "FlavorTagging:[Eventbased] Kinematical variables for FastSlowCorrelated category (pFastCMS, cosSlowFast, SlowFastHaveOpositeCharges, or cosTPTOFast).");
    REGISTER_VARIABLE("hasHighestProbInCat(particleListName, extraInfoName)", hasHighestProbInCat,
                      "Returns 1.0 if the given Particle is classified as target track, i.e. if it has the highest target track probability in particlelistName. The probability is accessed via extraInfoName.");
    REGISTER_VARIABLE("HighestProbInCat(particleListName, extraInfoName)", HighestProbInCat,
                      "Returns the highest target track probability value for the given category");

    REGISTER_VARIABLE("isRightTrack(particleName)", isRightTrack,
                      "Checks if the given Particle was really from a B. 1.0 if true otherwise 0.0");
    REGISTER_VARIABLE("isRightCategory(particleName)", isRightCategory,
                      "FlavorTagging: returns 1 if the class track by particleName category has the same flavor as the MC target track 0 else also if there is no target track");
    REGISTER_VARIABLE("QpOf(particleListName, outputExtraInfo, rankingExtraInfo)", QpOf,
                      "FlavorTagging: [Eventbased] Returns the q*p value for a given list (argument[0]), where p is the probability of a category stored as extraInfo (argument[1]).\n"
                      "The particle is selected after ranking according to a flavor tagging extraInfo (argument[2]).");
    REGISTER_VARIABLE("weightedQpOf(particleListName, outputExtraInfo, rankingExtraInfo)", weightedQpOf,
                      "FlavorTagging: [Eventbased] Returns the weighted q*p value for a given list (argument[0]), where p is the probability of a category stored as extraInfo (argument[1]).\n"
                      "The particles in the list are ranked according to a flavor tagging extraInfo (argument[2]). \n"
                      "The values for the three top particles is combined into an effective (weighted) output.");
    REGISTER_VARIABLE("variableOfTarget(particleListName, inputVariable, rankingExtraInfo)", variableOfTarget,
                      "FlavorTagging: [Eventbased] Returns the value of an input variable (argument[1]) for a particle selected from the given list (argument[0]).\n"
                      "The particles are ranked according to a flavor tagging extraInfo (argument[2]).");

    REGISTER_VARIABLE("hasTrueTarget(categoryName)", hasTrueTarget,
                      "Returns 1 if the given category has a target. 0 Else.");
    REGISTER_VARIABLE("isTrueCategory(categoryName)", isTrueCategory,
                      "Returns 1 if the given category tags the B0 MC flavor correctly. 0 Else.");
    REGISTER_VARIABLE("qrOutput(combinerMethod)", qrOutput,
                      "Returns the output of the flavorTagger for the given combinerMethod. The default methods are 'FBDT' or 'FANN'.");
    REGISTER_VARIABLE("qOutput(combinerMethod)", qOutput,
                      "Returns the flavor tag q output of the flavorTagger for the given combinerMethod. The default methods are 'FBDT' or 'FANN'.");
    REGISTER_VARIABLE("rBinBelle(combinerMethod)", rBinBelle,
                      "Returns the corresponding r (dilution) bin according to the Belle binning for the given combinerMethod. The default methods are 'FBDT' or 'FANN'.");
    REGISTER_VARIABLE("qpCategory(categoryName)", qpCategory,
                      "Returns the output q (charge of target track) times p (probability that this is the right category) of the category with the given name. The allowed categories are the official Flavor Tagger Category Names.");
    REGISTER_VARIABLE("isTrueFTCategory(categoryName)", isTrueFTCategory,
                      "Returns 1 if the target particle (checking the decay chain) of the category with the given name is found in the mc Particles, and if it provides the right Flavor. The allowed categories are the official Flavor Tagger Category Names.");
    REGISTER_VARIABLE("hasTrueTargets(categoryName)", hasTrueTargets,
                      "Returns 1 if target particles (checking only the decay chain) of the category with the given name is found in the mc Particles. The allowed categories are the official Flavor Tagger Category Names.");
  }
}
