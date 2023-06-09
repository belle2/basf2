/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/variables/ROEVariables.h>

#include <analysis/variables/Variables.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>

// framework aux
#include <framework/logging/Logger.h>
#include <framework/utilities/Conversion.h>
#include <framework/gearbox/Const.h>

// utility
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ReferenceFrame.h>

#include <TRandom.h>
#include <TMath.h>
#include <Math/AxisAngle.h>

#include <iostream>

using namespace std;

namespace Belle2 {
  namespace Variable {

    double isInRestOfEvent(const Particle* particle)
    {

      StoreObjPtr<RestOfEvent> roeobjptr;
      if (not roeobjptr.isValid())
        return 0;

      const RestOfEvent* roe = &(*roeobjptr);

      return isInThisRestOfEvent(particle, roe);
    }

    double isCloneOfSignalSide(const Particle* particle)
    {

      StoreObjPtr<RestOfEvent> roe;
      if (not roe.isValid()) {
        B2WARNING("Please use isCloneOfSignalSide variable in for_each ROE loop!");
        return Const::doubleNaN;
      }
      auto* particleMC = particle->getMCParticle();
      if (!particleMC) {
        return 0.0;
      }
      auto* signal = roe->getRelatedFrom<Particle>();
      auto signalFSPs = signal->getFinalStateDaughters();
      for (auto* daughter : signalFSPs) {
        auto* daughterMC = daughter->getMCParticle();
        if (daughterMC == particleMC) {
          return 1.0;
        }
      }
      return 0.0;
    }

    double hasAncestorFromSignalSide(const Particle* particle)
    {
      StoreObjPtr<RestOfEvent> roe;
      if (!roe.isValid()) {
        B2WARNING("Please use hasAncestorFromSignalSide variable in for_each ROE loop!");
        return Const::doubleNaN;
      }
      auto* particleMC = particle->getMCParticle();
      if (!particleMC) {
        return 0.0;
      }
      auto* signalReco = roe->getRelatedFrom<Particle>();
      auto* signalMC = signalReco->getMCParticle();
      MCParticle* ancestorMC = particleMC->getMother();
      while (ancestorMC) {
        if (ancestorMC == signalMC) {
          return 1.0;
        }
        ancestorMC = ancestorMC->getMother();
      }
      return 0.0;
    }

    Manager::FunctionPtr currentROEIsInList(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments (1 required) for meta function currentROEIsInList");

      std::string listName = arguments[0];

      auto func = [listName](const Particle*) -> double {

        StoreObjPtr<ParticleList> particleList(listName);
        if (!(particleList.isValid()))
        {
          B2FATAL("Invalid Listname " << listName << " given to currentROEIsInList!");
        }
        StoreObjPtr<RestOfEvent> roe("RestOfEvent");

        if (not roe.isValid())
          return 0;

        auto* particle = roe->getRelatedFrom<Particle>();
        if (particle == nullptr)
        {
          B2ERROR("Relation between particle and ROE doesn't exist! currentROEIsInList() variable has to be called from ROE loop");
          return Const::doubleNaN;
        }
        return particleList->contains(particle) ? 1 : 0;

      };
      return func;
    }

    Manager::FunctionPtr particleRelatedToCurrentROE(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments (1 required) for meta function particleRelatedToCurrentROE");

      const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
      auto func = [var](const Particle*) -> double {

        StoreObjPtr<RestOfEvent> roe("RestOfEvent");

        if (not roe.isValid())
          return Const::doubleNaN;

        auto* particle = roe->getRelatedFrom<Particle>();
        if (particle == nullptr)
        {
          B2ERROR("Relation between particle and ROE doesn't exist! particleRelatedToCurrentROE() variable has to be called from ROE loop");
          return Const::doubleNaN;
        }
        if (std::holds_alternative<double>(var->function(particle)))
        {
          return std::get<double>(var->function(particle));
        } else if (std::holds_alternative<int>(var->function(particle)))
        {
          return std::get<int>(var->function(particle));
        } else if (std::holds_alternative<bool>(var->function(particle)))
        {
          return std::get<bool>(var->function(particle));
        } else return Const::doubleNaN;

      };
      return func;
    }

    Manager::FunctionPtr useROERecoilFrame(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
        auto func = [var](const Particle * particle) -> double {
          // Here we prioritize old variable behaviour first:
          const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();
          // if related ROE not found, get the StoreArray pointer
          if (roe == nullptr)
          {
            StoreObjPtr<RestOfEvent> roeObjPtr("RestOfEvent");
            if (roeObjPtr.isValid()) {
              roe = &*roeObjPtr;
            }
          }
          if (roe == nullptr)
          {
            B2ERROR("Neither relation between particle and ROE doesn't exist nor ROE object has not been found!");
            return Const::doubleNaN;
          }
          PCmsLabTransform T;
          ROOT::Math::PxPyPzEVector pRecoil = T.getBeamFourMomentum() - roe->get4Vector();
          Particle tmp(pRecoil, 0);
          UseReferenceFrame<RestFrame> frame(&tmp);
          if (std::holds_alternative<double>(var->function(particle)))
          {
            return std::get<double>(var->function(particle));
          } else if (std::holds_alternative<int>(var->function(particle)))
          {
            return std::get<int>(var->function(particle));
          } else if (std::holds_alternative<bool>(var->function(particle)))
          {
            return std::get<bool>(var->function(particle));
          } else return Const::doubleNaN;
        };
        return func;
      } else {
        B2WARNING("Wrong number of arguments for meta function useROERecoilFrame");
        return nullptr;
      }
    }

