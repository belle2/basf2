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

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationVector.h>

#include <geometry/bfieldmap/BFieldMap.h>

#include <vxd/geometry/GeoCache.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>

#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>


#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <genfit/TrackCandHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <cdc/dataobjects/CDCHit.h>

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
  , m_rootFilePtr(NULL)
{

  setDescription("This module evaluates the tracking package performance");

  addParam("outputFileName", m_rootFileName, "Name of output root file.",
           std::string("TrackingPerformanceEvaluation_output.root"));
  addParam("TracksName", m_TracksName, "Name of genfit Track collection.", std::string(""));
  addParam("TrackCandsName", m_TrackCandsName, "Name of genfit TrackCand collection.", std::string(""));
  addParam("MCTrackCandsName", m_MCTrackCandsName, "Name of MC TrackCand collection.", std::string("MCGFTrackCands"));
  addParam("MCParticlesName", m_MCParticlesName, "Name of MC Particle collection.", std::string(""));

}

TrackingPerformanceEvaluationModule::~TrackingPerformanceEvaluationModule()
{

}

void TrackingPerformanceEvaluationModule::initialize()
{
  // MCParticles, MCTrackCands, TrackCands, Tracks needed for this module
  StoreArray<MCParticle>::required(m_MCParticlesName);
  StoreArray<genfit::Track>::required(m_TracksName);
  StoreArray<genfit::TrackCand>::required(m_TrackCandsName);
  StoreArray<genfit::TrackCand>::required(m_MCTrackCandsName);

  //create list of histograms to be saved in the rootfile
  m_histoList = new TList;


  //set the ROOT File
  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  //now create histograms

  //multiplicity histograms
  m_multiplicityTracks = createHistogram1D("h1nTrk", "number of tracks per MC Particle", 8, -0.5, 7.5, "# tracks", m_histoList);
  m_multiplicityFittedTracks = createHistogram1D("h1nFitTrk", "number of fitted tracks per MC Particle", 5, -0.5, 4.5, "# fitted tracks", m_histoList);
  m_multiplicityFittedTracksPerMCTC = createHistogram1D("h1nFitTrkMCTC", "number of fitted tracks per MCTrackCand", 5, -0.5, 4.5, "# fitted tracks", m_histoList);
  m_multiplicityMCParticles = createHistogram1D("h1nMCPrtcl", "number of MCParticles per fitted tracks", 5, -0.5, 4.5, "# MCParticles", m_histoList);
  m_multiplicityTrackCands = createHistogram1D("h1nTrackCand", "number of TrackCand per MCTrackCand", 5, -0.5, 4.5, "# TrackCand", m_histoList);
  m_multiplicityMCTrackCands = createHistogram1D("h1nMCTrackCand", "number of MCTrackCand per TrackCand", 5, -0.5, 4.5, "# MCTrackCand", m_histoList);

  //track parameters errors
  m_h1_d0_err = createHistogram1D("h1d0err", "d0 error", 100, 0, 0.1, "#sigma_{d0} (cm)", m_histoList);
  m_h1_phi_err = createHistogram1D("h1phierr", "#phi error", 100, 0, 0.02, "#sigma_{#phi} (rad)", m_histoList);
  m_h1_omega_err = createHistogram1D("h1omegaerr", "#omega error", 100, 0, 0.002, "#sigma_{#omega} (cm^{-1})", m_histoList);
  m_h1_z0_err = createHistogram1D("h1z0err", "z0 error", 100, 0, 0.1, "#sigma_{z0} (cm)", m_histoList);
  m_h1_cotTheta_err = createHistogram1D("h1cotThetaerr", "cot#theta error", 100, 0, 0.03, "#sigma_{cot#theta}", m_histoList);

  //track parameters residuals
  m_h1_d0_res = createHistogram1D("h1d0res", "d0 residuals", 100, -0.5, 0.5, "d0 resid (cm)", m_histoList);
  m_h1_phi_res = createHistogram1D("h1phires", "#phi residuals", 100, -10, 10, "#phi resid (rad)", m_histoList);
  m_h1_omega_res = createHistogram1D("h1omegares", "#omega residuals", 100, -0.01, 0.01, "#omega resid (cm^{-1})", m_histoList);
  m_h1_z0_res = createHistogram1D("h1z0res", "z0 residuals", 100, -1, 1, "z0 resid (cm)", m_histoList);
  m_h1_cotTheta_res = createHistogram1D("h1cotThetares", "cot#theta residuals", 100, -10, 10, "cot#theta resid", m_histoList);


  //track parameters pulls
  m_h1_d0_pll = createHistogram1D("h1d0pll", "d0 pulls", 100, -5, 5, "d0 pull", m_histoList);
  m_h1_phi_pll = createHistogram1D("h1phipll", "#phi pulls", 100, -5, 5, "#phi pull", m_histoList);
  m_h1_omega_pll = createHistogram1D("h1omegapll", "#omega pulls", 100, -5, 5, "#omega pull", m_histoList);
  m_h1_z0_pll = createHistogram1D("h1z0pll", "z0 pulls", 100,  -5, 5, "z0 pull", m_histoList);
  m_h1_cotTheta_pll = createHistogram1D("h1cotThetapll", "cot#theta pulls", 100,  -5, 5, "cot#theta pull", m_histoList);


  //first hit position using track parameters errors
  m_h2_d0errphi0err_xy = createHistogram2D("h2d0errphierrXY", "#sigma_{d0}/#sigma_{#phi} projected on x,y",
                                           2000, -10, 10, "x (cm)",
                                           2000, -10, 10, "y (cm)",
                                           m_histoList);

  m_h2_d0errphi0err_rz = createHistogram2D("h2d0errphierrRZ", "#sigma_{d0}/#sigma_{#phi} projected on z and r_{t}=#sqrt{x^{2}+y^{2}}",
                                           2000, -30, 40, "z (cm)",
                                           2000, 0, 15, "r_{t} (cm)",
                                           m_histoList);

  m_h2_z0errcotThetaerr_xy = (TH2F*)duplicateHistogram("h2z0errcotThetaerrXY", "#sigma_{z0}/#sigma_{cot#theta} projected on x,y",
                                                       m_h2_d0errphi0err_xy,
                                                       m_histoList);

  //hits used in the fit
  m_h1_nHitDetID = createHistogram1D("h1nHitDetID", "detector ID per hit", 4, -0.5, 3.5, "0=PXD, 1=SVD2D, 2=SVD,3=CDC", m_histoList);
  m_h1_nVXDhitsUsed = createHistogram1D("h1nVXDHitsUsed", "number of VXD hits per Layer", 6, 0.5, 6.5, "VXD Layer", m_histoList);
  m_h2_VXDhitsUsed_xy = createHistogram2D("h2hitsUsedXY", "hits used in the fit to the tracks, transverse plane",
                                          2000, -15, 15, "x (cm)",
                                          2000, -15, 15, "y (cm)",
                                          m_histoList);

  m_h2_VXDhitsUsed_rz = createHistogram2D("h2hitsUsedRZ", "hits used in the fit to the tracks, r_{t} z",
                                          2000, -30, 40, "z (cm)",
                                          2000, 0, 15, "r_{t} (cm)",
                                          m_histoList);

  m_h1_pValue = createHistogram1D("h1pValue", "pValue of the fit", 100, 0, 1, "pValue", m_histoList);


  m_h2_d0errVSpt = createHistogram2D("h2d0errVSpt", "#sigma_{d0} VS p_{t}",
                                     100, 0, 2, "p_{t} (GeV/c)",
                                     100, 0, 0.1, "#sigma_{d0} (cm)",
                                     m_histoList);

  m_h2_d0errVSpt_wpxd = (TH2F*)duplicateHistogram("h2d0errVSpt_wPXD", "#sigma_{d0} VS p_{t}, with PXD hits", m_h2_d0errVSpt, m_histoList);

  m_h2_d0errVSpt_wopxd = (TH2F*)duplicateHistogram("h2d0errVSpt_woPXD", "#sigma_{d0} VS p_{t}, no PXD hits", m_h2_d0errVSpt, m_histoList);

  m_h2_d0errMSVSpt = createHistogram2D("h2d0errMSVSpt", "#sigma_{d0} * #betapsin^{3/2}#theta VS p_{t}",
                                       50, 0, 2.5, "p_{t} (GeV/c)",
                                       500, 0, 1, "cm",
                                       m_histoList);


  //histograms to produce efficiency plots
  Double_t bins_pt[9 + 1] = {0, 0.05, 0.1, 0.15, 0.2, 0.3, 0.5, 1, 2, 3.5}; //GeV/c
  Double_t bins_theta[10 + 1] = {0, 0.25, 0.5, 0.75, 0.75 + 0.32, 0.75 + 2 * 0.32, 0.75 + 3 * 0.32, 0.75 + 4 * 0.32, 0.75 + 5 * 0.32, 2.65, TMath::Pi()};
  Double_t bins_phi[14 + 1];
  Double_t width_phi = 2 * TMath::Pi() / 14;
  for (int bin = 0; bin < 14 + 1; bin++)
    bins_phi[bin] = - TMath::Pi() + bin * width_phi;


  m_h3_MCParticle = createHistogram3D("h3MCParticle", "entry per MCParticle",
                                      9, bins_pt, "p_{t} (GeV/c)",
                                      10, bins_theta, "#theta",
                                      14, bins_phi, "#phi" /*, m_histoList*/);

  m_h3_TracksPerMCParticle = (TH3F*)duplicateHistogram("h3TracksPerMCParticle",
                                                       "entry per Track connected to a MCParticle",
                                                       m_h3_MCParticle /*, m_histoList*/);

  m_h3_MCTrackCand = (TH3F*)duplicateHistogram("h3MCTrackCand",
                                               "entry per MCTrackCand connected to the MCParticle",
                                               m_h3_MCParticle /*, m_histoList*/);

  m_h3_TracksPerMCTrackCand = (TH3F*)duplicateHistogram("h3TracksPerMCTrackCand",
                                                        "entry per Track connected to an MCTrackCand",
                                                        m_h3_MCParticle /*, m_histoList*/);

  //plus
  m_h3_MCParticle_plus = (TH3F*)duplicateHistogram("h3MCParticlee_plus", "entry per positive MCParticle",
                                                   m_h3_MCParticle /*, m_histoList*/);

  m_h3_TracksPerMCParticle_plus = (TH3F*)duplicateHistogram("h3TracksPerMCParticle_plus",
                                                            "entry per Track connected to a positive MCParticle",
                                                            m_h3_MCParticle /*, m_histoList*/);

  m_h3_MCTrackCand_plus = (TH3F*)duplicateHistogram("h3MCTrackCand_plus",
                                                    "entry per MCTrackCand connected to the positive MCParticle",
                                                    m_h3_MCParticle /*, m_histoList*/);

  m_h3_TracksPerMCTrackCand_plus = (TH3F*)duplicateHistogram("h3TracksPerMCTrackCand_plus",
                                                             "entry per Track connected to a positive MCTrackCand",
                                                             m_h3_MCParticle /*, m_histoList*/);


  //minus
  m_h3_MCParticle_minus = (TH3F*)duplicateHistogram("h3MCParticlee_minus", "entry per negative MCParticle",
                                                    m_h3_MCParticle /*, m_histoList*/);

  m_h3_TracksPerMCParticle_minus = (TH3F*)duplicateHistogram("h3TracksPerMCParticle_minus",
                                                             "entry per Track connected to a negative MCParticle",
                                                             m_h3_MCParticle /*, m_histoList*/);

  m_h3_MCTrackCand_minus = (TH3F*)duplicateHistogram("h3MCTrackCand_minus",
                                                     "entry per MCTrackCand connected to the negative MCParticle",
                                                     m_h3_MCParticle /*, m_histoList*/);

  m_h3_TracksPerMCTrackCand_minus = (TH3F*)duplicateHistogram("h3TracksPerMCTrackCand_minus",
                                                              "entry per Track connected to a negative MCTrackCand",
                                                              m_h3_MCParticle /*, m_histoList*/);

  //histograms to produce efficiency plots
  m_h1_HitsTrackCandPerMCTrackCand = createHistogram1D("h1hitsTCperMCTC", "TrackCand per MCTrackCand Hit in VXD layers", 6, 0.5, 6.5, "# VXD layer" /*, m_histoList*/);

  m_h1_HitsMCTrackCand = (TH1F*) duplicateHistogram("h1hitsMCTC", " MCTrackCand Hit in VXD layers", m_h1_HitsTrackCandPerMCTrackCand /*,  m_histoList*/);


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

  BFieldMap& bfieldMap = BFieldMap::Instance();
  TVector3 magField = bfieldMap.getBField(TVector3(0, 0, 0));

  bool hasTrack = false;
  B2DEBUG(99, "+++++ 1. loop on MCParticles");
  BOOST_FOREACH(MCParticle & mcParticle, mcParticles) {

    if (! isTraceable(mcParticle))
      continue;

    int pdgCode = mcParticle.getPDG();
    B2DEBUG(99, "MCParticle has PDG code " << pdgCode);

    int nFittedTracksMCTC = 0;
    int nFittedTracks = 0;
    int nTracks = 0;

    MCParticleInfo mcParticleInfo(mcParticle, magField);

    hasTrack = false;

    m_h3_MCParticle->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());

    if (mcParticleInfo.getCharge() > 0)
      m_h3_MCParticle_plus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
    else if (mcParticleInfo.getCharge() < 0)
      m_h3_MCParticle_minus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
    else
      continue;

    //1. retrieve all the Tracks related to the MCParticle

    //1.0 check if there is an MCTrackCand
    RelationVector<genfit::TrackCand> MCTrackCands_fromMCParticle = DataStore::getRelationsToObj<genfit::TrackCand>(&mcParticle, m_MCTrackCandsName);
    if (MCTrackCands_fromMCParticle.size() > 0) {
      m_h3_MCTrackCand->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());

      if (mcParticleInfo.getCharge() > 0)
        m_h3_MCTrackCand_plus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
      else if (mcParticleInfo.getCharge() < 0)
        m_h3_MCTrackCand_minus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
      else
        continue;
    }

    //1.a retrieve all TrackCands related to the MCParticle
    RelationVector<genfit::TrackCand> TrackCands_fromMCParticle = DataStore::getRelationsToObj<genfit::TrackCand>(&mcParticle);

    //    B2DEBUG(99,TrackCands_fromMCParticle.size()<<" TrackCands related to this MCParticle");

    for (int tc = 0; tc < (int)TrackCands_fromMCParticle.size(); tc++) {

      //1.b retrieve all Tracks related to the TrackCand
      RelationVector<genfit::Track> Tracks_fromTrackCand = DataStore::getRelationsFromObj<genfit::Track>(TrackCands_fromMCParticle[tc]);

      //      B2DEBUG(99,"   "<<Tracks_fromTrackCand.size()<<" Track related to this TrackCand");

      for (int trk = 0; trk < (int)Tracks_fromTrackCand.size(); trk++) {

        nTracks++;

        const TrackFitResult* fitResult = DataStore::getRelatedFromObj<TrackFitResult>(Tracks_fromTrackCand[trk]);

        if (fitResult != NULL) { // valid TrackFitResult found

          if (!hasTrack) {

            hasTrack = true;

            nFittedTracks++;

            m_h3_TracksPerMCParticle->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
            if (mcParticleInfo.getCharge() > 0)
              m_h3_TracksPerMCParticle_plus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
            else if (mcParticleInfo.getCharge() < 0)
              m_h3_TracksPerMCParticle_minus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
            else
              continue;

            if (MCTrackCands_fromMCParticle.size() > 0) {
              nFittedTracksMCTC++;
              m_h3_TracksPerMCTrackCand->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
              if (mcParticleInfo.getCharge() > 0)
                m_h3_TracksPerMCTrackCand_plus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
              else if (mcParticleInfo.getCharge() < 0)
                m_h3_TracksPerMCTrackCand_minus->Fill(mcParticleInfo.getPt(), mcParticleInfo.getPtheta(), mcParticleInfo.getPphi());
              else
                continue;
            }


          }


          fillTrackParams1DHistograms(fitResult, mcParticleInfo);

        }
      }
    }

    m_multiplicityTracks->Fill(nTracks);
    m_multiplicityFittedTracks->Fill(nFittedTracks);
    if (MCTrackCands_fromMCParticle.size() > 0)
      m_multiplicityFittedTracksPerMCTC->Fill(nFittedTracksMCTC);

  }


  B2DEBUG(99, "+++++ 2. loop on Tracks");

  //2. retrieve all the MCParticles related to the Tracks
  StoreArray<genfit::Track> tracks(m_TracksName);

  BOOST_FOREACH(genfit::Track & track, tracks) {

    int nMCParticles = 0;

    //check if the track has been fitted
    const TrackFitResult* fitResult = DataStore::getRelatedFromObj<TrackFitResult>(&track);
    if (fitResult == NULL)
      continue;


    m_h1_pValue->Fill(fitResult->getPValue());

    TVector3 momentum = fitResult->getMomentum();
    m_h3_Tracks->Fill(momentum.Pt(), momentum.Theta(), momentum.Phi());

    fillTrackErrParams2DHistograms(fitResult);

    fillHitsUsedInTrackFitHistograms(track);

    //2.a retrieve all TrackCands related to the Track
    RelationVector<genfit::TrackCand> TrackCands_fromTrack = DataStore::getRelationsToObj<genfit::TrackCand>(&track);

    for (int tc = 0; tc < (int)TrackCands_fromTrack.size(); tc++) {

      //2.b retrieve all MCParticles related to the TrackCand
      RelationVector<MCParticle> MCParticles_fromTrackCand = DataStore::getRelationsFromObj<MCParticle>(TrackCands_fromTrack[tc]);

      for (int mcp = 0; mcp < (int)MCParticles_fromTrackCand.size(); mcp++)
        if (isTraceable(*MCParticles_fromTrackCand[mcp])) {
          nMCParticles ++;
          m_h3_MCParticlesPerTrack->Fill(momentum.Pt(), momentum.Theta(), momentum.Phi());
        }
    }

    m_multiplicityMCParticles->Fill(nMCParticles);

  }

  B2DEBUG(99, "+++++ 3. loop on MCTrackCands");

  //3. retrieve all MCTrackCands
  StoreArray<genfit::TrackCand> mcTrackCands(m_MCTrackCandsName);
  StoreArray<PXDCluster> pxdClusters;
  StoreArray<SVDCluster> svdClusters;
  StoreArray<CDCHit> cdcHit;

  bool hasTrackCand = false;

  BOOST_FOREACH(genfit::TrackCand & mcTrackCand, mcTrackCands) {

    int nTrackCand = 0;
    hasTrackCand = false;

    //3.a retrieve the trackcand
    RelationVector<genfit::TrackCand> TrackCands_fromMCTrackCand = DataStore::getRelationsFromObj<genfit::TrackCand>(&mcTrackCand);
    B2DEBUG(99, "~ " << TrackCands_fromMCTrackCand.size() << " TrackCands related to this MCTrackCand");

    //3.a retrieve the MCParticle
    RelationVector<MCParticle> MCParticles_fromMCTrackCand = DataStore::getRelationsFromObj<MCParticle>(&mcTrackCand);

    B2DEBUG(99, "~~~ " << MCParticles_fromMCTrackCand.size() << " MCParticles related to this MCTrackCand");
    for (int mcp = 0; mcp < (int)MCParticles_fromMCTrackCand.size(); mcp++) {

      //3.b retrieve all TrackCands related to the MCTrackCand
      RelationVector<genfit::TrackCand> TrackCands_fromMCParticle = DataStore::getRelationsToObj<genfit::TrackCand>(MCParticles_fromMCTrackCand[mcp]);

      B2DEBUG(99, "~~~~~ " << TrackCands_fromMCParticle.size() << " TrackCands related to this MCParticle");
      for (int tc = 0; tc < (int)TrackCands_fromMCParticle.size(); tc++)

        if (!hasTrackCand) {

          hasTrackCand = true;
          nTrackCand++;

          genfit::TrackCandHit* thehitMCTC = 0;
          for (int hitMCTC = 0; hitMCTC < (int)mcTrackCand.getNHits(); hitMCTC++) {

            thehitMCTC = mcTrackCand.getHit(hitMCTC);
            if (!thehitMCTC)
              continue;

            int hitId = thehitMCTC->getHitId();
            int detId = thehitMCTC->getDetId();
            if (detId == 1)
              m_h1_HitsMCTrackCand->Fill(pxdClusters[hitId]->getSensorID().getLayerNumber());
            if (detId == 2)
              m_h1_HitsMCTrackCand->Fill(svdClusters[hitId]->getSensorID().getLayerNumber());
            //      if(thehitMCTC->getDetId() == 3)
            //        m_h1_HitsMCTrackCand->Fill( cdcHit[hitId]->getLayer() );

            genfit::TrackCandHit* thehitTC = 0;
            for (int hitTC = 0; hitTC < (int)TrackCands_fromMCParticle[tc]->getNHits(); hitTC++) {

              thehitTC = TrackCands_fromMCParticle[tc]->getHit(hitTC);
              if (!thehitTC)
                continue;

              if ((*thehitTC) == (*thehitMCTC)) {
                if (detId == Const::PXD)
                  m_h1_HitsTrackCandPerMCTrackCand->Fill(pxdClusters[hitId]->getSensorID().getLayerNumber());
                if (detId == Const::SVD)
                  m_h1_HitsTrackCandPerMCTrackCand->Fill(svdClusters[hitId]->getSensorID().getLayerNumber());
                continue;
              }
            }
          }

        }
    }
    m_multiplicityTrackCands->Fill(nTrackCand);

  }

  B2DEBUG(99, "+++++ 4. loop on TrackCands");

  //4. retrieve all TrackCands
  StoreArray<genfit::TrackCand> TrackCands;

  BOOST_FOREACH(genfit::TrackCand & trackCand, TrackCands) {

    int nMCTrackCand = 0;

    //4.a retrieve the MCParticle
    RelationVector<MCParticle> MCParticles_fromTrackCand = DataStore::getRelationsFromObj<MCParticle>(&trackCand);

    B2DEBUG(99, "~~~ " << MCParticles_fromTrackCand.size() << " MCParticles related to this TrackCand");
    for (int mcp = 0; mcp < (int)MCParticles_fromTrackCand.size(); mcp++) {

      //4.b retrieve all MCTrackCands related to the TrackCand
      RelationVector<genfit::TrackCand> mcTrackCands_fromMCParticle = DataStore::getRelationsToObj<genfit::TrackCand>(MCParticles_fromTrackCand[mcp], m_MCTrackCandsName);

      B2DEBUG(99, "~~~~~ " << mcTrackCands_fromMCParticle.size() << " MCTrackCands related to this MCParticle");
      for (int mctc = 0; mctc < (int)mcTrackCands_fromMCParticle.size(); mctc++) {
        nMCTrackCand++;

      }
    }

    m_multiplicityMCTrackCands->Fill(nMCTrackCand);

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

  double nFittedTracksMCTC = 0;
  for (int bin = 1; bin < m_multiplicityFittedTracksPerMCTC->GetNbinsX(); bin ++)
    nFittedTracksMCTC += m_multiplicityFittedTracksPerMCTC->GetBinContent(bin + 1);
  double efficiencyMCTC = nFittedTracksMCTC / m_multiplicityFittedTracksPerMCTC->GetEntries();
  double efficiencyMCTCErr =  sqrt(efficiencyMCTC * (1 - efficiencyMCTC)) / sqrt(m_multiplicityFittedTracksPerMCTC->GetEntries());

  double nTrackCand = 0;
  for (int bin = 1; bin < m_multiplicityTrackCands->GetNbinsX(); bin ++)
    nTrackCand += m_multiplicityTrackCands->GetBinContent(bin + 1);
  double efficiencyPR = nTrackCand / m_multiplicityTrackCands->GetEntries();
  double efficiencyPRErr =  sqrt(efficiencyPR * (1 - efficiencyPR)) / sqrt(m_multiplicityTrackCands->GetEntries());

  double nMCTrackCand = 0;
  for (int bin = 1; bin < m_multiplicityMCTrackCands->GetNbinsX(); bin ++)
    nMCTrackCand += m_multiplicityMCTrackCands->GetBinContent(bin + 1);
  double purityPR = nMCTrackCand / m_multiplicityMCTrackCands->GetEntries();
  double purityPRErr =  sqrt(purityPR * (1 - purityPR)) / sqrt(m_multiplicityMCTrackCands->GetEntries());

  double nMCParticles = 0;
  for (int bin = 1; bin < m_multiplicityMCParticles->GetNbinsX(); bin ++)
    nMCParticles += m_multiplicityMCParticles->GetBinContent(bin + 1);
  double purity = nMCParticles / m_multiplicityMCParticles->GetEntries();
  double purityErr =  sqrt(purity * (1 - purity)) / sqrt(m_multiplicityMCParticles->GetEntries());

  B2INFO("");
  B2INFO("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  B2INFO("~ Tracking Performance Evaluation ~ SHORT SUMMARY ~");
  B2INFO("");
  B2INFO(" + overall:");
  B2INFO("   efficiency = (" << efficiency * 100 << " +/- " << efficiencyErr * 100 << ")% ");
  B2INFO("       purity = " << purity * 100 << " +/- " << purityErr * 100 << ")% ");
  B2INFO("");
  B2INFO(" + factorizing geometrical acceptance:");
  B2INFO("   efficiency = " << efficiencyMCTC * 100 << " +/- " << efficiencyMCTCErr * 100 << ")% ");
  B2INFO("");
  B2INFO(" + pattern recognition:");
  B2INFO("   efficiency = " << efficiencyPR * 100 << " +/- " << efficiencyPRErr * 100 << ")% ");
  B2INFO("       purity = " << purityPR * 100 << " +/- " << purityPRErr * 100 << ")% ");
  /*  B2INFO("     nFittedTracks = "<< nFittedTracks );
  B2INFO("     n inefficiency = "<< m_multiplicityFittedTracks->GetBinContent(1) );
  B2INFO("           NUM = "<< nFittedTracks );
  B2INFO("           DEN = "<< nFittedTracks+m_multiplicityFittedTracks->GetBinContent(1) ); */
  /*  B2INFO("     nMCParticles = "<< nMCParticles);
  B2INFO("     n inpurity = "<< m_multiplicityMCParticles->GetBinContent(1) );
  B2INFO("           NUM = "<< nMCParticles );
  B2INFO("           DEN = "<< nMCParticles+m_multiplicityMCParticles->GetBinContent(1) );*/
  B2INFO("");
  B2INFO("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
}

void TrackingPerformanceEvaluationModule::terminate()
{


  addEfficiencyPlots(m_histoList);

  addInefficiencyPlots(m_histoList);

  addPurityPlots(m_histoList);

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TIter nextH(m_histoList);
    TObject* obj;
    while ((obj = nextH()))
      obj->Write();


    m_rootFilePtr->Close();
  }

}

TH1F* TrackingPerformanceEvaluationModule::createHistogram1D(const char* name, const char* title,
    Int_t nbins, Double_t min, Double_t max,
    const char* xtitle, TList* histoList)
{

  TH1F* h = new TH1F(name, title, nbins, min, max);

  h->GetXaxis()->SetTitle(xtitle);

  if (histoList)
    histoList->Add(h);

  return h;
}


TH2F* TrackingPerformanceEvaluationModule::createHistogram2D(const char* name, const char* title,
    Int_t nbinsX, Double_t minX, Double_t maxX,
    const char* titleX,
    Int_t nbinsY, Double_t minY, Double_t maxY,
    const char* titleY, TList* histoList)
{

  TH2F* h = new TH2F(name, title, nbinsX, minX, maxX, nbinsY, minY, maxY);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH3F* TrackingPerformanceEvaluationModule::createHistogram3D(const char* name, const char* title,
    Int_t nbinsX, Double_t minX, Double_t maxX,
    const char* titleX,
    Int_t nbinsY, Double_t minY, Double_t maxY,
    const char* titleY,
    Int_t nbinsZ, Double_t minZ, Double_t maxZ,
    const char* titleZ,
    TList* histoList)
{

  TH3F* h = new TH3F(name, title, nbinsX, minX, maxX, nbinsY, minY, maxY, nbinsZ, minZ, maxZ);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);
  h->GetZaxis()->SetTitle(titleZ);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH3F* TrackingPerformanceEvaluationModule::createHistogram3D(const char* name, const char* title,
    Int_t nbinsX, Double_t* binsX,
    const char* titleX,
    Int_t nbinsY, Double_t* binsY,
    const char* titleY,
    Int_t nbinsZ, Double_t* binsZ,
    const char* titleZ,
    TList* histoList)
{

  TH3F* h = new TH3F(name, title, nbinsX, binsX, nbinsY, binsY, nbinsZ, binsZ);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);
  h->GetZaxis()->SetTitle(titleZ);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH1* TrackingPerformanceEvaluationModule::duplicateHistogram(const char* newname, const char* newtitle,
    TH1* h, TList* histoList)
{

  TH1F* h1 =  dynamic_cast<TH1F*>(h);
  TH2F* h2 =  dynamic_cast<TH2F*>(h);
  TH3F* h3 =  dynamic_cast<TH3F*>(h);

  TH1* newh = 0;

  if (h1)
    newh = new TH1F(*h1);
  if (h2)
    newh = new TH2F(*h2);
  if (h3)
    newh = new TH3F(*h3);

  newh->SetName(newname);
  newh->SetTitle(newtitle);

  if (histoList)
    histoList->Add(newh);


  return newh;
}

TH1F* TrackingPerformanceEvaluationModule::createHistogramsRatio(const char* name, const char* title,
    TH1* hNum, TH1* hDen, bool isEffPlot,
    int axisRef)
{

  TH1F* h1den =  dynamic_cast<TH1F*>(hDen);
  TH1F* h1num =  dynamic_cast<TH1F*>(hNum);
  TH2F* h2den =  dynamic_cast<TH2F*>(hDen);
  TH2F* h2num =  dynamic_cast<TH2F*>(hNum);
  TH3F* h3den =  dynamic_cast<TH3F*>(hDen);
  TH3F* h3num =  dynamic_cast<TH3F*>(hNum);

  TH1* hden = 0;
  TH1* hnum = 0;

  if (h1den) {
    hden = new TH1F(*h1den);
    hnum = new TH1F(*h1num);
  }
  if (h2den) {
    hden = new TH2F(*h2den);
    hnum = new TH2F(*h2num);
  }
  if (h3den) {
    hden = new TH3F(*h3den);
    hnum = new TH3F(*h3num);
  }

  TAxis* the_axis;
  TAxis* the_other1;
  TAxis* the_other2;

  if (axisRef == 0) {
    the_axis = hden->GetXaxis();
    the_other1 = hden->GetYaxis();
    the_other2 = hden->GetZaxis();
  } else if (axisRef == 1) {
    the_axis = hden->GetYaxis();
    the_other1 = hden->GetXaxis();
    the_other2 = hden->GetZaxis();
  } else if (axisRef == 2) {
    the_axis = hden->GetZaxis();
    the_other1 = hden->GetXaxis();
    the_other2 = hden->GetYaxis();
  } else
    return NULL;


  TH1F* h;
  if (the_axis->GetXbins()->GetSize())
    h = new TH1F(name, title, the_axis->GetNbins(), (the_axis->GetXbins())->GetArray());
  else
    h = new TH1F(name, title, the_axis->GetNbins(), the_axis->GetXmin(), the_axis->GetXmax());
  h->GetXaxis()->SetTitle(the_axis->GetTitle());

  h->GetYaxis()->SetRangeUser(0.00001, 1);

  double num = 0;
  double den = 0;
  Int_t bin = 0;
  Int_t nBins = 0;

  //  m_nFittedTracks =0 ;
  //  m_nMCParticles =0;

  for (int the_bin = 1; the_bin < the_axis->GetNbins() + 1; the_bin++) {

    num = 0;
    den = 0 ;

    for (int other1_bin = 1; other1_bin < the_other1->GetNbins() + 1; other1_bin++)
      for (int other2_bin = 1; other2_bin < the_other2->GetNbins() + 1; other2_bin++) {

        if (axisRef == 0) bin = hden->GetBin(the_bin, other1_bin, other2_bin);
        else if (axisRef == 1) bin = hden->GetBin(other1_bin, the_bin, other2_bin);
        else if (axisRef == 2) bin = hden->GetBin(other1_bin, other2_bin, the_bin);

        if (hden->IsBinUnderflow(bin))
          B2INFO("  bin = " << bin << "(" << the_bin << "," << other1_bin << "," << other2_bin << "), UNDERFLOW");
        if (hden->IsBinOverflow(bin))
          B2INFO("  bin = " << bin << "(" << the_bin << "," << other1_bin << "," << other2_bin << "), OVERFLOW");

        num += hnum->GetBinContent(bin);
        den += hden->GetBinContent(bin);

        nBins++;

      }
    double eff = 0;
    double err = 0;

    if (den > 0) {
      eff = (double)num / den;
      err = sqrt(eff * (1 - eff)) / sqrt(den);
    }

    //    m_nFittedTracks += num;
    //    m_nMCParticles  += den;

    if (isEffPlot) {
      h->SetBinContent(the_bin, eff);
      h->SetBinError(the_bin, err);
    } else {
      h->SetBinContent(the_bin, 1 - eff);
      h->SetBinError(the_bin, err);
    }

    //    B2INFO(" the_bin = "<<the_bin<<" lowEdge: "<<h->GetXaxis()->GetBinLowEdge(the_bin)<<", den = "<<den<<", eff = "<<eff);
  }

  //  B2INFO("  total number of bins summed = "<<nBins);

  return h;

}

void  TrackingPerformanceEvaluationModule::fillTrackParams1DHistograms(const TrackFitResult* fitResult, MCParticleInfo mcParticleInfo)
{

  //track parameters errors
  double d0_err = sqrt((fitResult->getCovariance5())[0][0]);
  double phi_err =  sqrt((fitResult->getCovariance5())[1][1]);
  double omega_err = sqrt((fitResult->getCovariance5())[2][2]);
  double z0_err =  sqrt((fitResult->getCovariance5())[3][3]);
  double cotTheta_err = sqrt((fitResult->getCovariance5())[4][4]);

  //track parameters residuals:
  double d0_res = fitResult->getD0() - mcParticleInfo.getD0();
  double phi_res = fmod(fitResult->getPhi() - mcParticleInfo.getPhi() + TMath::Pi(), 2 * TMath::Pi()) - TMath::Pi();
  double omega_res =  fitResult->getOmega() - mcParticleInfo.getOmega();
  double z0_res = fitResult->getZ0() - mcParticleInfo.getZ0();
  double cotTheta_res = fitResult->getCotTheta() - mcParticleInfo.getCotTheta();

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

  m_h1_d0_pll->Fill(d0_res / d0_err);
  m_h1_phi_pll->Fill(phi_res / phi_err);
  m_h1_omega_pll->Fill(omega_res / omega_err);
  m_h1_z0_pll->Fill(z0_res / z0_err);
  m_h1_cotTheta_pll->Fill(cotTheta_res / cotTheta_res);

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


  m_h2_d0errMSVSpt->Fill(pt, d0_err * beta * p * pow(sinTheta, 3 / 2) / 0.0136);

}

void TrackingPerformanceEvaluationModule::fillHitsUsedInTrackFitHistograms(const genfit::Track& track)
{

  //hits used in the fit
  int nHits = track.getNumPointsWithMeasurement();
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  bool hasPXDhit = false;
  double d0_err = -999;
  double pt = -999;
  const TrackFitResult* fitResult = DataStore::getRelatedFromObj<TrackFitResult>(&track);

  if (fitResult != NULL) { // valid TrackFitResult found
    d0_err = sqrt((fitResult->getCovariance5())[0][0]);
    pt = fitResult->getMomentum().Pt();
  }

  for (int i = 0; i < nHits; i++) {
    genfit::TrackPoint* tp = track.getPointWithMeasurement(i);
    genfit::AbsMeasurement* absMeas = tp->getRawMeasurement();

    double detId(-999);
    TVector3 globalHit(-999, -999, -999);

    PXDRecoHit* pxdHit =  dynamic_cast<PXDRecoHit*>(absMeas);
    SVDRecoHit2D* svdHit2D =  dynamic_cast<SVDRecoHit2D*>(absMeas);
    SVDRecoHit* svdHit =  dynamic_cast<SVDRecoHit*>(absMeas);


    if (pxdHit) {
      hasPXDhit = true;

      detId = 0;
      double uCoor = pxdHit->getU();
      double vCoor = pxdHit->getV();
      VxdID sensor = pxdHit->getSensorID();

      m_h1_nVXDhitsUsed->Fill(sensor.getLayerNumber());

      const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(sensor);
      globalHit = aSensorInfo.pointToGlobal(TVector3(uCoor, vCoor, 0));

    } else if (svdHit2D) {

      detId = 1;
      double uCoor = svdHit2D->getU();
      double vCoor = svdHit2D->getV();
      VxdID sensor = svdHit2D->getSensorID();

      m_h1_nVXDhitsUsed->Fill(sensor.getLayerNumber());

      const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(sensor);
      globalHit = aSensorInfo.pointToGlobal(TVector3(uCoor, vCoor, 0));

    } else if (svdHit) {

      detId = 2;
      double uCoor = 0;
      double vCoor = 0;
      if (svdHit->isU())
        uCoor = svdHit->getPosition();
      else
        vCoor =  svdHit->getPosition();

      VxdID sensor = svdHit->getSensorID();
      m_h1_nVXDhitsUsed->Fill(sensor.getLayerNumber());

      const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(sensor);
      globalHit = aSensorInfo.pointToGlobal(TVector3(uCoor, vCoor, 0));
    } else
      detId = 3;


    m_h1_nHitDetID ->Fill(detId);

    m_h2_VXDhitsUsed_xy->Fill(globalHit.X(), globalHit.Y());

    m_h2_VXDhitsUsed_rz->Fill(globalHit.Z(), globalHit.Perp());
  }

  if (fitResult != NULL) {
    if (hasPXDhit)
      m_h2_d0errVSpt_wpxd->Fill(pt, d0_err);
    else
      m_h2_d0errVSpt_wopxd->Fill(pt, d0_err);
  }

}

void TrackingPerformanceEvaluationModule::addInefficiencyPlots(TList* histoList)
{

  //normalized to MCParticles
  TH1F* h_ineff_pt = createHistogramsRatio("hineffpt", "inefficiency VS pt, normalized to MCParticles", m_h3_TracksPerMCParticle, m_h3_MCParticle, false, 0);
  histoList->Add(h_ineff_pt);

  TH1F* h_ineff_theta = createHistogramsRatio("hinefftheta", "inefficiency VS #theta, normalized to MCParticles", m_h3_TracksPerMCParticle, m_h3_MCParticle, false, 1);
  histoList->Add(h_ineff_theta);

  TH1F* h_ineff_phi = createHistogramsRatio("hineffphi", "inefficiency VS #phi, normalized to MCParticles", m_h3_TracksPerMCParticle, m_h3_MCParticle, false, 2);
  histoList->Add(h_ineff_phi);

  //normalized to MCTrackCands
  TH1F* h_ineffMCTC_pt = createHistogramsRatio("hineffMCTCpt", "inefficiency VS pt, normalized to MCTrackCand", m_h3_TracksPerMCTrackCand, m_h3_MCTrackCand, false, 0);
  histoList->Add(h_ineffMCTC_pt);

  TH1F* h_ineffMCTC_theta = createHistogramsRatio("hineffMCTCtheta", "inefficiency VS #theta, normalized to MCTrackCand", m_h3_TracksPerMCTrackCand, m_h3_MCTrackCand, false, 1);
  histoList->Add(h_ineffMCTC_theta);

  TH1F* h_ineffMCTC_phi = createHistogramsRatio("hineffMCTCphi", "inefficiency VS #phi, normalized to MCTrackCand", m_h3_TracksPerMCTrackCand, m_h3_MCTrackCand, false, 2);
  histoList->Add(h_ineffMCTC_phi);

}

void TrackingPerformanceEvaluationModule::addEfficiencyPlots(TList* histoList)
{



  //normalized to MCParticles
  TH1F* h_eff_pt = createHistogramsRatio("heffpt", "efficiency VS pt, normalized to MCParticles", m_h3_TracksPerMCParticle, m_h3_MCParticle, true, 0);
  histoList->Add(h_eff_pt);
  //  B2INFO(" efficiency in pt, NUM =  "<<m_nFittedTracks<<", DEN = "<<m_nMCParticles<<", eff integrata = "<<(double)m_nFittedTracks/m_nMCParticles);

  TH1F* h_eff_theta = createHistogramsRatio("hefftheta", "efficiency VS #theta, normalized to MCParticles", m_h3_TracksPerMCParticle, m_h3_MCParticle, true, 1);
  histoList->Add(h_eff_theta);

  TH1F* h_eff_phi = createHistogramsRatio("heffphi", "efficiency VS #phi, normalized to MCParticles", m_h3_TracksPerMCParticle, m_h3_MCParticle, true, 2);
  histoList->Add(h_eff_phi);

  //normalized to MCTrackCands
  TH1F* h_effMCTC_pt = createHistogramsRatio("heffMCTCpt", "efficiency VS pt, normalized to MCTrackCand", m_h3_TracksPerMCTrackCand, m_h3_MCTrackCand, true, 0);
  histoList->Add(h_effMCTC_pt);

  TH1F* h_effMCTC_theta = createHistogramsRatio("heffMCTCtheta", "efficiency VS #theta, normalized to MCTrackCand", m_h3_TracksPerMCTrackCand, m_h3_MCTrackCand, true, 1);
  histoList->Add(h_effMCTC_theta);

  TH1F* h_effMCTC_phi = createHistogramsRatio("heffMCTCphi", "efficiency VS #phi, normalized to MCTrackCand", m_h3_TracksPerMCTrackCand, m_h3_MCTrackCand, true, 2);
  histoList->Add(h_effMCTC_phi);

  // plus

  //normalized to MCParticles
  TH1F* h_eff_pt_plus = createHistogramsRatio("heffpt_plus", "efficiency VS pt, normalized to positive MCParticles", m_h3_TracksPerMCParticle_plus, m_h3_MCParticle_plus, true, 0);
  histoList->Add(h_eff_pt_plus);
  //  B2INFO(" efficiency in pt, NUM =  "<<m_nFittedTracks<<", DEN = "<<m_nMCParticles<<", eff integrata = "<<(double)m_nFittedTracks/m_nMCParticles);

  TH1F* h_eff_theta_plus = createHistogramsRatio("hefftheta_plus", "efficiency VS #theta, normalized to positive MCParticles", m_h3_TracksPerMCParticle_plus, m_h3_MCParticle_plus, true, 1);
  histoList->Add(h_eff_theta_plus);

  TH1F* h_eff_phi_plus = createHistogramsRatio("heffphi_plus", "efficiency VS #phi, normalized to positive  MCParticles", m_h3_TracksPerMCParticle_plus, m_h3_MCParticle_plus, true, 2);
  histoList->Add(h_eff_phi_plus);

  //normalized to MCTrackCands
  TH1F* h_effMCTC_pt_plus = createHistogramsRatio("heffMCTCpt_plus", "efficiency VS pt, normalized to positive MCTrackCand", m_h3_TracksPerMCTrackCand_plus, m_h3_MCTrackCand_plus, true, 0);
  histoList->Add(h_effMCTC_pt_plus);

  TH1F* h_effMCTC_theta_plus = createHistogramsRatio("heffMCTCtheta_plus", "efficiency VS #theta, normalized to positive MCTrackCand", m_h3_TracksPerMCTrackCand_plus, m_h3_MCTrackCand_plus, true, 1);
  histoList->Add(h_effMCTC_theta_plus);

  TH1F* h_effMCTC_phi_plus = createHistogramsRatio("heffMCTCphi_plus", "efficiency VS #phi, normalized to positive MCTrackCand", m_h3_TracksPerMCTrackCand_plus, m_h3_MCTrackCand_plus, true, 2);
  histoList->Add(h_effMCTC_phi_plus);

  // minus

  //normalized to MCParticles
  TH1F* h_eff_pt_minus = createHistogramsRatio("heffpt_minus", "efficiency VS pt, normalized to positive MCParticles", m_h3_TracksPerMCParticle_minus, m_h3_MCParticle_minus, true, 0);
  histoList->Add(h_eff_pt_minus);
  //  B2INFO(" efficiency in pt, NUM =  "<<m_nFittedTracks<<", DEN = "<<m_nMCParticles<<", eff integrata = "<<(double)m_nFittedTracks/m_nMCParticles);

  TH1F* h_eff_theta_minus = createHistogramsRatio("hefftheta_minus", "efficiency VS #theta, normalized to positive MCParticles", m_h3_TracksPerMCParticle_minus, m_h3_MCParticle_minus, true, 1);
  histoList->Add(h_eff_theta_minus);

  TH1F* h_eff_phi_minus = createHistogramsRatio("heffphi_minus", "efficiency VS #phi, normalized to positive  MCParticles", m_h3_TracksPerMCParticle_minus, m_h3_MCParticle_minus, true, 2);
  histoList->Add(h_eff_phi_minus);

  //normalized to MCTrackCands
  TH1F* h_effMCTC_pt_minus = createHistogramsRatio("heffMCTCpt_minus", "efficiency VS pt, normalized to positive MCTrackCand", m_h3_TracksPerMCTrackCand_minus, m_h3_MCTrackCand_minus, true, 0);
  histoList->Add(h_effMCTC_pt_minus);

  TH1F* h_effMCTC_theta_minus = createHistogramsRatio("heffMCTCtheta_minus", "efficiency VS #theta, normalized to positive MCTrackCand", m_h3_TracksPerMCTrackCand_minus, m_h3_MCTrackCand_minus, true, 1);
  histoList->Add(h_effMCTC_theta_minus);

  TH1F* h_effMCTC_phi_minus = createHistogramsRatio("heffMCTCphi_minus", "efficiency VS #phi, normalized to positive MCTrackCand", m_h3_TracksPerMCTrackCand_minus, m_h3_MCTrackCand_minus, true, 2);
  histoList->Add(h_effMCTC_phi_minus);

  //pattern recognition efficiency
  TH1F* h_effPR = createHistogramsRatio("heffPR", "PR efficiency VS VXD Layer, normalized to MCTrackCand", m_h1_HitsTrackCandPerMCTrackCand, m_h1_HitsMCTrackCand, true, 0);
  histoList->Add(h_effPR);


}



void TrackingPerformanceEvaluationModule::addPurityPlots(TList* histoList)
{

//purity histograms
  TH1F* h_pur_pt = createHistogramsRatio("hpurpt", "purity VS pt", m_h3_MCParticlesPerTrack, m_h3_Tracks, true, 0);
  histoList->Add(h_pur_pt);
  //  B2INFO(" purity in pt, NUM =  "<<m_nFittedTracks<<", DEN = "<<m_nMCParticles<<", eff integrata = "<<(double)m_nFittedTracks/m_nMCParticles);

  TH1F* h_pur_theta = createHistogramsRatio("hpurtheta", "purity VS #theta", m_h3_MCParticlesPerTrack, m_h3_Tracks, true, 1);
  histoList->Add(h_pur_theta);

  TH1F* h_pur_phi = createHistogramsRatio("hpurphi", "purity VS #phi", m_h3_MCParticlesPerTrack, m_h3_Tracks, true, 2);
  histoList->Add(h_pur_phi);

}

bool TrackingPerformanceEvaluationModule::isTraceable(const MCParticle& the_mcParticle)
{

  bool isChargedStable = Const::chargedStableSet.find(abs(the_mcParticle.getPDG())) != Const::invalidParticle;

  bool isPrimary = the_mcParticle.hasStatus(MCParticle::c_PrimaryParticle);

  return (isPrimary && isChargedStable);

}
