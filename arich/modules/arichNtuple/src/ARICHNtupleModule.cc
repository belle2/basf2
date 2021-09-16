/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <arich/modules/arichNtuple/ARICHNtupleModule.h>

// Hit classes
#include <framework/dataobjects/EventMetaData.h>
#include <arich/dataobjects/ARICHLikelihood.h>
#include <arich/dataobjects/ARICHAeroHit.h>
#include <arich/dataobjects/ARICHTrack.h>
#include <arich/dataobjects/ARICHPhoton.h>
#include <arich/dataobjects/ARICHHit.h>
#include <arich/dataobjects/ARICHInfo.h>

// framework - DataStore
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/HitPatternCDC.h>

#ifdef ALIGNMENT_USING_BHABHA
#include <mdst/dataobjects/ECLCluster.h>
#endif

// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector3.h>
#include <vector>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ARICHNtuple)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ARICHNtupleModule::ARICHNtupleModule() : Module(),
    m_file(0),
    m_tree(0)
  {
    // set module description (e.g. insert text)
    setDescription("The module saves variables needed for performance analysis, such as postion and momentum of the hit, likelihoods for hypotheses and number of photons.");

    // Add parameters
    addParam("outputFile", m_outputFile, "ROOT output file name", string("ARICHNtuple.root"));
  }

  ARICHNtupleModule::~ARICHNtupleModule()
  {
  }

  void ARICHNtupleModule::initialize()
  {

    if (m_file) delete m_file;
    m_file = new TFile(m_outputFile.c_str(), "RECREATE");
    if (m_file->IsZombie()) {
      B2FATAL("Couldn't open file '" << m_outputFile << "' for writing!");
      return;
    }

    m_tree = new TTree("arich", "ARICH validation ntuple");

    m_tree->Branch("evt", &m_arich.evt, "evt/I");
    m_tree->Branch("run", &m_arich.run, "run/I");
    m_tree->Branch("exp", &m_arich.exp, "exp/I");

    m_tree->Branch("charge", &m_arich.charge, "charge/S");
    m_tree->Branch("pValue", &m_arich.pValue, "pValue/F");
    m_tree->Branch("d0", &m_arich.z0, "pValue/F");
    m_tree->Branch("z0", &m_arich.d0, "pValue/F");

#ifdef ALIGNMENT_USING_BHABHA
    m_tree->Branch("eop", &m_arich.eop, "eop/F");
    m_tree->Branch("e9e21", &m_arich.e9e21, "e9e21/F");
    m_tree->Branch("etot", &m_arich.etot, "etot/F");
#endif

    m_tree->Branch("PDG", &m_arich.PDG, "PDG/I");
    m_tree->Branch("motherPDG", &m_arich.motherPDG, "motherPDG/I");
    m_tree->Branch("primary", &m_arich.primary, "primary/S");
    m_tree->Branch("seen", &m_arich.seen, "seen/S");
    m_tree->Branch("scatter", &m_arich.scatter, "scatter/I");

    m_tree->Branch("rhoProd", &m_arich.rhoProd, "rhoProd/F");
    m_tree->Branch("zProd",   &m_arich.zProd,   "zProd/F");
    m_tree->Branch("phiProd", &m_arich.phiProd, "phiProd/F");
    m_tree->Branch("rhoDec", &m_arich.rhoDec, "rhoDec/F");
    m_tree->Branch("zDec",   &m_arich.zDec,   "zDec/F");
    m_tree->Branch("phiDec", &m_arich.phiDec, "phiDec/F");
    m_tree->Branch("status", &m_arich.status, "status/I");

    m_tree->Branch("detPhot",  &m_arich.detPhot,  "detPhot/I");
    m_tree->Branch("numBkg",  &m_arich.numBkg,  "e/F:mu:pi:K:p:d");
    m_tree->Branch("expPhot",  &m_arich.expPhot,  "e/F:mu:pi:K:p:d");
    m_tree->Branch("logL",  &m_arich.logL,  "e/F:mu:pi:K:p:d");

    m_tree->Branch("recHit",  &m_arich.recHit,  "PDG/I:x/F:y:z:p:theta:phi");
    m_tree->Branch("mcHit",  &m_arich.mcHit,  "PDG/I:x/F:y:z:p:theta:phi");
    m_tree->Branch("winHit",  &m_arich.winHit,  "x/F:y");
    m_tree->Branch("nrec", &m_arich.nRec, "nRec/I");
    m_tree->Branch("nCDC", &m_arich.nCDC, "nCDC/I");
    m_tree->Branch("inAcceptance", &m_arich.inAcc, "inAcc/O");
    m_tree->Branch("photons", "std::vector<Belle2::ARICHPhoton>", &m_arich.photons);

    // required input
    m_arichTracks.isRequired();
    m_arichLikelihoods.isRequired();

    // optional input
    m_tracks.isOptional();
    m_arichMCPs.isOptional();
    m_arichAeroHits.isOptional();
    m_arichInfo.isOptional();

    StoreArray<ARICHHit> arichHits;
    arichHits.isRequired();

  }

  void ARICHNtupleModule::beginRun()
  {
  }

  void ARICHNtupleModule::event()
  {

    StoreObjPtr<EventMetaData> evtMetaData;
    StoreArray<ARICHHit> arichHits;

    if (!m_arichTracks.isValid()) return;

#ifdef ALIGNMENT_USING_BHABHA
    double E_tot = 0.;
    StoreArray<ECLCluster> eclClusters;
    for (const auto& trk : m_tracks) {
      const ECLCluster* eclTrack = trk.getRelated<ECLCluster>();
      if (eclTrack) {
        if (eclTrack->getEnergy() > 0.1) E_tot += eclTrack->getEnergy();
      }
    }
    for (const auto& cluster : eclClusters) {
      if (!cluster.isNeutral()) continue;
      if (cluster.getHypothesisId() != 5 && cluster.getHypothesisId() != 1)
        continue;
      if (cluster.getEnergy() > 0.1) E_tot += cluster.getEnergy();
    }
#endif

    for (const auto& arichTrack : m_arichTracks) {

      const ExtHit* extHit = arichTrack.getRelated<ExtHit>();

      const Track* mdstTrack = NULL;
      if (extHit) mdstTrack = extHit->getRelated<Track>();

      const ARICHAeroHit* aeroHit = arichTrack.getRelated<ARICHAeroHit>();

      const ARICHLikelihood* lkh = NULL;
      if (mdstTrack) lkh = mdstTrack->getRelated<ARICHLikelihood>();
      else lkh = arichTrack.getRelated<ARICHLikelihood>();

      if (!lkh) continue;

      m_arich.clear();

      m_arich.evt = evtMetaData->getEvent();
      m_arich.run = evtMetaData->getRun();
      m_arich.exp = evtMetaData->getExperiment();

      // set hapd window hit if available
      if (arichTrack.hitsWindow()) {
        TVector2 winHit = arichTrack.windowHitPosition();
        m_arich.winHit[0] = winHit.X();
        m_arich.winHit[1] = winHit.Y();
      }

      if (lkh->getFlag() == 1) m_arich.inAcc = 1;
      m_arich.logL.e = lkh->getLogL(Const::electron);
      m_arich.logL.mu = lkh->getLogL(Const::muon);
      m_arich.logL.pi = lkh->getLogL(Const::pion);
      m_arich.logL.K = lkh->getLogL(Const::kaon);
      m_arich.logL.p = lkh->getLogL(Const::proton);
      m_arich.logL.d = lkh->getLogL(Const::deuteron);

      m_arich.expPhot.e = lkh->getExpPhot(Const::electron);
      m_arich.expPhot.mu = lkh->getExpPhot(Const::muon);
      m_arich.expPhot.pi = lkh->getExpPhot(Const::pion);
      m_arich.expPhot.K = lkh->getExpPhot(Const::kaon);
      m_arich.expPhot.p = lkh->getExpPhot(Const::proton);
      m_arich.expPhot.d = lkh->getExpPhot(Const::deuteron);

      m_arich.detPhot = lkh->getDetPhot();

#ifdef ALIGNMENT_USING_BHABHA
      m_arich.etot = E_tot;
#endif
      m_arich.status = 1;

      m_arich.trgtype = 0;
      if (m_arichInfo.getEntries() > 0) {
        auto arichInfo = *m_arichInfo[0];
        m_arich.trgtype = arichInfo.gettrgtype();
      }

      const MCParticle* particle = 0;

      if (mdstTrack) {
        const TrackFitResult* fitResult = mdstTrack->getTrackFitResultWithClosestMass(Const::pion);
        if (fitResult) {
          m_arich.pValue = fitResult->getPValue();
          TVector3 trkPos = fitResult->getPosition();
          m_arich.charge = fitResult->getChargeSign();
          m_arich.z0 = trkPos.Z();
          m_arich.d0 = (trkPos.XYvector()).Mod();
          m_arich.nCDC = fitResult->getHitPatternCDC().getNHits();
#ifdef ALIGNMENT_USING_BHABHA
          TVector3 trkMom = fitResult->getMomentum();
          const ECLCluster* eclTrack = mdstTrack->getRelated<ECLCluster>();
          if (eclTrack) {
            m_arich.eop = eclTrack->getEnergy() / trkMom.Mag();
            m_arich.e9e21 = eclTrack->getE9oE21();
          }
#endif
        }
        m_arich.status += 10;
        int fromARICHMCP = 0;
        particle = mdstTrack->getRelated<MCParticle>();
        if (!particle) { particle = mdstTrack->getRelated<MCParticle>("arichMCParticles"); fromARICHMCP = 1;}
        if (particle) {
          m_arich.PDG = particle->getPDG();
          m_arich.primary = particle->getStatus(MCParticle::c_PrimaryParticle);
          m_arich.seen = particle->hasSeenInDetector(Const::ARICH);
          B2Vector3D prodVertex = particle->getProductionVertex();
          m_arich.rhoProd = prodVertex.Perp();
          m_arich.zProd = prodVertex.Z();
          m_arich.phiProd = prodVertex.Phi();
          TVector3 decVertex = particle->getDecayVertex();
          m_arich.rhoDec = decVertex.Perp();
          m_arich.zDec = decVertex.Z();
          m_arich.phiDec = decVertex.Phi();

          if (!fromARICHMCP) {
            MCParticle* mother = particle->getMother();
            if (mother) m_arich.motherPDG = mother->getPDG();
            std::vector<Belle2::MCParticle*> daughs =  particle->getDaughters();
            for (const auto daugh : daughs) {
              if (daugh == NULL) continue;
              if (daugh->getPDG() == particle->getPDG()) m_arich.scatter = 1;
            }
          }
        }
      }


      // get reconstructed photons associated with track
      const std::vector<ARICHPhoton>& photons = arichTrack.getPhotons();
      m_arich.nRec = photons.size();
      int nphot = 0;
      for (auto it = photons.begin(); it != photons.end(); ++it) {
        ARICHPhoton iph = *it;
        if (iph.getHitID() < arichHits.getEntries()) {
          auto ihit = *arichHits[iph.getHitID()];
          int module  = ihit.getModule();
          int channel = ihit.getChannel();
          int chid = 1 << 16 | module << 8 | channel;
          iph.setHitID(chid);
        }
        m_arich.photons.push_back(iph);
        nphot++;
      }

      TVector3 recPos = arichTrack.getPosition();
      m_arich.recHit.x = recPos.X();
      m_arich.recHit.y = recPos.Y();
      m_arich.recHit.z = recPos.Z();

      TVector3 recMom = arichTrack.getDirection() * arichTrack.getMomentum();
      m_arich.recHit.p = recMom.Mag();
      m_arich.recHit.theta = recMom.Theta();
      m_arich.recHit.phi = recMom.Phi();

      if (aeroHit) {
        TVector3 truePos = aeroHit->getPosition();
        m_arich.mcHit.x = truePos.X();
        m_arich.mcHit.y = truePos.Y();
        m_arich.mcHit.z = truePos.Z();

        TVector3 trueMom = aeroHit->getMomentum();
        m_arich.mcHit.p = trueMom.Mag();
        m_arich.mcHit.theta = trueMom.Theta();
        m_arich.mcHit.phi = trueMom.Phi();

        m_arich.mcHit.PDG = aeroHit->getPDG();
        m_arich.status += 1000;

        if (!particle) {
          particle = aeroHit->getRelated<MCParticle>();
          if (particle) {
            m_arich.PDG = particle->getPDG();
            MCParticle* mother = particle->getMother();
            if (mother) m_arich.motherPDG = mother->getPDG();
            m_arich.primary = particle->getStatus(MCParticle::c_PrimaryParticle);
            m_arich.seen = particle->hasSeenInDetector(Const::ARICH);
            B2Vector3D prodVertex = particle->getProductionVertex();
            m_arich.rhoProd = prodVertex.Perp();
            m_arich.zProd = prodVertex.Z();
            m_arich.phiProd = prodVertex.Phi();
            TVector3 decVertex = particle->getDecayVertex();
            m_arich.rhoDec = decVertex.Perp();
            m_arich.zDec = decVertex.Z();
            m_arich.phiDec = decVertex.Phi();

            std::vector<Belle2::MCParticle*> daughs =  particle->getDaughters();
            for (const auto daugh : daughs) {
              if (daugh->getPDG() == particle->getPDG()) m_arich.scatter = 1;
            }
          }
        }
      }
      m_tree->Fill();
    }
  }



  void ARICHNtupleModule::endRun()
  {
  }

  void ARICHNtupleModule::terminate()
  {
    m_file->cd();
    m_tree->Write();
    m_file->Close();
  }

} // end Belle2 namespace

