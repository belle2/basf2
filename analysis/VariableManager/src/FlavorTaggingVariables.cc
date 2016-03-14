/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Gelb                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/FlavorTaggingVariables.h>
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
#include <analysis/ContinuumSuppression/Thrust.h>
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

    //   ############################################## FlavorTagger Variables   ###############################################

    // Track Level Variables ---------------------------------------------------------------------------------------------------

    double momentumMissingTagSide(const Particle*)
    {
      TLorentzVector trackiCMSVec;
      TLorentzVector roeCMSVec;
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (roe.isValid()) {
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
      }
      double missMom = -roeCMSVec.P();
      return missMom ;
    }

    double cosTPTO(const Particle* part)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      PCmsLabTransform T;
      std::vector<TVector3> p3_cms_roe;
      TVector3 thrustO;
      static const double P_MAX(3.2);

      double result = 0 ;

      if (roe.isValid()) {

        // The following calculation of the thrust axis has been copied and modified
        // from analysis/ContinuumSuppression/src/ContinuumSuppression.cc
        // At some point this has to be updated!

        // Charged tracks
        //
        const auto& roeTracks = roe->getTracks();
        for (auto& track : roeTracks) {
          if (track == nullptr) continue;
          // TODO: Add helix and KVF with IpProfile once available. Port from L163-199 of:
          // /belle/b20090127_0910/src/anal/ekpcontsuppress/src/ksfwmoments.cc
          const Const::ChargedStable charged = track->getRelated<PIDLikelihood>()->getMostLikely();
          Particle particle(track, charged);
          if (particle.getParticleType() == Particle::c_Track) {
            TLorentzVector p_cms = T.rotateLabToCms() * particle.get4Vector();
            if (p_cms != p_cms) continue;
            if (p_cms.Rho() > P_MAX) continue;
            p3_cms_roe.push_back(p_cms.Vect());
          }
        }

        // ECLCluster -> Gamma
        //
        const auto& roeECLClusters = roe->getECLClusters();
        for (auto& cluster : roeECLClusters) {
          if (cluster == nullptr) continue;
          if (cluster->isNeutral()) {
            // Create particle from ECLCluster with gamma hypothesis
            Particle particle(cluster);
            TLorentzVector p_lab = particle.get4Vector();
            if (p_lab != p_lab) continue;
            if (p_lab.Rho() < 0.05) continue;
            TLorentzVector p_cms = T.rotateLabToCms() * p_lab;
            if (p_cms != p_cms) continue;
            if (p_cms.Rho() > P_MAX) continue;
            p3_cms_roe.push_back(p_cms.Vect());
          }
        }

        const auto& roeKLMClusters = roe->getKLMClusters();
        for (auto& cluster : roeKLMClusters) {
          if (cluster == nullptr) continue;
          if (!(cluster -> getAssociatedTrackFlag()) && !(cluster -> getAssociatedEclClusterFlag())) {
            // Create particle from KLMCluster with K0_L hypothesis
            Particle particle(cluster);
            TLorentzVector p_lab = particle.get4Vector();
            if (p_lab != p_lab) continue;
            if (p_lab.Rho() < 0.05) continue;
            TLorentzVector p_cms = T.rotateLabToCms() * p_lab;
            if (p_cms != p_cms) continue;
            if (p_cms.Rho() > P_MAX) continue;
            p3_cms_roe.push_back(p_cms.Vect());
          }
        }

        thrustO  = thrust(p3_cms_roe.begin() , p3_cms_roe.end() , SelfFunc(TVector3()));
        const TVector3 pAxis = PCmsLabTransform::labToCms(part->get4Vector()).Vect();
        if (pAxis == pAxis) result = fabs(cos(pAxis.Angle(thrustO)));

      }
      return result;
    }

    double lambdaFlavor(const Particle* particle)
    {
      if (particle->getPDGCode() == 3122) return 1.0; //Lambda0
      else if (particle->getPDGCode() == -3122) return -1.0; //Anti-Lambda0
      else return 0.0;
    }

    double isLambda(const Particle* particle)
    {
      const MCParticle* mcparticle = particle->getRelatedTo<MCParticle>();
      // if (mcparticle ==nullptr)
      //  return 0.0;
      if (mcparticle != nullptr) {
        if (mcparticle->getPDG() == 3122) return 1.0; //Lambda0
        else if (mcparticle->getPDG() == -3122) return 1.0; //Anti-Lambda0
        else return 0.0;
      } else return 0.0;
    }

    double lambdaZError(const Particle* particle)
    {
      //This is a simplisitc hack. But I see no other way to get that information.
      //Should be removed if worthless
      TMatrixFSym ErrorPositionMatrix = particle->getVertexErrorMatrix();
      double zError = ErrorPositionMatrix[2][2];
      return zError;
    }

    double momentumOfSecondDaughter(const Particle* part)
    {
      if (part->getDaughter(1) == nullptr) return 0.0;
      else {
        return part->getDaughter(1)->getP();
      }
    }

    double momentumOfSecondDaughterCMS(const Particle* part)
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
      double maximumKaonid = 0;
      double maximum_charge = 0;
      StoreObjPtr<ParticleList> KaonList("K+:KaonROE");
      if (KaonList.isValid()) {
        for (unsigned int i = 0; i < KaonList->getListSize(); ++i) {
          Particle* p = KaonList->getParticle(i);
          double Kid = p->getRelatedTo<PIDLikelihood>()->getProbability(Const::kaon, Const::pion);
          if (Kid > maximumKaonid) {
            maximumKaonid = Kid;
            maximum_charge = p->getCharge();
          }
        }
      }
      return maximumKaonid * maximum_charge;
    }

    double transverseMomentumOfChargeTracksInRoe(const Particle* part)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");

      double sum = 0.0;

      if (roe.isValid()) {
        for (const auto& track : roe->getTracks()) {
          if (part->getTrack() == track) continue;
          if (track == nullptr) continue;
          const Const::ChargedStable charged = track->getRelated<PIDLikelihood>()->getMostLikely();
          if (track->getTrackFitResult(charged) == nullptr) continue;
          double pt = track->getTrackFitResult(charged)->getTransverseMomentum();
          if (pt == pt) sum += sqrt(pt * pt);
        }
      }
      return sum;

    }

//     Event Level Variables --------------------------------------------------------------------------------------------

    double isInElectronOrMuonCat(const Particle* particle)
    {

      StoreObjPtr<ParticleList> MuonList("mu+:MuonROE");
      StoreObjPtr<ParticleList> ElectronList("e+:ElectronROE");

      const Track* trackTargetMuon = nullptr;
      const Track* trackTargetElectron = nullptr;

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
      if (particle->getTrack() == trackTargetMuon || particle->getTrack() == trackTargetElectron) {
        return 1.0;
      } else return 0.0;
    }

