/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <top/modules/TOPNtuple/TOPNtupleModule.h>

// TOP headers.
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/MCInitialParticles.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <top/dataobjects/TOPBarHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <tracking/dataobjects/ExtHit.h>

// ROOT
#include <TDirectory.h>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  ///                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPNtuple);

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPNtupleModule::TOPNtupleModule() : Module()
  {
    // set module description
    setDescription("Writes ntuple of TOPLikelihoods with tracking info into a root file");

    // Add parameters
    addParam("outputFileName", m_outputFileName, "Output file name",
             string("TOPNtuple.root"));

  }

  TOPNtupleModule::~TOPNtupleModule()
  {
  }

  void TOPNtupleModule::initialize()
  {
    TDirectory::TContext context;
    m_file = TFile::Open(m_outputFileName.c_str(), "RECREATE");
    if (m_file->IsZombie()) {
      B2FATAL("Couldn't open file '" << m_outputFileName << "' for writing!");
      return;
    }

    m_tree = new TTree("top", "TOP validation ntuple");

    m_tree->Branch("evt", &m_top.evt, "evt/I");
    m_tree->Branch("run", &m_top.run, "run/I");

    m_tree->Branch("p",   &m_top.p,   "p/F");
    m_tree->Branch("cth", &m_top.cth, "cth/F");
    m_tree->Branch("phi", &m_top.phi, "phi/F");
    m_tree->Branch("pValue", &m_top.pValue, "pValue/F");

    m_tree->Branch("PDG", &m_top.PDG, "PDG/I");
    m_tree->Branch("motherPDG", &m_top.motherPDG, "motherPDG/I");
    m_tree->Branch("primary", &m_top.primary, "primary/S");
    m_tree->Branch("seen", &m_top.seen, "seen/S");
    m_tree->Branch("rhoProd", &m_top.rhoProd, "rhoProd/F");
    m_tree->Branch("zProd",   &m_top.zProd,   "zProd/F");
    m_tree->Branch("phiProd", &m_top.phiProd, "phiProd/F");
    m_tree->Branch("rhoDec", &m_top.rhoDec, "rhoDec/F");
    m_tree->Branch("zDec",   &m_top.zDec,   "zDec/F");
    m_tree->Branch("phiDec", &m_top.phiDec, "phiDec/F");
    m_tree->Branch("yieldMC", &m_top.yieldMC, "yieldMC/I");

    m_tree->Branch("numPhot", &m_top.numPhot, "numPhot/I");
    m_tree->Branch("numBkg", &m_top.numBkg, "numBkg/F");
    m_tree->Branch("moduleID", &m_top.moduleID, "moduleID/I");
    m_tree->Branch("phot",  &m_top.phot,  "e/F:mu:pi:K:p:d");
    m_tree->Branch("yield",  &m_top.yield,  "e/F:mu:pi:K:p:d");
    m_tree->Branch("logL",  &m_top.logL,  "e/F:mu:pi:K:p:d");

    m_tree->Branch("extHit",  &m_top.extHit,  "moduleID/I:PDG:x/F:y:z:p:theta:phi:time");
    m_tree->Branch("barHit",  &m_top.barHit,  "moduleID/I:PDG:x/F:y:z:p:theta:phi:time");

    StoreArray<Track> tracks;
    tracks.isRequired();
    StoreArray<ExtHit> extHits;
    extHits.isRequired();
    StoreArray<TOPLikelihood> likelihoods;
    likelihoods.isRequired();
    StoreArray<MCParticle> mcParticles;
    mcParticles.isOptional();
    StoreArray<TOPBarHit> barHits;
    barHits.isOptional();
    StoreObjPtr<MCInitialParticles> mcInitialParticles;
    mcInitialParticles.isOptional();
  }

  void TOPNtupleModule::beginRun()
  {
  }

  void TOPNtupleModule::event()
  {

    StoreObjPtr<EventMetaData> evtMetaData;
    StoreArray<Track> tracks;
    StoreObjPtr<MCInitialParticles> mcInitialParticles;
    double trueEventT0 = 0;
    if (mcInitialParticles.isValid()) trueEventT0 = mcInitialParticles->getTime();

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    for (const auto& track : tracks) {
      const auto* trackFit = track.getTrackFitResultWithClosestMass(Const::pion);
      if (!trackFit) continue;
      const TOPLikelihood* top = track.getRelated<TOPLikelihood>();
      if (!top) continue;
      if (top->getFlag() != 1) continue;

      const ExtHit* extHit = top->getRelated<ExtHit>();
      const TOPBarHit* barHit = top->getRelated<TOPBarHit>();
      const MCParticle* mcParticle = track.getRelated<MCParticle>();
      const MCParticle* mother = 0;
      if (mcParticle) {
        mother = mcParticle->getMother();
        if (not barHit) { // Track MC matching probably done after TOPReconstructor so no relation from TOPLikelihood
          const auto barHits = mcParticle->getRelationsWith<TOPBarHit>();
          for (const auto& bHit : barHits) {
            if (bHit.getModuleID() == extHit->getCopyID()) barHit = &bHit;
          }
        }
      }

      m_top.clear();

      m_top.evt = evtMetaData->getEvent();
      m_top.run = evtMetaData->getRun();

      ROOT::Math::XYZVector mom = trackFit->getMomentum();
      m_top.p = mom.R();
      m_top.cth = cos(mom.Theta());
      m_top.phi = mom.Phi();
      m_top.pValue = trackFit->getPValue();

      if (mcParticle) {
        m_top.PDG = mcParticle->getPDG();
        if (mother) m_top.motherPDG = mother->getPDG();
        m_top.primary = mcParticle->getStatus(MCParticle::c_PrimaryParticle);
        m_top.seen = mcParticle->hasSeenInDetector(Const::TOP);
        ROOT::Math::XYZVector prodVertex = mcParticle->getProductionVertex();
        m_top.rhoProd = prodVertex.Rho();
        m_top.zProd = prodVertex.Z();
        m_top.phiProd = prodVertex.Phi();
        ROOT::Math::XYZVector decVertex = mcParticle->getDecayVertex();
        m_top.rhoDec = decVertex.Rho();
        m_top.zDec = decVertex.Z();
        m_top.phiDec = decVertex.Phi();
        const auto digits = mcParticle->getRelationsWith<TOPDigit>();
        for (size_t i = 0; i < digits.size(); i++) {
          double wt = digits.weight(i);
          if (wt <= 0) continue; // photon not from this MC particle
          const auto* digit = digits[i];
          if (digit->getHitQuality() != TOPDigit::c_Good) continue;
          if (digit->getModuleID() != top->getModuleID()) continue;
          if (digit->getTime() < TOPRecoManager::getMinTime() or digit->getTime() > TOPRecoManager::getMaxTime()) continue;
          m_top.yieldMC++;
        }
      }

      m_top.numPhot = top->getNphot();
      m_top.numBkg = top->getEstBkg();
      m_top.moduleID = top->getModuleID();

      m_top.phot.e  = top->getEstPhot(Const::electron);
      m_top.phot.mu = top->getEstPhot(Const::muon);
      m_top.phot.pi = top->getEstPhot(Const::pion);
      m_top.phot.K  =  top->getEstPhot(Const::kaon);
      m_top.phot.p  = top->getEstPhot(Const::proton);
      m_top.phot.d  = top->getEstPhot(Const::deuteron);

      m_top.yield.e  = top->getEffectiveSignalYield(Const::electron);
      m_top.yield.mu = top->getEffectiveSignalYield(Const::muon);
      m_top.yield.pi = top->getEffectiveSignalYield(Const::pion);
      m_top.yield.K  =  top->getEffectiveSignalYield(Const::kaon);
      m_top.yield.p  = top->getEffectiveSignalYield(Const::proton);
      m_top.yield.d  = top->getEffectiveSignalYield(Const::deuteron);

      m_top.logL.e  = top->getLogL(Const::electron);
      m_top.logL.mu = top->getLogL(Const::muon);
      m_top.logL.pi = top->getLogL(Const::pion);
      m_top.logL.K  = top->getLogL(Const::kaon);
      m_top.logL.p  = top->getLogL(Const::proton);
      m_top.logL.d  = top->getLogL(Const::deuteron);

      if (extHit) {
        int moduleID = extHit->getCopyID();
        auto position = static_cast<ROOT::Math::XYZPoint>(extHit->getPosition());
        auto momentum = extHit->getMomentum();
        if (geo->isModuleIDValid(moduleID)) {
          const auto& module = geo->getModule(moduleID);
          position = module.pointToLocal(position);
          momentum = module.momentumToLocal(momentum);
        }
        m_top.extHit.moduleID = moduleID;
        m_top.extHit.PDG = extHit->getPdgCode();
        m_top.extHit.x = position.X();
        m_top.extHit.y = position.Y();
        m_top.extHit.z = position.Z();
        m_top.extHit.p = momentum.R();
        m_top.extHit.theta = momentum.Theta();
        m_top.extHit.phi = momentum.Phi();
        m_top.extHit.time = extHit->getTOF();
      }

      if (barHit) {
        int moduleID = barHit->getModuleID();
        auto position = barHit->getPosition();
        auto momentum = barHit->getMomentum();
        if (geo->isModuleIDValid(moduleID)) {
          const auto& module = geo->getModule(moduleID);
          position = module.pointToLocal(position);
          momentum = module.momentumToLocal(momentum);
        }
        m_top.barHit.moduleID = moduleID;
        m_top.barHit.PDG = barHit->getPDG();
        m_top.barHit.x = position.X();
        m_top.barHit.y = position.Y();
        m_top.barHit.z = position.Z();
        m_top.barHit.p = momentum.R();
        m_top.barHit.theta = momentum.Theta();
        m_top.barHit.phi = momentum.Phi();
        m_top.barHit.time = barHit->getTime() - trueEventT0;
      }

      m_tree->Fill();
    }

  }


  void TOPNtupleModule::endRun()
  {
  }

  void TOPNtupleModule::terminate()
  {
    TDirectory::TContext context;
    m_file->cd();
    m_tree->Write();
    m_file->Close();
  }

} // end Belle2 namespace