    // only the helper function
    int nRemainingTracksInROE(const Particle* particle, const std::string& maskName)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (not roe.isValid())
        return 0;
      int n_roe_tracks = roe->getNTracks(maskName);
      int n_par_tracks = 0;
      const auto& daughters = particle->getFinalStateDaughters();
      for (const auto& daughter : daughters) {
        if (daughter->getParticleSource() == Particle::EParticleSourceObject::c_Track && roe->hasParticle(daughter, maskName)) {
          n_par_tracks++;
        }
      }
      return n_roe_tracks - n_par_tracks;
    }

    Manager::FunctionPtr nROE_RemainingTracksWithMask(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function nROETracks");

      auto func = [maskName](const Particle * particle) -> int {
        return nRemainingTracksInROE(particle, maskName);
      };
      return func;
    }

    int nROE_RemainingTracks(const Particle* particle)
    {
      return nRemainingTracksInROE(particle);
    }

    double nROE_KLMClusters(const Particle* particle)
    {
      // Get related ROE object
      const RestOfEvent* roe = getRelatedROEObject(particle);

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        return Const::doubleNaN;
      }

      return roe->getNKLMClusters();
    }

    double ROE_MC_E(const Particle* particle)
    {
      const MCParticle* mcp = particle->getMCParticle();

      if (!mcp)
        return Const::doubleNaN;

      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector boostvec = T.getBeamFourMomentum();
      auto mcroe4vector = boostvec - mcp->get4Vector();
      const auto& frame = ReferenceFrame::GetCurrent();
      auto frameMCRoe4Vector = frame.getMomentum(mcroe4vector);
      return frameMCRoe4Vector.energy();
    }

    double ROE_MC_P(const Particle* particle)
    {
      const MCParticle* mcp = particle->getMCParticle();

      if (!mcp)
        return Const::doubleNaN;

      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector boostvec = T.getBeamFourMomentum();
      auto mcroe4vector = boostvec - mcp->get4Vector();
      const auto& frame = ReferenceFrame::GetCurrent();
      auto frameMCRoe4Vector = frame.getMomentum(mcroe4vector);
      return frameMCRoe4Vector.P();
    }

    double ROE_MC_Px(const Particle* particle)
    {
      const MCParticle* mcp = particle->getMCParticle();

      if (!mcp)
        return Const::doubleNaN;

      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector boostvec = T.getBeamFourMomentum();
      auto mcroe4vector = boostvec - mcp->get4Vector();
      const auto& frame = ReferenceFrame::GetCurrent();
      auto frameMCRoe4Vector = frame.getMomentum(mcroe4vector);

      return frameMCRoe4Vector.px();
    }

    double ROE_MC_Py(const Particle* particle)
    {
      const MCParticle* mcp = particle->getMCParticle();

      if (!mcp)
        return Const::doubleNaN;

      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector boostvec = T.getBeamFourMomentum();
      auto mcroe4vector = boostvec - mcp->get4Vector();
      const auto& frame = ReferenceFrame::GetCurrent();
      auto frameMCRoe4Vector = frame.getMomentum(mcroe4vector);

      return frameMCRoe4Vector.py();
    }

    double ROE_MC_Pz(const Particle* particle)
    {
      const MCParticle* mcp = particle->getMCParticle();

      if (!mcp)
        return Const::doubleNaN;

      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector boostvec = T.getBeamFourMomentum();
      auto mcroe4vector = boostvec - mcp->get4Vector();
      const auto& frame = ReferenceFrame::GetCurrent();
      auto frameMCRoe4Vector = frame.getMomentum(mcroe4vector);

      return frameMCRoe4Vector.pz();
    }

    double ROE_MC_Pt(const Particle* particle)
    {
      const MCParticle* mcp = particle->getMCParticle();

      if (!mcp)
        return Const::doubleNaN;

      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector boostvec = T.getBeamFourMomentum();
      auto mcroe4vector = boostvec - mcp->get4Vector();
      const auto& frame = ReferenceFrame::GetCurrent();
      auto frameMCRoe4Vector = frame.getMomentum(mcroe4vector);

      return frameMCRoe4Vector.pt();
    }

    double ROE_MC_PTheta(const Particle* particle)
    {
      const MCParticle* mcp = particle->getMCParticle();

      if (!mcp)
        return Const::doubleNaN;

      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector boostvec = T.getBeamFourMomentum();
      auto mcroe4vector = boostvec - mcp->get4Vector();
      const auto& frame = ReferenceFrame::GetCurrent();
      auto frameMCRoe4Vector = frame.getMomentum(mcroe4vector);

      return frameMCRoe4Vector.Theta();
    }

    double ROE_MC_M(const Particle* particle)
    {
      const MCParticle* mcp = particle->getMCParticle();

      if (!mcp)
        return Const::doubleNaN;

      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector boostvec = T.getBeamFourMomentum();

      return (boostvec - mcp->get4Vector()).M();
    }

    Manager::FunctionPtr ROE_MC_MissingFlags(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function nROETracks");

      auto func = [maskName](const Particle * particle) -> double {

        StoreArray<Particle> particles;

        //Get MC Particle of the B meson
        const MCParticle* mcParticle = particle->getMCParticle();

        if (!mcParticle)
          return Const::doubleNaN;

        // Get Mother
        const MCParticle* mcMother = mcParticle->getMother();

        if (!mcMother)
          return Const::doubleNaN;

        // Get daughters
        std::vector<MCParticle*> mcDaughters = mcMother->getDaughters();

        if (mcDaughters.size() != 2)
          return Const::doubleNaN;

        // Get the companion B meson
        MCParticle* mcROE = nullptr;
        if (mcDaughters[0]->getArrayIndex() == mcParticle->getArrayIndex())
          mcROE = mcDaughters[1];
        else
          mcROE = mcDaughters[0];

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        std::set<const MCParticle*> mcROEObjects;

        auto roeParticles = roe->getParticles(maskName);
        for (auto* roeParticle : roeParticles)
        {
          auto* mcroeParticle = roeParticle->getMCParticle();
          if (mcroeParticle != nullptr) {
            mcROEObjects.insert(mcroeParticle);
          }
        }
        int flags = 0;
        checkMCParticleMissingFlags(mcROE, mcROEObjects, flags);

        return flags;
      };
      return func;
    }

    Manager::FunctionPtr nROE_Tracks(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function nROETracks");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        return roe->getNTracks(maskName);
      };
      return func;
    }

    Manager::FunctionPtr nROE_ECLClusters(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function nROEECLClusters");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        return roe->getNECLClusters(maskName);
      };
      return func;
    }

    Manager::FunctionPtr nROE_NeutralECLClusters(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function nROENeutralECLClusters");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        return roe->getPhotons(maskName).size();
      };
      return func;
    }

    Manager::FunctionPtr nROE_Photons(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1) {
        maskName = arguments[0];
      } else if (arguments.size() > 1) {
        B2FATAL("At most 1 argument (name of mask) accepted for meta function nROE_Photons");
      }
      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        // Get unused ECLClusters in ROE
        auto roeClusters = roe->getPhotons(maskName);
        int nPhotons = 0;

        // Select ECLClusters with photon hypothesis
        for (auto& roeCluster : roeClusters)
          if (roeCluster->getECLClusterEHypothesisBit() == ECLCluster::EHypothesisBit::c_nPhotons)
            nPhotons++;
        return nPhotons;
      };
      return func;
    }

    Manager::FunctionPtr nROE_NeutralHadrons(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1) {
        maskName = arguments[0];
      } else if (arguments.size() > 1) {
        B2FATAL("At most 1 argument (name of mask) accepted for meta function nROE_NeutralHadrons");
      }
      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        return roe->getHadrons(maskName).size();
      };
      return func;
    }

    Manager::FunctionPtr nROE_ChargedParticles(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;
      int pdgCode = 0;
      if (arguments.size() == 1) {
        maskName = arguments[0];
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        try {
          pdgCode = Belle2::convertString<int>(arguments[1]);
        } catch (std::invalid_argument&) {
          B2ERROR("First argument of nROE_ChargedParticles must be a PDG code");
          return nullptr;
        }
      } else if (arguments.size() > 2)  {
        B2FATAL("Wrong number of arguments (2 optional) for meta function nROE_ChargedParticles");
      }
      auto func = [maskName, pdgCode](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        return roe->getChargedParticles(maskName, abs(pdgCode)).size();
      };
      return func;
    }

    Manager::FunctionPtr nROE_Composites(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1) {
        maskName = arguments[0];
      } else if (arguments.size() > 1) {
        B2FATAL("At most 1 argument (name of mask) accepted for meta function nROE_Composites");
      }
      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }
        int result = 0;
        auto particles = roe->getParticles(maskName, false);

        for (auto roeParticle : particles)
        {
          if (roeParticle->getParticleSource() == Particle::c_Composite or
              roeParticle->getParticleSource() == Particle::c_V0) {
            result++;
          }
        }
        return result;
      };
      return func;
    }

    Manager::FunctionPtr nROE_ParticlesInList(const std::vector<std::string>& arguments)
    {
      std::string pListName;
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1) {
        pListName = arguments[0];
      } else if (arguments.size() == 2) {
        pListName = arguments[0];
        maskName = arguments[1];
      } else
        B2FATAL("Wrong number of arguments (1 or 2 required) for meta function nROE_ParticlesInList");

      auto func = [pListName, maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        int nPart = 0;

        // Get particle list
        StoreObjPtr<ParticleList> pList(pListName);
        if (!pList.isValid())
          B2FATAL("ParticleList " << pListName << " could not be found or is not valid!");

        for (unsigned int i = 0; i < pList->getListSize(); i++)
        {
          const Particle* part = pList->getParticle(i);
          if (isInThisRestOfEvent(part, roe, maskName))
            ++nPart;
        }

        return nPart;
      };
      return func;
    }

    Manager::FunctionPtr ROE_Charge(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function ROECharge");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        // Get tracks in ROE
        auto roeParticles = roe->getParticles(maskName);
        int roeCharge = 0;

        for (auto* roeParticle : roeParticles)
        {
          roeCharge += roeParticle->getCharge();
        }

        return roeCharge;
      };
      return func;
    }

    Manager::FunctionPtr ROE_ExtraEnergy(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function extraEnergy");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        double extraE = 0.0;

        auto roeClusters = roe->getPhotons(maskName);

        for (auto& roeCluster : roeClusters)
          if (roeCluster->getECLClusterEHypothesisBit() == ECLCluster::EHypothesisBit::c_nPhotons)
            extraE += roeCluster->getECLClusterEnergy();

        auto roeChargedParticles = roe->getChargedParticles(maskName);

        for (auto& roeChargedParticle : roeChargedParticles)
        {
          if (roeChargedParticle->getECLCluster())
            extraE += roeChargedParticle->getECLClusterEnergy();
        }

        return extraE;
      };
      return func;
    }

    Manager::FunctionPtr ROE_NeutralExtraEnergy(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function extraEnergy");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }
        auto roephotons = roe->getPhotons(maskName);
        ROOT::Math::PxPyPzEVector total4vector;
        for (auto* photon : roephotons)
        {
          total4vector += photon->get4Vector();
        }
        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(total4vector);
        return frameRoe4Vector.energy();
      };
      return func;
    }

    Manager::FunctionPtr ROE_E(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function ROE_E");

      auto func = [maskName](const Particle * particle) -> double {
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();
        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }
        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(roe->get4Vector(maskName));
        return frameRoe4Vector.energy();
      };
      return func;
    }

    Manager::FunctionPtr ROE_M(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function ROE_M");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        return roe->get4Vector(maskName).M();
      };
      return func;
    }

    Manager::FunctionPtr ROE_P(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function ROE_P");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(roe->get4Vector(maskName));
        return frameRoe4Vector.P();
      };
      return func;
    }

    Manager::FunctionPtr ROE_Px(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function ROE_Px");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(roe->get4Vector(maskName));
        return frameRoe4Vector.px();
      };
      return func;
    }

    Manager::FunctionPtr ROE_Py(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function ROE_Py");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(roe->get4Vector(maskName));
        return frameRoe4Vector.py();
      };
      return func;
    }

    Manager::FunctionPtr ROE_Pt(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function ROE_Pt");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(roe->get4Vector(maskName));
        return frameRoe4Vector.pt();
      };
      return func;
    }

    Manager::FunctionPtr ROE_Pz(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function ROE_Pz");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(roe->get4Vector(maskName));
        return frameRoe4Vector.pz();
      };
      return func;
    }

    Manager::FunctionPtr ROE_PTheta(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function ROE_PTheta");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(roe->get4Vector(maskName));
        return frameRoe4Vector.Theta();
      };
      return func;
    }

    Manager::FunctionPtr ROE_DeltaE(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function ROE_deltae");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        PCmsLabTransform T;
        ROOT::Math::PxPyPzEVector vec = T.rotateLabToCms() * roe->get4Vector(maskName);
        return vec.E() - T.getCMSEnergy() / 2;
      };
      return func;
    }

    Manager::FunctionPtr ROE_Mbc(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function ROE_mbc");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        PCmsLabTransform T;
        ROOT::Math::PxPyPzEVector vec = T.rotateLabToCms() * roe->get4Vector(maskName);

        double E = T.getCMSEnergy() / 2;
        double m2 = E * E - vec.P2();
        double mbc = m2 > 0 ? sqrt(m2) : 0;

        return mbc;
      };
      return func;
    }

    Manager::FunctionPtr bssMassDifference(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function bssMassDifference");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        ROOT::Math::PxPyPzEVector neutrino4vec = missing4Vector(particle->getDaughter(0), maskName, "6");
        ROOT::Math::PxPyPzEVector sig4vec = particle->getDaughter(0)->get4Vector();

        ROOT::Math::PxPyPzEVector bsMom = neutrino4vec + sig4vec;
        ROOT::Math::PxPyPzEVector bssMom = bsMom + particle->getDaughter(1)->get4Vector();

        return bssMom.M() - bsMom.M();
      };
      return func;
    }

    Manager::FunctionPtr WE_DeltaE(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string opt;

      if (arguments.size() == 1) {
        maskName = RestOfEvent::c_defaultMaskName;
        opt = arguments[0];
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        opt = arguments[1];
      } else
        B2FATAL("Wrong number of arguments (2 required) for meta function correctedB_deltae");

      auto func = [maskName, opt](const Particle * particle) -> double {

        PCmsLabTransform T;
        ROOT::Math::PxPyPzEVector boostvec = T.getBeamFourMomentum();
        ROOT::Math::PxPyPzEVector sig4vec = T.rotateLabToCms() * particle->get4Vector();
        ROOT::Math::PxPyPzEVector sig4vecLAB = particle->get4Vector();
        ROOT::Math::PxPyPzEVector neutrino4vec = missing4Vector(particle, maskName, "1");
        ROOT::Math::PxPyPzEVector neutrino4vecLAB = missing4Vector(particle, maskName, "6");

        double deltaE = Const::doubleNaN;

        // Definition 0: CMS
        if (opt == "0")
        {
          double totalSigEnergy = (sig4vec + neutrino4vec).energy();
          double E = T.getCMSEnergy() / 2;
          deltaE = totalSigEnergy - E;
        }

        // Definition 1: LAB
        else if (opt == "1")
        {
          double Ecms = T.getCMSEnergy();
          double s = Ecms * Ecms;
          deltaE = ((sig4vecLAB + neutrino4vecLAB).Dot(boostvec) - s / 2.0) / sqrt(s);
        }

        else
          B2FATAL("Option for correctedB_deltae variable should only be 0/1 (CMS/LAB)");

        return deltaE;
      };
      return func;
    }

    Manager::FunctionPtr WE_Mbc(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string opt;

      if (arguments.size() == 1) {
        maskName = RestOfEvent::c_defaultMaskName;
        opt = arguments[0];
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        opt = arguments[1];
      } else
        B2FATAL("Wrong number of arguments (2 required) for meta function correctedB_mbc");

      auto func = [maskName, opt](const Particle * particle) -> double {

        PCmsLabTransform T;
        ROOT::Math::PxPyPzEVector boostvec = T.getBeamFourMomentum();
        ROOT::Math::PxPyPzEVector sig4vec = T.rotateLabToCms() * particle->get4Vector();
        ROOT::Math::PxPyPzEVector sig4vecLAB = particle->get4Vector();
        ROOT::Math::PxPyPzEVector neutrino4vec;

        double mbc = Const::doubleNaN;

        // Definition 0: CMS
        if (opt == "0")
        {
          neutrino4vec = missing4Vector(particle, maskName, "1");
          B2Vector3D bmom = (sig4vec + neutrino4vec).Vect();
          double E = T.getCMSEnergy() / 2;
          double m2 = E * E - bmom.Mag2();
          mbc = m2 > 0 ? sqrt(m2) : 0;
        }

        // Definition 1: LAB
        else if (opt == "1")
        {
          neutrino4vec = missing4Vector(particle, maskName, "6");
          B2Vector3D bmom = (sig4vecLAB + neutrino4vec).Vect();
          double Ecms = T.getCMSEnergy();
          double s = Ecms * Ecms;
          double m2 = pow((s / 2.0 + bmom * B2Vector3D(boostvec.Vect())) / boostvec.energy(), 2.0) - bmom.Mag2();
          mbc = m2 > 0 ? sqrt(m2) : 0;
        }

        // Definition 2: CMS with factor alpha (so that dE == 0)
        else if (opt == "2")
        {
          neutrino4vec = missing4Vector(particle, maskName, "7");
          B2Vector3D bmom = (sig4vec + neutrino4vec).Vect();
          double E = T.getCMSEnergy() / 2;
          double m2 = E * E - bmom.Mag2();
          mbc = m2 > 0 ? sqrt(m2) : 0;
        }

        else
          B2FATAL("Option for correctedB_mbc variable should only be 0/1/2 (CMS/LAB/CMS with factor)");

        return mbc;
      };
      return func;
    }

    Manager::FunctionPtr WE_MissM2(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string opt;

      if (arguments.size() == 1) {
        maskName = RestOfEvent::c_defaultMaskName;
        opt = arguments[0];
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        opt = arguments[1];
      } else
        B2FATAL("Wrong number of arguments (2 required) for meta function missM2");

      auto func = [maskName, opt](const Particle * particle) -> double {

        return missing4Vector(particle, maskName, opt).M2();
      };
      return func;
    }

    Manager::FunctionPtr WE_MissPTheta(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string opt;

      if (arguments.size() == 1) {
        maskName = RestOfEvent::c_defaultMaskName;
        opt = arguments[0];
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        opt = arguments[1];
      } else
        B2FATAL("Wrong number of arguments (2 required) for meta function WE_MissPTheta");

      auto func = [maskName, opt](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        return missing4Vector(particle, maskName, opt).Theta();
      };
      return func;
    }

    Manager::FunctionPtr WE_MissP(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string opt;

      if (arguments.size() == 1) {
        maskName = RestOfEvent::c_defaultMaskName;
        opt = arguments[0];
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        opt = arguments[1];
      } else
        B2FATAL("Wrong number of arguments (2 required) for meta function WE_MissP");

      auto func = [maskName, opt](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        return missing4Vector(particle, maskName, opt).P();
      };
      return func;
    }

    Manager::FunctionPtr WE_MissPx(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string opt;

      if (arguments.size() == 1) {
        maskName = RestOfEvent::c_defaultMaskName;
        opt = arguments[0];
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        opt = arguments[1];
      } else
        B2FATAL("Wrong number of arguments (2 required) for meta function WE_MissPx");

      auto func = [maskName, opt](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        return missing4Vector(particle, maskName, opt).Px();
      };
      return func;
    }

    Manager::FunctionPtr WE_MissPy(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string opt;

      if (arguments.size() == 1) {
        maskName = RestOfEvent::c_defaultMaskName;
        opt = arguments[0];
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        opt = arguments[1];
      } else
        B2FATAL("Wrong number of arguments (2 required) for meta function WE_MissPy");

      auto func = [maskName, opt](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        return missing4Vector(particle, maskName, opt).Py();
      };
      return func;
    }

    Manager::FunctionPtr WE_MissPz(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string opt;

      if (arguments.size() == 1) {
        maskName = RestOfEvent::c_defaultMaskName;
        opt = arguments[0];
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        opt = arguments[1];
      } else
        B2FATAL("Wrong number of arguments (2 required) for meta function WE_MissPz");

      auto func = [maskName, opt](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        return missing4Vector(particle, maskName, opt).Pz();
      };
      return func;
    }

    Manager::FunctionPtr WE_MissE(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string opt;

      if (arguments.size() == 1) {
        maskName = RestOfEvent::c_defaultMaskName;
        opt = arguments[0];
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        opt = arguments[1];
      } else
        B2FATAL("Wrong number of arguments (2 required) for meta function WE_MissE");

      auto func = [maskName, opt](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        return missing4Vector(particle, maskName, opt).energy();
      };
      return func;
    }

    Manager::FunctionPtr WE_xiZ(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function xiZ");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        double pz = 0;
        double energy = 0;

        // Get all Tracks on reconstructed side
        std::vector<const Particle*> recTrackParticles = particle->getFinalStateDaughters();

        // Loop the reconstructed side
        for (auto& recTrackParticle : recTrackParticles)
        {
          pz += recTrackParticle->getPz();
          energy += recTrackParticle->getEnergy();
        }

        // Loop the ROE side
        auto roeParticles = roe->getChargedParticles(maskName);
        for (auto* roeParticle : roeParticles)
        {
          pz += roeParticle->getPz();
          energy += roeParticle->getEnergy();
        }

        return pz / energy;
      };
      return func;
    }

    Manager::FunctionPtr WE_MissM2OverMissE(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function WE_MissM2OverMissE");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        return missing4Vector(particle, maskName, "5").M2() / (2.0 * missing4Vector(particle, maskName, "5").energy());
      };
      return func;
    }

    Manager::FunctionPtr WE_q2lnuSimple(const std::vector<std::string>& arguments)
    {
      std::string maskName(RestOfEvent::c_defaultMaskName);
      std::string option("1");

      if (arguments.size() == 1) {
        maskName = arguments[0];
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        option = arguments[1];
      } else if (arguments.size() > 2) {
        B2FATAL("Too many arguments. At most two arguments are allowed for meta function q2lnuSimple(maskname,option)");
      }

      auto func = [maskName, option](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        int n = particle->getNDaughters();

        if (n < 1)
          return Const::doubleNaN;

        // Assumes lepton is the last particle in the reco decay chain!
        PCmsLabTransform T;
        const Particle* lep = particle->getDaughter(n - 1);
        ROOT::Math::PxPyPzEVector lep4vec = T.rotateLabToCms() * lep->get4Vector();
        ROOT::Math::PxPyPzEVector nu4vec = missing4Vector(particle, maskName, option);

        return (lep4vec + nu4vec).M2();
      };
      return func;
    }

    Manager::FunctionPtr WE_q2lnu(const std::vector<std::string>& arguments)
    {
      std::string maskName(RestOfEvent::c_defaultMaskName);
      std::string option("7");

      if (arguments.size() == 1) {
        maskName = arguments[0];
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        option = arguments[1];
      } else if (arguments.size() > 2) {
        B2FATAL("Too many arguments. At most two arguments are allowed for meta function q2lnu(maskname, option)");
      }

      auto func = [maskName, option](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return Const::doubleNaN;
        }

        int n = particle->getNDaughters();

        if (n < 1)
          return Const::doubleNaN;

        PCmsLabTransform T;
        const Particle* lep = particle->getDaughter(n - 1);
        ROOT::Math::PxPyPzEVector lep_cm = T.rotateLabToCms() * lep->get4Vector();

        ROOT::Math::PxPyPzEVector Y_cm = T.rotateLabToCms() * particle->get4Vector();
        ROOT::Math::PxPyPzEVector neu_cm = missing4Vector(particle, maskName, option);

        double e_beam = T.getCMSEnergy() / 2.0;

        //just to make the formula simpler
        double bmass = particle->getPDGMass();
        double pB2 = e_beam * e_beam - bmass * bmass;

        //angle between the Y and the neutrino, from the Mbc=M_B constraint
        double cos_angle_nu = (pB2 - Y_cm.P2() - neu_cm.P2()) / (2.0 * Y_cm.P() * neu_cm.P());
        if (abs(cos_angle_nu) > 1)
        {
          return (lep_cm + neu_cm).M2();
        }

        double angle_nu = TMath::ACos(cos_angle_nu);
        //first get one random neutrino, on the allowed cone for the constraint
        ROOT::Math::PtEtaPhiEVector rotated_neu(-Y_cm); //first get reverse Y
        rotated_neu.SetE(Y_cm.E());

        double nu_eta = -log(tan((rotated_neu.Theta() + (TMath::Pi() - angle_nu)) / 2.));
        rotated_neu.SetEta(nu_eta);
        rotated_neu.SetPt(neu_cm.pt());
        rotated_neu.SetE(neu_cm.E());

        B2Vector3D Yneu_norm = Y_cm.Vect().Cross(neu_cm.Vect());
        B2Vector3D Yrot_norm = Y_cm.Vect().Cross(rotated_neu.Vect());
        //angle between the two crossproducts -> angle between the two vectors perpendicular to the Y-p_inc and Y-B planes -> angle between the planes
        //this angle needs to come out as zero

        double rot_angle = Yneu_norm.Angle(Yrot_norm);

        ROOT::Math::PxPyPzEVector rotated_neu2(rotated_neu);
        //unfortunately don't -and probably can't- know in which direction to rotate without trying
        //so create a copy of the vector, and later choose the correct one
        //However, rotation by 180 degrees is never needed, direction of the cross-product vector assures that when after rotation
        //the B-vector is in the plane, it always is on the side closer to pcm_lv_inc.
        //rotate around Y into the Y-neutrino-plane
        ROOT::Math::AxisAngle rotation(Y_cm.Vect(), rot_angle);
        rotation(rotated_neu);
        rotation.Invert();
        rotation(rotated_neu2);

        double dot1 = rotated_neu.Vect().Dot(Yneu_norm);
        double dot2 = rotated_neu2.Vect().Dot(Yneu_norm);

        if (abs(dot2) < abs(dot1)) rotated_neu = rotated_neu2;

        return (lep_cm + rotated_neu).M2();
      };
      return func;
    }

    Manager::FunctionPtr WE_cosThetaEll(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function cosThetaEll");

      auto func = [maskName](const Particle * particle) -> double {

        ROOT::Math::PxPyPzEVector pNu = missing4Vector(particle, maskName, "6");

        ROOT::Math::PxPyPzEVector pLep;
        // TODO: avoid hardocoded values
        for (unsigned i = 0; i < particle->getNDaughters(); i++)
        {
          int absPDG = abs(particle->getDaughter(i)->getPDGCode());
          if (absPDG == Const::electron.getPDGCode() || absPDG == Const::muon.getPDGCode() || absPDG == 15) {
            pLep = particle->getDaughter(i)->get4Vector();
            break;
          }
        }

        ROOT::Math::PxPyPzEVector pW = pNu + pLep;
        ROOT::Math::PxPyPzEVector pB = particle->get4Vector() + pNu;

        // boost lepton and B momentum to W frame
        B2Vector3D boost2W = pW.BoostToCM();
        pLep = ROOT::Math::Boost(boost2W) * pLep;
        pB = ROOT::Math::Boost(boost2W) * pB;

        B2Vector3D lep3Vector     = pLep.Vect();
        B2Vector3D B3Vector       = pB.Vect();
        double numerator   = lep3Vector.Dot(B3Vector);
        double denominator = (lep3Vector.Mag()) * (B3Vector.Mag());

        return numerator / denominator;
      };
      return func;
    }

    Manager::FunctionPtr passesROEMask(const std::vector<std::string>& arguments)
    {
      std::string maskName = RestOfEvent::c_defaultMaskName;

      if (arguments.size() == 1)
        maskName = arguments[0];
      else if (arguments.size() > 1)
        B2FATAL("At most 1 argument (name of mask) accepted for meta function passesROEMask");

      auto func = [maskName](const Particle * particle) -> double {

        double result = 0;

        StoreObjPtr<RestOfEvent> roe("RestOfEvent");
        if (not roe.isValid())
          return Const::doubleNaN;

        if (roe->hasParticle(particle, maskName))
        {
          return 1.0;
        }

        return result;
      };
      return func;
    }

    double printROE(const Particle* particle)
    {
      const RestOfEvent* roe = getRelatedROEObject(particle);

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
      } else roe->print();
      return 0;
    }

    double hasCorrectROECombination(const Particle* particle)
    {
      unsigned nDaughters = particle->getNDaughters();
      if (nDaughters < 2) {
        B2ERROR("The particle must have at least two daughters.");
        return Const::doubleNaN;
      }

      for (unsigned i = 0; i < particle->getNDaughters(); i++) {

        // Find a daughter that is loaded from a ROE object
        auto daughter = particle->getDaughter(i);
        auto roe = daughter->getRelatedFrom<RestOfEvent>();
        if (!roe)
          continue;

        auto sourceParticle = roe->getRelatedFrom<Particle>();
        for (unsigned j = 0; j < particle->getNDaughters(); j++) {
          if (i == j) continue;
          const auto anotherDaughter = particle->getDaughter(j);

          if (anotherDaughter == sourceParticle)
            return 1.0;
        }
        return 0.0;
      }

      B2ERROR("There is no daughter particle loaded from the ROE object.");
      return Const::doubleNaN;
    }

    Manager::FunctionPtr pi0Prob(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2ERROR("Wrong number of arguments (1 required) for pi0Prob");

      std::string mode;
      mode = arguments[0];

      if (mode != "standard" and mode != "tight" and mode != "cluster" and mode != "both")
        B2ERROR("the given argument is not supported in pi0Prob!");

      auto func = [mode](const Particle * particle) -> double {
        if (mode == "standard")
        {
          if (particle->hasExtraInfo("Pi0ProbOrigin")) {
            return particle->getExtraInfo("Pi0ProbOrigin");
          } else {
            B2WARNING("Pi0ProbOrigin is not registerted in extraInfo! \n"
                      "the function writePi0EtaVeto has to be executed to register this extraInfo.");
            return Const::doubleNaN;
          }
        } else if (mode == "tight")
        {
          if (particle->hasExtraInfo("Pi0ProbTightEnergyThreshold")) {
            return particle->getExtraInfo("Pi0ProbTightEnergyThreshold");
          } else {
            B2WARNING("Pi0ProbTightEnergyThreshold is not registerted in extraInfo! \n"
                      "the function writePi0EtaVeto has to be executed to register this extraInfo.");
            return Const::doubleNaN;
          }
        } else if (mode == "cluster")
        {
          if (particle->hasExtraInfo("Pi0ProbLargeClusterSize")) {
            return particle->getExtraInfo("Pi0ProbLargeClusterSize");
          } else {
            B2WARNING("Pi0ProbLargeClusterSize is not registerted in extraInfo! \n"
                      "the function writePi0EtaVeto has to be executed to register this extraInfo.");
            return Const::doubleNaN;
          }
        } else if (mode == "both")
        {
          if (particle->hasExtraInfo("Pi0ProbTightEnergyThresholdAndLargeClusterSize")) {
            return particle->getExtraInfo("Pi0ProbTightEnergyThresholdAndLargeClusterSize");
          } else {
            B2WARNING("Pi0ProbTightEnergyThresholdAndLargeClusterSize is not registerted in extraInfo! \n"
                      "the function writePi0EtaVeto has to be executed to register this extraInfo.");
            return Const::doubleNaN;
          }
        } else
        {
          return Const::doubleNaN;
        }
      };
      return func;
    }

    Manager::FunctionPtr etaProb(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 1)
        B2ERROR("Wrong number of arguments (1 required) for etaProb");

      std::string mode;
      mode = arguments[0];

      if (mode != "standard" and mode != "tight" and mode != "cluster" and mode != "both")
        B2ERROR("the given argument is not supported in etaProb!");

      auto func = [mode](const Particle * particle) -> double {
        if (mode == "standard")
        {
          if (particle->hasExtraInfo("EtaProbOrigin")) {
            return particle->getExtraInfo("EtaProbOrigin");
          } else {
            B2WARNING("EtaProbOrigin is not registerted in extraInfo! \n"
                      "the function writePi0EtaVeto has to be executed to register this extraInfo.");
            return Const::doubleNaN;
          }
        } else if (mode == "tight")
        {
          if (particle->hasExtraInfo("EtaProbTightEnergyThreshold")) {
            return particle->getExtraInfo("EtaProbTightEnergyThreshold");
          } else {
            B2WARNING("EtaProbTightEnergyThreshold is not registerted in extraInfo! \n"
                      "the function writePi0EtaVeto has to be executed to register this extraInfo.");
            return Const::doubleNaN;
          }
        } else if (mode == "cluster")
        {
          if (particle->hasExtraInfo("EtaProbLargeClusterSize")) {
            return particle->getExtraInfo("EtaProbLargeClusterSize");
          } else {
            B2WARNING("EtaProbLargeClusterSize is not registerted in extraInfo! \n"
                      "the function writePi0EtaVeto has to be executed to register this extraInfo.");
            return Const::doubleNaN;
          }
        } else if (mode == "both")
        {
          if (particle->hasExtraInfo("EtaProbTightEnergyThresholdAndLargeClusterSize")) {
            return particle->getExtraInfo("EtaProbTightEnergyThresholdAndLargeClusterSize");
          } else {
            B2WARNING("EtaProbTightEnergyThresholdAndLargeClusterSize is not registerted in extraInfo! \n"
                      "the function writePi0EtaVeto has to be executed to register this extraInfo.");
            return Const::doubleNaN;
          }
        } else
        {
          return Const::doubleNaN;
        }
      };
      return func;
    }

    // ------------------------------------------------------------------------------
    // Below are some functions for ease of usage, they are not a part of variables
    // ------------------------------------------------------------------------------

    ROOT::Math::PxPyPzEVector missing4Vector(const Particle* particle, const std::string& maskName, const std::string& opt)
    {
      // Get related ROE object
      const RestOfEvent* roe = getRelatedROEObject(particle);

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        ROOT::Math::PxPyPzEVector empty;
        return empty;
      }

      PCmsLabTransform T;
      ROOT::Math::PxPyPzEVector boostvec = T.getBeamFourMomentum();

      ROOT::Math::PxPyPzEVector rec4vecLAB = particle->get4Vector();
      ROOT::Math::PxPyPzEVector roe4vecLAB = roe->get4Vector(maskName);

      ROOT::Math::PxPyPzEVector rec4vec = T.rotateLabToCms() * rec4vecLAB;
      ROOT::Math::PxPyPzEVector roe4vec = T.rotateLabToCms() * roe4vecLAB;

      ROOT::Math::PxPyPzEVector miss4vec;
      double E_beam_cms = T.getCMSEnergy() / 2.0;

      // Definition 0: CMS, use energy and momentum of tracks and clusters
      if (opt == "0") {
        miss4vec = - (rec4vec + roe4vec);
        miss4vec.SetE(2 * E_beam_cms - (rec4vec.E() + roe4vec.E()));
      }

      // Definition 1: CMS, same as 0, fix Emiss = pmiss
      else if (opt == "1") {
        miss4vec = - (rec4vec + roe4vec);
        miss4vec.SetE(miss4vec.P());
      }

      // Definition 2: CMS, same as 0, fix Eroe = Ecms/2
      else if (opt == "2") {
        miss4vec = - (rec4vec + roe4vec);
        miss4vec.SetE(E_beam_cms - rec4vec.E());
      }

      // Definition 3: CMS, use only energy and momentum of signal side
      else if (opt == "3") {
        miss4vec = - rec4vec;
        miss4vec.SetE(E_beam_cms - rec4vec.E());
      }

      // Definition 4: CMS, same as 3, update with direction of ROE momentum
      else if (opt == "4") {
        B2Vector3D pB = - roe4vec.Vect();
        pB.SetMag(0.340);
        pB -= rec4vec.Vect();
        miss4vec.SetPxPyPzE(pB.X(), pB.Y(), pB.Z(), E_beam_cms - rec4vec.E());
      }

      // Definition 5: LAB, use energy and momentum of tracks and clusters from whole event
      else if (opt == "5") {
        miss4vec = boostvec - (rec4vecLAB + roe4vecLAB);
      }

      // Definition 6: LAB, same as 5, fix Emiss = pmiss
      else if (opt == "6") {
        miss4vec = boostvec - (rec4vecLAB + roe4vecLAB);
        miss4vec.SetE(miss4vec.P());
      }

      // Definition 7: CMS, correct pmiss 3-momentum vector with factor alpha so that dE = 0 (used for Mbc calculation)
      else if (opt == "7") {
        miss4vec = - (rec4vec + roe4vec);
        miss4vec.SetE(miss4vec.P());
        double factorAlpha = (E_beam_cms - rec4vec.E()) / miss4vec.E();
        miss4vec *= factorAlpha;
        miss4vec.SetE(miss4vec.P());
      }

      return miss4vec;
    }

    void checkMCParticleMissingFlags(const MCParticle* mcp, std::set<const MCParticle*> mcROEObjects, int& missingFlags)
    {
      std::vector<MCParticle*> daughters = mcp->getDaughters();
      for (auto& daughter : daughters) {

        if (!daughter->hasStatus(MCParticle::c_PrimaryParticle))
          continue;

        if (mcROEObjects.find(daughter) == mcROEObjects.end()) {

          int pdg = abs(daughter->getPDG());

          // photon
          if (pdg == Const::photon.getPDGCode() and (missingFlags & 1) == 0)
            missingFlags += 1;

          // electrons
          else if (pdg == Const::electron.getPDGCode() and (missingFlags & 2) == 0)
            missingFlags += 2;

          // muons
          else if (pdg == Const::muon.getPDGCode() and (missingFlags & 4) == 0)
            missingFlags += 4;

          // pions
          else if (pdg == Const::pion.getPDGCode() and (missingFlags & 8) == 0)
            missingFlags += 8;

          // kaons
          else if (pdg == Const::kaon.getPDGCode() and (missingFlags & 16) == 0)
            missingFlags += 16;

          // protons
          else if (pdg == Const::proton.getPDGCode() and (missingFlags & 32) == 0)
            missingFlags += 32;

          // neutrons
          else if (pdg == Const::neutron.getPDGCode() and (missingFlags & 64) == 0)
            missingFlags += 64;

          // kshort
          else if (pdg == Const::Kshort.getPDGCode() and ((missingFlags & 128) == 0 or (missingFlags & 256) == 0)) {
            std::vector<MCParticle*> ksDaug = daughter->getDaughters();
            if (ksDaug.size() == 2) {
              // K_S0 -> pi+ pi-
              if (abs(ksDaug[0]->getPDG()) == Const::pion.getPDGCode() and abs(ksDaug[1]->getPDG()) == Const::pion.getPDGCode()
                  and (missingFlags & 128) == 0) {
                if (mcROEObjects.find(ksDaug[0]) == mcROEObjects.end() or mcROEObjects.find(ksDaug[1]) == mcROEObjects.end())
                  missingFlags += 128;
              }
              // K_S0 -> pi0 pi0
              else if (abs(ksDaug[0]->getPDG()) == Const::pi0.getPDGCode() and abs(ksDaug[1]->getPDG()) == Const::pi0.getPDGCode()
                       and (missingFlags & 256) == 0) {
                std::vector<MCParticle*> pi0Daug0 = ksDaug[0]->getDaughters();
                std::vector<MCParticle*> pi0Daug1 = ksDaug[1]->getDaughters();
                if (mcROEObjects.find(pi0Daug0[0]) == mcROEObjects.end() or
                    mcROEObjects.find(pi0Daug0[1]) == mcROEObjects.end() or
                    mcROEObjects.find(pi0Daug1[0]) == mcROEObjects.end() or
                    mcROEObjects.find(pi0Daug1[1]) == mcROEObjects.end())
                  missingFlags += 256;
              }
            }
          }

          // klong
          else if (pdg == Const::Klong.getPDGCode() and (missingFlags & 512) == 0)
            missingFlags += 512;

          // neutrinos, which are not in the Const::
          else if ((pdg == 12 or pdg == 14 or pdg == 16) and (missingFlags & 1024) == 0)
            missingFlags += 1024;
        }
        checkMCParticleMissingFlags(daughter, mcROEObjects, missingFlags);
      }
    }

    double isInThisRestOfEvent(const Particle* particle, const RestOfEvent* roe, const std::string& maskName)
    {
      if (particle->getParticleSource() == Particle::c_Composite or
          particle->getParticleSource() == Particle::c_V0) {
        std::vector<const Particle*> fspDaug = particle->getFinalStateDaughters();
        for (auto& i : fspDaug) {
          if (isInThisRestOfEvent(i, roe, maskName) == 0)
            return 0;
        }
        return 1;
      }
      return roe->hasParticle(particle, maskName);
    }

    const RestOfEvent* getRelatedROEObject(const Particle* particle, bool returnHostOnly)
    {
      // Get related ROE object
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();
      if (!roe && !returnHostOnly) {
        roe = particle->getRelatedTo<RestOfEvent>("NestedRestOfEvents");

      }
      return roe;
    }

    VARIABLE_GROUP("Rest Of Event");

    REGISTER_METAVARIABLE("useROERecoilFrame(variable)", useROERecoilFrame,
                          "Returns the value of the variable using the rest frame of the ROE recoil as current reference frame.\n"
                          "Can be used inside for_each loop or outside of it if the particle has associated Rest of Event.\n"
                          "E.g. ``useROERecoilFrame(E)`` returns the energy of a particle in the ROE recoil frame.", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("isInRestOfEvent", isInRestOfEvent,
                      "Returns 1 if a track, ecl or klmCluster associated to particle is in the current RestOfEvent object, 0 otherwise."
                      "One can use this variable only in a for_each loop over the RestOfEvent StoreArray.");

    REGISTER_VARIABLE("isCloneOfSignalSide", isCloneOfSignalSide,
                      "Returns 1 if a particle is a clone of signal side final state particles, 0 otherwise. "
                      "Requires generator information and truth-matching. "
                      "One can use this variable only in a ``for_each`` loop over the RestOfEvent StoreArray.");

    REGISTER_VARIABLE("hasAncestorFromSignalSide", hasAncestorFromSignalSide,
                      "Returns 1 if a particle has ancestor from signal side, 0 otherwise. "
                      "Requires generator information and truth-matching. "
                      "One can use this variable only in a ``for_each`` loop over the RestOfEvent StoreArray.");

    REGISTER_METAVARIABLE("currentROEIsInList(particleList)", currentROEIsInList,
                          "[Eventbased] Returns 1 the associated particle of the current ROE is contained in the given list or its charge-conjugated."
                          "Useful to restrict the for_each loop over ROEs to ROEs of a certain ParticleList.", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("nROE_RemainingTracks", nROE_RemainingTracks,
                      "Returns number of tracks in ROE - number of tracks of given particle"
                      "One can use this variable only in a for_each loop over the RestOfEvent StoreArray.");

    REGISTER_METAVARIABLE("nROE_RemainingTracks(maskName)", nROE_RemainingTracksWithMask,
                          "Returns number of remaining tracks between the ROE (specified via a mask) and the given particle. For the given particle only tracks are counted which are in the RoE."
                          "One can use this variable only in a for_each loop over the RestOfEvent StoreArray."
                          "Is required for the specific FEI. :noindex:", Manager::VariableDataType::c_int);
    // nROE_RemainingTracks is overloaded (two C++ functions sharing one
    // variable name) so one of the two needs to be made the indexed
    // variable in sphinx

    REGISTER_VARIABLE("nROE_KLMClusters", nROE_KLMClusters,
                      "Returns number of all remaining KLM clusters in the related RestOfEvent object.");

    REGISTER_METAVARIABLE("nROE_Charged(maskName, PDGcode = 0)", nROE_ChargedParticles,
                          "Returns number of all charged particles in the related RestOfEvent object. First optional argument is ROE mask name. "
                          "Second argument is a PDG code to count only one charged particle species, independently of charge. "
                          "For example: ``nROE_Charged(cleanMask, 321)`` will output number of kaons in Rest Of Event with ``cleanMask``. "
                          "PDG code 0 is used to count all charged particles", Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("nROE_Photons(maskName)", nROE_Photons,
                          "Returns number of all photons in the related RestOfEvent object, accepts 1 optional argument of ROE mask name. ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("nROE_NeutralHadrons(maskName)", nROE_NeutralHadrons,
                          "Returns number of all neutral hadrons in the related RestOfEvent object, accepts 1 optional argument of ROE mask name. ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("particleRelatedToCurrentROE(var)", particleRelatedToCurrentROE,
                          "[Eventbased] Returns variable applied to the particle which is related to the current RestOfEvent object"
                          "One can use this variable only in a for_each loop over the RestOfEvent StoreArray.", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("roeMC_E", ROE_MC_E,
                      "Returns true energy of unused tracks and clusters in ROE, can be used with ``use***Frame()`` function.\n\n", "GeV");

    REGISTER_VARIABLE("roeMC_M", ROE_MC_M,
                      "Returns true invariant mass of unused tracks and clusters in ROE\n\n", "GeV/:math:`\\text{c}^2`");

    REGISTER_VARIABLE("roeMC_P", ROE_MC_P,
                      "Returns true momentum of unused tracks and clusters in ROE, can be used with ``use***Frame()`` function.\n\n", "GeV/c");

    REGISTER_VARIABLE("roeMC_Px", ROE_MC_Px,
                      "Returns x component of true momentum of unused tracks and clusters in ROE, can be used with ``use***Frame()`` function.\n\n",
                      "GeV/c");

    REGISTER_VARIABLE("roeMC_Py", ROE_MC_Py,
                      "Returns y component of true momentum of unused tracks and clusters in ROE, can be used with ``use***Frame()`` function.\n\n",
                      "GeV/c");

    REGISTER_VARIABLE("roeMC_Pz", ROE_MC_Pz,
                      "Returns z component of true momentum of unused tracks and clusters in ROE, can be used with ``use***Frame()`` function.\n\n",
                      "GeV/c");

    REGISTER_VARIABLE("roeMC_Pt", ROE_MC_Pt,
                      "Returns transverse component of true momentum of unused tracks and clusters in ROE, can be used with ``use***Frame()`` function.\n\n",
                      "GeV/c");

    REGISTER_VARIABLE("roeMC_PTheta", ROE_MC_PTheta,
                      "Returns polar angle of true momentum of unused tracks and clusters in ROE, can be used with ``use***Frame()`` function.\n\n",
                      "rad");

    REGISTER_METAVARIABLE("roeMC_MissFlags(maskName)", ROE_MC_MissingFlags,
                          "Returns flags corresponding to missing particles on ROE side.", Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("nROE_Tracks(maskName)",  nROE_Tracks,
                          "Returns number of tracks in the related RestOfEvent object that pass the selection criteria.",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("nROE_ECLClusters(maskName)", nROE_ECLClusters,
                          "Returns number of ECL clusters in the related RestOfEvent object that pass the selection criteria.",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("nROE_NeutralECLClusters(maskName)", nROE_NeutralECLClusters,
                          "Returns number of neutral ECL clusters in the related RestOfEvent object that pass the selection criteria.",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("nROE_Composites(maskName)", nROE_Composites,
                          "Returns number of composite particles or V0s in the related RestOfEvent object that pass the selection criteria.",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("nROE_ParticlesInList(pListName[, maskName])", nROE_ParticlesInList,
                          "Returns the number of particles in ROE from the given particle list. If a mask name is provided the selection criteria are applied.\n"
                          "Use of variable aliases is advised.", Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("roeCharge(maskName)", ROE_Charge,
                          "Returns total charge of the related RestOfEvent object. The unit of the charge is ``e`` ", Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("roeEextra(maskName)", ROE_ExtraEnergy,
                          "Returns extra energy from ECLClusters in the calorimeter that is not associated to the given Particle. The unit of the energy is ``GeV`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("roeNeextra(maskName)", ROE_NeutralExtraEnergy,
                          "Returns extra energy from neutral ECLClusters in the calorimeter that is not associated to the given Particle, can be used with ``use***Frame()`` function. The unit of the energy is ``GeV`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("roeE(maskName)", ROE_E,
                          "Returns energy of unused tracks and clusters in ROE, can be used with ``use***Frame()`` function. The unit of the energy is ``GeV`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("roeM(maskName)", ROE_M,
                          "Returns invariant mass of unused tracks and clusters in ROE. The unit of the invariant mass is :math:`\\text{GeV/c}^2`",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("roeP(maskName)", ROE_P,
                          "Returns momentum of unused tracks and clusters in ROE, can be used with ``use***Frame()`` function. The unit of the momentum is ``GeV/c`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("roePt(maskName)", ROE_Pt,
                          "Returns transverse component of momentum of unused tracks and clusters in ROE, can be used with ``use***Frame()`` function. The unit of the momentum is ``GeV/c`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("roePx(maskName)", ROE_Px,
                          "Returns x component of momentum of unused tracks and clusters in ROE, can be used with ``use***Frame()`` function. The unit of the momentum is ``GeV/c`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("roePy(maskName)", ROE_Py,
                          "Returns y component of momentum of unused tracks and clusters in ROE, can be used with ``use***Frame()`` function. The unit of the momentum is ``GeV/c`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("roePz(maskName)", ROE_Pz,
                          "Returns z component of momentum of unused tracks and clusters in ROE, can be used with ``use***Frame()`` function. The unit of the momentum is ``GeV/c`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("roePTheta(maskName)", ROE_PTheta,
                          "Returns theta angle of momentum of unused tracks and clusters in ROE, can be used with ``use***Frame()`` function. The unit of the angle is ``rad`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("roeDeltae(maskName)", ROE_DeltaE,
                          "Returns energy difference of the related RestOfEvent object with respect to :math:`E_\\mathrm{cms}/2`. The unit of the energy is ``GeV`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("roeMbc(maskName)", ROE_Mbc,
                          "Returns beam constrained mass of the related RestOfEvent object with respect to :math:`E_\\mathrm{cms}/2`. The unit of the beam constrained mass is :math:`\\text{GeV/c}^2`.",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("weDeltae(maskName, opt)", WE_DeltaE,
                          "Returns the energy difference of the B meson, corrected with the missing neutrino momentum (reconstructed side + neutrino) with respect to :math:`E_\\mathrm{cms}/2`. The unit of the energy is ``GeV`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("weMbc(maskName, opt)", WE_Mbc,
                          "Returns beam constrained mass of B meson, corrected with the missing neutrino momentum (reconstructed side + neutrino) with respect to :math:`E_\\mathrm{cms}/2`. The unit of the beam constrained mass is :math:`\\text{GeV/c}^2`.",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("weMissM2(maskName, opt)", WE_MissM2,
                          "Returns the invariant mass squared of the missing momentum (see :b2:var:`weMissE` possible options). The unit of the invariant mass squared is :math:`[\\text{GeV}/\\text{c}^2]^2`.",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("weMissPTheta(maskName, opt)", WE_MissPTheta,
                          "Returns the polar angle of the missing momentum (see possible :b2:var:`weMissE` options). The unit of the polar angle is ``rad`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("weMissP(maskName, opt)", WE_MissP,
                          "Returns the magnitude of the missing momentum (see possible :b2:var:`weMissE` options). The unit of the magnitude of missing momentum is ``GeV/c`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("weMissPx(maskName, opt)", WE_MissPx,
                          "Returns the x component of the missing momentum (see :b2:var:`weMissE` possible options). The unit of the missing momentum is ``GeV/c`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("weMissPy(maskName, opt)", WE_MissPy,
                          "Returns the y component of the missing momentum (see :b2:var:`weMissE` possible options). The unit of the missing momentum is ``GeV/c`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("weMissPz(maskName, opt)", WE_MissPz,
                          "Returns the z component of the missing momentum (see :b2:var:`weMissE` possible options). The unit of the missing momentum is ``GeV/c`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("weMissE(maskName, opt)", WE_MissE,
                          R"DOC(Returns the energy of the missing momentum. The unit of the Energy is ``GeV`` . Possible options ``opt`` are the following:

- ``0``: CMS, use energy and momentum of charged particles and photons
- ``1``: CMS, same as ``0``, fix :math:`E_\mathrm{miss} = p_\mathrm{miss}`
- ``2``: CMS, same as ``0``, fix :math:`E_\mathrm{roe} = E_\mathrm{cms}/2`
- ``3``: CMS, use only energy and momentum of signal side
- ``4``: CMS, same as ``3``, update with direction of ROE momentum
- ``5``: LAB, use energy and momentum of charged particles and photons from whole event
- ``6``: LAB, same as ``5``, fix :math:`E_\mathrm{miss} = p_\mathrm{miss}``
- ``7``: CMS, correct pmiss 3-momentum vector with factor alpha so that :math:`d_E = 0`` (used for :math:`M_\mathrm{bc}` calculation).)DOC",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("weXiZ(maskName)", WE_xiZ,
                          "Returns Xi_z in event (for Bhabha suppression and two-photon scattering). The unit of this variable is ``1/c`` ",
                          Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("bssMassDifference(maskName)", bssMassDifference,
                          "Bs* - Bs mass difference. The unit of the mass is :math:`\\text{GeV/c}^2`.", Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("weCosThetaEll(maskName)", WE_cosThetaEll, R"DOC(

Returns the cosine of the angle between :math:`M` and lepton in :math:`W` rest frame in the decays of the type:
:math:`M \to h_1 ... h_n \ell`, where W 4-momentum is given as

.. math::
    p_W = p_\ell + p_\nu.

The neutrino momentum is calculated from ROE taking into account the specified mask, and setting

.. math::
    E_{\nu} = |p_{miss}|.
    
)DOC", Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("weQ2lnuSimple(maskName,option)", WE_q2lnuSimple,
                      "Returns the momentum transfer squared, :math:`q^2`, calculated in CMS as :math:`q^2 = (p_l + p_\\nu)^2`, \n"
                      "where :math:`B \\to H_1\\dots H_n \\ell \\nu_\\ell`. Lepton is assumed to be the last reconstructed daughter. \n"
                      "By default, option is set to ``1`` (see :b2:var:`weMissE`). Unless you know what you are doing, keep this default value. The unit of the momentum transfer squared is :math:`[\\text{GeV}/\\text{c}]^2`.", Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("weQ2lnu(maskName,option)", WE_q2lnu,
                      "Returns the momentum transfer squared, :math:`q^2`, calculated in CMS as :math:`q^2 = (p_l + p_\\nu)^2`, \n"
                      "where :math:`B \\to H_1\\dots H_n \\ell \\nu_\\ell`. Lepton is assumed to be the last reconstructed daughter. \n"
                      "This calculation uses constraints from dE = 0 and Mbc = Mb to correct the neutrino direction. \n"
                      "By default, option is set to ``7`` (see :b2:var:`weMissE`). Unless you know what you are doing, keep this default value. The unit of the momentum transfer squared is :math:`[\\text{GeV}/\\text{c}]^2`.", Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("weMissM2OverMissE(maskName)", WE_MissM2OverMissE,
                      "Returns missing mass squared over missing energy. The unit of the missing mass squared is :math:`\\text{GeV/c}^4`.", Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("passesROEMask(maskName)", passesROEMask,
                      "Returns boolean value if a particle passes a certain mask or not. Only to be used in for_each path, otherwise returns quiet NaN.", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("printROE", printROE,
                      "For debugging, prints indices of all particles in the ROE and all masks. Returns 0.");

    REGISTER_VARIABLE("hasCorrectROECombination", hasCorrectROECombination,
		      "Returns 1 if there is correct combination of daughter particles between the particle that is the basis of the ROE and the particle loaded from the ROE. "
		      "Returns 0 if there is not correct combination. "
		      "If there is no daughter particle loaded from the ROE, returns quiet NaN.");

    REGISTER_METAVARIABLE("pi0Prob(mode)", pi0Prob,
                      "Returns pi0 probability, where mode is used to specify the selection criteria for soft photon. \n"
                      "The following strings are available. \n\n"
                      "- ``standard``: loose energy cut and no clusterNHits cut are applied to soft photon \n"
                      "- ``tight``: tight energy cut and no clusterNHits cut are applied to soft photon \n"
                      "- ``cluster``: loose energy cut and clusterNHits cut are applied to soft photon \n"
                      "- ``both``: tight energy cut and clusterNHits cut are applied to soft photon \n\n"
                      "You can find more details in `writePi0EtaVeto` function in modularAnalysis.py.", Manager::VariableDataType::c_double);

    REGISTER_METAVARIABLE("etaProb(mode)", etaProb,
                      "Returns eta probability, where mode is used to specify the selection criteria for soft photon. \n"
                      "The following strings are available. \n\n"
                      "- ``standard``: loose energy cut and no clusterNHits cut are applied to soft photon \n"
                      "- ``tight``: tight energy cut and no clusterNHits cut are applied to soft photon \n"
                      "- ``cluster``: loose energy cut and clusterNHits cut are applied to soft photon \n"
                      "- ``both``: tight energy cut and clusterNHits cut are applied to soft photon \n\n"
                      "You can find more details in `writePi0EtaVeto` function in modularAnalysis.py.", Manager::VariableDataType::c_double);

  }
}
