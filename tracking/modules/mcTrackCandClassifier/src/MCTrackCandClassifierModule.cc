/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <boost/foreach.hpp>

#include <framework/datastore/RelationArray.h>

#include <tracking/modules/mcTrackCandClassifier/MCTrackCandClassifierModule.h>

#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>
#include <genfit/TGeoMaterialInterface.h>
#include <genfit/Track.h>

#include <geometry/GeometryManager.h>
#include <mdst/dataobjects/Track.h>
#include <TGeoManager.h>
#include <tracking/gfbfield/GFGeant4Field.h>

#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <vxd/geometry/GeoCache.h>




using namespace std;
using namespace Belle2;

/// Register the Module
REG_MODULE(MCTrackCandClassifier)

/// Implementation
MCTrackCandClassifierModule::MCTrackCandClassifierModule() : Module()
  , m_rootFilePtr(NULL)
{
  //Set module properties
  setDescription("This module is meant to classify the MCTrackCands as either ideal, fine and nasty");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("MCParticlesName", m_mcParticlesName,
           "Name of MC Particle collection.",
           std::string(""));

  addParam("MCTrackCandCollName", m_mcTrackCandsColName,
           "Name of the input collection of MC track candidates",
           std::string(""));

  addParam("rootFileName", m_rootFileName,
           "Name of the root file",
           std::string("MCTrackCandClassifier.root"));

  addParam("isInAnnulusCriterium", m_applyAnnulus,
           "Require that the hit is in the expected annulus",
           bool(true));
  addParam("isInSemiplaneCriterium", m_applySemiplane,
           "Require that the hit is in the expected semiplane",
           bool(true));
  addParam("isInFirstLapCriterium", m_applyLap,
           "Require that the hit belong to the first lap in the transverse plane",
           bool(true));
  addParam("isInWedgePartCriterium", m_applyWedge,
           "Require that the hit belong to the barrel part of the SVD",
           bool(true));
  addParam("removeBadHits", m_removeBadHits,
           "Remove the clusters that do not satisfy the criteria from the idealMCTrackCands",
           bool(true));

  addParam("minNhits", m_minHit,
           "Minimum number of 1D Clusters to classify the MCTrackCand as ideal",
           int(5));

  addParam("nSigma_dR", m_nSigma, "n sigma dR", int(3));

  addParam("lapFraction", m_fraction, "Fraction of lap", double(1));

  addParam("usePXD", m_usePXD, "Use the PXD or not", bool(true));
}