//     Target Variables --------------------------------------------------------------------------------------------------

    double isMajorityInRestOfEventFromB0(const Particle*)
    {
      int vote = 0;
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (roe.isValid()) {
        for (auto& track : roe->getTracks()) {
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
      }
      return vote > 0;
    }

    double isMajorityInRestOfEventFromB0bar(const Particle*)
    {
      int vote = 0;
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (roe.isValid()) {
        for (auto& track : roe->getTracks()) {
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
      }
      return vote < 0;
    }

    double isRestOfEventEmpty(const Particle* part)
    {
      const RestOfEvent* roe = part->getRelatedTo<RestOfEvent>();
      float ObjectsInROE = 0; //Flavor of B
      if (roe != nullptr) {
        if (roe-> getNTracks() != 0) {
          ObjectsInROE++;
//       } else if (roe-> getNECLClusters() != 0) {
//         ObjectsInROE++;
//       } else if (roe-> getNKLMClusters() != 0) {
//         ObjectsInROE++;
        }
      }
      if (ObjectsInROE > 0) {
        return ObjectsInROE;
      } else return -2;
    }

    double isRelatedRestOfEventB0Flavor(const Particle* part)
    {
      const RestOfEvent* roe = part->getRelatedTo<RestOfEvent>();

      float OutputB0tagQ = -2;

      if (roe != nullptr) {
        const Particle* Bcp = roe->getRelated<Particle>();
        const MCParticle* BcpMC = roe->getRelated<Particle>()->getRelatedTo<MCParticle>();

        int MCMatchingError = MCMatching::getMCErrors(Bcp, BcpMC);

        if (MCMatchingError == 0 || MCMatchingError == 1) {
          const MCParticle* Y4S = BcpMC->getMother();
          StoreArray<MCParticle> AllMCParticles;
          unsigned numParticles = AllMCParticles.getEntries();

          if (AllMCParticles.isValid()) {
            for (unsigned i = 0; i < numParticles; ++i) {
              const MCParticle* newParticle = AllMCParticles[i];
              const MCParticle* newMother = newParticle->getMother();

              if (newMother != nullptr) {
                if (newMother == Y4S &&  newParticle != BcpMC) {
                  if (newParticle -> getPDG() == 511) OutputB0tagQ = 1;
                  else OutputB0tagQ = 0;
                }
              }
            }
          }
        }
      }
      return OutputB0tagQ;
    }

    double isRestOfEventB0Flavor(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");

      float OutputB0tagQ = -2;

      if (roe.isValid()) {
        const Particle* Bcp = roe->getRelated<Particle>();
        const MCParticle* BcpMC = roe->getRelated<Particle>()->getRelatedTo<MCParticle>();

        int MCMatchingError = MCMatching::getMCErrors(Bcp, BcpMC);

        if (MCMatchingError == 0 || MCMatchingError == 1) {
          const MCParticle* Y4S = BcpMC->getMother();
          StoreArray<MCParticle> AllMCParticles;
          unsigned numParticles = AllMCParticles.getEntries();

          if (AllMCParticles.isValid()) {
            for (unsigned i = 0; i < numParticles; ++i) {
              const MCParticle* newParticle = AllMCParticles[i];
              const MCParticle* newMother = newParticle->getMother();

              if (newMother != nullptr) {
                if (newMother == Y4S &&  newParticle != BcpMC) {
                  if (newParticle -> getPDG() == 511) OutputB0tagQ = 1;
                  else OutputB0tagQ = 0;
                }
              }
            }
          }
        }
      }
      return OutputB0tagQ;
    }

    double isRelatedRestOfEventMajorityB0Flavor(const Particle* part)
    {
      const RestOfEvent* roe = part->getRelatedTo<RestOfEvent>();

      float OutputStandard = 0.0;

      float q_MC = 0; //Flavor of B
      if (roe != nullptr) {
        if (roe-> getNTracks() != 0) {
          for (auto& track : roe->getTracks()) {
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
          for (auto& cluster : roe-> getECLClusters()) {
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
          for (auto& klmcluster : roe-> getKLMClusters()) {
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
      }

      if (q_MC > 0) {
        OutputStandard = 1;
      } else if (q_MC < 0) {
        OutputStandard = 0;
      } else OutputStandard = -2;

      return OutputStandard;
    }

    double isRestOfEventMajorityB0Flavor(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      float q_MC = 0; //Flavor of B
      if (roe.isValid()) {
        if (roe-> getNTracks() != 0) {
          for (auto& track : roe->getTracks()) {
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
          for (auto& cluster : roe-> getECLClusters()) {
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
          for (auto& klmcluster : roe-> getKLMClusters()) {
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
      }
      if (q_MC > 0) {
        return 1;
      } else if (q_MC < 0) {
        return 0;
      } else return -2;//gRandom->Uniform(0, 1);
    }

    double McFlavorOfTagSide(const Particle* part)
    {
      const RestOfEvent* roe = part->getRelatedTo<RestOfEvent>();
      if (roe != nullptr) {
        for (auto& track : roe->getTracks()) {
          const MCParticle* mcParticle = track->getRelated<MCParticle>();
          while (mcParticle != nullptr) {
            if (mcParticle->getPDG() == 511) return 511;
            else if (mcParticle->getPDG() == -511) return -511;
            mcParticle = mcParticle->getMother();
          }
        }
      }
      return 0;
    }

//  ######################################### Meta Variables ##############################################

//  Miscelaneous -------------------------------------------------------------------------------------------
    Manager::FunctionPtr CheckingVariables(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        auto ListName = arguments[0];
        auto requestedVariable = arguments[1];
        auto func = [requestedVariable, ListName](const Particle*) -> double {
          if (requestedVariable == "getListSize")
          {
            StoreObjPtr<ParticleList> ListOfParticles(ListName);
            if (ListOfParticles.isValid()) return ListOfParticles->getListSize();
            else return 0;
          } else {
            B2FATAL("Wrong requested Variable. Available is getListSize for particle lists");
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (2 required) for meta function CheckingVariables");
      }
    }

    Manager::FunctionPtr IsDaughterOf(const std::vector<std::string>& arguments)
    {
      auto motherlist = arguments[0];
      auto func = [motherlist](const Particle * particle) -> double {

        StoreObjPtr<ParticleList> Y(motherlist);
        std::vector<Particle*> daughters;
        if (Y.isValid())
        {
          for (unsigned int i = 0; i < Y->getListSize(); ++i) {
            const auto& x = Y->getParticle(i)->getDaughters();
            daughters.insert(daughters.end(), x.begin(), x.end());
          }
        }
        while (!daughters.empty())
        {
          std::vector<Particle*> tmpdaughters;
          for (auto& d : daughters) {
            if (d == particle)
              return 1.0;
            const auto& x = d->getDaughters();
            tmpdaughters.insert(tmpdaughters.end(), x.begin(), x.end());
          }
          daughters = tmpdaughters;
        }
        return 0.0;
      };
      return func;
    }

//  Track and Event Level variables ------------------------------------------------------------------------

    Manager::FunctionPtr NumberOfKShortinROEParticleList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto particleListName = arguments[0];
        auto func = [particleListName](const Particle * particle) -> double {
          int flag = 0;
          if (particleListName == "K_S0:ROEKaon" || particleListName == "K_S0:ROELambda")
          {
            StoreObjPtr<ParticleList> KShortList(particleListName);
            if (KShortList.isValid()) {
              for (unsigned int i = 0; i < KShortList->getListSize(); i++) {
                if (particle->overlapsWith(KShortList->getParticle(i))) {continue;}
                flag++;
              }
            }
          } else B2FATAL("NumberOfKShortinROEParticleList not available for the requested particleListName  " << particleListName << " . The possibilities are K_S0:ROEKaon or K_S0:ROELambda");          return flag;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (1 required) for meta function NumberOfKShortinROEParticleList");
      }
    }

    Manager::FunctionPtr SemiLeptonicVariables(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto requestedVariable = arguments[0];
        auto func = [requestedVariable](const Particle * particle) -> double {
          PCmsLabTransform T;
          TLorentzVector momXchargedtracks; //Momentum of charged X tracks in CMS-System
          TLorentzVector momXchargedclusters; //Momentum of charged X clusters in CMS-System
          TLorentzVector momXneutralclusters; //Momentum of neutral X clusters in CMS-System
          TLorentzVector momTarget = T.rotateLabToCms() * particle -> get4Vector();  //Momentum of Mu in CMS-System

          double Output = 0.0;

          StoreObjPtr<RestOfEvent> roe("RestOfEvent");
          if (roe.isValid())
          {
            const auto& tracks = roe->getTracks();
            for (auto& x : tracks) {
              const TrackFitResult* tracki = x->getTrackFitResult(x->getRelated<PIDLikelihood>()->getMostLikely());
              if (tracki == nullptr || particle->getTrack() == x) continue;
              TLorentzVector momtrack(tracki->getMomentum(), 0);
              TLorentzVector momXchargedtracki = T.rotateLabToCms() * momtrack;
              if (momXchargedtracki == momXchargedtracki) momXchargedtracks += momXchargedtracki;
            }
            const auto& ecl = roe->getECLClusters();
            for (auto& x : ecl) {
              if (x == nullptr) continue;
              TLorentzVector momXclusteri = T.rotateLabToCms() * x -> get4Vector();
              if (momXclusteri == momXclusteri) {
                if (x->isNeutral()) momXneutralclusters += momXclusteri;
                else if (!(x->isNeutral())) momXchargedclusters += momXclusteri;
              }
            }
            const auto& klm = roe->getKLMClusters();
            for (auto& x : klm) {
              if (x == nullptr) continue;
              TLorentzVector momXKLMclusteri = T.rotateLabToCms() * x -> getMomentum();
              if (momXKLMclusteri == momXKLMclusteri) {
                if (!(x -> getAssociatedTrackFlag()) && !(x -> getAssociatedEclClusterFlag())) {
                  momXneutralclusters += momXKLMclusteri;
                }
              }
            }

            TLorentzVector momXcharged(momXchargedtracks.Vect(), momXchargedclusters.E());
            TLorentzVector momX = (momXcharged + momXneutralclusters) - momTarget; //Total Momentum of the recoiling X in CMS-System
            TLorentzVector momMiss = -(momX + momTarget); //Momentum of Anti-v  in CMS-System
            if (requestedVariable == "recoilMass") Output = momX.M();
            else if (requestedVariable == "pMissCMS") Output = momMiss.Vect().Mag();
            else if (requestedVariable == "cosThetaMissCMS") Output = TMath::Cos(momTarget.Angle(momMiss.Vect()));
            else if (requestedVariable == "EW90") {
              TLorentzVector momW = momTarget + momMiss; //Momentum of the W-Boson in CMS
              float E_W_90 = 0 ; // Energy of all charged and neutral clusters in the hemisphere of the W-Boson
              for (auto& i : ecl) {
                if (i == nullptr) continue;
                float Energyi = i -> getEnergy();
                if (Energyi == Energyi) {
                  if ((T.rotateLabToCms() * i -> get4Vector()).Vect().Dot(momW.Vect()) > 0) E_W_90 += Energyi;
                }
                //       for (auto & i : klm) {
                //         if ((T.rotateLabToCms() * i -> getMomentum()).Vect().Dot(momW.Vect()) > 0) E_W_90 +=;
                //         }
              }
              Output = E_W_90;
            } else {
              B2FATAL("Wrong variable  " << requestedVariable <<
                      " requested. The possibilities are recoilMass, pMissCMS, cosThetaMissCMS or EW90");
            }
          }
          return Output;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (1 required) for meta function SemiLeptonicVariables");
      }
    }

    Manager::FunctionPtr KaonPionVariables(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto requestedVariable = arguments[0];
        auto func = [requestedVariable](const Particle * particle) -> double {
//       StoreObjPtr<ParticleList> KaonList("K+:ROE");
          StoreObjPtr<ParticleList> SlowPionList("pi+:SlowPionROE");

          PCmsLabTransform T;
          TLorentzVector momTargetKaon = T.rotateLabToCms() * particle -> get4Vector();
          TLorentzVector momTargetSlowPion;

          float chargeTargetKaon = particle -> getCharge();

          double Output = 0.0;

          if ((requestedVariable == "HaveOpositeCharges") || (requestedVariable == "cosKaonPion"))
          {
            float chargeTargetSlowPion = 0;
            if (SlowPionList.isValid()) {
              double maximumProbSlowPion = 0;
              for (unsigned int i = 0; i < SlowPionList->getListSize(); ++i) {
                Particle* pSlowPion = SlowPionList->getParticle(i);
                if (pSlowPion != nullptr) {
                  double probSlowPion = 0;
                  if (pSlowPion -> hasExtraInfo("isRightTrack(SlowPion)")) {
                    probSlowPion = pSlowPion->getExtraInfo("isRightTrack(SlowPion)");
                    if (probSlowPion > maximumProbSlowPion) {
                      maximumProbSlowPion = probSlowPion;
                      chargeTargetSlowPion =  pSlowPion -> getCharge();
                      momTargetSlowPion = T.rotateLabToCms() * pSlowPion -> get4Vector();
                    }
                  }
                }
              }
            }
            if (requestedVariable == "HaveOpositeCharges") {
              if (chargeTargetKaon * chargeTargetSlowPion == -1) Output = 1;
            }
            if (requestedVariable == "cosKaonPion") {
              if (momTargetKaon == momTargetKaon
                  && momTargetSlowPion == momTargetSlowPion) Output = TMath::Cos(momTargetKaon.Angle(momTargetSlowPion.Vect()));
            }

          } else {
            B2FATAL("Wrong variable  " << requestedVariable << " requested. The possibilities are cosKaonPion or HaveOpositeCharges");
          }
          return Output;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (1 required) for meta function KaonPionVariables");
      }
    }

    Manager::FunctionPtr FSCVariables(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto requestedVariable = arguments[0];
        auto func = [requestedVariable](const Particle * particle) -> double {
          StoreObjPtr<ParticleList> FastParticleList("pi+:FastPionROE");
          PCmsLabTransform T;
          TLorentzVector momSlowPion = T.rotateLabToCms() * particle -> get4Vector();  //Momentum of Slow Pion in CMS-System
          TLorentzVector momFastParticle;  //Momentum of Fast Pion in CMS-System
          double maximumProbFastest = 0;

          double Output = 0.0;

          if ((requestedVariable == "pFastCMS") || (requestedVariable == "cosSlowFast") || (requestedVariable == "cosTPTOFast") || (requestedVariable == "SlowFastHaveOpositeCharges"))
          {
            if (FastParticleList.isValid()) {
              Particle* TargetFastParticle = nullptr;
              for (unsigned int i = 0; i < FastParticleList->getListSize(); ++i) {
                Particle* particlei = FastParticleList->getParticle(i);
                if (particlei != nullptr) {
                  TLorentzVector momParticlei = T.rotateLabToCms() * particlei -> get4Vector();
                  if (momParticlei == momParticlei) {
                    double probFastest = momParticlei.P();
                    if (probFastest > maximumProbFastest) {
                      maximumProbFastest = probFastest;
                      TargetFastParticle = particlei;
                      momFastParticle = momParticlei;
                    }
                  }
                }
              }
              if (TargetFastParticle != nullptr) {
                if (requestedVariable == "cosTPTOFast") Output = Variable::Manager::Instance().getVariable("cosTPTO")->function(
                                                                     TargetFastParticle);
                if (momSlowPion == momSlowPion) {
                  if (requestedVariable == "cosSlowFast") Output = TMath::Cos(momSlowPion.Angle(momFastParticle.Vect()));
                  else if (requestedVariable == "SlowFastHaveOpositeCharges") {
                    if (particle->getCharge()*TargetFastParticle->getCharge() == -1) {
                      Output = 1;
                    }
                  } else Output = momFastParticle.P();
                }
              }
            }
          } else {
            B2FATAL("Wrong variable " << requestedVariable << " requested. The possibilities are pFastCMS, cosSlowFast or cosTPTOFast");
          }
          return Output;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (1 required) for meta function FSCVariables");
      }
    }

    Manager::FunctionPtr hasHighestProbInCat(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        auto particleListName = arguments[0];
        auto extraInfoName = arguments[1];
        auto func = [particleListName, extraInfoName](const Particle * particle) -> double {
          if (!(extraInfoName == "isRightTrack(Electron)" || extraInfoName == "isRightTrack(IntermediateElectron)" || extraInfoName == "isRightTrack(Muon)" || extraInfoName == "isRightTrack(IntermediateMuon)"
          || extraInfoName == "isRightTrack(KinLepton)" || extraInfoName == "isRightTrack(IntermediateKinLepton)" || extraInfoName == "isRightTrack(Kaon)"
          || extraInfoName == "isRightTrack(SlowPion)" || extraInfoName == "isRightTrack(FastPion)" || extraInfoName == "isRightTrack(MaximumPstar)" || extraInfoName == "isRightTrack(Lambda)"
          || extraInfoName == "isRightCategory(Electron)" || extraInfoName == "isRightCategory(IntermediateElectron)" || extraInfoName == "isRightCategory(Muon)" || extraInfoName == "isRightCategory(IntermediateMuon)"
          || extraInfoName == "isRightCategory(KinLepton)" || extraInfoName == "isRightCategory(IntermediateKinLepton)" || extraInfoName == "isRightCategory(Kaon)"
          || extraInfoName == "isRightCategory(SlowPion)" || extraInfoName == "isRightCategory(FastPion)" || extraInfoName == "isRightCategory(KaonPion)" || extraInfoName == "isRightCategory(Lambda)"
          || extraInfoName == "isRightCategory(MaximumPstar)" || extraInfoName == "isRightCategory(FSC)"))
          {
            B2FATAL("hasHighestProbInCat: Not available category" << extraInfoName <<
            ". The possibilities for isRightTrack() are \nElectron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastPion, MaximumPstar, and Lambda."
            << endl <<
            "The possibilities for isRightCategory() are \nElectron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastPion, KaonPion, MaximumPstar, FSC and Lambda");
            return 0.0;
          }

          StoreObjPtr<ParticleList> ListOfParticles(particleListName);
          PCmsLabTransform T;

          double Output = 0.0;

          if (ListOfParticles.isValid())
          {
            double maximumProb = 0;
            for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i) {
              Particle* particlei = ListOfParticles->getParticle(i);
              if (particlei != nullptr) {
                double prob = 0;
                if (extraInfoName == "isRightTrack(MaximumPstar)") {
                  TLorentzVector momParticlei = T.rotateLabToCms() * particlei -> get4Vector();
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
            }
            if ((extraInfoName == "isRightTrack(MaximumPstar)" && (T.rotateLabToCms() * particle -> get4Vector()).P() == maximumProb)) {
              Output = 1.0;
            } else if (extraInfoName != "isRightTrack(MaximumPstar)" && particle -> hasExtraInfo(extraInfoName)) {
              if (particle -> getExtraInfo(extraInfoName) == maximumProb) Output = 1.0;
            }
          }
          return Output;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (2 required) for meta function hasHighestProbInCat");
      }
    }

    Manager::FunctionPtr HighestProbInCat(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        auto particleListName = arguments[0];
        auto extraInfoName = arguments[1];
        auto func = [particleListName, extraInfoName](const Particle*) -> double {
          if (!(extraInfoName == "isRightTrack(Electron)" || extraInfoName == "isRightTrack(IntermediateElectron)" || extraInfoName == "isRightTrack(Muon)" || extraInfoName == "isRightTrack(IntermediateMuon)"
          || extraInfoName == "isRightTrack(KinLepton)" || extraInfoName == "isRightTrack(IntermediateKinLepton)" || extraInfoName == "isRightTrack(Kaon)"
          || extraInfoName == "isRightTrack(SlowPion)" || extraInfoName == "isRightTrack(FastPion)" || extraInfoName == "isRightTrack(MaximumPstar)" || extraInfoName == "isRightTrack(Lambda)"
          || extraInfoName == "isRightCategory(Electron)" || extraInfoName == "isRightCategory(IntermediateElectron" || extraInfoName == "isRightCategory(Muon)" || extraInfoName == "isRightCategory(IntermediateMuon)"
          || extraInfoName == "isRightCategory(KinLepton)" || extraInfoName == "isRightCategory(IntermediateKinLepton)" || extraInfoName == "isRightCategory(Kaon)"
          || extraInfoName == "isRightCategory(SlowPion)" || extraInfoName == "isRightCategory(FastPion)" || extraInfoName == "isRightCategory(KaonPion)" || extraInfoName == "isRightCategory(Lambda)"
          || extraInfoName == "isRightCategory(MaximumPstar)" || extraInfoName == "isRightCategory(FSC)"))
          {
            B2FATAL("hasHighestProbInCat: Not available category" << extraInfoName <<
            ". The possibilities for isRightTrack() are \nElectron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastPion, MaximumPstar, and Lambda."
            << endl <<
            "The possibilities for isRightCategory() are \nElectron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastPion, KaonPion, MaximumPstar, FSC and Lambda");
            return 0.0;
          }

          StoreObjPtr<ParticleList> ListOfParticles(particleListName);
          PCmsLabTransform T;
          double maximumProb = 0;
          if (ListOfParticles.isValid())
          {
            for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i) {
              Particle* particlei = ListOfParticles->getParticle(i);
              if (particlei != nullptr) {
                double prob = 0;
                if (extraInfoName == "isRightTrack(MaximumPstar)") {
                  TLorentzVector momParticlei = T.rotateLabToCms() * particlei -> get4Vector();
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
            }
          }
          return maximumProb;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (2 required) for meta function hasHighestProbInCat");
      }
    }

//  Target Variables ----------------------------------------------------------------------------------------------

    Manager::FunctionPtr isRightTrack(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto particleName = arguments[0];
        auto func = [particleName](const Particle * part) -> double {
          if (!(particleName == "Electron" || particleName == "IntermediateElectron" || particleName == "Muon" || particleName == "IntermediateMuon"
          || particleName == "KinLepton" || particleName == "IntermediateKinLepton" || particleName == "Kaon"
          || particleName == "SlowPion" || particleName == "FastPion" || particleName == "Lambda"))
          {
            B2FATAL("isRightTrack: Not available category " << particleName <<
            ". The possibilities are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastPion and Lambda");
            return 0.0;
          }

          const MCParticle* mcParticle = part->getRelated<MCParticle>();
          if (mcParticle == nullptr) return 0.0;
          //direct electron
          else if (particleName == "Electron"
          && (mcParticle->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 11
          && TMath::Abs(mcParticle->getMother()->getPDG()) == 511))
          {
            return 1.0;
            //intermediate electron
          } else if (particleName == "IntermediateElectron"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 11
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511)
          {
            return 1.0;
            //direct muon
          } else if (particleName == "Muon"
          && (mcParticle->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 13
          && TMath::Abs(mcParticle->getMother()->getPDG()) == 511))
          {
            return 1.0;
            //intermediate muon
          } else if (particleName == "IntermediateMuon"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 13
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511)
          {
            return 1.0;
            //KinLepton
          } else if (particleName == "KinLepton"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && (TMath::Abs(mcParticle->getPDG()) == 13 || TMath::Abs(mcParticle->getPDG()) == 11)
          && TMath::Abs(mcParticle->getMother()->getPDG()) == 511)
          {
            return 1.0;
            //IntermediateKinLepton
          } else if (particleName == "IntermediateKinLepton"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && (TMath::Abs(mcParticle->getPDG()) == 13 || TMath::Abs(mcParticle->getPDG()) == 11)
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511)
          {
            return 1.0;
            //kaon
          } else if (particleName == "Kaon"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 321
          && TMath::Abs(mcParticle->getMother()->getPDG()) > 400
          && TMath::Abs(mcParticle->getMother()->getPDG()) < 500
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511)
          {
            return 1.0;
            //slow pion
          } else if (particleName == "SlowPion"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 211
          && TMath::Abs(mcParticle->getMother()->getPDG()) == 413
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511)
          {
            return 1.0;
            //high momentum pions
          } else if (particleName == "FastPion"
          && mcParticle->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 211
          && TMath::Abs(mcParticle->getMother()->getPDG()) == 511)
          {
            return 1.0;
            //lambdas
          } else if (particleName == "Lambda"
          && mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 3122)
          {
            return 1.0;
          } else return 0.0;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (1 required) for meta function isRightTrack");
      }
    }

    Manager::FunctionPtr isRightCategory(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto particleName = arguments[0];
        auto func = [particleName](const Particle * particle) -> double {
          if (!(particleName == "Electron" || particleName == "IntermediateElectron" || particleName == "Muon" || particleName == "IntermediateMuon"
          || particleName == "KinLepton" || particleName == "IntermediateKinLepton" || particleName == "Kaon"
          || particleName == "SlowPion" || particleName == "FastPion" || particleName == "KaonPion" || particleName == "Lambda" || particleName == "MaximumPstar" || particleName == "FSC"))
          {
            B2FATAL("isRightCategory: Not available category " << particleName <<
            ". The possibilities are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastPion, KaonPion, MaximumPstar, FSC and Lambda");
            return 0.0;
          }

          Particle* nullpart = nullptr;
          float qTarget = 0;
          float qMC = 0;
          int maximumPDG = 0;
          int maximumPDGMother = 0;
          int maximumPDGMotherMother = 0;
          const MCParticle* MCp = particle ->getRelated<MCParticle>();
          qTarget = particle -> getCharge();
          qMC = 2 * (Variable::Manager::Instance().getVariable("qrCombined")->function(nullpart) - 0.5);

          // ---------------------------- Mothers and Grandmothers ---------------------------------
          if (MCp != nullptr)
          {
            maximumPDG = TMath::Abs(MCp->getPDG());
            if (particleName != "Lambda" && MCp->getMother() != nullptr) {
              maximumPDGMother = TMath::Abs(MCp->getMother()->getPDG());
            }
            //for some Categories we need the mother of the mother of the particle
            if ((particleName == "Kaon" || particleName == "SlowPion" || particleName == "IntermediateElectron"
                 || particleName == "IntermediateMuon" || particleName == "IntermediateKinLepton" || particleName == "FastPion")
                && MCp->getMother()->getMother() != nullptr) maximumPDGMotherMother =  TMath::Abs(MCp->getMother()->getMother()->getPDG());
          }

          // ----------------------------  For KaonPion Category ------------------------------------
          int SlowPionPDG = 0;
          int SlowPionPDGMother = 0;
          if (particleName == "KaonPion")
          {
            StoreObjPtr<ParticleList> SlowPionList("pi+:SlowPionROE");
            Particle* TargetSlowPion = nullptr;
            if (SlowPionList.isValid()) {
              double maximumProbSlowPion = 0;
              for (unsigned int i = 0; i < SlowPionList->getListSize(); ++i) {
                Particle* pSlowPion = SlowPionList->getParticle(i);
                if (pSlowPion != nullptr) {
                  if (pSlowPion -> hasExtraInfo("isRightTrack(SlowPion)")) {
                    double probSlowPion = pSlowPion->getExtraInfo("isRightTrack(SlowPion)");
                    if (probSlowPion > maximumProbSlowPion) {
                      maximumProbSlowPion = probSlowPion;
                      TargetSlowPion = pSlowPion;
                    }
                  }
                }
              }
              if (TargetSlowPion != nullptr) {
                const MCParticle* MCSlowPion = TargetSlowPion ->getRelated<MCParticle>();
//               SlowPion_q = TargetSlowPion -> getCharge();
                if (MCSlowPion != nullptr && MCSlowPion->getMother() != nullptr) {
                  SlowPionPDG = TMath::Abs(MCSlowPion->getPDG());
                  SlowPionPDGMother = TMath::Abs(MCSlowPion->getMother()->getPDG());
                }
              }
            }
          }

          // ----------------------------  For FastSlowCorrelated Category ----------------------------
          int FastParticlePDGMother = 0;
          if (particleName == "FSC")
          {
            StoreObjPtr<ParticleList> FastParticleList("pi+:FastPionROE");
            PCmsLabTransform T;
            Particle* TargetFastParticle = nullptr;
            if (FastParticleList.isValid()) {
              double maximumProbFastest = 0;
              for (unsigned int i = 0; i < FastParticleList->getListSize(); ++i) {
                Particle* particlei = FastParticleList->getParticle(i);
                if (particlei != nullptr) {
                  TLorentzVector momParticlei = T.rotateLabToCms() * particlei -> get4Vector();
                  if (momParticlei == momParticlei) {
                    double probFastest = momParticlei.P();
                    if (probFastest > maximumProbFastest) {
                      maximumProbFastest = probFastest;
                      TargetFastParticle = particlei;
                    }
                  }
                }
              }
              if (TargetFastParticle != nullptr) {
                const MCParticle* MCFastParticle = TargetFastParticle ->getRelated<MCParticle>();
//               FastParticle_q = TargetFastParticle -> getCharge();
                if (MCFastParticle != nullptr && MCFastParticle->getMother() != nullptr) {
                  FastParticlePDGMother = TMath::Abs(MCFastParticle->getMother()->getPDG());
                }
              }
            }
          }

          // ------------------------------  Outputs  -----------------------------------
          if (particleName == "Electron"
              && qTarget == qMC && maximumPDG == 11 && maximumPDGMother == 511)
          {
            return 1.0;
          } else if (particleName == "IntermediateElectron"
                     && qTarget != qMC && maximumPDG == 11 && maximumPDGMotherMother == 511)
          {
            return 1.0;
          } else if (particleName == "Muon"
                     && qTarget == qMC && maximumPDG == 13 && maximumPDGMother == 511)
          {
            return 1.0;
          } else if (particleName == "IntermediateMuon"
                     && qTarget != qMC && maximumPDG == 13 && maximumPDGMotherMother == 511)
          {
            return 1.0;
          }  else if (particleName == "KinLepton"
                      && qTarget == qMC && (maximumPDG == 11 || maximumPDG == 13) && maximumPDGMother == 511)
          {
            return 1.0;
          }  else if (particleName == "IntermediateKinLepton"
                      && qTarget != qMC && (maximumPDG == 11 || maximumPDG == 13) && maximumPDGMotherMother == 511)
          {
            return 1.0;
          } else if (particleName == "Kaon" && qTarget == qMC
                     && maximumPDG == 321 && maximumPDGMother > 400 && maximumPDGMother < 500 && maximumPDGMotherMother == 511)
          {
            return 1.0;
          } else if (particleName == "SlowPion" && qTarget != qMC
                     && maximumPDG == 211 && maximumPDGMother == 413 && maximumPDGMotherMother == 511)
          {
            return 1.0;
          } else if (particleName == "KaonPion" && qTarget == qMC
                     && maximumPDG == 321 && SlowPionPDG == 211 && maximumPDGMother == SlowPionPDGMother)
          {
            return 1.0;
          } else if (particleName == "FastPion" && qTarget == qMC
                     && maximumPDG == 211 && maximumPDGMother == 511)
          {
            return 1.0;
          } else if (particleName == "MaximumPstar" && qTarget == qMC)
          {
            return 1.0;
          } else if (particleName == "FSC" && qTarget != qMC
                     && maximumPDG == 211 && FastParticlePDGMother == 511)
          {
            return 1.0;
          } else if (particleName == "Lambda" && (particle->getPDGCode() / TMath::Abs(particle->getPDGCode())) != qMC
                     && maximumPDG == 3122)
          {
            return 1.0;
          } else {
            return 0.0;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (1 required) for meta function isRightCategory");
      }
    }

    Manager::FunctionPtr QrOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 3) {
        auto particleListName = arguments[0];
        auto extraInfoRightCategory = arguments[1];
        auto extraInfoRightTrack = arguments[2];
        auto func = [particleListName, extraInfoRightCategory, extraInfoRightTrack](const Particle*) -> double {
          if (!(extraInfoRightCategory == "isRightCategory(Electron)" || extraInfoRightCategory == "isRightCategory(IntermediateElectron)" || extraInfoRightCategory == "isRightCategory(Muon)" || extraInfoRightCategory == "isRightCategory(IntermediateMuon)"
          || extraInfoRightCategory == "isRightCategory(KinLepton)" || extraInfoRightCategory == "isRightCategory(IntermediateKinLepton)" || extraInfoRightCategory == "isRightCategory(Kaon)"
          || extraInfoRightCategory == "isRightCategory(SlowPion)" || extraInfoRightCategory == "isRightCategory(FastPion)" || extraInfoRightCategory == "isRightCategory(KaonPion)" || extraInfoRightCategory == "isRightCategory(Lambda)" || extraInfoRightCategory == "isRightCategory(MaximumPstar)" || extraInfoRightCategory == "isRightCategory(FSC)"))
          {
            B2FATAL("QrOf: Not available category in " << extraInfoRightCategory <<
            ". The possibilities for isRightCategory() are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastPion, KaonPion, MaximumPstar, FSC and Lambda");
            return 0.0;
          }

          if (!(extraInfoRightTrack == "isRightTrack(Electron)" || extraInfoRightTrack == "isRightTrack(IntermediateElectron)" || extraInfoRightTrack == "isRightTrack(Muon)" || extraInfoRightTrack == "isRightTrack(IntermediateMuon)"
          || extraInfoRightTrack == "isRightTrack(KinLepton)" || extraInfoRightTrack == "isRightTrack(IntermediateKinLepton)" || extraInfoRightTrack == "isRightTrack(Kaon)"
          || extraInfoRightTrack == "isRightTrack(SlowPion)" || extraInfoRightTrack == "isRightTrack(FastPion)" || extraInfoRightTrack == "isRightTrack(MaximumPstar)" || extraInfoRightTrack == "isRightTrack(Lambda)"))
          {
            B2FATAL("QrOf: Not available category in " << extraInfoRightTrack <<
            ". The possibilities for isRightTrack() are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastPion, MaximumPstar, and Lambda");
            return 0.0;
          }

          StoreObjPtr<ParticleList> ListOfParticles(particleListName);
          PCmsLabTransform T;
          Particle* target = nullptr; //Particle selected as target
          float prob = 0; //The probability of beeing right classified flavor from the event level
          float qTarget = 0; //Flavor of the track selected as target
          if (ListOfParticles.isValid())
          {
            float maximumTargetProb = 0; //Probability of being the target track from the track level
            for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i) {
              Particle* particlei = ListOfParticles->getParticle(i);
              if (particlei != nullptr) {
                double prob = 0;
                if (extraInfoRightTrack == "isRightTrack(MaximumPstar)") {
                  TLorentzVector momParticlei = T.rotateLabToCms() * particlei -> get4Vector();
                  if (momParticlei == momParticlei) {
                    prob = momParticlei.P();
                  }
                } else {
                  if (particlei->hasExtraInfo(extraInfoRightTrack)) {
                    prob = particlei->getExtraInfo(extraInfoRightTrack);
                  }
                }
                if (prob > maximumTargetProb) {
                  maximumTargetProb = prob;
                  target = particlei;
                }
              }
            }
            if (target != nullptr) {
              prob = target -> getExtraInfo(extraInfoRightCategory); //Gets the probability of beeing right classified flavor from the event level
              // Gets the flavor of the track selected as target
              if (extraInfoRightTrack == "isRightTrack(Lambda)") {
                qTarget = (-1) * target->getPDGCode() / TMath::Abs(target->getPDGCode());
              } else if (extraInfoRightTrack == "isRightTrack(IntermediateElectron)" || extraInfoRightTrack == "isRightTrack(IntermediateMuon)"
                         || extraInfoRightTrack == "isRightTrack(IntermediateKinLepton)" || extraInfoRightTrack == "isRightTrack(SlowPion)") {
                qTarget = (-1) * target -> getCharge();
              } else qTarget = target -> getCharge();
            }
          }
          //float r = TMath::Abs(2 * prob - 1); //Definition of the dilution factor  */
          //return 0.5 * (qTarget * r + 1);
          return qTarget * prob;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (3 required) for meta function QrOf");
      }
    }

    Manager::FunctionPtr weightedQrOf(const std::vector<std::string>& arguments)
    {
      //used by simple_flavor_tagger
      if (arguments.size() == 3) {
        auto particleListName = arguments[0];
        auto extraInfoRightCategory = arguments[1];
        auto extraInfoRightTrack = arguments[2];
        auto func = [particleListName, extraInfoRightCategory, extraInfoRightTrack](const Particle*) -> double {
          if (!(extraInfoRightCategory == "isRightCategory(Electron)" || extraInfoRightCategory == "isRightCategory(IntermediateElectron)" || extraInfoRightCategory == "isRightCategory(Muon)" || extraInfoRightCategory == "isRightCategory(IntermediateMuon)"
          || extraInfoRightCategory == "isRightCategory(KinLepton)" || extraInfoRightCategory == "isRightCategory(IntermediateKinLepton)" || extraInfoRightCategory == "isRightCategory(Kaon)"
          || extraInfoRightCategory == "isRightCategory(SlowPion)" || extraInfoRightCategory == "isRightCategory(FastPion)" || extraInfoRightCategory == "isRightCategory(KaonPion)" || extraInfoRightCategory == "isRightCategory(Lambda)" || extraInfoRightCategory == "isRightCategory(FSC)"))
          {
            B2FATAL("weightedQrOf: Not available category in " << extraInfoRightCategory <<
            ". The possibilities for isRightCategory() are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastPion, KaonPion, FSC and Lambda");
            return 0.0;
          }

          if (!(extraInfoRightTrack == "isRightTrack(Electron)" || extraInfoRightTrack == "isRightTrack(IntermediateElectron)" || extraInfoRightTrack == "isRightTrack(Muon)" || extraInfoRightTrack == "isRightTrack(IntermediateMuon)"
          || extraInfoRightTrack == "isRightTrack(KinLepton)" || extraInfoRightTrack == "isRightTrack(IntermediateKinLepton)" || extraInfoRightTrack == "isRightTrack(Kaon)"
          || extraInfoRightTrack == "isRightTrack(SlowPion)" || extraInfoRightTrack == "isRightTrack(FastPion)" || extraInfoRightTrack == "isRightTrack(Lambda)"))
          {
            B2FATAL("weightedQrOf: Not available category in " << extraInfoRightTrack <<
            ". The possibilities for isRightTrack() are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastPion and Lambda");
            return 0.0;
          }

          double final_value = 0.0;

          auto compare = [extraInfoRightTrack](const Particle * part1, const Particle * part2)-> bool {
            double info1 = 0;
            double info2 = 0;
            if (part1->hasExtraInfo(extraInfoRightTrack)) info1 = part1->getExtraInfo(extraInfoRightTrack);
            if (part2->hasExtraInfo(extraInfoRightTrack)) info2 = part2->getExtraInfo(extraInfoRightTrack);
            return (info1 > info2);
          };

          StoreObjPtr<ParticleList> ListOfParticles(particleListName);
          if (ListOfParticles)
          {
            if (ListOfParticles->getListSize() > 0) {
              double val1 = 1.0;
              double val2 = 1.0;
              std::vector<const Particle*> ParticleVector;
              ParticleVector.reserve(ListOfParticles->getListSize());

              for (unsigned int i = 0; i < ListOfParticles->getListSize(); i++) {
                ParticleVector.push_back(ListOfParticles->getParticle(i));
              }
              std::sort(ParticleVector.begin(), ParticleVector.end(), compare);

              if (ParticleVector.size() != 0) final_value = 1.0;
              //Loop over K+ vector until 3 or empty
              unsigned int Limit = ParticleVector.size() > 3 ? 3 : ParticleVector.size();
              for (unsigned int i = 0; i < Limit; i++) {
                if (ParticleVector[i]->hasExtraInfo(extraInfoRightCategory)) {
                  double flavor = 0.0;
                  if (particleListName == "Lambda0:LambdaROE") {
                    flavor = (-1) * ParticleVector[i]->getPDGCode() / TMath::Abs(ParticleVector[i]->getPDGCode());
                  } else if (extraInfoRightTrack == "isRightTrack(IntermediateElectron)" || extraInfoRightTrack == "isRightTrack(IntermediateMuon)"
                             || extraInfoRightTrack == "isRightTrack(IntermediateKinLepton)" || extraInfoRightTrack == "isRightTrack(SlowPion)") {
                    flavor = (-1) * ParticleVector[i] -> getCharge();
                  } else flavor = ParticleVector[i]->getCharge();

                  double r = ParticleVector[i]->getExtraInfo(extraInfoRightCategory);
//                 B2INFO("Right Cat:" << ParticleVector[i]->getExtraInfo(extraInfoRightCategory));
//                 B2INFO("Right Track:" << ParticleVector[i]->getExtraInfo(extraInfoRightTrack));
                  double qp = (flavor * r);
                  val1 = val1 * (1 + qp);
                  val2 = val2 * (1 - qp);
                }
              }
              final_value = (val1 - val2) / (val1 + val2);
            }
          }
          return final_value;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (3 required) for meta function QrOf");
      }
    }

    VARIABLE_GROUP("Flavor Tagger Variables");

    REGISTER_VARIABLE("pMissTag", momentumMissingTagSide,  "Calculates the missing Momentum for a given particle on the tag side.");
    REGISTER_VARIABLE("cosTPTO"  , cosTPTO , "cosine of angle between thrust axis of given particle and thrust axis of ROE");
    REGISTER_VARIABLE("lambdaFlavor", lambdaFlavor,  "1.0 if Lambda0, -1.0 if Anti-Lambda0, 0.0 else.");
    REGISTER_VARIABLE("isLambda", isLambda,  "1.0 if MCLambda0, 0.0 else.");
    REGISTER_VARIABLE("lambdaZError", lambdaZError,  "Returns the Matrixelement[2][2] of the PositionErrorMatrix of the Vertex fit.");
    REGISTER_VARIABLE("momentumOfSecondDaughter", momentumOfSecondDaughter,
                      "Returns the Momentum of second daughter if existing, else 0.");
    REGISTER_VARIABLE("momentumOfSecondDaughterCMS", momentumOfSecondDaughterCMS,
                      "Returns the Momentum of second daughter if existing in CMS, else 0.");
    REGISTER_VARIABLE("chargeTimesKaonLiklihood", chargeTimesKaonLiklihood,
                      "Returns the q*(highest PID_Likelihood for Kaons), else 0.");
    REGISTER_VARIABLE("ptTracksRoe", transverseMomentumOfChargeTracksInRoe,
                      "Returns the transverse momentum of all charged tracks if there exists a ROE for the given particle, else 0.");

    REGISTER_VARIABLE("isInElectronOrMuonCat", isInElectronOrMuonCat,
                      "Returns 1.0 if the particle has been selected as target in the Muon or Electron Category, 0.0 else.");

    REGISTER_VARIABLE("isMajorityInRestOfEventFromB0", isMajorityInRestOfEventFromB0,
                      "[Eventbased] Check if the majority of the tracks in the current RestOfEvent are from a B0.");
    REGISTER_VARIABLE("isMajorityInRestOfEventFromB0bar", isMajorityInRestOfEventFromB0bar,
                      "[Eventbased] Check if the majority of the tracks in the current RestOfEvent are from a B0bar.");
    REGISTER_VARIABLE("isRestOfEventEmpty", isRestOfEventEmpty,
                      "Returns the amount of tracks in the RestOfEvent related to the given Particle. -2 If ROE is empty.");
    REGISTER_VARIABLE("isRelatedRestOfEventB0Flavor", isRelatedRestOfEventB0Flavor,
                      "0 (1) if the RestOfEvent related to the given Particle is related to a B0bar (B0). The MCError of Breco has to be 0 or 1, the output of the variable is -2 otherwise.");
    REGISTER_VARIABLE("qrCombined", isRestOfEventB0Flavor,
                      "0 (1) if current RestOfEvent is related to a B0bar (B0). The MCError of Breco has to be 0 or 1, the output of the variable is -2 otherwise.");
    REGISTER_VARIABLE("isRelatedRestOfEventMajorityB0Flavor", isRelatedRestOfEventMajorityB0Flavor,
                      " 0 (1) if the majority of tracks and clusters of the RestOfEvent related to the given Particle are related to a B0bar (B0).");
    REGISTER_VARIABLE("isRestOfEventMajorityB0Flavor", isRestOfEventMajorityB0Flavor,
                      "0 (1) if the majority of tracks and clusters of the current RestOfEvent are related to a B0bar (B0).");
    REGISTER_VARIABLE("McFlavorOfTagSide",  McFlavorOfTagSide, "Flavor of tag side from MC extracted from the RoE");


    VARIABLE_GROUP("Flavor Tagger MetaFunctions")

    REGISTER_VARIABLE("CheckingVariables(ListName, requestedVariable)", CheckingVariables,
                      "FlavorTagging:[Eventbased] Available checking variables are getListSize for particle lists.");
    REGISTER_VARIABLE("IsDaughterOf(variable)", IsDaughterOf, "Check if the particle is a daughter of the given list.");

    REGISTER_VARIABLE("NumberOfKShortinROEParticleList(particleListName)", NumberOfKShortinROEParticleList,
                      "Returns the number of K_S0 in the given ROE ParticleList. The possibilities are K_S0:ROEKaon or K_S0:ROELambda");
    REGISTER_VARIABLE("SemiLeptonicVariables(requestedVariable)", SemiLeptonicVariables,
                      "FlavorTagging:[Eventbased] Kinematical variables (recoilMass, pMissCMS, cosThetaMissCMS or EW90) assuming a semileptonic decay with the given particle as target.");
    REGISTER_VARIABLE("KaonPionVariables(requestedVariable)"  , KaonPionVariables ,
                      " Kinematical variables for KaonPion category (cosKaonPion or HaveOpositeCharges)");
    REGISTER_VARIABLE("FSCVariables(requestedVariable)", FSCVariables,
                      "FlavorTagging:[Eventbased] Kinematical variables for FastSlowCorrelated category (pFastCMS, cosSlowFast, SlowFastHaveOpositeCharges, or cosTPTOFast).");
    REGISTER_VARIABLE("hasHighestProbInCat(particleListName, extraInfoName)", hasHighestProbInCat,
                      "Returns 1.0 if the given Particle is classified as target track, i.e. if it has the highest target track probability in particlelistName. The probability is accessed via extraInfoName.");
    REGISTER_VARIABLE("HighestProbInCat(particleListName, extraInfoName)", HighestProbInCat,
                      "Returns the highest target track probability value for the given category")

    REGISTER_VARIABLE("isRightTrack(particleName)", isRightTrack,
                      "Checks if the given Particle was really from a B. 1.0 if true otherwise 0.0");
    REGISTER_VARIABLE("isRightCategory(particleName)", isRightCategory,
                      "FlavorTagging: returns 1 if the class track by particleName category has the same flavor as the MC target track 0 else also if there is no target track");
    REGISTER_VARIABLE("QrOf(particleListName, extraInfoRightCategory, extraInfoRightTrack)", QrOf,
                      "FlavorTagging: [Eventbased] q*r where r is calculated from the output of event level in particlelistName.");
    REGISTER_VARIABLE("weightedQrOf(particleListName, extraInfoRightCategory, extraInfoRightTrack)", weightedQrOf,
                      "FlavorTagging: [Eventbased] weighted q*r where r is calculated from the output of event level for the 3 particles with highest track probability in particlelistName.");



  }
}
