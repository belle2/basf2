/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackingPerformanceEvaluation/TrackingPerformanceEvaluationModule.h>
#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationVector.h>

#include <framework/geometry/BFieldManager.h>

#include <vxd/geometry/GeoCache.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>

#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <cdc/dataobjects/CDCHit.h>

#include <genfit/KalmanFitterInfo.h>

#include <root/TTree.h>
#include <root/TAxis.h>
#include <root/TObject.h>

#include <boost/foreach.hpp>

#include <typeinfo>
#include <cxxabi.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackingPerformanceEvaluation)

TrackingPerformanceEvaluationModule::TrackingPerformanceEvaluationModule() :
  Module()
{

  setDescription("This module evaluates the tracking package performance");

  addParam("outputFileName", m_rootFileName, "Name of output root file.",
           std::string("TrackingPerformanceEvaluation_output.root"));
  addParam("MCParticlesName", m_MCParticlesName, "Name of MC Particle collection.", std::string(""));
  addParam("TracksName", m_TracksName, "Name of Track collection.", std::string(""));
  addParam("RecoTracksName", m_RecoTracksName, "Name of RecoTrack collection.", std::string("RecoTracks"));
  addParam("MCRecoTracksName", m_MCRecoTracksName, "Name of MCRecoTrack collection.", std::string("MCRecoTracks"));
  addParam("ParticleHypothesis", m_ParticleHypothesis, "Particle Hypothesis used in the track fit.", int(211));

}

TrackingPerformanceEvaluationModule::~TrackingPerformanceEvaluationModule()
{

}

