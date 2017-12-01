/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bianca Scavino                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackingPerformanceEvaluation/EffPlotsModule.h>
#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationVector.h>
#include <framework/datastore/RelationsObject.h>

#include <framework/geometry/BFieldManager.h>

#include <vxd/geometry/GeoCache.h>

#include <genfit/Track.h>
#include <genfit/TrackCand.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <root/TTree.h>
#include <root/TAxis.h>
#include <root/TObject.h>

#include <boost/foreach.hpp>
#include <vector>
#include <utility>

#include <typeinfo>
#include <cxxabi.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(EffPlots)

EffPlotsModule::EffPlotsModule() :
  Module()
{

  setDescription("This module makes some plots related to V0 and saves them into a root file. For the efficiency plots: _noGeoAcc -> normalized to MCParticles; _withGeoAcc -> normalized to RecoTracks.");

  addParam("outputFileName", m_rootFileName, "Name of output root file.",
           std::string("EffPlots_output.root"));
  addParam("V0sName", m_V0sName, "Name of V0 collection.", std::string("V0ValidationVertexs"));
  addParam("MCParticlesName", m_MCParticlesName, "Name of MC Particle collection.", std::string(""));
  addParam("TrackColName", m_TrackColName,
           "Belle2::Track collection name (input).  Note that the V0s use "
           "pointers indices into these arrays, so all hell may break loose "
           "if you change this.", std::string(""));
  addParam("RecoTracksName", m_RecoTracksName, "Name of RecoTrack collection.", std::string("RecoTracks"));
  addParam("MCRecoTracksName", m_MCRecoTracksName, "Name of MCRecoTrack collection.", std::string("MCRecoTracks"));

  addParam("V0sType", m_V0sType, "Type of V0 to perform plots. Default is Lambda0, alternatively Ks", std::string("Lambda0"));
  addParam("AllHistograms", m_allHistograms, "Create output for all histograms, not only efficiencies.", bool(false));
  addParam("GeometricalAccettance", m_geometricalAccettance, "Create output for geometrical accettance.", bool(false));
}

EffPlotsModule::~EffPlotsModule()
{

}

