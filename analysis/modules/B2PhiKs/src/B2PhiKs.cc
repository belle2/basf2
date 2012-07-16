/**************************************************************************
 * Module: B2PhiKs                                                        *
 *                                                                        *
 * Author: Anze Zupanc                                                    *
 *                                                                        *
 * Description: This modules performs the reconstruction of               *
 * B0 -> phi Ks; phi -> K+K-; Kshort -> pi+ pi- decays                    *
 *                                                                        *
 * Currently the "analysis tools" are quite limited, e.g. there is no     *
 * Particle class or soemthing equivalent available, therefore few        *
 * workarounds are made.                                                  *
 *                                                                        *
 * The module works as following:                                         *
 * 1. find MCParticles from the signal decay chain (B0/phi/Ks/K+/K-/pi+/pi-) *
 * 2. find reconstructed tracks produced by signal decay chain particles  *
 * 3. save relevant generator/reconstructed info                          *
 *                                                                        *
 * This software is provided "as is" without any warranty. Due to the     *
 * limited knowledge of BASF2 by the author some parts of the module might *
 * be written more elegantly. Eventualy also the reconstrcution/analysis tools *
 * will be improved and parts of the code might become obsolete or could be *
 * performed in a better way. If you find a better solution to anything below *
 * please fill free to modify it.               *
 **************************************************************************/

#include <analysis/modules/B2PhiKs/B2PhiKsModule.h>
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
#include <analysis/utility/tracking.h>
#include <analysis/utility/physics.h>
#include <analysis/utility/pid.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(B2PhiKs)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

B2PhiKsModule::B2PhiKsModule() : Module()
{
  //Set module properties
  setDescription("BtoPhiKshort Analysis Module - module for t-dep. study of B0 -> phi(K+K-) K0s(pi+pi-) decays");

  //Parameters definition
  addParam("outputFileName", m_dataOutFileName, "Output root file name of this module. Suffixes to distinguish them will be added automatically", string("B2PhiKs"));

  addParam("MCParticlesColName", m_mcParticlesColName, "MCParticles collection name" , string(""));
  addParam("GFTrackCandidatesColName",      m_gfTrackCandColName,     "GF Track cand. collection name",       string(""));
  addParam("GFTracksColName", m_gfTracksColName, "GFTracks collection name", string(""));
  addParam("TracksColName", m_tracksColName, "Tracks collection name", string(""));
}


void B2PhiKsModule::initialize()
{
  B2INFO("[B2PhiKs Module]: Starting initialization of B2PhiKs Module.");

  // Initialize store arrays
  // Why is it necessary to do here????
  StoreArray<MCParticle>     mcParticles(m_mcParticlesColName);
  StoreArray<GFTrackCand>    gfTrackCand(m_gfTrackCandColName);
  StoreArray<Track>          tracks(m_tracksColName);
  StoreArray<GFTrack>        gfTracks(m_gfTracksColName);

  // Initializing the output root file
  string dataFileName = m_dataOutFileName + ".root";
  m_rootFile = new TFile(dataFileName.c_str(), "RECREATE");
  m_tree     = new TTree("m_tree", "B2PhiKs tree");

  // initialize the TTree branches
  initBranches(m_tree);

  B2INFO("[B2PhiKs Module]: Finished initialising the EvtGen Study Module.");
}

void B2PhiKsModule::terminate()
{
  m_tree->Write();
  m_rootFile->Close();
}

