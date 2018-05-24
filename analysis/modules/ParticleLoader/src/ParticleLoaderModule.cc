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
#include <analysis/modules/ParticleLoader/ParticleLoaderModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>
#include <framework/core/ModuleParam.templateDetails.h>

// dataobjects
#include <mdst/dataobjects/V0.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/EventExtraInfo.h>

// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>

#include <utility>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticleLoader)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticleLoaderModule::ParticleLoaderModule() : Module()

  {
    setDescription("Loads MDST dataobjects as Particle objects to the StoreArray<Particle> and collects them in specified ParticleList.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    std::vector<std::tuple<std::string, std::string>> emptyDecayStringsAndCuts;

    addParam("decayStringsWithCuts", m_decayStringsWithCuts,
             "List of (decayString, Variable::Cut) tuples that specify all output ParticleLists to be created by the module. Only Particles that pass specified selection criteria are added to the ParticleList (see https://confluence.desy.de/display/BI/Physics+DecayString and https://confluence.desy.de/display/BI/Physics+ParticleSelectorFunctions).",
             emptyDecayStringsAndCuts);

    addParam("useMCParticles", m_useMCParticles,
             "Use MCParticles instead of reconstructed MDST dataobjects (tracks, ECL, KLM, clusters, V0s, ...)", false);

    addParam("writeOut", m_writeOut,
             "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);

    addParam("addDaughters", m_addDaughters,
             "If true, the particles from the bottom part of the selected particle's decay chain will also be created in the datastore and mother-daughter relations are recursively set",
             false);

    addParam("trackHypothesis", m_trackHypothesis,
             "Track hypothesis to use when loading the particle. By default, use the particle's own hypothesis.",
             0);

    addParam("enforceFitHypothesis", m_enforceFitHypothesis,
             "If true, a Particle is only created if a track fit with the particle hypothesis passed to the ParticleLoader is available.",
             m_enforceFitHypothesis);
  }

  void ParticleLoaderModule::initialize()
  {
    B2INFO("ParticleLoader's Summary of Actions:");

    StoreArray<Particle> particles;
    StoreArray<MCParticle> mcparticles;
    StoreArray<PIDLikelihood> pidlikelihoods;
    StoreObjPtr<ParticleExtraInfoMap> extraInfoMap;
    StoreObjPtr<EventExtraInfo> eventExtraInfo;

    particles.registerInDataStore();
    extraInfoMap.registerInDataStore();
    eventExtraInfo.registerInDataStore();
    //register relations if these things exists
    if (mcparticles.isOptional()) {
      particles.registerRelationTo(mcparticles);
    }
    if (pidlikelihoods.isOptional()) {
      particles.registerRelationTo(pidlikelihoods);
    }

    if (m_useMCParticles) {
      mcparticles.isRequired();
    }

    if (m_decayStringsWithCuts.empty()) {
      B2WARNING("Obsolete usage of the ParticleLoader module (load all MDST objects as all possible Particle object types). Specify the particle type via decayStringsWithCuts module parameter instead.");
    } else {
      for (auto decayStringCutTuple : m_decayStringsWithCuts) {

        // parsing of the input tuple (DecayString, Cut)
        string decayString = get<0>(decayStringCutTuple);
        std::string cutParameter = get<1>(decayStringCutTuple);

        // obtain the output particle lists from the decay string
        bool valid = m_decaydescriptor.init(decayString);
        if (!valid)
          B2ERROR("ParticleLoaderModule::initialize Invalid input DecayString: " << decayString);

        int nProducts = m_decaydescriptor.getNDaughters();
        if (nProducts > 0)
          B2ERROR("ParticleLoaderModule::initialize Invalid input DecayString " << decayString
                  << ". DecayString should not contain any daughters, only the mother particle.");

        // Mother particle
        const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();

        int pdgCode  = mother->getPDGCode();
        string listName = mother->getFullName();

        if (not isValidPDGCode(pdgCode) and m_useMCParticles == false)
          B2ERROR("Invalid particle type requested to be loaded. Set a valid decayString module parameter.");

        string antiListName = ParticleListName::antiParticleListName(listName);
        bool isSelfConjugatedParticle = (listName == antiListName);

        StoreObjPtr<ParticleList> particleList(listName);
        if (!particleList.isOptional()) {
          //if it doesn't exist:

          DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
          particleList.registerInDataStore(flags);
          if (!isSelfConjugatedParticle) {
            StoreObjPtr<ParticleList> antiParticleList(antiListName);
            antiParticleList.registerInDataStore(flags);
          }
        } else {
          // TODO: test that this actually works
          B2WARNING("Tha ParticleList with name " << listName << " already exists in the DataStore. Nothing to do.");
          continue;
        }

        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(cutParameter));

        // add PList to corresponding collection of Lists
        B2INFO(" o) creating (anti-)ParticleList with name: " << listName << " (" << antiListName << ")");
        if (m_useMCParticles) {
          B2INFO("   -> MDST source: MCParticles");
          m_MCParticles2Plists.push_back(make_tuple(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut));
        } else {
          bool chargedFSP = Const::chargedStableSet.contains(Const::ParticleType(abs(pdgCode)));
          if (chargedFSP) {
            B2INFO("   -> MDST source: Tracks");
            m_Tracks2Plists.push_back(make_tuple(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut));
          }

          if (abs(pdgCode) == abs(Const::photon.getPDGCode())) {
            if (m_addDaughters == false) {
              B2INFO("   -> MDST source: ECLClusters");
              m_ECLClusters2Plists.push_back(make_tuple(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut));
            } else {
              B2INFO("   -> MDST source: V0 (-> TFR(e) + TFR(e))");
              m_V02Plists.push_back(make_tuple(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut));
            }
          }

          if (abs(pdgCode) == abs(Const::Kshort.getPDGCode())) {
            B2INFO("   -> MDST source: V0 (-> TFR(pi) + TFR(pi))");
            m_V02Plists.push_back(make_tuple(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut));
          }

          if (abs(pdgCode) == abs(Const::Klong.getPDGCode())) {
            B2INFO("   -> MDST source: KLMClusters");
            m_KLMClusters2Plists.push_back(make_tuple(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut));
          }

          if (abs(pdgCode) == abs(Const::Lambda.getPDGCode())) {
            B2INFO("   -> MDST source: V0 (-> TFR(p) + TFR(pi))");
            m_V02Plists.push_back(make_tuple(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut));
          }
        }
        B2INFO("   -> With cuts  : " << cutParameter);
      }
    }
  }

  void ParticleLoaderModule::event()
  {
    StoreArray<Particle> particles;
    StoreObjPtr<ParticleExtraInfoMap> particleExtraInfoMap;
    if (not particleExtraInfoMap) {
      particleExtraInfoMap.create();
    }


    if (m_useMCParticles)
      mcParticlesToParticles();
    else {
      tracksToParticles();
      eclClustersToParticles();
      klmClustersToParticles();
      v0sToParticles();
    }
  }

  void ParticleLoaderModule::v0sToParticles()
  {
    if (m_V02Plists.empty()) // nothing to do
      return;

    // create all lists
    for (auto v02Plist : m_V02Plists) {
      string listName = get<c_PListName>(v02Plist);
      string antiListName = get<c_AntiPListName>(v02Plist);
      int pdgCode = get<c_PListPDGCode>(v02Plist);
      bool isSelfConjugatedParticle = get<c_IsPListSelfConjugated>(v02Plist);

      StoreObjPtr<ParticleList> plist(listName);
      plist.create();
      plist->initialize(pdgCode, listName);

      if (!isSelfConjugatedParticle) {
        StoreObjPtr<ParticleList> antiPlist(antiListName);
        antiPlist.create();
        antiPlist->initialize(-1 * pdgCode, antiListName);

        antiPlist->bindAntiParticleList(*(plist));
      }
    }

    StoreArray<V0> V0s;
    StoreArray<Particle> particles;

    // load reconstructed V0s as Kshorts (pi-pi+ combination), Lambdas (p+pi- combinations), and converted photons (e-e+ combinations)
    for (int i = 0; i < V0s.getEntries(); i++) {
      const V0* v0 = V0s[i];
      Const::ParticleType v0Type = v0->getV0Hypothesis();

      for (auto v02Plist : m_V02Plists) {
        int listPDGCode = get<c_PListPDGCode>(v02Plist);

        if (abs(listPDGCode) != abs(v0Type.getPDGCode()))
          continue;

        Const::ChargedStable pTypeP(Const::pion);
        Const::ChargedStable pTypeM(Const::pion);

        if (v0Type.getPDGCode() == Const::Kshort.getPDGCode()) { // K0s -> pi+ pi-
          pTypeP = Const::pion;
          pTypeM = Const::pion;
        } else if (v0Type.getPDGCode() == Const::Lambda.getPDGCode()) { // Lambda -> p+ pi-
          pTypeP = Const::proton;
          pTypeM = Const::pion;
        } else if (v0Type.getPDGCode() == Const::antiLambda.getPDGCode()) { // anti-Lambda -> pi+ anti-p-
          pTypeP = Const::pion;
          pTypeM = Const::proton;
        } else if (v0Type.getPDGCode() == Const::photon.getPDGCode()) { // gamma -> e+ e-
          pTypeP = Const::electron;
          pTypeM = Const::electron;
        } else {
          B2WARNING("Unknown V0 hypothesis!");
        }

        std::pair<Track*, Track*> v0Tracks = v0->getTracks();
        std::pair<TrackFitResult*, TrackFitResult*> v0TrackFitResults = v0->getTrackFitResults();

        Particle daugP((v0Tracks.first)->getArrayIndex(), v0TrackFitResults.first, pTypeP, v0TrackFitResults.first->getParticleType());
        Particle daugM((v0Tracks.second)->getArrayIndex(), v0TrackFitResults.second, pTypeM, v0TrackFitResults.second->getParticleType());

        const PIDLikelihood* pidP = (v0Tracks.first)->getRelated<PIDLikelihood>();
        const PIDLikelihood* pidM = (v0Tracks.second)->getRelated<PIDLikelihood>();

        const MCParticle* mcParticleP = (v0Tracks.first)->getRelated<MCParticle>();
        const MCParticle* mcParticleM = (v0Tracks.second)->getRelated<MCParticle>();

        // add V0 daughters to the Particle StoreArray
        Particle* newDaugP = particles.appendNew(daugP);
        if (pidP)
          newDaugP->addRelationTo(pidP);
        if (mcParticleP)
          newDaugP->addRelationTo(mcParticleP);

        Particle* newDaugM = particles.appendNew(daugM);
        if (pidM)
          newDaugM->addRelationTo(pidM);
        if (mcParticleM)
          newDaugM->addRelationTo(mcParticleM);

        TLorentzVector v0Momentum = newDaugP->get4Vector() + newDaugM->get4Vector();

        Particle v0P(v0Momentum, v0Type.getPDGCode());
        v0P.appendDaughter(newDaugP);
        v0P.appendDaughter(newDaugM);

        Particle* newPart = particles.appendNew(v0P);

        string listName = get<c_PListName>(v02Plist);
        auto& cut = get<c_CutPointer>(v02Plist);
        StoreObjPtr<ParticleList> plist(listName);

        if (cut->check(newPart))
          plist->addParticle(newPart);
      }
    }
  }

  void ParticleLoaderModule::tracksToParticles()
  {
    if (m_Tracks2Plists.empty()) // nothing to do
      return;

    // create all lists
    for (auto track2Plist : m_Tracks2Plists) {
      string listName = get<c_PListName>(track2Plist);
      string antiListName = get<c_AntiPListName>(track2Plist);
      int pdgCode = get<c_PListPDGCode>(track2Plist);
      bool isSelfConjugatedParticle = get<c_IsPListSelfConjugated>(track2Plist);

      StoreObjPtr<ParticleList> plist(listName);
      plist.create();
      plist->initialize(pdgCode, listName);

      if (!isSelfConjugatedParticle) {
        StoreObjPtr<ParticleList> antiPlist(antiListName);
        antiPlist.create();
        antiPlist->initialize(-1 * pdgCode, antiListName);

        antiPlist->bindAntiParticleList(*(plist));
      }
    }

    StoreArray<Track> Tracks;
    StoreArray<Particle> particles;

    for (int i = 0; i < Tracks.getEntries(); i++) {
      const Track* track = Tracks[i];
      const PIDLikelihood* pid = track->getRelated<PIDLikelihood>();
      const auto& mcParticleWithWeight = track->getRelatedToWithWeight<MCParticle>();

      for (auto track2Plist : m_Tracks2Plists) {
        string listName = get<c_PListName>(track2Plist);
        auto& cut = get<c_CutPointer>(track2Plist);
        StoreObjPtr<ParticleList> plist(listName);

        //if no track hypothesis is requested, use the particle's own
        int pdgCode;
        if (m_trackHypothesis == 0)
          pdgCode = get<c_PListPDGCode>(track2Plist);
        else pdgCode = m_trackHypothesis;
        Const::ChargedStable type(abs(pdgCode));

        // load the TrackFitResult for the requested particle or if not available use
        // the one with the closest mass
        const TrackFitResult* trackFit = track->getTrackFitResultWithClosestMass(type);

        if (!trackFit) {
          B2WARNING("Track returned null TrackFitResult pointer for ChargedStable::getPDGCode()  = " << type.getPDGCode());
          continue;
        }

        if (m_enforceFitHypothesis && (trackFit->getParticleType().getPDGCode() != type.getPDGCode())) {
          // the required hypothesis does not exist for this track, skip it
          continue;
        }

        int charge = trackFit->getChargeSign();
        if (charge == 0) {
          B2WARNING("Track with charge = 0 skipped!");
          continue;
        }

        // create particle and add it to the Particle list. The Particle class
        // internally also uses the getTrackFitResultWithClosestMass() to load the best available
        // track fit result
        Particle particle(track, type);
        if (particle.getParticleType() == Particle::c_Track) { // should always hold but...

          Particle* newPart = particles.appendNew(particle);
          if (pid)
            newPart->addRelationTo(pid);
          if (mcParticleWithWeight.first)
            newPart->addRelationTo(mcParticleWithWeight.first, mcParticleWithWeight.second);

          if (cut->check(newPart))
            plist->addParticle(newPart);
        }
      }
    }
  }

  void ParticleLoaderModule::eclClustersToParticles()
  {
    if (m_ECLClusters2Plists.empty()) // nothing to do
      return;

    // create all lists
    for (auto eclCluster2Plist : m_ECLClusters2Plists) {
      string listName = get<c_PListName>(eclCluster2Plist);
      string antiListName = get<c_AntiPListName>(eclCluster2Plist);
      int pdgCode = get<c_PListPDGCode>(eclCluster2Plist);
      bool isSelfConjugatedParticle = get<c_IsPListSelfConjugated>(eclCluster2Plist);

      StoreObjPtr<ParticleList> plist(listName);
      plist.create();
      plist->initialize(pdgCode, listName);

      if (!isSelfConjugatedParticle) {
        StoreObjPtr<ParticleList> antiPlist(antiListName);
        antiPlist.create();
        antiPlist->initialize(-1 * pdgCode, antiListName);

        antiPlist->bindAntiParticleList(*(plist));
      }
    }

    StoreArray<ECLCluster> ECLClusters;
    StoreArray<Particle> particles;
    StoreArray<MCParticle> mcParticles;

    for (int i = 0; i < ECLClusters.getEntries(); i++) {
      const ECLCluster* cluster      = ECLClusters[i];

      if (!cluster->isNeutral())
        continue;

      if (cluster->getHypothesisId() != ECLCluster::Hypothesis::c_nPhotons)
        continue;

      // const MCParticle* mcParticle = cluster->getRelated<MCParticle>();
      // ECLCluster can be matched to multiple MCParticles
      // order the relations by weights and set Particle -> multiple MCParticle relation
      // preserve the weight
      RelationVector<MCParticle> mcRelations = cluster->getRelationsTo<MCParticle>();
      // order relations bt weights
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

      // create Particle
      Particle particle(cluster);
      if (particle.getParticleType() == Particle::c_ECLCluster) { // should always hold but...
        Particle* newPart = particles.appendNew(particle);

        // set relation
        for (unsigned int j = 0; j < weightsAndIndices.size(); j++) {
          const MCParticle* relMCParticle = mcParticles[weightsAndIndices[j].first];
          double weight = weightsAndIndices[j].second;

          // TODO: study this further and avoid hardcoded values
          // set the relation only if the MCParticle's energy contribution
          // to this cluster ammounts to at least 25%
          if (relMCParticle)
            if (weight / newPart->getEnergy() > 0.20 &&  weight / relMCParticle->getEnergy() > 0.30)
              newPart->addRelationTo(relMCParticle, weight);
        }

        // old way (to be removed)
        //if (mcParticle)
        //newPart->addRelationTo(mcParticle);

        // add particle to list if it passes the selection criteria
        for (auto eclCluster2Plist : m_ECLClusters2Plists) {
          string listName = get<c_PListName>(eclCluster2Plist);
          auto& cut = get<c_CutPointer>(eclCluster2Plist);
          StoreObjPtr<ParticleList> plist(listName);

          if (cut->check(newPart))
            plist->addParticle(newPart);
        }
      }
    }
  }

  void ParticleLoaderModule::klmClustersToParticles()
  {
    if (m_KLMClusters2Plists.empty()) // nothing to do
      return;

    // create all lists
    for (auto klmCluster2Plist : m_KLMClusters2Plists) {
      string listName = get<c_PListName>(klmCluster2Plist);
      string antiListName = get<c_AntiPListName>(klmCluster2Plist);
      int pdgCode = get<c_PListPDGCode>(klmCluster2Plist);
      bool isSelfConjugatedParticle = get<c_IsPListSelfConjugated>(klmCluster2Plist);

      StoreObjPtr<ParticleList> plist(listName);
      plist.create();
      plist->initialize(pdgCode, listName);

      if (!isSelfConjugatedParticle) {
        StoreObjPtr<ParticleList> antiPlist(antiListName);
        antiPlist.create();
        antiPlist->initialize(-1 * pdgCode, antiListName);

        antiPlist->bindAntiParticleList(*(plist));
      }
    }

    StoreArray<KLMCluster> KLMClusters;
    StoreArray<Particle> particles;

    // load reconstructed neutral KLM cluster's as Klongs
    for (int i = 0; i < KLMClusters.getEntries(); i++) {
      const KLMCluster* cluster      = KLMClusters[i];

      if (cluster->getAssociatedTrackFlag())
        continue;

      const MCParticle* mcParticle = cluster->getRelated<MCParticle>();

      Particle particle(cluster);

      if (particle.getParticleType() == Particle::c_KLMCluster) { // should always hold but...
        Particle* newPart = particles.appendNew(particle);

        if (mcParticle)
          newPart->addRelationTo(mcParticle);

        // add particle to list if it passes the selection criteria
        for (auto klmCluster2Plist : m_KLMClusters2Plists) {
          string listName = get<c_PListName>(klmCluster2Plist);
          auto&  cut = get<c_CutPointer>(klmCluster2Plist);
          StoreObjPtr<ParticleList> plist(listName);

          if (cut->check(newPart))
            plist->addParticle(newPart);
        }
      }
    }
  }

  void ParticleLoaderModule::mcParticlesToParticles()
  {
    if (m_MCParticles2Plists.empty()) // nothing to do
      return;

    // create all lists
    for (auto mcParticle2Plist : m_MCParticles2Plists) {
      string listName = get<c_PListName>(mcParticle2Plist);
      string antiListName = get<c_AntiPListName>(mcParticle2Plist);
      int pdgCode = get<c_PListPDGCode>(mcParticle2Plist);
      bool isSelfConjugatedParticle = get<c_IsPListSelfConjugated>(mcParticle2Plist);

      StoreObjPtr<ParticleList> plist(listName);
      plist.create();
      plist->initialize(pdgCode, listName);

      if (!isSelfConjugatedParticle) {
        StoreObjPtr<ParticleList> antiPlist(antiListName);
        antiPlist.create();
        antiPlist->initialize(-1 * pdgCode, antiListName);

        antiPlist->bindAntiParticleList(*(plist));
      }
    }

    StoreArray<MCParticle> MCParticles;
    StoreArray<Particle> particles;

    for (int i = 0; i < MCParticles.getEntries(); i++) {
      const MCParticle* mcParticle = MCParticles[i];

      for (auto mcParticle2Plist : m_MCParticles2Plists) {
        int pdgCode = get<c_PListPDGCode>(mcParticle2Plist);

        if (abs(pdgCode) != abs(mcParticle->getPDG()))
          continue;

        Particle particle(mcParticle);
        Particle* newPart = particles.appendNew(particle);
        newPart->addRelationTo(mcParticle);

        //append the whole bottom part of the decay tree to this particle
        if (m_addDaughters) appendDaughtersRecursive(newPart);

        string listName = get<c_PListName>(mcParticle2Plist);
        auto&  cut = get<c_CutPointer>(mcParticle2Plist);
        StoreObjPtr<ParticleList> plist(listName);

        if (cut->check(newPart))
          plist->addParticle(newPart);
      }
    }
  }

  bool ParticleLoaderModule::isValidPDGCode(const int pdgCode)
  {
    bool result = false;

    // is particle type = charged final state particle?
    if (Const::chargedStableSet.find(abs(pdgCode)) != Const::invalidParticle)
      return true;

    if (abs(pdgCode) == abs(Const::photon.getPDGCode()))
      return true;

    if (abs(pdgCode) == abs(Const::Kshort.getPDGCode()))
      return true;

    if (abs(pdgCode) == abs(Const::Klong.getPDGCode()))
      return true;

    if (abs(pdgCode) == abs(Const::Lambda.getPDGCode()))
      return true;

    return result;
  }

  void ParticleLoaderModule::appendDaughtersRecursive(Particle* mother)
  {
    StoreArray<Particle> particles;
    MCParticle* mcmother = mother->getRelated<MCParticle>();

    if (!mcmother)
      return;

    vector<MCParticle*> mcdaughters = mcmother->getDaughters();

    for (unsigned int i = 0; i < mcdaughters.size(); i++) {
      Particle particle(mcdaughters[i]);
      Particle* daughter = particles.appendNew(particle);
      daughter->addRelationTo(mcdaughters[i]);
      mother->appendDaughter(daughter);

      if (mcdaughters[i]->getNDaughters() > 0)
        appendDaughtersRecursive(daughter);
    }
  }


} // end Belle2 namespace

