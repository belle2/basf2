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
#include <analysis/VariableManager/ROEVariables.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>

// framework aux
#include <framework/logging/Logger.h>

// utility
#include <analysis/utility/MCMatching.h>

#include <TRandom.h>
#include <TMath.h>

#include <iostream>

using namespace std;

namespace Belle2 {
  namespace Variable {

    double isCompletelyInRestOfEvent(const Particle* particle)
    {
      // It can happen that for example a cluster is in the rest of event,
      // which is CR - matched to a nearby track which is not in the ROE
      // Hence this variable checks if all MdstObjects are in the ROE
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (not roe.isValid())
        return 0;

      if (particle->getParticleType() == Particle::c_Composite) {
        std::vector<const Particle*> fspDaug = particle->getFinalStateDaughters();
        for (unsigned int i = 0; i < fspDaug.size(); i++) {
          if (isCompletelyInRestOfEvent(fspDaug[i]) == 0) {
            return 0;
          }
        }
        return 1.0;
      } else {
        // Check for Tracks
        const auto& tracks = roe->getTracks();
        if (particle->getTrack() and std::find(tracks.begin(), tracks.end(), particle->getTrack()) == tracks.end()) {
          return 0.0;
        }

        // Check for KLMClusters
        const auto& klm = roe->getKLMClusters();
        if (particle->getKLMCluster() and std::find(klm.begin(), klm.end(), particle->getKLMCluster()) == klm.end()) {
          return 0.0;
        }

        // Check for ECLClusters
        const auto& ecl = roe->getECLClusters();
        if (particle->getECLCluster() and std::find(ecl.begin(), ecl.end(), particle->getECLCluster()) == ecl.end()) {
          return 0.0;
        }
      }

      return 1.0;
    }

    double isInRestOfEvent(const Particle* particle)
    {

      StoreObjPtr<RestOfEvent> roeobjptr;
      if (not roeobjptr.isValid())
        return 0;

      const RestOfEvent* roe = &(*roeobjptr);

      return isInThisRestOfEvent(particle, roe);
    }

    Manager::FunctionPtr currentROEIsInList(const std::vector<std::string>& arguments)
    {
      std::string listName;

      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments (1 required) for meta function currentROEIsInList");

      auto func = [listName](const Particle*) -> double {

        StoreObjPtr<ParticleList> particleList(listName);
        StoreObjPtr<RestOfEvent> roe("RestOfEvent");

        if (not roe.isValid())
          return 0;

        Particle* particle = roe->getRelatedTo<Particle>();
        return particleList->contains(particle) ? 1 : 0;

      };
      return func;
    }


    Manager::FunctionPtr particleRelatedToCurrentROE(const std::vector<std::string>& arguments)
    {
      std::string listName;

      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments (1 required) for meta function particleRelatedToCurrentROE");

      const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);
      auto func = [var](const Particle*) -> double {

        StoreObjPtr<RestOfEvent> roe("RestOfEvent");

        if (not roe.isValid())
          return -999;

        Particle* particle = roe->getRelatedTo<Particle>();
        return var->function(particle);

      };
      return func;
    }


    // only the helper function
    double nRemainingTracksInROE(const Particle* particle, std::string maskName)
    {
      StoreObjPtr<RestOfEvent> roe("RestOfEvent");
      if (not roe.isValid())
        return 0.0;
      std::vector<const Track*> roeTracks = roe->getTracks(maskName);
      int n_roe_tracks = roe->getNTracks(maskName);
      int n_par_tracks = 0;
      const auto& daughters = particle->getFinalStateDaughters();
      for (const auto& daughter : daughters) {
        int pdg = abs(daughter->getPDGCode());
        if (pdg == 11 or pdg == 13 or pdg == 211 or pdg == 321 or pdg == 2212) {
          if (std::find(roeTracks.begin(), roeTracks.end(), daughter->getTrack()) != roeTracks.end())
            n_par_tracks++;
        }
      }
      return n_roe_tracks - n_par_tracks;
    }


    Manager::FunctionPtr nRemainingTracksInRestOfEventWithMask(const std::vector<std::string>& arguments)
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


    double nRemainingTracksInRestOfEvent(const Particle* particle)
    {
      return nRemainingTracksInROE(particle);
    }

    double nROEKLMClusters(const Particle* particle)
    {
      // Get related ROE object
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        return -1;
      }