void TrackingPerformanceEvaluationModule::initialize()
{
  // MCParticles, Tracks, RecoTracks, MCRecoTracks needed for this module
  StoreArray<MCParticle> mcParticles;
  mcParticles.isRequired(m_MCParticlesName);

  StoreArray<RecoTrack> recoTracks;
  recoTracks.isRequired(m_RecoTracksName);

  StoreArray<RecoTrack> mcRecoTracks;
  mcRecoTracks.isRequired(m_MCRecoTracksName);

  StoreArray<Track> tracks;
  tracks.isRequired(m_TracksName);

  //create list of histograms to be saved in the rootfile
  m_histoList = new TList;
  m_histoList_multiplicity = new TList;
  m_histoList_evtCharacterization = new TList;
  m_histoList_trkQuality = new TList;
  m_histoList_firstHit = new TList;
  m_histoList_pr = new TList;
  m_histoList_fit = new TList;
  m_histoList_efficiency = new TList;
  m_histoList_purity = new TList;
  m_histoList_others = new TList;

  //set the ROOT File
  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  //now create histograms

  //multiplicity histograms
  m_multiplicityTracks = createHistogram1D("h1nTrk", "number of tracks per MC Particle", 8, -0.5, 7.5, "# tracks",
                                           m_histoList_multiplicity);

  m_multiplicityRecoTracks = createHistogram1D("h1nRcTrk", "number of recoTracks per MC Particle", 8, -0.5, 7.5, "# tracks",
                                               m_histoList_multiplicity);

  m_multiplicityMCRecoTracks = createHistogram1D("h1nMCRcTrk", "number of MC recoTracks per MC Particle", 8, -0.5, 7.5, "# tracks",
                                                 m_histoList_multiplicity);

  m_multiplicityFittedTracks = createHistogram1D("h1nFitTrk", "number of fitted tracks per MC Particle", 5, -0.5, 4.5,
                                                 "# fitted tracks", m_histoList_multiplicity);
  m_multiplicityFittedTracksPerMCRT = createHistogram1D("h1nFitTrkMCRT", "number of fitted tracks per MCRecoTrack", 5, -0.5, 4.5,
                                                        "# fitted tracks", m_histoList_multiplicity);
  m_multiplicityMCParticlesPerTrack = createHistogram1D("h1nMCPrtcl", "number of MCParticles per fitted tracks", 5, -0.5, 4.5,
                                                        "# MCParticles", m_histoList_multiplicity);
  m_multiplicityRecoTracksPerMCRT = createHistogram1D("h1nRecoTrack", "number of RecoTrack per MCRecoTrack", 5, -0.5, 4.5,
                                                      "# RecoTrack",
                                                      m_histoList_multiplicity);
  m_multiplicityMCRecoTracksPerRT = createHistogram1D("h1nMCRecoTrack", "number of MCRecoTrack per RecoTrack", 5, -0.5, 4.5,
                                                      "# MCRecoTrack", m_histoList_multiplicity);

  //tracks pValue
  m_h1_pValue = createHistogram1D("h1pValue", "pValue of the fit", 100, 0, 1, "pValue", m_histoList_trkQuality);


  //track parameters errors
  m_h1_d0_err = createHistogram1D("h1d0err", "d0 error", 100, 0, 0.1, "#sigma_{d0} (cm)", m_histoList_trkQuality);
  m_h1_phi_err = createHistogram1D("h1phierr", "#phi error", 100, 0, 0.02, "#sigma_{#phi} (rad)", m_histoList_trkQuality);
  m_h1_omega_err = createHistogram1D("h1omegaerr", "#omega error", 100, 0, 0.002, "#sigma_{#omega} (cm^{-1})",
                                     m_histoList_trkQuality);
  m_h1_z0_err = createHistogram1D("h1z0err", "z0 error", 100, 0, 0.1, "#sigma_{z0} (cm)", m_histoList_trkQuality);
  m_h1_cotTheta_err = createHistogram1D("h1cotThetaerr", "cot#theta error", 100, 0, 0.03, "#sigma_{cot#theta}",
                                        m_histoList_trkQuality);

  //track parameters residuals
  m_h1_d0_res = createHistogram1D("h1d0res", "d0 residuals", 100, -0.1, 0.1, "d0 resid (cm)", m_histoList_trkQuality);
  m_h1_phi_res = createHistogram1D("h1phires", "#phi residuals", 100, -0.1, 0.1, "#phi resid (rad)", m_histoList_trkQuality);
  m_h1_omega_res = createHistogram1D("h1omegares", "#omega residuals", 100, -0.0005, 0.0005, "#omega resid (cm^{-1})",
                                     m_histoList_trkQuality);
  m_h1_z0_res = createHistogram1D("h1z0res", "z0 residuals", 100, -0.1, 0.1, "z0 resid (cm)", m_histoList_trkQuality);
  m_h1_cotTheta_res = createHistogram1D("h1cotThetares", "cot#theta residuals", 100, -0.1, 0.1, "cot#theta resid",
                                        m_histoList_trkQuality);

  //track parameters residuals - momentum
  m_h1_px_res = createHistogram1D("h1pxres", "px absolute residuals", 200, -0.05, 0.05, "px_{reco} - px_{MC} (GeV/c)",
                                  m_histoList_trkQuality);
  m_h1_py_res = createHistogram1D("h1pyres", "py absolute residuals", 200, -0.05, 0.05, "py_{reco} - py_{MC} (GeV/c)",
                                  m_histoList_trkQuality);
  m_h1_pz_res = createHistogram1D("h1pzres", "pz absolute residuals", 200, -0.05, 0.05, "pz_{reco} - pz_{MC} (GeV/c)",
                                  m_histoList_trkQuality);
  m_h1_p_res = createHistogram1D("h1pres", "p relative residuals", 200, -0.05, 0.05, "p_{reco} - p_{MC} / p_{MC}",
                                 m_histoList_trkQuality);
  m_h1_pt_res = createHistogram1D("h1ptres", "pt relative residuals", 200, -0.05, 0.05, "pt_{reco} - pt_{MC} / pt_{MC}",
                                  m_histoList_trkQuality);
  //track parameters residuals - position
  m_h1_x_res = createHistogram1D("h1xres", " residuals", 200, -0.05, 0.05, "x_{reco} - x_{MC} (cm)", m_histoList_trkQuality);
  m_h1_y_res = createHistogram1D("h1yres", " residuals", 200, -0.05, 0.05, "y_{reco} - y_{MC} (cm)", m_histoList_trkQuality);
  m_h1_z_res = createHistogram1D("h1zres", " residuals", 200, -0.05, 0.05, "z_{reco} - z_{MC} (cm)", m_histoList_trkQuality);
  m_h1_r_res = createHistogram1D("h1rres", " residuals", 200, -0.05, 0.05, "r_{reco} - r_{MC} (cm)", m_histoList_trkQuality);
  m_h1_rtot_res = createHistogram1D("h1rtotres", " residuals", 200, -0.05, 0.05, "rtot_{reco} - rtot_{MC} (cm)",
                                    m_histoList_trkQuality);

  m_h2_chargeVSchargeMC  = createHistogram2D("h2chargecheck", "chargeVSchargeMC", 3, -1.5, 1.5, "charge MC", 3, -1.5, 1.5,
                                             "charge reco", m_histoList_trkQuality);

  //track parameters pulls
  m_h1_d0_pll = createHistogram1D("h1d0pll", "d0 pulls", 100, -5, 5, "d0 pull", m_histoList_trkQuality);
  m_h1_phi_pll = createHistogram1D("h1phipll", "#phi pulls", 100, -5, 5, "#phi pull", m_histoList_trkQuality);
  m_h1_omega_pll = createHistogram1D("h1omegapll", "#omega pulls", 100, -5, 5, "#omega pull", m_histoList_trkQuality);
  m_h1_z0_pll = createHistogram1D("h1z0pll", "z0 pulls", 100,  -5, 5, "z0 pull", m_histoList_trkQuality);
  m_h1_cotTheta_pll = createHistogram1D("h1cotThetapll", "cot#theta pulls", 100,  -5, 5, "cot#theta pull", m_histoList_trkQuality);


  //first hit position using track parameters errors
  m_h2_d0errphi0err_xy = createHistogram2D("h2d0errphierrXY", "#sigma_{d0}/#sigma_{#phi} projected on x,y",
                                           2000, -10, 10, "x (cm)",
                                           2000, -10, 10, "y (cm)",
                                           m_histoList_firstHit);

  m_h2_d0errphi0err_rz = createHistogram2D("h2d0errphierrRZ", "#sigma_{d0}/#sigma_{#phi} projected on z and r_{t}=#sqrt{x^{2}+y^{2}}",
                                           2000, -30, 40, "z (cm)",
                                           2000, 0, 15, "r_{t} (cm)",
                                           m_histoList_firstHit);

  m_h2_z0errcotThetaerr_xy = (TH2F*)duplicateHistogram("h2z0errcotThetaerrXY", "#sigma_{z0}/#sigma_{cot#theta} projected on x,y",
                                                       m_h2_d0errphi0err_xy,
                                                       m_histoList_firstHit);

  m_h2_OmegaerrOmegaVSpt = createHistogram2D("h2OmegaerrOmegaVSpt", "#sigma_{#omega}/#omega VS p_{t}",
                                             100, 0, 3, "p_{t} (GeV/c)",
                                             1000, 0, 0.2, "#sigma_{#omega}/#omega",
                                             m_histoList_firstHit);


  m_h2_z0errVSpt = createHistogram2D("h2z0errVSpt", "#sigma_{z0} VS p_{t}",
                                     100, 0, 3, "p_{t} (GeV/c)",
                                     100, 0, 0.1, "#sigma_{z0} (cm)",
                                     m_histoList_firstHit);

  m_h2_z0errVSpt_wpxd = (TH2F*)duplicateHistogram("h2z0errVSpt_wPXD", "#sigma_{z0} VS p_{t}, with PXD hits", m_h2_z0errVSpt,
                                                  m_histoList_firstHit);

  m_h2_z0errVSpt_wopxd = (TH2F*)duplicateHistogram("h2z0errVSpt_woPXD", "#sigma_{z0} VS p_{t}, no PXD hits", m_h2_z0errVSpt,
                                                   m_histoList_firstHit);

  m_h2_d0errVSpt = createHistogram2D("h2d0errVSpt", "#sigma_{d0} VS p_{t}",
                                     100, 0, 3, "p_{t} (GeV/c)",
                                     100, 0, 0.1, "#sigma_{d0} (cm)",
                                     m_histoList_firstHit);

  m_h2_d0errVSpt_wpxd = (TH2F*)duplicateHistogram("h2d0errVSpt_wPXD", "#sigma_{d0} VS p_{t}, with PXD hits", m_h2_d0errVSpt,
                                                  m_histoList_firstHit);

  m_h2_d0errVSpt_wopxd = (TH2F*)duplicateHistogram("h2d0errVSpt_woPXD", "#sigma_{d0} VS p_{t}, no PXD hits", m_h2_d0errVSpt,
                                                   m_histoList_firstHit);

  m_h2_d0errMSVSpt = createHistogram2D("h2d0errMSVSpt", "#sigma_{d0} * #betapsin^{3/2}#theta VS p_{t}",
                                       50, 0, 2.5, "p_{t} (GeV/c)",
                                       500, 0, 1, "cm",
                                       m_histoList_firstHit);

  //hits used in the fit
  m_h2_TrackPointFitWeightVXD = createHistogram2D("h2TPFitWeightVXD", "VXD TrackPoint Fit Weight", 6, 0.5, 6.5, "VXD layer", 20, 0, 1,
                                                  "weight", m_histoList);
  m_h2_TrackPointFitWeightCDC = createHistogram2D("h2TPFitWeightCDC", "CDC TrackPoint Fit Weight", 56, -0.5, 55.5, "CDC layer", 20, 0,
                                                  1, "weight", m_histoList);

  m_h1_nHitDetID = createHistogram1D("h1nHitDetID", "detector ID per hit", 4, -0.5, 3.5, "0=PXD, 1=SVD2D, 2=SVD,3=CDC", m_histoList);
  m_h1_nCDChitsPR = createHistogram1D("h1nCDCHitsPR", "number of CDC hits from PR per Layer", 56, -0.5, 55.5, "CDC Layer",
                                      m_histoList);
  m_h1_nCDChitsWeighted = (TH1F*)duplicateHistogram("h1nCDCHitsWeighted", "CDC hits used in the fit per Layer, weighted",
                                                    m_h1_nCDChitsPR, m_histoList);
  m_h1_nCDChitsUsed = (TH1F*)duplicateHistogram("h1nCDCHitsUsed",
                                                "approximated number of CDC hits used in the fit per Layer, weighted", m_h1_nCDChitsPR, m_histoList);
  m_h1_nVXDhitsPR = createHistogram1D("h1nVXDHitsPR", "number of VXD hits from PR per Layer", 6, 0.5, 6.5, "VXD Layer", m_histoList);
  m_h1_nVXDhitsWeighted = (TH1F*)duplicateHistogram("h1nVXDHitsWeighted", "number of VXD hits used in the fit per Layer, weighted",
                                                    m_h1_nVXDhitsPR, m_histoList);
  m_h1_nVXDhitsUsed = (TH1F*)duplicateHistogram("h1nVXDHitsUsed",
                                                "approximate number of VXD hits used in the fit per Layer, weighted", m_h1_nVXDhitsPR, m_histoList);
  m_h2_VXDhitsPR_xy = createHistogram2D("h2hitsPRXY", "Pattern Recognition hits, transverse plane",
                                        2000, -15, 15, "x (cm)",
                                        2000, -15, 15, "y (cm)",
                                        m_histoList);

  m_h2_VXDhitsPR_rz = createHistogram2D("h2hitsPRRZ", "Pattern Recognition Hits, r_{t} z",
                                        2000, -30, 40, "z (cm)",
                                        2000, 0, 15, "r_{t} (cm)",
                                        m_histoList);



  //histograms to produce efficiency plots
  Double_t bins_pt[10 + 1] = {0, 0.05, 0.1, 0.15, 0.2, 0.3, 0.5, 1, 1.5, 2, 3.5}; //GeV/c
  Double_t bins_theta[10 + 1] = {0, 0.25, 0.5, 0.75, 0.75 + 0.32, 0.75 + 2 * 0.32, 0.75 + 3 * 0.32, 0.75 + 4 * 0.32, 0.75 + 5 * 0.32, 2.65, TMath::Pi()};
  Double_t bins_phi[14 + 1];
  Double_t width_phi = 2 * TMath::Pi() / 14;
  for (int bin = 0; bin < 14 + 1; bin++)
    bins_phi[bin] = - TMath::Pi() + bin * width_phi;


  m_h3_MCParticle = createHistogram3D("h3MCParticle", "entry per MCParticle",
                                      10, bins_pt, "p_{t} (GeV/c)",
                                      10, bins_theta, "#theta",
                                      14, bins_phi, "#phi" /*, m_histoList*/);

  m_h3_TracksPerMCParticle = (TH3F*)duplicateHistogram("h3TracksPerMCParticle",
                                                       "entry per Track connected to a MCParticle",
                                                       m_h3_MCParticle /*, m_histoList*/);

  m_h3_TrackswPXDHitsPerMCParticle = (TH3F*)duplicateHistogram("h3TrackswPXDHitsPerMCParticle",
                                     "entry per Track with PXD hits connected to a MCParticle",
                                     m_h3_MCParticle /*, m_histoList*/);

  m_h3_RecoTrackswPXDHitsPerMCParticle = (TH3F*)duplicateHistogram("h3RecoTrackswPXDHitsPerMCParticle",
                                         "entry per RecoTrack with PXD hits connected to a MCParticle",
                                         m_h3_MCParticle /*, m_histoList*/);

  m_h3_RecoTrackswPXDHitsPerMCParticlewPXDHits = (TH3F*)duplicateHistogram("h3RecoTrackswPXDHitsPerMCParticlewPXDHits",
                                                 "entry per RecoTrack with PXD hits connected to a MCParticle with PXD hits",
                                                 m_h3_MCParticle /*, m_histoList*/);

  m_h3_MCParticleswPXDHits = (TH3F*)duplicateHistogram("h3MCParticleswPXDHitsPerMCParticle",
                                                       "entry per MCParticle with PXD hits",
                                                       m_h3_MCParticle /*, m_histoList*/);

  m_h3_MCRecoTrack = (TH3F*)duplicateHistogram("h3MCRecoTrack",
                                               "entry per MCRecoTrack connected to the MCParticle",
                                               m_h3_MCParticle /*, m_histoList*/);

  m_h3_TracksPerMCRecoTrack = (TH3F*)duplicateHistogram("h3TracksPerMCRecoTrack",
                                                        "entry per Track connected to an MCRecoTrack",
                                                        m_h3_MCParticle /*, m_histoList*/);
  //plus
  m_h3_MCParticle_plus = (TH3F*)duplicateHistogram("h3MCParticle_plus", "entry per positive MCParticle",
                                                   m_h3_MCParticle /*, m_histoList*/);

  m_h3_TracksPerMCParticle_plus = (TH3F*)duplicateHistogram("h3TracksPerMCParticle_plus",
                                                            "entry per Track connected to a positive MCParticle",
                                                            m_h3_MCParticle /*, m_histoList*/);

  m_h3_MCRecoTrack_plus = (TH3F*)duplicateHistogram("h3MCRecoTrack_plus",
                                                    "entry per MCRecoTrack connected to the positive MCParticle",
                                                    m_h3_MCParticle /*, m_histoList*/);

  m_h3_TracksPerMCRecoTrack_plus = (TH3F*)duplicateHistogram("h3TracksPerMCRecoTrack_plus",
                                                             "entry per Track connected to a positive MCRecoTrack",
                                                             m_h3_MCParticle /*, m_histoList*/);


  //minus
  m_h3_MCParticle_minus = (TH3F*)duplicateHistogram("h3MCParticlee_minus", "entry per negative MCParticle",
                                                    m_h3_MCParticle /*, m_histoList*/);

  m_h3_TracksPerMCParticle_minus = (TH3F*)duplicateHistogram("h3TracksPerMCParticle_minus",
                                                             "entry per Track connected to a negative MCParticle",
                                                             m_h3_MCParticle /*, m_histoList*/);

  m_h3_MCRecoTrack_minus = (TH3F*)duplicateHistogram("h3MCRecoTrack_minus",
                                                     "entry per MCRecoTrack connected to the negative MCParticle",
                                                     m_h3_MCParticle /*, m_histoList*/);

  m_h3_TracksPerMCRecoTrack_minus = (TH3F*)duplicateHistogram("h3TracksPerMCRecoTrack_minus",
                                                              "entry per Track connected to a negative MCRecoTrack",
                                                              m_h3_MCParticle /*, m_histoList*/);

  //histograms to produce efficiency plots
  m_h1_HitsRecoTrackPerMCRecoTrack = createHistogram1D("h1hitsTCperMCRT", "RecoTrack per MCRecoTrack Hit in VXD layers", 6, 0.5, 6.5,
                                                       "# VXD layer" /*, m_histoList*/);

  m_h1_HitsMCRecoTrack = (TH1F*) duplicateHistogram("h1hitsMCRT", " MCRecoTrack Hit in VXD layers",
                                                    m_h1_HitsRecoTrackPerMCRecoTrack /*,  m_histoList*/);


  //histograms to produce purity plots
  m_h3_Tracks = (TH3F*)duplicateHistogram("h3Tracks", "entry per Track",
                                          m_h3_MCParticle /*, m_histoList*/);

  m_h3_MCParticlesPerTrack = (TH3F*)duplicateHistogram("h3MCParticlesPerTrack",
                                                       "entry per MCParticle connected to a Track",
                                                       m_h3_MCParticle /*, m_histoList*/);
}