void MCTrackCandClassifierModule::initialize()
{
  // MCParticles, MCTrackCands, MCTracks needed for this module
  StoreArray<PXDCluster> pxdClusters;
  pxdClusters.isRequired();

  StoreArray<SVDCluster> svdClusters;
  svdClusters.isRequired();

  StoreArray<MCParticle> mcParticles(m_mcParticlesName);
  mcParticles.isRequired();

  StoreArray<genfit::TrackCand> mcTrackCands(m_mcTrackCandsColName);
  mcTrackCands.isRequired();
  StoreArray<genfit::TrackCand> idealMCTrackCands("idealMCTrackCands");
  idealMCTrackCands.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);

  StoreArray<PXDTrueHit> pxdTrueHits;
  pxdTrueHits.isRequired();

  StoreArray<SVDTrueHit> svdTrueHits;
  svdTrueHits.isRequired();

  //create list of histograms to be saved in the rootfile
  m_histoList = new TList;

  //set the ROOT File
  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  //histograms to produce efficiency plots
  Double_t bins_pt[9 + 1] = {0, 0.05, 0.1, 0.15, 0.2, 0.3, 0.5, 1, 2, 3.5}; //GeV/c
  Double_t bins_theta[10 + 1] = {0, 0.25, 0.5, 0.75, 0.75 + 0.32, 0.75 + 2 * 0.32, 0.75 + 3 * 0.32, 0.75 + 4 * 0.32, 0.75 + 5 * 0.32, 2.65, TMath::Pi()};
  Double_t bins_lambda[10 + 1];
  Double_t width_lambda =  TMath::Pi() / 10;
  Double_t bins_phi[14 + 1];
  Double_t width_phi = 2 * TMath::Pi() / 14;
  for (int bin = 0; bin < 14 + 1; bin++)
    bins_phi[bin] = - TMath::Pi() + bin * width_phi;

  for (int bin = 0; bin < 10 + 1; bin++) {
    bins_lambda[bin] = - TMath::Pi() / 2 + bin * width_lambda;
    B2DEBUG(1, bins_lambda[bin] << "   " <<  bins_theta[bin]);
  }

  m_h3_MCParticle = createHistogram3D("h3MCParticle", "entry per MCParticle",
                                      9, bins_pt, "p_{t} (GeV/c)",
                                      10, bins_lambda, "#lambda",
                                      14, bins_phi, "#phi" /*, m_histoList*/);

  m_h3_idealMCTrackCand = (TH3F*)duplicateHistogram("h3idealMCTrackCand",
                                                    "entry per idealMCTrackCand",
                                                    m_h3_MCParticle /*, m_histoList*/);

  m_h3_MCTrackCand = (TH3F*)duplicateHistogram("h3MCTrackCand",
                                               "entry per MCTrackCand",
                                               m_h3_MCParticle /*, m_histoList*/);


  m_h1_thetaMS_SVD = new TH1F("h1thetaMS_SVD", "Multiple Scattering Angle (SVD)", 500, 0, 500);
  m_histoList->Add(m_h1_thetaMS_SVD);
  m_h1_thetaMS_SVD->GetXaxis()->SetTitle("#theta_{MS} (mrad)");

  m_h1_thetaMS_PXD = (TH1F*) duplicateHistogram("h1thetaMS_PXD", "Multiple Scattering Angle (PXD)", m_h1_thetaMS_SVD, m_histoList);

  m_h1_dR = new TH1F("h1dR", "dR, annulus half width", 1000, 0, 5);
  m_histoList->Add(m_h1_dR);
  m_h1_dR->GetXaxis()->SetTitle("dR (cm)");

  m_h1_dRoverR = new TH1F("h1dRoverR", "dR over helix radius", 1000, 0, 0.1);
  m_histoList->Add(m_h1_dRoverR);
  m_h1_dRoverR->GetXaxis()->SetTitle("dR/R");

  m_h1_distOVERdR = new TH1F("h1distOVERdR", "(hit radius - helix radius)/dR", 100, -5, 5);
  m_histoList->Add(m_h1_distOVERdR);
  m_h1_distOVERdR->GetXaxis()->SetTitle("(hit R - helix R)/dR");

  m_h1_hitRadius_accepted = new TH1F("h1hitRadAccep", "hit radius for accepted hits", 100, 0, 500);
  m_histoList->Add(m_h1_hitRadius_accepted);
  m_h1_hitRadius_accepted->GetXaxis()->SetTitle("hit radius (cm)");

  m_h1_hitRadius_rejected = new TH1F("h1hitRadRejec", "hit radius for rejected hits", 100, 0, 500);
  m_histoList->Add(m_h1_hitRadius_rejected);
  m_h1_hitRadius_rejected->GetXaxis()->SetTitle("hit radius (cm)");

  m_h1_hitDistance_accepted = new TH1F("h1hitDistCentAccep", "hit distance from 0,0 for accepted hits", 100, 0, 15);
  m_histoList->Add(m_h1_hitDistance_accepted);
  m_h1_hitDistance_accepted->GetXaxis()->SetTitle("hit distance (cm)");

  m_h1_hitDistance_rejected = new TH1F("h1hitDistCentRejec", "hit distance from 0,0 for rejected hits", 100, 0, 15);
  m_histoList->Add(m_h1_hitDistance_rejected);
  m_h1_hitDistance_rejected->GetXaxis()->SetTitle("hit distance (cm)");

  m_h1_lapTime = new TH1F("h1LapTime", "lap time", 200, 0, 100);
  m_histoList->Add(m_h1_lapTime);
  m_h1_lapTime->GetXaxis()->SetTitle("time (ns)");

  m_h1_timeDifference = (TH1F*)duplicateHistogram("h1TimeDiff", "Hit Time Difference",
                                                  m_h1_lapTime, m_histoList);
  m_h1_diffOVERlap = new TH1F("h1HitDiffOVERlap", "Hit Time Difference over Lap Time",
                              100, 0, 1.5);
  m_histoList->Add(m_h1_diffOVERlap);
  m_h1_diffOVERlap->GetXaxis()->SetTitle("hit difference/lap");

  m_h1_nGoodTrueHits = new TH1F("h1nTrueHitsGoods", "Number of True Hits for Accepted Tracks", 20, 0, 20);
  m_histoList->Add(m_h1_nGoodTrueHits);
  m_h1_nGoodTrueHits->GetXaxis()->SetTitle("number of hits");

  m_h1_nBadTrueHits = new TH1F("h1nTrueHitsBads", "Number of True Hits for Rejected Tracks", 10, 0, 10);
  m_histoList->Add(m_h1_nBadTrueHits);
  m_h1_nBadTrueHits->GetXaxis()->SetTitle("number of hits");

  m_h1_nGood1dInfo = new TH1F("h1nGood1Dinfo", "Number of 1D Info for Accepted Tracks", 20, 0, 20);
  m_histoList->Add(m_h1_nGood1dInfo);
  m_h1_nGood1dInfo->GetXaxis()->SetTitle("number of hits");

  m_h1_nBad1dInfo = new TH1F("h1nBad1Dinfo", "Number of 1D Info for Rejected Tracks", 20, 0, 20);
  m_histoList->Add(m_h1_nBad1dInfo);
  m_h1_nBad1dInfo->GetXaxis()->SetTitle("number of hits");

  m_h1_firstRejectedHit = new TH1F("h1idealMCTCnHit", "idealMCTrackCands number of hits", 40, 0, 40);
  m_histoList->Add(m_h1_firstRejectedHit);
  m_h1_firstRejectedHit->GetXaxis()->SetTitle("# idealMCTrackCands hits");

  m_h1_firstRejectedOVERMCHit = new TH1F("h1FirstRejOVERmc", "# idealMCTrackCands hits / # MCTrackCands hits", 100, 0, 1);
  m_histoList->Add(m_h1_firstRejectedOVERMCHit);
  m_h1_firstRejectedOVERMCHit->GetXaxis()->SetTitle("# idealMCTrackCands hits / # MCTrackCands hits");

  m_h1_MCTrackCandNhits = (TH1F*)duplicateHistogram("h1MCTrackCandNhits", "number of MCTrackCands hits", m_h1_firstRejectedHit,
                                                    m_histoList);
}