void B2PhiKsModule::event()
{
  //B2INFO("[B2PhiKs Module]: Begin Event.");

  // Array of MC particles
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);

  // Track Candidates (not fitted, just found)
  // MC matching (MCParticle -> GFTrackCand) is performed with CDCMCMatching module
  StoreArray<GFTrackCand> gfTrackCand(m_gfTrackCandColName);
  int nGFTrackCands = gfTrackCand.getEntries();

  // GFTrack collection is needed for extrapolation of pion tracks to Ks decay vertex
  StoreArray<GFTrack> gfTracks(m_gfTracksColName);
  int nGFTracks = gfTracks.getEntries();

  // Fitted tracks with GenFit
  StoreArray<Track> tracks(m_tracksColName);
  int nTracks = tracks.getEntries();

  // PXD/SVD/CDC (true generated) hits
  StoreArray<PXDTrueHit> pxdTrueHits;
  StoreArray<SVDTrueHit> svdTrueHits;
  StoreArray<CDCSimHit>  cdcTrueHits;

  // relation MCParticle -> PXD/SVD/CDC true hit
  RelationArray mcPartToPXDTrueHits(mcParticles, pxdTrueHits);
  RelationArray mcPartToSVDTrueHits(mcParticles, svdTrueHits);
  RelationArray mcPartToCDCTrueHits(mcParticles, cdcTrueHits);

  // Obtain the CMS boost vector and CMS Energy
  // getCMSBoostVector() and  getCMSEnergy() are defined in physics.cc utility
  boostToCMS = getCMSBoostVector();
  cmsE       = getCMSEnergy() / 2.0;


  // check if the event contains the signal B0->phiKs decay
  // if yes, save the signal side MC particles (B0,phi,Kshort,Kaons and Pions)
  int signalB0Decay     = 0;
  int signalPhiDecay    = 0;
  int signalKshortDecay = 0;

  for (int i = 0; i < mcParticles.getEntries(); i++) {
    MCParticle* mc = mcParticles[i];

    // find the signal side decay chain MCParticles (B0, Phi, Ks, K+, K-, pi+, pi-)
    if (abs(mc->getPDG()) == 511) {
      signalB0Decay = isBtoPhiKshortDecay(mc);

      if (signalB0Decay) {
        signalPhiDecay = isPhiKpKmDecay(PhiSig);
        signalKshortDecay = isKshortPipPimDecay(KshortSig);
      }
    }
  }

  // set all reconstructed info to initial values;
  initReconstructedInfo();

  if (signalB0Decay && signalPhiDecay && signalKshortDecay) {
    // fill the generator related info
    fillGeneratedInfo(signalB0Decay, signalPhiDecay, signalKshortDecay);

    // loop over GFTrackCandidates and check if signal FSP particles are reconstructed
    for (int i = 0; i < nGFTrackCands; i++) {
      const int mcTrackId = gfTrackCand[i]->getMcTrackId();

      if (mcTrackId == KpSig->getArrayIndex()) {
        m_kpTrackFound++;
      }

      if (mcTrackId == KmSig->getArrayIndex()) {
        m_kmTrackFound++;
      }

      if (mcTrackId == PipSig->getArrayIndex()) {
        m_pipTrackFound++;
      }

      if (mcTrackId == KmSig->getArrayIndex()) {
        m_pimTrackFound++;
      }
    }

    // Loop over fitted Tracks
    // Find track corresponding to a signal side MCParticle
    // save reconstructed track related info
    for (int i = 0; i < nTracks; i++) {
      const int mcTrackId = tracks[i]->getMCId();

      // K+
      if (mcTrackId == KpSig->getArrayIndex()) {
        if (tracks[i]->getFitFailed() == false) {
          m_kpTrackFitted++;
          m_kpTrackCDCRecoHits = tracks[i]->getNCDCHits();
          m_kpTrackSVDRecoHits = tracks[i]->getNSVDHits();
          m_kpTrackPXDRecoHits = tracks[i]->getNPXDHits();

          m_kpTrackD0 = tracks[i]->getD0();
          m_kpTrackZ0 = tracks[i]->getZ0();

          m_kp3Momentum = tracks[i]->getMomentum();
          m_kp_ptot  = m_kp3Momentum.Mag();
          m_kp_pt    = m_kp3Momentum.Perp();
          m_kp_pz    = m_kp3Momentum.Pz();
          m_kp_costh = m_kp3Momentum.CosTheta();

          m_kpTrackPValue = tracks[i]->getPValue();

          m_kp_pterr = sqrt(tracks[i]->getPErrors().x() * tracks[i]->getPErrors().x() + tracks[i]->getPErrors().y() * tracks[i]->getPErrors().y());
          m_kp_pzerr = tracks[i]->getPErrors().z();
        }
      }
      // K-
      if (mcTrackId == KmSig->getArrayIndex()) {
        if (tracks[i]->getFitFailed() == false) {
          m_kmTrackFitted++;
          m_kmTrackCDCRecoHits = tracks[i]->getNCDCHits();
          m_kmTrackSVDRecoHits = tracks[i]->getNSVDHits();
          m_kmTrackPXDRecoHits = tracks[i]->getNPXDHits();

          m_kmTrackD0 = tracks[i]->getD0();
          m_kmTrackZ0 = tracks[i]->getZ0();

          m_km3Momentum = tracks[i]->getMomentum();
          m_km_ptot  = m_km3Momentum.Mag();
          m_km_pt    = m_km3Momentum.Perp();
          m_km_pz    = m_km3Momentum.Pz();
          m_km_costh = m_km3Momentum.CosTheta();

          m_kmTrackPValue = tracks[i]->getPValue();

          m_km_pterr = sqrt(tracks[i]->getPErrors().x() * tracks[i]->getPErrors().x() + tracks[i]->getPErrors().y() * tracks[i]->getPErrors().y());
          m_km_pzerr = tracks[i]->getPErrors().z();
        }
      }
      // Pi+
      if (mcTrackId == PipSig->getArrayIndex()) {
        if (tracks[i]->getFitFailed() == false) {
          m_pipTrackFitted++;
          m_pipTrackCDCRecoHits = tracks[i]->getNCDCHits();
          m_pipTrackSVDRecoHits = tracks[i]->getNSVDHits();
          m_pipTrackPXDRecoHits = tracks[i]->getNPXDHits();

          m_pipTrackD0 = tracks[i]->getD0();
          m_pipTrackZ0 = tracks[i]->getZ0();

          // Assuming that index in GFTrack and Track points to same track
          if (i < nGFTracks) {
            if (mcTrackId == gfTracks[i]->getCand().getMcTrackId()) {
              // point of closest approach (POCA) of pion track to the generated Kshort vertex
              // momentum at poca (need to extrapolate the GFTrack to the generated Kshort vertex)
              m_pipTrackExtrapolated = getTrackParametersAtGivenPoint(gfTracks[i], KshortSig->getDecayVertex(), m_pipPOCA, m_pip3Momentum);
            }
          } else {
            B2INFO("[B2PhiKs]: GFTrack index out of bounds!");
          }

          m_pip_ptot  = m_pip3Momentum.Mag();
          m_pip_pt    = m_pip3Momentum.Perp();
          m_pip_pz    = m_pip3Momentum.Pz();
          m_pip_costh = m_pip3Momentum.CosTheta();

          m_pipTrackPValue = tracks[i]->getPValue();

          // TODO: Estimate momentum error at POCA
        }
      }
      // Pi-
      if (mcTrackId == PimSig->getArrayIndex()) {
        if (tracks[i]->getFitFailed() == false) {
          m_pimTrackFitted++;
          m_pimTrackCDCRecoHits = tracks[i]->getNCDCHits();
          m_pimTrackSVDRecoHits = tracks[i]->getNSVDHits();
          m_pimTrackPXDRecoHits = tracks[i]->getNPXDHits();

          m_pimTrackD0 = tracks[i]->getD0();
          m_pimTrackZ0 = tracks[i]->getZ0();

          // Assuming that index in GFTrack and Track points to same track
          if (i < nGFTracks) {
            if (mcTrackId == gfTracks[i]->getCand().getMcTrackId()) {
              // point of closest approach (POCA) of pion track to the generated Kshort vertex
              // momentum at poca (need to extrapolate the GFTrack to the generated Kshort vertex)
              m_pimTrackExtrapolated = getTrackParametersAtGivenPoint(gfTracks[i], KshortSig->getDecayVertex(), m_pimPOCA, m_pim3Momentum);
            }
          } else {
            B2INFO("[B2PhiKs]: GFTrack index out of bounds!");
          }

          m_pim_ptot  = m_pim3Momentum.Mag();
          m_pim_pt    = m_pim3Momentum.Perp();
          m_pim_pz    = m_pim3Momentum.Pz();
          m_pim_costh = m_pim3Momentum.CosTheta();

          m_pimTrackPValue = tracks[i]->getPValue();

          // TODO: Estimate momentum error at POCA
        }
      }
    }

    // obtain the number of generated hits by the signal particles in PXD/SVD/CDC
    m_kpTrackPXDHits = getNumberOfHits(KpSig, mcPartToPXDTrueHits);
    m_kmTrackPXDHits = getNumberOfHits(KmSig, mcPartToPXDTrueHits);
    m_pipTrackPXDHits = getNumberOfHits(PipSig, mcPartToPXDTrueHits);
    m_pimTrackPXDHits = getNumberOfHits(PimSig, mcPartToPXDTrueHits);

    m_kpTrackCDCHits = getNumberOfHits(KpSig, mcPartToCDCTrueHits);
    m_kmTrackCDCHits = getNumberOfHits(KmSig, mcPartToCDCTrueHits);
    m_pipTrackCDCHits = getNumberOfHits(PipSig, mcPartToCDCTrueHits);
    m_pimTrackCDCHits = getNumberOfHits(PimSig, mcPartToCDCTrueHits);

    m_kpTrackSVDHits = getNumberOfHits(KpSig, mcPartToSVDTrueHits);
    m_kmTrackSVDHits = getNumberOfHits(KmSig, mcPartToSVDTrueHits);
    m_pipTrackSVDHits = getNumberOfHits(PipSig, mcPartToSVDTrueHits);
    m_pimTrackSVDHits = getNumberOfHits(PimSig, mcPartToSVDTrueHits);

    // obtain the TOP/ARICH Likelihood difference for signal particles
    int sigHypothesis = 4; // Kaon hypothesis
    int bkgHypothesis = 3; // Pion hypothesis
    m_kpTOPKPiDeltaLogL = getTOPPID(sigHypothesis, bkgHypothesis, KpSig);
    m_kmTOPKPiDeltaLogL = getTOPPID(sigHypothesis, bkgHypothesis, KmSig);
    m_pipTOPKPiDeltaLogL = getTOPPID(sigHypothesis, bkgHypothesis, PipSig);
    m_pimTOPKPiDeltaLogL = getTOPPID(sigHypothesis, bkgHypothesis, PimSig);

    m_kpARICHKPiDeltaLogL = getARICHPID(sigHypothesis, bkgHypothesis, KpSig);
    m_kmARICHKPiDeltaLogL = getARICHPID(sigHypothesis, bkgHypothesis, KmSig);
    m_pipARICHKPiDeltaLogL = getARICHPID(sigHypothesis, bkgHypothesis, PipSig);
    m_pimARICHKPiDeltaLogL = getARICHPID(sigHypothesis, bkgHypothesis, PimSig);

    // Composite particles
    if (m_pipTrackExtrapolated && m_pimTrackExtrapolated) {
      // Pi+/- "impact parameters" with respect to the generated Ks decay Vertex
      TVector3 ksDecV  = KshortSig->getDecayVertex();
      TVector3 pipVect = m_pipPOCA - ksDecV;
      TVector3 pimVect = m_pimPOCA - ksDecV;

      double pipZDist = pipVect.Z();
      double pimZDist = pimVect.Z();
      m_ks_mindz = min(pipZDist, pimZDist);
      m_ks_maxdz = max(pipZDist, pimZDist);

      double pipRDist = pipVect.Perp();
      double pimRDist = pimVect.Perp();
      m_ks_mindr = min(pipRDist, pimRDist);
      m_ks_maxdr = max(pipRDist, pimRDist);

      // calculate Kshort invariant mass
      double PIC_MASS = 0.139570;
      m_pipLV.SetXYZM(m_pip3Momentum.Px(), m_pip3Momentum.Py(), m_pip3Momentum.Pz(), PIC_MASS);
      m_pimLV.SetXYZM(m_pim3Momentum.Px(), m_pim3Momentum.Py(), m_pim3Momentum.Pz(), PIC_MASS);

      m_ksLV = m_pipLV + m_pimLV;

      m_ksmass = m_ksLV.Mag();
      m_ksptot = m_ksLV.Vect().Mag();
      m_kspt   = m_ksLV.Perp();
      m_kspz   = m_ksLV.Pz();
    }

    if (m_kpTrackFitted && m_kmTrackFitted) {
      // calculate Phi invariant mass
      double KC_MASS = 0.493677;

      m_kpLV.SetXYZM(m_kp3Momentum.Px(), m_kp3Momentum.Py(), m_kp3Momentum.Pz(), KC_MASS);
      m_kmLV.SetXYZM(m_km3Momentum.Px(), m_km3Momentum.Py(), m_km3Momentum.Pz(), KC_MASS);

      m_phiLV = m_kpLV + m_kmLV;

      m_phimass = m_phiLV.Mag();

    }

    if (m_kpTrackFitted && m_kmTrackFitted && m_pipTrackExtrapolated && m_pimTrackExtrapolated) {
      // calculate B0 invariant mass
      m_b0LV = m_phiLV + m_ksLV;

      m_b0mass = m_b0LV.Mag();

      m_b0LV.Boost(boostToCMS);
      m_deltae = m_b0LV.E() - cmsE;
      m_mbc = sqrt((cmsE * cmsE) - m_b0LV.Vect().Mag2());

    }

    // dump event related info to Tree
    m_tree->Fill();
  }

  B2INFO("[B2PhiKs] - end of event!");

}

