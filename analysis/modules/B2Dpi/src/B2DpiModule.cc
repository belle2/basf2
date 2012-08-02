/*******************************************************************************
 * Module: B2Dpi                                                               *
 *                                                                             *
 * Contributors: Tagir Aushev, Karim Trabelsi, Anze Zupanc                      *
 *                                                                             *
 * Description: This modules performs the reconstruction of                    *
 * B- -> D0 pi-; D0 -> K-pi+ decays                                            *
 *                                                                             *
 * This software is provided "as is" without any warranty. Due to the          *
 * limited knowledge of BASF2 by the author some parts of the module might     *
 * be written more elegantly. Eventualy also the reconstrcution/analysis tools *
 * will be improved and parts of the code might become obsolete or could be    *
 * performed in a better way. If you find a better solution to anything below  *
 * please fill free to modify it.                                              *
 *******************************************************************************/

#include <analysis/modules/B2Dpi/B2DpiModule.h>
#include <framework/core/Environment.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include <generators/utilities/cm2LabBoost.h>

// utilities
#include <analysis/particle/combination.h>
#include <analysis/utility/physics.h>
#include <analysis/utility/pid.h>
#include <analysis/utility/makeMother.h>

using namespace std;
using namespace Belle2;
using namespace analysis;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(B2Dpi)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

B2DpiModule::B2DpiModule() : Module()
{
  //Set module properties
  setDescription("BtoDpi Analysis Module");

  //Parameters definition
  addParam("outputFileName", m_dataOutFileName, "Output root file name of this module. Suffixes to distinguish them will be added automatically", string("B2Dpi"));

  addParam("MCParticlesColName", m_mcParticlesColName, "MCParticles collection name" , string(""));
  addParam("GFTrackCandidatesColName", m_gfTrackCandColName, "GF Track cand. collection name",       string(""));
  addParam("GFTracksColName", m_gfTracksColName, "GFTracks collection name", string(""));
  addParam("TracksColName", m_tracksColName, "Tracks collection name", string(""));
}

void B2DpiModule::initialize()
{
  B2INFO("[B2Dpi Module]: Starting initialization of B2Dpi Module.");

  nevt = 0;

  // Initialize store arrays
  // Why is it necessary to do here????
  StoreArray<MCParticle>     mcParticles(m_mcParticlesColName);
  StoreArray<GFTrackCand>    gfTrackCand(m_gfTrackCandColName);
  StoreArray<GFTrack>        gfTracks(m_gfTracksColName);
  StoreArray<Track>          tracks(m_tracksColName);

  // Initializing the output root file
  string dataFileName = m_dataOutFileName + ".root";

  m_rootFile = new TFile(dataFileName.c_str(), "RECREATE");
  m_tree     = new TTree("m_tree", "B2Dpi tree");

  m_tree->Branch("bsize",     &m_bsize,      "bsize/D");
  m_tree->Branch("dmass",     &m_md,         "dmass/D");
  m_tree->Branch("mbc",       &m_mbc,        "mbc/D");
  m_tree->Branch("deltae",    &m_deltae,     "deltae/D");
  m_tree->Branch("dmassFit",  &m_md_fit,     "dmassFit/D");
  m_tree->Branch("mbcFit",    &m_mbc_fit,    "mbcFit/D");
  m_tree->Branch("deltaeFit", &m_deltae_fit, "deltaeFit/D");
  m_tree->Branch("cmse",      &m_cmse,       "cmse/D");
  m_tree->Branch("bpvx",      &m_bp_vx,      "bpvx/D");
  m_tree->Branch("bpvy",      &m_bp_vy,      "bpvy/D");
  m_tree->Branch("bpvz",      &m_bp_vz,      "bpvz/D");
  m_tree->Branch("bpevx",     &m_bp_evx,     "bpevx/D");
  m_tree->Branch("bpevy",     &m_bp_evy,     "bpevy/D");
  m_tree->Branch("bpevz",     &m_bp_evz,     "bpevz/D");
  m_tree->Branch("gen_bpvx",  &m_gen_bp_vx,  "gen_bp_vx/D");
  m_tree->Branch("gen_bpvy",  &m_gen_bp_vy,  "gen_bp_vy/D");
  m_tree->Branch("gen_bpvz",  &m_gen_bp_vz,  "gen_bp_vz/D");
  m_tree->Branch("bpchi2",    &m_bp_chi2,    "bpchi2/D");
  m_tree->Branch("d0vx",      &m_d0_vx,      "d0vx/D");
  m_tree->Branch("d0vy",      &m_d0_vy,      "d0vy/D");
  m_tree->Branch("d0vz",      &m_d0_vz,      "d0vz/D");
  m_tree->Branch("d0evx",     &m_d0_evx,     "d0evx/D");
  m_tree->Branch("d0evy",     &m_d0_evy,     "d0evy/D");
  m_tree->Branch("d0evz",     &m_d0_evz,     "d0evz/D");
  m_tree->Branch("gen_d0vx",  &m_gen_d0_vx,  "gen_d0_vx/D");
  m_tree->Branch("gen_d0vy",  &m_gen_d0_vy,  "gen_d0_vy/D");
  m_tree->Branch("gen_d0vz",  &m_gen_d0_vz,  "gen_d0_vz/D");
  m_tree->Branch("d0chi2",    &m_d0_chi2,    "d0chi2/D");

  B2INFO("[B2Dpi Module]: Finished initialising the EvtGen Study Module.");


}

