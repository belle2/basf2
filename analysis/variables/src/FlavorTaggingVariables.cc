/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/variables/FlavorTaggingVariables.h>
#include <analysis/variables/ROEVariables.h>

#include <analysis/variables/MCTruthVariables.h>
#include <analysis/variables/KLMClusterVariables.h>

#include <analysis/ClusterUtility/ClusterUtils.h>

#include <analysis/utility/MCMatching.h>
#include <analysis/utility/PCmsLabTransform.h>

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

#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <framework/geometry/B2Vector3.h>

#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

    static const double realNaN = std::numeric_limits<double>::quiet_NaN();
    //   ############################################## FlavorTagger Variables   ###############################################

    // Track Level Variables ---------------------------------------------------------------------------------------------------

    double momentumMissingTagSide(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (!roe.isValid()) return 0;

      ROOT::Math::PxPyPzEVector roeCMSVec;

      const auto& roeChargedParticles = roe->getChargedParticles();
      for (auto roeChargedParticle : roeChargedParticles) {
        roeCMSVec += PCmsLabTransform::labToCms(roeChargedParticle->get4Vector());
      }

      double missMom = -roeCMSVec.P();
      return missMom ;
    }

    Manager::FunctionPtr momentumMissingTagSideWithMask(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      if (arguments.size() == 0)
        maskName = RestOfEvent::c_defaultMaskName;
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("At most 1 argument (name of mask) accepted.");

      auto func = [maskName](const Particle*) -> double {
        StoreObjPtr<RestOfEvent> roe("RestOfEvent");
        if (!roe.isValid()) return 0;

        ROOT::Math::PxPyPzEVector roeCMSVec;

        const auto& roeChargedParticles = roe->getChargedParticles(maskName);
        for (auto roeChargedParticle : roeChargedParticles)
        {
          roeCMSVec += PCmsLabTransform::labToCms(roeChargedParticle->get4Vector());
        }
        double missMom = -roeCMSVec.P();
        return missMom ;
      };
      return func;
    }

    double cosTPTO(const Particle* part)
    {
      StoreObjPtr<RestOfEvent> roeobject("RestOfEvent");
      const RestOfEvent* roe;
      if (roeobject.isValid()) { // if in for_each loop over ROE
        roe = &(*roeobject);
      } else {
        roe = getRelatedROEObject(part);
        if (!roe)
          return 0;
      }

      std::vector<ROOT::Math::XYZVector> p3_cms_roe;
      static const double P_MAX(3.2);

      // Charged tracks
      const auto& roeTracks = roe->getChargedParticles();
      for (auto& roeChargedParticle : roeTracks) {
        // TODO: Add helix and KVF with IpProfile once available. Port from L163-199 of:
        // /belle/b20090127_0910/src/anal/ekpcontsuppress/src/ksfwmoments.cc
        ROOT::Math::PxPyPzEVector p_cms = PCmsLabTransform::labToCms(roeChargedParticle->get4Vector());
        if (p_cms != p_cms) continue;
        if (p_cms.P() > P_MAX) continue;
        p3_cms_roe.push_back(p_cms.Vect());
      }

      // ECLCluster->Gamma
      const auto& roePhotons = roe->getPhotons();
      for (auto& roePhoton : roePhotons) {
        if (roePhoton->getECLClusterEHypothesisBit() == ECLCluster::EHypothesisBit::c_nPhotons) {
          ROOT::Math::PxPyPzEVector p_lab = roePhoton->get4Vector();
          if (p_lab != p_lab) continue;
          if (p_lab.P() < 0.05) continue;
          ROOT::Math::PxPyPzEVector p_cms = PCmsLabTransform::labToCms(p_lab);
          if (p_cms != p_cms) continue;
          if (p_cms.P() > P_MAX) continue;
          p3_cms_roe.push_back(p_cms.Vect());
        }
      }

      const auto& roeKlongs = roe->getHadrons();
      for (auto& roeKlong : roeKlongs) {
        if (nKLMClusterTrackMatches(roeKlong) == 0 && !(roeKlong->getKLMCluster()->getAssociatedEclClusterFlag())) {
          ROOT::Math::PxPyPzEVector p_lab = roeKlong->get4Vector();
          if (p_lab != p_lab) continue;
          if (p_lab.P() < 0.05) continue;
          ROOT::Math::PxPyPzEVector p_cms = PCmsLabTransform::labToCms(p_lab);
          if (p_cms != p_cms) continue;
          if (p_cms.P() > P_MAX) continue;
          p3_cms_roe.push_back(p_cms.Vect());
        }
      }

      const B2Vector3D thrustO = Thrust::calculateThrust(p3_cms_roe);
      const B2Vector3D pAxis = PCmsLabTransform::labToCms(part->get4Vector()).Vect();

      double result = 0 ;
      if (pAxis == pAxis) result = abs(cos(pAxis.Angle(thrustO)));

      return result;
    }

    Manager::FunctionPtr cosTPTOWithMask(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      if (arguments.size() == 0)
        maskName = RestOfEvent::c_defaultMaskName;
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("At most 1 argument (name of mask) accepted.");

      auto func = [maskName](const Particle * particle) -> double {
        StoreObjPtr<RestOfEvent> roeobject("RestOfEvent");
        const RestOfEvent* roe;
        if (roeobject.isValid())   // if in for_each loop over ROE
        {
          roe = &(*roeobject);
        } else
        {
          roe = getRelatedROEObject(particle);
          if (!roe)
            return 0;
        }

        std::vector<ROOT::Math::XYZVector> p3_cms_roe;
        // static const double P_MAX(3.2);

        // Charged tracks
        const auto& roeTracks = roe->getChargedParticles(maskName);
        for (auto& roeChargedParticle : roeTracks)
        {
          // TODO: Add helix and KVF with IpProfile once available. Port from L163-199 of:
          // /belle/b20090127_0910/src/anal/ekpcontsuppress/src/ksfwmoments.cc
          ROOT::Math::PxPyPzEVector p_cms = PCmsLabTransform::labToCms(roeChargedParticle->get4Vector());
          if (p_cms != p_cms) continue;
          // if (p_cms.P() > P_MAX) continue; // Should not be added without any description.
          p3_cms_roe.push_back(p_cms.Vect());
        }

        // ECLCluster->Gamma
        const auto& roePhotons = roe->getPhotons(maskName);
        for (auto& roePhoton : roePhotons)
        {
          if (roePhoton->getECLClusterEHypothesisBit() == ECLCluster::EHypothesisBit::c_nPhotons) {
            ROOT::Math::PxPyPzEVector p_lab = roePhoton->get4Vector();
            if (p_lab != p_lab) continue;
            // if (p_lab.P() < 0.05) continue; // Should not be added without any description.
            ROOT::Math::PxPyPzEVector p_cms = PCmsLabTransform::labToCms(p_lab);
            if (p_cms != p_cms) continue;
            // if (p_cms.P() > P_MAX) continue; // Should not be added without any description.
            p3_cms_roe.push_back(p_cms.Vect());
          }
        }

        // KLMCluster
        const auto& roeKlongs = roe->getHadrons(maskName);
        for (auto& roeKlong : roeKlongs)
        {
          if (nKLMClusterTrackMatches(roeKlong) == 0 && !(roeKlong->getKLMCluster()->getAssociatedEclClusterFlag())) {
            ROOT::Math::PxPyPzEVector p_lab = roeKlong->get4Vector();
            if (p_lab != p_lab) continue;
            // if (p_lab.P() < 0.05) continue; // Should not be added without any description.
            ROOT::Math::PxPyPzEVector p_cms = PCmsLabTransform::labToCms(p_lab);
            if (p_cms != p_cms) continue;
            // if (p_cms.P() > P_MAX) continue; // Should not be added without any description.
            p3_cms_roe.push_back(p_cms.Vect());
          }
        }

        const B2Vector3D thrustO  = Thrust::calculateThrust(p3_cms_roe);
        const B2Vector3D pAxis = PCmsLabTransform::labToCms(particle->get4Vector()).Vect();

        double result = 0 ;
        if (pAxis == pAxis)
          result = abs(cos(pAxis.Angle(thrustO))); // abs??

        return result;

      };
      return func;
    }

    int lambdaFlavor(const Particle* particle)
    {
      if (particle->getPDGCode() == Const::Lambda.getPDGCode()) return 1; //Lambda0
      else if (particle->getPDGCode() == Const::antiLambda.getPDGCode()) return -1; //Anti-Lambda0
      else return 0;
    }

    bool isLambda(const Particle* particle)
    {
      const MCParticle* mcparticle = particle->getMCParticle();
      if (!mcparticle) return false;
      return (abs(mcparticle->getPDG()) == Const::Lambda.getPDGCode());
    }

    double lambdaZError(const Particle* particle)
    {
      //This is a simplistic hack. But I see no other way to get that information.
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
      ROOT::Math::PxPyPzEVector vec = PCmsLabTransform::labToCms(part->getDaughter(1)->get4Vector());
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
        sum += track->getMomentum().Perp2();
      }

      return sum;

    }

    Manager::FunctionPtr transverseMomentumOfChargeTracksInRoeWithMask(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      if (arguments.size() == 0)
        maskName = RestOfEvent::c_defaultMaskName;
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("At most 1 argument (name of mask) accepted.");

      auto func = [maskName](const Particle * particle) -> double {
        StoreObjPtr<RestOfEvent> roe("RestOfEvent");
        if (!roe.isValid()) return 0;

        double sum = 0.0;

        for (const auto& track : roe->getChargedParticles(maskName))
        {
          if (particle->isCopyOf(track, true)) continue;
          sum += track->getMomentum().Rho();
        }

        return sum;
      };
      return func;
    }

    Manager::FunctionPtr transverseMomentumSquaredOfChargeTracksInRoeWithMask(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      if (arguments.size() == 0)
        maskName = RestOfEvent::c_defaultMaskName;
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("At most 1 argument (name of mask) accepted.");

      auto func = [maskName](const Particle * particle) -> double {
        StoreObjPtr<RestOfEvent> roe("RestOfEvent");
        if (!roe.isValid()) return 0;

        double sum = 0.0;

        for (const auto& track : roe->getChargedParticles(maskName))
        {
          if (particle->isCopyOf(track, true)) continue;
          sum += track->getMomentum().Perp2();
        }

        return sum;
      };
      return func;
    }

    int NumberOfKShortsInRoe(const Particle* particle)
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

    bool isInElectronOrMuonCat(const Particle* particle)
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
            trackTargetMuon = pMuon->getTrack();
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
            trackTargetElectron = pElectron->getTrack();
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

    bool isMajorityInRestOfEventFromB0(const Particle*)
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

    Manager::FunctionPtr isMajorityInRestOfEventFromB0WithMask(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      if (arguments.size() == 0)
        maskName = RestOfEvent::c_defaultMaskName;
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("At most 1 argument (name of mask) accepted.");

      auto func = [maskName](const Particle*) -> bool {
        StoreObjPtr<RestOfEvent> roe("RestOfEvent");
        if (!roe.isValid()) return 0;

        int vote = 0;
        for (auto& track : roe->getChargedParticles(maskName))
        {
          const MCParticle* mcParticle = track->getMCParticle();
          vote += getB0flavourMC(mcParticle);
        }

        return vote > 0;

      };
      return func;
    }

    bool isMajorityInRestOfEventFromB0bar(const Particle*)
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

    Manager::FunctionPtr isMajorityInRestOfEventFromB0barWithMask(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      if (arguments.size() == 0)
        maskName = RestOfEvent::c_defaultMaskName;
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("At most 1 argument (name of mask) accepted.");

      auto func = [maskName](const Particle*) -> bool {
        StoreObjPtr<RestOfEvent> roe("RestOfEvent");
        if (!roe.isValid()) return 0;

        int vote = 0;
        for (auto& track : roe->getChargedParticles(maskName))
        {
          const MCParticle* mcParticle = track->getMCParticle();
          vote += getB0flavourMC(mcParticle);
        }

        return vote < 0;

      };
      return func;
    }

    bool hasRestOfEventTracks(const Particle* part)
    {
      const RestOfEvent* roe = part->getRelatedTo<RestOfEvent>();
      return (roe && roe-> getNTracks() > 0);
    }

    Manager::FunctionPtr hasRestOfEventTracksWithMask(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      if (arguments.size() == 0)
        maskName = RestOfEvent::c_defaultMaskName;
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("At most 1 argument (name of mask) accepted.");

      auto func = [maskName](const Particle*) -> bool {
        StoreObjPtr<RestOfEvent> roe("RestOfEvent");
        if (!roe.isValid()) return 0;

        return roe->getNTracks(maskName) > 0;
      };
      return func;
    }

    int isRelatedRestOfEventB0Flavor(const Particle* particle)
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

    Manager::FunctionPtr isRelatedRestOfEventB0FlavorWithMask(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      if (arguments.size() == 0)
        maskName = RestOfEvent::c_defaultMaskName;
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("At most 1 argument (name of mask) accepted.");

      auto func = [maskName](const Particle * particle) -> int {
        StoreObjPtr<RestOfEvent> roe("RestOfEvent");
        if (!roe.isValid()) return 0;

        const MCParticle* BcpMC = particle->getMCParticle();
        if (!BcpMC) return 0;
        if (Variable::isSignal(particle) <= 0) return 0;

        const MCParticle* Y4S = BcpMC->getMother();
        if (!Y4S) return 0;

        int BtagFlavor = 0;
        int BcpFlavor = 0;

        for (auto& roeChargedParticle : roe->getChargedParticles(maskName))
        {
          const MCParticle* mcParticle = roeChargedParticle->getMCParticle();
          while (mcParticle) {
            if (mcParticle->getMother() != Y4S) {
              mcParticle = mcParticle->getMother();
              continue;
            }

            if (mcParticle == BcpMC) { // if mcParticle is associated with CP-side unfortunately
              if (mcParticle->getPDG() > 0)
                BcpFlavor = 2;
              else
                BcpFlavor = -2;
            } else if (BtagFlavor == 0) { // only first mcParticle is checked.
              if (abs(mcParticle->getPDG()) == 511 || abs(mcParticle->getPDG()) == 521) {
                if (mcParticle->getPDG() > 0)
                  BtagFlavor = 1;
                else
                  BtagFlavor = -1;
              } else {
                BtagFlavor = 5;
              }
            }
            break;
          }
          if (BcpFlavor != 0 || BtagFlavor == 5) break;
        }

        return (BcpFlavor != 0) ? BcpFlavor : BtagFlavor;
      };
      return func;

    }

    int isRestOfEventB0Flavor(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (!roe.isValid()) return 0;

      const Particle* Bcp = roe->getRelatedFrom<Particle>();
      return Variable::isRelatedRestOfEventB0Flavor(Bcp);
    }

    int ancestorHasWhichFlavor(const Particle* particle)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (!roe.isValid()) return 0;

      const MCParticle* BcpMC = roe->getRelatedFrom<Particle>()->getMCParticle();
      const MCParticle* Y4S = BcpMC->getMother();
      const MCParticle* mcParticle = particle->getMCParticle();

      int outputB0tagQ = 0;
      while (mcParticle) {
        if (mcParticle->getMother() == Y4S) {
          if (mcParticle != BcpMC && abs(mcParticle->getPDG()) == 511) {
            if (mcParticle->getPDG() == 511) outputB0tagQ = 1;
            else outputB0tagQ = -1;
          } else if (mcParticle == BcpMC) {
            if (mcParticle->getPDG() == 511) outputB0tagQ = 2;
            else outputB0tagQ = -2;
          } else outputB0tagQ = 5;
          break;
        }
        mcParticle = mcParticle->getMother();
      }

      return outputB0tagQ;
    }

    int B0mcErrors(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (!roe.isValid()) return -1;

      const Particle* Bcp = roe->getRelatedFrom<Particle>();
      const MCParticle* BcpMC = roe->getRelatedFrom<Particle>()->getMCParticle();
      return MCMatching::getMCErrors(Bcp, BcpMC);
    }

    int isRelatedRestOfEventMajorityB0Flavor(const Particle* part)
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

    Manager::FunctionPtr isRelatedRestOfEventMajorityB0FlavorWithMask(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      if (arguments.size() == 0)
        maskName = RestOfEvent::c_defaultMaskName;
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("At most 1 argument (name of mask) accepted.");

      auto func = [maskName](const Particle*) -> int {
        StoreObjPtr<RestOfEvent> roe("RestOfEvent");
        if (!roe.isValid()) return -2;

        int q_MC = 0; //Flavor of B

        if (roe->getNTracks(maskName) > 0)
        {
          for (auto& track : roe->getChargedParticles(maskName)) {
            const MCParticle* mcParticle = track->getMCParticle();
            q_MC += getB0flavourMC(mcParticle);
          }
        } else if (roe->getNECLClusters(maskName) > 0)   // only if there are no tracks
        {
          for (auto& cluster : roe->getPhotons(maskName)) {
            if (cluster->getECLClusterEHypothesisBit() != ECLCluster::EHypothesisBit::c_nPhotons) continue;
            const MCParticle* mcParticle = cluster->getMCParticle();
            q_MC += getB0flavourMC(mcParticle);
          }
        } else if (roe->getNKLMClusters(maskName) > 0)   // only if there are no tracks nor ecl-clusters
        {
          for (auto& klmcluster : roe->getHadrons(maskName)) {
            const MCParticle* mcParticle = klmcluster->getMCParticle();
            q_MC += getB0flavourMC(mcParticle);
          }
        }

        if (q_MC == 0)
          return -2;
        else
          return int(q_MC > 0);

      };
      return func;
    }

    int isRestOfEventMajorityB0Flavor(const Particle*)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (!roe.isValid()) return -2;//gRandom->Uniform(0, 1);

      const Particle* Bcp = roe->getRelatedFrom<Particle>();
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

      std::string requestedVariable;
      std::string maskName;
      if (arguments.size() == 1) {
        requestedVariable = arguments[0];
        maskName = RestOfEvent::c_defaultMaskName;
      } else if (arguments.size() == 2) {
        requestedVariable = arguments[0];
        maskName = arguments[1];
      } else {
        B2FATAL("Number of arguments must be 1 (requestedVariable) or 2 (requestedVariable, maskName).");
      }

      const std::vector<string> availableVariables = {"recoilMass",
                                                      "recoilMassSqrd",
                                                      "pMissCMS",
                                                      "cosThetaMissCMS",
                                                      "EW90"
                                                     };

      if (std::find(availableVariables.begin(), availableVariables.end(), requestedVariable) == availableVariables.end()) {
        B2FATAL("Wrong variable " << requestedVariable <<
                " requested. The possibilities are recoilMass, recoilMassSqrd, pMissCMS, cosThetaMissCMS or EW90");
      }

      auto func = [requestedVariable, maskName](const Particle * particle) -> double {
        StoreObjPtr<RestOfEvent> roe("RestOfEvent");
        if (!roe.isValid())
          return 0;

        ROOT::Math::PxPyPzEVector momXChargedTracks; //Momentum of charged X tracks in lab-System
        const auto& roeChargedParticles = roe->getChargedParticles(maskName);
        for (auto& roeChargedParticle : roeChargedParticles)
        {
          if (roeChargedParticle->isCopyOf(particle, true)) continue;
          momXChargedTracks += roeChargedParticle->get4Vector();
        }

        ROOT::Math::PxPyPzEVector momXNeutralClusters = roe->get4VectorNeutralECLClusters(maskName); //Momentum of neutral X clusters in lab-System
        const auto& klongs = roe->getHadrons(maskName);
        for (auto& klong : klongs)
        {
          if (nKLMClusterTrackMatches(klong) == 0 && !(klong->getKLMCluster()->getAssociatedEclClusterFlag())) {
            momXNeutralClusters += klong->get4Vector();
          }
        }

        ROOT::Math::PxPyPzEVector momX = PCmsLabTransform::labToCms(momXChargedTracks + momXNeutralClusters); //Total Momentum of the recoiling X in CMS-System
        ROOT::Math::PxPyPzEVector momTarget = PCmsLabTransform::labToCms(particle->get4Vector());  //Momentum of Mu in CMS-System
        ROOT::Math::PxPyPzEVector momMiss = -(momX + momTarget); //Momentum of Anti-v  in CMS-System

        double output = 0.0;
        if (requestedVariable == "recoilMass") output = momX.M();
        if (requestedVariable == "recoilMassSqrd") output = momX.M2();
        if (requestedVariable == "pMissCMS") output = momMiss.P();
        if (requestedVariable == "cosThetaMissCMS") output = momTarget.Vect().Unit().Dot(momMiss.Vect().Unit());
        if (requestedVariable == "EW90")
        {

          ROOT::Math::PxPyPzEVector momW = momTarget + momMiss; //Momentum of the W-Boson in CMS
          float E_W_90 = 0 ; // Energy of all charged and neutral clusters in the hemisphere of the W-Boson

          const auto& photons = roe->getPhotons(maskName);
          for (auto& photon : photons) {
            if (PCmsLabTransform::labToCms(photon->get4Vector()).Vect().Dot(momW.Vect()) > 0) {
              E_W_90 += photon->getECLClusterEnergy();
            }
          }
          for (auto& roeChargedParticle : roeChargedParticles) {
            if (roeChargedParticle->isCopyOf(particle, true))
              continue;

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

          output = E_W_90;
        }

        return output;
      };
      return func;
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


        ROOT::Math::PxPyPzEVector momTargetSlowPion;
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
              momTargetSlowPion = PCmsLabTransform::labToCms(pSlowPion->get4Vector());
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
          ROOT::Math::PxPyPzEVector momTargetKaon = PCmsLabTransform::labToCms(particle->get4Vector());
          if (momTargetKaon == momTargetKaon && momTargetSlowPion == momTargetSlowPion)
            output = momTargetKaon.Vect().Unit().Dot(momTargetSlowPion.Vect().Unit());
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
        ROOT::Math::PxPyPzEVector momFastParticle;  //Momentum of Fast Pion in CMS-System
        Particle* TargetFastParticle = nullptr;
        for (unsigned int i = 0; i < FastParticleList->getListSize(); ++i)
        {
          Particle* particlei = FastParticleList->getParticle(i);
          if (!particlei) continue;

          ROOT::Math::PxPyPzEVector momParticlei = PCmsLabTransform::labToCms(particlei->get4Vector());
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
          output = cosTPTO(TargetFastParticle);

        ROOT::Math::PxPyPzEVector momSlowPion = PCmsLabTransform::labToCms(particle->get4Vector());  //Momentum of Slow Pion in CMS-System
        if (momSlowPion == momSlowPion)   // FIXME
        {
          if (requestedVariable == "cosSlowFast") {
            output = momSlowPion.Vect().Unit().Dot(momFastParticle.Vect().Unit());
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


//  Target Variables ----------------------------------------------------------------------------------------------

    // Lists used in target variables
    static const std::vector<int> charmMesons = {
      411 /*D+*/, 413 /*D*+*/, 415/*D_2*+*/, 421 /*D0*/, 423 /*D*0*/, 425/*D_2*0*/, 431/*D_s+*/, 433/*D_s*+*/, 435/*D_s2*+*/,
      10411 /*D_0*+*/, 10413 /*D_1+*/, 10421 /*D_0*0*/, 10423 /*D_10*/, 10431 /*D_s0*+*/, 10433/*D'_s1+*/,
      20413 /*D'_1+*/, 20423 /*D'_10*/, 20433/*D_s1+*/,
    };

    static const std::vector<int> charmBaryons = {
      4112 /*Sigma_c0*/, 4114 /*Sigma_c*0*/, 4122 /*Lambda_c+*/, 4132 /*Xi_c0*/,
      4212 /*Sigma_c+*/, 4214 /*Sigma_c*+*/, 4222 /*Sigma_c++*/, 4224 /*sigma_c*++*/, 4232 /*Xi_c+*/,
      4312 /*Xi'_c0*/, 4314 /*Xi_c*0*/, 4322 /*Xi'_c+*/, 4324 /*Xi_c*+*/, 4332 /*Omega_c0*/, 4334 /*Omega_c*0*/,
      4412 /*Xi_cc+*/, 4414 /*Xi_cc*+*/, 4422 /*Xi_cc++*/, 4424 /*Xi_cc*0*/, 4432 /*Omega_cc+*/, 4434 /*Omega_cc*+*/,

      4444 /*Omega_ccc++ not in evt.pdl*/
    };

    static const std::vector<int> qqbarMesons = {
      // light qqbar
      111 /*pi0*/, 113 /*rho_0*/, 115 /*a_20*/, 117 /*rho(3)(1690)0*/, 119 /*a_4(1970)0*/,
      10111 /*pi(2S)0*/, 10113 /*b_10*/, 10115 /*pi(2)(1670)0*/,
      20113 /*a_10*/, 30113 /*rho(3S)0*/,
      100111 /*pi0_1300 not in evt.pdl*/, 100113 /*rho(2S)0*/,
      9000111 /*a_00*/, 9000113 /*pi(1)(1400)0*/, 9010113 /*pi(1)(1600)0*/, 9040113 /*rho(2150)0*/,

      // not in evt.pdl
      200111 /*?*/, 9020113 /*a1_1640*/, 9030113 /*rho(1900)0*/, 100115 /*?*/, 9000115 /*?*/,
      9000117 /*rho(3)(1990)0*/, 9010117 /*rho(3)(2250)0*/,

      // ssbar Mesons
      221 /*eta*/, 223 /*omega*/, 225 /*f_2*/, 227 /*omega(3)(1670)*/, 229 /*f(4)(2050)*/,
      331 /*eta'*/, 333 /*phi*/, 335 /*f'_2*/, 337 /*phi(3)(1850)*/,
      10223 /*h_1*/, 10225 /*eta(2)(1645)*/, 10331 /*f(0)(1710)*/, 10333 /*h'_1*/,
      20223 /*f_1*/, 20333 /*f'_1*/, 30223 /*omega(1650)*/,
      100223 /*omega(2S)*/, 100333 /*phi(1680)*/,
      9000221 /*sigma_0*/, 9010221 /*f_0*/, 9020221 /*eta(1405)*/, 9030221 /*f_0(1500)*/,
      9050225 /*f(2)(1950)*/, 9060225 /*f(2)(2010)*/,

      // not in evt.pdl
      10221 /*f0(980)*/,  10335 /*eta(2)(1870)*/,
      100331 /*eta(1470)*/,
      9000223 /*f_1(1510)*/, 9000225 /*f_2(1430)*/, 9000229 /*f_J(2220)*/,
      9010223 /*h_1(1595)*/, 9010225 /*f_2(1565)*/, 9020225 /*f_2(1640)*/, 9030225 /*f_2(1810)*/,
      9040221 /*eta(1760)*/, 9040225 /*f_2(1910)*/, 9050221 /*f_0(2020)*/,
      9060221 /*f_0(2100)*/, 9070221 /*f_0(2200)*/, 9070225 /*f_2(2150)*/,

      100225 /*?*/, 100335 /*?*/,
      200221 /*?*/,
      9000339 /*?*/,

      // ccbar Mesons
      441 /*eta_c*/, 443 /*J/psi*/, 445 /*chi_c2*/,
      10441 /*chi_c0*/, 10443 /*h_c*/, 20443 /*chi_c1*/, 30443 /*psi(3770)*/,
      100441 /*eta_c(2S)*/, 100443 /*phi(2S)*/,
      9000443 /*phi(4040)*/, 9010443 /*psi(4160)*/, 9020443 /*psi(4415)*/,
      9000445 /*? not in evt.pdl*/,
    };

    static const std::vector<int> flavorConservingMesons = {
      // Excited light mesons that can decay into hadrons conserving flavor
      213 /*rho+*/, 215 /*a_2+*/, 217 /*rho(3)(1690)+*/, 219 /*a_4(1970)+*/,
      10211 /*a(0)(1450)+*/, 10213 /*b_1+*/, 10215 /*pi(2)(1670)+*/,
      20213 /*a_1+*/, 30213 /*rho(3S)+*/,
      100211 /*pi(2S)+*/, 100213 /*rho(2S)+*/,
      9000211 /*a_0+*/, 9000213 /*pi(1)(1400)+*/, 9010213 /*pi(1)(1600)+*/,

      // not in evt.pdl
      9000215 /*a_2(1700)+*/, 9000217 /*rho_3(1990)+*/,
      9010217 /*rho_3(2250)+*/, 9020213 /*a_1(1640)+*/, 9030213 /*rho(1900)+*/, 9040213 /*rho(2150)+*/,

      100215 /*?*/, 200211 /*?*/,

      // Excited K Mesons that hadronize conserving flavor
      313 /*K*0*/, 315 /*K_2*0*/, 317 /*K_3*0*/, 319 /*K_4*0*/,
      323 /*K*+*/, 325 /*K_2*+*/, 327 /*K_3*+*/, 329 /*K_4*+*/,
      10311 /*K_0*0*/, 10313 /*K_10*/, 10315 /*K(2)(1770)0*/,
      10321 /*K_0*+*/, 10323 /*K_1+*/, 10325 /*K(2)(1770)+*/,
      20313 /*K'_10*/, 20315 /*K(2)(1820)0*/,
      20323 /*K'_1+*/, 20325 /*K(2)(1820)+*/,
      30313 /*K''*0*/, 30323 /*K''*+*/, 30343 /*Xsd*/,
      100313 /*K'*0*/, 100323 /*K'*+*/,

      // not in evt.pdl
      100311 /*K(1460)0*/, 100321 /*K(1460)+*/,
      9000311 /*K*(700)0*/, 9000313 /*K_1(1650)0*/, 9000315 /*K_2(1580)0*/,
      9000321 /*K*(700)+*/, 9000323 /*K_1(1650)+*/, 9000325 /*K_2(1580)+*/,
      9010315 /*K_2*(1980)0*/, 9010317 /*K_3(2320)0*/,
      9010325 /*K_2*(1980)+*/, 9010327 /*K_3(2320)+*/,

      100315 /*?*/, 100325 /*?*/,
      200311 /*?*/, 200321 /*?*/,
      9000319 /*?*/, 9000329 /*?*/,
    };

    const std::vector<std::string> availableForIsRightTrack = { "Electron",             // 0
                                                                "IntermediateElectron", // 1
                                                                "Muon",                 // 2
                                                                "IntermediateMuon",     // 3
                                                                "KinLepton",            // 4
                                                                "IntermediateKinLepton",// 5
                                                                "Kaon",                 // 6
                                                                "SlowPion",             // 7
                                                                "FastHadron",           // 8
                                                                "Lambda",               // 9
                                                                "mcAssociated"          // 10
                                                              };

    const std::vector<std::string> availableForIsRightCategory = { "Electron",             // 0
                                                                   "IntermediateElectron", // 1
                                                                   "Muon",                 // 2
                                                                   "IntermediateMuon",     // 3
                                                                   "KinLepton",            // 4
                                                                   "IntermediateKinLepton",// 5
                                                                   "Kaon",                 // 6
                                                                   "SlowPion",             // 7
                                                                   "FastHadron",           // 8
                                                                   "KaonPion",             // 9
                                                                   "MaximumPstar",         // 10
                                                                   "FSC",                  // 11
                                                                   "Lambda",               // 12
                                                                   "mcAssociated"          // 13
                                                                 };



    Manager::FunctionPtr isRightTrack(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1) {
        B2FATAL("Wrong number of arguments (1 required) for meta function isRightTrack");
      }

      auto particleName = arguments[0];

      unsigned index = std::find(availableForIsRightTrack.begin(), availableForIsRightTrack.end(), particleName)
                       - availableForIsRightTrack.begin();
      if (index == availableForIsRightTrack.size()) {
        B2FATAL("isRightTrack: Not available category " << particleName <<
                ". The possibilities are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron and Lambda");
      }

      auto func = [index](const Particle * particle) -> int {

        const MCParticle* mcParticle = particle->getMCParticle();
        if (!mcParticle) return -2;

        int mcPDG = abs(mcParticle->getPDG());

        // ---------------------------- Mothers and Grandmothers ----------------------------------
        std::vector<int> mothersPDG;
        std::vector<const MCParticle*> mothersPointers;

        const MCParticle* mcMother = mcParticle->getMother();
        while (mcMother)
        {
          mothersPDG.push_back(abs(mcMother->getPDG()));
          mothersPointers.push_back(mcMother);
          if (abs(mcMother->getPDG()) == 511) break;
          mcMother = mcMother->getMother();
        }

        if (mothersPDG.size() == 0) return -2;

        //has associated mothers up to a B meson
        if (index == 10) return 1;

        // ----------------  Is D Meson in the decay chain  --------------------------------------

        bool isCharmedMesonInChain = false;
        for (auto& iMCMotherPDG : mothersPDG)
        {
          if (std::find(charmMesons.begin(), charmMesons.end(), iMCMotherPDG) != charmMesons.end()) {
            isCharmedMesonInChain = true;
            break;
          }
        }

        // ----------------  Is Charmed Baryon in the decay chain  --------------------------------

        bool isCharmedBaryonInChain = false;
        for (auto& iMCMotherPDG : mothersPDG)
        {
          if (std::find(charmBaryons.begin(), charmBaryons.end(), iMCMotherPDG) != charmBaryons.end()) {
            isCharmedBaryonInChain = true;
            break;
          }
        }

        // ----------------  Is neutral qqbar Meson in the decay chain  --------------------------------

        bool isQQbarMesonInChain = false;
        for (auto& iMCMotherPDG : mothersPDG)
        {
          if (std::find(qqbarMesons.begin(), qqbarMesons.end(), iMCMotherPDG) != qqbarMesons.end()) {
            isQQbarMesonInChain = true;
            break;
          }
        }

        // --------------  Is the Hadron a descendant of a Meson that conserves flavor  --------------------------

        bool isB0DaughterConservingFlavor = false;
        if (std::find(flavorConservingMesons.begin(), flavorConservingMesons.end(),
                      mothersPDG.rbegin()[1]) != flavorConservingMesons.end())
        {
          isB0DaughterConservingFlavor = true;
        }

        // -----------------------------  Is the Hadron a single daughter of a tau ----- --------------------------

        bool isHadronSingleTauDaughter = false;
        if (mothersPDG.size() > 1 && mothersPDG.rbegin()[1] == 15)
        {
          int numberOfChargedDaughters = 0;
          for (auto& tauDaughter : mothersPointers.rbegin()[1]->getDaughters()) {
            if (tauDaughter->getCharge() != 0)
              numberOfChargedDaughters += 1;
          }
          if (numberOfChargedDaughters == 1)
            isHadronSingleTauDaughter = true;
        }

        if (index == 0 // Electron
            && mcPDG == Const::electron.getPDGCode() && mothersPDG[0] == 511)
        {
          return 1;
        } else if (index == 1 // IntermediateElectron
                   && mcPDG == Const::electron.getPDGCode() && mothersPDG.size() > 1 && isQQbarMesonInChain == false)
        {
          return 1;
        } else if (index == 2 // Muon
                   && mcPDG == Const::muon.getPDGCode() && mothersPDG[0] == 511)
        {
          return 1;
        } else if (index == 3 // IntermediateMuon
                   && mcPDG == Const::muon.getPDGCode() && mothersPDG.size() > 1 && isQQbarMesonInChain == false)
        {
          return 1;
        } else if (index == 4 // KinLepton
                   && (mcPDG == Const::muon.getPDGCode() || mcPDG == Const::electron.getPDGCode()) && mothersPDG[0] == 511)
        {
          return 1;
        } else if (index == 5 //IntermediateKinLepton
                   && (mcPDG == Const::muon.getPDGCode() || mcPDG == Const::electron.getPDGCode()) && mothersPDG.size() > 1
                   && isQQbarMesonInChain == false)
        {
          return 1;
        } else if (index == 6 // Kaon
                   && mcPDG == Const::kaon.getPDGCode() && isQQbarMesonInChain == false
                   && (isCharmedMesonInChain == true || isCharmedBaryonInChain == true))
        {
          return 1;
        } else if (index == 7 // SlowPion
                   && mcPDG == Const::pion.getPDGCode() && mothersPDG.size() > 1 && mothersPDG[0] == 413 && mothersPDG[1] == 511)
        {
          return 1;
        } else if (index == 8 // FastHadron
                   && (mcPDG == Const::pion.getPDGCode() || mcPDG == Const::kaon.getPDGCode())
                   && isQQbarMesonInChain == false
                   && (mothersPDG[0] == 511
                       || (mothersPDG.rbegin()[0] == 511 && (isB0DaughterConservingFlavor == true || isHadronSingleTauDaughter == true))))
        {
          return 1;
        } else if (index == 9 // Lambda
                   && mcPDG == Const::Lambda.getPDGCode() && isCharmedBaryonInChain == true)
        {
          return 1;
        } else
        {
          return 0;
        }
      };
      return func;
    }

    Manager::FunctionPtr isRightCategory(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1) {
        B2FATAL("Wrong number of arguments (1 required) for meta function isRightCategory");
      }

      auto particleName = arguments[0];

      unsigned index = find(availableForIsRightCategory.begin(), availableForIsRightCategory.end(), particleName)
                       - availableForIsRightCategory.begin();
      if (index == availableForIsRightCategory.size()) {
        B2FATAL("isRightCategory: Not available category " << particleName <<
                ". The possibilities are Electron, IntermediateElectron, Muon, IntermediateMuon, KinLepton, IntermediateKinLepton, Kaon, SlowPion, FastHadron, KaonPion, MaximumPstar, FSC and Lambda");
      }

      auto func = [index](const Particle * particle) -> int {

        Particle* nullParticle = nullptr;
        double qTarget = particle->getCharge();
        double qMC = Variable::isRestOfEventB0Flavor(nullParticle);

        const MCParticle* mcParticle = particle->getMCParticle();
        if (!mcParticle) return -2;

        int mcPDG = abs(mcParticle->getPDG());

        // ---------------------------- Mothers and Grandmothers ---------------------------------
        std::vector<int> mothersPDG;
        std::vector<const MCParticle*> mothersPointers;

        const MCParticle* mcMother = mcParticle->getMother();
        while (mcMother)
        {
          mothersPDG.push_back(abs(mcMother->getPDG()));
          mothersPointers.push_back(mcMother);
          if (abs(mcMother->getPDG()) == 511) break;
          mcMother = mcMother->getMother();
        }

        if (mothersPDG.size() == 0) return -2;
        //has associated mothers up to a B meson
        if (index == 13) return 1;

        // ----------------  Is D Meson in the decay chain  --------------------------------------

        bool isCharmedMesonInChain = false;
        for (auto& iMCMotherPDG : mothersPDG)
        {
          if (std::find(charmMesons.begin(), charmMesons.end(), iMCMotherPDG) != charmMesons.end()) {
            isCharmedMesonInChain = true;
            break;
          }
        }

        // ----------------  Is Charmed Baryon in the decay chain  --------------------------------

        bool isCharmedBaryonInChain = false;
        for (auto& iMCMotherPDG : mothersPDG)
        {
          if (std::find(charmBaryons.begin(), charmBaryons.end(), iMCMotherPDG) != charmBaryons.end()) {
            isCharmedBaryonInChain = true;
            break;
          }
        }

        // ----------------  Is neutral qqbar Meson in the decay chain  --------------------------------

        bool isQQbarMesonInChain = false;
        for (auto& iMCMotherPDG : mothersPDG)
        {
          if (std::find(qqbarMesons.begin(), qqbarMesons.end(), iMCMotherPDG) != qqbarMesons.end()) {
            isQQbarMesonInChain = true;
            break;
          }
        }

        // --------------  Is the Hadron a descendant of a Meson that conserves flavor  --------------------------

        bool isB0DaughterConservingFlavor = false;
        if (mothersPDG.size() > 1)
        {
          if (std::find(flavorConservingMesons.begin(), flavorConservingMesons.end(),
                        mothersPDG.rbegin()[1]) != flavorConservingMesons.end()) {
            isB0DaughterConservingFlavor = true;
          }
        }

        // -----------------------------  Is the Hadron a single daughter of a tau ----- --------------------------

        bool isHadronSingleTauDaughter = false;
        if (mothersPDG.size() > 1 && mothersPDG.rbegin()[1] == 15)
        {
          int numberOfChargedDaughters = 0;
          for (auto& tauDaughter : mothersPointers.rbegin()[1]->getDaughters()) {
            if (tauDaughter->getCharge() != 0)
              numberOfChargedDaughters += 1;
          }
          if (numberOfChargedDaughters == 1)
            isHadronSingleTauDaughter = true;
        }

        // ----------------------------  For KaonPion Category ------------------------------------
        bool haveKaonPionSameMother = false;
        if (index == 9)   // KaonPion
        {
          const MCParticle* mcSlowPionMother = nullptr;
          StoreObjPtr<ParticleList> SlowPionList("pi+:inRoe");
          Particle* targetSlowPion = nullptr;
          if (SlowPionList.isValid()) {
            double mcProbSlowPion = 0;
            for (unsigned int i = 0; i < SlowPionList->getListSize(); ++i) {
              Particle* pSlowPion = SlowPionList->getParticle(i);
              if (!pSlowPion) continue;
              if (pSlowPion->hasExtraInfo("isRightCategory(SlowPion)")) {
                double probSlowPion = pSlowPion->getExtraInfo("isRightCategory(SlowPion)");
                if (probSlowPion > mcProbSlowPion) {
                  mcProbSlowPion = probSlowPion;
                  targetSlowPion = pSlowPion;
                }
              }
            }
            if (targetSlowPion != nullptr) {
              const MCParticle* mcSlowPion = targetSlowPion ->getMCParticle();
              //               SlowPion_q = targetSlowPion->getCharge();
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
        if (index == 11)   // FSC
        {
          StoreObjPtr<ParticleList> FastParticleList("pi+:inRoe");
          Particle* targetFastParticle = nullptr;
          if (FastParticleList.isValid()) {
            double mcProbFastest = 0;
            for (unsigned int i = 0; i < FastParticleList->getListSize(); ++i) {
              Particle* particlei = FastParticleList->getParticle(i);
              if (!particlei) continue;

              ROOT::Math::PxPyPzEVector momParticlei = PCmsLabTransform::labToCms(particlei->get4Vector());
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
              //               FastParticle_q = targetFastParticle->getCharge();
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
          return 1;
        } else if (index == 1 // IntermediateElectron
                   && qTarget != qMC && mcPDG == Const::electron.getPDGCode() && mothersPDG.size() > 1
                   && isQQbarMesonInChain == false)
        {
          return 1;
        } else if (index == 2 // Muon
                   && qTarget == qMC && mcPDG == Const::muon.getPDGCode() && mothersPDG[0] == 511)
        {
          return 1;
        } else if (index == 3 // IntermediateMuon
                   && qTarget != qMC && mcPDG == Const::muon.getPDGCode() && mothersPDG.size() > 1
                   && isQQbarMesonInChain == false)
        {
          return 1;
        }  else if (index == 4 // KinLepton
                    && qTarget == qMC
                    && (mcPDG == Const::electron.getPDGCode() || mcPDG == Const::muon.getPDGCode()) && mothersPDG[0] == 511)
        {
          return 1;
        }  else if (index == 5 // IntermediateKinLepton
                    && qTarget != qMC && (mcPDG == Const::electron.getPDGCode() || mcPDG == Const::muon.getPDGCode())
                    && mothersPDG.size() > 1 && isQQbarMesonInChain == false)
        {
          return 1;
        } else if (index == 6// Kaon
                   && qTarget == qMC && mcPDG == Const::kaon.getPDGCode() && isQQbarMesonInChain == false
                   && (isCharmedMesonInChain == true || isCharmedBaryonInChain == true))
        {
          return 1;
        } else if (index == 7 // SlowPion
                   && qTarget != qMC && mcPDG == Const::pion.getPDGCode()
                   && mothersPDG.size() > 1 && mothersPDG[0] == 413 && mothersPDG[1] == 511)
        {
          return 1;
        } else if (index == 8 // FastHadron
                   && qTarget == qMC && (mcPDG == Const::pion.getPDGCode() || mcPDG == Const::kaon.getPDGCode())
                   && isQQbarMesonInChain == false
                   && (mothersPDG[0] == 511 || (mothersPDG.rbegin()[0] == 511
                                                && (isB0DaughterConservingFlavor == true || isHadronSingleTauDaughter == true))))
        {
          return 1;
        } else if (index == 9 // KaonPion
                   && qTarget == qMC && mcPDG == Const::kaon.getPDGCode() && haveKaonPionSameMother == true)
        {
          return 1;
        } else if (index == 10 && qTarget == qMC)   // MaximumPstar
        {
          return 1;
        } else if (index == 11 // FSC
                   && qTarget != qMC && mothersPDG.size() > 1 && qFSC == qMC
                   && mcPDG == Const::pion.getPDGCode() && FastParticlePDGMother == 511 && isQQbarMesonInChain == false)
        {
          return 1;
        } else if (index == 12 // Lambda
                   && (particle->getPDGCode() / abs(particle->getPDGCode())) != qMC
                   && mcPDG == Const::Lambda.getPDGCode() && isCharmedBaryonInChain == true)
        {
          return 1;
        } else
        {
          return 0;
        }
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

      bool isAvailable = false;
      for (const auto& name : availableForIsRightTrack) {
        if (extraInfoName == "isRightTrack(" + name + ")") {
          isAvailable = true;
          break;
        }
      }
      for (const auto& name : availableForIsRightCategory) {
        if (extraInfoName == "isRightCategory(" + name + ")") {
          isAvailable = true;
          break;
        }
      }
      if (extraInfoName == "isRightTrack(MaximumPstar)")
        isAvailable = true;


      if (!isAvailable) {
        string strAvailableForIsRightTrack;
        for (const auto& name : availableForIsRightTrack)
          strAvailableForIsRightTrack += name + " ";
        string strAvailableForIsRightCategory;
        for (const auto& name : availableForIsRightCategory)
          strAvailableForIsRightCategory += name + " ";

        B2FATAL("hasHighestProbInCat: Not available category" << extraInfoName <<
                ". The possibilities for isRightTrack() are " << endl << strAvailableForIsRightTrack << " MaximumPstar" << endl <<
                "The possibilities for isRightCategory() are " << endl << strAvailableForIsRightCategory);
      }

      auto func = [particleListName, extraInfoName](const Particle * particle) -> bool {
        StoreObjPtr<ParticleList> ListOfParticles(particleListName);
        if (!ListOfParticles.isValid()) return 0;

        double maximumProb = 0;
        for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i)
        {
          const Particle* particlei = ListOfParticles->getParticle(i);
          if (!particlei) continue;

          double prob = 0;
          if (extraInfoName == "isRightTrack(MaximumPstar)") {
            ROOT::Math::PxPyPzEVector momParticlei = PCmsLabTransform::labToCms(particlei->get4Vector());
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

        bool output = false;
        if ((extraInfoName == "isRightTrack(MaximumPstar)") && (PCmsLabTransform::labToCms(particle->get4Vector()).P() == maximumProb))
        {
          output = true;
        } else if (extraInfoName != "isRightTrack(MaximumPstar)" && particle->hasExtraInfo(extraInfoName))
        {
          if (particle->getExtraInfo(extraInfoName) == maximumProb) output = true;
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

      bool isAvailable = false;
      for (const auto& name : availableForIsRightTrack) {
        if (extraInfoName == "isRightTrack(" + name + ")") {
          isAvailable = true;
          break;
        }
      }
      for (const auto& name : availableForIsRightCategory) {
        if (extraInfoName == "isRightCategory(" + name + ")") {
          isAvailable = true;
          break;
        }
      }
      if (extraInfoName == "isRightTrack(MaximumPstar)")
        isAvailable = true;


      if (!isAvailable) {
        string strAvailableForIsRightTrack;
        for (const auto& name : availableForIsRightTrack)
          strAvailableForIsRightTrack += name + " ";
        string strAvailableForIsRightCategory;
        for (const auto& name : availableForIsRightCategory)
          strAvailableForIsRightCategory += name + " ";

        B2FATAL("HighestProbInCat: Not available category" << extraInfoName <<
                ". The possibilities for isRightTrack() are " << endl << strAvailableForIsRightTrack << " MaximumPstar" << endl <<
                "The possibilities for isRightCategory() are " << endl << strAvailableForIsRightCategory);
      }

      auto func = [particleListName, extraInfoName](const Particle*) -> double {
        StoreObjPtr<ParticleList> ListOfParticles(particleListName);
        if (!ListOfParticles.isValid()) return 0;

        double maximumProb = 0;
        for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i)
        {
          const Particle* particlei = ListOfParticles->getParticle(i);
          if (!particlei) continue;

          double prob = 0;
          if (extraInfoName == "isRightTrack(MaximumPstar)") {
            ROOT::Math::PxPyPzEVector momParticlei = PCmsLabTransform::labToCms(particlei->get4Vector());
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


    // List of available extrainfos used in QpOf, weightedQpOf and variableOfTarget.
    const std::vector<std::string> availableExtraInfos = {"isRightTrack(Electron)",             // 0
                                                          "isRightTrack(IntermediateElectron)", // 1
                                                          "isRightTrack(Muon)",                 // 2
                                                          "isRightTrack(IntermediateMuon)",     // 3
                                                          "isRightTrack(KinLepton)",            // 4
                                                          "isRightTrack(IntermediateKinLepton)",// 5
                                                          "isRightTrack(Kaon)",                 // 6
                                                          "isRightTrack(SlowPion)",             // 7
                                                          "isRightTrack(FastHadron)",           // 8
                                                          "isRightTrack(MaximumPstar)",         // 9
                                                          "isRightTrack(Lambda)",               // 10
                                                          "isRightCategory(Electron)",             // 11
                                                          "isRightCategory(IntermediateElectron)", // 12
                                                          "isRightCategory(Muon)",                 // 13
                                                          "isRightCategory(IntermediateMuon)",     // 14
                                                          "isRightCategory(KinLepton)",            // 15
                                                          "isRightCategory(IntermediateKinLepton)",// 16
                                                          "isRightCategory(Kaon)",                 // 17
                                                          "isRightCategory(SlowPion)",             // 18
                                                          "isRightCategory(FastHadron)",           // 19
                                                          "isRightCategory(MaximumPstar)",         // 20
                                                          "isRightCategory(Lambda)",               // 21
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

      if (indexRanking == availableExtraInfos.size() or indexOutput == availableExtraInfos.size()) {
        string strAvailableForIsRightTrack;
        for (const auto& name : availableForIsRightTrack)
          strAvailableForIsRightTrack += name + " ";
        string strAvailableForIsRightCategory;
        for (const auto& name : availableForIsRightCategory)
          strAvailableForIsRightCategory += name + " ";

        B2FATAL("QpOf: Not available category " << rankingExtraInfo <<
                ". The possibilities for isRightTrack() are " << endl << strAvailableForIsRightTrack << " MaximumPstar" << endl <<
                "The possibilities for isRightCategory() are " << endl << strAvailableForIsRightCategory);
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
            ROOT::Math::PxPyPzEVector momParticlei = PCmsLabTransform::labToCms(particlei->get4Vector());
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
          qTarget = (-1) * target->getCharge();
        } else
        {
          qTarget = target->getCharge();
        }

        //Get the probability of being right classified flavor from event level
        double prob = target->getExtraInfo(availableExtraInfos[indexOutput]);

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


      if (indexRanking == availableExtraInfos.size() or indexOutput == availableExtraInfos.size()) {
        string strAvailableForIsRightTrack;
        for (const auto& name : availableForIsRightTrack)
          strAvailableForIsRightTrack += name + " ";
        string strAvailableForIsRightCategory;
        for (const auto& name : availableForIsRightCategory)
          strAvailableForIsRightCategory += name + " ";

        B2FATAL("weightedQpOf: Not available category " << rankingExtraInfo <<
                ". The possibilities for isRightTrack() are " << endl << strAvailableForIsRightTrack << " MaximumPstar" << endl <<
                "The possibilities for isRightCategory() are " << endl << strAvailableForIsRightCategory);
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
          double info1 = PCmsLabTransform::labToCms(part1->get4Vector()).P();
          double info2 = PCmsLabTransform::labToCms(part2->get4Vector()).P();
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


    Manager::FunctionPtr QpTrack(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 3) {
        B2FATAL("Wrong number of arguments (3 required) for meta function QpTrack");
      }

      auto particleListName = arguments[0];
      auto outputExtraInfo = arguments[1];
      auto rankingExtraInfo = arguments[2];

      unsigned indexRanking = find(availableExtraInfos.begin(), availableExtraInfos.end(),
                                   rankingExtraInfo) - availableExtraInfos.begin();
      unsigned indexOutput  = find(availableExtraInfos.begin(), availableExtraInfos.end(),
                                   outputExtraInfo)  - availableExtraInfos.begin();

      if (indexRanking == availableExtraInfos.size() or indexOutput == availableExtraInfos.size()) {
        string strAvailableForIsRightTrack;
        for (const auto& name : availableForIsRightTrack)
          strAvailableForIsRightTrack += name + " ";
        string strAvailableForIsRightCategory;
        for (const auto& name : availableForIsRightCategory)
          strAvailableForIsRightCategory += name + " ";

        B2FATAL("QpTrack: Not available category " << rankingExtraInfo <<
                ". The possibilities for isRightTrack() are " << endl << strAvailableForIsRightTrack << " MaximumPstar" << endl <<
                "The possibilities for isRightCategory() are " << endl << strAvailableForIsRightCategory);
      }

      auto func = [particleListName, indexOutput, indexRanking](const Particle * particle) -> double {
        StoreObjPtr<ParticleList> ListOfParticles(particleListName);
        if (!ListOfParticles.isValid()) return 0;

        const auto mdstIndex = particle->getMdstArrayIndex();

        Particle* target = nullptr; //Particle selected as target
        for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i)
        {
          Particle* particlei = ListOfParticles->getParticle(i);
          if (!particlei)
            continue;

          if (particlei->getMdstArrayIndex() == mdstIndex) {
            target = particlei;
            break;
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
          qTarget = (-1) * target->getCharge();
        } else
        {
          qTarget = target->getCharge();
        }

        //Get the probability of being right classified flavor from event level
        double prob = target->getExtraInfo(availableExtraInfos[indexOutput]);

        return qTarget * prob;
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
        string strAvailableForIsRightTrack;
        for (const auto& name : availableForIsRightTrack)
          strAvailableForIsRightTrack += name + " ";
        string strAvailableForIsRightCategory;
        for (const auto& name : availableForIsRightCategory)
          strAvailableForIsRightCategory += name + " ";

        B2FATAL("variableOfTarget: Not available category " << rankingExtraInfo <<
                ". The possibilities for isRightTrack() are " << endl << strAvailableForIsRightTrack << " MaximumPstar" << endl <<
                "The possibilities for isRightCategory() are " << endl << strAvailableForIsRightCategory);
      }

      const Variable::Manager::Var* var = Manager::Instance().getVariable(inputVariable);
      auto func = [particleListName, var, indexRanking](const Particle*) -> double {
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
            ROOT::Math::PxPyPzEVector momParticlei = PCmsLabTransform::labToCms(particlei->get4Vector());
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

        if (std::holds_alternative<double>(var->function(target)))
        {
          return std::get<double>(var->function(target));
        } else if (std::holds_alternative<int>(var->function(target)))
        {
          return std::get<int>(var->function(target));
        } else if (std::holds_alternative<bool>(var->function(target)))
        {
          return std::get<bool>(var->function(target));
        } else return realNaN;
      };
      return func;
    }

    Manager::FunctionPtr hasTrueTarget(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1) {
        B2FATAL("Wrong number of arguments (1 required) for meta function hasTrueTarget");
      }

      auto categoryName = arguments[0];

      bool isAvailable = false;
      for (const auto& name : availableForIsRightCategory) {
        if (categoryName == name) {
          isAvailable = true;
          break;
        }
      }
      if (categoryName == "mcAssociated")
        isAvailable = false;

      if (!isAvailable) {
        string strAvailableForIsRightCategory;
        for (const auto& name : availableForIsRightCategory) {
          if (name == "mcAssociated") continue;
          strAvailableForIsRightCategory += name + " ";
        }
        B2FATAL("hasTrueTarget: Not available category" << categoryName <<
                ". The possibilities for the category name are " << endl << strAvailableForIsRightCategory);
      }

      auto func = [categoryName](const Particle*) -> double {
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

        bool particlesHaveMCAssociated = false;
        int nTargets = 0;
        for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i)
        {
          Particle* iParticle = ListOfParticles->getParticle(i);
          if (!iParticle) continue;

          if (categoryName == "MaximumPstar") {
            static Manager::FunctionPtr selectionFunction =
            hasHighestProbInCat({"pi+:inRoe", "isRightTrack(MaximumPstar)"});
            bool targetFlag = std::get<bool>(selectionFunction(iParticle));
            if (targetFlag) {
              particlesHaveMCAssociated = true;
              ++nTargets;
            }
          } else {
            int targetFlag = std::get<int>(manager.getVariable("isRightTrack(" + trackTargetName + ")")->function(iParticle));
            if (targetFlag != -2) particlesHaveMCAssociated = true;
            if (targetFlag == 1) ++nTargets;
          }
        }

        if (!particlesHaveMCAssociated) return realNaN;
        return (nTargets > 0);
      };
      return func;
    }

    Manager::FunctionPtr isTrueCategory(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1) {
        B2FATAL("Wrong number of arguments (1 required) for meta function isTrueCategory");
      }
      auto categoryName = arguments[0];

      bool isAvailable = false;
      for (const auto& name : availableForIsRightCategory) {
        if (categoryName == name) {
          isAvailable = true;
          break;
        }
      }
      if (categoryName == "mcAssociated")
        isAvailable = false;

      if (!isAvailable) {
        string strAvailableForIsRightCategory;
        for (const auto& name : availableForIsRightCategory) {
          if (name == "mcAssociated") continue;
          strAvailableForIsRightCategory += name + " ";
        }
        B2FATAL("isTrueCategory: Not available category" << categoryName <<
                ". The possibilities for the category name are " << endl << strAvailableForIsRightCategory);
      }

      auto func = [categoryName](const Particle*) -> double {
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
        for (unsigned int i = 0; i < ListOfParticles->getListSize(); ++i)
        {
          Particle* iParticle = ListOfParticles->getParticle(i);
          if (!iParticle) continue;

          if (categoryName == "MaximumPstar") {
            static Manager::FunctionPtr selectionFunction =
            hasHighestProbInCat({"pi+:inRoe", "isRightTrack(MaximumPstar)"});
            if (std::get<bool>(selectionFunction(iParticle)))
              targetParticles.push_back(iParticle);
          } else if (std::get<int>(manager.getVariable("isRightTrack(" + trackTargetName + ")")->function(iParticle))) {
            targetParticles.push_back(iParticle);
          }
        }

        for (const auto& targetParticle : targetParticles)
        {
          int isTargetOfRightCategory = std::get<int>(manager.getVariable("isRightCategory(" +  categoryName + ")")->function(
                                                        targetParticle));
          if (isTargetOfRightCategory == 1) {
            output = 1;
            targetParticlesCategory.push_back(targetParticle);
          } else if (isTargetOfRightCategory == -2 && output != 1)
            output = realNaN;
        }

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
      auto func = [combinerMethod](const Particle * particle) -> int {

        int output = 0;
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

    VARIABLE_GROUP("Flavor Tagger Expert Variables");

    REGISTER_VARIABLE("pMissTag", momentumMissingTagSide, R"DOC(
[Expert] Calculates the missing momentum for a given particle on the tag side. 
:noindex:
)DOC","GeV/c");
    REGISTER_METAVARIABLE("pMissTag(maskName)", momentumMissingTagSideWithMask,
                          "[Expert] Calculates the missing momentum for a given particle on the tag side. The unit of the missing momentum is ``GeV/c`` ",
                          Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("cosTPTO"  , cosTPTO, R"DOC(
[Expert] Returns cosine of angle between thrust axis of given particle and thrust axis of ROE.
:noindex:
)DOC");
    REGISTER_METAVARIABLE("cosTPTO(maskName)", cosTPTOWithMask,
                          "[Expert] Returns cosine of angle between thrust axis of given particle and thrust axis of ROE.",
                          Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("lambdaFlavor", lambdaFlavor,
                      "[Expert] Returns 1.0 if particle is ``Lambda0``, -1.0 in case of ``anti-Lambda0``, 0.0 otherwise.");
    REGISTER_VARIABLE("isLambda", isLambda,  "[Expert] Returns 1.0 if particle is truth-matched to ``Lambda0``, 0.0 otherwise.");
    REGISTER_VARIABLE("lambdaZError", lambdaZError,  "[Expert] Returns the variance of the z-component of the decay vertex.\n\n",":math:`\\text{cm}^2`");
    REGISTER_VARIABLE("momentumOfSecondDaughter", momentumOfSecondDaughter,
                      "[Expert] Returns the momentum of second daughter if exists, 0. otherwise.\n\n","GeV/c");
    REGISTER_VARIABLE("momentumOfSecondDaughterCMS", momentumOfSecondDaughterCMS,
                      "[Expert] Returns the momentum of the second daughter in the centre-of-mass system, 0. if this daughter doesn't exist.\n\n","GeV/c");
    REGISTER_VARIABLE("chargeTimesKaonLiklihood", chargeTimesKaonLiklihood,
                      "[Expert] Returns ``q*(highest PID_Likelihood for Kaons)``, 0. otherwise.");
    REGISTER_VARIABLE("ptTracksRoe", transverseMomentumOfChargeTracksInRoe, R"DOC(
[Expert] Returns the transverse momentum of all charged tracks of the ROE related to the given particle, 0.0 if particle has no related ROE.
:noindex:
)DOC","GeV/c");
    REGISTER_METAVARIABLE("ptTracksRoe(maskName)", transverseMomentumOfChargeTracksInRoeWithMask,
                          "[Expert] Returns the transverse momentum of all charged tracks of the ROE related to the given particle, 0.0 if particle has no related ROE. The unit of the momentum is ``GeV/c`` ",
                          Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("pt2TracksRoe(maskName)", transverseMomentumSquaredOfChargeTracksInRoeWithMask,
                          "[Expert] Returns the transverse momentum squared of all charged tracks of the ROE related to the given particle, 0.0 if particle has no related ROE. The unit of the momentum squared is :math:`[\\text{GeV}/\\text{c}]^2` ",
                          Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("NumberOfKShortsInRoe", NumberOfKShortsInRoe,
                      "[Expert] Returns the number of ``K_S0`` in the rest of event. The particle list ``K_S0:inRoe`` has to be filled beforehand.");

    REGISTER_VARIABLE("isInElectronOrMuonCat", isInElectronOrMuonCat,
                      "[Expert] Returns 1.0 if the particle has been selected as target in the Muon or Electron Category, 0.0 otherwise.");

    REGISTER_VARIABLE("isMajorityInRestOfEventFromB0", isMajorityInRestOfEventFromB0, R"DOC(
[Eventbased][Expert] Checks if the majority of the tracks in the current RestOfEvent are from a ``B0``.
:noindex:
)DOC");
    REGISTER_METAVARIABLE("isMajorityInRestOfEventFromB0(maskName)", isMajorityInRestOfEventFromB0WithMask,
                          "[Eventbased][Expert] Checks if the majority of the tracks in the current RestOfEvent are from a ``B0``.",
                          Manager::VariableDataType::c_bool);
    REGISTER_VARIABLE("isMajorityInRestOfEventFromB0bar", isMajorityInRestOfEventFromB0bar, R"DOC(
[Eventbased][Expert] Check if the majority of the tracks in the current RestOfEvent are from a ``anti-B0``.
:noindex:
)DOC");
    REGISTER_METAVARIABLE("isMajorityInRestOfEventFromB0bar(maskName)", isMajorityInRestOfEventFromB0barWithMask,
                          "[Eventbased][Expert] Check if the majority of the tracks in the current RestOfEvent are from a ``anti-B0``.",
                          Manager::VariableDataType::c_bool);
    REGISTER_VARIABLE("hasRestOfEventTracks", hasRestOfEventTracks, R"DOC(
[Expert] Returns the number of tracks in the RestOfEvent related to the given Particle. -2 if the RestOfEvent is empty.
:noindex:
)DOC");
    REGISTER_METAVARIABLE("hasRestOfEventTracks(maskName)", hasRestOfEventTracksWithMask,
                          "[Expert] Returns the number of tracks in the RestOfEvent related to the given Particle. -2 if the RestOfEvent is empty.",
                          Manager::VariableDataType::c_bool);

    REGISTER_VARIABLE("qrCombined", isRestOfEventB0Flavor, R"DOC(
[Eventbased][Expert] Returns -1 (1) if current RestOfEvent is related to a ``anti-B0`` (``B0``). 
The ``MCError`` bit of Breco has to be 0, 1, 2, 16 or 1024. 
The output of the variable is 0 otherwise. 
If one particle in the RestOfEvent is found to belong to the reconstructed ``B0``, the output is -2(2) for a ``anti-B0`` (``B0``) on the reconstructed side.
)DOC");
    REGISTER_VARIABLE("ancestorHasWhichFlavor", ancestorHasWhichFlavor,
                      "[Expert] Checks the decay chain of the given particle upwards up to the ``Upsilon(4S)`` resonance and outputs 0 (1) if an ancestor is found to be a ``anti-B0`` (``B0``), if not -2.");
    REGISTER_VARIABLE("B0mcErrors", B0mcErrors, "[Expert] Returns MC-matching flag, see :b2:var:`mcErrors` for the particle, e.g. ``B0`` .");
    REGISTER_VARIABLE("isRelatedRestOfEventMajorityB0Flavor", isRelatedRestOfEventMajorityB0Flavor, R"DOC(
[Expert] Returns 0 (1) if the majority of tracks and clusters of the RestOfEvent related to the given Particle are related to a ``anti-B0`` (``B0``).
:noindex:
)DOC");
    REGISTER_METAVARIABLE("isRelatedRestOfEventMajorityB0Flavor(maskName)", isRelatedRestOfEventMajorityB0FlavorWithMask,
                      "[Expert] Returns 0 (1) if the majority of tracks and clusters of the RestOfEvent related to the given Particle are related to a ``anti-B0`` (``B0``).",
                      Manager::VariableDataType::c_int);
    REGISTER_VARIABLE("isRestOfEventMajorityB0Flavor", isRestOfEventMajorityB0Flavor,
                      "[Expert] Returns 0 (1) if the majority of tracks and clusters of the current RestOfEvent are related to a ``anti-B0`` (``B0``).");
    REGISTER_VARIABLE("mcFlavorOfOtherB", mcFlavorOfOtherB,  R"DOC(
[Expert] Returns the MC flavor (+1 or -1) of the accompanying tag-side B meson if the given particle is a correctly truth-matched B candidate, 0 otherwise.
In other words, this variable checks the generated flavor of the other generated ``Upsilon(4S)`` daughter.
)DOC");


    REGISTER_METAVARIABLE("BtagToWBosonVariables(requestedVariable[, maskName])", BtagToWBosonVariables, R"DOC(
[Eventbased][Expert] Returns values of FlavorTagging-specific kinematical variables assuming a semileptonic decay with the given particle as target.
The input values of ``requestedVariable`` can be the following:  recoilMass in GeV/c^2 , pMissCMS in ``GeV/c``, cosThetaMissCMS and EW90.
)DOC", Manager::VariableDataType::c_double);
  REGISTER_METAVARIABLE("KaonPionVariables(requestedVariable)"  , KaonPionVariables , R"DOC(
[Expert] Returns values of FlavorTagging-specific kinematical variables for ``KaonPion`` category.
The input values of ``requestedVariable`` can be the following:  cosKaonPion, HaveOpositeCharges.
)DOC", Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("FSCVariables(requestedVariable)", FSCVariables, R"DOC(
[Eventbased][Expert] Returns values of FlavorTagging-specific kinematical variables for ``FastSlowCorrelated`` category.
The input values of ``requestedVariable`` can be the following: pFastCMS in ``GeV/c``, cosSlowFast, SlowFastHaveOpositeCharges, or cosTPTOFast.
)DOC", Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("hasHighestProbInCat(particleListName, extraInfoName)", hasHighestProbInCat, R"DOC(
[Expert] Returns 1.0 if the given Particle is classified as target track, i.e. if it has the highest target track probability in particleListName. 
The probability is accessed via ``extraInfoName``, which can have the following input values:

* isRightTrack(Electron),
* isRightTrack(IntermediateElectron),
* isRightTrack(Muon),
* isRightTrack(IntermediateMuon),
* isRightTrack(KinLepton),
* isRightTrack(IntermediateKinLepton),
* isRightTrack(Kaon),
* isRightTrack(SlowPion),
* isRightTrack(FastHadron),
* isRightTrack(MaximumPstar),
* isRightTrack(Lambda),
* isRightCategory(Electron),
* isRightCategory(IntermediateElectron),
* isRightCategory(Muon),
* isRightCategory(IntermediateMuon),
* isRightCategory(KinLepton),
* isRightCategory(IntermediateKinLepton),
* isRightCategory(Kaon),
* isRightCategory(SlowPion),
* isRightCategory(FastHadron),
* isRightCategory(MaximumPstar),
* isRightCategory(Lambda),
* isRightCategory(KaonPion),
* isRightCategory(FSC).

)DOC", Manager::VariableDataType::c_bool);
    REGISTER_METAVARIABLE("HighestProbInCat(particleListName, extraInfoName)", HighestProbInCat,
                      "[Expert] Returns the highest target track probability value for the given category, for allowed input values for ``extraInfoName`` see :b2:var:`hasHighestProbInCat`.", Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("isRightTrack(particleName)", isRightTrack, R"DOC(
[Expert] Returns 1.0 if the given particle was really from a B-meson depending on category provided in ``particleName`` argument, 0.0 otherwise.
Allowed input values for ``particleName`` argument in this variable are the following:

* Electron,
* IntermediateElectron,
* Muon,
* IntermediateMuon,
* KinLepton,
* IntermediateKinLepton,
* Kaon,
* SlowPion,
* FastHadron,
* Lambda,
* mcAssociated.

)DOC", Manager::VariableDataType::c_int);
    REGISTER_METAVARIABLE("isRightCategory(particleName)", isRightCategory,  R"DOC(
[Expert] Returns 1.0 if the class track by ``particleName`` category has the same flavor as the MC target track, 0.0 otherwise.
Allowed input values for ``particleName`` argument in this variable are the following:

* Electron,
* IntermediateElectron,
* Muon,
* IntermediateMuon,
* KinLepton,
* IntermediateKinLepton
* Kaon,
* SlowPion,
* FastHadron, 
* KaonPion,
* MaximumPstar,
* FSC, 
* Lambda,
* mcAssociated.

)DOC", Manager::VariableDataType::c_int);
    REGISTER_METAVARIABLE("QpOf(particleListName, outputExtraInfo, rankingExtraInfo)", QpOf,  R"DOC(
[Eventbased][Expert] Returns the :math:`q*p` value for a given particle list provided as the 1st argument, 
where :math:`p` is the probability of a category stored as extraInfo, provided as the 2nd argument, 
allowed values are same as in :b2:var:`hasHighestProbInCat`.
The particle is selected after ranking according to a flavor tagging extraInfo, provided as the 3rd argument, 
allowed values are same as in :b2:var:`hasHighestProbInCat`.
)DOC", Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("weightedQpOf(particleListName, outputExtraInfo, rankingExtraInfo)", weightedQpOf, R"DOC(
[Eventbased][Expert] Returns the weighted :math:`q*p` value for a given particle list, provided as the  1st argument, 
where :math:`p` is the probability of a category stored as extraInfo, provided in the 2nd argument, 
allowed values are same as in :b2:var:`hasHighestProbInCat`.
The particles in the list are ranked according to a flavor tagging extraInfo, provided as the 3rd argument, 
allowed values are same as in :b2:var:`hasHighestProbInCat`.
The values for the three top particles is combined into an effective (weighted) output.
)DOC", Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("QpTrack(particleListName, outputExtraInfo, rankingExtraInfo)", QpTrack,  R"DOC(
[Expert] Returns the :math:`q*p` value of the particle in a given particle list provided as the 1st argument that is originated from the same Track of given particle.
where :math:`p` is the probability of a category stored as extraInfo, provided as the 2nd argument, 
allowed values are same as in :b2:var:`hasHighestProbInCat`.
The particle is selected after ranking according to a flavor tagging extraInfo, provided as the 3rd argument, 
allowed values are same as in :b2:var:`hasHighestProbInCat`.
)DOC", Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("variableOfTarget(particleListName, inputVariable, rankingExtraInfo)", variableOfTarget, R"DOC(
[Eventbased][Expert] Returns the value of an input variable provided as the 2nd argument for a particle selected from the given list provided as the 1st argument.
The particles are ranked according to a flavor tagging extraInfo, provided as the 2nd argument, 
allowed values are same as in :b2:var:`hasHighestProbInCat`.
)DOC", Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("hasTrueTarget(categoryName)", hasTrueTarget,
                      "[Expert] Returns 1 if the given category has a target, 0 otherwise.", Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("isTrueCategory(categoryName)", isTrueCategory,
                      "[Expert] Returns 1 if the given category tags the B0 MC flavor correctly, 0 otherwise.", Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("qpCategory(categoryName)", qpCategory, R"DOC(
[Expert] Returns the output :math:`q` (charge of target track) times :math:`p` (probability that this is the right category) of the category with the given name. 
The allowed categories are the official Flavor Tagger Category Names.
)DOC", Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("isTrueFTCategory(categoryName)", isTrueFTCategory, R"DOC(
[Expert] Returns 1 if the target particle (checking the decay chain) of the category with the given name is found in the MC particles, 
and if it provides the right flavor. The allowed categories are the official Flavor Tagger Category Names.
)DOC", Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("hasTrueTargets(categoryName)", hasTrueTargets, R"DOC(
[Expert] Returns 1 if target particles (checking only the decay chain) of the category with the given name is found in the MC particles. 
The allowed categories are the official Flavor Tagger Category Names.
)DOC", Manager::VariableDataType::c_double);

    VARIABLE_GROUP("Flavor Tagger Analysis Variables")

    REGISTER_METAVARIABLE("rBinBelle(combinerMethod)", rBinBelle, R"DOC(
Returns the corresponding :math:`r` (dilution) bin according to the Belle binning for the given ``combinerMethod``. 
The available methods are 'FBDT' and 'FANN' (category-based combiners), and 'DNN' (DNN tagger output).
The return values and the corresponding dilution ranges are the following:

* 0: :math:`0.000 < r < 0.100`;
* 1: :math:`0.100 < r < 0.250`;
* 2: :math:`0.250 < r < 0.500`;
* 3: :math:`0.500 < r < 0.625`;
* 4: :math:`0.625 < r < 0.750`;
* 5: :math:`0.750 < r < 0.875`;
* 6: :math:`0.875 < r < 1.000`.

.. warning:: You have to run the Flavor Tagger for this variable to be meaningful.
.. seealso:: :ref:`FlavorTagger` and :func:`flavorTagger.flavorTagger`.
)DOC", Manager::VariableDataType::c_int);
    REGISTER_METAVARIABLE("qrOutput(combinerMethod)", qrOutput, R"DOC(
Returns the output of the flavorTagger, flavor tag :math:`q` times the dilution factor :math:`r`, for the given combiner method. 
The available methods are 'FBDT' and 'FANN' (category-based combiners), and 'DNN' (DNN tagger output).

.. warning:: You have to run the Flavor Tagger for this variable to be meaningful.
.. seealso:: :ref:`FlavorTagger` and :func:`flavorTagger.flavorTagger`.
)DOC", Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("qOutput(combinerMethod)", qOutput, R"DOC(
Returns the flavor tag :math:`q` output of the flavorTagger for the given combinerMethod. 
The available methods are 'FBDT' and 'FANN' (category-based combiners), and 'DNN' (DNN tagger output).

.. warning:: You have to run the Flavor Tagger for this variable to be meaningful.
.. seealso:: :ref:`FlavorTagger` and :func:`flavorTagger.flavorTagger`.
)DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("isRelatedRestOfEventB0Flavor", isRelatedRestOfEventB0Flavor,  R"DOC(
Returns -1 (1) if the RestOfEvent related to the given particle is related to a ``anti-B0`` (``B0``). 
The ``MCError`` bit of Breco has to be 0, 1, 2, 16 or 1024. 
The output of the variable is 0 otherwise. 
If one particle in the RestOfEvent is found to belong to the reconstructed ``B0``, the output is -2(2) for a ``anti-B0`` (``B0``) on the reconstructed side.
)DOC");
  }
}