int B2PhiKsModule::isBtoPhiKshortDecay(MCParticle* part)
{
  std::vector<MCParticle*> daughters = part->getDaughters();

  int signal = 0;
  if (daughters.size() == 2) {
    if (daughters[0]->getPDG() == 333 && daughters[1]->getPDG() == 310) {
      signal = 1;
      B0Sig = part;
      PhiSig = daughters[0];
      KshortSig = daughters[1];
    }
  } else
    B0Tag = part;

  return signal;
}

int B2PhiKsModule::isPhiKpKmDecay(MCParticle* part)
{
  std::vector<MCParticle*> daughters = part->getDaughters();
  std::vector<MCParticle*> daughtersWOFSR;
  removeFSRPhoton(daughters, daughtersWOFSR);

  int signal = 0;
  if (daughtersWOFSR.size() == 2) {
    if (daughters[0]->getPDG() == 321 && daughters[1]->getPDG() == -321) {
      signal = 1;
      KpSig = daughters[0];
      KmSig = daughters[1];
    } else if (daughters[1]->getPDG() == 321 && daughters[0]->getPDG() == -321) {
      signal = 1;
      KpSig = daughters[1];
      KmSig = daughters[0];
    }
  }

  return signal;
}

int B2PhiKsModule::isKshortPipPimDecay(MCParticle* part)
{
  std::vector<MCParticle*> daughters = part->getDaughters();
  std::vector<MCParticle*> daughtersWOFSR;
  removeFSRPhoton(daughters, daughtersWOFSR);

  int signal = 0;
  if (daughtersWOFSR.size() == 2) {
    if (daughters[0]->getPDG() == 211 && daughters[1]->getPDG() == -211) {
      signal = 1;
      PipSig = daughters[0];
      PimSig = daughters[1];
    } else if (daughters[1]->getPDG() == 211 && daughters[0]->getPDG() == -211) {
      signal = 1;
      PipSig = daughters[1];
      PimSig = daughters[0];
    }
  }

  return signal;
}