void TrackingPerformanceEvaluationModule::beginRun()
{

}

void TrackingPerformanceEvaluationModule::event()
{

  StoreArray<MCParticle> mcParticles(m_MCParticlesName);

  B2Vector3D magField = BFieldManager::getField(0, 0, 0) / Unit::T;

  bool hasTrack = false;
  B2DEBUG(99, "+++++ 1. loop on MCParticles");
  BOOST_FOREACH(MCParticle & mcParticle, mcParticles) {

    if (! isTraceable(mcParticle))
      continue;

    int pdgCode = mcParticle.getPDG();
    B2DEBUG(99, "MCParticle has PDG code " << pdgCode);

    int nFittedTracksMCRT = 0;
    int nFittedTracks = 0;
    int nFittedTrackswPXDHits = 0;

    MCParticleInfo mcParticleInfo(mcParticle, magField);

    hasTrack = false;

    m_h3_MCParticle->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());

    if (mcParticleInfo.getCharge() > 0)
      m_h3_MCParticle_plus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
    else if (mcParticleInfo.getCharge() < 0)
      m_h3_MCParticle_minus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
    else
      continue;

    if (mcParticle.hasSeenInDetector(Const::PXD))
      m_h3_MCParticleswPXDHits->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());

    //1. retrieve all the Tracks related to the MCParticle

    //1.0 check if there is a RecoTrack
    RelationVector<RecoTrack> MCRecoTracks_fromMCParticle =
      DataStore::getRelationsWithObj<RecoTrack>(&mcParticle, m_MCRecoTracksName);

    if (MCRecoTracks_fromMCParticle.size() > 0)
      if (MCRecoTracks_fromMCParticle[0]->hasPXDHits()) {
        m_h3_RecoTrackswPXDHitsPerMCParticle->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
        if (mcParticle.hasSeenInDetector(Const::PXD))
          m_h3_RecoTrackswPXDHitsPerMCParticlewPXDHits->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
      }
    m_multiplicityMCRecoTracks->Fill(MCRecoTracks_fromMCParticle.size());

    RelationVector<RecoTrack> RecoTracks_fromMCParticle =
      DataStore::getRelationsWithObj<RecoTrack>(&mcParticle, m_RecoTracksName);

    m_multiplicityRecoTracks->Fill(RecoTracks_fromMCParticle.size());

    if (MCRecoTracks_fromMCParticle.size() > 0) {
      m_h3_MCRecoTrack->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());

      if (mcParticleInfo.getCharge() > 0)
        m_h3_MCRecoTrack_plus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
      else if (mcParticleInfo.getCharge() < 0)
        m_h3_MCRecoTrack_minus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
      else
        continue;
    }

    //1.a retrieve all Tracks related to the MCParticle
    RelationVector<Track> Tracks_fromMCParticle = DataStore::getRelationsWithObj<Track>(&mcParticle);
    m_multiplicityTracks->Fill(Tracks_fromMCParticle.size());

    B2DEBUG(99, Tracks_fromMCParticle.size() << " Tracks related to this MCParticle");

    for (int trk = 0; trk < (int)Tracks_fromMCParticle.size(); trk++) {

      const TrackFitResult* fitResult = Tracks_fromMCParticle[trk]->getTrackFitResult(Const::ChargedStable(m_ParticleHypothesis));

      if ((fitResult == NULL) || (fitResult->getParticleType() != Const::ChargedStable(m_ParticleHypothesis)))
        B2DEBUG(99, " the TrackFitResult is not found!");

      else { // valid TrackFitResult found

        if (!hasTrack) {

          hasTrack = true;

          nFittedTracks++;

          if (fitResult->getHitPatternVXD().getNPXDHits() > 0) {
            m_h3_TrackswPXDHitsPerMCParticle->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
            nFittedTrackswPXDHits++;
          }

          m_h3_TracksPerMCParticle->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
          if (mcParticleInfo.getCharge() > 0)
            m_h3_TracksPerMCParticle_plus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
          else if (mcParticleInfo.getCharge() < 0)
            m_h3_TracksPerMCParticle_minus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
          else
            continue;

          if (MCRecoTracks_fromMCParticle.size() > 0) {
            nFittedTracksMCRT++;
            m_h3_TracksPerMCRecoTrack->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
            if (mcParticleInfo.getCharge() > 0)
              m_h3_TracksPerMCRecoTrack_plus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
            else if (mcParticleInfo.getCharge() < 0)
              m_h3_TracksPerMCRecoTrack_minus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
            else
              continue;
          }


        }


        fillTrackParams1DHistograms(fitResult, mcParticleInfo);

      }
    }

    m_multiplicityFittedTracks->Fill(nFittedTracks);
    if (MCRecoTracks_fromMCParticle.size() > 0)
      m_multiplicityFittedTracksPerMCRT->Fill(nFittedTracksMCRT);

  }


  B2DEBUG(99, "+++++ 2. loop on Tracks");

  //2. retrieve all the MCParticles related to the Tracks
  StoreArray<Track> tracks(m_TracksName);

  BOOST_FOREACH(Track & track, tracks) {

    int nMCParticles = 0;

    //check if the track has been fitted
    const TrackFitResult* fitResult = track.getTrackFitResult(Const::ChargedStable(m_ParticleHypothesis));

    if ((fitResult == NULL) || (fitResult->getParticleType() != Const::ChargedStable(m_ParticleHypothesis)))
      continue;

    m_h1_pValue->Fill(fitResult->getPValue());

    TVector3 momentum = fitResult->getMomentum();
    m_h3_Tracks->Fill(momentum.Pt(), momentum.Theta(), momentum.Phi());

    fillTrackErrParams2DHistograms(fitResult);

    fillHitsUsedInTrackFitHistograms(track);

    for (int layer = 0; layer < 56; layer++) {
      if (fitResult->getHitPatternCDC().hasLayer(layer))
        m_h1_nCDChitsUsed->Fill(layer);
    }
    for (int layer = 1; layer <= 2; layer++) {
      for (int i = 0; i < fitResult->getHitPatternVXD().getPXDLayer(layer); i++)
        m_h1_nVXDhitsUsed->Fill(layer);
    }
    for (int layer = 3; layer <= 6; layer++) {
      int n1 = fitResult->getHitPatternVXD().getSVDLayer(layer).first;
      int n2 = fitResult->getHitPatternVXD().getSVDLayer(layer).second;
      int N = n1 + n2;

      for (int i = 0; i < N; i++)
        m_h1_nVXDhitsUsed->Fill(layer);
    }


    RelationVector<MCParticle> MCParticles_fromTrack = DataStore::getRelationsWithObj<MCParticle>(&track);

    for (int mcp = 0; mcp < (int)MCParticles_fromTrack.size(); mcp++)
      if (isTraceable(*MCParticles_fromTrack[mcp])) {
        nMCParticles ++;
        m_h3_MCParticlesPerTrack->Fill(momentum.Pt(), momentum.Theta(), momentum.Phi());
      }
    //    }

    m_multiplicityMCParticlesPerTrack->Fill(nMCParticles);

  }


  B2DEBUG(99, "+++++ 3. loop on MCRecoTracks");

  //3. retrieve all MCRecoTracks
  StoreArray<RecoTrack> mcRecoTracks(m_MCRecoTracksName);
  StoreArray<PXDCluster> pxdClusters;
  StoreArray<SVDCluster> svdClusters;
  StoreArray<CDCHit> cdcHit;

  bool hasRecoTrack = false;

  BOOST_FOREACH(RecoTrack & mcRecoTrack, mcRecoTracks) {

    int nRecoTrack = 0;
    hasRecoTrack = false;

    //3.a retrieve the RecoTrack
    RelationVector<RecoTrack> RecoTracks_fromMCRecoTrack = DataStore::getRelationsWithObj<RecoTrack>(&mcRecoTrack);
    B2DEBUG(99, "~ " << RecoTracks_fromMCRecoTrack.size() << " RecoTracks related to this MCRecoTrack");
    m_multiplicityRecoTracksPerMCRT->Fill(RecoTracks_fromMCRecoTrack.size());

    //3.a retrieve the MCParticle
    RelationVector<MCParticle> MCParticles_fromMCRecoTrack = DataStore::getRelationsWithObj<MCParticle>(&mcRecoTrack);

    B2DEBUG(99, "~~~ " << MCParticles_fromMCRecoTrack.size() << " MCParticles related to this MCRecoTrack");
    for (int mcp = 0; mcp < (int)MCParticles_fromMCRecoTrack.size(); mcp++) {

      //3.b retrieve all RecoTracks related to the MCRecoTrack
      RelationVector<RecoTrack> RecoTracks_fromMCParticle = DataStore::getRelationsWithObj<RecoTrack>
                                                            (MCParticles_fromMCRecoTrack[mcp]);

      B2DEBUG(99, "~~~~~ " << RecoTracks_fromMCParticle.size() << " RecoTracks related to this MCParticle");
      for (int tc = 0; tc < (int)RecoTracks_fromMCParticle.size(); tc++)

        if (!hasRecoTrack) {

          hasRecoTrack = true;
          nRecoTrack++;

          /*
                genfit::TrackCandHit* thehitMCRT = 0;
                for (int hitMCRT = 0; hitMCRT < (int)mcTrackCand.getNHits(); hitMCRT++) {

                  thehitMCRT = mcTrackCand.getHit(hitMCRT);
                  if (!thehitMCRT)
                    continue;

                  int hitId = thehitMCRT->getHitId();
                  int detId = thehitMCRT->getDetId();
                  if (detId == 1)
                    m_h1_HitsMCRecoTrack->Fill(pxdClusters[hitId]->getSensorID().getLayerNumber());
                  if (detId == 2)
                    m_h1_HitsMCRecoTrack->Fill(svdClusters[hitId]->getSensorID().getLayerNumber());
                  //      if(thehitMCRT->getDetId() == 3)
                  //        m_h1_HitsMCRecoTrack->Fill( cdcHit[hitId]->getLayer() );

                  genfit::TrackCandHit* thehitTC = 0;
                  for (int hitTC = 0; hitTC < (int)TrackCands_fromMCParticle[tc]->getNHits(); hitTC++) {

                    thehitTC = TrackCands_fromMCParticle[tc]->getHit(hitTC);
                    if (!thehitTC)
                      continue;

                    if ((*thehitTC) == (*thehitMCRT)) {
                      if (detId == Const::PXD)
                        m_h1_HitsRecoTrackPerMCRecoTrack->Fill(pxdClusters[hitId]->getSensorID().getLayerNumber());
                      if (detId == Const::SVD)
                        m_h1_HitsRecoTrackPerMCRecoTrack->Fill(svdClusters[hitId]->getSensorID().getLayerNumber());
                      continue;
                    }
                  }
            }
          */
        }

    }

  }


  B2DEBUG(99, "+++++ 4. loop on RecoTracks");

  //4. retrieve all RecoTracks
  StoreArray<RecoTrack> RecoTracks;

  BOOST_FOREACH(RecoTrack & recoTrack, RecoTracks) {

    //   int nMCRecoTrack = 0;

    // retrieve the MCRecoTrack
    RelationVector<RecoTrack> MCRecoTracks_fromRecoTrack = DataStore::getRelationsWithObj<RecoTrack>(&recoTrack, m_MCRecoTracksName);
    m_multiplicityMCRecoTracksPerRT->Fill(MCRecoTracks_fromRecoTrack.size());


    /*
    //4.a retrieve the MCParticle
    RelationVector<MCParticle> MCParticles_fromRecoTrack = DataStore::getRelationsWithObj<MCParticle>(&recoTrack);

    B2DEBUG(99, "~~~ " << MCParticles_fromRecoTrack.size() << " MCParticles related to this RecoTrack");
    for (int mcp = 0; mcp < (int)MCParticles_fromRecoTrack.size(); mcp++) {

      //4.b retrieve all MCRecoTracks related to the RecoTrack
      RelationVector<RecoTrack> mcRecoTracks_fromMCParticle = DataStore::getRelationsWithObj<RecoTrack>
    (MCParticles_fromRecoTrack[mcp], m_MCRecoTracksName);

      B2DEBUG(99, "~~~~~ " << mcRecoTracks_fromMCParticle.size() << " MCRecoTracks related to this MCParticle");
      for (int mctc = 0; mctc < (int)mcRecoTracks_fromMCParticle.size(); mctc++) {
        nMCRecoTrack++;

      }
    }

    //    m_multiplicityMCRecoTracksPerRT->Fill(nMCRecoTrack);
    */
  }

}