void MCTrackCandClassifierModule::beginRun()
{
  nWedge = 0;
  nBarrel = 0;
}


void MCTrackCandClassifierModule::event()
{
  B2Vector3D magField = BFieldManager::getField(0, 0, 0) / Unit::T;

  B2DEBUG(1, "+++++ 1. loop on MCTrackCands");

  StoreArray<genfit::TrackCand> idealMCTrackCands("idealMCTrackCands");
  StoreArray<genfit::TrackCand> mcTrackCands;
  StoreArray<PXDCluster> pxdClusters;
  StoreArray<SVDCluster> svdClusters;

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  //1.a retrieve the MCTrackCands
  BOOST_FOREACH(genfit::TrackCand & mcTrackCand, mcTrackCands) {

    int nGoodTrueHits = 0;
    int nBadTrueHits = 0;
    int nGood1Dinfo = 0;

    B2DEBUG(1, " a NEW MCTrackCand ");

    //1.b retrieve the MCParticle
    RelationVector<MCParticle> MCParticles_fromMCTrackCand = DataStore::getRelationsWithObj<MCParticle>(&mcTrackCand);

    B2DEBUG(1, "~~~ " << MCParticles_fromMCTrackCand.size() << " MCParticles related to this MCTrackCand");
    for (int mcp = 0; mcp < (int)MCParticles_fromMCTrackCand.size(); mcp++) { //should be ONE

      MCParticle mcParticle = *MCParticles_fromMCTrackCand[mcp];

      B2DEBUG(1, " a NEW charged MC Particle, " << mcParticle.getIndex() << ", " << mcParticle.getPDG());

      MCParticleInfo mcParticleInfo(mcParticle, magField);

      TVector3 decayVertex = mcParticle.getProductionVertex();
      TVector3 mom = mcParticle.getMomentum();
      double charge = mcParticle.getCharge();
      double omega = mcParticleInfo.getOmega();
      double px = mom.Px();
      double py = mom.Py();
      double pt = mom.Pt();
      double x = decayVertex.X();
      double y = decayVertex.Y();
      double R = 1 / abs(omega); //cm

      m_h3_MCTrackCand->Fill(mcParticleInfo.getPt(), mcParticleInfo.getLambda(), mcParticleInfo.getPphi());

      double alpha =  R / pt * charge; //cm/GeV
      double Cx = x + alpha * py; //cm
      double Cy = y - alpha * px; //cm

      TVector3 center(Cx, Cy, 0);

      //recover Clusters and loop on them
      int Nhits = mcTrackCand.getNHits();
      m_h1_MCTrackCandNhits->Fill(Nhits);

      int cluster = 0;
      bool hasTrueHit = true;
      bool isAccepted = true;
      int firstRejectedHit =  Nhits + 1;
      double prevHitRadius = abs(1 / omega);

      double lapTime = 2 * M_PI * mcParticle.getEnergy() / 0.299792 / magField.Z();
      double FirstHitTime = -1;
      double HitTime = -1;

      bool isFirstSVDhit = true;

      while (cluster < Nhits && isAccepted && hasTrueHit) {
        int detId, hitId;
        mcTrackCand.getHit(cluster, detId, hitId);

        bool hasPXDCluster = false;
        bool hasSVDuCluster = false;
        bool hasSVDvCluster = false;

        double uCoor = 0;
        double vCoor = 0;
        VxdID sensor = 0;

        double thetaMS = 0;

        if (detId == Const::PXD && m_usePXD) {

          PXDCluster* aPXDCluster = pxdClusters[hitId];
          RelationVector<PXDTrueHit> PXDTrueHit_fromPXDCluster = aPXDCluster->getRelationsWith<PXDTrueHit>();
          if (PXDTrueHit_fromPXDCluster.size() == 0) {
            B2WARNING("What's happening?!? no True Hit associated to the PXD Cluster");
            hasTrueHit = false;
            isAccepted = false;
            continue;
          }

          PXDTrueHit* aPXDTrueHit = PXDTrueHit_fromPXDCluster[0];
          thetaMS = compute_thetaMS(mcParticleInfo, aPXDTrueHit);
          m_h1_thetaMS_PXD->Fill(thetaMS / 2 * 1000); //PXD

          uCoor = aPXDTrueHit->getU();
          vCoor = aPXDTrueHit->getV();
          sensor = aPXDTrueHit->getSensorID();
          if (cluster == 0) {
            FirstHitTime = aPXDTrueHit->getGlobalTime();
            HitTime = FirstHitTime;
          } else
            HitTime = aPXDTrueHit->getGlobalTime();

          hasPXDCluster = true;
        } else if (detId == Const::SVD) {
          SVDCluster* aSVDCluster = svdClusters[hitId];
          RelationVector<SVDTrueHit> SVDTrueHit_fromSVDCluster = aSVDCluster->getRelationsWith<SVDTrueHit>();
          if (SVDTrueHit_fromSVDCluster.size() == 0) {
            B2WARNING("What's happening?!? no True Hit associated to the SVD Cluster");
            hasTrueHit = false;
            isAccepted = false;
            continue;
          }

          SVDTrueHit* aSVDTrueHit = SVDTrueHit_fromSVDCluster[0];

          thetaMS = compute_thetaMS(mcParticleInfo, aSVDTrueHit);
          m_h1_thetaMS_SVD->Fill(thetaMS * 1000); //SVD

          uCoor = aSVDTrueHit->getU();
          vCoor = aSVDTrueHit->getV();
          sensor = aSVDTrueHit->getSensorID();
          if (isFirstSVDhit) {
            FirstHitTime = aSVDTrueHit->getGlobalTime();
            HitTime = FirstHitTime;
            isFirstSVDhit = false;

          } else
            HitTime = aSVDTrueHit->getGlobalTime();
          if (aSVDCluster->isUCluster())
            hasSVDuCluster = true;
          else
            hasSVDvCluster = true;
        } else {
          cluster++;
          continue;
        }

        const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(sensor);
        bool accepted4 = true;
        if (m_applyWedge) {
          if (aSensorInfo.getForwardWidth() != aSensorInfo.getBackwardWidth()) {
            nWedge++;
            accepted4 = false;
          } else
            nBarrel++;
        }

        TVector3 globalHit = aSensorInfo.pointToGlobal(TVector3(uCoor, vCoor, 0), true);
        double hitRadius = theDistance(center, globalHit);

        bool accepted1 = true;
        if (m_applySemiplane)
          accepted1 = isInSemiPlane(semiPlane(decayVertex, center, globalHit), omega);

        if (accepted1) {
          B2DEBUG(1, "     semiplane: ACCEPTED");
        } else {
          B2DEBUG(1, "     semiplane: REJECTED, next track");
        }

        double dR = compute_dR(thetaMS, theDistance(TVector3(0, 0, 0), globalHit));
        m_h1_dR->Fill(dR);
        m_h1_dRoverR->Fill(dR * abs(omega));
        m_h1_distOVERdR->Fill((hitRadius - abs(1 / omega)) / dR);


        bool accepted2 = true;
        if (m_applyAnnulus)
          accepted2 = isInAnnulus(hitRadius, prevHitRadius, dR);

        prevHitRadius = hitRadius;

        if (accepted2) {
          B2DEBUG(1, "     annulus: ACCEPTED");
        } else {
          B2DEBUG(1, "     annulus: REJECTED, next track");
        }

        bool accepted3 = true;
        if (m_applyLap)
          accepted3 = isFirstLap(FirstHitTime,  HitTime, lapTime);

        if (accepted3) {
          B2DEBUG(1, "     lapTime: ACCEPTED");
        } else {
          B2DEBUG(1, "     lapTime: REJECTED, next track");
        }

        if (accepted2 && accepted1 && accepted3 && accepted4) {
          nGoodTrueHits ++;
          m_h1_hitDistance_accepted->Fill(theDistance(TVector3(0, 0, 0), globalHit));
          m_h1_hitRadius_accepted->Fill(hitRadius);
        } else {
          nBadTrueHits ++;
          m_h1_hitDistance_rejected->Fill(theDistance(TVector3(0, 0, 0), globalHit));
          m_h1_hitRadius_rejected->Fill(hitRadius);
          if (m_removeBadHits)
            firstRejectedHit = cluster;
          isAccepted = false;
          continue;
        }

        if (hasPXDCluster)
          nGood1Dinfo = +2;
        else {
          if (hasSVDuCluster)
            nGood1Dinfo++;
          if (hasSVDvCluster)
            nGood1Dinfo++;
        }
        if (hasPXDCluster || hasSVDuCluster || hasSVDvCluster)
          B2DEBUG(1, "cluster: ACCEPTED (" << nGood1Dinfo << ")");

        cluster++;
      }//close loop on clusters

      if (nGood1Dinfo >= m_minHit) {
        B2DEBUG(1, "  idealMCTrackCand FOUND!! " << nGood1Dinfo << " 1D infos (" << nGoodTrueHits << " good true hits)");
        m_h3_idealMCTrackCand->Fill(mcParticleInfo.getPt(), mcParticleInfo.getLambda(), mcParticleInfo.getPphi());
        m_h1_nGoodTrueHits->Fill(nGoodTrueHits);
        m_h1_nGood1dInfo->Fill(nGood1Dinfo);

        genfit::TrackCand* tmpTrackCand = new genfit::TrackCand(mcTrackCand);

        if ((int)firstRejectedHit <= (int)mcTrackCand.getNHits()) {
          tmpTrackCand->reset();
          for (int hit = 0; hit < firstRejectedHit; hit++)
            if (mcTrackCand.getHit(hit))
              tmpTrackCand->addHit(mcTrackCand.getHit(hit));
          tmpTrackCand->sortHits();
        }
        idealMCTrackCands.appendNew(*tmpTrackCand);

        m_h1_firstRejectedHit->Fill(tmpTrackCand->getNHits());
        m_h1_firstRejectedOVERMCHit->Fill((float)tmpTrackCand->getNHits() / mcTrackCand.getNHits());
      } else {
        B2DEBUG(1, "  too few good hits (" << nGood1Dinfo << ") to track this one ( vs " << nGoodTrueHits << " true hits)");
        m_h1_nBadTrueHits->Fill(nGoodTrueHits);
        m_h1_nBad1dInfo->Fill(nGood1Dinfo);
      }

      B2DEBUG(1, "");
    }//close loop on MCParticles
  }//close loop on MCTrackCands
}