// removes all photons from the daughters list
// assumes that all photons are due to the final stat radiation
// should not be used in radiative decays
void B2PhiKsModule::removeFSRPhoton(std::vector<MCParticle*> daughters, std::vector<MCParticle*> &daughtersWithoutFSR)
{
  for (unsigned i = 0; i < daughters.size(); i++) {
    if (daughters[i]->getPDG() != 22)
      daughtersWithoutFSR.push_back(daughters[i]);
    else
      B2INFO("Removing FSR photon!");
  }
}

// for debugging
void B2PhiKsModule::printGenHepevtParticles(std::vector<MCParticle*> partList)
{
  for (unsigned i = 0; i < partList.size(); i++) {
    B2INFO(" " << i << " - > " << partList[i]->getPDG());
  }
}

void B2PhiKsModule::initBranches(TTree* tree)
{
  B2INFO("Adding branches to the tree");

  // Generator related INFO
  tree->Branch("gBtag_lund", &m_genBtag, "gBtag_lund/I");
  tree->Branch("gB_deltat", &m_genB_deltat, "gB_deltat/D");

  tree->Branch("gB_ptot", &m_genB_ptot, "gB_ptot/D");
  tree->Branch("gPhi_ptot", &m_genPhi_ptot, "gPhi_ptot/D");
  tree->Branch("gKs_ptot", &m_genKs_ptot, "gKs_ptot/D");
  tree->Branch("gKs_pt", &m_genKs_pt, "gKs_pt/D");
  tree->Branch("gKs_pz", &m_genKs_pz, "gKs_pz/D");

  tree->Branch("gPhi_helicity", &m_genPhi_helicity, "gPhi_helicity/D");

  tree->Branch("gKp_ptot", &m_genKp_ptot, "gKp_ptot/D");
  tree->Branch("gKp_pt", &m_genKp_pt, "gKp_pt/D");
  tree->Branch("gKp_pz", &m_genKp_pz, "gKp_pz/D");
  tree->Branch("gKp_phi", &m_genKp_phi, "gKp_phi/D");
  tree->Branch("gKp_costh", &m_genKp_costh, "gKp_costh/D");

  tree->Branch("gKm_ptot", &m_genKm_ptot, "gKm_ptot/D");
  tree->Branch("gKm_pt", &m_genKm_pt, "gKm_pt/D");
  tree->Branch("gKm_pz", &m_genKm_pz, "gKm_pz/D");
  tree->Branch("gKm_phi", &m_genKm_phi, "gKm_phi/D");
  tree->Branch("gKm_costh", &m_genKm_costh, "gKm_costh/D");

  tree->Branch("gPip_ptot", &m_genPip_ptot, "gPip_ptot/D");
  tree->Branch("gPip_pt", &m_genPip_pt, "gPip_pt/D");
  tree->Branch("gPip_pz", &m_genPip_pz, "gPip_pz/D");
  tree->Branch("gPip_costh", &m_genPip_costh, "gPip_costh/D");

  tree->Branch("gPim_ptot", &m_genPim_ptot, "gPim_ptot/D");
  tree->Branch("gPim_pt", &m_genPim_pt, "gPim_pt/D");
  tree->Branch("gPim_pz", &m_genPim_pz, "gPim_pz/D");
  tree->Branch("gPim_costh", &m_genPim_costh, "gPim_costh/D");


  tree->Branch("gKs_fl", &m_genKs_fl, "gKs_fl/D");

  // Reconstructed Info
  tree->Branch("kpTrackFound", &m_kpTrackFound, "kpTrackFound/I");
  tree->Branch("kmTrackFound", &m_kmTrackFound, "kmTrackFound/I");
  tree->Branch("pipTrackFound", &m_pipTrackFound, "pipTrackFound/I");
  tree->Branch("pimTrackFound", &m_pimTrackFound, "pimTrackFound/I");
  tree->Branch("kpTrackFitted", &m_kpTrackFitted, "kpTrackFitted/I");
  tree->Branch("kmTrackFitted", &m_kmTrackFitted, "kmTrackFitted/I");
  tree->Branch("pipTrackFitted", &m_pipTrackFitted, "pipTrackFitted/I");
  tree->Branch("pimTrackFitted", &m_pimTrackFitted, "pimTrackFitted/I");
  tree->Branch("pipTrackExtrapolated", &m_pipTrackExtrapolated, "pipTrackExtrapolated/I");
  tree->Branch("pimTrackExtrapolated", &m_pimTrackExtrapolated, "pimTrackExtrapolated/I");
  tree->Branch("kpTrackCDCRecoHits", &m_kpTrackCDCRecoHits, "kpTrackCDCRecoHits/I");
  tree->Branch("kmTrackCDCRecoHits", &m_kmTrackCDCRecoHits, "kmTrackCDCRecoHits/I");
  tree->Branch("pipTrackCDCRecoHits", &m_pipTrackCDCRecoHits, "pipTrackCDCRecoHits/I");
  tree->Branch("pimTrackCDCRecoHits", &m_pimTrackCDCRecoHits, "pimTrackCDCRecoHits/I");
  tree->Branch("kpTrackSVDRecoHits", &m_kpTrackSVDRecoHits, "kpTrackSVDRecoHits/I");
  tree->Branch("kmTrackSVDRecoHits", &m_kmTrackSVDRecoHits, "kmTrackSVDRecoHits/I");
  tree->Branch("pipTrackSVDRecoHits", &m_pipTrackSVDRecoHits, "pipTrackSVDRecoHits/I");
  tree->Branch("pimTrackSVDRecoHits", &m_pimTrackSVDRecoHits, "pimTrackSVDRecoHits/I");
  tree->Branch("kpTrackPXDRecoHits", &m_kpTrackPXDRecoHits, "kpTrackPXDRecoHits/I");
  tree->Branch("kmTrackPXDRecoHits", &m_kmTrackPXDRecoHits, "kmTrackPXDRecoHits/I");
  tree->Branch("pipTrackPXDRecoHits", &m_pipTrackPXDRecoHits, "pipTrackPXDRecoHits/I");
  tree->Branch("pimTrackPXDRecoHits", &m_pimTrackPXDRecoHits, "pimTrackPXDRecoHits/I");
  tree->Branch("kpTrackCDCHits", &m_kpTrackCDCHits, "kpTrackCDCHits/I");
  tree->Branch("kmTrackCDCHits", &m_kmTrackCDCHits, "kmTrackCDCHits/I");
  tree->Branch("pipTrackCDCHits", &m_pipTrackCDCHits, "pipTrackCDCHits/I");
  tree->Branch("pimTrackCDCHits", &m_pimTrackCDCHits, "pimTrackCDCHits/I");
  tree->Branch("kpTrackSVDHits", &m_kpTrackSVDHits, "kpTrackSVDHits/I");
  tree->Branch("kmTrackSVDHits", &m_kmTrackSVDHits, "kmTrackSVDHits/I");
  tree->Branch("pipTrackSVDHits", &m_pipTrackSVDHits, "pipTrackSVDHits/I");
  tree->Branch("pimTrackSVDHits", &m_pimTrackSVDHits, "pimTrackSVDHits/I");
  tree->Branch("kpTrackPXDHits", &m_kpTrackPXDHits, "kpTrackPXDHits/I");
  tree->Branch("kmTrackPXDHits", &m_kmTrackPXDHits, "kmTrackPXDHits/I");
  tree->Branch("pipTrackPXDHits", &m_pipTrackPXDHits, "pipTrackPXDHits/I");
  tree->Branch("pimTrackPXDHits", &m_pimTrackPXDHits, "pimTrackPXDHits/I");
  tree->Branch("kpTrackD0", &m_kpTrackD0, "kpTrackD0/D");
  tree->Branch("kmTrackD0", &m_kmTrackD0, "kmTrackD0/D");
  tree->Branch("pipTrackD0", &m_pipTrackD0, "pipTrackD0/D");
  tree->Branch("pimTrackD0", &m_pimTrackD0, "pimTrackD0/D");
  tree->Branch("kpTrackZ0", &m_kpTrackZ0, "kpTrackZ0/D");
  tree->Branch("kmTrackZ0", &m_kmTrackZ0, "kmTrackZ0/D");
  tree->Branch("pipTrackZ0", &m_pipTrackZ0, "pipTrackZ0/D");
  tree->Branch("pimTrackZ0", &m_pimTrackZ0, "pimTrackZ0/D");
  tree->Branch("kpTrackPValue", &m_kpTrackPValue, "kpTrackPValue/D");
  tree->Branch("kmTrackPValue", &m_kmTrackPValue, "kmTrackPValue/D");
  tree->Branch("pipTrackPValue", &m_pipTrackPValue, "pipTrackPValue/D");
  tree->Branch("pimTrackPValue", &m_pimTrackPValue, "pimTrackPValue/D");

  tree->Branch("kpTOPKPiDeltaLogL", &m_kpTOPKPiDeltaLogL, "kpTOPKPiDeltaLogL/D");
  tree->Branch("kmTOPKPiDeltaLogL", &m_kmTOPKPiDeltaLogL, "kmTOPKPiDeltaLogL/D");
  tree->Branch("pipTOPKPiDeltaLogL", &m_pipTOPKPiDeltaLogL, "pipTOPKPiDeltaLogL/D");
  tree->Branch("pimTOPKPiDeltaLogL", &m_pimTOPKPiDeltaLogL, "pimTOPKPiDeltaLogL/D");
  tree->Branch("kpARICHKPiDeltaLogL", &m_kpARICHKPiDeltaLogL, "kpARICHKPiDeltaLogL/D");
  tree->Branch("kmARICHKPiDeltaLogL", &m_kmARICHKPiDeltaLogL, "kmARICHKPiDeltaLogL/D");
  tree->Branch("pipARICHKPiDeltaLogL", &m_pipARICHKPiDeltaLogL, "pipARICHKPiDeltaLogL/D");
  tree->Branch("pimARICHKPiDeltaLogL", &m_pimARICHKPiDeltaLogL, "pimARICHKPiDeltaLogL/D");

  tree->Branch("kp_ptot", &m_kp_ptot, "kp_ptot/D");
  tree->Branch("km_ptot", &m_km_ptot, "km_ptot/D");
  tree->Branch("pip_ptot", &m_pip_ptot, "pip_ptot/D");
  tree->Branch("pim_ptot", &m_pim_ptot, "pim_ptot/D");

  tree->Branch("kp_pt", &m_kp_pt, "kp_pt/D");
  tree->Branch("km_pt", &m_km_pt, "km_pt/D");
  tree->Branch("pip_pt", &m_pip_pt, "pip_pt/D");
  tree->Branch("pim_pt", &m_pim_pt, "pim_pt/D");

  tree->Branch("kp_pterr", &m_kp_pterr, "kp_pterr/D");
  tree->Branch("km_pterr", &m_km_pterr, "km_pterr/D");
  tree->Branch("pip_pterr", &m_pip_pterr, "pip_pterr/D");
  tree->Branch("pim_pterr", &m_pim_pterr, "pim_pterr/D");

  tree->Branch("kp_pz", &m_kp_pz, "kp_pz/D");
  tree->Branch("km_pz", &m_km_pz, "km_pz/D");
  tree->Branch("pip_pz", &m_pip_pz, "pip_pz/D");
  tree->Branch("pim_pz", &m_pim_pz, "pim_pz/D");

  tree->Branch("kp_pzerr", &m_kp_pzerr, "kp_pzerr/D");
  tree->Branch("km_pzerr", &m_km_pzerr, "km_pzerr/D");
  tree->Branch("pip_pzerr", &m_pip_pzerr, "pip_pzerr/D");
  tree->Branch("pim_pzerr", &m_pim_pzerr, "pim_pzerr/D");

  tree->Branch("kp_costh", &m_kp_costh, "kp_costh/D");
  tree->Branch("km_costh", &m_km_costh, "km_costh/D");
  tree->Branch("pip_costh", &m_pip_costh, "pip_costh/D");
  tree->Branch("pim_costh", &m_pim_costh, "pim_costh/D");

  tree->Branch("ks_mindr", &m_ks_mindr, "ks_mindr/D");
  tree->Branch("ks_maxdr", &m_ks_maxdr, "ks_maxdr/D");
  tree->Branch("ks_mindz", &m_ks_mindz, "ks_mindz/D");
  tree->Branch("ks_maxdz", &m_ks_maxdz, "ks_maxdz/D");

  tree->Branch("ksmass", &m_ksmass, "ksmass/D");
  tree->Branch("ksptot", &m_ksptot, "ksptot/D");
  tree->Branch("kspt", &m_kspt, "kspt/D");
  tree->Branch("kspz", &m_kspz, "kspz/D");

  tree->Branch("phimass", &m_phimass, "phimass/D");
  tree->Branch("genphimass", &m_genphimass, "genphimass/D");

  tree->Branch("b0mass", &m_b0mass, "b0mass/D");
  tree->Branch("deltae", &m_deltae, "deltae/D");
  tree->Branch("mbc", &m_mbc, "mbc/D");
}

