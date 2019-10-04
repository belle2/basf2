/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Matic Lubej                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/variables/ROEVariables.h>
#include <analysis/variables/Variables.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <mdst/dataobjects/ECLCluster.h>

// framework aux
#include <framework/logging/Logger.h>

// utility
#include <analysis/utility/ReferenceFrame.h>

#include <TRandom.h>
#include <TMath.h>

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
        return -999.;
      }
      auto* particleMC = particle->getRelatedTo<MCParticle>();
      if (!particleMC) {
        return 0.0;
      }
      auto* signal = roe->getRelatedFrom<Particle>();
      auto signalFSPs = signal->getFinalStateDaughters();
      for (auto* daughter : signalFSPs) {
        auto* daughterMC = daughter->getRelatedTo<MCParticle>();
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
        return -999.;
      }
      auto* particleMC = particle->getRelatedTo<MCParticle>();
      if (!particleMC) {
        return 0.0;
      }
      auto* signalReco = roe->getRelatedFrom<Particle>();
      auto* signalMC = signalReco->getRelatedTo<MCParticle>();
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
          return -999.;
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
          return -999;

        auto* particle = roe->getRelatedFrom<Particle>();
        if (particle == nullptr)
        {
          B2ERROR("Relation between particle and ROE doesn't exist! particleRelatedToCurrentROE() variable has to be called from ROE loop");
          return -999.;
        }
        return var->function(particle);

      };
      return func;
    }

    // only the helper function
    double nRemainingTracksInROE(const Particle* particle, const std::string& maskName)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (not roe.isValid())
        return 0.0;
      int n_roe_tracks = roe->getNTracks(maskName);
      int n_par_tracks = 0;
      const auto& daughters = particle->getFinalStateDaughters();
      for (const auto& daughter : daughters) {
        if (daughter->getParticleType() == Particle::EParticleType::c_Track && roe->hasParticle(daughter, maskName)) {
          n_par_tracks++;
        }
      }
      return n_roe_tracks - n_par_tracks;
    }

    Manager::FunctionPtr nROE_RemainingTracksWithMask(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function nROETracks");

      auto func = [maskName](const Particle * particle) -> double {
        return nRemainingTracksInROE(particle, maskName);
      };
      return func;
    }

    double nROE_RemainingTracks(const Particle* particle)
    {
      return nRemainingTracksInROE(particle);
    }

    double nROE_KLMClusters(const Particle* particle)
    {
      // Get related ROE object
      const RestOfEvent* roe = getRelatedROEObject(particle);

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        return -1;
      }

      return roe->getNKLMClusters();
    }

    double ROE_MC_E(const Particle* particle)
    {
      const MCParticle* mcp = particle->getRelated<MCParticle>();

      if (!mcp)
        return -999;

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBeamFourMomentum();
      auto mcroe4vector = boostvec - mcp->get4Vector();
      const auto& frame = ReferenceFrame::GetCurrent();
      auto frameMCRoe4Vector = frame.getMomentum(mcroe4vector);
      return frameMCRoe4Vector.Energy();
    }

    double ROE_MC_P(const Particle* particle)
    {
      const MCParticle* mcp = particle->getRelated<MCParticle>();

      if (!mcp)
        return -999;

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBeamFourMomentum();
      auto mcroe4vector = boostvec - mcp->get4Vector();
      const auto& frame = ReferenceFrame::GetCurrent();
      auto frameMCRoe4Vector = frame.getMomentum(mcroe4vector);
      return frameMCRoe4Vector.Vect().Mag();
    }

    double ROE_MC_Px(const Particle* particle)
    {
      const MCParticle* mcp = particle->getRelated<MCParticle>();

      if (!mcp)
        return -999;

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBeamFourMomentum();
      auto mcroe4vector = boostvec - mcp->get4Vector();
      const auto& frame = ReferenceFrame::GetCurrent();
      auto frameMCRoe4Vector = frame.getMomentum(mcroe4vector);

      return frameMCRoe4Vector.Vect().X();
    }

    double ROE_MC_Py(const Particle* particle)
    {
      const MCParticle* mcp = particle->getRelated<MCParticle>();

      if (!mcp)
        return -999;

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBeamFourMomentum();
      auto mcroe4vector = boostvec - mcp->get4Vector();
      const auto& frame = ReferenceFrame::GetCurrent();
      auto frameMCRoe4Vector = frame.getMomentum(mcroe4vector);

      return frameMCRoe4Vector.Vect().Y();
    }

    double ROE_MC_Pz(const Particle* particle)
    {
      const MCParticle* mcp = particle->getRelated<MCParticle>();

      if (!mcp)
        return -999;

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBeamFourMomentum();
      auto mcroe4vector = boostvec - mcp->get4Vector();
      const auto& frame = ReferenceFrame::GetCurrent();
      auto frameMCRoe4Vector = frame.getMomentum(mcroe4vector);

      return frameMCRoe4Vector.Vect().Z();
    }

    double ROE_MC_Pt(const Particle* particle)
    {
      const MCParticle* mcp = particle->getRelated<MCParticle>();

      if (!mcp)
        return -999;

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBeamFourMomentum();
      auto mcroe4vector = boostvec - mcp->get4Vector();
      const auto& frame = ReferenceFrame::GetCurrent();
      auto frameMCRoe4Vector = frame.getMomentum(mcroe4vector);

      return frameMCRoe4Vector.Vect().Perp();
    }

    double ROE_MC_PTheta(const Particle* particle)
    {
      const MCParticle* mcp = particle->getRelated<MCParticle>();

      if (!mcp)
        return -999;

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBeamFourMomentum();
      auto mcroe4vector = boostvec - mcp->get4Vector();
      const auto& frame = ReferenceFrame::GetCurrent();
      auto frameMCRoe4Vector = frame.getMomentum(mcroe4vector);

      return frameMCRoe4Vector.Theta();
    }

    double ROE_MC_M(const Particle* particle)
    {
      const MCParticle* mcp = particle->getRelated<MCParticle>();

      if (!mcp)
        return -999;

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBeamFourMomentum();

      return (boostvec - mcp->get4Vector()).Mag();
    }

    Manager::FunctionPtr ROE_MC_MissingFlags(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function nROETracks");

      auto func = [maskName](const Particle * particle) -> double {

        StoreArray<Particle> particles;

        //Get MC Particle of the B meson
        const MCParticle* mcParticle = particle->getRelatedTo<MCParticle>();

        if (!mcParticle)
          return -999;

        // Get Mother
        const MCParticle* mcMother = mcParticle->getMother();

        if (!mcMother)
          return -999;

        // Get daughters
        std::vector<MCParticle*> mcDaughters = mcMother->getDaughters();

        if (mcDaughters.size() != 2)
          return -999;

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
          auto* mcroeParticle = roeParticle->getRelated<MCParticle>();
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
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function nROETracks");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return roe->getNTracks(maskName);
      };
      return func;
    }

    Manager::FunctionPtr nROE_ECLClusters(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function nROEECLClusters");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return roe->getNECLClusters(maskName);
      };
      return func;
    }

    Manager::FunctionPtr nROE_NeutralECLClusters(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function nROENeutralECLClusters");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        // Get unused ECLClusters in ROE
        std::vector<const ECLCluster*> roeClusters = roe->getECLClusters(maskName);
        int nNeutrals = 0;

        // Select ECLClusters with no associated tracks
        for (auto& roeCluster : roeClusters)
          if (roeCluster->isNeutral())
            nNeutrals++;

        return nNeutrals;
      };
      return func;
    }

    Manager::FunctionPtr nROE_Photons(const std::vector<std::string>& arguments)
    {
      std::string maskName = "";

      if (arguments.size() == 1) {
        maskName = arguments[0];
      }
      if (arguments.size() > 1) {
        B2FATAL("Wrong number of arguments (1 required) for meta function nROE_Photons");
      }
      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return roe->getPhotons(maskName).size();
      };
      return func;
    }

    Manager::FunctionPtr nROE_NeutralHadrons(const std::vector<std::string>& arguments)
    {
      std::string maskName = "";

      if (arguments.size() == 1) {
        maskName = arguments[0];
      }
      if (arguments.size() > 1) {
        B2FATAL("Wrong number of arguments (1 optional only) for meta function nROE_NeutralHadrons");
      }
      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return roe->getHadrons(maskName).size();
      };
      return func;
    }

    Manager::FunctionPtr nROE_ChargedParticles(const std::vector<std::string>& arguments)
    {
      std::string maskName = "";
      int pdgCode = 0;
      if (arguments.size() == 1) {
        maskName = arguments[0];
      }
      if (arguments.size() == 2) {
        maskName = arguments[0];
        try {
          pdgCode = std::stoi(arguments[1]);
        } catch (std::invalid_argument& e) {
          B2ERROR("First argument of nROE_ChargedParticles must be a PDG code");
          return nullptr;
        }
      }
      if (arguments.size() > 2)  {
        B2FATAL("Wrong number of arguments (2 optional) for meta function nROE_ChargedParticles");
      }
      auto func = [maskName, pdgCode](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return roe->getChargedParticles(maskName, abs(pdgCode)).size();
      };
      return func;
    }

    Manager::FunctionPtr nROE_ParticlesInList(const std::vector<std::string>& arguments)
    {
      std::string pListName;
      std::string maskName;

      if (arguments.size() == 1) {
        pListName = arguments[0];
        maskName = "";
      } else if (arguments.size() == 2) {
        pListName = arguments[0];
        maskName = arguments[1];
      } else
        B2FATAL("Wrong number of arguments (1 required) for meta function nROE_ParticlesInList");

      auto func = [pListName, maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
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
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function ROECharge");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
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
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function extraEnergy");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        std::vector<const ECLCluster*> roeClusters = roe->getECLClusters(maskName);
        double extraE = 0.0;

        for (auto& roeCluster : roeClusters)
          extraE += roeCluster->getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);

        return extraE;
      };
      return func;
    }

    Manager::FunctionPtr ROE_NeutralExtraEnergy(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function extraEnergy");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }
        auto roephotons = roe->getPhotons(maskName);
        TLorentzVector total4vector;
        for (auto* photon : roephotons)
        {
          total4vector += photon->get4Vector();
        }
        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(total4vector);
        return frameRoe4Vector.Energy();
      };
      return func;
    }

    Manager::FunctionPtr ROE_E(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function ROE_E");
      auto func = [maskName](const Particle * particle) -> double {
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();
        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }
        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(roe->get4Vector(maskName));
        return frameRoe4Vector.Energy();
      };
      return func;
    }

    Manager::FunctionPtr ROE_M(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function ROE_M");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return roe->get4Vector(maskName).Mag();
      };
      return func;
    }

    Manager::FunctionPtr ROE_P(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function ROE_P");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(roe->get4Vector(maskName));
        return frameRoe4Vector.Vect().Mag();
      };
      return func;
    }

    Manager::FunctionPtr ROE_Px(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function ROE_Px");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(roe->get4Vector(maskName));
        return frameRoe4Vector.Vect().X();
      };
      return func;
    }

    Manager::FunctionPtr ROE_Py(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function ROE_Py");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(roe->get4Vector(maskName));
        return frameRoe4Vector.Vect().Y();
      };
      return func;
    }

    Manager::FunctionPtr ROE_Pt(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function ROE_Pt");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(roe->get4Vector(maskName));
        return frameRoe4Vector.Vect().Perp();
      };
      return func;
    }

    Manager::FunctionPtr ROE_Pz(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function ROE_Pz");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(roe->get4Vector(maskName));
        return frameRoe4Vector.Vect().Z();
      };
      return func;
    }

    Manager::FunctionPtr ROE_PTheta(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function ROE_PTheta");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        const auto& frame = ReferenceFrame::GetCurrent();
        auto frameRoe4Vector = frame.getMomentum(roe->get4Vector(maskName));
        return frameRoe4Vector.Theta();
      };
      return func;
    }

    Manager::FunctionPtr ROE_DeltaE(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function ROE_deltae");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        PCmsLabTransform T;
        TLorentzVector vec = T.rotateLabToCms() * roe->get4Vector(maskName);
        return vec.E() - T.getCMSEnergy() / 2;
      };
      return func;
    }

    Manager::FunctionPtr ROE_Mbc(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function ROE_mbc");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        PCmsLabTransform T;
        TLorentzVector vec = T.rotateLabToCms() * roe->get4Vector(maskName);

        double E = T.getCMSEnergy() / 2;
        double m2 = E * E - vec.Vect().Mag2();
        double mbc = m2 > 0 ? sqrt(m2) : 0;

        return mbc;
      };
      return func;
    }

    Manager::FunctionPtr bssMassDifference(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function bssMassDifference");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        TLorentzVector neutrino4vec = missing4Vector(particle->getDaughter(0), maskName, "6");
        TLorentzVector sig4vec = particle->getDaughter(0)->get4Vector();

        TLorentzVector bsMom = neutrino4vec + sig4vec;
        TLorentzVector bssMom = bsMom + particle->getDaughter(1)->get4Vector();

        return bssMom.M() - bsMom.M();
      };
      return func;
    }

    Manager::FunctionPtr WE_DeltaE(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string opt;

      if (arguments.size() == 1) {
        maskName = "";
        opt = arguments[0];
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        opt = arguments[1];
      } else
        B2FATAL("Wrong number of arguments (2 required) for meta function correctedB_deltae");

      auto func = [maskName, opt](const Particle * particle) -> double {

        PCmsLabTransform T;
        TLorentzVector boostvec = T.getBeamFourMomentum();
        TLorentzVector sig4vec = T.rotateLabToCms() * particle->get4Vector();
        TLorentzVector sig4vecLAB = particle->get4Vector();
        TLorentzVector neutrino4vec = missing4Vector(particle, maskName, "1");
        TLorentzVector neutrino4vecLAB = missing4Vector(particle, maskName, "6");

        double deltaE = -999.9;

        // Definition 0: CMS
        if (opt == "0")
        {
          double totalSigEnergy = (sig4vec + neutrino4vec).Energy();
          double E = T.getCMSEnergy() / 2;
          deltaE = totalSigEnergy - E;
        }

        // Definition 1: LAB
        else if (opt == "1")
        {
          double Ecms = T.getCMSEnergy();
          double s = Ecms * Ecms;
          deltaE = ((sig4vecLAB + neutrino4vecLAB) * boostvec - s / 2.0) / sqrt(s);
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
        maskName = "";
        opt = arguments[0];
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        opt = arguments[1];
      } else
        B2FATAL("Wrong number of arguments (2 required) for meta function correctedB_mbc");

      auto func = [maskName, opt](const Particle * particle) -> double {

        PCmsLabTransform T;
        TLorentzVector boostvec = T.getBeamFourMomentum();
        TLorentzVector sig4vec = T.rotateLabToCms() * particle->get4Vector();
        TLorentzVector sig4vecLAB = particle->get4Vector();
        TLorentzVector neutrino4vec;

        double mbc = -999.9;

        // Definition 0: CMS
        if (opt == "0")
        {
          neutrino4vec = missing4Vector(particle, maskName, "1");
          TVector3 bmom = (sig4vec + neutrino4vec).Vect();
          double E = T.getCMSEnergy() / 2;
          double m2 = E * E - bmom.Mag2();
          mbc = m2 > 0 ? sqrt(m2) : 0;
        }

        // Definition 1: LAB
        else if (opt == "1")
        {
          neutrino4vec = missing4Vector(particle, maskName, "6");
          TVector3 bmom = (sig4vecLAB + neutrino4vec).Vect();
          double Ecms = T.getCMSEnergy();
          double s = Ecms * Ecms;
          double m2 = pow((s / 2.0 + bmom * boostvec.Vect()) / boostvec.Energy(), 2.0) - bmom.Mag2();
          mbc = m2 > 0 ? sqrt(m2) : 0;
        }

        // Definition 2: CMS with factor alpha (so that dE == 0)
        else if (opt == "2")
        {
          neutrino4vec = missing4Vector(particle, maskName, "7");
          TVector3 bmom = (sig4vec + neutrino4vec).Vect();
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
        maskName = "";
        opt = arguments[0];
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        opt = arguments[1];
      } else
        B2FATAL("Wrong number of arguments (2 required) for meta function missM2");

      auto func = [maskName, opt](const Particle * particle) -> double {

        return missing4Vector(particle, maskName, opt).Mag2();
      };
      return func;
    }

    double REC_MissM2(const Particle* particle)
    {
      PCmsLabTransform T;
      TLorentzVector rec4vecLAB = particle->get4Vector();
      TLorentzVector rec4vec = T.rotateLabToCms() * rec4vecLAB;

      TLorentzVector miss4vec;
      double E_beam_cms = T.getCMSEnergy() / 2.0;

      miss4vec.SetVect(-rec4vec.Vect());
      miss4vec.SetE(E_beam_cms - rec4vec.Energy());

      return miss4vec.Mag2();
    }

    Manager::FunctionPtr WE_MissPTheta(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string opt;

      if (arguments.size() == 1) {
        maskName = "";
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
          return -1;
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
        maskName = "";
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
          return -1;
        }

        return missing4Vector(particle, maskName, opt).Vect().Mag();
      };
      return func;
    }

    Manager::FunctionPtr WE_MissPx(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string opt;

      if (arguments.size() == 1) {
        maskName = "";
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
          return -1;
        }

        return missing4Vector(particle, maskName, opt).Vect().Px();
      };
      return func;
    }

    Manager::FunctionPtr WE_MissPy(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string opt;

      if (arguments.size() == 1) {
        maskName = "";
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
          return -1;
        }

        return missing4Vector(particle, maskName, opt).Vect().Py();
      };
      return func;
    }

    Manager::FunctionPtr WE_MissPz(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string opt;

      if (arguments.size() == 1) {
        maskName = "";
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
          return -1;
        }

        return missing4Vector(particle, maskName, opt).Vect().Pz();
      };
      return func;
    }

    Manager::FunctionPtr WE_MissE(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string opt;

      if (arguments.size() == 1) {
        maskName = "";
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
          return -1;
        }

        return missing4Vector(particle, maskName, opt).Energy();
      };
      return func;
    }

    Manager::FunctionPtr WE_xiZ(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function xiZ");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
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

        // "Loop" the ROE side
        pz += roe->get4VectorTracks(maskName).Vect().Pz();
        energy += roe->get4VectorTracks(maskName).Energy();

        return pz / energy;
      };
      return func;
    }

    Manager::FunctionPtr WE_MissM2OverMissE(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function WE_MissM2OverMissE");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        PCmsLabTransform T;
        TLorentzVector missing4Momentum;
        TLorentzVector boostvec = T.getBeamFourMomentum();

        return missing4Vector(particle, maskName, "5").Mag2() / (2.0 * missing4Vector(particle, maskName, "5").Energy());
      };
      return func;
    }

    double REC_q2BhSimple(const Particle* particle)
    {
      // calculates q^2 = (p_B - p_h) in decays of B -> h_1 .. h_n ell nu_ell,
      // where p_h = Sum_i^n p_h_i is the 4-momentum of hadrons in the final
      // state. The calculation is performed in the CMS system, where B-meson
      // is assumed to be at rest p_B = (m_B, 0).

      TLorentzVector hadron4vec;

      int n = particle->getNDaughters();

      if (n < 1)
        return -999.9;

      // TODO: avoid hardocoded values
      for (unsigned i = 0; i < particle->getNDaughters(); i++) {
        int absPDG = abs(particle->getDaughter(i)->getPDGCode());
        if (absPDG == 11 || absPDG == 13 || absPDG == 15)
          continue;

        hadron4vec += particle->getDaughter(i)->get4Vector();
      }

      // boost to CMS
      PCmsLabTransform T;
      TLorentzVector phCMS = T.rotateLabToCms() * hadron4vec;
      TLorentzVector pBCMS;
      pBCMS.SetXYZM(0.0, 0.0, 0.0, particle->getPDGMass());

      return (pBCMS - phCMS).Mag2();
    }

    double REC_q2Bh(const Particle* particle)
    {
      // calculates q^2 = (p_B - p_h) in decays of B -> h_1 .. h_n ell nu_ell,
      // where p_h = Sum_i^n p_h_i is the 4-momentum of hadrons in the final
      // state. The calculation is performed in the CMS system,
      // with a weighter average in a cone around the true B direction

      TLorentzVector hadron4vec;

      int n = particle->getNDaughters();

      if (n < 1)
        return -999.9;

      for (unsigned i = 0; i < particle->getNDaughters(); i++) {
        int absPDG = abs(particle->getDaughter(i)->getPDGCode());
        if (absPDG == 11 || absPDG == 13 || absPDG == 15)
          continue;

        hadron4vec += particle->getDaughter(i)->get4Vector();
      }

      // boost to CMS
      PCmsLabTransform T;
      TLorentzVector had_cm = T.rotateLabToCms() * hadron4vec;
      TLorentzVector Y_cm = T.rotateLabToCms() * particle->get4Vector();

      // Recycled code from Uwe Gebauer <uwe.gebauer@phys.uni-goettingen.de>

      double bmass = particle->getPDGMass();

      // B theta angle
      double cos_cone_angle = Variable::cosThetaBetweenParticleAndNominalB(particle);

      if (abs(cos_cone_angle) > 1) {
        //makes no sense in this case, return simple value
        return Variable::REC_q2BhSimple(particle);
      }

      double thetaBY = TMath::ACos(cos_cone_angle);
      const double E_B = T.getCMSEnergy() / 2.0;
      const double p_B = sqrt(E_B * E_B - bmass * bmass);

      double phi_start = gRandom->Uniform(0, TMath::Pi() / 2);

      double q2 = 0;
      double denom = 0;

      for (int around_the_cone = 0; around_the_cone < 4; around_the_cone++) {
        TLorentzVector one_B(1, 1, 1, E_B);
        double B_theta = Y_cm.Theta() + thetaBY * cos(phi_start + around_the_cone / 2.*TMath::Pi());
        double B_phi = Y_cm.Phi() + thetaBY * sin(phi_start + around_the_cone / 2.*TMath::Pi());
        one_B.SetTheta(B_theta);
        one_B.SetPhi(B_phi);
        one_B.SetRho(p_B);
        one_B.SetE(E_B);
        double this_q2 = (one_B - had_cm).Mag2();
        q2 += this_q2 * sin(B_theta) * sin(B_theta);
        denom += sin(B_theta) * sin(B_theta);
      }

      q2 /= denom;

      return q2;
    }

    Manager::FunctionPtr WE_q2lnuSimple(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      std::string option;

      if (arguments.size() == 0) {
        maskName = "";
        option = "1";
      } else if (arguments.size() == 1) {
        maskName = arguments[0];
        option = "1";
      } else if (arguments.size() == 2) {
        maskName = arguments[0];
        option = arguments[1];
      } else
        B2FATAL("Wrong number of arguments (2 required) for meta function q2lnuSimple(maskname,option)");

      auto func = [maskName, option](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        int n = particle->getNDaughters();

        if (n < 1)
          return -999.9;

        // Assumes lepton is the last particle in the reco decay chain!
        PCmsLabTransform T;
        const Particle* lep = particle->getDaughter(n - 1);
        TLorentzVector lep4vec = T.rotateLabToCms() * lep->get4Vector();
        TLorentzVector nu4vec = missing4Vector(particle, maskName, option);

        return (lep4vec + nu4vec).Mag2();
      };
      return func;
    }

    Manager::FunctionPtr WE_q2lnu(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function q2lnu");

      auto func = [arguments, maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = getRelatedROEObject(particle);

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        int n = particle->getNDaughters();

        if (n < 1)
          return -999.9;

        PCmsLabTransform T;
        const Particle* lep = particle->getDaughter(n - 1);
        TLorentzVector lep_cm = T.rotateLabToCms() * lep->get4Vector();

        TLorentzVector Y_cm = T.rotateLabToCms() * particle->get4Vector();
        TLorentzVector neu_cm = missing4Vector(particle, maskName, "7");

        // Recycled code from Uwe Gebauer <uwe.gebauer@phys.uni-goettingen.de>
        double e_beam = T.getCMSEnergy() / 2.0;

        //just to make the formula simpler
        double bmass = particle->getPDGMass();
        double pB2 = e_beam * e_beam - bmass * bmass;

        //angle between the Y and the neutrino, from the Mbc=M_B constraint
        double cos_angle_nu = (pB2 - Y_cm.Vect().Mag2() - neu_cm.Vect().Mag2()) / (2.0 * Y_cm.Vect().Mag() * neu_cm.Vect().Mag());
        if (abs(cos_angle_nu) > 1)
        {
          return (lep_cm + neu_cm).Mag2();
        }

        double angle_nu = TMath::ACos(cos_angle_nu);
        //first get one random neutrino, on the allowed cone for the constraint
        TLorentzVector rotated_neu(-1 * Y_cm.Vect(), Y_cm.E()); //first get reverse Y

        double nu_theta = rotated_neu.Theta() + (TMath::Pi() - angle_nu);
        double nu_phi = rotated_neu.Phi();
        rotated_neu.SetTheta(nu_theta);
        rotated_neu.SetPhi(nu_phi);
        rotated_neu.SetRho(neu_cm.Rho());
        rotated_neu.SetE(neu_cm.E());

        TVector3 Yneu_norm = Y_cm.Vect().Cross(neu_cm.Vect());
        TVector3 Yrot_norm = Y_cm.Vect().Cross(rotated_neu.Vect());
        //angle between the two crossproducts -> angle between the two vectors perpendicular to the Y-p_inc and Y-B planes -> angle between the planes
        //this angle needs to come out as zero

        double rot_angle = Yneu_norm.Angle(Yrot_norm);

        TLorentzVector rotated_neu2(rotated_neu);
        //unfortunately don't -and probably can't- know in which direction to rotate without trying
        //so create a copy of the vector, and later choose the correct one
        //However, rotation by 180 degrees is never needed, direction of the cross-product vector assures that when after rotation
        //the B-vector is in the plane, it always is on the side closer to pcm_lv_inc.
        //rotate around Y into the Y-neutrino-plane
        rotated_neu.Rotate(rot_angle, Y_cm.Vect());
        rotated_neu2.Rotate(-rot_angle, Y_cm.Vect());

        double dot1 = rotated_neu.Vect().Dot(Yneu_norm);
        double dot2 = rotated_neu2.Vect().Dot(Yneu_norm);

        if (abs(dot2) < abs(dot1)) rotated_neu = rotated_neu2;

        return (lep_cm + rotated_neu).Mag2();
      };
      return func;
    }

    Manager::FunctionPtr WE_cosThetaEll(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function cosThetaEll");

      auto func = [maskName](const Particle * particle) -> double {

        PCmsLabTransform T;
        TLorentzVector boostvec = T.getBeamFourMomentum();
        TLorentzVector pNu = missing4Vector(particle, maskName, "6");

        TLorentzVector pLep;
        // TODO: avoid hardocoded values
        for (unsigned i = 0; i < particle->getNDaughters(); i++)
        {
          int absPDG = abs(particle->getDaughter(i)->getPDGCode());
          if (absPDG == 11 || absPDG == 13 || absPDG == 15) {
            pLep = particle->getDaughter(i)->get4Vector();
            break;
          }
        }

        TLorentzVector pW = pNu + pLep;
        TLorentzVector pB = particle->get4Vector() + pNu;

        // boost lepton and B momentum to W frame
        TVector3 boost2W = -(pW.BoostVector());
        pLep.Boost(boost2W);
        pB.Boost(boost2W);

        TVector3 lep3Vector     = pLep.Vect();
        TVector3 B3Vector       = pB.Vect();
        double numerator   = lep3Vector.Dot(B3Vector);
        double denominator = (lep3Vector.Mag()) * (B3Vector.Mag());

        return numerator / denominator;
      };
      return func;
    }

    Manager::FunctionPtr passesROEMask(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function passesROEMask");

      auto func = [maskName](const Particle * particle) -> double {

        double result = -1;

        StoreObjPtr<RestOfEvent> roe("RestOfEvent");
        if (not roe.isValid())
          return result;

        if (maskName == "")
          return 1.0;
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
      return 0.0;
    }

    // ------------------------------------------------------------------------------
    // Below are some functions for ease of usage, they are not a part of variables
    // ------------------------------------------------------------------------------

    TLorentzVector missing4Vector(const Particle* particle, const std::string& maskName, const std::string& opt)
    {
      // Get related ROE object
      const RestOfEvent* roe = getRelatedROEObject(particle);

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        TLorentzVector empty;
        return empty;
      }

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBeamFourMomentum();

      TLorentzVector rec4vecLAB = particle->get4Vector();
      TLorentzVector roe4vecLAB = roe->get4Vector(maskName);

      TLorentzVector rec4vec = T.rotateLabToCms() * rec4vecLAB;
      TLorentzVector roe4vec = T.rotateLabToCms() * roe4vecLAB;

      TLorentzVector miss4vec;
      double E_beam_cms = T.getCMSEnergy() / 2.0;

      // Definition 0: CMS, use energy and momentum of tracks and clusters
      if (opt == "0") {
        miss4vec.SetVect(- (rec4vec.Vect() + roe4vec.Vect()));
        miss4vec.SetE(2 * E_beam_cms - (rec4vec.E() + roe4vec.E()));
      }

      // Definition 1: CMS, same as 0, fix Emiss = pmiss
      else if (opt == "1") {
        miss4vec.SetVect(- (rec4vec.Vect() + roe4vec.Vect()));
        miss4vec.SetE(miss4vec.Vect().Mag());
      }

      // Definition 2: CMS, same as 0, fix Eroe = Ecms/2
      else if (opt == "2") {
        miss4vec.SetVect(- (rec4vec.Vect() + roe4vec.Vect()));
        miss4vec.SetE(E_beam_cms - rec4vec.E());
      }

      // Definition 3: CMS, use only energy and momentum of signal side
      else if (opt == "3") {
        miss4vec.SetVect(- rec4vec.Vect());
        miss4vec.SetE(E_beam_cms - rec4vec.E());
      }

      // Definition 4: CMS, same as 3, update with direction of ROE momentum
      else if (opt == "4") {
        TVector3 pB = - roe4vec.Vect();
        pB.SetMag(0.340);
        miss4vec.SetVect(pB - rec4vec.Vect());
        miss4vec.SetE(E_beam_cms - rec4vec.E());
      }

      // Definition 5: LAB, use energy and momentum of tracks and clusters from whole event
      else if (opt == "5") {
        miss4vec.SetVect(boostvec.Vect() - (rec4vecLAB.Vect() + roe4vecLAB.Vect()));
        miss4vec.SetE(boostvec.E() - (rec4vecLAB.E() + roe4vecLAB.E()));
      }

      // Definition 6: LAB, same as 5, fix Emiss = pmiss
      else if (opt == "6") {
        miss4vec.SetVect(boostvec.Vect() - (rec4vecLAB.Vect() + roe4vecLAB.Vect()));
        miss4vec.SetE(miss4vec.Vect().Mag());
      }

      // Definition 7: CMS, correct pmiss 3-momentum vector with factor alpha so that dE = 0 (used for Mbc calculation)
      else if (opt == "7") {
        miss4vec.SetVect(-(rec4vec.Vect() + roe4vec.Vect()));
        miss4vec.SetE(miss4vec.Vect().Mag());
        double factorAlpha = (E_beam_cms - rec4vec.E()) / miss4vec.E();
        miss4vec.SetRho(factorAlpha * miss4vec.Rho());
        miss4vec.SetE(miss4vec.Rho());
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
      if (particle->getParticleType() == Particle::c_Composite) {
        std::vector<const Particle*> fspDaug = particle->getFinalStateDaughters();
        for (auto& i : fspDaug) {
          if (isInThisRestOfEvent(i, roe, maskName) == 0)
            return 0;
        }
        return 1.0;
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

    REGISTER_VARIABLE("isInRestOfEvent", isInRestOfEvent,
                      "Returns 1 if a track, ecl or klmCluster associated to particle is in the current RestOfEvent object, 0 otherwise."
                      "One can use this variable only in a for_each loop over the RestOfEvent StoreArray.");

    REGISTER_VARIABLE("isCloneOfSignalSide", isCloneOfSignalSide,
                      "Returns 1 if a particle is a clone of signal side final state particles, 0 otherwise. "
                      "Requires generator information and truth-matching. "
                      "One can use this variable only in a for_each loop over the RestOfEvent StoreArray.");

    REGISTER_VARIABLE("hasAncestorFromSignalSide", hasAncestorFromSignalSide,
                      "Returns 1 if a particle has ancestor from signal side, 0 otherwise. "
                      "Requires generator information and truth-matching. "
                      "One can use this variable only in a for_each loop over the RestOfEvent StoreArray.");

    REGISTER_VARIABLE("currentROEIsInList(particleList)", currentROEIsInList,
                      "[Eventbased] Returns 1 the associated particle of the current ROE is contained in the given list or its charge-conjugated."
                      "Useful to restrict the for_each loop over ROEs to ROEs of a certain ParticleList.");

    REGISTER_VARIABLE("nROE_RemainingTracks", nROE_RemainingTracks,
                      "Returns number of tracks in ROE - number of tracks of given particle"
                      "One can use this variable only in a for_each loop over the RestOfEvent StoreArray.");

    REGISTER_VARIABLE("nROE_RemainingTracks(maskName)", nROE_RemainingTracksWithMask,
                      "Returns number of remaining tracks between the ROE (specified via a mask) and the given particle. For the given particle only tracks are counted which are in the RoE."
                      "One can use this variable only in a for_each loop over the RestOfEvent StoreArray."
                      "Is required for the specific FEI. :noindex:");
    // nROE_RemainingTracks is overloaded (two C++ functions sharing one
    // variable name) so one of the two needs to be made the indexed
    // variable in sphinx

    REGISTER_VARIABLE("nROE_KLMClusters", nROE_KLMClusters,
                      "Returns number of all remaining KLM clusters in the related RestOfEvent object.");

    REGISTER_VARIABLE("nROE_Charged(maskName, PDGcode = 0)", nROE_ChargedParticles,
                      "Returns number of all charged particles in the related RestOfEvent object. First optional argument is ROE mask name. "
                      "Second argument is a PDG code to count only one charged particle species, independently of charge. "
                      "For example: nROE_Charged(cleanMask, 321) will output number of kaons in Rest Of Event with 'cleanMask'. "
                      "PDG code 0 is used to count all charged particles");

    REGISTER_VARIABLE("nROE_Photons(maskName)", nROE_Photons,
                      "Returns number of all photons in the related RestOfEvent object, accepts 1 optional argument of ROE mask name. ");

    REGISTER_VARIABLE("nROE_NeutralHadrons(maskName)", nROE_NeutralHadrons,
                      "Returns number of all neutral hadrons in the related RestOfEvent object, accepts 1 optional argument of ROE mask name. ");

    REGISTER_VARIABLE("particleRelatedToCurrentROE(var)", particleRelatedToCurrentROE,
                      "[Eventbased] Returns variable applied to the particle which is related to the current RestOfEvent object"
                      "One can use this variable only in a for_each loop over the RestOfEvent StoreArray.");

    REGISTER_VARIABLE("ROE_MC_E", ROE_MC_E,
                      "Returns true energy of unused tracks and clusters in ROE, can be used with Use***Frame() function.");

    REGISTER_VARIABLE("ROE_MC_M", ROE_MC_M,
                      "Returns true invariant mass of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("ROE_MC_P", ROE_MC_P,
                      "Returns true momentum of unused tracks and clusters in ROE, can be used with Use***Frame() function.");

    REGISTER_VARIABLE("ROE_MC_Px", ROE_MC_Px,
                      "Returns x component of true momentum of unused tracks and clusters in ROE, can be used with Use***Frame() function.");

    REGISTER_VARIABLE("ROE_MC_Py", ROE_MC_Py,
                      "Returns y component of true momentum of unused tracks and clusters in ROE, can be used with Use***Frame() function.");

    REGISTER_VARIABLE("ROE_MC_Pz", ROE_MC_Pz,
                      "Returns z component of true momentum of unused tracks and clusters in ROE, can be used with Use***Frame() function.");

    REGISTER_VARIABLE("ROE_MC_Pt", ROE_MC_Pt,
                      "Returns transverse component of true momentum of unused tracks and clusters in ROE, can be used with Use***Frame() function.");

    REGISTER_VARIABLE("ROE_MC_PTheta", ROE_MC_PTheta,
                      "Returns polar angle of true momentum of unused tracks and clusters in ROE, can be used with Use***Frame() function.");

    REGISTER_VARIABLE("ROE_MC_MissFlags(maskName)", ROE_MC_MissingFlags,
                      "Returns flags corresponding to missing particles on ROE side.");

    REGISTER_VARIABLE("nROE_Tracks(maskName)",  nROE_Tracks,
                      "Returns number of tracks in the related RestOfEvent object that pass the selection criteria.");

    REGISTER_VARIABLE("nROE_ECLClusters(maskName)", nROE_ECLClusters,
                      "Returns number of ECL clusters in the related RestOfEvent object that pass the selection criteria.");

    REGISTER_VARIABLE("nROE_NeutralECLClusters(maskName)", nROE_NeutralECLClusters,
                      "Returns number of neutral ECL clusters in the related RestOfEvent object that pass the selection criteria.");

    REGISTER_VARIABLE("nROE_ParticlesInList(pListName)", nROE_ParticlesInList,
                      "Returns the number of particles in ROE from the given particle list.\n"
                      "Use of variable aliases is advised.");

    REGISTER_VARIABLE("ROE_charge(maskName)", ROE_Charge,
                      "Returns total charge of the related RestOfEvent object.");

    REGISTER_VARIABLE("ROE_eextra(maskName)", ROE_ExtraEnergy,
                      "Returns extra energy from ECLClusters in the calorimeter that is not associated to the given Particle");

    REGISTER_VARIABLE("ROE_neextra(maskName)", ROE_NeutralExtraEnergy,
                      "Returns extra energy from neutral ECLClusters in the calorimeter that is not associated to the given Particle, can be used with Use***Frame() function.");

    REGISTER_VARIABLE("ROE_E(maskName)", ROE_E,
                      "Returns energy of unused tracks and clusters in ROE, can be used with Use***Frame() function.");

    REGISTER_VARIABLE("ROE_M(maskName)", ROE_M,
                      "Returns invariant mass of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("ROE_P(maskName)", ROE_P,
                      "Returns momentum of unused tracks and clusters in ROE, can be used with Use***Frame() function.");

    REGISTER_VARIABLE("ROE_Pt(maskName)", ROE_Pt,
                      "Returns transverse component of momentum of unused tracks and clusters in ROE, can be used with Use***Frame() function.");

    REGISTER_VARIABLE("ROE_Px(maskName)", ROE_Px,
                      "Returns x component of momentum of unused tracks and clusters in ROE, can be used with Use***Frame() function.");

    REGISTER_VARIABLE("ROE_Py(maskName)", ROE_Py,
                      "Returns y component of momentum of unused tracks and clusters in ROE, can be used with Use***Frame() function.");

    REGISTER_VARIABLE("ROE_Pz(maskName)", ROE_Pz,
                      "Returns z component of momentum of unused tracks and clusters in ROE, can be used with Use***Frame() function.");

    REGISTER_VARIABLE("ROE_PTheta(maskName)", ROE_PTheta,
                      "Returns theta angle of momentum of unused tracks and clusters in ROE, can be used with Use***Frame() function.");

    REGISTER_VARIABLE("ROE_deltae(maskName)", ROE_DeltaE,
                      "Returns energy difference of the related RestOfEvent object with respect to E_cms/2.");

    REGISTER_VARIABLE("ROE_mbc(maskName)", ROE_Mbc,
                      "Returns beam constrained mass of the related RestOfEvent object with respect to E_cms/2.");

    REGISTER_VARIABLE("WE_deltae(maskName, opt)", WE_DeltaE,
                      "Returns the energy difference of the B meson, corrected with the missing neutrino momentum (reconstructed side + neutrino) with respect to E_cms/2.");

    REGISTER_VARIABLE("WE_mbc(maskName, opt)", WE_Mbc,
                      "Returns beam constrained mass of B meson, corrected with the missing neutrino momentum (reconstructed side + neutrino) with respect to E_cms/2.");

    REGISTER_VARIABLE("WE_MissM2(maskName, opt)", WE_MissM2,
                      "Returns the invariant mass squared of the missing momentum (see WE_MissE possible options)");

    REGISTER_VARIABLE("REC_MissM2", REC_MissM2,
                      "Returns the invariant mass squared of the missing momentum calculated assumings the"
                      "reco B is at rest and calculating the neutrino (missing) momentum from p_nu = pB - p_had - p_lep");

    REGISTER_VARIABLE("WE_MissPTheta(maskName, opt)", WE_MissPTheta,
                      "Returns the polar angle of the missing momentum (see possible WE_MissE options)");

    REGISTER_VARIABLE("WE_MissP(maskName, opt)", WE_MissP,
                      "Returns the magnitude of the missing momentum (see possible WE_MissE options)");

    REGISTER_VARIABLE("WE_MissPx(maskName, opt)", WE_MissPx,
                      "Returns the x component of the missing momentum (see WE_MissE possible options)");

    REGISTER_VARIABLE("WE_MissPy(maskName, opt)", WE_MissPy,
                      "Returns the y component of the missing momentum (see WE_MissE possible options)");

    REGISTER_VARIABLE("WE_MissPz(maskName, opt)", WE_MissPz,
                      "Returns the z component of the missing momentum (see WE_MissE possible options)");

    REGISTER_VARIABLE("WE_MissE(maskName, opt)", WE_MissE,
                      R"DOC(Returns the energy of the missing momentum, possible options are the following:
opt = 0: CMS, use energy and momentum of charged particles and photons; 
opt = 1: CMS, same as 0, fix Emiss = pmiss; 
opt = 2: CMS, same as 0, fix Eroe = Ecms/2; 
opt = 3: CMS, use only energy and momentum of signal side; 
opt = 4: CMS, same as 3, update with direction of ROE momentum; 
opt = 5: LAB, use energy and momentum of charged particles and photons from whole event; 
opt = 6: LAB, same as 5, fix Emiss = pmiss; 
opt = 7: CMS, correct pmiss 3-momentum vector with factor alpha so that dE = 0 (used for Mbc calculation).)DOC");

    REGISTER_VARIABLE("WE_xiZ(maskName)", WE_xiZ,
                      "Returns Xi_z in event (for Bhabha suppression and two-photon scattering)");

    REGISTER_VARIABLE("bssMassDifference(maskName)", bssMassDifference,
                      "Bs* - Bs mass difference");

    REGISTER_VARIABLE("WE_cosThetaEll(maskName)", WE_cosThetaEll, R"DOC(

Returns the angle between $M$ and lepton in W rest frame in the decays of the type:
:math`M \to h_1 ... h_n \ell`, where W 4-momentum is given as

.. math::
    p_W = p_\ell + p_\nu.

The neutrino momentum is calculated from ROE taking into account the specified mask, and setting

.. math::
    E_{\nu} = |p_{miss}|.
    
)DOC");

    REGISTER_VARIABLE("REC_q2BhSimple", REC_q2BhSimple,
                      "Returns the momentum transfer squared, q^2, calculated in CMS as q^2 = (p_B - p_h)^2, \n"
                      "where p_h is the CMS momentum of all hadrons in the decay B -> H_1 ... H_n ell nu_ell.\n"
                      "The B meson momentum in CMS is assumed to be 0.");

    REGISTER_VARIABLE("REC_q2Bh", REC_q2Bh,
                      "Returns the momentum transfer squared, q^2, calculated in CMS as q^2 = (p_B - p_h)^2, \n"
                      "where p_h is the CMS momentum of all hadrons in the decay B -> H_1 ... H_n ell nu_ell.\n"
                      "This calculation uses a weighted average of the B meson around the reco B cone");

    REGISTER_VARIABLE("WE_q2lnuSimple(maskName,option)", WE_q2lnuSimple,
                      "Returns the momentum transfer squared, q^2, calculated in LAB as q^2 = (p_l + p_nu)^2, \n"
                      "where B -> H_1 ... H_n ell nu_ell. Lepton is assumed to be the last reconstructed daughter.");

    REGISTER_VARIABLE("WE_q2lnu(maskName)", WE_q2lnu,
                      "Returns the momentum transfer squared, q^2, calculated in LAB as q^2 = (p_l + p_nu)^2, \n"
                      "where B -> H_1 ... H_n ell nu_ell. Lepton is assumed to be the last reconstructed daughter. \n"
                      "This calculation uses constraints from dE = 0 and Mbc = Mb to correct the neutrino direction");

    REGISTER_VARIABLE("WE_MissM2OverMissE(maskName)", WE_MissM2OverMissE,
                      "Returns missing mass squared over missing energy");

    REGISTER_VARIABLE("passesROEMask(maskName)", passesROEMask,
                      "Returns boolean value if track or eclCluster type particle passes a certain mask or not. Only to be used in for_each path");

    REGISTER_VARIABLE("printROE", printROE,
                      "For debugging, prints indices of all particles in the ROE and all masks. Returns 0.");
  }
}
