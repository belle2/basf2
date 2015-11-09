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

    // Flavour tagging variables

    double isMajorityInRestOfEventFromB0(const Particle*)
    {
      int vote = 0;
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
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
      return vote > 0;
    }

    double isMajorityInRestOfEventFromB0bar(const Particle*)
    {
      int vote = 0;
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
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
      return vote < 0;
    }

    double isRestOfEventEmpty(const Particle* part)
    {
      const RestOfEvent* roe = part->getRelatedTo<RestOfEvent>();
      float ObjectsInROE = 0; //Flavor of B
      if (roe-> getNTracks() != 0) {
        ObjectsInROE++;
//       } else if (roe-> getNECLClusters() != 0) {
//         ObjectsInROE++;
//       } else if (roe-> getNKLMClusters() != 0) {
//         ObjectsInROE++;
      }
      if (ObjectsInROE > 0) {
        return ObjectsInROE;
      } else return -2;
    }

    double isRelatedRestOfEventB0Flavor(const Particle* part)
    {
      const RestOfEvent* roe = part->getRelatedTo<RestOfEvent>();
      float q_MC = 0; //Flavor of B
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
      if (q_MC > 0) {
        return 1;
      } else if (q_MC < 0) {
        return 0;
      } else return -2;
    }

    double isRestOfEventB0Flavor(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      float q_MC = 0; //Flavor of B
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

    double isLambda(const Particle* particle)
    {
      const MCParticle* mcparticle = particle->getRelatedTo<MCParticle>();
      // if (mcparticle ==nullptr)
      //  return 0.0;
      if (mcparticle->getPDG() == 3122) return 0.0; //Lambda0
      else if (mcparticle->getPDG() == -3122) return 0.0; //Anti-Lambda0
      else return 1.0;
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
      for (const auto& track : roe->getTracks()) {
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
      for (auto& track : roe->getTracks()) {
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
        double prob_mu = p_mu->getExtraInfo("IsRightTrack(Muon)");
        if (prob_mu > maximum_prob_mu) {
          maximum_prob_mu = prob_mu;
          track_target_mu = p_mu -> getTrack();
        }
      }
      for (unsigned int i = 0; i < ElectronList->getListSize(); ++i) {
        Particle* p_el = ElectronList->getParticle(i);
//             const Particle* p_el = p ->getRelated<PartList>();
        double prob_el = p_el->getExtraInfo("IsRightTrack(Electron)");
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
      StoreObjPtr<ParticleList> SlowPionList("pi+:SlowPionROE");
//       StoreObjPtr<RestOfEvent> roe("RestOfEvent");

//       double maximum_prob_K = 0;
      double maximum_prob_pi = 0;

      PCmsLabTransform T;
      TLorentzVector momTarget_K = T.rotateLabToCms() * particle -> get4Vector();
      TLorentzVector momTarget_pi;

//       for (unsigned int i = 0; i < KaonList->getListSize(); ++i) {
//         Particle* p_K = KaonList->getParticle(i);
//         double prob_K = p_K->getExtraInfo("IsRightTrack(Kaon)");
//         if (prob_K > maximum_prob_K) {
//           maximum_prob_K = prob_K;
//           momTarget_K = T.rotateLabToCms() * p_K -> get4Vector();
//         }
//       }
      for (unsigned int i = 0; i < SlowPionList->getListSize(); ++i) {
        Particle* p_pi = SlowPionList->getParticle(i);
        double prob_pi = p_pi->getExtraInfo("IsRightTrack(SlowPion)");
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
      StoreObjPtr<ParticleList> SlowPionList("pi+:SlowPionROE");
//       StoreObjPtr<RestOfEvent> roe("RestOfEvent");

//       double maximum_prob_K = 0;
      double maximum_prob_pi = 0;

      float chargeTarget_K = particle -> getCharge();
      float chargeTarget_pi = 0;

      for (unsigned int i = 0; i < SlowPionList->getListSize(); ++i) {
        Particle* p_pi = SlowPionList->getParticle(i);
        double prob_pi = p_pi->getExtraInfo("IsRightTrack(SlowPion)");
        if (prob_pi > maximum_prob_pi) {
          maximum_prob_pi = prob_pi;
          chargeTarget_pi =  p_pi -> getCharge();
        }
      }
      if (chargeTarget_K * chargeTarget_pi == -1) {
        return 1;
      } else return 0;
    }

    //FLAVOR TAGGING RELATED:

    Manager::FunctionPtr QrOf(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 3) {
        auto particleListName = arguments[0];
        auto extraInfoRightCategory = arguments[1];
        auto extraInfoRightTrack = arguments[2];
        auto func = [particleListName, extraInfoRightCategory, extraInfoRightTrack](const Particle*) -> double {
          StoreObjPtr<ParticleList> ListOfParticles(particleListName);
          PCmsLabTransform T;
          Particle* target = nullptr; //Particle selected as target
          float maximum_p_track = 0; //Probability of being the target track from the track level
          float prob = 0; //The probability of beeing right classified flavor from the event level
          float maximum_q = 0; //Flavour of the track selected as target
          if (ListOfParticles)
          {
            if (ListOfParticles->getListSize() > 0) {
              for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i) {
                Particle* particle = ListOfParticles->getParticle(i);
                double x = 0;
                if (extraInfoRightTrack == "IsRightTrack(MaximumPstar)") {
                  x = (T.rotateLabToCms() * particle->get4Vector()).P();
                } else x = particle->getExtraInfo(extraInfoRightTrack);
                if (x > maximum_p_track) {
                  maximum_p_track = x;
                  target = particle;
                }
              }
              if (target != nullptr) {
                prob = target -> getExtraInfo(extraInfoRightCategory); //Gets the probability of beeing right classified flavor from the event level
                maximum_q = target -> getCharge(); //Gets the flavor of the track selected as target
                if (extraInfoRightTrack == "IsRightTrack(Lambda)") {
                  maximum_q = target->getPDGCode() / TMath::Abs(target->getPDGCode());
                }
              }
            }
          }
          //float r = TMath::Abs(2 * prob - 1); //Definition of the dilution factor  */
          //return 0.5 * (maximum_q * r + 1);
//            if (extraInfoRightTrack == ("IsRightTrack(IntermediateElectron)" || "IsRightTrack(IntermediateMuon)" || "IsRightTrack(SlowPion)" || "IsRightTrack(FSC)" || "IsRightTrack(Lambda)")) return -1 * maximum_q * prob;
          return maximum_q * prob;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (3 required) for meta function QrOf");
      }
    }

    Manager::FunctionPtr InputQrOf(const std::vector<std::string>& arguments)
    {
      //used by simple_flavor_tagger
      if (arguments.size() == 3) {
        auto particleListName = arguments[0];
        auto extraInfoRightCategory = arguments[1];
        auto extraInfoRightTrack = arguments[2];
        auto func = [particleListName, extraInfoRightCategory, extraInfoRightTrack](const Particle*) -> double {
          if (particleListName == "K+:KaonROE" || particleListName == "Lambda0:LambdaROE")
          {
            double flavor = 0.0;
            double r = 0.0;
            double qr = 0.0;
            double final_value = 0.0;
            double val1 = 1.0;
            double val2 = 1.0;
            auto compare = [extraInfoRightTrack](const Particle * part1, const Particle * part2)-> bool {
              double info1 = part1->getExtraInfo(extraInfoRightTrack);
              double info2 = part2->getExtraInfo(extraInfoRightTrack);
              return (info1 > info2);
            };
            StoreObjPtr<ParticleList> ListOfParticles(particleListName);
            if (ListOfParticles) {
              if (ListOfParticles->getListSize() > 0) {
                std::vector<const Particle*> ParticleVector;
                ParticleVector.reserve(ListOfParticles->getListSize());
                for (unsigned int i = 0; i < ListOfParticles->getListSize(); i++) {
                  ParticleVector.push_back(ListOfParticles->getParticle(i));
                }
                std::sort(ParticleVector.begin(), ParticleVector.end(), compare);
                if (particleListName == "Lambda0:LambdaROE") {
                  //Loop over Lambda vector until 3 or empty
                  if (ParticleVector.size() != 0) final_value = 1.0;
                  for (unsigned int i = 0; i < ParticleVector.size(); ++i) {
                    //PDG Code Lambda0 3122 (indicates a B0bar)
                    if (ParticleVector[i]->getPDGCode() == 3122) flavor = -1.0;
                    else if (ParticleVector[i]->getPDGCode() == -3122) flavor = 1.0;
                    else {flavor = 0.0;}
                    r = ParticleVector[i]->getExtraInfo(extraInfoRightTrack);
                    qr = (flavor * r);
                    val1 = val1 * (1 + qr);
                    val2 = val2 * (1 - qr);
                  }
                  final_value = (val1 - val2) / (val1 + val2);
                } else if (particleListName == "K+:KaonROE") {
                  //Loop over K+ vector until 3 or empty
                  if (ParticleVector.size() != 0) final_value = 1.0;
                  for (unsigned int i = 0; i < ParticleVector.size(); i++) {
                    flavor = ParticleVector[i]->getCharge();
                    r = ParticleVector[i]->getExtraInfo(extraInfoRightTrack);
                    qr = (flavor * r);
                    val1 = val1 * (1 + qr);
                    val2 = val2 * (1 - qr);
                  }
                  final_value = (val1 - val2) / (val1 + val2);
                }
              }
            }


            return final_value;
          }

          //SlowPion, Electron, Muon
          else{
            StoreObjPtr<EventExtraInfo> Info("EventExtraInfo");
            StoreObjPtr<ParticleList> ListOfParticles(particleListName);
            double flavor = 0.0; //Flavour of the track selected as target
            double maximum_p_track = 0.0; //Probability of being the target track from the track level
            double final_value = 0.0;
            for (unsigned int i = 0; i < ListOfParticles->getListSize(); i++)
            {
              Particle* p = ListOfParticles->getParticle(i);
              double x = p->getExtraInfo(extraInfoRightTrack);
              if (x > maximum_p_track) {
                maximum_p_track = x;
                //In case of slowPions and intermediate momentum leptons sign is flipped
                if (particleListName == "pi+:SlowPionROE" && extraInfoRightTrack == "IsRightTrack(SlowPion)") flavor = (-1.0) * p->getCharge();
                else if (particleListName == "e+:IntermediateElectronROE"
                && extraInfoRightTrack == "IsRightTrack(IntermediateElectron)") flavor = (-1.0) * p->getCharge();
                else if (particleListName == "mu+:IntermediateMuonROE"
                && extraInfoRightTrack == "IsRightTrack(IntermediateMuon)") flavor = (-1.0) * p->getCharge();
                else flavor = p->getCharge();
              }
            }
            final_value = flavor * maximum_p_track;
            return final_value;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (3 required) for meta function QrOf");
      }
    }

    Manager::FunctionPtr IsRightCategory(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto particleName = arguments[0];
        auto func = [particleName](const Particle * particle) -> double {
          Particle* nullpart = nullptr;
          float maximum_q = 0;
          float qMC = 0;
          int maximum_PDG = 0;
          int maximum_PDG_Mother = 0;
          int maximum_PDG_Mother_Mother = 0;
          const MCParticle* MCp = particle ->getRelated<MCParticle>();
          maximum_q = particle -> getCharge();
          qMC = 2 * (Variable::Manager::Instance().getVariable("qrCombined")->function(nullpart) - 0.5);
          if (MCp != nullptr)
          {
            maximum_PDG = TMath::Abs(MCp->getPDG());
            if (particleName != "Lambda" && MCp->getMother() != nullptr) {
              maximum_PDG_Mother = TMath::Abs(MCp->getMother()->getPDG());
            }
            //for Kaons and SlowPions we need the mother of the mother for the particle
            if ((particleName == "Kaon" || particleName == "SlowPion" || particleName == "IntermediateElectron"
                 || particleName == "IntermediateMuon" || particleName == "FastPion")
                && MCp->getMother()->getMother() != nullptr) maximum_PDG_Mother_Mother =  TMath::Abs(MCp->getMother()->getMother()->getPDG());
          } else {
            maximum_PDG = 0;
            maximum_PDG_Mother = 0;
          }
//           float SlowPion_q = 0;
          int SlowPion_PDG = 0;
          int SlowPion_PDG_Mother = 0;
          if (particleName == "KaonPion")
          {
            StoreObjPtr<ParticleList> SlowPionList("pi+:SlowPionROE");
            double maximum_prob_pi = 0;
            Particle* TargetSlowPion = nullptr;
            for (unsigned int i = 0; i < SlowPionList->getListSize(); ++i) {
              Particle* p_pi = SlowPionList->getParticle(i);
              double prob_pi = p_pi->getExtraInfo("IsRightTrack(SlowPion)");
              if (prob_pi > maximum_prob_pi) {
                maximum_prob_pi = prob_pi;
                TargetSlowPion = p_pi;
              }
              const MCParticle* MCSlowPion = TargetSlowPion ->getRelated<MCParticle>();
//               SlowPion_q = TargetSlowPion -> getCharge();
              if (MCSlowPion->getMother() != nullptr && MCSlowPion->getMother()->getMother() != nullptr) {
                SlowPion_PDG = TMath::Abs(MCSlowPion->getPDG());
                SlowPion_PDG_Mother = TMath::Abs(MCSlowPion->getMother()->getPDG());
              }
            }
          }
//           float FastParticle_q = 0;
          int FastParticle_PDG_Mother = 0;
          if (particleName == "FSC")
          {
            StoreObjPtr<ParticleList> FastParticleList("pi+:FastPionROE");
            PCmsLabTransform T;
            double maximum_prob_fast = 0;
            Particle* TargetFastParticle = nullptr;
            for (unsigned int i = 0; i < FastParticleList->getListSize(); ++i) {
              Particle* p_fast = FastParticleList->getParticle(i);
              double prob_fast = (T.rotateLabToCms() * p_fast -> get4Vector()).P();
              if (prob_fast > maximum_prob_fast) {
                maximum_prob_fast = prob_fast;
                TargetFastParticle = p_fast;
              }
              const MCParticle* MCFastParticle = TargetFastParticle ->getRelated<MCParticle>();
//               FastParticle_q = TargetFastParticle -> getCharge();
              if (MCFastParticle->getMother() != nullptr && MCFastParticle->getMother()->getMother() != nullptr) {
                FastParticle_PDG_Mother = TMath::Abs(MCFastParticle->getMother()->getPDG());
              }
            }
          }
          if (particleName == "Electron"
              && ((maximum_q == qMC && maximum_PDG == 11 && maximum_PDG_Mother == 511)
                  || (maximum_q != qMC && maximum_PDG == 11 && maximum_PDG_Mother_Mother == 511)))
          {
            return 1.0;
          } else if (particleName == "IntermediateElectron"
                     && maximum_q != qMC && maximum_PDG == 11 && maximum_PDG_Mother_Mother == 511)
          {
            return 1.0;
          } else if (particleName == "Muon"
                     && ((maximum_q == qMC && maximum_PDG == 13 && maximum_PDG_Mother == 511)
                         || (maximum_q != qMC && maximum_PDG == 13 && maximum_PDG_Mother_Mother == 511)))
          {
            return 1.0;
          } else if (particleName == "IntermediateMuon"
                     && maximum_q != qMC && maximum_PDG == 13 && maximum_PDG_Mother_Mother == 511)
          {
            return 1.0;
          }  else if (particleName == "KinLepton"
                      && maximum_q == qMC && (maximum_PDG == 11 || maximum_PDG == 13) && maximum_PDG_Mother == 511)
          {
            return 1.0;
          } else if (particleName == "Kaon" && maximum_q == qMC
                     && maximum_PDG == 321 && maximum_PDG_Mother > 400 && maximum_PDG_Mother < 500 && maximum_PDG_Mother_Mother == 511)
          {
            return 1.0;
          } else if (particleName == "SlowPion" && maximum_q != qMC
                     && maximum_PDG == 211 && maximum_PDG_Mother == 413 && maximum_PDG_Mother_Mother == 511)
          {
            return 1.0;
          } else if (particleName == "KaonPion" && maximum_q == qMC
                     && maximum_PDG == 321 && SlowPion_PDG == 211 && maximum_PDG_Mother == SlowPion_PDG_Mother)
          {
            return 1.0;
          } else if (particleName == "FastPion" && maximum_q == qMC
                     && maximum_PDG == 211 && maximum_PDG_Mother == 511)
          {
            return 1.0;
          } else if (particleName == "MaximumPstar" && maximum_q == qMC)
          {
            return 1.0;
          } else if (particleName == "FSC" && maximum_q != qMC
                     && maximum_PDG == 211 && FastParticle_PDG_Mother == 511)
          {
            return 1.0;
          } else if (particleName == "Lambda" && (particle->getPDGCode() / TMath::Abs(particle->getPDGCode())) != qMC
                     && maximum_PDG == 3122)
          {
            return 1.0;
          } else {
            return 0.0;
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (1 required) for meta function IsRightCategory");
      }
    }

    Manager::FunctionPtr IsRightTrack(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto particleName = arguments[0];
        auto func = [particleName](const Particle * part) -> double {
          const MCParticle* mcParticle = part->getRelated<MCParticle>();
          if (mcParticle == nullptr) return 0.0;
          //direct electron
          else if (particleName == "Electron"
          && ((mcParticle->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 11
          && TMath::Abs(mcParticle->getMother()->getPDG()) == 511)
          || (mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 11
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511)))
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
          && ((mcParticle->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 13
          && TMath::Abs(mcParticle->getMother()->getPDG()) == 511)
          || (mcParticle->getMother() != nullptr
          && mcParticle->getMother()->getMother() != nullptr
          && TMath::Abs(mcParticle->getPDG()) == 13
          && TMath::Abs(mcParticle->getMother()->getMother()->getPDG()) == 511))
                    )
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
        B2FATAL("Wrong number of arguments (1 required) for meta function IsRightTrack");
      }
    }

    Manager::FunctionPtr hasHighestProbInCat(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        auto particleListName = arguments[0];
        auto extraInfoName = arguments[1];
        auto func = [particleListName, extraInfoName](const Particle * particle) -> double {
          StoreObjPtr<ParticleList> ListOfParticles(particleListName);
          PCmsLabTransform T;
          double maximum_prob = 0;
          for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i)
          {
            Particle* p = ListOfParticles->getParticle(i);
            double prob = 0;
            if (extraInfoName == "IsRightTrack(MaximumPstar)") {
              prob = (T.rotateLabToCms() * p->get4Vector()).P();
            } else prob = p->getExtraInfo(extraInfoName);
            if (prob > maximum_prob) {
              maximum_prob = prob;
            }
          }
          if ((extraInfoName == "IsRightTrack(MaximumPstar)" && (T.rotateLabToCms() * particle -> get4Vector()).P() == maximum_prob) ||
              (extraInfoName != "IsRightTrack(MaximumPstar)" && particle -> getExtraInfo(extraInfoName) == maximum_prob))
          {
            return 1.0;
          } else return 0.0;
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
          StoreObjPtr<ParticleList> ListOfParticles(particleListName);
          double maximum_prob = 0;
          for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i)
          {
            Particle* p = ListOfParticles->getParticle(i);
            double prob = p->getExtraInfo(extraInfoName);
            if (prob > maximum_prob) {
              maximum_prob = prob;
            }
          }
          return maximum_prob;
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (2 required) for meta function hasHighestProbInCat");
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
          StoreObjPtr<RestOfEvent> roe("RestOfEvent");
          const auto& tracks = roe->getTracks();
          for (auto& x : tracks)
          {
            const TrackFitResult* tracki = x->getTrackFitResult(x->getRelated<PIDLikelihood>()->getMostLikely());
            if (tracki == nullptr || particle->getTrack() == x) continue;
            TLorentzVector momtrack(tracki->getMomentum(), 0);
            momXchargedtracks += T.rotateLabToCms() * momtrack;
          }
          const auto& ecl = roe->getECLClusters();
          for (auto& x : ecl)
          {
            if (x->isNeutral()) momXneutralclusters += T.rotateLabToCms() * x -> get4Vector();
            if (!(x->isNeutral())) momXchargedclusters += T.rotateLabToCms() * x -> get4Vector();
          }
          const auto& klm = roe->getKLMClusters();
          for (auto& x : klm)
          {
            if (!(x -> getAssociatedTrackFlag()) && !(x -> getAssociatedEclClusterFlag())) {
              momXneutralclusters += T.rotateLabToCms() * x -> getMomentum();
            }
          }
          TLorentzVector momXcharged(momXchargedtracks.Vect(), momXchargedclusters.E());
          TLorentzVector momX = (momXcharged + momXneutralclusters) - momTarget; //Total Momentum of the recoiling X in CMS-System
          TLorentzVector momMiss = -(momX + momTarget); //Momentum of Anti-v  in CMS-System
          if (requestedVariable == "recoilMass") return momX.M();
          else if (requestedVariable == "p_missing_CMS") return momMiss.Vect().Mag();
          else if (requestedVariable == "CosTheta_missing_CMS") return TMath::Cos(momTarget.Angle(momMiss.Vect()));
          else if (requestedVariable == "EW90")
          {
            TLorentzVector momW = momTarget + momMiss; //Momentum of the W-Boson in CMS
            float E_W_90 = 0 ; // Energy of all charged and neutral clusters in the hemisphere of the W-Boson
            for (auto& i : ecl) {
              if ((T.rotateLabToCms() * i -> get4Vector()).Vect().Dot(momW.Vect()) > 0) E_W_90 += i -> getEnergy();
            }
//       for (auto & i : klm) {
//         if ((T.rotateLabToCms() * i -> getMomentum()).Vect().Dot(momW.Vect()) > 0) E_W_90 +=;
//         }
            return E_W_90;
          } else {
            B2FATAL("Wrong variable requested. The possibilities are recoilMass, p_missing_CMS, CosTheta_missing_CMS or EW90");
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (1 required) for meta function SemiLeptonicVariables");
      }
    }

    Manager::FunctionPtr FSCVariables(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto requestedVariable = arguments[0];
        auto func = [requestedVariable](const Particle * particle) -> double {
          StoreObjPtr<ParticleList> FastParticleList("pi+:FastPionROE");
          PCmsLabTransform T;
          double maximum_prob_fast = 0;
          Particle* TargetFastParticle = nullptr;
          if ((requestedVariable == "p_CMS_Fast") || (requestedVariable == "cosSlowFast") || (requestedVariable == "cosTPTO_Fast") || (requestedVariable == "SlowFastHaveOpositeCharges"))
          {
            if (FastParticleList) {
              for (unsigned int i = 0; i < FastParticleList->getListSize(); ++i) {
                Particle* p_fast = FastParticleList->getParticle(i);
                double prob_fast = (T.rotateLabToCms() * p_fast -> get4Vector()).P();
                if (prob_fast > maximum_prob_fast) {
                  maximum_prob_fast = prob_fast;
                  TargetFastParticle = p_fast;
                }
              }
              TLorentzVector momSlowPion = T.rotateLabToCms() * particle -> get4Vector();  //Momentum of Slow Pion in CMS-System
              TLorentzVector momFastParticle = T.rotateLabToCms() * TargetFastParticle -> get4Vector();  //Momentum of Slow Pion in CMS-System

              if (requestedVariable == "cosSlowFast") return TMath::Cos(momSlowPion.Angle(momFastParticle.Vect()));
              else if (requestedVariable == "cosTPTO_Fast") return Variable::Manager::Instance().getVariable("cosTPTO")->function(
                                                                       TargetFastParticle);
              else if (requestedVariable == "SlowFastHaveOpositeCharges") {
                if (particle->getCharge()*TargetFastParticle->getCharge() == -1) {
                  return 1;
                } else return 0;
              } else return momFastParticle.P();
            } else return 0;
          } else {
            B2FATAL("Wrong variable requested. The possibilities are p_CMS_Fast, cosSlowFast or cosTPTO_Fast");
          }
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments (1 required) for meta function FSCVariables");
      }
    }

    Manager::FunctionPtr CheckingVariables(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 2) {
        auto ListName = arguments[0];
        auto requestedVariable = arguments[1];
        auto func = [requestedVariable, ListName](const Particle*) -> double {
          if (requestedVariable == "getListSize")
          {
            StoreObjPtr<ParticleList> ListOfParticles(ListName);
            return ListOfParticles->getListSize();
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
        for (unsigned int i = 0; i < Y->getListSize(); ++i)
        {
          const auto& x = Y->getParticle(i)->getDaughters();
          daughters.insert(daughters.end(), x.begin(), x.end());
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



    VARIABLE_GROUP("Flavour tagging");
    REGISTER_VARIABLE("isMajorityInRestOfEventFromB0", isMajorityInRestOfEventFromB0,
                      "[Eventbased] Check if the majority of the tracks in the current RestOfEvent are from a B0.");
    REGISTER_VARIABLE("isMajorityInRestOfEventFromB0bar", isMajorityInRestOfEventFromB0bar,
                      "[Eventbased] Check if the majority of the tracks in the current RestOfEvent are from a B0bar.");
    REGISTER_VARIABLE("isRestOfEventEmpty", isRestOfEventEmpty,
                      "Returns the amount of tracks in the RestOfEvent related to the given Particle. -2 If ROE is empty.");
    REGISTER_VARIABLE("isRelatedRestOfEventB0Flavor", isRelatedRestOfEventB0Flavor,
                      "0 (1) if the RestOfEvent related to the given Particle is related to a B0bar (B0).");
    REGISTER_VARIABLE("qrCombined", isRestOfEventB0Flavor,  "0 (1) if current RestOfEvent is related to a B0bar (B0).");
    REGISTER_VARIABLE("p_miss", p_miss,  "Calculates the missing Momentum for a given particle on the tag side.");
    REGISTER_VARIABLE("NumberOfKShortinRemainingROEKaon", NumberOfKShortinRemainingROEKaon,
                      "Returns the number of K_S0 in the remainging Kaon ROE.");
    REGISTER_VARIABLE("NumberOfKShortinRemainingROELambda", NumberOfKShortinRemainingROELambda,
                      "Returns the number of K_S0 in the remainging Lambda ROE.");
    REGISTER_VARIABLE("lambdaFlavor", lambdaFlavor,  "1.0 if Lambda0, -1.0 if Anti-Lambda0, 0.0 else.");
    REGISTER_VARIABLE("isLambda", isLambda,  "0.0 if MCLambda0, 1.0 else.");
    REGISTER_VARIABLE("lambdaZError", lambdaZError,  "Returns the Matrixelement[2][2] of the PositionErrorMatrix of the Vertex fit.");
    REGISTER_VARIABLE("MomentumOfSecondDaughter", MomentumOfSecondDaughter,
                      "Returns the Momentum of second daughter if existing, else 0.");
    REGISTER_VARIABLE("MomentumOfSecondDaughter_CMS", MomentumOfSecondDaughter_CMS,
                      "Returns the Momentum of second daughter if existing in CMS, else 0.");
    REGISTER_VARIABLE("chargeTimesKaonLiklihood", chargeTimesKaonLiklihood,
                      "Returns the q*(highest PID_Likelihood for Kaons), else 0.");
    REGISTER_VARIABLE("ptTracksRoe", transverseMomentumOfChargeTracksInRoe,
                      "Returns the transverse momentum of all charged tracks if there exists a ROE for the given particle, else 0.");
    REGISTER_VARIABLE("McFlavorOfTagSide",  McFlavorOfTagSide, "Flavour of tag side from MC extracted from the RoE");
    REGISTER_VARIABLE("BtagClassFlavor",  particleClassifiedFlavor,    "Flavour of Btag from trained Method");
    REGISTER_VARIABLE("BtagMCFlavor",  particleMCFlavor,    "Flavour of Btag from MC");
    REGISTER_VARIABLE("isInElectronOrMuonCat", isInElectronOrMuonCat,
                      "Returns 1.0 if the particle has been selected as target in the Muon or Electron Category, 0.0 else.");
    REGISTER_VARIABLE("cosKaonPion"  , cosKaonPion ,
                      "cosine of angle between kaon and slow pion momenta, i.e. between the momenta of the particles selected as target kaon and slow pion");
    REGISTER_VARIABLE("ImpactXY"  , ImpactXY , "The impact parameter of the given particle in the xy plane");
    REGISTER_VARIABLE("KaonPionHaveOpositeCharges", KaonPionHaveOpositeCharges,
                      "Returns 1 if the particles selected as target kaon and slow pion have oposite charges, 0 else")
    VARIABLE_GROUP("MetaFunctions FlavorTagging")
    REGISTER_VARIABLE("InputQrOf(particleListName, extraInfoRightCategory, extraInfoRightTrack)", InputQrOf,
                      "FlavorTagging: [Eventbased] q*r where r is calculated from the output of event level in particlelistName.");
    REGISTER_VARIABLE("QrOf(particleListName, extraInfoRightCategory, extraInfoRightTrack)", QrOf,
                      "FlavorTagging: [Eventbased] q*r where r is calculated from the output of event level in particlelistName.");
    REGISTER_VARIABLE("IsRightCategory(particleName)", IsRightCategory,
                      "FlavorTagging: returns 1 if the class track by particleName category has the same flavor as the MC target track 0 else also if there is no target track");
    REGISTER_VARIABLE("IsRightTrack(particleName)", IsRightTrack,
                      "Checks if the given Particle was really from a B. 1.0 if true otherwise 0.0");
    REGISTER_VARIABLE("hasHighestProbInCat(particleListName, extraInfoName)", hasHighestProbInCat,
                      "Returns 1.0 if the given Particle is classified as target, i.e. if it has the highest probability in particlelistName. The probability is accessed via extraInfoName.");
    REGISTER_VARIABLE("SemiLeptonicVariables(requestedVariable)", SemiLeptonicVariables,
                      "FlavorTagging:[Eventbased] Kinematical variables (recoilMass, p_missing_CMS, CosTheta_missing_CMS or EW90) assuming a semileptonic decay with the given particle as target.");
    REGISTER_VARIABLE("FSCVariables(requestedVariable)", FSCVariables,
                      "FlavorTagging:[Eventbased] Kinematical variables for FastSlowCorrelated category (p_CMS_Fast, cosSlowFast, SlowFastHaveOpositeCharges, or cosTPTO_Fast).");
    REGISTER_VARIABLE("CheckingVariables(ListName, requestedVariable)", CheckingVariables,
                      "FlavorTagging:[Eventbased] Available checking variables are getListSize for particle lists.");
    REGISTER_VARIABLE("HighestProbInCat(particleListName, extraInfoName)", HighestProbInCat,
                      "Returns the highest probability value for the given category")
    REGISTER_VARIABLE("IsDaughterOf(variable)", IsDaughterOf, "Check if the particle is a daughter of the given list.");

    REGISTER_VARIABLE("cosTPTO"  , cosTPTO , "cosine of angle between thrust axis of given particle and thrust axis of ROE");

  }
}