void B2PhiKsModule::initGeneratedInfo()
{
  m_genBtag     = 0;
  m_genB_deltat = -999.;
  m_genB_ptot   = 0.0;
  m_genPhi_ptot = 0.0;
  m_genKs_ptot  = 0.0;
  m_genKs_pt    = 0.0;
  m_genKs_pz    = 0.0;
  m_genPhi_helicity = -999.;
  m_genKp_ptot  = 0.0;
  m_genKp_pt    = 0.0;
  m_genKp_pz    = 0.0;
  m_genKp_phi   = -999.;
  m_genKp_costh = -999.;
  m_genKm_ptot  = 0.0;
  m_genKm_pt    = 0.0;
  m_genKm_pz    = 0.0;
  m_genKm_phi   = -999.;
  m_genKm_costh = -999.;
  m_genPip_ptot  = 0.0;
  m_genPip_pt    = 0.0;
  m_genPip_pz    = 0.0;
  m_genPip_costh = -999.;
  m_genPim_ptot  = 0.0;
  m_genPim_pt    = 0.0;
  m_genPim_pz    = 0.0;
  m_genPim_costh = -999.;

  m_genphimass = -999.;
  m_genKs_fl = -999.;
}

void B2PhiKsModule::initReconstructedInfo()
{
  m_kpTrackFound = 0;
  m_kmTrackFound = 0;
  m_pipTrackFound = 0;
  m_pimTrackFound = 0;

  m_kpTrackFitted = 0;
  m_kmTrackFitted = 0;
  m_pipTrackFitted = 0;
  m_pimTrackFitted = 0;
  m_pipTrackExtrapolated = 0;
  m_pimTrackExtrapolated = 0;

  m_kpTrackCDCRecoHits = 0;
  m_kmTrackCDCRecoHits = 0;
  m_pipTrackCDCRecoHits = 0;
  m_pimTrackCDCRecoHits = 0;

  m_kpTrackSVDRecoHits = 0;
  m_kmTrackSVDRecoHits = 0;
  m_pipTrackSVDRecoHits = 0;
  m_pimTrackSVDRecoHits = 0;

  m_kpTrackPXDRecoHits = 0;
  m_kmTrackPXDRecoHits = 0;
  m_pipTrackPXDRecoHits = 0;
  m_pimTrackPXDRecoHits = 0;

  m_kpTrackCDCHits = 0;
  m_kmTrackCDCHits = 0;
  m_pipTrackCDCHits = 0;
  m_pimTrackCDCHits = 0;

  m_kpTrackSVDHits = 0;
  m_kmTrackSVDHits = 0;
  m_pipTrackSVDHits = 0;
  m_pimTrackSVDHits = 0;

  m_kpTrackPXDHits = 0;
  m_kmTrackPXDHits = 0;
  m_pipTrackPXDHits = 0;
  m_pimTrackPXDHits = 0;

  m_kpTrackD0 = 0;
  m_kmTrackD0 = 0;
  m_pipTrackD0 = 0;
  m_pimTrackD0 = 0;

  m_kpTOPKPiDeltaLogL = 0;
  m_kmTOPKPiDeltaLogL = 0;
  m_pipTOPKPiDeltaLogL = 0;
  m_pimTOPKPiDeltaLogL = 0;

  m_kpARICHKPiDeltaLogL = 0;
  m_kmARICHKPiDeltaLogL = 0;
  m_pipARICHKPiDeltaLogL = 0;
  m_pimARICHKPiDeltaLogL = 0;

  m_kpTrackZ0 = 0;
  m_kmTrackZ0 = 0;
  m_pipTrackZ0 = 0;
  m_pimTrackZ0 = 0;

  m_kpTrackPValue = 0;
  m_kmTrackPValue = 0;
  m_pipTrackPValue = 0;
  m_pimTrackPValue = 0;

  m_kp_ptot = 0;
  m_km_ptot = 0;
  m_pip_ptot = 0;
  m_pim_ptot = 0;

  m_kp_pt = 0;
  m_km_pt = 0;
  m_pip_pt = 0;
  m_pim_pt = 0;

  m_kp_pterr = 0;
  m_km_pterr = 0;
  m_pip_pterr = 0;
  m_pim_pterr = 0;

  m_kp_pz = 0;
  m_km_pz = 0;
  m_pip_pz = 0;
  m_pim_pz = 0;

  m_kp_pzerr = 0;
  m_km_pzerr = 0;
  m_pip_pzerr = 0;
  m_pim_pzerr = 0;

  m_kp_costh = 0;
  m_km_costh = 0;
  m_pip_costh = 0;
  m_pim_costh = 0;

  m_ks_mindr = -999.;
  m_ks_maxdr = -999.;
  m_ks_mindz = -999.;
  m_ks_maxdz = -999.;

  m_ksmass = -999.;
  m_ksptot = -999.;
  m_kspt   = -999.;
  m_kspz   = -999.;

  m_phimass = -999.;

  m_b0mass = -999.;
  m_deltae = -999.;
  m_mbc    = -999.;
}


