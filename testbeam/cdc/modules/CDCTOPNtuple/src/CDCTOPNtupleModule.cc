/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <testbeam/cdc/modules/CDCTOPNtuple/CDCTOPNtupleModule.h>
#include <top/geometry/TOPGeometryPar.h>



// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <framework/dataobjects/EventMetaData.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <top/dataobjects/TOPBarHit.h>
#include <tracking/dataobjects/ExtHit.h>
#include <reconstruction/dataobjects/CDCDedxTrack.h>

#include <TRandom3.h>


using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(CDCTOPNtuple)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  CDCTOPNtupleModule::CDCTOPNtupleModule() : Module()
  {
    // set module description
    setDescription("Makes ntuples for CDC-TOP cosmic tests analysis");

    // Add parameters
    addParam("outputFileName", m_outputFileName, "Output file name",
             string("CDCTOPNtuple.root"));


  }

  CDCTOPNtupleModule::~CDCTOPNtupleModule()
  {
  }

  void CDCTOPNtupleModule::initialize()
  {

    m_file = new TFile(m_outputFileName.c_str(), "RECREATE");
    if (m_file->IsZombie()) {
      B2FATAL("Couldn't open file '" << m_outputFileName << "' for writing!");
      return;
    }

    m_treeTop = new TTree("top", "TOP validation ntuple");

    m_treeTop->Branch("evt", &m_top.evt, "evt/I");
    m_treeTop->Branch("run", &m_top.run, "run/I");

    m_treeTop->Branch("p",   &m_top.p,   "p/F");
    m_treeTop->Branch("cth", &m_top.cth, "cth/F");
    m_treeTop->Branch("phi", &m_top.phi, "phi/F");
    m_treeTop->Branch("pValue", &m_top.pValue, "pValue/F");

    m_treeTop->Branch("PDG", &m_top.PDG, "PDG/I");
    m_treeTop->Branch("motherPDG", &m_top.motherPDG, "motherPDG/I");
    m_treeTop->Branch("primary", &m_top.primary, "primary/S");
    m_treeTop->Branch("seen", &m_top.seen, "seen/S");
    m_treeTop->Branch("rhoProd", &m_top.rhoProd, "rhoProd/F");
    m_treeTop->Branch("zProd",   &m_top.zProd,   "zProd/F");
    m_treeTop->Branch("phiProd", &m_top.phiProd, "phiProd/F");
    m_treeTop->Branch("rhoDec", &m_top.rhoDec, "rhoDec/F");
    m_treeTop->Branch("zDec",   &m_top.zDec,   "zDec/F");
    m_treeTop->Branch("phiDec", &m_top.phiDec, "phiDec/F");

    m_treeTop->Branch("numPhot", &m_top.numPhot, "numPhot/I");
    m_treeTop->Branch("numBkg", &m_top.numBkg, "numBkg/F");
    m_treeTop->Branch("phot",  &m_top.phot,  "e/F:mu:pi:K:p:d");
    m_treeTop->Branch("logL",  &m_top.logL,  "e/F:mu:pi:K:p:d");

    m_treeTop->Branch("extHit",  &m_top.extHit,  "moduleID/I:PDG:x/F:y:z:p:theta:phi:time");
    m_treeTop->Branch("barHit",  &m_top.barHit,  "moduleID/I:PDG:x/F:y:z:p:theta:phi:time");

    m_treeTop->Branch("dedx",   &m_dedx,   "dedx/F");
    m_treeTop->Branch("dedxErr",   &m_dedxErr,   "dedxErr/F");
    m_treeTop->Branch("Theta",   &m_theta,   "Theta/F");

    m_treePulls = new TTree("pulls", "TOP pulls");
    m_treePulls->Branch("cth", &m_top.cth, "cth/F");
    m_treePulls->Branch("phi", &m_top.phi, "phi/F");
    m_treePulls->Branch("pValue", &m_top.pValue, "pValue/F");
    m_treePulls->Branch("dedx",   &m_dedx,   "dedx/F");
    m_treePulls->Branch("Theta",   &m_theta,   "Theta/F");
    m_treePulls->Branch("x",  &m_top.extHit.x,  "x/F");
    m_treePulls->Branch("z",  &m_top.extHit.z,  "z/F");
    m_treePulls->Branch("pixel",  &m_pixelID,  "pixel/I");
    m_treePulls->Branch("t",  &m_t,  "t/F");
    m_treePulls->Branch("t0",  &m_t0,  "t0/F");
    m_treePulls->Branch("sigma",  &m_sigma,  "sigma/F");
    m_treePulls->Branch("phiCer",  &m_phiCer,  "phiCer/F");
    m_treePulls->Branch("wt",  &m_wt,  "wt/F");

    StoreArray<Track>::required();
    StoreArray<ExtHit>::required();
    StoreArray<TOPLikelihood>::required();
    StoreArray<MCParticle>::optional();
    StoreArray<TOPBarHit>::optional();

  }

  void CDCTOPNtupleModule::beginRun()
  {
  }

  void CDCTOPNtupleModule::event()
  {

    StoreObjPtr<EventMetaData> evtMetaData;
    StoreArray<Track> tracks;

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    if (tracks.getEntries() != 2) return; // TODO: uporabi tudi evente z vec kot 2 tracki

    std::vector<TVector3> trackMomentums;
    std::vector<float> dedxs;
    std::vector<float> dedxErrs;

    for (const auto& track : tracks) {
      const auto* trackFit = track.getTrackFitResult(Const::pion);
      if (!trackFit) continue;
      const auto* dedxTrack = track.getRelated<CDCDedxTrack>();
      if (!dedxTrack) continue;
      trackMomentums.push_back(trackFit->getMomentum());
      dedxs.push_back(dedxTrack->getTruncatedMean());
      dedxErrs.push_back(dedxTrack->getError());
    }
    if (trackMomentums.size() != 2) {
      B2WARNING("CDCTOPNtuple: missing TrackFitResult or CDCDedxTrack");
      return;
    }

    auto p1 = trackMomentums[0].Unit();
    auto p2 = trackMomentums[1].Unit();
    m_theta = acos(p1 * p2) * 1000; // mrad

    float errsq1 = dedxErrs[0] * dedxErrs[0];
    float errsq2 = dedxErrs[1] * dedxErrs[1];
    float errsq = 1.0 / (1.0 / errsq1 + 1.0 / errsq2);
    m_dedx = (dedxs[0] / errsq1 + dedxs[1] / errsq2) * errsq * 1e6; // keV/cm
    m_dedxErr = sqrt(errsq) * 1e6; // keV/cm

    for (const auto& track : tracks) {
      const TrackFitResult* trackFit = track.getTrackFitResult(Const::pion);
      if (!trackFit) continue;
      const TOPLikelihood* top = track.getRelated<TOPLikelihood>();
      if (!top) continue;
      if (top->getFlag() != 1) continue;

      const ExtHit* extHit = top->getRelated<ExtHit>();
      const TOPBarHit* barHit = top->getRelated<TOPBarHit>();
      const MCParticle* mcParticle = track.getRelated<MCParticle>();
      const MCParticle* mother = 0;
      if (mcParticle) mother = mcParticle->getMother();

      m_top.clear();

      m_top.evt = evtMetaData->getEvent();
      m_top.run = evtMetaData->getRun();

      TVector3 momentum = trackFit->getMomentum();
      m_top.p = momentum.Mag();
      m_top.cth = momentum.CosTheta();
      m_top.phi = momentum.Phi();
      m_top.pValue = trackFit->getPValue();

      if (mcParticle) {
        m_top.PDG = mcParticle->getPDG();
        if (mother) m_top.motherPDG = mother->getPDG();
        m_top.primary = mcParticle->getStatus(MCParticle::c_PrimaryParticle);
        m_top.seen = mcParticle->hasSeenInDetector(Const::TOP);
        TVector3 prodVertex = mcParticle->getProductionVertex();
        m_top.rhoProd = prodVertex.Perp();
        m_top.zProd = prodVertex.Z();
        m_top.phiProd = prodVertex.Phi();
        TVector3 decVertex = mcParticle->getDecayVertex();
        m_top.rhoDec = decVertex.Perp();
        m_top.zDec = decVertex.Z();
        m_top.phiDec = decVertex.Phi();
      }

      m_top.numPhot = top->getNphot();
      m_top.numBkg = top->getEstBkg();
      m_top.phot.e  = top->getEstPhot(Const::electron);
      m_top.phot.mu = top->getEstPhot(Const::muon);
      m_top.phot.pi = top->getEstPhot(Const::pion);
      m_top.phot.K  =  top->getEstPhot(Const::kaon);
      m_top.phot.p  = top->getEstPhot(Const::proton);
      m_top.phot.d  = top->getEstPhot(Const::deuteron);

      m_top.logL.e  = top->getLogL(Const::electron);
      m_top.logL.mu = top->getLogL(Const::muon);
      m_top.logL.pi = top->getLogL(Const::pion);
      m_top.logL.K  = top->getLogL(Const::kaon);
      m_top.logL.p  = top->getLogL(Const::proton);
      m_top.logL.d  = top->getLogL(Const::deuteron);

      if (extHit) {
        int moduleID = extHit->getCopyID();
        TVector3 position = extHit->getPosition();
        TVector3 momentum = extHit->getMomentum();
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
        m_top.extHit.p = momentum.Mag();
        m_top.extHit.theta = momentum.Theta();
        m_top.extHit.phi = momentum.Phi();
        m_top.extHit.time = extHit->getTOF();
      }

      if (barHit) {
        int moduleID = barHit->getModuleID();
        TVector3 position = barHit->getPosition();
        TVector3 momentum = barHit->getMomentum();
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
        m_top.barHit.p = momentum.Mag();
        m_top.barHit.theta = momentum.Theta();
        m_top.barHit.phi = momentum.Phi();
        m_top.barHit.time = barHit->getTime();
      }

      m_treeTop->Fill();

    }

  }


  void CDCTOPNtupleModule::endRun()
  {
  }

  void CDCTOPNtupleModule::terminate()
  {
    m_file->cd();
    m_treeTop->Write();
    m_treePulls->Write();
    m_file->Close();
  }


} // end Belle2 namespace