void B2DpiModule::terminate()
{
  m_tree->Write();
  m_rootFile->Close();
}

void B2DpiModule::event()
{
  B2INFO("[B2Dpi Module]: Begin Event.");

  nevt++;
  //std::cout << "******* EVT " << nevt << std::endl;

  m_md = 0;
  m_mbc = 0;
  m_deltae = 0;
  m_md_fit = 0;
  m_mbc_fit = 0;
  m_deltae_fit = 0;
  m_bsize = 0;
  m_cmse = 0;

  m_bp_vx = 0;
  m_bp_vy = 0;
  m_bp_vz = 0;
  m_bp_evx = 0;
  m_bp_evy = 0;
  m_bp_evz = 0;
  m_gen_bp_vx = 0;
  m_gen_bp_vy = 0;
  m_gen_bp_vz = 0;
  m_bp_chi2 = 0;

  m_d0_vx = 0;
  m_d0_vy = 0;
  m_d0_vz = 0;
  m_d0_evx = 0;
  m_d0_evy = 0;
  m_d0_evz = 0;
  m_gen_d0_vx = 0;
  m_gen_d0_vy = 0;
  m_gen_d0_vz = 0;
  m_d0_chi2 = 0;

  // Fitted tracks with GenFit
  StoreArray<Track>    tracks(m_tracksColName);
  int nTracks = tracks.getEntries();

  Ptype ptype_pion_plus(211);
  Ptype ptype_pion_minus(-211);
  Ptype ptype_kaon_plus(321);
  Ptype ptype_kaon_minus(-321);
  Ptype ptype_pion_zero(111);
  Ptype ptype_gamma(22);
  Ptype ptype_d0(421);
  Ptype ptype_d0b(-421);
  Ptype ptype_phi(333);
  Ptype ptype_ks(310);
  Ptype ptype_bp(521);
  Ptype ptype_bm(-521);

  // Array of MC particles
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);

  // check if the event contains the signal B+ -> D0bar[ -> K+ pi-] pi+ decay
  // if yes, save the signal side MC particles (B+, D0bar, Kaons and Pions)
  int signalBPDecay    = 0;
  int signalD0Decay    = 0;

  for (int i = 0; i < mcParticles.getEntries(); i++) {
    MCParticle* mc = mcParticles[i];

    // find the signal side decay chain MCParticles (B+, D0bar, K+, pi-, pi+)
    if (abs(mc->getPDG()) == 521) {
      signalBPDecay = isBtoD0PiDecay(mc);

      if (signalBPDecay) {
        signalD0Decay = isD0Decay(D0Sig);
        break;
      }
    }
  }

  if (signalBPDecay && signalD0Decay) {

    m_gen_bp_vx = BPSig->getDecayVertex().x();
    m_gen_bp_vy = BPSig->getDecayVertex().y();
    m_gen_bp_vz = BPSig->getDecayVertex().z();

    m_gen_d0_vx = D0Sig->getDecayVertex().x();
    m_gen_d0_vy = D0Sig->getDecayVertex().y();
    m_gen_d0_vz = D0Sig->getDecayVertex().z();
  }


  // Reconstruction
  // Fill final state particle (charged tracks) vectors
  // all charged tracks are considered as kaons/pions,
  // e.g. there is no PID information used as this point
  // to separate the two species
  vector<Particle> k_p, k_m, pi_p, pi_m;

  for (int i = 0; i < nTracks; ++i) {
    if (tracks[i]->getOmega() > 0) {
      Particle tmp1(*tracks[i], ptype_kaon_plus);
      k_p.push_back(tmp1);
      Particle tmp2(*tracks[i], ptype_pion_plus);
      pi_p.push_back(tmp2);
    } else {
      Particle tmp3(*tracks[i], ptype_kaon_minus);
      k_m.push_back(tmp3);
      Particle tmp4(*tracks[i], ptype_pion_minus);
      pi_m.push_back(tmp4);
    }
  }

  // Make D0 and D0-bar combinations (D0 ->Kpi decays)
  // all candidates within +/- 100 MeV around the nominal
  // D0 mass are accepted
  std::vector<Particle> D0;
  std::vector<Particle> D0b;
  combination(D0,  ptype_d0,  k_m, pi_p,   0.100);
  combination(D0b, ptype_d0b, k_p, pi_m,   0.100);

  // Make B+/- combinations (both charg combinations are
  // saved to the same vector of particles)
  // No cut on B mesons is performed at this stage
  std::vector<Particle> B;
  combination(B, ptype_bp, D0,  pi_m);
  combination(B, ptype_bm, D0b, pi_p);

  // Obtain the CM boost vector and CM energy
  // (functions are implemented in utility/include/physics.h)
  boostToCMS = getCMSBoostVector();
  cmsE       = getCMSEnergy() / 2.0;

  // loop over B candidates and remove all bad candidates
  for (int i = 0; i < (int) B.size(); ++i) {

    m_cmse = cmsE;
    m_md   = B[i].child(0).momentum().p().mag();

    m_b.SetPxPyPzE(B[i].px(), B[i].py(), B[i].pz(), B[i].e());
    m_b.Boost(boostToCMS);

    m_deltae = m_b.E() - cmsE;
    m_mbc    = sqrt((cmsE * cmsE) - m_b.Vect().Mag2());

    if (!(m_mbc > 5.2 && abs(m_deltae) < 0.3)) {
      // remove bad candidate
      B.erase(B.begin() + i);
      --i;
    }
  }


  // loop over remaining B candidates and save desired information
  // to the ntuple
  for (int i = 0; i < (int) B.size(); ++i) {
    m_bsize = (double) B.size();

    // calculate DeltaE and Mbc before any kinematic fitting
    // for later comparison
    m_cmse = cmsE;
    m_md   = B[i].child(0).momentum().p().mag();

    m_b.SetPxPyPzE(B[i].px(), B[i].py(), B[i].pz(), B[i].e());
    m_b.Boost(boostToCMS);

    m_deltae = m_b.E() - cmsE;
    m_mbc    = sqrt((cmsE * cmsE) - m_b.Vect().Mag2());

    // perform mass-constrained-vertex fit for D0 candidate
    // and save updated D0 mass, Mbc, DeltaE to ntuple
    // for comparison with the "unconstrained" values
    double confLevel(0);

    doKmvFit(B[i].child(0), confLevel);
    m_d0_chi2 = confLevel;

    // perform vertex fit for B+ candidate
    doKvFit(B[i], confLevel);
    m_bp_chi2 = confLevel;

    // save m(Kpi), Mbc and DeltaE after the kinematic fits
    m_md_fit = B[i].child(0).momentum().p().mag();

    m_b.SetPxPyPzE(B[i].px(), B[i].py(), B[i].pz(), B[i].e());
    m_b.Boost(boostToCMS);
    m_deltae_fit = m_b.E() - cmsE;
    m_mbc_fit    = sqrt((cmsE * cmsE) - m_b.Vect().Mag2());

    // save vertex information
    m_d0_vx = B[i].child(0).momentum().x().x();
    m_d0_vy = B[i].child(0).momentum().x().y();
    m_d0_vz = B[i].child(0).momentum().x().z();

    m_d0_evx = sqrt(B[i].child(0).momentum().dx()[0][0]);
    m_d0_evy = sqrt(B[i].child(0).momentum().dx()[1][1]);
    m_d0_evz = sqrt(B[i].child(0).momentum().dx()[2][2]);

    m_bp_vx = B[i].momentum().x().x();
    m_bp_vy = B[i].momentum().x().y();
    m_bp_vz = B[i].momentum().x().z();

    m_bp_evx = sqrt(B[i].momentum().dx()[0][0]);
    m_bp_evy = sqrt(B[i].momentum().dx()[1][1]);
    m_bp_evz = sqrt(B[i].momentum().dx()[2][2]);

    m_tree->Fill();
  }

  B2INFO("[B2Dpi] - end of event!");
}

