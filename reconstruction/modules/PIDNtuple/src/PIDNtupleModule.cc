/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Jake Bennett                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <reconstruction/modules/PIDNtuple/PIDNtupleModule.h>

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
#include <mdst/dataobjects/PIDLikelihood.h>

// ROOT
#include <TRandom3.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PIDNtuple)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PIDNtupleModule::PIDNtupleModule() : Module(),
  m_norm(0), m_value(0), m_file(0), m_tree(0)

{
  // set module description (e.g. insert text)
  setDescription("Writes a flat ntuple of PIDLikelihoods with track info into a root file");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Add parameters
  addParam("outputFileName", m_outputFileName, "Output file name",
           std::string("PIDNtuple.root"));
  addParam("makeFlat", m_makeFlat, "make momentum distribution flat up to pMax", false);
  addParam("p1", m_p1, "parameter of momentum distribution", 0.90311E-01);
  addParam("p2", m_p2, "parameter of momentum distribution", 0.56846);
  addParam("pMax", m_pMax, "make distribution flat up to this momentum", 3.0);
}

PIDNtupleModule::~PIDNtupleModule() { }

void PIDNtupleModule::initialize()
{

  // required inputs
  StoreArray<Track> tracks;
  StoreArray<TrackFitResult> trackfitResults;
  StoreArray<PIDLikelihood> PIDLikelihoods;

  tracks.isRequired();
  trackfitResults.isRequired();
  PIDLikelihoods.isRequired();

  // optional inputs
  StoreArray<MCParticle> mcparticles;
  mcparticles.isOptional();
  tracks.optionalRelationTo(mcparticles);

  // Create and book the ROOT file and TTree
  m_file = new TFile(m_outputFileName.c_str(), "RECREATE");
  m_tree = new TTree("pid", "PID tree");

  m_tree->Branch("evt", &m_pid.evt, "evt/I");
  m_tree->Branch("run", &m_pid.run, "run/I");
  m_tree->Branch("p",   &m_pid.p,   "p/F");
  m_tree->Branch("cth", &m_pid.cth, "cth/F");
  m_tree->Branch("phi", &m_pid.phi, "phi/F");
  m_tree->Branch("pValue", &m_pid.pValue, "pValue/F");
  m_tree->Branch("PDG", &m_pid.PDG, "PDG/I");
  m_tree->Branch("motherPDG", &m_pid.motherPDG, "motherPDG/I");
  m_tree->Branch("primary", &m_pid.primary, "primary/S");
  m_tree->Branch("rhoProd", &m_pid.rhoProd, "rhoProd/F");
  m_tree->Branch("zProd",   &m_pid.zProd,   "zProd/F");
  m_tree->Branch("phiProd", &m_pid.phiProd, "phiProd/F");
  m_tree->Branch("rhoDec", &m_pid.rhoDec, "rhoDec/F");
  m_tree->Branch("zDec",   &m_pid.zDec,   "zDec/F");
  m_tree->Branch("phiDec", &m_pid.phiDec, "phiDec/F");
  m_tree->Branch("cdcdedx",  &m_pid.cdcdedx,  "le/F:lmu:lpi:lk:lp:flag/S:seen/S");
  m_tree->Branch("svddedx",  &m_pid.svddedx,  "le/F:lmu:lpi:lk:lp:flag/S:seen/S");
  m_tree->Branch("top",   &m_pid.top,   "le/F:lmu:lpi:lk:lp:flag/S:seen/S");
  m_tree->Branch("arich", &m_pid.arich, "le/F:lmu:lpi:lk:lp:flag/S:seen/S");
  m_tree->Branch("ecl",   &m_pid.ecl,   "le/F:lmu:lpi:lk:lp:flag/S:seen/S");
  m_tree->Branch("klm",   &m_pid.klm,   "le/F:lmu:lpi:lk:lp:flag/S:seen/S");

  m_norm = 1;
  double p0 = m_p1 * log(1.0 + 4.0 * m_p2 / m_p1); // distribution peak
  m_norm = 1.0 / momDistribution(p0);
  m_value = momDistribution(m_pMax);
}

void PIDNtupleModule::beginRun()
{
}