void MCTrackCandClassifierModule::endRun()
{
  B2INFO("** MCTrackCandClassifier parameters **");
  B2INFO("rootfilename = " << m_rootFileName);
  B2INFO("use PXD informations = " << m_usePXD);
  B2INFO("--> classification criteria:");
  if (m_applyAnnulus)
    B2INFO("  -)  |d - R| < " << m_nSigma << " dL thetaMS");
  if (m_applySemiplane)
    B2INFO("  -)  hit in the expected semiplane");
  if (m_applyLap)
    B2INFO("  -)  HitTime < " << m_fraction << " lap time");
  if (m_applyWedge)
    B2INFO("  -)  hit must be in the barrel part of the VXD");
  B2INFO("");

  double num = 0;
  double den = 0;

  num = m_h3_idealMCTrackCand->GetEntries();
  den = m_h3_MCTrackCand->GetEntries();
  double efficiency = num / den ;
  double efficiencyErr =  sqrt(efficiency * (1 - efficiency)) / sqrt(den);

  B2INFO("");
  B2INFO("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  B2INFO("~ MCTrackCandClassifier ~ SHORT SUMMARY ~");
  B2INFO("");
  B2INFO(" + overall:");
  B2INFO("   fraction of ideal MCTrackCands = (" << efficiency * 100 << " +/- " << efficiencyErr * 100 << ")% ");
  B2INFO("");
  B2INFO("   # idealMCTrackCand = " << num);
  B2INFO("        # MCTrackCand = " << den);
  B2INFO("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  B2INFO("");
  B2DEBUG(1, "  nWedge = " << nWedge);
  B2DEBUG(1, " nBarrel = " << nBarrel);
}


void MCTrackCandClassifierModule::terminate()
{
  addEfficiencyPlots(m_histoList);
  addInefficiencyPlots(m_histoList);

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TIter nextH(m_histoList);
    TObject* obj;
    while ((obj = nextH()))
      obj->Write();

    m_rootFilePtr->Close();
  }
}


double MCTrackCandClassifierModule::getXintersect(double d0, double omega, double R)
{
  double Xc = d0 + 1 / omega;
  return (R * R + Xc * Xc - 1 / omega / omega) / 2 / Xc;
}


double MCTrackCandClassifierModule::semiPlane(TVector3 vertex, TVector3 center, TVector3 hit)
{
  TVector3 err = center - vertex;

  double semiPlane = err.Y() / err.X() * hit.X() + err.Y() / err.X() * vertex.x() - vertex.Y();

  B2DEBUG(1, "");
  B2DEBUG(1, " SEMI-PLANE defined by: y + " << err.Y() / err.X() << " x + " << err.Y() / err.X()*vertex.x() - vertex.Y() << " = 0");
  B2DEBUG(1, "     with: center(" << center.X() << "," << center.Y() << ")");
  B2DEBUG(1, "           decayV(" << vertex.X() << "," << vertex.Y() << ")");
  B2DEBUG(1, "           vector(" << err.X() << "," << err.Y() << ")");
  B2DEBUG(1, "           y SLOPE = " << semiPlane << " VS y HIT = " << hit.Y());
  B2DEBUG(1, "           HIT - SLOPE = " << - semiPlane + hit.Y());

  if (vertex.X() < center.X())
    return hit.Y() - semiPlane;
  else
    return semiPlane - hit.Y();
}


bool MCTrackCandClassifierModule::isInSemiPlane(double semiPlane, double omega)
{
  if (semiPlane * omega > 0)
    return true;
  else
    return false;
}


double MCTrackCandClassifierModule::theDistance(TVector3 center, TVector3 hit)
{
  double xSquared = TMath::Power(center.X() - hit.X(), 2);
  double ySquared = TMath::Power(center.Y() - hit.Y(), 2);

  return TMath::Sqrt(xSquared + ySquared);
}


bool MCTrackCandClassifierModule::isInAnnulus(double hitDistance, double R, double dR)
{
  bool accepted = false;

  B2DEBUG(1, "");
  B2DEBUG(1, " ANNULUS defined between radii: " << R - dR << " and " << R + dR);
  B2DEBUG(1, "     hit distance = " << hitDistance);
  B2DEBUG(1, "     helix radius = " << R);
  B2DEBUG(1, "               dR = " << dR);

  if ((hitDistance > R - dR) && (hitDistance < R + dR))
    accepted = true;

  return accepted;
}


bool MCTrackCandClassifierModule::isFirstLap(double FirstHitTime, double HitTime, double LapTime)
{
  bool accepted = false;

  B2DEBUG(1, "");
  B2DEBUG(1, " lapTime: " << LapTime);
  B2DEBUG(1, "     FirstHitTime = " << FirstHitTime);
  B2DEBUG(1, "          HitTime = " << HitTime);
  B2DEBUG(1, "       difference = " << HitTime - FirstHitTime);

  m_h1_lapTime->Fill(LapTime);
  m_h1_timeDifference->Fill(HitTime - FirstHitTime);
  m_h1_diffOVERlap->Fill((HitTime - FirstHitTime) / LapTime);

  if (HitTime - FirstHitTime < m_fraction * LapTime)
    accepted = true;

  return accepted;
}


TH3F* MCTrackCandClassifierModule::createHistogram3D(const char* name, const char* title,
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


TH3F* MCTrackCandClassifierModule::createHistogram3D(const char* name, const char* title,
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


TH1* MCTrackCandClassifierModule::duplicateHistogram(const char* newname, const char* newtitle,
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


void MCTrackCandClassifierModule::addEfficiencyPlots(TList* histoList)
{
  //normalized to MCTrackCands
  TH1F* h_effMCTC_pt = createHistogramsRatio("heffMCTCpt", "fraction of idealMCTrackCand VS pt", m_h3_idealMCTrackCand,
                                             m_h3_MCTrackCand, true, 0);
  histoList->Add(h_effMCTC_pt);

  TH1F* h_effMCTC_theta = createHistogramsRatio("heffMCTCtheta", "fraction of idealMCTrackCandVS #lambda", m_h3_idealMCTrackCand,
                                                m_h3_MCTrackCand, true, 1);
  histoList->Add(h_effMCTC_theta);

  TH1F* h_effMCTC_phi = createHistogramsRatio("heffMCTCphi", "fraction of idealMCTrackCand VS #phi", m_h3_idealMCTrackCand,
                                              m_h3_MCTrackCand, true, 2);
  histoList->Add(h_effMCTC_phi);
}


void MCTrackCandClassifierModule::addInefficiencyPlots(TList* histoList)
{
  //normalized to MCTrackCands
  TH1F* h_ineffMCTC_pt = createHistogramsRatio("hineffMCTCpt", "1 - fraction of idealMCTrackCand VS pt", m_h3_idealMCTrackCand,
                                               m_h3_MCTrackCand, false, 0);
  histoList->Add(h_ineffMCTC_pt);

  TH1F* h_ineffMCTC_theta = createHistogramsRatio("hineffMCTCtheta", "1 - fraction of idealMCTrackCandVS #lambda",
                                                  m_h3_idealMCTrackCand, m_h3_MCTrackCand, false, 1);
  histoList->Add(h_ineffMCTC_theta);

  TH1F* h_ineffMCTC_phi = createHistogramsRatio("hineffMCTCphi", "1 - fraction of idealMCTrackCand VS #phi", m_h3_idealMCTrackCand,
                                                m_h3_MCTrackCand, false, 2);
  histoList->Add(h_ineffMCTC_phi);
}


TH1F* MCTrackCandClassifierModule::createHistogramsRatio(const char* name, const char* title,
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

  for (int the_bin = 1; the_bin < the_axis->GetNbins() + 1; the_bin++) {

    num = 0;
    den = 0 ;

    for (int other1_bin = 1; other1_bin < the_other1->GetNbins() + 1; other1_bin++)
      for (int other2_bin = 1; other2_bin < the_other2->GetNbins() + 1; other2_bin++) {

        if (axisRef == 0) bin = hden->GetBin(the_bin, other1_bin, other2_bin);
        else if (axisRef == 1) bin = hden->GetBin(other1_bin, the_bin, other2_bin);
        else if (axisRef == 2) bin = hden->GetBin(other1_bin, other2_bin, the_bin);

        if (hden->IsBinUnderflow(bin))
          B2DEBUG(1, "  bin = " << bin << "(" << the_bin << "," << other1_bin << "," << other2_bin << "), UNDERFLOW");
        if (hden->IsBinOverflow(bin))
          B2DEBUG(1, "  bin = " << bin << "(" << the_bin << "," << other1_bin << "," << other2_bin << "), OVERFLOW");

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

    if (isEffPlot) {
      h->SetBinContent(the_bin, eff);
      h->SetBinError(the_bin, err);
    } else {
      h->SetBinContent(the_bin, 1 - eff);
      h->SetBinError(the_bin, err);
    }

  }

  return h;
}


float MCTrackCandClassifierModule::compute_dR(double thetaMS, double hitDistance)
{
  double dL;
  if (hitDistance < 1.8) //L1
    dL = 0.4;
  else if (hitDistance < 3) //L2
    dL = 0.8;
  else if (hitDistance < 5) //L3
    dL = 1.6;
  else if (hitDistance < 9) //L4
    dL = 4.2;
  else if (hitDistance < 12) //L5
    dL = 2.4;
  else dL = 3.1;

  if ((hitDistance < 3) && (hitDistance > 1.2))
    thetaMS = thetaMS / 2;

  double dR = m_nSigma * dL * thetaMS;

  return dR;
};


float MCTrackCandClassifierModule::compute_thetaMS(MCParticleInfo& mcParticleInfo, VXDTrueHit* aTrueHit)
{
  //  double thetaMS = 0.0136 * 14 * sqrt(0.008); //SVD, PXD is half of it
  double thetaMS = 0.0136 * 14;  //SVD, PXD is half of it

  double p = mcParticleInfo.getP();
  //  double pt = mcParticleInfo.getPt();
  double E = mcParticleInfo.getEnergy();

  double X = sqrt(pow(aTrueHit->getEntryU() - aTrueHit->getExitU(), 2) +
                  pow(aTrueHit->getEntryV() - aTrueHit->getExitV(), 2) +
                  pow(aTrueHit->getEntryW() - aTrueHit->getExitW(), 2));

  double X0 = 21.82; // g cm-2
  double rho = 2.329; // g cm-3
  thetaMS = thetaMS / (p * p / E) * sqrt(X / X0 * rho);

  return thetaMS;
};