      return roe->getNKLMClusters();
    }

    double mcROE_E(const Particle* particle)
    {
      const MCParticle* mcp = particle->getRelated<MCParticle>();

      if (!mcp)
        return -999.9;

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBoostVector();

      return boostvec.Energy() - mcp->getEnergy();
    }

    double mcROE_P(const Particle* particle)
    {
      const MCParticle* mcp = particle->getRelated<MCParticle>();

      if (!mcp)
        return -999.9;

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBoostVector();

      return (boostvec.Vect() - mcp->getMomentum()).Mag();
    }

    double mcROE_Px(const Particle* particle)
    {
      const MCParticle* mcp = particle->getRelated<MCParticle>();

      if (!mcp)
        return -999.9;

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBoostVector();

      return boostvec.Vect().X() - mcp->getMomentum().X();
    }

    double mcROE_Py(const Particle* particle)
    {
      const MCParticle* mcp = particle->getRelated<MCParticle>();

      if (!mcp)
        return -999.9;

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBoostVector();

      return boostvec.Vect().Y() - mcp->getMomentum().Y();
    }

    double mcROE_Pz(const Particle* particle)
    {
      const MCParticle* mcp = particle->getRelated<MCParticle>();

      if (!mcp)
        return -999.9;

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBoostVector();

      return boostvec.Vect().Z() - mcp->getMomentum().Z();
    }

    double mcROE_M(const Particle* particle)
    {
      const MCParticle* mcp = particle->getRelated<MCParticle>();

      if (!mcp)
        return -999.9;

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBoostVector();

      return (boostvec - mcp->get4Vector()).Mag();
    }

    Manager::FunctionPtr ROEMCMissingFlags(const std::vector<std::string>& arguments)
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        // Load ROE Tracks
        std::vector<const Track*> roeTracks = roe->getTracks(maskName);

        // Add tracks in ROE V0 list, if they exist
        std::vector<unsigned int> v0List = roe->getV0IDList(maskName);
        for (unsigned int iV0 = 0; iV0 < v0List.size(); iV0++)
        {
          roeTracks.push_back(particles[v0List[iV0]]->getDaughter(0)->getTrack());
          roeTracks.push_back(particles[v0List[iV0]]->getDaughter(1)->getTrack());
        }

        // Load ROE ECLClusters
        std::vector<const ECLCluster*> roeECL = roe->getECLClusters(maskName);

        StoreArray<MCParticle> mcParticles;
        std::set<const MCParticle*> mcROEObjects;

        // Fill Track MCParticles to std::set
        for (unsigned i = 0; i < roeTracks.size(); i++)
          if (roeTracks[i]->getRelated<MCParticle>())
            mcROEObjects.insert(roeTracks[i]->getRelated<MCParticle>());

        // Fill only photon MCParticles which have good relations, copied from ParticleLoader until a better solution is established
        for (unsigned i = 0; i < roeECL.size(); i++)
        {
          if (roeECL[i]->isNeutral()) {
            // ECLCluster can be matched to multiple MCParticles
            // order the relations by weights and set Particle -> multiple MCParticle relation
            // preserve the weight
            RelationVector<MCParticle> mcRelations = roeECL[i]->getRelationsTo<MCParticle>();
            // order relations by weights
            std::vector<std::pair<int, double>> weightsAndIndices;
            for (unsigned int iMCParticle = 0; iMCParticle < mcRelations.size(); iMCParticle++) {
              const MCParticle* relMCParticle = mcRelations[iMCParticle];
              double weight = mcRelations.weight(iMCParticle);
              if (relMCParticle)
                weightsAndIndices.push_back(std::make_pair(relMCParticle->getArrayIndex(), weight));
            }
            // sort descending by weight
            std::sort(weightsAndIndices.begin(), weightsAndIndices.end(), [](const std::pair<int, double>& left,
            const std::pair<int, double>& right) {
              return left.second > right.second;
            });

            // insert MCParticle based on relation strength
            for (unsigned int j = 0; j < weightsAndIndices.size(); j++) {
              const MCParticle* relMCParticle = mcParticles[weightsAndIndices[j].first];
              double weight = weightsAndIndices[j].second;

              // TODO: study this further and avoid hardcoded values
              // set the relation only if the MCParticle's energy contribution
              // to this cluster amounts to at least 25%
              if (relMCParticle)
                if (weight / roeECL[i]->getEnergy() > 0.20 &&  weight / relMCParticle->getEnergy() > 0.30 && relMCParticle->getPDG() == 22)
                  mcROEObjects.insert(relMCParticle);
            }
          }
        }

        int flags = 0;
        checkMCParticleMissingFlags(mcROE, mcROEObjects, flags);

        return flags;
      };
      return func;
    }

    Manager::FunctionPtr nROETracks(const std::vector<std::string>& arguments)
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return roe->getNTracks(maskName);
      };
      return func;
    }

    Manager::FunctionPtr nROEECLClusters(const std::vector<std::string>& arguments)
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return roe->getNECLClusters(maskName);
      };
      return func;
    }

    Manager::FunctionPtr nROENeutralECLClusters(const std::vector<std::string>& arguments)
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        // Get unused ECLClusters in ROE
        std::vector<const ECLCluster*> roeClusters = roe->getECLClusters(maskName);
        int nNeutrals = 0;

        // Select ECLClusters with no associated tracks
        for (unsigned int iEcl = 0; iEcl < roeClusters.size(); iEcl++)
          if (roeClusters[iEcl]->isNeutral())
            nNeutrals++;

        return nNeutrals;
      };
      return func;
    }

    Manager::FunctionPtr nParticlesInROE(const std::vector<std::string>& arguments)
    {

      if (arguments.size() != 1)
        B2FATAL("Wrong number of arguments (1 required) for meta function nParticlesInROE");

      std::string pListName = arguments[0];

      auto func = [pListName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        int nPart = 0;

        // Get pi0 particle list
        StoreObjPtr<ParticleList> pList(pListName);
        if (!pList.isValid())
          B2FATAL("ParticleList " << pListName << " could not be found or is not valid!");

        for (unsigned int i = 0; i < pList->getListSize(); i++)
        {
          const Particle* part = pList->getParticle(i);
          if (isInThisRestOfEvent(part, roe) == 1)
            ++nPart;
        }

        return nPart;
      };
      return func;
    }

    Manager::FunctionPtr ROECharge(const std::vector<std::string>& arguments)
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        // Get tracks in ROE
        std::vector<const Track*> roeTracks = roe->getTracks(maskName);
        int roeCharge = 0;

        for (unsigned int iTrack = 0; iTrack < roeTracks.size(); iTrack++)
        {
          auto closestMassTrackFitResult = roeTracks[iTrack]->getTrackFitResultWithClosestMass(Const::pion);
          roeCharge += closestMassTrackFitResult->getChargeSign();
        }

        return roeCharge;
      };
      return func;
    }

    Manager::FunctionPtr ROEExtraEnergy(const std::vector<std::string>& arguments)
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        std::vector<const ECLCluster*> roeClusters = roe->getECLClusters(maskName);
        double extraE = 0.0;

        for (unsigned int iEcl = 0; iEcl < roeClusters.size(); iEcl++)
          extraE += roeClusters[iEcl]->getEnergy();

        return extraE;
      };
      return func;
    }

    Manager::FunctionPtr ROENeutralExtraEnergy(const std::vector<std::string>& arguments)
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        std::vector<const ECLCluster*> roeClusters = roe->getECLClusters(maskName);
        double extraE = 0.0;

        for (unsigned int iEcl = 0; iEcl < roeClusters.size(); iEcl++)
          if (roeClusters[iEcl]->isNeutral())
            extraE += roeClusters[iEcl]->getEnergy();

        return extraE;
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
        return roe->get4Vector(maskName).Energy();
      };
      return func;
    }

    Manager::FunctionPtr ROE_Ecms(const std::vector<std::string>& arguments)
    {
      std::string maskName;
      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function ROE_Ecms");
      auto func = [maskName](const Particle * particle) -> double {
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();
        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }
        PCmsLabTransform T;
        return (T.rotateLabToCms() * roe->get4Vector(maskName)).Energy();
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return roe->get4Vector(maskName).Vect().Mag();
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return roe->get4Vector(maskName).Vect().X();
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return roe->get4Vector(maskName).Vect().Y();
      };
      return func;
    }

    Manager::FunctionPtr ROE_Pcms(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function ROE_Pcms");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }
        PCmsLabTransform T;
        return (T.rotateLabToCms() * roe->get4Vector(maskName)).Vect().Mag();
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return roe->get4Vector(maskName).Vect().Perp();
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return roe->get4Vector(maskName).Vect().Z();
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return roe->get4Vector(maskName).Theta();
      };
      return func;
    }

    Manager::FunctionPtr ROE_PThetacms(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function ROE_PThetacms");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }
        PCmsLabTransform T;
        return (T.rotateLabToCms() * roe->get4Vector(maskName)).Theta();
      };
      return func;
    }

    Manager::FunctionPtr ROEDeltaE(const std::vector<std::string>& arguments)
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

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

    Manager::FunctionPtr ROEMbc(const std::vector<std::string>& arguments)
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

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

    Manager::FunctionPtr correctedBMesonDeltaE(const std::vector<std::string>& arguments)
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
        TLorentzVector boostvec = T.getBoostVector();
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

    Manager::FunctionPtr correctedBMesonMbc(const std::vector<std::string>& arguments)
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
        TLorentzVector boostvec = T.getBoostVector();
        TLorentzVector sig4vec = T.rotateLabToCms() * particle->get4Vector();
        TLorentzVector sig4vecLAB = particle->get4Vector();
        TLorentzVector neutrino4vec = missing4Vector(particle, maskName, "1");
        TLorentzVector neutrino4vecLAB = missing4Vector(particle, maskName, "6");

        double mbc = -999.9;

        // Definition 0: CMS
        if (opt == "0")
        {
          TVector3 bmom = (sig4vec + neutrino4vec).Vect();
          double E = T.getCMSEnergy() / 2;
          double m2 = E * E - bmom.Mag2();
          mbc = m2 > 0 ? sqrt(m2) : 0;
        }

        // Definition 1: LAB
        else if (opt == "1")
        {
          TVector3 bmom = (sig4vecLAB + neutrino4vecLAB).Vect();
          double Ecms = T.getCMSEnergy();
          double s = Ecms * Ecms;
          double m2 = pow((s / 2.0 + bmom * boostvec.Vect()) / boostvec.Energy(), 2.0) - bmom.Mag2();
          mbc = m2 > 0 ? sqrt(m2) : 0;
        }

        else
          B2FATAL("Option for correctedB_mbc variable should only be 0/1 (CMS/LAB)");

        return mbc;
      };
      return func;
    }

    Manager::FunctionPtr missM2(const std::vector<std::string>& arguments)
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

    double missM2rec(const Particle* particle)
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

    Manager::FunctionPtr missPTheta(const std::vector<std::string>& arguments)
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
        B2FATAL("Wrong number of arguments (2 required) for meta function missPTheta");

      auto func = [maskName, opt](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return missing4Vector(particle, maskName, opt).Theta();
      };
      return func;
    }

    Manager::FunctionPtr missP(const std::vector<std::string>& arguments)
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
        B2FATAL("Wrong number of arguments (2 required) for meta function missP");

      auto func = [maskName, opt](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return missing4Vector(particle, maskName, opt).Vect().Mag();
      };
      return func;
    }

    Manager::FunctionPtr missPx(const std::vector<std::string>& arguments)
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
        B2FATAL("Wrong number of arguments (2 required) for meta function missPx");

      auto func = [maskName, opt](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return missing4Vector(particle, maskName, opt).Vect().Px();
      };
      return func;
    }

    Manager::FunctionPtr missPy(const std::vector<std::string>& arguments)
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
        B2FATAL("Wrong number of arguments (2 required) for meta function missPy");

      auto func = [maskName, opt](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return missing4Vector(particle, maskName, opt).Vect().Py();
      };
      return func;
    }

    Manager::FunctionPtr missPz(const std::vector<std::string>& arguments)
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
        B2FATAL("Wrong number of arguments (2 required) for meta function missPz");

      auto func = [maskName, opt](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return missing4Vector(particle, maskName, opt).Vect().Pz();
      };
      return func;
    }

    Manager::FunctionPtr missE(const std::vector<std::string>& arguments)
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
        B2FATAL("Wrong number of arguments (2 required) for meta function missE");

      auto func = [maskName, opt](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        return missing4Vector(particle, maskName, opt).Energy();
      };
      return func;
    }

    Manager::FunctionPtr xiZ(const std::vector<std::string>& arguments)
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

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
        for (unsigned int i = 0; i < recTrackParticles.size(); i++)
        {
          pz += recTrackParticles[i]->getPz();
          energy += recTrackParticles[i]->getEnergy();
        }

        // "Loop" the ROE side
        pz += roe->get4VectorTracks(maskName).Vect().Pz();
        energy += roe->get4VectorTracks(maskName).Energy();

        return pz / energy;
      };
      return func;
    }

    Manager::FunctionPtr missM2OverMissE(const std::vector<std::string>& arguments)
    {
      std::string maskName;

      if (arguments.size() == 0)
        maskName = "";
      else if (arguments.size() == 1)
        maskName = arguments[0];
      else
        B2FATAL("Wrong number of arguments (1 required) for meta function missM2OverMissE");

      auto func = [maskName](const Particle * particle) -> double {

        // Get related ROE object
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

        if (!roe)
        {
          B2ERROR("Relation between particle and ROE doesn't exist!");
          return -1;
        }

        PCmsLabTransform T;
        TLorentzVector missing4Momentum;
        TLorentzVector boostvec = T.getBoostVector();

        return missing4Vector(particle, maskName, "5").Mag2() / (2.0 * missing4Vector(particle, maskName, "5").Energy());
      };
      return func;
    }

    double q2BhSimple(const Particle* particle)
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

    double q2Bh(const Particle* particle)
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
      double cos_cone_angle = Variable::cosThetaBetweenParticleAndTrueB(particle);

      if (abs(cos_cone_angle) > 1) {
        //makes no sense in this case, return simple value
        return Variable::q2BhSimple(particle);
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


    Manager::FunctionPtr q2lnuSimple(const std::vector<std::string>& arguments)
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

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

    Manager::FunctionPtr q2lnu(const std::vector<std::string>& arguments)
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
        const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

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

    Manager::FunctionPtr cosThetaEll(const std::vector<std::string>& arguments)
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
        TLorentzVector boostvec = T.getBoostVector();
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

        if (particle->getParticleType() == Particle::c_Track)
        {
          const Track* track = particle->getTrack();

          std::map<unsigned int, bool> trackMask = roe->getTrackMask(maskName);

          auto it = trackMask.find(track->getArrayIndex());
          if (it == trackMask.end())
            B2ERROR("Something is wrong, track not found in map of ROE tracks!");
          else
            result = trackMask[track->getArrayIndex()];
        } else if (particle->getParticleType() == Particle::c_ECLCluster)
        {
          const ECLCluster* ecl = particle->getECLCluster();

          std::map<unsigned int, bool> eclClusterMask = roe->getECLClusterMask(maskName);

          auto it = eclClusterMask.find(ecl->getArrayIndex());
          if (it == eclClusterMask.end())
            B2ERROR("Something is wrong, cluster not found in map of ROE clusters!");
          else
            result = eclClusterMask[ecl->getArrayIndex()];
        } else
          B2ERROR("Particle used is not an ECLCluster or Track type particle!");
        return result;
      };
      return func;
    }



    // ------------------------------------------------------------------------------
    // Below are some functions for ease of usage, they are not a part of variables
    // ------------------------------------------------------------------------------

    TLorentzVector missing4Vector(const Particle* particle, std::string maskName, const std::string& opt)
    {
      // Get related ROE object
      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (!roe) {
        B2ERROR("Relation between particle and ROE doesn't exist!");
        TLorentzVector empty;
        return empty;
      }

      PCmsLabTransform T;
      TLorentzVector boostvec = T.getBoostVector();

      TLorentzVector rec4vecLAB = particle->get4Vector();
      TLorentzVector roe4vecLAB = roe->get4Vector(maskName);

      TLorentzVector rec4vec = T.rotateLabToCms() * rec4vecLAB;
      TLorentzVector roe4vec = T.rotateLabToCms() * roe4vecLAB;

      TLorentzVector miss4vec;
      double E_beam_cms = T.getCMSEnergy() / 2.0;

      // Definition 0: CMS, use energy and momentum of tracks and clusters
      if (opt == "0") {
        miss4vec.SetVect(- (rec4vec.Vect() + roe4vec.Vect()));
        miss4vec.SetE(2 * E_beam_cms - (rec4vec.Energy() + roe4vec.Energy()));
      }

      // Definition 1: CMS, same as 0, fix Emiss = pmiss
      else if (opt == "1") {
        miss4vec.SetVect(- (rec4vec.Vect() + roe4vec.Vect()));
        miss4vec.SetE(miss4vec.Vect().Mag());
      }

      // Definition 2: CMS, same as 0, fix Eroe = Ecms/2
      else if (opt == "2") {
        miss4vec.SetVect(- (rec4vec.Vect() + roe4vec.Vect()));
        miss4vec.SetE(E_beam_cms - rec4vec.Energy());
      }

      // Definition 3: CMS, use only energy and momentum of signal side
      else if (opt == "3") {
        miss4vec.SetVect(- rec4vec.Vect());
        miss4vec.SetE(E_beam_cms - rec4vec.Energy());
      }

      // Definition 4: CMS, same as 3, update with direction of ROE momentum
      else if (opt == "4") {
        TVector3 pB = - roe4vec.Vect();
        pB.SetMag(0.340);
        miss4vec.SetVect(pB - rec4vec.Vect());
        miss4vec.SetE(E_beam_cms - rec4vec.Energy());
      }

      // Definition 5: LAB, use energy and momentum of tracks and clusters from whole event
      else if (opt == "5") {
        miss4vec.SetVect(boostvec.Vect() - (rec4vecLAB.Vect() + roe4vecLAB.Vect()));
        miss4vec.SetE(boostvec.Energy() - (rec4vecLAB.Energy() + roe4vecLAB.Energy()));
      }

      // Definition 6: LAB, same as 5, fix Emiss = pmiss
      else if (opt == "6") {
        miss4vec.SetVect(boostvec.Vect() - (rec4vecLAB.Vect() + roe4vecLAB.Vect()));
        miss4vec.SetE(miss4vec.Vect().Mag());
      }

      // Definition 7: LAB, same as 6, correct pmiss 4-momentum vector with factor alpha
      else if (opt == "7") {
        TLorentzVector miss4vecRaw;
        miss4vecRaw.SetVect(boostvec.Vect() - (rec4vecLAB.Vect() + roe4vecLAB.Vect()));
        double Emiss = boostvec.Energy() - (rec4vecLAB.Energy() + roe4vecLAB.Energy());
        miss4vecRaw.SetE(Emiss);
        double Ecms = T.getCMSEnergy();
        double s = Ecms * Ecms;
        double deltaE = ((rec4vecLAB + miss4vecRaw) * boostvec - s / 2.0) / sqrt(s);
        double factorAlpha = 1.0 - deltaE / Emiss;
        miss4vec.SetVect(TVector3(factorAlpha * miss4vecRaw.Vect()));
        miss4vec.SetE(miss4vec.Vect().Mag());
      }

      return miss4vec;
    }

    void checkMCParticleMissingFlags(const MCParticle* mcp, std::set<const MCParticle*> mcROEObjects, int& missingFlags)
    {
      std::vector<MCParticle*> daughters = mcp->getDaughters();
      for (unsigned i = 0; i < daughters.size(); i++) {

        if (!daughters[i]->hasStatus(MCParticle::c_PrimaryParticle))
          continue;

        if (mcROEObjects.find(daughters[i]) == mcROEObjects.end()) {

          int pdg = abs(daughters[i]->getPDG());

          // photon
          if (pdg == 22 and (missingFlags & 1) == 0)
            missingFlags += 1;

          // electrons
          else if (pdg == 11 and (missingFlags & 2) == 0)
            missingFlags += 2;

          // muons
          else if (pdg == 13 and (missingFlags & 4) == 0)
            missingFlags += 4;

          // pions
          else if (pdg == 211 and (missingFlags & 8) == 0)
            missingFlags += 8;

          // kaons
          else if (pdg == 321 and (missingFlags & 16) == 0)
            missingFlags += 16;

          // protons
          else if (pdg == 2212 and (missingFlags & 32) == 0)
            missingFlags += 32;

          // neutrons
          else if (pdg == 2112 and (missingFlags & 64) == 0)
            missingFlags += 64;

          // kshort
          else if (pdg == 310 and ((missingFlags & 128) == 0 or (missingFlags & 256) == 0)) {
            std::vector<MCParticle*> ksDaug = daughters[i]->getDaughters();
            if (ksDaug.size() == 2) {
              // K_S0 -> pi+ pi-
              if (abs(ksDaug[0]->getPDG()) == 211 and abs(ksDaug[1]->getPDG()) == 211 and (missingFlags & 128) == 0) {
                if (mcROEObjects.find(ksDaug[0]) == mcROEObjects.end() or mcROEObjects.find(ksDaug[1]) == mcROEObjects.end())
                  missingFlags += 128;
              }
              // K_S0 -> pi0 pi0
              else if (abs(ksDaug[0]->getPDG()) == 111 and abs(ksDaug[1]->getPDG()) == 111 and (missingFlags & 256) == 0) {
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
          else if (pdg == 130 and (missingFlags & 512) == 0)
            missingFlags += 512;

          // neutrino
          else if ((pdg == 12 or pdg == 14 or pdg == 16) and (missingFlags & 1024) == 0)
            missingFlags += 1024;
        }
        checkMCParticleMissingFlags(daughters[i], mcROEObjects, missingFlags);
      }
    }

    double isInThisRestOfEvent(const Particle* particle, const RestOfEvent* roe)
    {
      if (particle->getParticleType() == Particle::c_Composite) {
        std::vector<const Particle*> fspDaug = particle->getFinalStateDaughters();
        for (unsigned int i = 0; i < fspDaug.size(); i++) {
          if (isInThisRestOfEvent(fspDaug[i], roe) == 0)
            return 0;
        }
        return 1.0;
      } else {
        // Check for Tracks
        const auto& tracks = roe->getTracks();
        if (std::find(tracks.begin(), tracks.end(), particle->getTrack()) != tracks.end()) {
          return 1.0;
        }

        // Check for KLMClusters
        const auto& klm = roe->getKLMClusters();
        if (std::find(klm.begin(), klm.end(), particle->getKLMCluster()) != klm.end()) {
          return 1.0;
        }

        // Check for ECLClusters
        const auto& ecl = roe->getECLClusters();
        if (std::find(ecl.begin(), ecl.end(), particle->getECLCluster()) != ecl.end()) {
          return 1.0;
        }
      }
      return 0;
    }


    VARIABLE_GROUP("Rest Of Event");

    REGISTER_VARIABLE("isInRestOfEvent", isInRestOfEvent,
                      "Returns 1 if a track, ecl or klmCluster associated to particle is in the current RestOfEvent object, 0 otherwise."
                      "One can use this variable only in a for_each loop over the RestOfEvent StoreArray.");

    REGISTER_VARIABLE("isCompletelyInRestOfEvent", isCompletelyInRestOfEvent,
                      "Similar to isInRestOfEvent, but checks if all Mdst objects of the particle are in the RestOfEvent"
                      "One can use this to distinguish ECLClusters with a connected-region matched Track outside the ROE,"
                      "from ordinary ECLClusters in the ROE mask for ECLClusters.");

    REGISTER_VARIABLE("currentROEIsInList(particleList)", currentROEIsInList,
                      "[Eventbased] Returns 1 the associated particle of the current ROE is contained in the given list or its charge-conjugated."
                      "Useful to restrict the for_each loop over ROEs to ROEs of a certain ParticleList.");

    REGISTER_VARIABLE("nRemainingTracksInRestOfEvent", nRemainingTracksInRestOfEvent,
                      "Returns number of tracks in ROE - number of tracks of given particle"
                      "One can use this variable only in a for_each loop over the RestOfEvent StoreArray.");

    REGISTER_VARIABLE("nRemainingTracksInRestOfEvent(maskName)", nRemainingTracksInRestOfEventWithMask,
                      "Returns number of remaining tracks between the ROE (specified via a mask) and the given particle. For the given particle only tracks are counted which are in the RoE."
                      "One can use this variable only in a for_each loop over the RestOfEvent StoreArray."
                      "Is required for the specific FEI.");

    REGISTER_VARIABLE("nROEKLMClusters", nROEKLMClusters,
                      "Returns number of all remaining KLM clusters in the related RestOfEvent object.");

    REGISTER_VARIABLE("particleRelatedToCurrentROE(var)", particleRelatedToCurrentROE,
                      "[Eventbased] Returns variable applied to the particle which is related to the current RestOfEvent object"
                      "One can use this variable only in a for_each loop over the RestOfEvent StoreArray.");

    REGISTER_VARIABLE("mcROE_E", mcROE_E,
                      "Returns true energy of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("mcROE_M", mcROE_M,
                      "Returns true invariant mass of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("mcROE_P", mcROE_P,
                      "Returns true momentum of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("mcROE_Px", mcROE_Px,
                      "Returns x component of true momentum of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("mcROE_Py", mcROE_Py,
                      "Returns y component of true momentum of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("mcROE_Pz", mcROE_Pz,
                      "Returns z component of true momentum of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("ROE_mcMissFlags(maskName)", ROEMCMissingFlags,
                      "Returns flags corresponding to missing particles on ROE side.");

    REGISTER_VARIABLE("nROETracks(maskName)",  nROETracks,
                      "Returns number of tracks in the related RestOfEvent object that pass the selection criteria.");

    REGISTER_VARIABLE("nROEECLClusters(maskName)", nROEECLClusters,
                      "Returns number of ECL clusters in the related RestOfEvent object that pass the selection criteria.");

    REGISTER_VARIABLE("nROENeutralECLClusters(maskName)", nROENeutralECLClusters,
                      "Returns number of neutral ECL clusters in the related RestOfEvent object that pass the selection criteria.");

    REGISTER_VARIABLE("nParticlesInROE(pListName)", nParticlesInROE,
                      "Returns the number of particles in ROE from the given particle list.\n"
                      "Use of variable aliases is advised.");

    REGISTER_VARIABLE("ROE_charge(maskName)", ROECharge,
                      "Returns total charge of the related RestOfEvent object.");

    REGISTER_VARIABLE("ROE_eextra(maskName)", ROEExtraEnergy,
                      "Returns extra energy from ECLClusters in the calorimeter that is not associated to the given Particle");

    REGISTER_VARIABLE("ROE_neextra(maskName)", ROENeutralExtraEnergy,
                      "Returns extra energy from neutral ECLClusters in the calorimeter that is not associated to the given Particle");

    REGISTER_VARIABLE("ROE_E(maskName)", ROE_E,
                      "Returns energy of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("ROE_Ecms(maskName)", ROE_Ecms,
                      "Returns energy of unused tracks and clusters in ROE in cms fame");

    REGISTER_VARIABLE("ROE_M(maskName)", ROE_M,
                      "Returns invariant mass of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("ROE_P(maskName)", ROE_P,
                      "Returns momentum of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("ROE_Pcms(maskName)", ROE_Pcms,
                      "Returns momentum of unused tracks and clusters in ROE in cms");

    REGISTER_VARIABLE("ROE_Pt(maskName)", ROE_Pt,
                      "Returns transverse component of momentum of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("ROE_Px(maskName)", ROE_Px,
                      "Returns x component of momentum of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("ROE_Py(maskName)", ROE_Py,
                      "Returns y component of momentum of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("ROE_Pz(maskName)", ROE_Pz,
                      "Returns z component of momentum of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("ROE_PTheta(maskName)", ROE_PTheta,
                      "Returns theta angle of momentum of unused tracks and clusters in ROE");

    REGISTER_VARIABLE("ROE_PThetacms(maskName)", ROE_PThetacms,
                      "Returns theta angle of momentum of unused tracks and clusters in ROE in cms");

    REGISTER_VARIABLE("ROE_deltae(maskName)", ROEDeltaE,
                      "Returns energy difference of the related RestOfEvent object with respect to E_cms/2.");

    REGISTER_VARIABLE("ROE_mbc(maskName)", ROEMbc,
                      "Returns beam constrained mass of the related RestOfEvent object with respect to E_cms/2.");

    REGISTER_VARIABLE("correctedB_deltae(maskName, opt)", correctedBMesonDeltaE,
                      "Returns the energy difference of the B meson, corrected with the missing neutrino momentum (reconstructed side + neutrino) with respect to E_cms/2.");

    REGISTER_VARIABLE("correctedB_mbc(maskName, opt)", correctedBMesonMbc,
                      "Returns beam constrained mass of B meson, corrected with the missing neutrino momentum (reconstructed side + neutrino) with respect to E_cms/2.");

    REGISTER_VARIABLE("missM2(maskName, opt)", missM2,
                      "Returns the invariant mass squared of the missing momentum (see possible options)");

    REGISTER_VARIABLE("missM2rec", missM2rec,
                      "Returns the invariant mass squared of the missing momentum calculated only with the reco side.");

    REGISTER_VARIABLE("missPTheta(maskName, opt)", missPTheta,
                      "Returns the polar angle of the missing momentum (see possible options)");

    REGISTER_VARIABLE("missP(maskName, opt)", missP,
                      "Returns the magnitude of the missing momentum (see possible options)");

    REGISTER_VARIABLE("missPx(maskName, opt)", missPx,
                      "Returns the x component of the missing momentum (see possible options)");

    REGISTER_VARIABLE("missPy(maskName, opt)", missPy,
                      "Returns the y component of the missing momentum (see possible options)");

    REGISTER_VARIABLE("missPz(maskName, opt)", missPz,
                      "Returns the z component of the missing momentum (see possible options)");

    REGISTER_VARIABLE("missE(maskName, opt)", missE,
                      "Returns the energy of the missing momentum (see possible options)");

    REGISTER_VARIABLE("xiZ(maskName)", xiZ,
                      "Returns Xi_z in event (for Bhabha suppression and two-photon scattering)");

    REGISTER_VARIABLE("bssMassDifference(maskName)", bssMassDifference,
                      "Bs* - Bs mass difference");

    REGISTER_VARIABLE("cosThetaEll(maskName)", cosThetaEll,
                      "Returns the angle between M and lepton in W rest frame in the decays of the type\n"
                      "M -> h_1 ... h_n ell, where W 4-momentum is given as pW = p_ell + p_nu. The neutrino\n"
                      "momentum is calculated from ROE taking into account the specified mask and setting\n"
                      "E_nu = |p_miss|.");

    REGISTER_VARIABLE("q2BhSimple", q2BhSimple,
                      "Returns the momentum transfer squared, q^2, calculated in CMS as q^2 = (p_B - p_h)^2, \n"
                      "where p_h is the CMS momentum of all hadrons in the decay B -> H_1 ... H_n ell nu_ell.\n"
                      "The B meson momentum in CMS is assumed to be 0.");

    REGISTER_VARIABLE("q2Bh", q2Bh,
                      "Returns the momentum transfer squared, q^2, calculated in CMS as q^2 = (p_B - p_h)^2, \n"
                      "where p_h is the CMS momentum of all hadrons in the decay B -> H_1 ... H_n ell nu_ell.\n"
                      "This calculation uses a weighted average of the B meson around the reco B cone");

    REGISTER_VARIABLE("q2lnuSimple(maskName,option)", q2lnuSimple,
                      "Returns the momentum transfer squared, q^2, calculated in LAB as q^2 = (p_l + p_nu)^2, \n"
                      "where B -> H_1 ... H_n ell nu_ell. Lepton is assumed to be the last reconstructed daughter.");

    REGISTER_VARIABLE("q2lnu(maskName)", q2lnu,
                      "Returns the momentum transfer squared, q^2, calculated in LAB as q^2 = (p_l + p_nu)^2, \n"
                      "where B -> H_1 ... H_n ell nu_ell. Lepton is assumed to be the last reconstructed daughter. \n"
                      "This calculation uses constraints from dE = 0 and Mbc = Mb to correct the neutrino direction");

    REGISTER_VARIABLE("missM2OverMissE(maskName)", missM2OverMissE,
                      "Returns custom variable missing mass squared over missing energy");

    REGISTER_VARIABLE("passesROEMask(maskName)", passesROEMask,
                      "Returns boolean value if track or eclCluster type particle passes a certain mask or not. Only to be used in for_each path");
  }
}