void EffPlotsModule::initialize()
{

  StoreArray<MCParticle>::required(m_MCParticlesName);
  StoreArray<V0ValidationVertex>::required(m_V0sName);

  StoreArray<TrackFitResult> trackFitResults(m_TFRColName);
  trackFitResults.isRequired();

  StoreArray<Track> tracks(m_TrackColName);
  tracks.isRequired();

  StoreArray<RecoTrack>::required(m_MCRecoTracksName);

  //create list of histograms to be saved in the rootfile
  m_histoList = new TList;
  m_histoList_MCParticles = new TList;
  m_histoList_RecoTracks = new TList;
  m_histoList_Tracks = new TList;
  m_histoList_Efficiencies = new TList;
  m_histoList_GA = new TList;
  m_histoList_check = new TList;

  //set the ROOT File
  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  Double_t bins_pt_new[25 + 1] = {0., 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 1.2, 1.4, 1.6, 1.8, 2, 2.2, 2.4, 2.6, 2.8, 3., 3.2, 3.4};

  Double_t bins_theta[10 + 1];
  Double_t width_theta = TMath::Pi() / 10;
  for (unsigned int bin = 0; bin < 10 + 1; bin++)
    bins_theta[bin] = bin * width_theta;

  Double_t bins_phi[14 + 1];
  Double_t width_phi = 2 * TMath::Pi() / 14;
  for (unsigned int bin = 0; bin < 14 + 1; bin++)
    bins_phi[bin] = - TMath::Pi() + bin * width_phi;

  Double_t bins_costheta[20 + 1];
  Double_t width_cosTheta = 2. / 20;
  for (unsigned int bin1 = 0; bin1 < 20 + 1; bin1++)
    bins_costheta[bin1] = - 1 + bin1 * width_cosTheta;

  //create histograms

  //------------------------------------------------------------------//
  //                          MC PARTICLES                            //
  //------------------------------------------------------------------//

  // MC dau0
  m_h1_MC_dau0_d0 = createHistogram1D("h1MCdau0D0", "d0 dau_{0}", 100, -10, 10, "d0_{dau_{0}}", m_histoList_MCParticles);
  m_h1_MC_dau0_z0 = createHistogram1D("h1MCdau0Z0", "z0 dau_{0}", 100, -10, 10, "z0_{dau_{0}}", m_histoList_MCParticles);
  m_h1_MC_dau0_RMother = createHistogram1D("h1MCdau0RMother", "dau_{0}, R mother", 200, 0, 20, "R mother", m_histoList_MCParticles);
  m_h3_MC_dau0 = createHistogram3D("h3MCdau0", "entry per MC dau_{0}",
                                   25, bins_pt_new, "p_{t} (GeV/c)",
                                   10, bins_theta, "#theta",
                                   14, bins_phi, "#phi", m_histoList_MCParticles);
  m_h1_MC_dau0_pt = createHistogram1D("h1MCdau0Pt", "dau_{0}, p_{T}", 25, bins_pt_new, "p_{T} (GeV/c)", m_histoList_MCParticles);
  m_h1_MC_dau0_pz = createHistogram1D("h1MCdau0Pz", "dau_{0}, p_{z}", 25, bins_pt_new, "p_{z} (GeV/c)", m_histoList_MCParticles);
  m_h1_MC_dau0_p = createHistogram1D("h1MCdau0P", "dau_{0}, p", 25, bins_pt_new, "p (GeV/c)", m_histoList_MCParticles);
  m_h1_MC_dau0_theta = createHistogram1D("h1MCdau0Theta", "dau_{0}, #theta", 10, bins_theta, "#theta",
                                         m_histoList_MCParticles);
  m_h1_MC_dau0_costheta = createHistogram1D("h1MCdau0CosTheta", "dau_{0}, cos#theta", 20, bins_costheta, "cos#theta",
                                            m_histoList_MCParticles);
  m_h1_MC_dau0_Mother_cosAngle = createHistogram1D("h1MCdau0MothercosAngle", "cos#theta_{mother,dau_{0}}", 20,
                                                   bins_costheta, "cos#theta", m_histoList_MCParticles);
  m_h1_MC_dau0_phi = createHistogram1D("h1MCdau0Phi", "dau_{0}, #phi", 14, bins_phi, "#phi", m_histoList_MCParticles);
  m_h1_MC_dau0_phi_BW = createHistogram1D("h1MCdau0PhiBW", "dau_{0}, #phi", 14, bins_phi, "#phi", m_histoList_MCParticles);
  m_h1_MC_dau0_phi_barrel = createHistogram1D("h1MCdau0Phibarrel", "dau_{0}, #phi", 14, bins_phi, "#phi",
                                              m_histoList_MCParticles);
  m_h1_MC_dau0_phi_FW = createHistogram1D("h1MCdau0PhiFW", "dau_{0}, #phi", 14, bins_phi, "#phi", m_histoList_MCParticles);

  m_h1_MC_dau0_phiMother_total = createHistogram1D("h1MCdau0phiMothertotal", "dau_{0}, #phi_{mother}", 14, bins_phi,
                                                   "#phi_{mother}", m_histoList_MCParticles);
  m_h1_MC_dau0_phiMother_BW = createHistogram1D("h1MCdau0phiMotherBW", "dau_{0}, #phi_{mother}, BW", 14, bins_phi,
                                                "#phi_{mother} BW", m_histoList_MCParticles);
  m_h1_MC_dau0_phiMother_barrel = createHistogram1D("h1MCdau0phiMotherbarrel", "dau_{0}, #phi_{mother}, barrel", 14, bins_phi,
                                                    "#phi_{mother} barrel", m_histoList_MCParticles);
  m_h1_MC_dau0_phiMother_FW = createHistogram1D("h1MCdau0phiMotherFW", "dau_{0}, #phi_{mother}, FW", 14, bins_phi,
                                                "#phi_{mother} FW", m_histoList_MCParticles);

  m_h1_MC_dau0_thetaMother = createHistogram1D("h1MCdau0ThetaMother", "dau_{0}, #theta_{mother}", 10, bins_theta,
                                               "#theta_{mother}", m_histoList_MCParticles);
  m_h1_MC_dau0_ptMother = createHistogram1D("h1MCdau0PtMother", "dau_{0}, p_{T,mother}", 25, bins_pt_new, "p_{T,mother} (GeV/c)",
                                            m_histoList_MCParticles);

  m_h2_MC_dau0_2D = createHistogram2D("h2MCdau0", "entry per MC dau_{0}",
                                      10, bins_theta, "#theta",
                                      25, bins_pt_new, "p_{t} (GeV/c)", m_histoList_MCParticles);
  m_h2_MC_dau0_2D_BP = createHistogram2D("h2MCdau0BP", "entry per MC dau_{0}, beam pipe",
                                         10, bins_theta, "#theta",
                                         25, bins_pt_new, "p_{t} (GeV/c)", m_histoList_MCParticles);

  m_h2_MC_dau0_2DMother = createHistogram2D("h2MCdau0Mother", "entry mother per MC dau_{0}",
                                            10, bins_theta, "#theta_{mother}",
                                            25, bins_pt_new, "p_{T,mother} (GeV/c)", m_histoList_MCParticles);

  m_h2_MC_dau0_pVScostheta = createHistogram2D("h2MCdau0pVScostheta", "p_{CM} VS cos(#theta)_{CM}, dau_{0}",
                                               20, -1., 1.,
                                               "cos(#theta)_{CM}",
                                               50, 0., 5.,
                                               "p_{CM} [GeV]", m_histoList_MCParticles);

  m_h1_MC_dau0_PDG = createHistogram1D("h1MCdau0PDG", "PDG code, dau_{0}", 4600, -2300, 2300, "PDG", m_histoList_check);

  //MC dau1
  m_h1_MC_dau1_d0 = createHistogram1D("h1MCdau1D0", "d0 dau_{1}", 100, -10, 10, "d0_{dau_{1}}", m_histoList_MCParticles);
  m_h1_MC_dau1_z0 = createHistogram1D("h1MCdau1Z0", "z0 dau_{1}", 100, -10, 10, "z0_{dau_{1}}", m_histoList_MCParticles);
  m_h1_MC_dau1_RMother = createHistogram1D("h1MCdau1RMother", "dau_{1}, R mother", 200, 0, 20, "R mother", m_histoList_MCParticles);
  m_h3_MC_dau1 = createHistogram3D("h3MCdau1", "entry per MC dau_{1}",
                                   25, bins_pt_new, "p_{t} (GeV/c)",
                                   10, bins_theta, "#theta",
                                   14, bins_phi, "#phi", m_histoList_MCParticles);
  m_h1_MC_dau1_pt = createHistogram1D("h1MCdau1Pt", "dau_{1}, p_{T}", 25, bins_pt_new, "p_{T} (GeV/c)", m_histoList_MCParticles);
  m_h1_MC_dau1_pz = createHistogram1D("h1MCdau1Pz", "dau_{1}, p_{z}", 25, bins_pt_new, "p_{z} (GeV/c)", m_histoList_MCParticles);
  m_h1_MC_dau1_p = createHistogram1D("h1MCdau1P", "dau_{1}, ", 25, bins_pt_new, "p (GeV/c)", m_histoList_MCParticles);
  m_h1_MC_dau1_theta = createHistogram1D("h1MCdau1Theta", "dau_{1}, #theta", 10, bins_theta, "#theta",
                                         m_histoList_MCParticles);
  m_h1_MC_dau1_costheta = createHistogram1D("h1MCdau1CosTheta", "dau_{1}, cos#theta", 20, bins_costheta, "cos#theta",
                                            m_histoList_MCParticles);
  m_h1_MC_dau1_phi = createHistogram1D("h1MCdau1Phi", "dau_{1}, #phi", 14, bins_phi, "#phi", m_histoList_MCParticles);
  m_h1_MC_dau1_phi_BW = createHistogram1D("h1MCdau1PhiBW", "dau_{1}, #phi", 14, bins_phi, "#phi", m_histoList_MCParticles);
  m_h1_MC_dau1_phi_barrel = createHistogram1D("h1MCdau1Phibarrel", "dau_{1}, #phi", 14, bins_phi, "#phi",
                                              m_histoList_MCParticles);
  m_h1_MC_dau1_phi_FW = createHistogram1D("h1MCdau1PhiFW", "dau_{1}, #phi", 14, bins_phi, "#phi", m_histoList_MCParticles);

  m_h1_MC_dau1_Mother_cosAngle = createHistogram1D("h1MCdau1MothercosAngle", "cos#theta_{mother,p}", 20, bins_costheta,
                                                   "cos#theta", m_histoList_MCParticles);

  m_h1_MC_dau1_phiMother_total = createHistogram1D("h1MCdau1phiMothertotal", "dau_{1}, #phi_{mother}", 14, bins_phi,
                                                   "#phi_{mother}", m_histoList_MCParticles);
  m_h1_MC_dau1_phiMother_BW = createHistogram1D("h1MCdau1phiMotherBW", "dau_{1}, #phi_{mother}, BW", 14, bins_phi,
                                                "#phi_{mother} BW", m_histoList_MCParticles);
  m_h1_MC_dau1_phiMother_barrel = createHistogram1D("h1MCdau1phiMotherbarrel", "dau_{1}, #phi_{mother}, barrel", 14, bins_phi,
                                                    "#phi_{mother} barrel", m_histoList_MCParticles);
  m_h1_MC_dau1_phiMother_FW = createHistogram1D("h1MCdau1phiMotherFW", "dau_{1}, #phi_{mother}, FW", 14, bins_phi,
                                                "#phi_{mother} FW", m_histoList_MCParticles);

  m_h1_MC_dau1_thetaMother = createHistogram1D("h1MCdau1ThetaMother", "dau_{1}, #theta_{mother}", 10, bins_theta,
                                               "#theta_{mother}", m_histoList_MCParticles);
  m_h1_MC_dau1_ptMother = createHistogram1D("h1MCdau1PtMother", "dau_{1}, p_{T,mother}", 25, bins_pt_new, "p_{T,mother} (GeV/c)",
                                            m_histoList_MCParticles);

  m_h2_MC_dau1_2D = createHistogram2D("h2MCdau1", "entry per MC dau_{1}",
                                      10, bins_theta, "#theta",
                                      25, bins_pt_new, "p_{t} (GeV/c)", m_histoList_MCParticles);

  m_h2_MC_dau1_2D_BP = createHistogram2D("h2MCdau1BP", "entry per MC dau_{1}, beam pipe",
                                         10, bins_theta, "#theta",
                                         25, bins_pt_new, "p_{t} (GeV/c)", m_histoList_MCParticles);

  m_h2_MC_dau1_2DMother = createHistogram2D("h2MCdau1Mother", "entry mother per MC dau_{1}",
                                            10, bins_theta, "#theta_{mother}",
                                            25, bins_pt_new, "p_{T,mother} (GeV/c)", m_histoList_MCParticles);

  m_h2_MC_dau1_pVScostheta = createHistogram2D("h2MCdau1pVScostheta", "p_{CM} VS cos(#theta)_{CM}, dau_{1}",
                                               20, -1., 1.,
                                               "cos(#theta)_{CM}",
                                               50, 0., 5.,
                                               "p_{CM} [GeV]", m_histoList_MCParticles);

  m_h1_MC_dau1_PDG = createHistogram1D("h1MCdau1PDG", "PDG code, dau_{1}", 4600, -2300, 2300, "PDG", m_histoList_check);

  //MC mother
  m_h1_MC_Mother_RMother = createHistogram1D("h1MCMotherRMother", "mother, R mother", 200, 0, 20, "R mother",
                                             m_histoList_MCParticles);
  m_h3_MC_Mother = createHistogram3D("h3MCMother", "entry per MCmother",
                                     25, bins_pt_new, "p_{t} (GeV/c)",
                                     10, bins_theta, "#theta",
                                     14, bins_phi, "#phi", m_histoList_MCParticles);
  m_h1_MC_Mother_pt = createHistogram1D("h1MCMotherPt", "mother, p_{T}", 25, bins_pt_new, "p_{T} (GeV/c)",
                                        m_histoList_MCParticles);
  m_h1_MC_Mother_pz = createHistogram1D("h1MCMotherPz", "mother, p_{z}", 25, bins_pt_new, "p_{z} (GeV/c)",
                                        m_histoList_MCParticles);
  m_h1_MC_Mother_p = createHistogram1D("h1MCMotherP", "mother, p", 25, bins_pt_new, "p (GeV/c)", m_histoList_MCParticles);
  m_h1_MC_Mother_theta = createHistogram1D("h1MCMotherTheta", "mother, #theta", 10, bins_theta, "#theta",
                                           m_histoList_MCParticles);
  m_h1_MC_Mother_costheta = createHistogram1D("h1MCMotherCosTheta", "mother, cos#theta", 20, bins_costheta, "cos#theta",
                                              m_histoList_MCParticles);
  m_h1_MC_Mother_phi = createHistogram1D("h1MCMotherPhi", "mother, #phi", 14, bins_phi, "#phi", m_histoList_MCParticles);
  m_h1_MC_Mother_phi_BW = createHistogram1D("h1MCMotherPhiBW", "mother, #phi", 14, bins_phi, "#phi", m_histoList_MCParticles);
  m_h1_MC_Mother_phi_barrel = createHistogram1D("h1MCMotherPhibarrel", "mother, #phi", 14, bins_phi, "#phi",
                                                m_histoList_MCParticles);
  m_h1_MC_Mother_phi_FW = createHistogram1D("h1MCMotherPhiFW", "mother, #phi", 14, bins_phi, "#phi", m_histoList_MCParticles);

  m_h2_MC_Mother_2D = createHistogram2D("h2MCMother", "entry per MCmother",
                                        10, bins_theta, "#theta",
                                        25, bins_pt_new, "p_{t} (GeV/c)", m_histoList_MCParticles);

  m_h2_MC_Mother_2D_BP = createHistogram2D("h2MCMotherBP", "entry per MCmother, beam pipe",
                                           10, bins_theta, "#theta",
                                           25, bins_pt_new, "p_{t} (GeV/c)", m_histoList_MCParticles);

  m_h2_MC_Mother_pVScostheta = createHistogram2D("h2MCMotherpVScostheta", "p_{CM} VS cos(#theta)_{CM}, mother",
                                                 20, -1., 1.,
                                                 "cos(#theta)_{CM}",
                                                 50, 0., 5.,
                                                 "p_{CM} [GeV]", m_histoList_MCParticles);

  m_h1_MC_Mother_PDG = createHistogram1D("h1MCMotherPDG", "PDG code, mother", 6400, -3200, 3200, "PDG", m_histoList_check);


  //------------------------------------------------------------------//
  //                          TRACKS                                  //
  //------------------------------------------------------------------//

  //track dau0
  m_h1_track_dau0_d0 = (TH1F*)duplicateHistogram("h1trackdau0D0", "d0 dau_{0}", m_h1_MC_dau0_d0, m_histoList_Tracks);
  m_h1_track_dau0_z0 = (TH1F*)duplicateHistogram("h1trackdau0Z0", "z0 dau_{0}", m_h1_MC_dau0_z0, m_histoList_Tracks);
  m_h1_track_dau0_RMother = (TH1F*)duplicateHistogram("h1trackdau0RMother", "dau_{0}, R mother", m_h1_MC_dau0_RMother,
                                                      m_histoList_Tracks);
  m_h3_track_dau0 = (TH3F*)duplicateHistogram("h3trackdau0", "entry per track dau_{0}", m_h3_MC_dau0, m_histoList_Tracks);
  m_h1_track_dau0_pt = (TH1F*)duplicateHistogram("h1trackdau0Pt", "p_{T} dau_{0}", m_h1_MC_dau0_pt, m_histoList_Tracks);
  m_h1_track_dau0_pz = (TH1F*)duplicateHistogram("h1trackdau0Pz", "p_{z} dau_{0}", m_h1_MC_dau0_pz, m_histoList_Tracks);
  m_h1_track_dau0_p = (TH1F*)duplicateHistogram("h1trackdau0P", "p dau_{0}", m_h1_MC_dau0_p, m_histoList_Tracks);
  m_h1_track_dau0_theta = (TH1F*)duplicateHistogram("h1trackdau0Theta", "#theta dau_{0}", m_h1_MC_dau0_theta, m_histoList_Tracks);
  m_h1_track_dau0_costheta = (TH1F*)duplicateHistogram("h1trackdau0CosTheta", "cos#theta dau_{0}", m_h1_MC_dau0_costheta,
                                                       m_histoList_Tracks);
  m_h1_track_dau0_Mother_cosAngle = (TH1F*)duplicateHistogram("h1trackdau0MothercosAngle", "#alpha_{mother,dau_{0}}",
                                                              m_h1_MC_dau0_Mother_cosAngle, m_histoList_Tracks);
  m_h1_track_dau0_phi = (TH1F*)duplicateHistogram("h1trackdau0Phi", "#phi dau_{0}", m_h1_MC_dau0_phi, m_histoList_Tracks);
  m_h1_track_dau0_phi_BW = (TH1F*)duplicateHistogram("h1trackdau0PhiBW", "#phi dau_{0}", m_h1_MC_dau0_phi_BW, m_histoList_Tracks);
  m_h1_track_dau0_phi_barrel = (TH1F*)duplicateHistogram("h1trackdau0Phibarrel", "#phi dau_{0}", m_h1_MC_dau0_phi_barrel,
                                                         m_histoList_Tracks);
  m_h1_track_dau0_phi_FW = (TH1F*)duplicateHistogram("h1trackdau0PhiFW", "#phi dau_{0}", m_h1_MC_dau0_phi_FW, m_histoList_Tracks);

  m_h1_track_dau0_phiMother_total = (TH1F*)duplicateHistogram("h1trackdau0PhiMothertotal", "dau_{0}, #phi_{mother}",
                                                              m_h1_MC_dau0_phiMother_total, m_histoList_Tracks);
  m_h1_track_dau0_phiMother_BW = (TH1F*)duplicateHistogram("h1trackdau0PhiMotherBW", "dau_{0}, #phi_{mother}, BW",
                                                           m_h1_MC_dau0_phiMother_BW, m_histoList_Tracks);
  m_h1_track_dau0_phiMother_barrel = (TH1F*)duplicateHistogram("h1trackdau0PhiMotherbarrel", "dau_{0}, #phi_{mother}, barrel",
                                     m_h1_MC_dau0_phiMother_barrel, m_histoList_Tracks);
  m_h1_track_dau0_phiMother_FW = (TH1F*)duplicateHistogram("h1trackdau0PhiMotherFW", "dau_{0}, #phi_{mother}, FW",
                                                           m_h1_MC_dau0_phiMother_FW, m_histoList_Tracks);

  m_h1_track_dau0_thetaMother = (TH1F*)duplicateHistogram("h1trackdau0ThetaMother", "#theta_{mother} dau_{0}",
                                                          m_h1_MC_dau0_thetaMother, m_histoList_Tracks);
  m_h1_track_dau0_ptMother = (TH1F*)duplicateHistogram("h1trackdau0PtMother", "p_{T,mother} dau_{0}", m_h1_MC_dau0_ptMother,
                                                       m_histoList_Tracks);

  m_h2_track_dau0_2D = (TH2F*)duplicateHistogram("h2trackdau02D", "p_{T} VS #theta, dau_{0}", m_h2_MC_dau0_2D, m_histoList_Tracks);
  m_h2_track_dau0_2D_BP = (TH2F*)duplicateHistogram("h2trackdau02dBP", "p_{T} VS #theta, dau_{0} BP", m_h2_MC_dau0_2D_BP,
                                                    m_histoList_Tracks);
  m_h2_track_dau0_2DMother = (TH2F*)duplicateHistogram("h2trackdau02DMother", "p_{T,mother} VS #theta_{mother}, dau_{0}",
                                                       m_h2_MC_dau0_2DMother, m_histoList_Tracks);

  m_h2_track_dau0_pVScostheta = (TH2F*)duplicateHistogram("h2trackdau0pVScostheta", "p VS cos(#theta), dau_{0}",
                                                          m_h2_MC_dau0_pVScostheta, m_histoList_Tracks);


  //track dau1
  m_h1_track_dau1_d0 = (TH1F*)duplicateHistogram("h1trackdau1D0", "d0 dau_{1}", m_h1_MC_dau1_d0, m_histoList_Tracks);
  m_h1_track_dau1_z0 = (TH1F*)duplicateHistogram("h1trackdau1Z0", "z0 dau_{1}", m_h1_MC_dau1_z0, m_histoList_Tracks);
  m_h1_track_dau1_RMother = (TH1F*)duplicateHistogram("h1trackdau1RMother", "p, R mother", m_h1_MC_dau1_RMother, m_histoList_Tracks);
  m_h3_track_dau1 = (TH3F*)duplicateHistogram("h3trackdau1", "entry per track dau_{1}", m_h3_MC_dau1, m_histoList_Tracks);
  m_h1_track_dau1_pt = (TH1F*)duplicateHistogram("h1trackdau1Pt", "p_{T} dau_{1}", m_h1_MC_dau1_pt, m_histoList_Tracks);
  m_h1_track_dau1_pz = (TH1F*)duplicateHistogram("h1trackdau1Pz", "p_{z} dau_{1}", m_h1_MC_dau1_pz, m_histoList_Tracks);
  m_h1_track_dau1_p = (TH1F*)duplicateHistogram("h1trackdau1P", "p dau_{1}", m_h1_MC_dau1_p, m_histoList_Tracks);
  m_h1_track_dau1_theta = (TH1F*)duplicateHistogram("h1trackdau1Theta", "#theta dau_{1}", m_h1_MC_dau1_theta, m_histoList_Tracks);
  m_h1_track_dau1_costheta = (TH1F*)duplicateHistogram("h1trackdau1CosTheta", "cos#theta dau_{1}", m_h1_MC_dau1_costheta,
                                                       m_histoList_Tracks);
  m_h1_track_dau1_Mother_cosAngle = (TH1F*)duplicateHistogram("h1trackdau1MothercosAngle", "#alpha_{mother,p}",
                                                              m_h1_MC_dau1_Mother_cosAngle, m_histoList_Tracks);
  m_h1_track_dau1_phi = (TH1F*)duplicateHistogram("h1trackdau1Phi", "#phi dau_{1}", m_h1_MC_dau1_phi, m_histoList_Tracks);
  m_h1_track_dau1_phi_BW = (TH1F*)duplicateHistogram("h1trackdau1PhiBW", "#phi dau_{1}", m_h1_MC_dau1_phi_BW, m_histoList_Tracks);
  m_h1_track_dau1_phi_barrel = (TH1F*)duplicateHistogram("h1trackdau1Phibarrel", "#phi dau_{1}", m_h1_MC_dau1_phi_barrel,
                                                         m_histoList_Tracks);
  m_h1_track_dau1_phi_FW = (TH1F*)duplicateHistogram("h1trackdau1PhiFW", "#phi dau_{1}", m_h1_MC_dau1_phi_FW, m_histoList_Tracks);

  m_h1_track_dau1_phiMother_total = (TH1F*)duplicateHistogram("h1trackdau1PhiMothertotal", "dau_{1}, #phi_{mother}",
                                                              m_h1_MC_dau1_phiMother_total, m_histoList_Tracks);
  m_h1_track_dau1_phiMother_BW = (TH1F*)duplicateHistogram("h1trackdau1PhiMotherBW", "dau_{1}, #phi_{mother}, BW",
                                                           m_h1_MC_dau1_phiMother_BW, m_histoList_Tracks);
  m_h1_track_dau1_phiMother_barrel = (TH1F*)duplicateHistogram("h1trackdau1PhiMotherbarrel", "dau_{1}, #phi_{mother}, barrel",
                                     m_h1_MC_dau1_phiMother_barrel, m_histoList_Tracks);
  m_h1_track_dau1_phiMother_FW = (TH1F*)duplicateHistogram("h1trackdau1PhiMotherFW", "dau_{1}, #phi_{mother}, FW",
                                                           m_h1_MC_dau1_phiMother_FW, m_histoList_Tracks);

  m_h1_track_dau1_thetaMother = (TH1F*)duplicateHistogram("h1trackdau1ThetaMother", "#theta_{mother} dau_{1}",
                                                          m_h1_MC_dau1_thetaMother, m_histoList_Tracks);
  m_h1_track_dau1_ptMother = (TH1F*)duplicateHistogram("h1trackdau1PtMother", "p_{T,mother} dau_{1}", m_h1_MC_dau1_ptMother,
                                                       m_histoList_Tracks);

  m_h2_track_dau1_2D = (TH2F*)duplicateHistogram("h2trackdau12D", "p_{T} VS #theta, dau_{1}", m_h2_MC_dau1_2D, m_histoList_Tracks);
  m_h2_track_dau1_2D_BP = (TH2F*)duplicateHistogram("h2trackdau12dBP", "p_{T} VS #theta, p BDAU_{1}", m_h2_MC_dau1_2D_BP,
                                                    m_histoList_Tracks);
  m_h2_track_dau1_2DMother = (TH2F*)duplicateHistogram("h2trackdau12DMother", "p_{T,mother} VS #theta_{mother}, dau_{1}",
                                                       m_h2_MC_dau1_2DMother, m_histoList_Tracks);

  m_h2_track_dau1_pVScostheta = (TH2F*)duplicateHistogram("h2trackdau1pVScostheta", "p VS cos(#theta), dau_{1}",
                                                          m_h2_MC_dau1_pVScostheta, m_histoList_Tracks);

  //V0
  m_h1_V0_RMother = (TH1F*)duplicateHistogram("h1V0RMother", "mother, R mother", m_h1_MC_Mother_RMother, m_histoList_Tracks);
  m_h3_V0 = (TH3F*)duplicateHistogram("h3V0", "entry per V0", m_h3_MC_Mother, m_histoList_Tracks);
  m_h1_V0_pt = (TH1F*)duplicateHistogram("h1V0Pt", "p_{T} mother", m_h1_MC_Mother_pt, m_histoList_Tracks);
  m_h1_V0_pz = (TH1F*)duplicateHistogram("h1V0Pz", "p_{z} mother", m_h1_MC_Mother_pz, m_histoList_Tracks);
  m_h1_V0_p = (TH1F*)duplicateHistogram("h1V0P", "p mother", m_h1_MC_Mother_p, m_histoList_Tracks);
  m_h1_V0_theta = (TH1F*)duplicateHistogram("h1V0Theta", "#theta mother", m_h1_MC_Mother_theta, m_histoList_Tracks);
  m_h1_V0_costheta = (TH1F*)duplicateHistogram("h1V0CosTheta", "cos#theta mother", m_h1_MC_Mother_costheta, m_histoList_Tracks);
  m_h1_V0_phi = (TH1F*)duplicateHistogram("h1V0Phi", "#phi mother", m_h1_MC_Mother_phi, m_histoList_Tracks);
  m_h1_V0_phi_BW = (TH1F*)duplicateHistogram("h1V0PhiBW", "#phi mother", m_h1_MC_Mother_phi_BW, m_histoList_Tracks);
  m_h1_V0_phi_barrel = (TH1F*)duplicateHistogram("h1V0Phibarrel", "#phi mother", m_h1_MC_Mother_phi_barrel, m_histoList_Tracks);
  m_h1_V0_phi_FW = (TH1F*)duplicateHistogram("h1V0PhiFW", "#phi mother", m_h1_MC_Mother_phi_FW, m_histoList_Tracks);

  m_h2_V0_Mother_2D = (TH2F*)duplicateHistogram("h2V0Mother2D", "p_{T} VS #theta, mother", m_h2_MC_Mother_2D, m_histoList_Tracks);
  m_h2_V0_Mother_2D_BP = (TH2F*)duplicateHistogram("h2V0Mother2dBP", "p_{T} VS #theta, mother BP", m_h2_MC_Mother_2D_BP,
                                                   m_histoList_Tracks);

  m_h2_V0_Mother_pVScostheta = (TH2F*)duplicateHistogram("h2V0MotherpVScostheta", "p VS cos(#theta), mother",
                                                         m_h2_MC_Mother_pVScostheta, m_histoList_Tracks);


  //------------------------------------------------------------------//
  //                           RECO TRACKS                            //
  //------------------------------------------------------------------//

  //RecoTrack dau0
  m_h1_RecoTrack_dau0_d0 = (TH1F*)duplicateHistogram("h1RecoTrackdau0D0", "d0 dau_{0}", m_h1_MC_dau0_d0, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_z0 = (TH1F*)duplicateHistogram("h1RecoTrackdau0Z0", "z0 dau_{0}", m_h1_MC_dau0_z0, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_RMother = (TH1F*)duplicateHistogram("h1RecoTrackdau0RMother", "dau_{0}, R mother", m_h1_MC_dau0_RMother,
                                                          m_histoList_RecoTracks);
  m_h3_RecoTrack_dau0 = (TH3F*)duplicateHistogram("h3RecoTrackdau0", "entry per RecoTrack dau_{0}", m_h3_MC_dau0,
                                                  m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_pt = (TH1F*)duplicateHistogram("h1RecoTrackdau0Pt", "p_{T} dau_{0}", m_h1_MC_dau0_pt, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_pz = (TH1F*)duplicateHistogram("h1RecoTrackdau0Pz", "p_{z} dau_{0}", m_h1_MC_dau0_pz, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_p = (TH1F*)duplicateHistogram("h1RecoTrackdau0P", "p dau_{0}", m_h1_MC_dau0_p, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_theta = (TH1F*)duplicateHistogram("h1RecoTrackdau0Theta", "#theta dau_{0}", m_h1_MC_dau0_theta,
                                                        m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_costheta = (TH1F*)duplicateHistogram("h1RecoTrackdau0CosTheta", "cos#theta dau_{0}", m_h1_MC_dau0_costheta,
                                                           m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_Mother_cosAngle = (TH1F*)duplicateHistogram("h1RecoTrackdau0MothercosAngle", "#alpha_{mother,dau_{0}}",
                                        m_h1_MC_dau0_Mother_cosAngle, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_phi = (TH1F*)duplicateHistogram("h1RecoTrackdau0Phi", "#phi dau_{0}", m_h1_MC_dau0_phi, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_phi_BW = (TH1F*)duplicateHistogram("h1RecoTrackdau0PhiBW", "#phi dau_{0}", m_h1_MC_dau0_phi_BW,
                                                         m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_phi_barrel = (TH1F*)duplicateHistogram("h1RecoTrackdau0Phibarrel", "#phi dau_{0}", m_h1_MC_dau0_phi_barrel,
                                                             m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_phi_FW = (TH1F*)duplicateHistogram("h1RecoTrackdau0PhiFW", "#phi dau_{0}", m_h1_MC_dau0_phi_FW,
                                                         m_histoList_RecoTracks);

  m_h1_RecoTrack_dau0_phiMother_total = (TH1F*)duplicateHistogram("h1RecoTrackdau0PhiMothertotal", "dau_{0}, #phi_{mother}",
                                        m_h1_MC_Mother_phi, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_phiMother_BW = (TH1F*)duplicateHistogram("h1RecoTrackdau0PhiMotherBW", "dau_{0}, #phi_{mother}, BW",
                                     m_h1_MC_dau0_phiMother_BW, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_phiMother_barrel = (TH1F*)duplicateHistogram("h1RecoTrackdau0PhiMotherbarrel", "dau_{0}, #phi_{mother}, barrel",
                                         m_h1_MC_dau0_phiMother_barrel, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_phiMother_FW = (TH1F*)duplicateHistogram("h1RecoTrackdau0PhiMotherFW", "dau_{0}, #phi_{mother}, FW",
                                     m_h1_MC_dau0_phiMother_FW, m_histoList_RecoTracks);

  m_h1_RecoTrack_dau0_thetaMother = (TH1F*)duplicateHistogram("h1RecoTrackdau0ThetaMother", "#theta_{mother} dau_{0}",
                                                              m_h1_MC_dau0_theta, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau0_ptMother = (TH1F*)duplicateHistogram("h1RecoTrackdau0PtMother", "p_{T,mother} dau_{0}", m_h1_MC_dau0_pt,
                                                           m_histoList_RecoTracks);

  m_h2_RecoTrack_dau0_2D = (TH2F*)duplicateHistogram("h2RecoTrackdau02D", "p_{T} VS #theta, dau_{0}", m_h2_MC_dau0_2D,
                                                     m_histoList_RecoTracks);
  m_h2_RecoTrack_dau0_2D_BP = (TH2F*)duplicateHistogram("h2RecoTrackdau02dBP", "p_{T} VS #theta, dau_{0} BP", m_h2_MC_dau0_2D_BP,
                                                        m_histoList_RecoTracks);
  m_h2_RecoTrack_dau0_2DMother = (TH2F*)duplicateHistogram("h2RecoTrackdau02DMother", "p_{T,mother} VS #theta_{mother}, dau_{0}",
                                                           m_h2_MC_dau0_2DMother, m_histoList_RecoTracks);

  m_h2_RecoTrack_dau0_pVScostheta = (TH2F*)duplicateHistogram("h2RecoTrackdau0pVscostheta", "p VS cos(#theta), dau_{0}",
                                                              m_h2_MC_dau0_pVScostheta, m_histoList_RecoTracks);

  //RecoTrack dau1
  m_h1_RecoTrack_dau1_d0 = (TH1F*)duplicateHistogram("h1RecoTrackdau1D0", "d0 dau_{1}", m_h1_MC_dau1_d0, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_z0 = (TH1F*)duplicateHistogram("h1RecoTrackdau1Z0", "z0 dau_{1}", m_h1_MC_dau1_z0, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_RMother = (TH1F*)duplicateHistogram("h1RecoTrackdau1RMother", "dau_{1}, R mother", m_h1_MC_dau1_RMother,
                                                          m_histoList_RecoTracks);
  m_h3_RecoTrack_dau1 = (TH3F*)duplicateHistogram("h3RecoTrackdau1", "entry per RecoTrack dau_{1}", m_h3_MC_dau1,
                                                  m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_pt = (TH1F*)duplicateHistogram("h1RecoTrackdau1Pt", "p_{T} dau_{1}", m_h1_MC_dau1_pt, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_pz = (TH1F*)duplicateHistogram("h1RecoTrackdau1Pz", "p_{z} dau_{1}", m_h1_MC_dau1_pz, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_p = (TH1F*)duplicateHistogram("h1RecoTrackdau1P", "p dau_{1}", m_h1_MC_dau1_p, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_theta = (TH1F*)duplicateHistogram("h1RecoTrackdau1Theta", "#theta dau_{1}", m_h1_MC_dau1_theta,
                                                        m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_costheta = (TH1F*)duplicateHistogram("h1RecoTrackdau1CosTheta", "cos#theta dau_{1}", m_h1_MC_dau1_costheta,
                                                           m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_Mother_cosAngle = (TH1F*)duplicateHistogram("h1RecoTrackdau1MothercosAngle", "#alpha_{mother,p}",
                                        m_h1_MC_dau1_Mother_cosAngle, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_phi = (TH1F*)duplicateHistogram("h1RecoTrackdau1Phi", "#phi dau_{1}", m_h1_MC_dau1_phi, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_phi_BW = (TH1F*)duplicateHistogram("h1RecoTrackdau1PhiBW", "#phi dau_{1}", m_h1_MC_dau1_phi_BW,
                                                         m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_phi_barrel = (TH1F*)duplicateHistogram("h1RecoTrackdau1Phibarrel", "#phi dau_{1}", m_h1_MC_dau1_phi_barrel,
                                                             m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_phi_FW = (TH1F*)duplicateHistogram("h1RecoTrackdau1PhiFW", "#phi dau_{1}", m_h1_MC_dau1_phi_FW,
                                                         m_histoList_RecoTracks);

  m_h1_RecoTrack_dau1_phiMother_total = (TH1F*)duplicateHistogram("h1RecoTrackdau1PhiMothertotal", "dau_{1}, #phi_{mother}",
                                        m_h1_MC_dau1_phiMother_total, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_phiMother_BW = (TH1F*)duplicateHistogram("h1RecoTrackdau1PhiMotherBW", "dau_{1}, #phi_{mother}, BW",
                                     m_h1_MC_dau1_phiMother_BW, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_phiMother_barrel = (TH1F*)duplicateHistogram("h1RecoTrackdau1PhiMotherbarrel", "dau_{1}, #phi_{mother}, barrel",
                                         m_h1_MC_dau1_phiMother_barrel, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_phiMother_FW = (TH1F*)duplicateHistogram("h1RecoTrackdau1PhiMotherFW", "dau_{1}, #phi_{mother}, FW",
                                     m_h1_MC_dau1_phiMother_FW, m_histoList_RecoTracks);

  m_h1_RecoTrack_dau1_thetaMother = (TH1F*)duplicateHistogram("h1RecoTrackdau1ThetaMother", "#theta_{mother} dau_{1}",
                                                              m_h1_MC_dau1_theta, m_histoList_RecoTracks);
  m_h1_RecoTrack_dau1_ptMother = (TH1F*)duplicateHistogram("h1RecoTrackdau1PtMother", "p_{T,mother} dau_{1}", m_h1_MC_dau1_pt,
                                                           m_histoList_RecoTracks);

  m_h2_RecoTrack_dau1_2D = (TH2F*)duplicateHistogram("h2RecoTrackdau12D", "p_{T} VS #theta, dau_{1}", m_h2_MC_dau1_2D,
                                                     m_histoList_RecoTracks);
  m_h2_RecoTrack_dau1_2D_BP = (TH2F*)duplicateHistogram("h2RecoTrackdau12dBP", "p_{T} VS #theta, p BP", m_h2_MC_dau1_2D_BP,
                                                        m_histoList_RecoTracks);
  m_h2_RecoTrack_dau1_2DMother = (TH2F*)duplicateHistogram("h2RecoTrackdau12DMother", "p_{T,mother} VS #theta_{mother}, dau_{1}",
                                                           m_h2_MC_dau1_2DMother, m_histoList_RecoTracks);

  m_h2_RecoTrack_dau1_pVScostheta = (TH2F*)duplicateHistogram("h2RecoTrackdau1pVscostheta", "p VS cos(#theta), dau_{1}",
                                                              m_h2_MC_dau1_pVScostheta, m_histoList_RecoTracks);

  //RecoTrack Mother
  m_h1_RecoTrack_Mother_RMother = (TH1F*)duplicateHistogram("h1RecoTrack_MotherRMother", "mother, R mother", m_h1_MC_Mother_RMother,
                                                            m_histoList_RecoTracks);
  m_h3_RecoTrack_Mother = (TH3F*)duplicateHistogram("h3RecoTrack_Mother", "entry per RecoTrack_Mother", m_h3_MC_Mother,
                                                    m_histoList_RecoTracks);
  m_h1_RecoTrack_Mother_pt = (TH1F*)duplicateHistogram("h1RecoTrack_MotherPt", "p_{T} mother", m_h1_MC_dau0_pt,
                                                       m_histoList_RecoTracks);
  m_h1_RecoTrack_Mother_pz = (TH1F*)duplicateHistogram("h1RecoTrack_MotherPz", "p_{z} mother", m_h1_MC_dau0_pz,
                                                       m_histoList_RecoTracks);
  m_h1_RecoTrack_Mother_p = (TH1F*)duplicateHistogram("h1RecoTrack_MotherP", "p mother", m_h1_MC_dau0_p, m_histoList_RecoTracks);
  m_h1_RecoTrack_Mother_theta = (TH1F*)duplicateHistogram("h1RecoTrack_MotherTheta", "#theta mother", m_h1_MC_dau0_theta,
                                                          m_histoList_RecoTracks);
  m_h1_RecoTrack_Mother_costheta = (TH1F*)duplicateHistogram("h1RecoTrack_MotherCosTheta", "cos#theta mother", m_h1_MC_dau0_costheta,
                                                             m_histoList_RecoTracks);
  m_h1_RecoTrack_Mother_phi = (TH1F*)duplicateHistogram("h1RecoTrack_MotherPhi", "#phi mother", m_h1_MC_dau0_phi,
                                                        m_histoList_RecoTracks);
  m_h1_RecoTrack_Mother_phi_BW = (TH1F*)duplicateHistogram("h1RecoTrack_MotherPhiBW", "#phi mother", m_h1_MC_dau0_phi_BW,
                                                           m_histoList_RecoTracks);
  m_h1_RecoTrack_Mother_phi_barrel = (TH1F*)duplicateHistogram("h1RecoTrack_MotherPhibarrel", "#phi mother", m_h1_MC_dau0_phi_barrel,
                                     m_histoList_RecoTracks);
  m_h1_RecoTrack_Mother_phi_FW = (TH1F*)duplicateHistogram("h1RecoTrack_MotherPhiFW", "#phi mother", m_h1_MC_dau0_phi_FW,
                                                           m_histoList_RecoTracks);

  m_h2_RecoTrack_Mother_2D = (TH2F*)duplicateHistogram("h2RecoTrack_Mother2D", "p_{T} VS #theta, mother", m_h2_MC_Mother_2D,
                                                       m_histoList_RecoTracks);
  m_h2_RecoTrack_Mother_2D_BP = (TH2F*)duplicateHistogram("h2RecoTrack_Mother2dBP", "p_{T} VS #theta, mother BP",
                                                          m_h2_MC_Mother_2D_BP, m_histoList_RecoTracks);

  m_h2_RecoTrack_Mother_pVScostheta = (TH2F*)duplicateHistogram("h2RecoTrackMotherpVscostheta", "p VS cos(#theta), mother",
                                      m_h2_MC_Mother_pVScostheta, m_histoList_RecoTracks);

}

void EffPlotsModule::beginRun()
{

}

void EffPlotsModule::event()
{

  StoreArray<MCParticle> mcParticles(m_MCParticlesName);

  B2Vector3D magField = BFieldManager::getField(0, 0, 0) / Unit::T;

  B2DEBUG(99, "+++++ 1. loop on MCParticles");
  BOOST_FOREACH(MCParticle & mcParticle, mcParticles) {

    //------------------------------------------------------------------//
    //                          MC PARTICLES                            //
    //------------------------------------------------------------------//

    if (m_V0sType == "Lambda0") {
      if (! isLambda0(mcParticle))
        continue;

      std::vector< MCParticle* > MCPart_dau = mcParticle.getDaughters();

      if (abs(MCPart_dau[0]->getPDG()) == 211 && abs(MCPart_dau[1]->getPDG()) == 2212) {
        mc_dau0 = MCPart_dau[0];
        mc_dau1 = MCPart_dau[1];
      } else if (abs(MCPart_dau[0]->getPDG()) == 2212 && abs(MCPart_dau[1]->getPDG()) == 211) {
        mc_dau0 = MCPart_dau[1];
        mc_dau1 = MCPart_dau[0];
      } else B2INFO("Lambda daughters != pi & p");
    }

    else if (m_V0sType == "Ks") {
      if (! isK_Short(mcParticle))
        continue;

      std::vector< MCParticle* > MCPart_dau = mcParticle.getDaughters();

      if (MCPart_dau[0]->getPDG() == 211 && MCPart_dau[1]->getPDG() == -211) {
        mc_dau0 = MCPart_dau[0];
        mc_dau1 = MCPart_dau[1];
      } else if (MCPart_dau[0]->getPDG() == -211 && MCPart_dau[1]->getPDG() == 211) {
        mc_dau0 = MCPart_dau[1];
        mc_dau1 = MCPart_dau[0];
      } else B2INFO("Ks daughters != pi+ & pi-");
    }

    MCParticleInfo mcParticleInfo(mcParticle, magField);
    MCParticleInfo mcParticleInfo_dau0(*mc_dau0, magField);
    MCParticleInfo mcParticleInfo_dau1(*mc_dau1, magField);

    TVector3 MC_prodvtx = mcParticle.getVertex();
    TVector3 MC_vtx = mcParticle.getDecayVertex();
    TVector3 MC_FL = MC_vtx - MC_prodvtx;

    float MC_transDist = sqrt(MC_vtx.X() * MC_vtx.X() + MC_vtx.Y() * MC_vtx.Y());
    float MC_pt = mcParticle.getMomentum().Pt();
    float MC_p = mcParticle.getMomentum().Mag();
    float MC_phi = mcParticle.getMomentum().Phi();
    float MC_theta = mcParticle.getMomentum().Theta();
    float MC_costheta = mcParticle.getMomentum().CosTheta();

    m_h1_MC_dau0_d0->Fill(mcParticleInfo_dau0.getD0());
    m_h1_MC_dau0_z0->Fill(mcParticleInfo_dau0.getZ0());
    m_h1_MC_dau0_RMother->Fill(MC_transDist);
    m_h3_MC_dau0->Fill(mc_dau0->getMomentum().Pt(), mc_dau0->getMomentum().Theta(), mc_dau0->getMomentum().Phi());
    m_h1_MC_dau0_pt->Fill(mc_dau0->getMomentum().Pt());
    m_h1_MC_dau0_pz->Fill(mc_dau0->getMomentum().Pz());
    m_h1_MC_dau0_p->Fill(mc_dau0->getMomentum().Mag());
    m_h1_MC_dau0_phi->Fill(mc_dau0->getMomentum().Phi());
    m_h1_MC_dau0_theta->Fill(mc_dau0->getMomentum().Theta());
    m_h1_MC_dau0_costheta->Fill(mc_dau0->getMomentum().CosTheta());
    m_h1_MC_dau0_Mother_cosAngle->Fill(mcParticle.getMomentum() * mc_dau0->getMomentum() / mcParticle.getMomentum().Mag() /
                                       mc_dau0->getMomentum().Mag());

    m_h1_MC_dau0_thetaMother->Fill(MC_theta);
    m_h1_MC_dau0_ptMother->Fill(MC_pt);

    m_h1_MC_dau0_phiMother_total->Fill(MC_phi);
    m_h2_MC_dau0_2D->Fill(mc_dau0->getMomentum().Theta(), mc_dau0->getMomentum().Pt());
    m_h2_MC_dau0_2DMother->Fill(MC_theta, MC_pt);
    m_h2_MC_dau0_pVScostheta->Fill(mc_dau0->getMomentum().CosTheta(), mc_dau0->getMomentum().Mag());
    m_h1_MC_dau0_PDG->Fill(mc_dau0->getPDG());

    m_h1_MC_dau1_d0->Fill(mcParticleInfo_dau1.getD0());
    m_h1_MC_dau1_z0->Fill(mcParticleInfo_dau1.getZ0());
    m_h1_MC_dau1_RMother->Fill(MC_transDist);
    m_h3_MC_dau1->Fill(mc_dau1->getMomentum().Pt(), mc_dau1->getMomentum().Theta(), mc_dau1->getMomentum().Phi());
    m_h1_MC_dau1_pt->Fill(mc_dau1->getMomentum().Pt());
    m_h1_MC_dau1_pz->Fill(mc_dau1->getMomentum().Pz());
    m_h1_MC_dau1_p->Fill(mc_dau1->getMomentum().Mag());
    m_h1_MC_dau1_phi->Fill(mc_dau1->getMomentum().Phi());
    m_h1_MC_dau1_theta->Fill(mc_dau1->getMomentum().Theta());
    m_h1_MC_dau1_costheta->Fill(mc_dau1->getMomentum().CosTheta());
    m_h1_MC_dau1_Mother_cosAngle->Fill(mcParticle.getMomentum() * mc_dau1->getMomentum() / mcParticle.getMomentum().Mag() /
                                       mc_dau1->getMomentum().Mag());

    m_h1_MC_dau1_thetaMother->Fill(MC_theta);
    m_h1_MC_dau1_ptMother->Fill(MC_pt);

    m_h1_MC_dau1_phiMother_total->Fill(MC_phi);
    m_h2_MC_dau1_2D->Fill(mc_dau1->getMomentum().Theta(), mc_dau1->getMomentum().Pt());
    m_h2_MC_dau1_2DMother->Fill(MC_theta, MC_pt);
    m_h2_MC_dau1_pVScostheta->Fill(mc_dau1->getMomentum().CosTheta(), mc_dau1->getMomentum().Mag());
    m_h1_MC_dau1_PDG->Fill(mc_dau1->getPDG());

    m_h1_MC_Mother_RMother->Fill(MC_transDist);
    m_h3_MC_Mother->Fill(MC_pt, MC_theta, MC_phi);
    m_h1_MC_Mother_pt->Fill(MC_pt);
    m_h1_MC_Mother_pz->Fill(mcParticle.getMomentum().Pz());
    m_h1_MC_Mother_p->Fill(mcParticle.getMomentum().Mag());
    m_h1_MC_Mother_phi->Fill(MC_phi);
    m_h1_MC_Mother_theta->Fill(MC_theta);
    m_h1_MC_Mother_costheta->Fill(mcParticle.getMomentum().CosTheta());
    m_h2_MC_Mother_2D->Fill(MC_theta, MC_pt);
    m_h2_MC_Mother_pVScostheta->Fill(MC_costheta, MC_p);
    m_h1_MC_Mother_PDG->Fill(mcParticle.getPDG());

    //beam pipe
    if (MC_transDist < 1.) {
      m_h2_MC_dau0_2D_BP->Fill(mc_dau0->getMomentum().Theta(), mc_dau0->getMomentum().Pt());
      m_h2_MC_dau1_2D_BP->Fill(mc_dau1->getMomentum().Theta(), mc_dau1->getMomentum().Pt());
      m_h2_MC_Mother_2D_BP->Fill(MC_theta, MC_pt);
    }


    if (MC_theta > (120 * TMath::Pi() / 180.)) { //BW
      m_h1_MC_dau0_phiMother_BW->Fill(MC_phi);
      m_h1_MC_dau1_phiMother_BW->Fill(MC_phi);
      m_h1_MC_dau0_phi_BW->Fill(mc_dau0->getMomentum().Phi());
      m_h1_MC_dau1_phi_BW->Fill(mc_dau1->getMomentum().Phi());
      m_h1_MC_Mother_phi_BW->Fill(MC_phi);
    } else if (MC_theta < (30. * TMath::Pi() / 180.)) { //FW, theta < 30)
      m_h1_MC_dau0_phiMother_FW->Fill(MC_phi);
      m_h1_MC_dau1_phiMother_FW->Fill(MC_phi);
      m_h1_MC_dau0_phi_FW->Fill(mc_dau0->getMomentum().Phi());
      m_h1_MC_dau1_phi_FW->Fill(mc_dau1->getMomentum().Phi());
      m_h1_MC_Mother_phi_FW->Fill(MC_phi);
    } else { //barrel
      m_h1_MC_dau0_phiMother_barrel->Fill(MC_phi);
      m_h1_MC_dau1_phiMother_barrel->Fill(MC_phi);
      m_h1_MC_dau0_phi_barrel->Fill(mc_dau0->getMomentum().Phi());
      m_h1_MC_dau1_phi_barrel->Fill(mc_dau1->getMomentum().Phi());
      m_h1_MC_Mother_phi_barrel->Fill(MC_phi);
    }

    //------------------------------------------------------------------//
    //                          MC RECO TRACKS                          //
    //------------------------------------------------------------------//

    RelationVector<RecoTrack> MCRecoTracks_MCdau0 =
      DataStore::getRelationsWithObj<RecoTrack>(mc_dau0, m_MCRecoTracksName); //oppure &mc_dau0;

    RelationVector<RecoTrack> MCRecoTracks_MCdau1 =
      DataStore::getRelationsWithObj<RecoTrack>(mc_dau1, m_MCRecoTracksName);

    if (MCRecoTracks_MCdau0.size() > 0) {
      m_h1_RecoTrack_dau0_d0->Fill(mcParticleInfo_dau0.getD0());
      m_h1_RecoTrack_dau0_z0->Fill(mcParticleInfo_dau0.getZ0());
      m_h1_RecoTrack_dau0_RMother->Fill(MC_transDist);
      m_h3_RecoTrack_dau0->Fill(mc_dau0->getMomentum().Pt(), mc_dau0->getMomentum().Theta(), mc_dau0->getMomentum().Phi());
      m_h1_RecoTrack_dau0_pt->Fill(mc_dau0->getMomentum().Pt());
      m_h1_RecoTrack_dau0_pz->Fill(mc_dau0->getMomentum().Pz());
      m_h1_RecoTrack_dau0_p->Fill(mc_dau0->getMomentum().Mag());
      m_h1_RecoTrack_dau0_phi->Fill(mc_dau0->getMomentum().Phi());
      m_h1_RecoTrack_dau0_theta->Fill(mc_dau0->getMomentum().Theta());
      m_h1_RecoTrack_dau0_costheta->Fill(mc_dau0->getMomentum().CosTheta());
      m_h1_RecoTrack_dau0_Mother_cosAngle->Fill(mcParticle.getMomentum() * mc_dau0->getMomentum() / mcParticle.getMomentum().Mag() /
                                                mc_dau0->getMomentum().Mag());

      m_h1_RecoTrack_dau0_thetaMother->Fill(MC_theta);
      m_h1_RecoTrack_dau0_ptMother->Fill(MC_pt);

      m_h1_RecoTrack_dau0_phiMother_total->Fill(MC_phi);
      m_h2_RecoTrack_dau0_2D->Fill(mc_dau0->getMomentum().Theta(), mc_dau0->getMomentum().Pt());
      m_h2_RecoTrack_dau0_2DMother->Fill(MC_theta, MC_pt);
      m_h2_RecoTrack_dau0_pVScostheta->Fill(mc_dau0->getMomentum().CosTheta(), mc_dau0->getMomentum().Mag());

      if (MC_transDist < 1.) {
        m_h2_RecoTrack_dau0_2D_BP->Fill(mc_dau0->getMomentum().Theta(), mc_dau0->getMomentum().Pt());
      }

      if (mc_dau0->getMomentum().Theta() > (120 * TMath::Pi() / 180.)) m_h1_RecoTrack_dau0_phi_BW->Fill(mc_dau0->getMomentum().Phi());
      else if (mc_dau0->getMomentum().Theta() < (30. * TMath::Pi() / 180.)) m_h1_RecoTrack_dau0_phi_FW->Fill(
          mc_dau0->getMomentum().Phi());
      else m_h1_RecoTrack_dau0_phi_barrel->Fill(mc_dau0->getMomentum().Phi());

      if (MC_theta > (120 * TMath::Pi() / 180.)) m_h1_RecoTrack_dau0_phiMother_BW->Fill(MC_phi);
      else if (MC_theta < (30. * TMath::Pi() / 180.)) m_h1_RecoTrack_dau0_phiMother_FW->Fill(MC_phi);
      else m_h1_RecoTrack_dau0_phiMother_barrel->Fill(MC_phi);

      //V0 candidates
      if (MCRecoTracks_MCdau1.size() > 0) {
        m_h1_RecoTrack_Mother_RMother->Fill(MC_transDist);
        m_h3_RecoTrack_Mother->Fill(MC_pt, MC_theta, MC_phi);
        m_h1_RecoTrack_Mother_pt->Fill(MC_pt);
        m_h1_RecoTrack_Mother_pz->Fill(mcParticle.getMomentum().Pz());
        m_h1_RecoTrack_Mother_p->Fill(mcParticle.getMomentum().Mag());
        m_h1_RecoTrack_Mother_phi->Fill(MC_phi);
        m_h1_RecoTrack_Mother_theta->Fill(MC_theta);
        m_h1_RecoTrack_Mother_costheta->Fill(mcParticle.getMomentum().CosTheta());

        m_h1_RecoTrack_Mother_pt->Fill(MC_pt);
        m_h2_RecoTrack_Mother_2D->Fill(MC_theta, MC_pt);

        m_h2_RecoTrack_Mother_pVScostheta->Fill(MC_costheta, MC_p);

        if (MC_transDist < 1.) m_h2_RecoTrack_Mother_2D_BP->Fill(MC_theta, MC_pt);

        if (MC_theta > (120 * TMath::Pi() / 180.)) //BW
          m_h1_RecoTrack_Mother_phi_BW->Fill(MC_phi);
        else if (MC_theta < (30. * TMath::Pi() / 180.))//FW
          m_h1_RecoTrack_Mother_phi_FW->Fill(MC_phi);
        else
          m_h1_RecoTrack_Mother_phi_barrel->Fill(MC_phi);
      }
    }

    if (MCRecoTracks_MCdau1.size() > 0) {
      m_h1_RecoTrack_dau1_d0->Fill(mcParticleInfo_dau1.getD0());
      m_h1_RecoTrack_dau1_z0->Fill(mcParticleInfo_dau1.getZ0());
      m_h1_RecoTrack_dau1_RMother->Fill(MC_transDist);
      m_h3_RecoTrack_dau1->Fill(mc_dau1->getMomentum().Pt(), mc_dau1->getMomentum().Theta(), mc_dau1->getMomentum().Phi());
      m_h1_RecoTrack_dau1_pt->Fill(mc_dau1->getMomentum().Pt());
      m_h1_RecoTrack_dau1_pz->Fill(mc_dau1->getMomentum().Pz());
      m_h1_RecoTrack_dau1_p->Fill(mc_dau1->getMomentum().Mag());
      m_h1_RecoTrack_dau1_phi->Fill(mc_dau1->getMomentum().Phi());
      m_h1_RecoTrack_dau1_theta->Fill(mc_dau1->getMomentum().Theta());
      m_h1_RecoTrack_dau1_costheta->Fill(mc_dau1->getMomentum().CosTheta());
      m_h1_RecoTrack_dau1_Mother_cosAngle->Fill(mcParticle.getMomentum() * mc_dau1->getMomentum() / mcParticle.getMomentum().Mag() /
                                                mc_dau1->getMomentum().Mag());

      m_h1_RecoTrack_dau1_thetaMother->Fill(MC_theta);
      m_h1_RecoTrack_dau1_ptMother->Fill(MC_pt);

      m_h1_RecoTrack_dau1_phiMother_total->Fill(MC_phi);
      m_h2_RecoTrack_dau1_2D->Fill(mc_dau1->getMomentum().Theta(), mc_dau1->getMomentum().Pt());
      m_h2_RecoTrack_dau1_2DMother->Fill(MC_theta, MC_pt);
      m_h2_RecoTrack_dau1_pVScostheta->Fill(mc_dau1->getMomentum().CosTheta(), mc_dau1->getMomentum().Mag());

      if (MC_transDist < 1.) {
        m_h2_RecoTrack_dau1_2D_BP->Fill(mc_dau1->getMomentum().Theta(), mc_dau1->getMomentum().Pt());

        if (mc_dau1->getMomentum().Theta() > (120 * TMath::Pi() / 180.)) m_h1_RecoTrack_dau1_phi_BW->Fill(mc_dau1->getMomentum().Phi());
        else if (mc_dau1->getMomentum().Theta() < (30. * TMath::Pi() / 180.)) m_h1_RecoTrack_dau1_phi_FW->Fill(
            mc_dau1->getMomentum().Phi());
        else m_h1_RecoTrack_dau1_phi_barrel->Fill(mc_dau1->getMomentum().Phi());

        if (MC_theta > (120 * TMath::Pi() / 180.)) m_h1_RecoTrack_dau1_phiMother_BW->Fill(MC_phi);
        else if (MC_theta < (30. * TMath::Pi() / 180.)) m_h1_RecoTrack_dau1_phiMother_FW->Fill(MC_phi);
        else m_h1_RecoTrack_dau1_phiMother_barrel->Fill(MC_phi);
      }
    }

    //------------------------------------------------------------------//
    //                          TRACKS                                  //
    //------------------------------------------------------------------//

    Track* Track_dau0ToMCParticle = mc_dau0->getRelated<Track>();
    Track* Track_dau1ToMCParticle = mc_dau1->getRelated<Track>();

    if (Track_dau0ToMCParticle) {
      m_h1_track_dau0_d0->Fill(mcParticleInfo_dau0.getD0());
      m_h1_track_dau0_z0->Fill(mcParticleInfo_dau0.getZ0());
      m_h1_track_dau0_RMother->Fill(MC_transDist);
      m_h3_track_dau0->Fill(mc_dau0->getMomentum().Pt(), mc_dau0->getMomentum().Theta(), mc_dau0->getMomentum().Phi());
      m_h1_track_dau0_pt->Fill(mc_dau0->getMomentum().Pt());
      m_h1_track_dau0_pz->Fill(mc_dau0->getMomentum().Pz());
      m_h1_track_dau0_p->Fill(mc_dau0->getMomentum().Mag());
      m_h1_track_dau0_phi->Fill(mc_dau0->getMomentum().Phi());
      m_h1_track_dau0_theta->Fill(mc_dau0->getMomentum().Theta());
      m_h1_track_dau0_costheta->Fill(mc_dau0->getMomentum().CosTheta());
      m_h1_track_dau0_Mother_cosAngle->Fill(mcParticle.getMomentum() * mc_dau0->getMomentum() / mcParticle.getMomentum().Mag() /
                                            mc_dau0->getMomentum().Mag());

      m_h1_track_dau0_thetaMother->Fill(MC_theta);
      m_h1_track_dau0_ptMother->Fill(MC_pt);

      m_h1_track_dau0_phiMother_total->Fill(MC_phi);

      m_h2_track_dau0_2D->Fill(mc_dau0->getMomentum().Theta(), mc_dau0->getMomentum().Pt());
      m_h2_track_dau0_2DMother->Fill(MC_theta, MC_pt);
      m_h2_track_dau0_pVScostheta->Fill(mc_dau0->getMomentum().CosTheta(), mc_dau0->getMomentum().Mag());

      if (MC_transDist < 1.)
        m_h2_track_dau0_2D_BP->Fill(mc_dau0->getMomentum().Theta(), mc_dau0->getMomentum().Pt());

      if (MC_theta > (120 * TMath::Pi() / 180.)) { //BW
        m_h1_track_dau0_phiMother_BW->Fill(MC_phi);
        m_h1_track_dau0_phi_BW->Fill(mc_dau0->getMomentum().Phi());
      } else if (MC_theta < (30 * TMath::Pi() / 180.)) { //FW
        m_h1_track_dau0_phiMother_FW->Fill(MC_phi);
        m_h1_track_dau0_phi_FW->Fill(mc_dau0->getMomentum().Phi());
      } else { //barrel
        m_h1_track_dau0_phiMother_barrel->Fill(MC_phi);
        m_h1_track_dau0_phi_barrel->Fill(mc_dau0->getMomentum().Phi());
      }
    }

    if (Track_dau1ToMCParticle) {
      m_h1_track_dau1_d0->Fill(mcParticleInfo_dau1.getD0());
      m_h1_track_dau1_z0->Fill(mcParticleInfo_dau1.getZ0());
      m_h1_track_dau1_RMother->Fill(MC_transDist);
      m_h3_track_dau1->Fill(mc_dau1->getMomentum().Pt(), mc_dau1->getMomentum().Theta(), mc_dau1->getMomentum().Phi());
      m_h1_track_dau1_pt->Fill(mc_dau1->getMomentum().Pt());
      m_h1_track_dau1_pz->Fill(mc_dau1->getMomentum().Pz());
      m_h1_track_dau1_p->Fill(mc_dau1->getMomentum().Mag());
      m_h1_track_dau1_phi->Fill(mc_dau1->getMomentum().Phi());
      m_h1_track_dau1_theta->Fill(mc_dau1->getMomentum().Theta());
      m_h1_track_dau1_costheta->Fill(mc_dau1->getMomentum().CosTheta());
      m_h1_track_dau1_Mother_cosAngle->Fill(mcParticle.getMomentum() * mc_dau1->getMomentum() / mcParticle.getMomentum().Mag() /
                                            mc_dau1->getMomentum().Mag());

      m_h1_track_dau1_thetaMother->Fill(MC_theta);
      m_h1_track_dau1_ptMother->Fill(MC_pt);

      m_h1_track_dau1_phiMother_total->Fill(MC_phi);

      m_h2_track_dau1_2D->Fill(mc_dau1->getMomentum().Theta(), mc_dau1->getMomentum().Pt());
      m_h2_track_dau1_2DMother->Fill(MC_theta, MC_pt);
      m_h2_track_dau1_pVScostheta->Fill(mc_dau1->getMomentum().CosTheta(), mc_dau1->getMomentum().Mag());

      if (MC_transDist < 1.)
        m_h2_track_dau1_2D_BP->Fill(mc_dau1->getMomentum().Theta(), mc_dau1->getMomentum().Pt());

      if (MC_theta > (120 * TMath::Pi() / 180.)) { //BW
        m_h1_track_dau1_phiMother_BW->Fill(MC_phi);
        m_h1_track_dau1_phi_BW->Fill(mc_dau1->getMomentum().Phi());
      } else if (MC_theta < (30 * TMath::Pi() / 180.)) { //FW
        m_h1_track_dau1_phiMother_FW->Fill(MC_phi);
        m_h1_track_dau1_phi_FW->Fill(mc_dau1->getMomentum().Phi());
      } else { //barrel
        m_h1_track_dau1_phiMother_barrel->Fill(MC_phi);
        m_h1_track_dau1_phi_barrel->Fill(mc_dau1->getMomentum().Phi());
      }
    }

    int nMatchedDau =  nMatchedDaughters(mcParticle);

    //V0: proceed only in case the MCParticle daughters have one associated reconstructed track:
    if (nMatchedDau != 2)
      continue;

    int pdgCode = mcParticle.getPDG();
    B2DEBUG(99, "MCParticle has PDG code " << pdgCode);

    RelationVector<V0ValidationVertex> V0s_toMCParticle =
      DataStore::getRelationsWithObj<V0ValidationVertex>(&mcParticle, m_V0sName);

    if (V0s_toMCParticle.size() > 0) {

      m_h1_V0_RMother->Fill(MC_transDist);
      m_h3_V0->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
      m_h1_V0_pt->Fill(mcParticleInfo.getPt());
      m_h1_V0_pz->Fill(mcParticleInfo.getPz());
      m_h1_V0_p->Fill(mcParticleInfo.getP());
      m_h1_V0_phi->Fill(mcParticleInfo.getPphi());
      m_h1_V0_theta->Fill(mcParticleInfo.getPtheta());
      m_h1_V0_costheta->Fill(mcParticle.getMomentum().CosTheta());
      m_h2_V0_Mother_2D->Fill(MC_theta, MC_p);
      m_h2_V0_Mother_pVScostheta->Fill(MC_costheta, MC_p);

      if (MC_transDist < 1.) m_h2_V0_Mother_2D_BP->Fill(MC_theta, MC_pt);

      if (MC_theta > (120 * TMath::Pi() / 180.)) //BW
        m_h1_V0_phi_BW->Fill(mcParticleInfo.getPphi());

      else if (MC_theta < (30 * TMath::Pi() / 180.))//FW
        m_h1_V0_phi_FW->Fill(mcParticleInfo.getPphi());

      else //barrel
        m_h1_V0_phi_barrel->Fill(mcParticleInfo.getPphi());
    }
  }
}

void EffPlotsModule::endRun()
{
  double track_dau0 = m_h1_track_dau0_RMother->GetEntries();
  double RecoTrack_dau0 = m_h1_RecoTrack_dau0_RMother->GetEntries();
  double MC_dau0 = m_h1_MC_dau0_RMother->GetEntries();
  double eff_dau0_noGA = track_dau0 / MC_dau0;
  double effErr_dau0_noGA = sqrt(eff_dau0_noGA * (1 - eff_dau0_noGA)) / sqrt(MC_dau0);
  double eff_dau0_withGA = track_dau0 / RecoTrack_dau0;
  double effErr_dau0_withGA = sqrt(eff_dau0_withGA * (1 - eff_dau0_withGA)) / sqrt(RecoTrack_dau0);

  double track_dau1 = m_h1_track_dau1_RMother->GetEntries();
  double RecoTrack_dau1 = m_h1_RecoTrack_dau1_RMother->GetEntries();
  double MC_dau1 = m_h1_MC_dau1_RMother->GetEntries();
  double eff_dau1_noGA = track_dau1 / MC_dau1;
  double effErr_dau1_noGA = sqrt(eff_dau1_noGA * (1 - eff_dau1_noGA)) / sqrt(MC_dau1);
  double eff_dau1_withGA = track_dau1 / RecoTrack_dau1;
  double effErr_dau1_withGA = sqrt(eff_dau1_withGA * (1 - eff_dau1_withGA)) / sqrt(RecoTrack_dau1);

  double track_Mother = m_h1_V0_RMother->GetEntries();
  double RecoTrack_Mother = m_h1_RecoTrack_Mother_RMother->GetEntries();
  double MC_Mother = m_h1_MC_Mother_RMother->GetEntries();
  double eff_Mother_noGA = track_Mother / MC_Mother;
  double effErr_Mother_noGA = sqrt(eff_Mother_noGA * (1 - eff_Mother_noGA)) / sqrt(MC_Mother);
  double eff_Mother_withGA = track_Mother / RecoTrack_Mother;
  double effErr_Mother_withGA = sqrt(eff_Mother_withGA * (1 - eff_Mother_withGA)) / sqrt(RecoTrack_Mother);




  B2INFO("");
  B2INFO("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  B2INFO("~ V0 Finding Performance Evaluation ~ SHORT SUMMARY ~");
  B2INFO("");
  B2INFO(" + overall, normalized to MC particles (_noGeoAcc):");
  B2INFO("");
  B2INFO("   efficiency dau0 = (" << eff_dau0_noGA * 100 << " +/- " << effErr_dau0_noGA * 100 << ")% ");
  B2INFO("   efficiency dau1 = (" << eff_dau1_noGA * 100 << " +/- " << effErr_dau1_noGA * 100 << ")% ");
  B2INFO("   efficiency Mother = (" << eff_Mother_noGA * 100 << " +/- " << effErr_Mother_noGA * 100 << ")% ");
  B2INFO("");
  B2INFO(" + overall, normalized to RecoTracks(_withGeoAcc):");
  B2INFO("");
  B2INFO("   efficiency dau0 = (" << eff_dau0_withGA * 100 << " +/- " << effErr_dau0_withGA * 100 << ")% ");
  B2INFO("   efficiency dau1 = (" << eff_dau1_withGA * 100 << " +/- " << effErr_dau1_withGA * 100 << ")% ");
  B2INFO("   efficiency Mother = (" << eff_Mother_withGA * 100 << " +/- " << effErr_Mother_withGA * 100 << ")% ");
  B2INFO("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

}

void EffPlotsModule::terminate()
{
  //------------------------------------------------------------------//
  //                         EFFICIENCIES                             //
  //------------------------------------------------------------------//

  //dau0
  TH1F* h_eff_dau0_d0 = effPlot1D(m_h1_MC_dau0_d0, m_h1_RecoTrack_dau0_d0, m_h1_track_dau0_d0, "h_eff_dau0_d0",
                                  "efficiency VS d0, dau_{0}", m_histoList_Efficiencies);
  if (h_eff_dau0_d0->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau0_d0");

  TH1F* h_eff_dau0_z0 = effPlot1D(m_h1_MC_dau0_z0, m_h1_RecoTrack_dau0_z0, m_h1_track_dau0_z0, "h_eff_dau0_z0",
                                  "efficiency VS z0, dau_{0}", m_histoList_Efficiencies);
  if (h_eff_dau0_z0->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau0_z0");

  TH1F* h_eff_dau0_RMother = effPlot1D(m_h1_MC_dau0_RMother, m_h1_RecoTrack_dau0_RMother, m_h1_track_dau0_RMother,
                                       "h_eff_dau0_RMother", "efficiency VS R_{mother}, dau_{0}", m_histoList_Efficiencies);
  if (h_eff_dau0_RMother->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau0_RMother");

  TH1F* h_eff_dau0_thetaMother = effPlot1D(m_h1_MC_dau0_thetaMother, m_h1_RecoTrack_dau0_thetaMother, m_h1_track_dau0_thetaMother,
                                           "h_eff_dau0_thetaMother", "efficiency VS #theta_{mother}, dau_{0}", m_histoList_Efficiencies);
  if (h_eff_dau0_thetaMother->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau0_thetaMother");

  TH1F* h_eff_dau0_phiMother_total = effPlot1D(m_h1_MC_dau0_phiMother_total, m_h1_RecoTrack_dau0_phiMother_total,
                                               m_h1_track_dau0_phiMother_total, "h_eff_dau0_phiMother_total", "efficiency VS #phi_{mother}, dau_{0}", m_histoList_Efficiencies);
  if (h_eff_dau0_phiMother_total->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau0_phiMother_total");

  TH1F* h_eff_dau0_phiMother_BW = effPlot1D(m_h1_MC_dau0_phiMother_BW, m_h1_RecoTrack_dau0_phiMother_BW, m_h1_track_dau0_phiMother_BW,
                                            "h_eff_dau0_phiMother_BW", "efficiency VS #phi_{mother}, dau_{0} BW", m_histoList_Efficiencies);
  if (h_eff_dau0_phiMother_BW->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau0_phiMother_BW");

  TH1F* h_eff_dau0_phiMother_barrel = effPlot1D(m_h1_MC_dau0_phiMother_barrel, m_h1_RecoTrack_dau0_phiMother_barrel,
                                                m_h1_track_dau0_phiMother_barrel, "h_eff_dau0_phiMother_barrel", "efficiency VS #phi_{mother}, dau_{0} barrel",
                                                m_histoList_Efficiencies);
  if (h_eff_dau0_phiMother_barrel->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau0_phiMother_barrel");

  TH1F* h_eff_dau0_phiMother_FW = effPlot1D(m_h1_MC_dau0_phiMother_FW, m_h1_RecoTrack_dau0_phiMother_FW, m_h1_track_dau0_phiMother_FW,
                                            "h_eff_dau0_phiMother_FW", "efficiency VS #phi_{mother}, dau_{0} FW", m_histoList_Efficiencies);
  if (h_eff_dau0_phiMother_FW->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau0_phiMother_FW");

  TH1F* h_eff_dau0_phi_total = effPlot1D(m_h1_MC_dau0_phi, m_h1_RecoTrack_dau0_phi, m_h1_track_dau0_phi, "h_eff_dau0_phi_total",
                                         "efficiency VS #phi_{dau_{0}}, dau_{0}", m_histoList_Efficiencies);
  if (h_eff_dau0_phi_total->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau0_phi_total");

  TH1F* h_eff_dau0_phi_BW = effPlot1D(m_h1_MC_dau0_phi_BW, m_h1_RecoTrack_dau0_phi_BW, m_h1_track_dau0_phi_BW, "h_eff_dau0_phi_BW",
                                      "efficiency VS #phi_{dau_{0}}, dau_{0} BW", m_histoList_Efficiencies);
  if (h_eff_dau0_phi_BW->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau0_phi_BW");

  TH1F* h_eff_dau0_phi_barrel = effPlot1D(m_h1_MC_dau0_phi_barrel, m_h1_RecoTrack_dau0_phi_barrel, m_h1_track_dau0_phi_barrel,
                                          "h_eff_dau0_phi_barrel", "efficiency VS #phi_{dau_{0}}, dau_{0} barrel", m_histoList_Efficiencies);
  if (h_eff_dau0_phi_barrel->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau0_phi_barrel");

  TH1F* h_eff_dau0_phi_FW = effPlot1D(m_h1_MC_dau0_phi_FW, m_h1_RecoTrack_dau0_phi_FW, m_h1_track_dau0_phi_FW, "h_eff_dau0_phi_FW",
                                      "efficiency VS #phi_{dau_{0}}, dau_{0} FW", m_histoList_Efficiencies);
  if (h_eff_dau0_phi_FW->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau0_phi_FW");

  TH2F* h2_effMap_dau0_pVScostheta = effPlot2D(m_h2_MC_dau0_pVScostheta, m_h2_RecoTrack_dau0_pVScostheta, m_h2_track_dau0_pVScostheta,
                                               "h2_effMap_dau0_pVScostheta", "efficiency map, p_{CM} VS cos(#theta)_{CM} dau_{0}", m_histoList_Efficiencies);
  if (h2_effMap_dau0_pVScostheta->GetEntries() == 0) B2WARNING("Empty histogram h2_effMap_dau0_pVScostheta");

  //dau1
  TH1F* h_eff_dau1_d0 = effPlot1D(m_h1_MC_dau1_d0, m_h1_RecoTrack_dau1_d0, m_h1_track_dau1_d0, "h_eff_dau1_d0", "efficiency VS d0, p",
                                  m_histoList_Efficiencies);
  if (h_eff_dau1_d0->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau1_d0");

  TH1F* h_eff_dau1_z0 = effPlot1D(m_h1_MC_dau1_z0, m_h1_RecoTrack_dau1_z0, m_h1_track_dau1_z0, "h_eff_dau1_z0", "efficiency VS z0, p",
                                  m_histoList_Efficiencies);
  if (h_eff_dau1_z0->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau1_z0");

  TH1F* h_eff_dau1_RMother = effPlot1D(m_h1_MC_dau1_RMother, m_h1_RecoTrack_dau1_RMother, m_h1_track_dau1_RMother,
                                       "h_eff_dau1_RMother", "efficiency VS R_{mother}, p", m_histoList_Efficiencies);
  if (h_eff_dau1_RMother->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau1_RMother");

  TH1F* h_eff_dau1_thetaMother = effPlot1D(m_h1_MC_dau1_thetaMother, m_h1_RecoTrack_dau1_thetaMother, m_h1_track_dau1_thetaMother,
                                           "h_eff_dau1_thetaMother", "efficiency VS #theta_{mother}, p", m_histoList_Efficiencies);
  if (h_eff_dau1_thetaMother->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau1_thetaMother");

  TH1F* h_eff_dau1_phiMother_total = effPlot1D(m_h1_MC_dau1_phiMother_total, m_h1_RecoTrack_dau1_phiMother_total,
                                               m_h1_track_dau1_phiMother_total, "h_eff_dau1_phiMother_total", "efficiency VS #phi_{mother}, p", m_histoList_Efficiencies);
  if (h_eff_dau1_phiMother_total->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau1_phiMother_total");

  TH1F* h_eff_dau1_phiMother_BW = effPlot1D(m_h1_MC_dau1_phiMother_BW, m_h1_RecoTrack_dau1_phiMother_BW, m_h1_track_dau1_phiMother_BW,
                                            "h_eff_dau1_phiMother_BW", "efficiency VS #phi_{mother}, p BW", m_histoList_Efficiencies);
  if (h_eff_dau1_phiMother_BW->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau1_phiMother_BW");

  TH1F* h_eff_dau1_phiMother_barrel = effPlot1D(m_h1_MC_dau1_phiMother_barrel, m_h1_RecoTrack_dau1_phiMother_barrel,
                                                m_h1_track_dau1_phiMother_barrel, "h_eff_dau1_phiMother_barrel", "efficiency VS #phi_{mother}, p barrel", m_histoList_Efficiencies);
  if (h_eff_dau1_phiMother_barrel->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau1_phiMother_barrel");

  TH1F* h_eff_dau1_phiMother_FW = effPlot1D(m_h1_MC_dau1_phiMother_FW, m_h1_RecoTrack_dau1_phiMother_FW, m_h1_track_dau1_phiMother_FW,
                                            "h_eff_dau1_phiMother_FW", "efficiency VS #phi_{mother}, p FW", m_histoList_Efficiencies);
  if (h_eff_dau1_phiMother_FW->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau1_phiMother_FW");

  TH1F* h_eff_dau1_phi_total = effPlot1D(m_h1_MC_dau1_phi, m_h1_RecoTrack_dau1_phi, m_h1_track_dau1_phi, "h_eff_dau1_phi_total",
                                         "efficiency VS #phi_{p}, p", m_histoList_Efficiencies);
  if (h_eff_dau1_phi_total->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau1_phi_total");

  TH1F* h_eff_dau1_phi_BW = effPlot1D(m_h1_MC_dau1_phi_BW, m_h1_RecoTrack_dau1_phi_BW, m_h1_track_dau1_phi_BW, "h_eff_dau1_phi_BW",
                                      "efficiency VS #phi_{p}, p BW", m_histoList_Efficiencies);
  if (h_eff_dau1_phi_BW->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau1_phi_BW");

  TH1F* h_eff_dau1_phi_barrel = effPlot1D(m_h1_MC_dau1_phi_barrel, m_h1_RecoTrack_dau1_phi_barrel, m_h1_track_dau1_phi_barrel,
                                          "h_eff_dau1_phi_barrel", "efficiency VS #phi_{p}, p barrel", m_histoList_Efficiencies);
  if (h_eff_dau1_phi_barrel->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau1_phi_barrel");

  TH1F* h_eff_dau1_phi_FW = effPlot1D(m_h1_MC_dau1_phi_FW, m_h1_RecoTrack_dau1_phi_FW, m_h1_track_dau1_phi_FW, "h_eff_dau1_phi_FW",
                                      "efficiency VS #phi_{p}, p FW", m_histoList_Efficiencies);
  if (h_eff_dau1_phi_FW->GetEntries() == 0) B2WARNING("Empty histogram h_eff_dau1_phi_FW");

  TH2F* h2_effMap_dau1_pVScostheta = effPlot2D(m_h2_MC_dau1_pVScostheta, m_h2_RecoTrack_dau1_pVScostheta, m_h2_track_dau1_pVScostheta,
                                               "h2_effMap_dau1_pVScostheta", "efficiency map, p_{CM} VS cos(#theta)_{CM} dau_{1}", m_histoList_Efficiencies);
  if (h2_effMap_dau1_pVScostheta->GetEntries() == 0) B2WARNING("Empty histogram h2_effMap_dau1_pVScostheta");

  //mother
  TH1F* h_eff_Mother_RMother = effPlot1D(m_h1_MC_Mother_RMother, m_h1_RecoTrack_Mother_RMother, m_h1_V0_RMother,
                                         "h_eff_Mother_RMother", "efficiency VS R_{mother}, mother", m_histoList_Efficiencies);
  if (h_eff_Mother_RMother->GetEntries() == 0) B2WARNING("Empty histogram h_eff_Mother_RMother");

  TH1F* h_eff_Mother_theta = effPlot1D(m_h1_MC_Mother_theta, m_h1_RecoTrack_Mother_theta, m_h1_V0_theta, "h_eff_Mother_theta",
                                       "efficiency VS #theta_{mother}, mother", m_histoList_Efficiencies);
  if (h_eff_Mother_theta->GetEntries() == 0) B2WARNING("Empty histogram h_eff_Mother_theta");

  TH1F* h_eff_Mother_phi_total = effPlot1D(m_h1_MC_Mother_phi, m_h1_RecoTrack_Mother_phi, m_h1_V0_phi, "h_eff_Mother_phi_total",
                                           "efficiency VS #phi_{mother}, mother", m_histoList_Efficiencies);
  if (h_eff_Mother_phi_total->GetEntries() == 0) B2WARNING("Empty histogram h_eff_Mother_phi_total");

  TH1F* h_eff_Mother_phiMother_BW = effPlot1D(m_h1_MC_Mother_phi_BW, m_h1_RecoTrack_Mother_phi_BW, m_h1_V0_phi_BW,
                                              "h_eff_Mother_phi_BW", "efficiency VS #phi_{mother}, mother BW", m_histoList_Efficiencies);
  if (h_eff_Mother_phiMother_BW->GetEntries() == 0) B2WARNING("Empty histogram h_eff_Mother_phiMother_BW");

  TH1F* h_eff_Mother_phiMother_barrel = effPlot1D(m_h1_MC_Mother_phi_barrel, m_h1_RecoTrack_Mother_phi_barrel, m_h1_V0_phi_barrel,
                                                  "h_eff_Mother_phi_barrel", "efficiency VS #phi_{mother}, mother barrel", m_histoList_Efficiencies);
  if (h_eff_Mother_phiMother_barrel->GetEntries() == 0) B2WARNING("Empty histogram h_eff_Mother_phiMother_barrel");

  TH1F* h_eff_Mother_phiMother_FW = effPlot1D(m_h1_MC_Mother_phi_FW, m_h1_RecoTrack_Mother_phi_FW, m_h1_V0_phi_FW,
                                              "h_eff_Mother_phi_FW", "efficiency VS #phi_{mother}, mother FW", m_histoList_Efficiencies);
  if (h_eff_Mother_phiMother_FW->GetEntries() == 0) B2WARNING("Empty histogram h_eff_Mother_phiMother_FW");

  TH2F* h2_effMap_Mother_pVScostheta = effPlot2D(m_h2_MC_Mother_pVScostheta, m_h2_RecoTrack_Mother_pVScostheta,
                                                 m_h2_V0_Mother_pVScostheta, "h2_effMap_Mother_pVScostheta", "efficiency map, p_{CM} VS cos(#theta)_{CM} mother",
                                                 m_histoList_Efficiencies);
  if (h2_effMap_Mother_pVScostheta->GetEntries() == 0) B2WARNING("Empty histogram h2_effMap_Mother_pVScostheta");

  //eff_V0Finder [ = eff_mother / (eff_dau0 * eff_dau1)]
  TH1F* h_eff_V0Finder_RMother_withGeoAcc = V0FinderEff(h_eff_dau0_RMother, h_eff_dau1_RMother, h_eff_Mother_RMother,
                                                        "h_eff_V0Finder_RMother_withGeoAcc", "efficiency VS R_{mother}, V0Finder", m_histoList_Efficiencies);
  if (h_eff_V0Finder_RMother_withGeoAcc->GetEntries() == 0) B2WARNING("Empty histogram h_eff_V0Finder_RMother_withGeoAcc");

  TH1F* h_eff_V0Finder_theta_withGeoAcc = V0FinderEff(h_eff_dau0_thetaMother, h_eff_dau1_thetaMother, h_eff_Mother_theta,
                                                      "h_eff_V0Finder_theta_withGeoAcc", "efficiency VS #theta_{mother}, V0Finder", m_histoList_Efficiencies);
  if (h_eff_V0Finder_theta_withGeoAcc->GetEntries() == 0) B2WARNING("Empty histogram h_eff_V0Finder_theta_withGeoAcc");

  TH1F* h_eff_V0Finder_phi_total_withGeoAcc = V0FinderEff(h_eff_dau0_phiMother_total, h_eff_dau1_phiMother_total,
                                                          h_eff_Mother_phi_total, "h_eff_V0Finder_phi_total_withGeoAcc", "efficiency VS #phi_{mother}, V0Finder", m_histoList_Efficiencies);
  if (h_eff_V0Finder_phi_total_withGeoAcc->GetEntries() == 0) B2WARNING("Empty histogram h_eff_V0Finder_phi_total_withGeoAcc");

  TH1F* h_eff_V0Finder_phiMother_BW_withGeoAcc = V0FinderEff(h_eff_dau0_phiMother_BW, h_eff_dau1_phiMother_BW,
                                                             h_eff_Mother_phiMother_BW, "h_eff_V0Finder_phi_BW_withGeoAcc", "efficiency VS #phi_{mother}, V0Finder BW",
                                                             m_histoList_Efficiencies);
  if (h_eff_V0Finder_phiMother_BW_withGeoAcc->GetEntries() == 0) B2WARNING("Empty histogram h_eff_V0Finder_phiMother_BW_withGeoAcc");

  TH1F* h_eff_V0Finder_phiMother_barrel_withGeoAcc = V0FinderEff(h_eff_dau0_phiMother_barrel, h_eff_dau1_phiMother_barrel,
                                                     h_eff_Mother_phiMother_barrel, "h_eff_V0Finder_phi_barrel_withGeoAcc", "efficiency VS #phi_{mother}, V0Finder barrel",
                                                     m_histoList_Efficiencies);
  if (h_eff_V0Finder_phiMother_barrel_withGeoAcc->GetEntries() == 0)
    B2WARNING("Empty histogram h_eff_V0Finder_phiMother_barrel_withGeoAcc");

  TH1F* h_eff_V0Finder_phiMother_FW_withGeoAcc = V0FinderEff(h_eff_dau0_phiMother_FW, h_eff_dau1_phiMother_FW,
                                                             h_eff_Mother_phiMother_FW, "h_eff_V0Finder_phi_FW_withGeoAcc", "efficiency VS #phi_{mother}, V0Finder FW",
                                                             m_histoList_Efficiencies);
  if (h_eff_V0Finder_phiMother_FW_withGeoAcc->GetEntries() == 0) B2WARNING("Empty histogram h_eff_V0Finder_phiMother_FW_withGeoAcc");


  //-------------------------------------------------------------------------//
  //                           GEOMETRICAL ACCETTANCE                        //
  //-------------------------------------------------------------------------//

  if (m_geometricalAccettance) {
    //dau0
    TH1F* h_dau0_geoAcc_theta = geoAcc1D(m_h1_MC_dau0_theta, m_h1_RecoTrack_dau0_theta, "h_dau0_geoAcc_theta",
                                         "geometrical acceptance VS #theta_{dau_{0}}, dau_{0}", m_histoList_GA);
    if (h_dau0_geoAcc_theta->GetEntries() == 0) B2WARNING("Empty histogram h_dau0_geoAcc_theta");

    TH1F* h_dau0_geoAcc_phi = geoAcc1D(m_h1_MC_dau0_phi, m_h1_RecoTrack_dau0_phi, "h_dau0_geoAcc_phi",
                                       "geometrical acceptance VS #phi_{dau_{0}}, dau_{0}", m_histoList_GA);
    if (h_dau0_geoAcc_phi->GetEntries() == 0) B2WARNING("Empty histogram h_dau0_geoAcc_phi");

    TH1F* h_dau0_geoAcc_pt = geoAcc1D(m_h1_MC_dau0_pt, m_h1_RecoTrack_dau0_pt, "h_dau0_geoAcc_pt",
                                      "geometrical acceptance VS p_{T,dau_{0}}, dau_{0}", m_histoList_GA);
    if (h_dau0_geoAcc_pt->GetEntries() == 0) B2WARNING("Empty histogram h_dau0_geoAcc_pt");

    TH1F* h_dau0_geoAcc_thetaMother = geoAcc1D(m_h1_MC_dau0_thetaMother, m_h1_RecoTrack_dau0_thetaMother, "h_dau0_geoAcc_thetaMother",
                                               "geometrical acceptance VS #theta_{mother}, dau_{0}", m_histoList_GA);
    if (h_dau0_geoAcc_thetaMother->GetEntries() == 0) B2WARNING("Empty histogram h_dau0_geoAcc_thetaMother");

    TH1F* h_dau0_geoAcc_phiMother = geoAcc1D(m_h1_MC_dau0_phiMother_total, m_h1_RecoTrack_dau0_phiMother_total,
                                             "h_dau0_geoAcc_phiMother", "geometrical acceptance VS #phi_{mother}, dau_{0}", m_histoList_GA);
    if (h_dau0_geoAcc_phiMother->GetEntries() == 0) B2WARNING("Empty histogram h_dau0_geoAcc_phiMother");

    TH1F* h_dau0_geoAcc_ptMother = geoAcc1D(m_h1_MC_dau0_ptMother, m_h1_RecoTrack_dau0_ptMother, "h_dau0_geoAcc_ptMother",
                                            "geometrical acceptance VS #p_{T,mother}, dau_{0}", m_histoList_GA);
    if (h_dau0_geoAcc_ptMother->GetEntries() == 0) B2WARNING("Empty histogram h_dau0_geoAcc_ptMother");

    //dau1
    TH1F* h_dau1_geoAcc_theta = geoAcc1D(m_h1_MC_dau1_theta, m_h1_RecoTrack_dau1_theta, "h_dau1_geoAcc_theta",
                                         "geometrical acceptance VS #theta_{p}, p", m_histoList_GA);
    if (h_dau1_geoAcc_theta->GetEntries() == 0) B2WARNING("Empty histogram h_dau1_geoAcc_theta");

    TH1F* h_dau1_geoAcc_phi = geoAcc1D(m_h1_MC_dau1_phi, m_h1_RecoTrack_dau1_phi, "h_dau1_geoAcc_phi",
                                       "geometrical acceptance VS #phi_{p}, p", m_histoList_GA);
    if (h_dau1_geoAcc_phi->GetEntries() == 0) B2WARNING("Empty histogram h_dau1_geoAcc_phi");

    TH1F* h_dau1_geoAcc_pt = geoAcc1D(m_h1_MC_dau1_pt, m_h1_RecoTrack_dau1_pt, "h_dau1_geoAcc_pt",
                                      "geometrical acceptance VS p_{T,p}, p", m_histoList_GA);
    if (h_dau1_geoAcc_pt->GetEntries() == 0) B2WARNING("Empty histogram h_dau1_geoAcc_pt");

    TH1F* h_dau1_geoAcc_thetaMother = geoAcc1D(m_h1_MC_dau1_thetaMother, m_h1_RecoTrack_dau1_thetaMother, "h_dau1_geoAcc_thetaMother",
                                               "geometrical acceptance VS #theta_{mother}, p", m_histoList_GA);
    if (h_dau1_geoAcc_thetaMother->GetEntries() == 0) B2WARNING("Empty histogram h_dau1_geoAcc_thetaMother");

    TH1F* h_dau1_geoAcc_phiMother = geoAcc1D(m_h1_MC_dau1_phiMother_total, m_h1_RecoTrack_dau1_phiMother_total,
                                             "h_dau1_geoAcc_phiMother", "geometrical acceptance VS #phi_{mother}, p", m_histoList_GA);
    if (h_dau1_geoAcc_phiMother->GetEntries() == 0) B2WARNING("Empty histogram h_dau1_geoAcc_phiMother");

    TH1F* h_dau1_geoAcc_ptMother = geoAcc1D(m_h1_MC_dau1_ptMother, m_h1_RecoTrack_dau1_ptMother, "h_dau1_geoAcc_ptMother",
                                            "geometrical acceptance VS #p_{T,mother}, p", m_histoList_GA);
    if (h_dau1_geoAcc_ptMother->GetEntries() == 0) B2WARNING("Empty histogram h_dau1_geoAcc_ptMother");

    //mother
    TH1F* h_Mother_geoAcc_theta = geoAcc1D(m_h1_MC_Mother_theta, m_h1_RecoTrack_Mother_theta, "h_Mother_geoAcc_theta",
                                           "geometrical acceptance VS #theta_{mother}, mother", m_histoList_GA);
    if (h_Mother_geoAcc_theta->GetEntries() == 0) B2WARNING("Empty histogram h_Mother_geoAcc_theta");

    TH1F* h_Mother_geoAcc_phi = geoAcc1D(m_h1_MC_Mother_phi, m_h1_RecoTrack_Mother_phi, "h_Mother_geoAcc_phi",
                                         "geometrical acceptance VS #phi_{mother}, mother", m_histoList_GA);
    if (h_Mother_geoAcc_phi->GetEntries() == 0) B2WARNING("Empty histogram h_Mother_geoAcc_phi");

    TH1F* h_Mother_geoAcc_pt = geoAcc1D(m_h1_MC_Mother_pt, m_h1_RecoTrack_Mother_pt, "h_Mother_geoAcc_pt",
                                        "geometrical acceptance VS p_{T,mother}, mother", m_histoList_GA);
    if (h_Mother_geoAcc_pt->GetEntries() == 0) B2WARNING("Empty histogram h_Mother_geoAcc_pt");

    //2D
    TH2F* h2_dau0_geoAcc2D = geoAcc2D(m_h2_MC_dau0_2D, m_h2_RecoTrack_dau0_2D, "h2_dau0_geoAcc2D",
                                      "geometrical acceptance dau_{0}, p_{T,dau_{0}} VS #theta_{dau_{0}}", m_histoList_GA);
    if (h2_dau0_geoAcc2D->GetEntries() == 0) B2WARNING("Empty histogram h2_dau0_geoAcc2D");

    TH2F* h2_dau1_geoAcc2D = geoAcc2D(m_h2_MC_dau1_2D, m_h2_RecoTrack_dau1_2D, "h2_dau1_geoAcc2D",
                                      "geometrical acceptance p, p_{T,p} VS #theta_{p}", m_histoList_GA);
    if (h2_dau1_geoAcc2D->GetEntries() == 0) B2WARNING("Empty histogram h2_dau1_geoAcc2D");

    TH2F* h2_Mother_geoAcc2D = geoAcc2D(m_h2_MC_Mother_2D, m_h2_RecoTrack_Mother_2D, "h2_Mother_geoAcc2D",
                                        "geometrical acceptance mother, p_{T,mother} VS #theta_{mother}", m_histoList_GA);
    if (h2_Mother_geoAcc2D->GetEntries() == 0) B2WARNING("Empty histogram h2_Mother_geoAcc2D");

    TH2F* h2_dau0Mother_geoAcc2D = geoAcc2D(m_h2_MC_dau0_2DMother, m_h2_RecoTrack_dau0_2DMother, "h2_dau0Mother_geoAcc2D",
                                            "geometrical acceptance dau_{0}, p_{T,mother} VS #theta_{mother}", m_histoList_GA);
    if (h2_dau0Mother_geoAcc2D->GetEntries() == 0) B2WARNING("Empty histogram h2_dau0Mother_geoAcc2D");

    TH2F* h2_dau1Mother_geoAcc2D = geoAcc2D(m_h2_MC_dau1_2DMother, m_h2_RecoTrack_dau1_2DMother, "h2_dau1Mother_geoAcc2D",
                                            "geometrical acceptance p, p_{T,mother} VS #theta_{mother}", m_histoList_GA);
    if (h2_dau1Mother_geoAcc2D->GetEntries() == 0) B2WARNING("Empty histogram h2_dau1Mother_geoAcc2D");

    //particles coming from inside the Beam Pipe
    TH2F* h2_dau0_geoAcc2D_BP = geoAcc2D(m_h2_MC_dau0_2D_BP, m_h2_RecoTrack_dau0_2D_BP, "h2_dau0_geoAcc2D_BP",
                                         "geometrical acceptance dau_{0}, p_{T,dau_{0}} VS #theta_{dau_{0}}, BP", m_histoList_GA);
    if (h2_dau0_geoAcc2D_BP->GetEntries() == 0) B2WARNING("Empty histogram h2_dau0_geoAcc2D_BP");

    TH2F* h2_dau1_geoAcc2D_BP = geoAcc2D(m_h2_MC_dau1_2D_BP, m_h2_RecoTrack_dau1_2D_BP, "h2_dau1_geoAcc2D_BP",
                                         "geometrical acceptance p, p_{T,p} VS #theta_{p}, BP", m_histoList_GA);
    if (h2_dau1_geoAcc2D_BP->GetEntries() == 0) B2WARNING("Empty histogram h2_dau1_geoAcc2D_BP");

    TH2F* h2_Mother_geoAcc2D_BP = geoAcc2D(m_h2_MC_Mother_2D_BP, m_h2_RecoTrack_Mother_2D_BP, "h2_Mother_geoAcc2D_BP",
                                           "geometrical acceptance mother, p_{T,mother} VS #theta_{mother}, BP", m_histoList_GA);
    if (h2_Mother_geoAcc2D_BP->GetEntries() == 0) B2WARNING("Empty histogram h2_Mother_geoAcc2D_BP");
  }

  //---------------------------------------------------------------------------------

  //write histograms on the output root file
  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TIter nextHMC(m_histoList_MCParticles);
    TIter nextHTC(m_histoList_RecoTracks);
    TIter nextHT(m_histoList_Tracks);
    TIter nextHEff(m_histoList_Efficiencies);
    TIter nextHGA(m_histoList_GA);
    TIter nextHCheck(m_histoList_check);

    TObject* obj;

    if (m_allHistograms) {
      TDirectory* MCdir = m_rootFilePtr->mkdir("MCParticles");
      MCdir->cd();
      while ((obj = nextHMC()))
        obj->Write("", BIT(2), 0);

      TDirectory* TCdir = m_rootFilePtr->mkdir("RecoTracks");
      TCdir->cd();
      while ((obj = nextHTC()))
        obj->Write("", BIT(2), 0);

      TDirectory* Trdir = m_rootFilePtr->mkdir("Tracks");
      Trdir->cd();
      while ((obj = nextHT()))
        obj->Write("", BIT(2), 0);

      TDirectory* check = m_rootFilePtr->mkdir("check");
      check->cd();
      while ((obj = nextHCheck()))
        obj->Write("", BIT(2), 0);
    }

    TDirectory* Effdir = m_rootFilePtr->mkdir("Efficiencies");
    Effdir->cd();
    while ((obj = nextHEff()))
      obj->Write("", BIT(2), 0);

    if (m_geometricalAccettance) {
      TDirectory* geoAcc = m_rootFilePtr->mkdir("geoAccettance");
      geoAcc->cd();
      while ((obj = nextHGA()))
        obj->Write("", BIT(2), 0);
    }


    m_rootFilePtr->Close();
  }
}

bool EffPlotsModule::isV0(const MCParticle& the_mcParticle)
{

  bool isGamma = false;
  if (abs(the_mcParticle.getPDG()) == 22)
    isGamma = true;

  bool isK_S0 = false;
  if (abs(the_mcParticle.getPDG()) == 310)
    isK_S0 = true;

  bool isK_0 = false;
  if (abs(the_mcParticle.getPDG()) == 311)
    isK_0 = true;

  bool isLambda = false;
  if (abs(the_mcParticle.getPDG()) == 3122)
    isLambda = true;

  bool twoChargedProngs = false;

  if (the_mcParticle.getDaughters().size() == 2)
    twoChargedProngs = true;

  if (twoChargedProngs)
    if (the_mcParticle.getDaughters()[0]->getCharge() == 0)
      twoChargedProngs = false;

  return ((isGamma || isK_S0 || isK_0 || isLambda) && twoChargedProngs);

}

bool EffPlotsModule::isK_Short(const MCParticle& the_mcParticle)
{

  bool isK_S0 = false;
  if (abs(the_mcParticle.getPDG()) == 310)
    isK_S0 = true;

  bool twoChargedProngs = false;

  if (the_mcParticle.getDaughters().size() == 2 &&
      ((the_mcParticle.getDaughters()[0]->getPDG() == 211 && the_mcParticle.getDaughters()[1]->getPDG() == -211) ||
       (the_mcParticle.getDaughters()[0]->getPDG() == -211 && the_mcParticle.getDaughters()[1]->getPDG() == 211)))
    twoChargedProngs = true;

  return (isK_S0 && twoChargedProngs);

}

bool EffPlotsModule::isLambda0(const MCParticle& the_mcParticle)
{

  bool isLambda = false;
  if (abs(the_mcParticle.getPDG()) == 3122)
    isLambda = true;

  bool twoChargedProngs = false;

  if (the_mcParticle.getDaughters().size() == 2 &&
      ((the_mcParticle.getDaughters()[0]->getPDG() == 211 && the_mcParticle.getDaughters()[1]->getPDG() == -2212) ||
       (the_mcParticle.getDaughters()[0]->getPDG() == -211 && the_mcParticle.getDaughters()[1]->getPDG() == 2212) ||
       (the_mcParticle.getDaughters()[0]->getPDG() == 2212 && the_mcParticle.getDaughters()[1]->getPDG() == -211) ||
       (the_mcParticle.getDaughters()[0]->getPDG() == -2212 && the_mcParticle.getDaughters()[1]->getPDG() == 211)))
    twoChargedProngs = true;

  return (isLambda && twoChargedProngs);

}

int EffPlotsModule::nMatchedDaughters(const MCParticle& the_mcParticle)
{

  int nMatchedDau = 0;

  std::vector< MCParticle* > MCPart_dau = the_mcParticle.getDaughters();

  bool first = false;
  bool second = false;

  RelationVector<Track> Tracks_fromMCParticle_0 = DataStore::getRelationsWithObj<Track>(MCPart_dau[0]);
  if (Tracks_fromMCParticle_0.size() > 0)
    first = true;

  RelationVector<Track> Tracks_fromMCParticle_1 = DataStore::getRelationsWithObj<Track>(MCPart_dau[1]);
  if (Tracks_fromMCParticle_1.size() > 0)
    second = true;


  if (first)
    nMatchedDau++;

  if (second)
    nMatchedDau++;


  return nMatchedDau;

}
