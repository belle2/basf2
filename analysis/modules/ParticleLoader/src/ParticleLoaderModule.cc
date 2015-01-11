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

// dataobjects
#include <mdst/dataobjects/V0.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>

// utilities
#include <analysis/utility/EvtPDLUtil.h>

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
    std::vector<std::tuple<std::string, Variable::Cut::Parameter>> emptyDecayStringsAndCuts;

    addParam("decayStringsWithCuts", m_decayStringsWithCuts,
             "List of (decayString, Variable::Cut) tuples that specify all output ParticleLists to be created by the module. Only Particles that pass specified selection criteria are added to the ParticleList (see https://belle2.cc.kek.jp/~twiki/bin/view/Physics/DecayString and https://belle2.cc.kek.jp/~twiki/bin/view/Physics/ParticleSelectorFunctions).", emptyDecayStringsAndCuts);

    addParam("useMCParticles", m_useMCParticles, "Use MCParticles instead of reconstructed MDST dataobjects (tracks, ECL, KLM, clusters, V0s, ...)", false);

    addParam("writeOut", m_writeOut,
             "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);
  }

  ParticleLoaderModule::~ParticleLoaderModule()
  {
  }

  void ParticleLoaderModule::terminate()
  {
    // clean-up the memory
    for (auto track2Plist : m_Tracks2Plists) {
      Variable::Cut* cut = get<c_CutPointer>(track2Plist);
      delete cut;
    }
    for (auto v02Plist : m_V02Plists) {
      Variable::Cut* cut = get<c_CutPointer>(v02Plist);
      delete cut;
    }
    for (auto eclCluster2Plist : m_ECLClusters2Plists) {
      Variable::Cut* cut = get<c_CutPointer>(eclCluster2Plist);
      delete cut;
    }
    for (auto klmCluster2Plist : m_KLMClusters2Plists) {
      Variable::Cut* cut = get<c_CutPointer>(klmCluster2Plist);
      delete cut;
    }
    for (auto mcParticle2Plist : m_MCParticles2Plists) {
      Variable::Cut* cut = get<c_CutPointer>(mcParticle2Plist);
      delete cut;
    }

  }

  void ParticleLoaderModule::initialize()
  {
    B2INFO("ParticleLoader's Summary of Actions:");

    StoreArray<Particle> particles;
    StoreArray<MCParticle> mcparticles;
    StoreArray<PIDLikelihood> pidlikelihoods;
    StoreObjPtr<ParticleExtraInfoMap> extraInfoMap;

    particles.registerInDataStore();
    extraInfoMap.registerInDataStore();
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
        Variable::Cut::Parameter cutParameter = get<1>(decayStringCutTuple);

        // obtain the output particle lists from the decay string
        bool valid = m_decaydescriptor.init(decayString);
        if (!valid)
          B2ERROR("ParticleLoaderModule::initialize Invalid input DecayString: " << decayString);

        int nProducts = m_decaydescriptor.getNDaughters();
        if (nProducts > 0)
          B2ERROR("ParticleSelectorModule::initialize Invalid input DecayString " << decayString
                  << ". DecayString should not contain any daughters, only the mother particle.");

        // Mother particle
        const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();

        int pdgCode  = mother->getPDGCode();
        string listName = mother->getFullName();

        if (not isValidPDGCode(pdgCode) and m_useMCParticles == false)
          B2ERROR("Invalid particle type requested to be loaded. Set a valid decayString module parameter.");

        bool isSelfConjugatedParticle = !(Belle2::EvtPDLUtil::hasAntiParticle(pdgCode));
        string antiListName             = Belle2::EvtPDLUtil::antiParticleListName(pdgCode, mother->getLabel());

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

        Variable::Cut* cut = new Variable::Cut(cutParameter);

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
            B2INFO("   -> MDST source: ECLClusters");
            m_ECLClusters2Plists.push_back(make_tuple(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut));
          }

          if (abs(pdgCode) == abs(Const::Kshort.getPDGCode())) {
            B2INFO("   -> MDST source: V0");
            m_V02Plists.push_back(make_tuple(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut));
          }

          if (abs(pdgCode) == abs(Const::Klong.getPDGCode())) {
            B2INFO("   -> MDST source: KLMClusters");
            m_KLMClusters2Plists.push_back(make_tuple(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut));
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
      // TODO: make it const once V0 dataobject is corrected (const qualifier properly applied)
      V0* v0 = V0s[i];

      std::pair<Track*, Track*> v0Tracks = v0->getTrackPtrs();
      std::pair<TrackFitResult*, TrackFitResult*> v0TrackFitResults = v0->getTrackFitResultPtrs();

      // load Kshort -> pi- pi+
      Particle piP((v0Tracks.first)->getArrayIndex(), v0TrackFitResults.first, Const::pion);
      Particle piM((v0Tracks.second)->getArrayIndex(), v0TrackFitResults.second, Const::pion);

      const PIDLikelihood* pidP = (v0Tracks.first)->getRelated<PIDLikelihood>();
      const PIDLikelihood* pidM = (v0Tracks.second)->getRelated<PIDLikelihood>();

      const MCParticle* mcParticleP = (v0Tracks.first)->getRelated<MCParticle>();
      const MCParticle* mcParticleM = (v0Tracks.second)->getRelated<MCParticle>();

      // add V0 daughters to the Particle StoreArray
      Particle* newPiP = particles.appendNew(piP);
      if (pidP)
        newPiP->addRelationTo(pidP);
      if (mcParticleP)
        newPiP->addRelationTo(mcParticleP);

      Particle* newPiM = particles.appendNew(piM);
      if (pidM)
        newPiM->addRelationTo(pidM);
      if (mcParticleM)
        newPiM->addRelationTo(mcParticleM);

      TLorentzVector v0Momentum = newPiP->get4Vector() + newPiM->get4Vector();

      // TODO: avoid hard-coded values
      Particle v0P(v0Momentum, Const::Kshort.getPDGCode());
      v0P.appendDaughter(newPiP);
      v0P.appendDaughter(newPiM);

      Particle* newPart = particles.appendNew(v0P);

      for (auto v02Plist : m_V02Plists) {
        int pdgCode = get<c_PListPDGCode>(v02Plist);
        if (abs(Const::Kshort.getPDGCode()) != abs(pdgCode))
          continue;

        string listName = get<c_PListName>(v02Plist);
        Variable::Cut* cut = get<c_CutPointer>(v02Plist);
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
      const MCParticle* mcParticle = track->getRelated<MCParticle>();

      for (auto track2Plist : m_Tracks2Plists) {
        string listName = get<c_PListName>(track2Plist);
        int pdgCode = get<c_PListPDGCode>(track2Plist);
        Variable::Cut* cut = get<c_CutPointer>(track2Plist);
        StoreObjPtr<ParticleList> plist(listName);

        Const::ChargedStable type(abs(pdgCode));
        Particle particle(track, type);
        if (particle.getParticleType() == Particle::c_Track) { // should always hold but...

          Particle* newPart = particles.appendNew(particle);
          if (pid)
            newPart->addRelationTo(pid);
          if (mcParticle)
            newPart->addRelationTo(mcParticle);

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

    for (int i = 0; i < ECLClusters.getEntries(); i++) {
      const ECLCluster* cluster      = ECLClusters[i];

      if (!cluster->isNeutral())
        continue;

      const MCParticle* mcParticle = cluster->getRelated<MCParticle>();

      Particle particle(cluster);
      if (particle.getParticleType() == Particle::c_ECLCluster) { // should always hold but...
        Particle* newPart = particles.appendNew(particle);

        if (mcParticle)
          newPart->addRelationTo(mcParticle);

        // add particle to list if it passes the selection criteria
        for (auto eclCluster2Plist : m_ECLClusters2Plists) {
          string listName = get<c_PListName>(eclCluster2Plist);
          Variable::Cut* cut = get<c_CutPointer>(eclCluster2Plist);
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
          Variable::Cut* cut = get<c_CutPointer>(klmCluster2Plist);
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

        string listName = get<c_PListName>(mcParticle2Plist);
        Variable::Cut* cut = get<c_CutPointer>(mcParticle2Plist);
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

    // TODO: Lambda0 is missing

    return result;
  }
} // end Belle2 namespace