void PIDNtupleModule::event()
{

  StoreObjPtr<EventMetaData> evtMetaData;
  StoreArray<Track> tracks;

  // loop over tracks
  for (const auto& track : tracks) {

    const TrackFitResult* trackFit = track.getTrackFitResultWithClosestMass(Const::pion);
    if (!trackFit) {
      B2WARNING("No track fit result... Skipping.");
      continue;
    }

    const PIDLikelihood* pid = track.getRelated<PIDLikelihood>();
    if (!pid) {
      B2WARNING("No track fit result... Skipping.");
      continue;
    }

    const MCParticle* mcParticle = track.getRelated<MCParticle>();
    const MCParticle* mother = 0;
    if (mcParticle) mother = mcParticle->getMother();

    m_pid.clear();
    m_pid.evt = evtMetaData->getEvent();
    m_pid.run = evtMetaData->getRun();
    TVector3 momentum = trackFit->getMomentum();
    m_pid.p = momentum.Mag();
    if (m_makeFlat) {
      if (gRandom->Rndm() * momDistribution(m_pid.p) > m_value) continue;
    }
    m_pid.cth = momentum.CosTheta();
    m_pid.phi = momentum.Phi();
    m_pid.pValue = trackFit->getPValue();
    if (mcParticle) {
      m_pid.PDG = mcParticle->getPDG();
      if (mother) m_pid.motherPDG = mother->getPDG();
      m_pid.primary = mcParticle->getStatus(MCParticle::c_PrimaryParticle);
      TVector3 prodVertex = mcParticle->getProductionVertex();
      m_pid.rhoProd = prodVertex.Perp();
      m_pid.zProd = prodVertex.Z();
      m_pid.phiProd = prodVertex.Phi();
      TVector3 decVertex = mcParticle->getDecayVertex();
      m_pid.rhoDec = decVertex.Perp();
      m_pid.zDec = decVertex.Z();
      m_pid.phiDec = decVertex.Phi();
    }

    m_pid.cdcdedx.le = pid->getLogL(Const::electron, Const::CDC);
    m_pid.cdcdedx.lmu = pid->getLogL(Const::muon, Const::CDC);
    m_pid.cdcdedx.lpi = pid->getLogL(Const::pion, Const::CDC);
    m_pid.cdcdedx.lk = pid->getLogL(Const::kaon, Const::CDC);
    m_pid.cdcdedx.lp = pid->getLogL(Const::proton, Const::CDC);
    m_pid.cdcdedx.flag = pid->isAvailable(Const::CDC);
    if (mcParticle)
      m_pid.cdcdedx.seen = mcParticle->hasSeenInDetector(Const::CDC);

    m_pid.svddedx.le = pid->getLogL(Const::electron, Const::SVD);
    m_pid.svddedx.lmu = pid->getLogL(Const::muon, Const::SVD);
    m_pid.svddedx.lpi = pid->getLogL(Const::pion, Const::SVD);
    m_pid.svddedx.lk = pid->getLogL(Const::kaon, Const::SVD);
    m_pid.svddedx.lp = pid->getLogL(Const::proton, Const::SVD);
    m_pid.svddedx.flag = pid->isAvailable(Const::SVD);
    if (mcParticle)
      m_pid.svddedx.seen = mcParticle->hasSeenInDetector(Const::SVD);

    m_pid.top.le = pid->getLogL(Const::electron, Const::TOP);
    m_pid.top.lmu = pid->getLogL(Const::muon, Const::TOP);
    m_pid.top.lpi = pid->getLogL(Const::pion, Const::TOP);
    m_pid.top.lk = pid->getLogL(Const::kaon, Const::TOP);
    m_pid.top.lp = pid->getLogL(Const::proton, Const::TOP);
    m_pid.top.flag = pid->isAvailable(Const::TOP);
    if (mcParticle)
      m_pid.top.seen = mcParticle->hasSeenInDetector(Const::TOP);

    m_pid.arich.le = pid->getLogL(Const::electron, Const::ARICH);
    m_pid.arich.lmu = pid->getLogL(Const::muon, Const::ARICH);
    m_pid.arich.lpi = pid->getLogL(Const::pion, Const::ARICH);
    m_pid.arich.lk = pid->getLogL(Const::kaon, Const::ARICH);
    m_pid.arich.lp = pid->getLogL(Const::proton, Const::ARICH);
    m_pid.arich.flag = pid->isAvailable(Const::ARICH);
    if (mcParticle)
      m_pid.arich.seen = mcParticle->hasSeenInDetector(Const::ARICH);

    m_pid.ecl.le = pid->getLogL(Const::electron, Const::ECL);
    m_pid.ecl.lmu = pid->getLogL(Const::muon, Const::ECL);
    m_pid.ecl.lpi = pid->getLogL(Const::pion, Const::ECL);
    m_pid.ecl.lk = pid->getLogL(Const::kaon, Const::ECL);
    m_pid.ecl.lp = pid->getLogL(Const::proton, Const::ECL);
    m_pid.ecl.flag = pid->isAvailable(Const::ECL);
    if (mcParticle)
      m_pid.ecl.seen = mcParticle->hasSeenInDetector(Const::ECL);

    m_pid.klm.le = pid->getLogL(Const::electron, Const::KLM);
    m_pid.klm.lmu = pid->getLogL(Const::muon, Const::KLM);
    m_pid.klm.lpi = pid->getLogL(Const::pion, Const::KLM);
    m_pid.klm.lk = pid->getLogL(Const::kaon, Const::KLM);
    m_pid.klm.lp = pid->getLogL(Const::proton, Const::KLM);
    m_pid.klm.flag = pid->isAvailable(Const::KLM);
    if (mcParticle)
      m_pid.klm.seen = mcParticle->hasSeenInDetector(Const::KLM);

    m_tree->Fill();
  }

}


void PIDNtupleModule::endRun() { }

void PIDNtupleModule::terminate()
{
  m_file->cd();
  m_tree->Write();
  m_file->Close();
}

void PIDNtupleModule::printModuleParams() const { }