unsigned
B2DpiModule::doKvFit(Particle& p, double& confLevel)
{

  VertexFitKFit kv;
  kv.setMagneticField(1.5);

  kv.addTrack(p.child(0).momentum().p(),
              p.child(0).momentum().x(),
              p.child(0).momentum().dpx(),
              p.child(0).pType().charge());

  kv.addTrack(p.child(1).momentum().p(),
              p.child(1).momentum().x(),
              p.child(1).momentum().dpx(),
              p.child(1).pType().charge());

  unsigned err = kv.doFit();
  if (!err) {
    confLevel = kv.getCHIsq();

    return makeMother(kv, p);
  } else
    return 0;
}


unsigned
B2DpiModule::doKmFit(Particle& p, double& confLevel)
{

  MassFitKFit km;
  km.setMagneticField(1.5);

  km.addTrack(p.child(0).momentum().p(),
              p.child(0).momentum().x(),
              p.child(0).momentum().dpx(),
              p.child(0).pType().charge());

  km.addTrack(p.child(1).momentum().p(),
              p.child(1).momentum().x(),
              p.child(1).momentum().dpx(),
              p.child(1).pType().charge());

  km.setInvariantMass(p.pType().mass());

  unsigned err = km.doFit();
  if (!err) {
    confLevel = km.getCHIsq();

    return makeMother(km, p);
  } else
    return 0;
}



