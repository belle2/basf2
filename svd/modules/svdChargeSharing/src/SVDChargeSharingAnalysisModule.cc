/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdChargeSharing/SVDChargeSharingAnalysisModule.h>

#include <TDirectory.h>
#include <TCollection.h>
#include <reconstruction/dataobjects/VXDDedxTrack.h>
#include <vxd/geometry/GeoCache.h>
#include <mdst/dataobjects/HitPatternVXD.h>

#include <boost/foreach.hpp>

#include <TCanvas.h>
#include <TSystem.h>
#include <TLegend.h>
#include <TROOT.h>
#include <TStyle.h>


/* --------------- WARNING ---------------------------------------------- *
If you have more complex parameter types in your class then simple int,
double or std::vector of those you might need to uncomment the following
include directive to avoid an undefined reference on compilation.
* ---------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>



using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDChargeSharingAnalysis)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDChargeSharingAnalysisModule::SVDChargeSharingAnalysisModule()
  : Module()
{
  // Set module properties
  setDescription("Module for monitoring DSSD cluster charge deposition");

  // Parameter definitions
  addParam("outputDirName", m_outputDirName, "Name of the output directory.");
  addParam("outputRootFileName", m_outputRootFileName, "Name of output rootfile.");
  addParam("useTrackInfo", m_useTrackInfo, "True if using clusters related to tracks in the analysis", bool(true));
}

SVDChargeSharingAnalysisModule::~SVDChargeSharingAnalysisModule()
{
}

void SVDChargeSharingAnalysisModule::initialize()
{
  // cppcheck-suppress publicAllocationError
  m_outputRootFile = new TFile((m_outputDirName + "/" + m_outputRootFileName).c_str(), "RECREATE");

  //StoreArrays
  m_svdClusters.isRequired();
  m_Tracks.isOptional();

  // Tracks
  m_histoList_Tracks = new TList;
  h_nTracks = createHistogram1D("N_tracks", "Number of tracks", 10, 0., 10., "N tracks", "Entries", m_histoList_Tracks);
  h_TracksPvalue = createHistogram1D("Tracks_Pvalue", "P value of tracks", 10, 0., 1., "PValue", "Entries",
                                     m_histoList_Tracks);
  h_TracksMomentum = createHistogram1D("Tracks_momentum", "Tracks momentum", 500, 0., 5., "Momentum[GeV/c]", "Entries",
                                       m_histoList_Tracks);
  h_TracksnSVDhits = createHistogram1D("SVD_hits", "SVD hits", 20, 0., 20., "# SVD hits", "Entries", m_histoList_Tracks);

  // clCharge, clSize, clSNR etc.
  TString NameOfHisto;
  TString TitleOfHisto;

  for (int i = 0; i < m_nSensorTypes; i++) {
    m_histoList_clCharge[i] = new TList;
    m_histoList_clChargeVsMomentum[i] = new TList;
    m_histoList_clChargeVsIncidentAngle[i] = new TList;
    m_histoList_clChargeVsSNR[i] = new TList;

    m_histoList_clSize[i] = new TList;
    m_histoList_clSizeVsMomentum[i] = new TList;
    m_histoList_clSizeVsIncidentAngle[i] = new TList;
    m_histoList_clSizeVsSNR[i] = new TList;

    m_histoList_clSNR[i] = new TList;
    m_histoList_clSNRVsMomentum[i] = new TList;
    m_histoList_clSNRVsIncidentAngle[i] = new TList;

    TString nameSensorType = "";
    nameSensorType += m_nameSensorTypes[i];

    for (int j = 0; j < m_nSides; j++) {
      TString nameSide = "";
      nameSide = (j > 0 ? "U" : "V");

      // Cluster size
      NameOfHisto = "DSSDClusterSize_" + nameSensorType + "Side_" + nameSide;
      TitleOfHisto = "DSSD Cluster Size: " + nameSensorType + ", Side " + nameSide;
      h_clSize[i][j] = createHistogram1D(NameOfHisto, TitleOfHisto, 4, 1, 5, "Cluster Size", "Entries", m_histoList_clSize[i]);

      NameOfHisto = "DSSDClusterSizeVsMomentum_" + nameSensorType + "Side_" + nameSide;
      TitleOfHisto = "DSSD Cluster Size vs. Track Momentum: " + nameSensorType + ", Side " + nameSide;
      h_clSizeVsMomentum[i][j] = createHistogram2D(NameOfHisto, TitleOfHisto, 500, 0., 5., "Track Momentum [GeV/c]", 4, 1, 5,
                                                   "Cluster Size", m_histoList_clSizeVsMomentum[i]);

      NameOfHisto = "DSSDClusterSizeVsIncidentAngle_" + nameSensorType + "Side_" + nameSide;
      TitleOfHisto = "DSSD Cluster Size vs. Incident Angle: " + nameSensorType + ", Side " + nameSide;
      h_clSizeVsIncidentAngle[i][j] = createHistogram2D(NameOfHisto, TitleOfHisto, 0, 90., 90., "Incident Angle [deg]", 4, 1, 5,
                                                        "Cluster Size", m_histoList_clSizeVsIncidentAngle[i]);

      NameOfHisto = "DSSDClusterSizeVsSNR_" + nameSensorType + "Side_" + nameSide;
      TitleOfHisto = "DSSD Cluster Size vs. SNR: " + nameSensorType + ", Side " + nameSide;
      h_clSizeVsSNR[i][j] = createHistogram2D(NameOfHisto, TitleOfHisto, 100, 0., 100., "Cluster SNR", 4, 1, 5,
                                              "Cluster Size", m_histoList_clSizeVsSNR[i]);

      for (int k = 0; k < m_nClSizes; k++) {
        TString nameclSize = "";
        if (k < 2) nameclSize += k + 1;
        else nameclSize += ">= 3";

        // Cluster charge
        NameOfHisto = "DSSDClusterCharge_" + nameSensorType + "Side_" + nameSide + "clSize_" + nameclSize;
        TitleOfHisto = "DSSD Cluster Charge: " + nameSensorType + ", Side " + nameSide + ", clSize " + nameclSize;
        h_clCharge[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, m_nBins, m_minCharge, m_maxCharge,
                                                "Cluster Charge [e]", "Entries / (375 e)", m_histoList_clCharge[i]);

        NameOfHisto = "DSSDClusterChargeVsMomentum_" + nameSensorType + "Side_" + nameSide + "clSize_" + nameclSize;
        TitleOfHisto = "DSSD Cluster Charge vs. Track Momentum: " + nameSensorType + ", Side " + nameSide + ", clSize " + nameclSize;
        h_clChargeVsMomentum[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 500, 0, 5., "Track Momentum[GeV/c]", m_nBins,
                                                          m_minCharge, m_maxCharge, "Cluster Charge [e]",
                                                          m_histoList_clChargeVsMomentum[i]);

        NameOfHisto = "DSSDClusterChargeVsIncidentAngle_" + nameSensorType + "Side_" + nameSide + "clSize_" + nameclSize;
        TitleOfHisto = "DSSD Cluster Charge vs. Incident Angle: " + nameSensorType + ", Side " + nameSide + ", clSize " + nameclSize;
        h_clChargeVsIncidentAngle[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 90, 0., 90., "Incident Angle [deg]", m_nBins,
                                                               m_minCharge, m_maxCharge, "Cluster Charge [e]",
                                                               m_histoList_clChargeVsIncidentAngle[i]);

        NameOfHisto = "DSSDClusterChargeVsSNR_" + nameSensorType + "Side_" + nameSide + "clSize_" + nameclSize;
        TitleOfHisto = "DSSD Cluster Charge vs. SNR: " + nameSensorType + ", Side " + nameSide + ", clSize " + nameclSize;
        h_clChargeVsSNR[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 100, 0., 100., "Cluster SNR", m_nBins, m_minCharge,
                                                     m_maxCharge, "Cluster Charge [e]", m_histoList_clChargeVsSNR[i]);

        // cluster SNR
        NameOfHisto = "DSSDClusterSNR_" + nameSensorType + "Side_" + nameSide + "clSize_" + nameclSize;
        TitleOfHisto = "DSSD Cluster SNR: " + nameSensorType + ", Side " + nameSide + ", clSize " + nameclSize;
        h_clSNR[i][j][k] = createHistogram1D(NameOfHisto, TitleOfHisto, 100, 0., 100., "Cluster SNR", "Entries",
                                             m_histoList_clSNR[i]);

        NameOfHisto = "DSSDClusterSNRVsMomentum_" + nameSensorType + "Side_" + nameSide + "clSize_" + nameclSize;
        TitleOfHisto = "DSSD Cluster SNR vs. Track Momentum: " + nameSensorType + ", Side " + nameSide + ", clSize " + nameclSize;
        h_clSNRVsMomentum[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 500, 0, 5., "Track Momentum[GeV/c]",
                                                       100, 0., 100., "Cluster SNR",  m_histoList_clSNRVsMomentum[i]);

        NameOfHisto = "DSSDClusterSNRVsIncAngle_" + nameSensorType + "Side_" + nameSide + "clSize_" + nameclSize;
        TitleOfHisto = "DSSD Cluster SNR vs. Incident Angle: " + nameSensorType + ", Side " + nameSide + ", clSize " + nameclSize;
        h_clSNRVsIncidentAngle[i][j][k] = createHistogram2D(NameOfHisto, TitleOfHisto, 90, 0., 90., "Incident Angle [deg]",
                                                            100, 0., 100., "Cluster SNR",  m_histoList_clSNRVsIncidentAngle[i]);
      } //cl sizes
    } //sides
  } //sensor types
} // initialize

void SVDChargeSharingAnalysisModule::event()
{
  if (m_useTrackInfo == false) {
    B2ERROR(" you must use track information! change the module parameter to TRUE");
    return;
  }

  BOOST_FOREACH(Track & track, m_Tracks) {

    h_nTracks->Fill(m_Tracks.getEntries());
    // Obtaining track momentum, P value & SVD hits, track hypothesis made for pions(or electrons in case of TB)
    const TrackFitResult* tfr = nullptr;
    tfr = track.getTrackFitResultWithClosestMass(Const::pion);

    if (tfr) {
      h_TracksPvalue->Fill(tfr->getPValue());
      h_TracksMomentum->Fill(tfr->getMomentum().Mag());
      h_TracksnSVDhits->Fill((tfr->getHitPatternVXD()).getNSVDHits());
    } // trf

    RelationVector<RecoTrack> theRC = DataStore::getRelationsWithObj<RecoTrack>(&track);
    RelationVector<SVDCluster> svdClustersTrack = DataStore::getRelationsWithObj<SVDCluster>(theRC[0]);
    RelationVector<VXDDedxTrack> VXDdedxTr = DataStore::getRelationsWithObj<VXDDedxTrack>(&track);
    // if (theRC.size() == 0 || svdClustersTrack.size() == 0 || VXDdedxTr.size() == 0) continue;

    // double trkMom = VXDdedxTr[0]->getMomentum();

    for (int cl = 0; cl < static_cast<int>(svdClustersTrack.size()); cl++) {
      VxdID cl_VxdID = svdClustersTrack[cl]->getSensorID();
      int layer = cl_VxdID.getLayerNumber() - 3;
      // int ladder = cl_VxdID.getLadderNumber() - 1;
      int sensor = cl_VxdID.getSensorNumber() - 1;
      int side = static_cast<int>(svdClustersTrack[cl]->isUCluster());
      float clCharge = svdClustersTrack[cl]->getCharge();
      float clSNR = svdClustersTrack[cl]->getSNR();
      unsigned short clSize = svdClustersTrack[cl]->getSize();
      unsigned short clSizeIndex;
      unsigned short sensTypeIndex;
      VXD::SensorInfoBase info = VXD::GeoCache::get(cl_VxdID);
      // double sensThickness = info.getThickness();
      // double pathLength = VXDdedxTr[0]->getDx(cl); // path length in the silicon, for a given sensor
      // double incAngle = 180. / M_PI * acos(sensThickness / pathLength);

      // clSizeIndex = 0,1 for clSize 1,2 respectively and =2 for clSize >=3
      clSizeIndex = clSize >= 3 ? 2 : clSize - 1;

      if (layer == 0) { // L3 small rectangular
        sensTypeIndex = 0;
      } else if ((sensor != 0) && (sensor != (m_nSensorsOnLayer[layer] - 1))) { // L456 large rectangular (origami)
        sensTypeIndex = 1;
      } else if (sensor == m_nSensorsOnLayer[layer] - 1) { // L456 large rectangular (BWD)
        sensTypeIndex = 2;
      } else { // L456 trapezoidal (FWD)
        sensTypeIndex = 3;
      }

      // cl charge histograms
      h_clCharge[sensTypeIndex][side][clSizeIndex]->Fill(clCharge);
      // h_clChargeVsMomentum[sensTypeIndex][side][clSizeIndex]->Fill(trkMom, clCharge);
      // h_clChargeVsIncidentAngle[sensTypeIndex][side][clSizeIndex]->Fill(incAngle, clCharge);
      h_clChargeVsSNR[sensTypeIndex][side][clSizeIndex]->Fill(clSNR, clCharge);

      //cl size
      h_clSize[sensTypeIndex][side]->Fill(clSize);
      // h_clSizeVsMomentum[sensTypeIndex][side]->Fill(trkMom, clSize);
      // h_clSizeVsIncidentAngle[sensTypeIndex][side]->Fill(incAngle, clSize);
      h_clSizeVsSNR[sensTypeIndex][side]->Fill(clSNR, clSize);

      //cl SNR
      h_clSNR[sensTypeIndex][side][clSizeIndex]->Fill(clSNR);
      // h_clSNRVsMomentum[sensTypeIndex][side][clSizeIndex]->Fill(trkMom, clSize);
      // h_clSNRVsIncidentAngle[sensTypeIndex][side][clSizeIndex]->Fill(incAngle, clSize);

    } //clusters
  } //tracks

} //event()


void SVDChargeSharingAnalysisModule::terminate()
{
  gROOT->SetBatch(1);
  // comparison plots
  TCanvas* c;
  for (int i = 0; i < m_nSensorTypes; i++) {
    for (int j = 0; j < m_nSides; j++) {
      std::string nameSensorType = "";
      nameSensorType += m_nameSensorTypes[i];
      std::string nameSide = "";
      nameSide = (j > 0 ? "U" : "V");

      c = comparisonPlot(h_clCharge[i][j][0], h_clCharge[i][j][1], h_clCharge[i][j][2]);
      c->SaveAs((m_outputDirName + "/clChargeComparison_" + nameSensorType + nameSide + ".png").c_str());
      c->Close();
      gSystem->ProcessEvents();

      c = comparisonPlot(h_clSNR[i][j][0], h_clSNR[i][j][1], h_clSNR[i][j][2]);
      c->SaveAs((m_outputDirName + "/clSNRComparison_" + nameSensorType + nameSide + ".png").c_str());
      c->Close();
      gSystem->ProcessEvents();
    }
  }
  // save to .root file
  if (m_outputRootFile != nullptr) {
    m_outputRootFile->cd();
    TDirectory* oldDir = gDirectory;

    TDirectory* dir_clCharge = oldDir->mkdir("clCharge");
    TDirectory* dir_clChargeVsSNR = oldDir->mkdir("clChargeVsSNR");
    TDirectory* dir_clSize = oldDir->mkdir("clSize");
    TDirectory* dir_clSNR = oldDir->mkdir("clSNR");

    for (int i = 0; i < m_nSensorTypes; i++) {
      // cluster charge
      dir_clCharge->cd();
      TDirectory* dir_clChargeSt = dir_clCharge->mkdir(m_nameSensorTypes[i].c_str());
      dir_clChargeSt->cd();
      TIter nextH_clCharge(m_histoList_clCharge[i]);
      TObject* obj;

      while ((obj = dynamic_cast<TH1F*>(nextH_clCharge()))) {
        obj->Write();
      }

      // cluster charge vs. SNR
      dir_clChargeVsSNR->cd();
      TDirectory* dir_clChargeVsSNRSt = dir_clChargeVsSNR->mkdir(m_nameSensorTypes[i].c_str());
      dir_clChargeVsSNRSt->cd();
      TIter nextH_clChargeVsSNR(m_histoList_clChargeVsSNR[i]);
      while ((obj = dynamic_cast<TH2F*>(nextH_clChargeVsSNR()))) {
        obj->Write();
      }

      // cluster size
      dir_clSize->cd();
      TDirectory* dir_clSizeSt = dir_clSize->mkdir(m_nameSensorTypes[i].c_str());
      dir_clSizeSt->cd();
      TIter nextH_clSize(m_histoList_clSize[i]);
      while ((obj = dynamic_cast<TH1F*>(nextH_clSize()))) {
        obj->Write();
      }

      // cluster SNR
      dir_clSNR->cd();
      TDirectory* dir_clSNRSt = dir_clSNR->mkdir(m_nameSensorTypes[i].c_str());
      dir_clSNRSt->cd();
      TIter nextH_clSNR(m_histoList_clSNR[i]);
      while ((obj = dynamic_cast<TH1F*>(nextH_clSNR()))) {
        obj->Write();
      }
    }
    m_outputRootFile->Close();
  }

  delete m_outputRootFile;

} //terminate

TH1F* SVDChargeSharingAnalysisModule::createHistogram1D(const char* name, const char* title,
                                                        Int_t nbins, Double_t min, Double_t max,
                                                        const char* xtitle, const char* ytitle, TList* histoList)
{
  TH1F* h = new TH1F(name, title, nbins, min, max);
  h->GetXaxis()->SetTitle(xtitle);
  h->GetYaxis()->SetTitle(ytitle);
  if (histoList) {
    histoList->Add(h);
  }
  return h;
}

TH2F* SVDChargeSharingAnalysisModule::createHistogram2D(const char* name, const char* title,
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

TCanvas* SVDChargeSharingAnalysisModule::comparisonPlot(TH1F* h1, TH1F* h2, TH1F* h3)
{
  TCanvas* c = new TCanvas("c", "c");
  c->cd();

  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  gStyle->SetCanvasPreferGL(1);

  h1->SetLineColor(kRed);
  h1->SetFillColorAlpha(kRed, 0.35);

  h2->SetLineColor(kBlue);
  h2->SetFillColorAlpha(kBlue, 0.35);

  h3->SetLineColor(kGreen);
  h3->SetFillColorAlpha(kGreen, 0.35);

  h2->GetYaxis()->SetTitleOffset(1.4);
  h2->Draw();
  h1->Draw("][sames");
  h3->Draw("][sames");

  auto legend = new TLegend(0.7, 0.7, 0.9, 0.9);
  legend->SetTextAlign(22);
  legend->AddEntry(h1,  "clSize 1", "f");
  legend->AddEntry(h2,  "clSize 2", "f");
  legend->AddEntry(h3,  "clSize >= 3", "f");
  legend->Draw();

  return c;
}