void TrackingPerformanceEvaluationModule::endRun()
{

  double num = 0;
  double den = 0;

  for (int bin = 1; bin < m_multiplicityFittedTracks->GetNbinsX(); bin ++)
    num += m_multiplicityFittedTracks->GetBinContent(bin + 1);
  den = m_multiplicityFittedTracks->GetEntries();
  double efficiency = num / den ;
  double efficiencyErr =  sqrt(efficiency * (1 - efficiency)) / sqrt(den);

  double nFittedTracksMCRT = 0;
  for (int bin = 1; bin < m_multiplicityFittedTracksPerMCRT->GetNbinsX(); bin ++)
    nFittedTracksMCRT += m_multiplicityFittedTracksPerMCRT->GetBinContent(bin + 1);
  double efficiencyMCRT = nFittedTracksMCRT / m_multiplicityFittedTracksPerMCRT->GetEntries();
  double efficiencyMCRTErr =  sqrt(efficiencyMCRT * (1 - efficiencyMCRT)) / sqrt(m_multiplicityFittedTracksPerMCRT->GetEntries());

  double nRecoTrack = 0;
  for (int bin = 1; bin < m_multiplicityRecoTracksPerMCRT->GetNbinsX(); bin ++)
    nRecoTrack += m_multiplicityRecoTracksPerMCRT->GetBinContent(bin + 1);
  double efficiencyPR = nRecoTrack / m_multiplicityRecoTracksPerMCRT->GetEntries();
  double efficiencyPRErr =  sqrt(efficiencyPR * (1 - efficiencyPR)) / sqrt(m_multiplicityRecoTracksPerMCRT->GetEntries());

  double nMCRecoTrack = 0;
  for (int bin = 1; bin < m_multiplicityMCRecoTracksPerRT->GetNbinsX(); bin ++)
    nMCRecoTrack += m_multiplicityMCRecoTracksPerRT->GetBinContent(bin + 1);
  double purityPR = nMCRecoTrack / m_multiplicityMCRecoTracksPerRT->GetEntries();
  double purityPRErr =  sqrt(purityPR * (1 - purityPR)) / sqrt(m_multiplicityMCRecoTracksPerRT->GetEntries());

  double nMCParticles = 0;
  for (int bin = 1; bin < m_multiplicityMCParticlesPerTrack->GetNbinsX(); bin ++)
    nMCParticles += m_multiplicityMCParticlesPerTrack->GetBinContent(bin + 1);
  double purity = nMCParticles / m_multiplicityMCParticlesPerTrack->GetEntries();
  double purityErr =  sqrt(purity * (1 - purity)) / sqrt(m_multiplicityMCParticlesPerTrack->GetEntries());

  B2INFO("");
  B2INFO("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  B2INFO("~ Tracking Performance Evaluation ~ SHORT SUMMARY ~");
  B2INFO("");
  B2INFO(" + overall:");
  B2INFO("   efficiency = (" << efficiency * 100 << " +/- " << efficiencyErr * 100 << ")% ");
  B2INFO("       purity = " << purity * 100 << " +/- " << purityErr * 100 << ")% ");
  B2INFO("");
  B2INFO(" + factorizing geometrical acceptance:");
  B2INFO("   efficiency = " << efficiencyMCRT * 100 << " +/- " << efficiencyMCRTErr * 100 << ")% ");
  B2INFO("");
  B2INFO(" + pattern recognition:");
  B2INFO("   efficiency = " << efficiencyPR * 100 << " +/- " << efficiencyPRErr * 100 << ")% ");
  B2INFO("       purity = " << purityPR * 100 << " +/- " << purityPRErr * 100 << ")% ");
  B2INFO("");
  B2INFO("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
}

void TrackingPerformanceEvaluationModule::terminate()
{


  addMoreEfficiencyPlots(m_histoList_efficiency);
  addMoreInefficiencyPlots(m_histoList_efficiency);

  addPurityPlots(m_histoList, m_h3_MCParticlesPerTrack, m_h3_Tracks);

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TDirectory* oldDir = gDirectory;

    TDirectory* dir_multiplicity = oldDir->mkdir("multiplicity");
    dir_multiplicity->cd();
    TIter nextH_multiplicity(m_histoList_multiplicity);
    TObject* obj;
    while ((obj = nextH_multiplicity()))
      obj->Write();

    TDirectory* dir_efficiency = oldDir->mkdir("efficiency");
    dir_efficiency->cd();
    TIter nextH_efficiency(m_histoList_efficiency);
    while ((obj = nextH_efficiency()))
      obj->Write();

    TDirectory* dir_trkQuality = oldDir->mkdir("trkQuality");
    dir_trkQuality->cd();
    TIter nextH_trkQuality(m_histoList_trkQuality);
    while ((obj = nextH_trkQuality()))
      obj->Write();

    TDirectory* dir_firstHit = oldDir->mkdir("firstHit");
    dir_firstHit->cd();
    TIter nextH_firstHit(m_histoList_firstHit);
    while ((obj = nextH_firstHit()))
      obj->Write();




    m_rootFilePtr->Close();
  }

}


void  TrackingPerformanceEvaluationModule::fillTrackParams1DHistograms(const TrackFitResult* fitResult,
    MCParticleInfo mcParticleInfo)
{

  //track parameters errors
  double d0_err = sqrt((fitResult->getCovariance5())[0][0]);
  double phi_err =  sqrt((fitResult->getCovariance5())[1][1]);
  double omega_err = sqrt((fitResult->getCovariance5())[2][2]);
  double z0_err =  sqrt((fitResult->getCovariance5())[3][3]);
  double cotTheta_err = sqrt((fitResult->getCovariance5())[4][4]);

  //track parameters residuals:
  double d0_res = fitResult->getD0() - mcParticleInfo.getD0();
  double phi_res = TMath::ASin(TMath::Sin(fitResult->getPhi() - mcParticleInfo.getPhi()));   //giulia
  double omega_res =  fitResult->getOmega() - mcParticleInfo.getOmega();
  double z0_res = fitResult->getZ0() - mcParticleInfo.getZ0();
  double cotTheta_res = fitResult->getCotTheta() - mcParticleInfo.getCotTheta();

  //track parameters residuals in momentum:
  double px_res = fitResult->getMomentum().X() - mcParticleInfo.getPx();
  double py_res = fitResult->getMomentum().Y() - mcParticleInfo.getPy();
  double pz_res = fitResult->getMomentum().Z() - mcParticleInfo.getPz();
  double p_res = (fitResult->getMomentum().Mag() - mcParticleInfo.getP()) / mcParticleInfo.getP();
  double pt_res = (fitResult->getMomentum().Pt() - mcParticleInfo.getPt()) / mcParticleInfo.getPt();

  //track parameters residuals in position:
  double x_res = fitResult->getPosition().X() - mcParticleInfo.getX();
  double y_res = fitResult->getPosition().Y() - mcParticleInfo.getY();
  double z_res = fitResult->getPosition().Z() - mcParticleInfo.getZ();
  double r_res = fitResult->getPosition().Perp() - sqrt(mcParticleInfo.getX() * mcParticleInfo.getX() + mcParticleInfo.getY() *
                                                        mcParticleInfo.getY());
  double rtot_res = fitResult->getPosition().Mag() - sqrt(mcParticleInfo.getX() * mcParticleInfo.getX() + mcParticleInfo.getY() *
                                                          mcParticleInfo.getY() + mcParticleInfo.getZ() * mcParticleInfo.getZ());

  m_h1_d0_err->Fill(d0_err);
  m_h1_phi_err->Fill(phi_err);
  m_h1_omega_err->Fill(omega_err);
  m_h1_z0_err->Fill(z0_err);
  m_h1_cotTheta_err->Fill(cotTheta_err);

  m_h1_d0_res->Fill(d0_res);
  m_h1_phi_res->Fill(phi_res);
  m_h1_omega_res->Fill(omega_res);
  m_h1_z0_res->Fill(z0_res);
  m_h1_cotTheta_res->Fill(cotTheta_res);

  m_h1_px_res->Fill(px_res);
  m_h1_py_res->Fill(py_res);
  m_h1_pz_res->Fill(pz_res);
  m_h1_p_res->Fill(p_res);
  m_h1_pt_res->Fill(pt_res);

  m_h1_x_res->Fill(x_res);
  m_h1_y_res->Fill(y_res);
  m_h1_z_res->Fill(z_res);
  m_h1_r_res->Fill(r_res);
  m_h1_rtot_res->Fill(rtot_res);

  m_h2_chargeVSchargeMC->Fill(mcParticleInfo.getCharge(), fitResult->getChargeSign());

  m_h1_d0_pll->Fill(d0_res / d0_err);
  m_h1_phi_pll->Fill(phi_res / phi_err);
  m_h1_omega_pll->Fill(omega_res / omega_err);
  m_h1_z0_pll->Fill(z0_res / z0_err);
  m_h1_cotTheta_pll->Fill(cotTheta_res / cotTheta_err);


  m_h2_OmegaerrOmegaVSpt->Fill(fitResult->getMomentum().Pt(), omega_err / mcParticleInfo.getOmega());


}

void  TrackingPerformanceEvaluationModule::fillTrackErrParams2DHistograms(const TrackFitResult* fitResult)
{


  double d0_err = sqrt((fitResult->getCovariance5())[0][0]);
  double phi_err =  sqrt((fitResult->getCovariance5())[1][1]);
  double z0_err =  sqrt((fitResult->getCovariance5())[3][3]);
  double cotTheta_err = sqrt((fitResult->getCovariance5())[4][4]);

  TVector3 momentum = fitResult->getMomentum();

  double px = momentum.Px();
  double py = momentum.Py();
  double pz = momentum.Pz();
  double pt = momentum.Pt();
  double p = momentum.Mag();
  double mass = fitResult->getParticleType().getMass();
  double beta = p / sqrt(p * p + mass * mass);
  double sinTheta = TMath::Sin(momentum.Theta());

  m_h2_d0errphi0err_xy->Fill(d0_err / phi_err * px / pt,
                             d0_err / phi_err * py / pt);
  m_h2_z0errcotThetaerr_xy->Fill(z0_err / cotTheta_err * px / pt,
                                 z0_err / cotTheta_err * py / pt);
  m_h2_d0errphi0err_rz->Fill(d0_err / phi_err * pz / pt,
                             d0_err / phi_err);

  m_h2_d0errVSpt->Fill(pt, d0_err);

  m_h2_z0errVSpt->Fill(pt, z0_err);

  m_h2_d0errMSVSpt->Fill(pt, d0_err * beta * p * pow(sinTheta, 3 / 2) / 0.0136);

}

void TrackingPerformanceEvaluationModule::fillHitsUsedInTrackFitHistograms(const Track& theTrack)
{

  //hits used in the fit

  const TrackFitResult* fitResult = theTrack.getTrackFitResult(Const::ChargedStable(m_ParticleHypothesis));

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  bool hasPXDhit = false;
  double d0_err = -999;
  double z0_err = -999;
  double pt = -999;


  if ((fitResult != NULL)
      || (fitResult->getParticleType() != Const::ChargedStable(m_ParticleHypothesis))) { // valid TrackFitResult found
    d0_err = sqrt((fitResult->getCovariance5())[0][0]);
    z0_err = sqrt((fitResult->getCovariance5())[3][3]);
    pt = fitResult->getMomentum().Pt();
  }

  bool hasCDChit[56] = { false };

  RelationVector<RecoTrack> RecoTracks_fromTrack = DataStore::getRelationsWithObj<RecoTrack>(&theTrack);

  for (int tc = 0; tc < (int)RecoTracks_fromTrack.size(); tc++) {

    const std::vector< genfit::TrackPoint* >& tp_vector = RecoTracks_fromTrack[tc]->getHitPointsWithMeasurement();
    for (int i = 0; i < (int) tp_vector.size(); i++) {
      genfit::TrackPoint* tp = tp_vector[i];

      int nMea = tp->getNumRawMeasurements();
      for (int mea = 0; mea < nMea; mea++) {

        genfit::AbsMeasurement* absMeas = tp->getRawMeasurement(mea);
        double weight = 0;

        std::vector<double> weights;
        genfit::KalmanFitterInfo* kalmanInfo = tp->getKalmanFitterInfo();
        if (kalmanInfo)
          weights = kalmanInfo->getWeights();
        else //no kalman fitter info, fill the weights vector with 0 (VXD), or 0,0 (CDC)
          B2WARNING(" No KalmanFitterInfo associated to the TrackPoint!");

        double detId(-999);
        TVector3 globalHit(-999, -999, -999);

        PXDRecoHit* pxdHit =  dynamic_cast<PXDRecoHit*>(absMeas);
        SVDRecoHit2D* svdHit2D =  dynamic_cast<SVDRecoHit2D*>(absMeas);
        SVDRecoHit* svdHit =  dynamic_cast<SVDRecoHit*>(absMeas);
        CDCRecoHit* cdcHit =  dynamic_cast<CDCRecoHit*>(absMeas);

        if (pxdHit) {
          hasPXDhit = true;

          if (kalmanInfo)
            weight = weights.at(mea);

          detId = 0;
          double uCoor = pxdHit->getU();
          double vCoor = pxdHit->getV();
          VxdID sensor = pxdHit->getSensorID();

          m_h1_nVXDhitsPR->Fill(sensor.getLayerNumber());

          m_h1_nVXDhitsWeighted->Fill(sensor.getLayerNumber() , weight);

          m_h2_TrackPointFitWeightVXD->Fill(sensor.getLayerNumber(), weight);
          const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(sensor);
          globalHit = aSensorInfo.pointToGlobal(TVector3(uCoor, vCoor, 0), true);

        } else if (svdHit2D) {

          if (kalmanInfo)
            weight = weights.at(mea);

          detId = 1;
          double uCoor = svdHit2D->getU();
          double vCoor = svdHit2D->getV();
          VxdID sensor = svdHit2D->getSensorID();

          m_h1_nVXDhitsPR->Fill(sensor.getLayerNumber());

          m_h1_nVXDhitsWeighted->Fill(sensor.getLayerNumber() , weight);

          m_h2_TrackPointFitWeightVXD->Fill(sensor.getLayerNumber(), weight);

          const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(sensor);
          globalHit = aSensorInfo.pointToGlobal(TVector3(uCoor, vCoor, 0), true);

        } else if (svdHit) {

          if (kalmanInfo)
            weight = weights.at(mea);

          detId = 2;
          double uCoor = 0;
          double vCoor = 0;
          if (svdHit->isU())
            uCoor = svdHit->getPosition();
          else
            vCoor =  svdHit->getPosition();

          VxdID sensor = svdHit->getSensorID();
          m_h1_nVXDhitsPR->Fill(sensor.getLayerNumber());

          m_h1_nVXDhitsWeighted->Fill(sensor.getLayerNumber() , weight);

          m_h2_TrackPointFitWeightVXD->Fill(sensor.getLayerNumber(), weight);
          const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(sensor);
          globalHit = aSensorInfo.pointToGlobal(TVector3(uCoor, vCoor, 0), true);
        } else if (cdcHit) {

          if (kalmanInfo)
            weight = weights.at(mea);

          WireID wire = cdcHit->getWireID();
          if (! hasCDChit[wire.getICLayer()]) { //needed to validate the HitPatternCDC filling
            m_h1_nCDChitsPR->Fill(wire.getICLayer());

            m_h1_nCDChitsWeighted->Fill(wire.getICLayer() , weight);
            //    hasCDChit[wire.getICLayer()] = true;  //to validate the HitPatternCDC filling: uncomment this
          }
          m_h2_TrackPointFitWeightCDC->Fill(wire.getICLayer(), weight);
          detId = 3;
        }


        m_h1_nHitDetID ->Fill(detId);

        m_h2_VXDhitsPR_xy->Fill(globalHit.X(), globalHit.Y());

        m_h2_VXDhitsPR_rz->Fill(globalHit.Z(), globalHit.Perp());

      }

    }
  }

  if ((fitResult != NULL) && (fitResult->getParticleType() == Const::ChargedStable(m_ParticleHypothesis))) {
    if (hasPXDhit) {
      m_h2_d0errVSpt_wpxd->Fill(pt, d0_err);
      m_h2_z0errVSpt_wpxd->Fill(pt, z0_err);
    } else {
      m_h2_d0errVSpt_wopxd->Fill(pt, d0_err);
      m_h2_z0errVSpt_wopxd->Fill(pt, z0_err);
    }
  }

}

void TrackingPerformanceEvaluationModule::addMoreInefficiencyPlots(TList* histoList)
{

  //normalized to MCParticles
  TH1F* h_ineff_pt = createHistogramsRatio("hineffpt", "inefficiency VS pt, normalized to MCParticles", m_h3_TracksPerMCParticle,
                                           m_h3_MCParticle, false, 0);
  histoList->Add(h_ineff_pt);

  TH1F* h_ineff_theta = createHistogramsRatio("hinefftheta", "inefficiency VS #theta, normalized to MCParticles",
                                              m_h3_TracksPerMCParticle, m_h3_MCParticle, false, 1);
  histoList->Add(h_ineff_theta);

  TH1F* h_ineff_phi = createHistogramsRatio("hineffphi", "inefficiency VS #phi, normalized to MCParticles", m_h3_TracksPerMCParticle,
                                            m_h3_MCParticle, false, 2);
  histoList->Add(h_ineff_phi);

  //normalized to MCRecoTracks
  TH1F* h_ineffMCRT_pt = createHistogramsRatio("hineffMCRTpt", "inefficiency VS pt, normalized to MCRecoTrack",
                                               m_h3_TracksPerMCRecoTrack, m_h3_MCRecoTrack, false, 0);
  histoList->Add(h_ineffMCRT_pt);

  TH1F* h_ineffMCRT_theta = createHistogramsRatio("hineffMCRTtheta", "inefficiency VS #theta, normalized to MCRecoTrack",
                                                  m_h3_TracksPerMCRecoTrack, m_h3_MCRecoTrack, false, 1);
  histoList->Add(h_ineffMCRT_theta);

  TH1F* h_ineffMCRT_phi = createHistogramsRatio("hineffMCRTphi", "inefficiency VS #phi, normalized to MCRecoTrack",
                                                m_h3_TracksPerMCRecoTrack, m_h3_MCRecoTrack, false, 2);
  histoList->Add(h_ineffMCRT_phi);

}

void TrackingPerformanceEvaluationModule::addMoreEfficiencyPlots(TList* histoList)
{


  TH1F* h_MCPwPXDhits_pt = createHistogramsRatio("hMCPwPXDhits", "fraction of MCParticles with PXD hits VS pt",
                                                 m_h3_MCParticleswPXDHits,
                                                 m_h3_MCParticle, true, 0);
  histoList->Add(h_MCPwPXDhits_pt);

  TH1F* h_RTwPXDhitsMCPwPXDHits_pt = createHistogramsRatio("hRecoTrkswPXDhitsMCPwPXDHits",
                                                           "fraction of MCParticles with PXD Hits with RecoTracks with PXD hits VS pt",
                                                           m_h3_RecoTrackswPXDHitsPerMCParticlewPXDHits,
                                                           m_h3_MCParticleswPXDHits, true, 0);
  histoList->Add(h_RTwPXDhitsMCPwPXDHits_pt);

  TH1F* h_wPXDhits_pt = createHistogramsRatio("hTrkswPXDhits", "fraction of tracks with PXD hits VS pt",
                                              m_h3_TrackswPXDHitsPerMCParticle,
                                              m_h3_TracksPerMCParticle, true, 0);
  histoList->Add(h_wPXDhits_pt);

  //normalized to MCParticles
  TH1F* h_eff_pt = createHistogramsRatio("heffpt", "efficiency VS pt, normalized to MCParticles",  m_h3_TracksPerMCParticle,
                                         m_h3_MCParticle, true, 0);
  histoList->Add(h_eff_pt);
  //  B2INFO(" efficiency in pt, NUM =  "<<m_nFittedTracks<<", DEN = "<<m_nMCParticles<<", eff integrata = "<<(double)m_nFittedTracks/m_nMCParticles);

  TH1F* h_eff_theta = createHistogramsRatio("hefftheta", "efficiency VS #theta, normalized to MCParticles", m_h3_TracksPerMCParticle,
                                            m_h3_MCParticle, true, 1);
  histoList->Add(h_eff_theta);

  TH1F* h_eff_phi = createHistogramsRatio("heffphi", "efficiency VS #phi, normalized to MCParticles", m_h3_TracksPerMCParticle,
                                          m_h3_MCParticle, true, 2);
  histoList->Add(h_eff_phi);

  //normalized to MCRecoTracks
  TH1F* h_effMCRT_pt = createHistogramsRatio("heffMCRTpt", "efficiency VS pt, normalized to MCRecoTrack", m_h3_TracksPerMCRecoTrack,
                                             m_h3_MCRecoTrack, true, 0);
  histoList->Add(h_effMCRT_pt);

  TH1F* h_effMCRT_theta = createHistogramsRatio("heffMCRTtheta", "efficiency VS #theta, normalized to MCRecoTrack",
                                                m_h3_TracksPerMCRecoTrack, m_h3_MCRecoTrack, true, 1);
  histoList->Add(h_effMCRT_theta);

  TH1F* h_effMCRT_phi = createHistogramsRatio("heffMCRTphi", "efficiency VS #phi, normalized to MCRecoTrack",
                                              m_h3_TracksPerMCRecoTrack, m_h3_MCRecoTrack, true, 2);
  histoList->Add(h_effMCRT_phi);

  // plus

  //normalized to MCParticles
  TH1F* h_eff_pt_plus = createHistogramsRatio("heffpt_plus", "efficiency VS pt, normalized to positive MCParticles",
                                              m_h3_TracksPerMCParticle_plus, m_h3_MCParticle_plus, true, 0);
  histoList->Add(h_eff_pt_plus);
  //  B2INFO(" efficiency in pt, NUM =  "<<m_nFittedTracks<<", DEN = "<<m_nMCParticles<<", eff integrata = "<<(double)m_nFittedTracks/m_nMCParticles);

  TH1F* h_eff_theta_plus = createHistogramsRatio("hefftheta_plus", "efficiency VS #theta, normalized to positive MCParticles",
                                                 m_h3_TracksPerMCParticle_plus, m_h3_MCParticle_plus, true, 1);
  histoList->Add(h_eff_theta_plus);

  TH1F* h_eff_phi_plus = createHistogramsRatio("heffphi_plus", "efficiency VS #phi, normalized to positive  MCParticles",
                                               m_h3_TracksPerMCParticle_plus, m_h3_MCParticle_plus, true, 2);
  histoList->Add(h_eff_phi_plus);

  //normalized to MCRecoTracks
  TH1F* h_effMCRT_pt_plus = createHistogramsRatio("heffMCRTpt_plus", "efficiency VS pt, normalized to positive MCRecoTrack",
                                                  m_h3_TracksPerMCRecoTrack_plus, m_h3_MCRecoTrack_plus, true, 0);
  histoList->Add(h_effMCRT_pt_plus);

  TH1F* h_effMCRT_theta_plus = createHistogramsRatio("heffMCRTtheta_plus", "efficiency VS #theta, normalized to positive MCRecoTrack",
                                                     m_h3_TracksPerMCRecoTrack_plus, m_h3_MCRecoTrack_plus, true, 1);
  histoList->Add(h_effMCRT_theta_plus);

  TH1F* h_effMCRT_phi_plus = createHistogramsRatio("heffMCRTphi_plus", "efficiency VS #phi, normalized to positive MCRecoTrack",
                                                   m_h3_TracksPerMCRecoTrack_plus, m_h3_MCRecoTrack_plus, true, 2);
  histoList->Add(h_effMCRT_phi_plus);

  // minus

  //normalized to MCParticles
  TH1F* h_eff_pt_minus = createHistogramsRatio("heffpt_minus", "efficiency VS pt, normalized to positive MCParticles",
                                               m_h3_TracksPerMCParticle_minus, m_h3_MCParticle_minus, true, 0);
  histoList->Add(h_eff_pt_minus);
  //  B2INFO(" efficiency in pt, NUM =  "<<m_nFittedTracks<<", DEN = "<<m_nMCParticles<<", eff integrata = "<<(double)m_nFittedTracks/m_nMCParticles);

  TH1F* h_eff_theta_minus = createHistogramsRatio("hefftheta_minus", "efficiency VS #theta, normalized to positive MCParticles",
                                                  m_h3_TracksPerMCParticle_minus, m_h3_MCParticle_minus, true, 1);
  histoList->Add(h_eff_theta_minus);

  TH1F* h_eff_phi_minus = createHistogramsRatio("heffphi_minus", "efficiency VS #phi, normalized to positive  MCParticles",
                                                m_h3_TracksPerMCParticle_minus, m_h3_MCParticle_minus, true, 2);
  histoList->Add(h_eff_phi_minus);

  //normalized to MCRecoTracks
  TH1F* h_effMCRT_pt_minus = createHistogramsRatio("heffMCRTpt_minus", "efficiency VS pt, normalized to positive MCRecoTrack",
                                                   m_h3_TracksPerMCRecoTrack_minus, m_h3_MCRecoTrack_minus, true, 0);
  histoList->Add(h_effMCRT_pt_minus);

  TH1F* h_effMCRT_theta_minus = createHistogramsRatio("heffMCRTtheta_minus",
                                                      "efficiency VS #theta, normalized to positive MCRecoTrack", m_h3_TracksPerMCRecoTrack_minus, m_h3_MCRecoTrack_minus, true, 1);
  histoList->Add(h_effMCRT_theta_minus);

  TH1F* h_effMCRT_phi_minus = createHistogramsRatio("heffMCRTphi_minus", "efficiency VS #phi, normalized to positive MCRecoTrack",
                                                    m_h3_TracksPerMCRecoTrack_minus, m_h3_MCRecoTrack_minus, true, 2);
  histoList->Add(h_effMCRT_phi_minus);

  //pattern recognition efficiency
  TH1F* h_effPR = createHistogramsRatio("heffPR", "PR efficiency VS VXD Layer, normalized to MCRecoTrack",
                                        m_h1_HitsRecoTrackPerMCRecoTrack, m_h1_HitsMCRecoTrack, true, 0);
  histoList->Add(h_effPR);

  //tracks used in the fit
  TH1F* h_effVXDHitFit = createHistogramsRatio("heffVXDHitFit",
                                               "weighted hits used in the fit VS VXD Layer, normalized to hits form PR", m_h1_nVXDhitsWeighted, m_h1_nVXDhitsPR, true, 0);
  histoList->Add(h_effVXDHitFit);

  TH1F* h_effCDCHitFit = createHistogramsRatio("heffCDCHitFit",
                                               "weighted hits used in the fit VS CDC Layer, normalized to hits form PR", m_h1_nCDChitsWeighted, m_h1_nCDChitsPR, true, 0);
  histoList->Add(h_effCDCHitFit);

}




bool TrackingPerformanceEvaluationModule::isTraceable(const MCParticle& the_mcParticle)
{

  bool isChargedStable = Const::chargedStableSet.find(abs(the_mcParticle.getPDG())) != Const::invalidParticle;

  bool isPrimary = the_mcParticle.hasStatus(MCParticle::c_PrimaryParticle);

  return (isPrimary && isChargedStable);

}