unsigned
B2DpiModule::doKmvFit(Particle& p, double& confLevel)
{

  MassVertexFitKFit kmv;
  kmv.setMagneticField(1.5);

  kmv.addTrack(p.child(0).momentum().p(),
               p.child(0).momentum().x(),
               p.child(0).momentum().dpx(),
               p.child(0).pType().charge());

  kmv.addTrack(p.child(1).momentum().p(),
               p.child(1).momentum().x(),
               p.child(1).momentum().dpx(),
               p.child(1).pType().charge());

  kmv.setInvariantMass(p.pType().mass());

  unsigned err = kmv.doFit();
  if (!err) {
    confLevel = kmv.getCHIsq();

    return makeMother(kmv, p);
  } else
    return 0;
}

int B2DpiModule::isBtoD0PiDecay(MCParticle* part)
{
  std::vector<MCParticle*> daughters = part->getDaughters();
  std::vector<MCParticle*> daughtersWOFSR;
  removeFSRPhoton(daughters, daughtersWOFSR);

  int signal = 0;
  if (daughtersWOFSR.size() == 2) {
    if (abs(daughtersWOFSR[0]->getPDG()) == 421 && abs(daughtersWOFSR[1]->getPDG()) == 211 && daughtersWOFSR[1]->getPDG()*part->getPDG() > 0) {
      signal = 1;
      BPSig = part;
      D0Sig = daughtersWOFSR[0];
      PrimaryPI = daughtersWOFSR[1];
    } else if (abs(daughtersWOFSR[1]->getPDG()) == 421 && abs(daughtersWOFSR[0]->getPDG()) == 211 && daughtersWOFSR[0]->getPDG()*part->getPDG() > 0) {
      signal = 1;
      BPSig = part;
      D0Sig = daughtersWOFSR[1];
      PrimaryPI = daughtersWOFSR[0];
    }
  } else
    BMTag = part;

  return signal;
}

int B2DpiModule::isD0Decay(MCParticle* part)
{
  std::vector<MCParticle*> daughters = part->getDaughters();
  std::vector<MCParticle*> daughtersWOFSR;
  removeFSRPhoton(daughters, daughtersWOFSR);

  int signal = 0;
  if (daughtersWOFSR.size() == 2) {
    if (daughtersWOFSR[0]->getPDG() == -321 && daughtersWOFSR[1]->getPDG() == 211 && part->getPDG() == 421) {
      signal = 1;
      KSig  = daughtersWOFSR[0];
      PiSig = daughtersWOFSR[1];
    } else if (daughtersWOFSR[0]->getPDG() == 321 && daughtersWOFSR[1]->getPDG() == -211 && part->getPDG() == -421) {
      signal = 1;
      KSig = daughtersWOFSR[0];
      PiSig = daughtersWOFSR[1];
    }
  }

  return signal;
}

// removes all photons from the daughters list
// assumes that all photons are due to the final stat radiation
// should not be used in radiative decays
void B2DpiModule::removeFSRPhoton(std::vector<MCParticle*> daughters, std::vector<MCParticle*> &daughtersWithoutFSR)
{
  for (unsigned i = 0; i < daughters.size(); i++) {
    if (daughters[i]->getPDG() != 22)
      daughtersWithoutFSR.push_back(daughters[i]);
  }
}