void B2PhiKsModule::dumpParticleGeneratorInfo(MCParticle* part)
{
  B2INFO(part->getPDG() << " [" <<  part->getArrayIndex()
         << "] px = " << part->getMomentum().Px()
         << "; py = " << part->getMomentum().Py()
         << "; pz = " << part->getMomentum().Pz()
         << "; cosTheta = " << part->getMomentum().CosTheta());
}

void B2PhiKsModule::fillGeneratedInfo(int signalB0Decay, int signalPhiDecay, int signalKshortDecay)
{
  initGeneratedInfo();

  if (signalB0Decay) {
    // B0sig and B0tag related info
    m_genBtag = B0Tag->getPDG();
    m_genB_deltat = (B0Sig->getDecayTime() - B0Tag->getDecayTime()) * 1000.0; // conversion to ps
    m_genB_ptot = B0Sig->getMomentum().Mag();

    if (signalPhiDecay) {
      // Phi related info
      m_genphimass = PhiSig->getMass();
      m_genPhi_ptot = PhiSig->getMomentum().Mag();

      TLorentzVector phiLV = PhiSig->get4Vector();
      TLorentzVector kpLV  = KpSig->get4Vector();
      TLorentzVector b0LV  = B0Sig->get4Vector();
      m_genPhi_helicity = cosHelicityAngle(kpLV, phiLV, b0LV);

      m_genKp_ptot  = KpSig->getMomentum().Mag();
      m_genKp_pt    = KpSig->getMomentum().Perp();
      m_genKp_pz    = KpSig->getMomentum().Pz();
      m_genKp_phi   = KpSig->getMomentum().Phi();
      m_genKp_costh = KpSig->getMomentum().CosTheta();

      m_genKm_ptot  = KmSig->getMomentum().Mag();
      m_genKm_pt    = KmSig->getMomentum().Perp();
      m_genKm_pz    = KmSig->getMomentum().Pz();
      m_genKm_phi   = KmSig->getMomentum().Phi();
      m_genKm_costh = KmSig->getMomentum().CosTheta();
    }

    if (signalKshortDecay) {
      m_genKs_ptot = KshortSig->getMomentum().Mag();
      m_genKs_pt = KshortSig->getMomentum().Perp();
      m_genKs_pz = KshortSig->getMomentum().Pz();

      m_genPip_ptot  = PipSig->getMomentum().Mag();
      m_genPip_pt    = PipSig->getMomentum().Perp();
      m_genPip_pz    = PipSig->getMomentum().Pz();
      m_genPip_costh = PipSig->getMomentum().CosTheta();

      m_genPim_ptot  = PimSig->getMomentum().Mag();
      m_genPim_pt    = PimSig->getMomentum().Perp();
      m_genPim_pz    = PimSig->getMomentum().Pz();
      m_genPim_costh = PimSig->getMomentum().CosTheta();

      m_genKs_fl   = (KshortSig->getDecayVertex() - KshortSig->getProductionVertex()).Mag();
    }
  }

}

