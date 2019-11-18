/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Filippo Dattola                                          *
 *                                                                        *
 * Module to study VXD hits from overlapping sensors of a same VXD layer. *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/VXDOverlaps/OverlapResidualsModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/core/Environment.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <genfit/TrackPoint.h>
#include <TVector3.h>
#include <TDirectory.h>
#include <math.h>
#include <iostream>

using namespace Belle2;
using namespace std;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(OverlapResiduals)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

OverlapResidualsModule::OverlapResidualsModule() : HistoModule()
{
  //Set module properties
  setDescription("The module studies consecutive hits in overlapping sensors of a same VXD layer, and the differences of their residuals, to monitor the detector alignment.");
  //Parameter to take only specific RecoTracks as input
  addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName, "StoreArray name of the input and output RecoTracks.",
           m_recoTracksStoreArrayName);
  //Parameter to produce TTrees storing global information on VXD overlaps
  addParam("ExpertLevel", m_ExpertLevel,
           "If set, enables the production of TTrees containing low level information on PXD and SVD overlaps", false);
}

void OverlapResidualsModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);
  recoTracks.isOptional();
  //Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}

void OverlapResidualsModule::defineHisto()
{
  //Create directory to store monitoring histograms
  TDirectory* oldDir = gDirectory;
  TDirectory* ResDir = oldDir->mkdir("Monitoring_VXDOverlaps");
  ResDir->cd();
  //Define histograms of residuals differences
  h_U_DeltaRes = new TH1F("h_U_DeltaRes", "Histrogram of residual difference #Delta res_{u} for overlapping hits", 100, -1000, 1000);
  h_U_DeltaRes->GetXaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_U_DeltaRes->GetYaxis()->SetTitle("counts");
  h_V_DeltaRes = new TH1F("h_V_DeltaRes", "Histrogram of residual difference #Delta res_{v} for overlapping hits", 100, -1000, 1000);
  h_V_DeltaRes->GetXaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_V_DeltaRes->GetYaxis()->SetTitle("counts");
  h_U_DeltaRes_PXD = new TH1F("h_U_DeltaRes_PXD", "Histrogram of residual difference #Delta res_{u} for overlapping PXD hits", 100,
                              -1000, 1000);
  h_U_DeltaRes_PXD->GetXaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_U_DeltaRes_PXD->GetYaxis()->SetTitle("counts");
  h_U_DeltaRes_PXD_Lyr1 = new TH1F("h_U_DeltaRes_PXD_Lyr1",
                                   "Layer1: histrogram of residual difference #Delta res_{u} for overlapping PXD hits", 100, -1000, 1000);
  h_U_DeltaRes_PXD_Lyr1->GetXaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_U_DeltaRes_PXD_Lyr1->GetYaxis()->SetTitle("counts");
  h_U_DeltaRes_PXD_Lyr2 = new TH1F("h_U_DeltaRes_PXD_Lyr2",
                                   "Layer 2: hstrogram of residual difference #Delta res_{u} for overlapping PXD hits", 100, -1000, 1000);
  h_U_DeltaRes_PXD_Lyr2->GetXaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_U_DeltaRes_PXD_Lyr2->GetYaxis()->SetTitle("counts");
  h_V_DeltaRes_PXD = new TH1F("h_V_DeltaRes_PXD", "Histrogram of residual difference #Delta res_{v} for overlapping PXD hits", 100,
                              -1000, 1000);
  h_V_DeltaRes_PXD->GetXaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_V_DeltaRes_PXD->GetYaxis()->SetTitle("counts");
  h_V_DeltaRes_PXD_Lyr1 = new TH1F("h_V_DeltaRes_PXD_Lyr1",
                                   "Layer 1: histrogram of residual difference #Delta res_{v} for overlapping PXD hits", 100, -1000, 1000);
  h_V_DeltaRes_PXD_Lyr1->GetXaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_V_DeltaRes_PXD_Lyr1->GetYaxis()->SetTitle("counts");
  h_V_DeltaRes_PXD_Lyr2 = new TH1F("h_V_DeltaRes_PXD_Lyr2",
                                   "Layer 2: histrogram of residual difference #Delta res_{v} for overlapping PXD hits", 100, -1000, 1000);
  h_V_DeltaRes_PXD_Lyr2->GetXaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_V_DeltaRes_PXD_Lyr2->GetYaxis()->SetTitle("counts");
  h_U_DeltaRes_SVD = new TH1F("h_U_DeltaRes_SVD", "Histrogram of residual difference #Delta res_{u} for overlapping SVD hits", 100,
                              -1000, 1000);
  h_U_DeltaRes_SVD->GetXaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_U_DeltaRes_SVD->GetYaxis()->SetTitle("counts");
  h_U_DeltaRes_SVD_Lyr3 = new TH1F("h_U_DeltaRes_SVD_Lyr3",
                                   "Layer 3: histrogram of residual difference #Delta res_{u} for overlapping SVD hits", 100, -1000, 1000);
  h_U_DeltaRes_SVD_Lyr3->GetXaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_U_DeltaRes_SVD_Lyr3->GetYaxis()->SetTitle("counts");
  h_U_DeltaRes_SVD_Lyr4 = new TH1F("h_U_DeltaRes_SVD_Lyr4",
                                   "Layer 4: histrogram of residual difference #Delta res_{u} for overlapping SVD hits", 100, -1000, 1000);
  h_U_DeltaRes_SVD_Lyr4->GetXaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_U_DeltaRes_SVD_Lyr4->GetYaxis()->SetTitle("counts");
  h_U_DeltaRes_SVD_Lyr5 = new TH1F("h_U_DeltaRes_SVD_Lyr5",
                                   "Layer 5: histrogram of residual difference #Delta res_{u} for overlapping SVD hits", 100, -1000, 1000);
  h_U_DeltaRes_SVD_Lyr5->GetXaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_U_DeltaRes_SVD_Lyr5->GetYaxis()->SetTitle("counts");
  h_U_DeltaRes_SVD_Lyr6 = new TH1F("h_U_DeltaRes_SVD_Lyr6",
                                   "Layer 6: histrogram of residual difference #Delta res_{u} for overlapping SVD hits", 100, -1000, 1000);
  h_U_DeltaRes_SVD_Lyr6->GetXaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_U_DeltaRes_SVD_Lyr6->GetYaxis()->SetTitle("counts");
  h_V_DeltaRes_SVD = new TH1F("h_V_DeltaRes_SVD", "Histrogram of residual difference #Delta res_{v} for overlapping SVD hits", 100,
                              -1000, 1000);
  h_V_DeltaRes_SVD->GetXaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_V_DeltaRes_SVD->GetYaxis()->SetTitle("counts");
  h_V_DeltaRes_SVD_Lyr3 = new TH1F("h_V_DeltaRes_SVD_Lyr3",
                                   "Layer 3: histrogram of residual difference #Delta res_{v} for overlapping SVD hits", 100, -1000, 1000);
  h_V_DeltaRes_SVD_Lyr3->GetXaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_V_DeltaRes_SVD_Lyr3->GetYaxis()->SetTitle("counts");
  h_V_DeltaRes_SVD_Lyr4 = new TH1F("h_V_DeltaRes_SVD_Lyr4",
                                   "Layer 4: histrogram of residual difference #Delta res_{v} for overlapping SVD hits", 100, -1000, 1000);
  h_V_DeltaRes_SVD_Lyr4->GetXaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_V_DeltaRes_SVD_Lyr4->GetYaxis()->SetTitle("counts");
  h_V_DeltaRes_SVD_Lyr5 = new TH1F("h_V_DeltaRes_SVD_Lyr5",
                                   "Layer 5: histrogram of residual difference #Delta res_{v} for overlapping SVD hits", 100, -1000, 1000);
  h_V_DeltaRes_SVD_Lyr5->GetXaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_V_DeltaRes_SVD_Lyr5->GetYaxis()->SetTitle("counts");
  h_V_DeltaRes_SVD_Lyr6 = new TH1F("h_V_DeltaRes_SVD_Lyr6",
                                   "Layer 6: histrogram of residual difference #Delta res_{v} for overlapping SVD hits", 100, -1000, 1000);
  h_V_DeltaRes_SVD_Lyr6->GetXaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_V_DeltaRes_SVD_Lyr6->GetYaxis()->SetTitle("counts");
  h_SVDstrips_Mult = new TH1F("h_SVDstrips_Mult", "SVD strips multipicity for SVD clusters in overlapping sensors", 15, 0.5, 15.5);
  h_SVDstrips_Mult->GetXaxis()->SetTitle("N. of SVD strips contributing to the cluster");
  h_SVDstrips_Mult->GetYaxis()->SetTitle("counts");
  //Define 2D histograms: difference of u-residuals vs phi of VXD overlaps for each layer (1 to 6)
  h_DeltaResUPhi_Lyr1 = new TH2F("h_DeltaResUPhi_Lyr1", "Layer 1: #Delta res_{u} vs #phi", 200, -3.4, 3.4, 100, -200, 200);
  h_DeltaResUPhi_Lyr1->GetXaxis()->SetTitle("#phi(rad)");
  h_DeltaResUPhi_Lyr1->GetYaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_DeltaResUPhi_Lyr2 = new TH2F("h_DeltaResUPhi_Lyr2", "Layer 2: #Delta res_{u} vs #phi", 200, -3.4, 3.4, 100, -200, 200);
  h_DeltaResUPhi_Lyr2->GetXaxis()->SetTitle("#phi(rad)");
  h_DeltaResUPhi_Lyr2->GetYaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_DeltaResUPhi_Lyr3 = new TH2F("h_DeltaResUPhi_Lyr3", "Layer 3: #Delta res_{u} vs #phi", 200, -3.4, 3.4, 100, -1000, 1000);
  h_DeltaResUPhi_Lyr3->GetXaxis()->SetTitle("#phi(rad)");
  h_DeltaResUPhi_Lyr3->GetYaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_DeltaResUPhi_Lyr4 = new TH2F("h_DeltaResUPhi_Lyr4", "Layer 4: #Delta res_{u} vs #phi", 200, -3.4, 3.4, 100, -1000, 1000);
  h_DeltaResUPhi_Lyr4->GetXaxis()->SetTitle("#phi(rad)");
  h_DeltaResUPhi_Lyr4->GetYaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_DeltaResUPhi_Lyr5 = new TH2F("h_DeltaResUPhi_Lyr5", "Layer 5: #Delta res_{u} vs #phi", 200, -3.4, 3.4, 100, -1000, 1000);
  h_DeltaResUPhi_Lyr5->GetXaxis()->SetTitle("#phi(rad)");
  h_DeltaResUPhi_Lyr5->GetYaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_DeltaResUPhi_Lyr6 = new TH2F("h_DeltaResUPhi_Lyr6", "Layer 6: #Delta res_{u} vs #phi", 200, -3.4, 3.4, 100, -1000, 1000);
  h_DeltaResUPhi_Lyr6->GetXaxis()->SetTitle("#phi(rad)");
  h_DeltaResUPhi_Lyr6->GetYaxis()->SetTitle("#Delta res_{u} (#mum)");
  //Define 2D histograms: difference of u-residuals vs z of VXD overlaps for each layer (1 to 6)
  h_DeltaResUz_Lyr1 = new TH2F("h_DeltaResUz_Lyr1", "Layer 1: #Delta res_{u} vs z", 100, -4, 8, 100, -200, 200);
  h_DeltaResUz_Lyr1->GetXaxis()->SetTitle("z (cm)");
  h_DeltaResUz_Lyr1->GetYaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_DeltaResUz_Lyr2 = new TH2F("h_DeltaResUz_Lyr2", "Layer 2: #Delta res_{u} vs z", 100, -10, 15, 100, -200, 200);
  h_DeltaResUz_Lyr2->GetXaxis()->SetTitle("z (cm)");
  h_DeltaResUz_Lyr2->GetYaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_DeltaResUz_Lyr3 = new TH2F("h_DeltaResUz_Lyr3", "Layer 3: #Delta res_{u} vs z", 250, -15, 20, 100, -1000, 1000);
  h_DeltaResUz_Lyr3->GetXaxis()->SetTitle("z (cm)");
  h_DeltaResUz_Lyr3->GetYaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_DeltaResUz_Lyr4 = new TH2F("h_DeltaResUz_Lyr4", "Layer 4: #Delta res_{u} vs z", 250, -20, 25, 100, -1000, 1000);
  h_DeltaResUz_Lyr4->GetXaxis()->SetTitle("z (cm)");
  h_DeltaResUz_Lyr4->GetYaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_DeltaResUz_Lyr5 = new TH2F("h_DeltaResUz_Lyr5", "Layer 5: #Delta res_{u} vs z", 250, -25, 35, 100, -1000, 1000);
  h_DeltaResUz_Lyr5->GetXaxis()->SetTitle("z (cm)");
  h_DeltaResUz_Lyr5->GetYaxis()->SetTitle("#Delta res_{u} (#mum)");
  h_DeltaResUz_Lyr6 = new TH2F("h_DeltaResUz_Lyr6", "Layer 6: #Delta res_{u} vs z", 250, -30, 45, 100, -1000, 1000);
  h_DeltaResUz_Lyr6->GetXaxis()->SetTitle("z (cm)");
  h_DeltaResUz_Lyr6->GetYaxis()->SetTitle("#Delta res_{u} (#mum)");
  //Define 2D histograms: difference of u-residuals vs phi of VXD overlaps for each layer (1 to 6)
  h_DeltaResVPhi_Lyr1 = new TH2F("h_DeltaResVPhi_Lyr1", "Layer 1: #Delta res_{v} vs #phi", 200, -3.4, 3.4, 100, -200, 200);
  h_DeltaResVPhi_Lyr1->GetXaxis()->SetTitle("#phi(rad)");
  h_DeltaResVPhi_Lyr1->GetYaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_DeltaResVPhi_Lyr2 = new TH2F("h_DeltaResVPhi_Lyr2", "Layer 2: #Delta res_{v} vs #phi", 200, -3.4, 3.4, 100, -200, 200);
  h_DeltaResVPhi_Lyr2->GetXaxis()->SetTitle("#phi(rad)");
  h_DeltaResVPhi_Lyr2->GetYaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_DeltaResVPhi_Lyr3 = new TH2F("h_DeltaResVPhi_Lyr3", "Layer 3: #Delta res_{v} vs #phi", 200, -3.4, 3.4, 100, -1000, 1000);
  h_DeltaResVPhi_Lyr3->GetXaxis()->SetTitle("#phi(rad)");
  h_DeltaResVPhi_Lyr3->GetYaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_DeltaResVPhi_Lyr4 = new TH2F("h_DeltaResVPhi_Lyr4", "Layer 4: #Delta res_{v} vs #phi", 200, -3.4, 3.4, 100, -1000, 1000);
  h_DeltaResVPhi_Lyr4->GetXaxis()->SetTitle("#phi(rad)");
  h_DeltaResVPhi_Lyr4->GetYaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_DeltaResVPhi_Lyr5 = new TH2F("h_DeltaResVPhi_Lyr5", "Layer 5: #Delta res_{v} vs #phi", 200, -3.4, 3.4, 100, -1000, 1000);
  h_DeltaResVPhi_Lyr5->GetXaxis()->SetTitle("#phi(rad)");
  h_DeltaResVPhi_Lyr5->GetYaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_DeltaResVPhi_Lyr6 = new TH2F("h_DeltaResVPhi_Lyr6", "Layer 6: #Delta res_{v} vs #phi", 200, -3.4, 3.4, 100, -1000, 1000);
  h_DeltaResVPhi_Lyr6->GetXaxis()->SetTitle("#phi(rad)");
  h_DeltaResVPhi_Lyr6->GetYaxis()->SetTitle("#Delta res_{v} (#mum)");
  //Define 2D histograms: difference of v-residuals vs z of VXD overlaps for each layer (1 to 6)
  h_DeltaResVz_Lyr1 = new TH2F("h_DeltaResVz_Lyr1", "Layer 1: #Delta res_{v} vs z", 100, -4, 8, 100, -200, 200);
  h_DeltaResVz_Lyr1->GetXaxis()->SetTitle("z (cm)");
  h_DeltaResVz_Lyr1->GetYaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_DeltaResVz_Lyr2 = new TH2F("h_DeltaResVz_Lyr2", "Layer 2: #Delta res_{v} vs z", 100, -10, 15, 100, -200, 200);
  h_DeltaResVz_Lyr2->GetXaxis()->SetTitle("z (cm)");
  h_DeltaResVz_Lyr2->GetYaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_DeltaResVz_Lyr3 = new TH2F("h_DeltaResVz_Lyr3", "Layer 3: #Delta res_{v} vs z", 250, -15, 20, 100, -1000, 1000);
  h_DeltaResVz_Lyr3->GetXaxis()->SetTitle("z (cm)");
  h_DeltaResVz_Lyr3->GetYaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_DeltaResVz_Lyr4 = new TH2F("h_DeltaResVz_Lyr4", "Layer 4: #Delta res_{v} vs z", 250, -20, 25, 100, -1000, 1000);
  h_DeltaResVz_Lyr4->GetXaxis()->SetTitle("z (cm)");
  h_DeltaResVz_Lyr4->GetYaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_DeltaResVz_Lyr5 = new TH2F("h_DeltaResVz_Lyr5", "Layer 5: #Delta res_{v} vs z", 250, -25, 35, 100, -1000, 1000);
  h_DeltaResVz_Lyr5->GetXaxis()->SetTitle("z (cm)");
  h_DeltaResVz_Lyr5->GetYaxis()->SetTitle("#Delta res_{v} (#mum)");
  h_DeltaResVz_Lyr6 = new TH2F("h_DeltaResVz_Lyr6", "Layer 6: #Delta res_{v} vs z", 250, -30, 45, 100, -1000, 1000);
  h_DeltaResVz_Lyr6->GetXaxis()->SetTitle("z (cm)");
  h_DeltaResVz_Lyr6->GetYaxis()->SetTitle("#Delta res_{v} (#mum)");
  //Restricting to SVD clusters sizes
  for (int i = 1; i < 5; i++) {
    //The name is the product of cluster sizes for 2 consecutive hits (maximum size considered is 2)
    TString h_name_U = "h_U_Cl1Cl2_" + std::to_string(i);
    TString h_name_V = "h_V_Cl1Cl2_" + std::to_string(i);
    TString title_U = "#Delta res_{u}: SVDClusterSize_1 x SVDClusterSize_2 = " + std::to_string(i);
    TString title_V = "#Delta res_{v}: SVDClusterSize_1 x SVDClusterSize_2 = " + std::to_string(i);
    h_U_Cl1Cl2_DeltaRes[i] = new TH1F(h_name_U, title_U, 100, -1000, 1000);
    h_U_Cl1Cl2_DeltaRes[i]->GetXaxis()->SetTitle("#Delta res_{u} (#mum)");
    h_U_Cl1Cl2_DeltaRes[i]->GetYaxis()->SetTitle("counts");
    h_V_Cl1Cl2_DeltaRes[i] = new TH1F(h_name_V, title_V, 100, -1000, 1000);
    h_V_Cl1Cl2_DeltaRes[i]->GetXaxis()->SetTitle("#Delta res_{v} (#mum)");
    h_V_Cl1Cl2_DeltaRes[i]->GetYaxis()->SetTitle("counts");
  }

  //Special case of ExpertLevel option enabled
  if (m_ExpertLevel) {
    //Create directory to store PXD and SVD hitmaps for overlapping hits
    TDirectory* HMDir = oldDir->mkdir("HitMaps_VXDOverlaps");
    HMDir->cd();
    //Define 2D sensor hit-maps for reconstructed hits
    for (int i = 1; i <= 5; i++) {
      for (int j = 1; j <= 16; j++) {
        TString h_name = "h_6" + std::to_string(j) + std::to_string(i);
        TString title = "Layer:Ladder:Sensor = 6:" + std::to_string(j) + ":" + std::to_string(i);
        h_Lyr6[j][i] = new TH2F(h_name, title, 100, -2.88, 2.88, 100, -6.14, 6.14);
        h_Lyr6[j][i]->GetXaxis()->SetTitle("u (cm)");
        h_Lyr6[j][i]->GetYaxis()->SetTitle("v (cm)");
      }
    }
    for (int i = 1; i <= 4; i++) {
      for (int j = 1; j <= 12; j++) {
        TString h_name = "h_5" + std::to_string(j) + std::to_string(i);
        TString title = "Layer:Ladder:Sensor = 5:" + std::to_string(j) + ":" + std::to_string(i);
        h_Lyr5[j][i] = new TH2F(h_name, title, 100, -2.88, 2.88, 100, -6.14, 6.14);
        h_Lyr5[j][i]->GetXaxis()->SetTitle("u (cm)");
        h_Lyr5[j][i]->GetYaxis()->SetTitle("v (cm)");
      }
    }
    for (int i = 1; i <= 3; i++) {
      for (int j = 1; j <= 10; j++) {
        TString h_name = "h_4" + std::to_string(j) + std::to_string(i);
        TString title = "Layer:Ladder:Sensor = 4:" + std::to_string(j) + ":" + std::to_string(i);
        h_Lyr4[j][i] = new TH2F(h_name, title, 100, -2.88, 2.88, 100, -6.14, 6.14);
        h_Lyr4[j][i]->GetXaxis()->SetTitle("u (cm)");
        h_Lyr4[j][i]->GetYaxis()->SetTitle("v (cm)");
      }
    }
    for (int i = 1; i <= 2; i++) {
      for (int j = 1; j <= 7; j++) {
        TString h_name = "h_3" + std::to_string(j) + std::to_string(i);
        TString title = "Layer:Ladder:Sensor = 3:" + std::to_string(j) + ":" + std::to_string(i);
        h_Lyr3[j][i] = new TH2F(h_name, title, 100, -1.92, 1.92, 100, -6.14, 6.14);
        h_Lyr3[j][i]->GetXaxis()->SetTitle("u (cm)");
        h_Lyr3[j][i]->GetYaxis()->SetTitle("v (cm)");
      }
    }
    for (int i = 1; i <= 2; i++) {
      for (int j = 1; j <= 12; j++) {
        TString h_name = "h_2" + std::to_string(j) + std::to_string(i);
        TString title = "Layer:Ladder:Sensor = 2:" + std::to_string(j) + ":" + std::to_string(i);
        h_Lyr2[j][i] = new TH2F(h_name, title, 100, -0.625, 0.625, 100, -3.072, 3.072);
        h_Lyr2[j][i]->GetXaxis()->SetTitle("u (cm)");
        h_Lyr2[j][i]->GetYaxis()->SetTitle("v (cm)");
      }
    }
    for (int i = 1; i <= 2; i++) {
      for (int j = 1; j <= 8; j++) {
        TString h_name = "h_1" + std::to_string(j) + std::to_string(i);
        TString title = "Layer:Ladder:Sensor = 1:" + std::to_string(j) + ":" + std::to_string(i);
        h_Lyr1[j][i] = new TH2F(h_name, title, 100, -0.625, 0.625, 100, -2.24, 2.24);
        h_Lyr1[j][i]->GetXaxis()->SetTitle("u (cm)");
        h_Lyr1[j][i]->GetYaxis()->SetTitle("v (cm)");
      }
    }
    //Create directory to store PXD and SVD trees
    TDirectory* TreeDir = oldDir->mkdir("Trees_VXDOverlaps");
    TreeDir->cd();
    //Tree for PXD
    t_PXD = new TTree("t_PXD", "Tree for PXD overlaps");
    t_PXD->Branch("deltaResU_PXD", &deltaResU_PXD, "deltaResU_PXD/F");
    t_PXD->Branch("intResU_PXD", &intResU_PXD, "intResU_PXD/F");
    t_PXD->Branch("intResV_PXD", &intResV_PXD, "intResV_PXD/F");
    t_PXD->Branch("intU_PXD", &intU_PXD, "intU_PXD/F");
    t_PXD->Branch("intV_PXD", &intV_PXD, "intV_PXD/F");
    t_PXD->Branch("intPhi_PXD", &intPhi_PXD, "intPhi_PXD/F");
    t_PXD->Branch("intZ_PXD", &intZ_PXD, "intZ_PXD/F");
    t_PXD->Branch("intLayer_PXD", &intLayer_PXD, "intLayer_PXD/i");
    t_PXD->Branch("intLadder_PXD", &intLadder_PXD, "intLadder_PXD/i");
    t_PXD->Branch("intSensor_PXD", &intSensor_PXD, "intSensor_PXD/i");
    t_PXD->Branch("extResU_PXD", &extResU_PXD, "extResU_PXD/F");
    t_PXD->Branch("extResV_PXD", &extResV_PXD, "extResV_PXD/F");
    t_PXD->Branch("extU_PXD", &extU_PXD, "extU_PXD/F");
    t_PXD->Branch("extV_PXD", &extV_PXD, "extV_PXD/F");
    t_PXD->Branch("extPhi_PXD", &extPhi_PXD, "extPhi_PXD/F");
    t_PXD->Branch("extZ_PXD", &extZ_PXD, "extZ_PXD/F");
    t_PXD->Branch("extLayer_PXD", &extLayer_PXD, "extLayer_PXD/i");
    t_PXD->Branch("extLadder_PXD", &extLadder_PXD, "extLadder_PXD/i");
    t_PXD->Branch("extSensor_PXD", &extSensor_PXD, "extSensor_PXD/i");
    //Tree for SVD u overlapping clusters
    t_SVD_U = new TTree("t_SVD_U", "Tree for SVD u-overlaps");
    t_SVD_U->Branch("deltaRes", &deltaRes_SVD_U, "deltaResU/F");
    t_SVD_U->Branch("intRes", &intRes_SVD_U, "intRes/F");
    t_SVD_U->Branch("intClPos", &intClPos_SVD_U, "intClPos/F");
    t_SVD_U->Branch("intClPosErr", &intClPosErr_SVD_U, "intClPosErr/F");
    t_SVD_U->Branch("intTruePos", &intTruePos_SVD_U, "intTruePos/F");
    t_SVD_U->Branch("intClPhi", &intClPhi_SVD_U, "intClPhi/F");
    t_SVD_U->Branch("intClZ", &intClZ_SVD_U, "intClZ/F");
    t_SVD_U->Branch("intTrkPos", &intTrkPos_SVD_U, "intTrkPos/F");
    t_SVD_U->Branch("intTrkPosErr", &intTrkPosErr_SVD_U, "intTrkPosErr/F");
    t_SVD_U->Branch("intTrkQoP", &intTrkQoP_SVD_U, "intTrkQoP/F");
    t_SVD_U->Branch("intTrkPrime", &intTrkPrime_SVD_U, "intTrkPrime/F");
    t_SVD_U->Branch("intLayer", &intLayer_SVD_U, "intLayer/i");
    t_SVD_U->Branch("intLadder", &intLadder_SVD_U, "intLadder/i");
    t_SVD_U->Branch("intSensor", &intSensor_SVD_U, "intSensor/i");
    t_SVD_U->Branch("intSize", &intSize_SVD_U, "intSize/i");
    t_SVD_U->Branch("deltaRes", &deltaRes_SVD_U, "deltaResU/F");
    t_SVD_U->Branch("extRes", &extRes_SVD_U, "extRes/F");
    t_SVD_U->Branch("extClPos", &extClPos_SVD_U, "extClPos/F");
    t_SVD_U->Branch("extClPosErr", &extClPosErr_SVD_U, "extClPosErr/F");
    t_SVD_U->Branch("extTruePos", &extTruePos_SVD_U, "extTruePos/F");
    t_SVD_U->Branch("extClPhi", &extClPhi_SVD_U, "extClPhi/F");
    t_SVD_U->Branch("extClZ", &extClZ_SVD_U, "extClZ/F");
    t_SVD_U->Branch("extTrkPos", &extTrkPos_SVD_U, "extTrkPos/F");
    t_SVD_U->Branch("extTrkPosErr", &extTrkPosErr_SVD_U, "extTrkPosErr/F");
    t_SVD_U->Branch("extTrkQoP", &extTrkQoP_SVD_U, "extTrkQoP/F");
    t_SVD_U->Branch("extTrkPrime", &extTrkPrime_SVD_U, "extTrkPrime/F");
    t_SVD_U->Branch("extLayer", &extLayer_SVD_U, "extLayer/i");
    t_SVD_U->Branch("extLadder", &extLadder_SVD_U, "extLadder/i");
    t_SVD_U->Branch("extSensor", &extSensor_SVD_U, "extSensor/i");
    t_SVD_U->Branch("extSize", &extSize_SVD_U, "extSize/i");
    //Tree for SVD v overlapping clusters
    t_SVD_V = new TTree("t_SVD_V", "Tree for SVD v-overlaps");
    t_SVD_V->Branch("deltaRes", &deltaRes_SVD_V, "deltaResV/F");
    t_SVD_V->Branch("intRes", &intRes_SVD_V, "intRes/F");
    t_SVD_V->Branch("intClPos", &intClPos_SVD_V, "intClPos/F");
    t_SVD_V->Branch("intClPosErr", &intClPosErr_SVD_V, "intClPosErr/F");
    t_SVD_V->Branch("intTruePos", &intTruePos_SVD_V, "intTruePos/F");
    t_SVD_V->Branch("intClPhi", &intClPhi_SVD_V, "intClPhi/F");
    t_SVD_V->Branch("intClZ", &intClZ_SVD_V, "intClZ/F");
    t_SVD_V->Branch("intTrkPos", &intTrkPos_SVD_V, "intTrkPos/F");
    t_SVD_V->Branch("intTrkPosErr", &intTrkPosErr_SVD_V, "intTrkPosErr/F");
    t_SVD_V->Branch("intTrkQoP", &intTrkQoP_SVD_V, "intTrkQoP/F");
    t_SVD_V->Branch("intTrkPrime", &intTrkPrime_SVD_V, "intTrkPrime/F");
    t_SVD_V->Branch("intLayer", &intLayer_SVD_V, "intLayer/i");
    t_SVD_V->Branch("intLadder", &intLadder_SVD_V, "intLadder/i");
    t_SVD_V->Branch("intSensor", &intSensor_SVD_V, "intSensor/i");
    t_SVD_V->Branch("intSize", &intSize_SVD_V, "intSize/i");
    t_SVD_V->Branch("deltaRes", &deltaRes_SVD_V, "deltaResV/F");
    t_SVD_V->Branch("extRes", &extRes_SVD_V, "extRes/F");
    t_SVD_V->Branch("extClPos", &extClPos_SVD_V, "extClPos/F");
    t_SVD_V->Branch("extClPosErr", &extClPosErr_SVD_V, "extClPosErr/F");
    t_SVD_V->Branch("extTruePos", &extTruePos_SVD_V, "extTruePos/F");
    t_SVD_V->Branch("extClPhi", &extClPhi_SVD_V, "extClPhi/F");
    t_SVD_V->Branch("extClZ", &extClZ_SVD_V, "extClZ/F");
    t_SVD_V->Branch("extTrkPos", &extTrkPos_SVD_V, "extTrkPos/F");
    t_SVD_V->Branch("extTrkPosErr", &extTrkPosErr_SVD_V, "extTrkPosErr/F");
    t_SVD_V->Branch("extTrkQoP", &extTrkQoP_SVD_V, "extTrkQoP/F");
    t_SVD_V->Branch("extTrkPrime", &extTrkPrime_SVD_V, "extTrkPrime/F");
    t_SVD_V->Branch("extLayer", &extLayer_SVD_V, "extLayer/i");
    t_SVD_V->Branch("extLadder", &extLadder_SVD_V, "extLadder/i");
    t_SVD_V->Branch("extSensor", &extSensor_SVD_V, "extSensor/i");
    t_SVD_V->Branch("extSize", &extSize_SVD_V, "extSize/i");
  }
  //Go back to the default output directory
  oldDir->cd();
}

void OverlapResidualsModule::event()
{
  bool isMC = Environment::Instance().isMC();

  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);
  for (const auto& trk : recoTracks) {
    if (! trk.wasFitSuccessful()) {
      continue;
    }
    const vector<PXDCluster* > pxdClusters = trk.getPXDHitList();
    const vector<SVDCluster* > svdClusters = trk.getSVDHitList();
    B2DEBUG(40, "FITTED TRACK:   NUMBER OF PXD HITS = " << pxdClusters.size() << "    NUMBER OF SVD HITS = " << svdClusters.size());
    //LOOKING FOR 2 CONSECUTIVE PXD HITS IN OVERLAPPING MODULES OF A SAME LAYER
    for (unsigned int i = 0; i < pxdClusters.size(); i++) {
      const PXDCluster* pxd_1 = pxdClusters[i];
      const RecoHitInformation* infoPXD_1 = trk.getRecoHitInformation(pxd_1);
      if (!infoPXD_1) {
        continue;
      }
      const auto* hitTrackPoint_1 = trk.getCreatedTrackPoint(infoPXD_1);
      const auto* fittedResult_1 = hitTrackPoint_1->getFitterInfo();
      if (!fittedResult_1) {
        continue;
      }
      const VxdID pxd_id_1 = pxd_1->getSensorID();
      const unsigned short pxd_Layer_1 = pxd_id_1.getLayerNumber();
      const unsigned short pxd_Ladder_1 = pxd_id_1.getLadderNumber();
      const unsigned short pxd_Sensor_1 = pxd_id_1.getSensorNumber();
      for (unsigned int l = i + 1; l < pxdClusters.size(); l++) {
        const PXDCluster* pxd_2 = pxdClusters[l];
        const RecoHitInformation* infoPXD_2 = trk.getRecoHitInformation(pxd_2);
        if (!infoPXD_2) {
          continue;
        }
        const auto* hitTrackPoint_2 = trk.getCreatedTrackPoint(infoPXD_2);
        const auto* fittedResult_2 = hitTrackPoint_2->getFitterInfo();
        if (!fittedResult_2) {
          continue;
        }
        const VxdID pxd_id_2 = pxd_2->getSensorID();
        const unsigned short pxd_Layer_2 = pxd_id_2.getLayerNumber();
        const unsigned short pxd_Ladder_2 = pxd_id_2.getLadderNumber();
        const unsigned short pxd_Sensor_2 = pxd_id_2.getSensorNumber();
        if (pxd_Layer_1 == pxd_Layer_2 && ((pxd_Ladder_2 - pxd_Ladder_1) == -1.0 || (pxd_Ladder_2 - pxd_Ladder_1) == 7.0
                                           || (pxd_Ladder_2 - pxd_Ladder_1) == 11.0)) {
          B2DEBUG(40, " ============= 2 hits in a PXD overlap ============= ");
          const TVectorD resUnBias_PXD_1 = fittedResult_1->getResidual(0, false).getState();
          const TVectorD resUnBias_PXD_2 = fittedResult_2->getResidual(0, false).getState();
          const float res_U_1 = resUnBias_PXD_1.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
          const float res_V_1 = resUnBias_PXD_1.GetMatrixArray()[1] * Unit::convertValueToUnit(1.0, "um");
          const double res_U_2 = resUnBias_PXD_2.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
          const double res_V_2 = resUnBias_PXD_2.GetMatrixArray()[1] * Unit::convertValueToUnit(1.0, "um");
          const float over_U_PXD = res_U_2 - res_U_1;
          const float over_V_PXD = res_V_2 - res_V_1;
          const TVector3 pxdLocal_1(pxd_1->getU(), pxd_1->getV(), 0.);
          const TVector3 pxdLocal_2(pxd_2->getU(), pxd_2->getV(), 0.);
          const VXD::SensorInfoBase& pxdSensor_1 = geo.get(pxd_id_1);
          const VXD::SensorInfoBase& pxdSensor_2 = geo.get(pxd_id_2);
          const TVector3& pxdGlobal_1 = pxdSensor_1.pointToGlobal(pxdLocal_1);
          const TVector3& pxdGlobal_2 = pxdSensor_2.pointToGlobal(pxdLocal_2);
          double pxdPhi_1 = atan2(pxdGlobal_1(1), pxdGlobal_1(0));
          double pxdPhi_2 = atan2(pxdGlobal_2(1), pxdGlobal_2(0));
          double pxdZ_1 = pxdGlobal_1(2);
          double pxdZ_2 = pxdGlobal_2(2);
          B2DEBUG(40, "PXD: difference of residuals " << over_U_PXD << "   " << over_V_PXD);
          //Fill PXD tree for overlaps if required by the user
          if (m_ExpertLevel) {
            deltaResU_PXD = over_U_PXD;
            intResU_PXD = res_U_1;
            intResV_PXD = res_V_1;
            intU_PXD = pxd_1->getU();
            intV_PXD = pxd_1->getV();
            intPhi_PXD = pxdPhi_1;
            intZ_PXD = pxdZ_1;
            intLayer_PXD = pxd_Layer_1;
            intLadder_PXD = pxd_Ladder_1;
            intSensor_PXD = pxd_Sensor_1;
            extResU_PXD = res_U_2;
            extResV_PXD = res_V_2;
            extU_PXD = pxd_2->getU();
            extV_PXD = pxd_2->getV();
            extPhi_PXD = pxdPhi_2;
            extZ_PXD = pxdZ_2;
            extLayer_PXD = pxd_Layer_2;
            extLadder_PXD = pxd_Ladder_2;
            extSensor_PXD = pxd_Sensor_2;
            t_PXD->Fill();
          }
          //Fill histograms of residuals differences with PXD clusters
          h_U_DeltaRes->Fill(over_U_PXD);
          h_V_DeltaRes->Fill(over_V_PXD);
          h_U_DeltaRes_PXD->Fill(over_U_PXD);
          h_V_DeltaRes_PXD->Fill(over_V_PXD);
          //Fill sensor hit-maps and 2D histograms with PXD clusters
          if (pxd_Layer_1 == 1 && pxd_Layer_2 == 1) {
            h_Lyr1[pxd_Ladder_1][pxd_Sensor_1]->Fill(pxd_1->getU(), pxd_1->getV());
            h_Lyr1[pxd_Ladder_2][pxd_Sensor_2]->Fill(pxd_2->getU(), pxd_2->getV());
            h_U_DeltaRes_PXD_Lyr1->Fill(over_U_PXD);
            h_V_DeltaRes_PXD_Lyr1->Fill(over_V_PXD);
            h_DeltaResUPhi_Lyr1->Fill(pxdPhi_1, over_U_PXD);
            h_DeltaResUPhi_Lyr1->Fill(pxdPhi_2, over_U_PXD);
            h_DeltaResVPhi_Lyr1->Fill(pxdPhi_1, over_V_PXD);
            h_DeltaResVPhi_Lyr1->Fill(pxdPhi_2, over_V_PXD);
            h_DeltaResVz_Lyr1->Fill(pxdZ_1, over_V_PXD);
            h_DeltaResVz_Lyr1->Fill(pxdZ_2, over_V_PXD);
            h_DeltaResUz_Lyr1->Fill(pxdZ_1, over_U_PXD);
            h_DeltaResUz_Lyr1->Fill(pxdZ_2, over_U_PXD);
          }
          if (pxd_Layer_1 == 2 && pxd_Layer_2 == 2) {
            h_Lyr2[pxd_Ladder_1][pxd_Sensor_1]->Fill(pxd_1->getU(), pxd_1->getV());
            h_Lyr2[pxd_Ladder_2][pxd_Sensor_2]->Fill(pxd_2->getU(), pxd_2->getV());
            h_U_DeltaRes_PXD_Lyr2->Fill(over_U_PXD);
            h_V_DeltaRes_PXD_Lyr2->Fill(over_V_PXD);
            h_DeltaResUPhi_Lyr2->Fill(pxdPhi_1, over_U_PXD);
            h_DeltaResUPhi_Lyr2->Fill(pxdPhi_2, over_U_PXD);
            h_DeltaResVPhi_Lyr2->Fill(pxdPhi_1, over_V_PXD);
            h_DeltaResVPhi_Lyr2->Fill(pxdPhi_2, over_V_PXD);
            h_DeltaResVz_Lyr2->Fill(pxdZ_1, over_V_PXD);
            h_DeltaResVz_Lyr2->Fill(pxdZ_2, over_V_PXD);
            h_DeltaResUz_Lyr2->Fill(pxdZ_1, over_U_PXD);
            h_DeltaResUz_Lyr2->Fill(pxdZ_2, over_U_PXD);
          }
        }
      }

    }

    //LOOKING FOR 2 CONSECUTIVE SVD HITS IN OVERLAPPING MODULES OF A SAME LAYER
    for (unsigned int i = 0; i < svdClusters.size(); i++) {
      const SVDCluster* svd_1 = svdClusters[i];

      //get true hits, used only if isMC
      RelationVector<SVDTrueHit> trueHit_1 = DataStore::getRelationsWithObj<SVDTrueHit>(svd_1);

      const RecoHitInformation* infoSVD_1 = trk.getRecoHitInformation(svd_1);
      if (!infoSVD_1) {
        continue;
      }
      const auto* hitTrackPoint_1 = trk.getCreatedTrackPoint(infoSVD_1);
      const auto* fittedResult_1 = hitTrackPoint_1->getFitterInfo();
      if (!fittedResult_1) {
        continue;
      }
      const VxdID svd_id_1 = svd_1->getSensorID();
      const unsigned short svd_Layer_1 = svd_id_1.getLayerNumber();
      const unsigned short svd_Ladder_1 = svd_id_1.getLadderNumber();
      const unsigned short svd_Sensor_1 = svd_id_1.getSensorNumber();
      for (unsigned int l = i + 1; l < svdClusters.size(); l++) {
        const SVDCluster* svd_2 = svdClusters[l];

        //get true hits, used only if isMC
        RelationVector<SVDTrueHit> trueHit_2 = DataStore::getRelationsWithObj<SVDTrueHit>(svd_2);

        const RecoHitInformation* infoSVD_2 = trk.getRecoHitInformation(svd_2);
        if (!infoSVD_2) {
          continue;
        }
        const auto* hitTrackPoint_2 = trk.getCreatedTrackPoint(infoSVD_2);
        const auto* fittedResult_2 = hitTrackPoint_2->getFitterInfo();
        if (!fittedResult_2) {
          continue;
        }
        const VxdID svd_id_2 = svd_2->getSensorID();
        const unsigned short svd_Layer_2 = svd_id_2.getLayerNumber();
        const unsigned short svd_Ladder_2 = svd_id_2.getLadderNumber();
        const unsigned short svd_Sensor_2 = svd_id_2.getSensorNumber();
        if (svd_Layer_1 == svd_Layer_2 && ((svd_Ladder_2 - svd_Ladder_1) == 1.0 || (svd_Ladder_2 - svd_Ladder_1) == -6.0
                                           || (svd_Ladder_2 - svd_Ladder_1) == -9.0 || (svd_Ladder_2 - svd_Ladder_1) == -11.0 || (svd_Ladder_2 - svd_Ladder_1) == -15.0)) {
          B2DEBUG(40, " ============= 2 hits in a SVD overlap ============= ");
          const TVectorD resUnBias_SVD_1 =  fittedResult_1->getResidual(0, false).getState();
          const TVectorD resUnBias_SVD_2 =  fittedResult_2->getResidual(0, false).getState();
          genfit::MeasuredStateOnPlane state_1 = trk.getMeasuredStateOnPlaneFromRecoHit(infoSVD_1);
          const TVectorD& svd_predIntersect_1 = state_1.getState();
          const TMatrixDSym& covMatrix_1 = state_1.getCov();
          genfit::MeasuredStateOnPlane state_2 = trk.getMeasuredStateOnPlaneFromRecoHit(infoSVD_2);
          const TVectorD& svd_predIntersect_2 = state_2.getState();
          const TMatrixDSym& covMatrix_2 = state_2.getCov();
          //Restricting to consecutive SVD u-clusters
          if (svd_1->isUCluster() == true && svd_2->isUCluster() == true) {
            const int strips_1 = svd_1->getSize();
            h_SVDstrips_Mult->Fill(strips_1);
            const int strips_2 = svd_2->getSize();
            h_SVDstrips_Mult->Fill(strips_2);
            const double res_U_1 = resUnBias_SVD_1.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
            const double res_U_2 = resUnBias_SVD_2.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
            const float over_U_SVD = res_U_2 - res_U_1;
            const TVector3 svdLocal_1(svd_1->getPosition(), svd_predIntersect_1[4], 0.);
            const TVector3 svdLocal_2(svd_2->getPosition(), svd_predIntersect_2[4], 0.);
            const VXD::SensorInfoBase& svdSensor_1 = geo.get(svd_id_1);
            const VXD::SensorInfoBase& svdSensor_2 = geo.get(svd_id_2);
            const TVector3& svdGlobal_1 = svdSensor_1.pointToGlobal(svdLocal_1);
            const TVector3& svdGlobal_2 = svdSensor_2.pointToGlobal(svdLocal_2);
            double svdPhi_1 = atan2(svdGlobal_1(1), svdGlobal_1(0));
            double svdPhi_2 = atan2(svdGlobal_2(1), svdGlobal_2(0));
            double svdZ_1 = svdGlobal_1(2);
            double svdZ_2 = svdGlobal_2(2);
            B2DEBUG(40, "SVD: difference of u-residuals =========> " << over_U_SVD);
            //Fill SVD tree for u-overlaps if required by the user
            if (m_ExpertLevel) {
              deltaRes_SVD_U = over_U_SVD;
              intRes_SVD_U = res_U_1;
              intClPos_SVD_U = svd_1->getPosition();
              intClPosErr_SVD_U = svd_1->getPositionSigma();
              if (isMC && trueHit_1.size() > 0)
                intTruePos_SVD_U = trueHit_1[0]->getU();
              else
                intTruePos_SVD_U = -99;
              intClPhi_SVD_U = svdPhi_1;
              intClZ_SVD_U = svdZ_1;
              intTrkPos_SVD_U = svd_predIntersect_1[3];
              intTrkPosErr_SVD_U = sqrt(covMatrix_1[3][3]);
              intTrkQoP_SVD_U = svd_predIntersect_1[0];
              intTrkPrime_SVD_U = svd_predIntersect_1[1];
              intLayer_SVD_U = svd_Layer_1;
              intLadder_SVD_U = svd_Ladder_1;
              intSensor_SVD_U = svd_Sensor_1;
              intSize_SVD_U = strips_1;
              extRes_SVD_U = res_U_2;
              extClPos_SVD_U = svd_2->getPosition();
              extClPosErr_SVD_U = svd_2->getPositionSigma();
              if (isMC && trueHit_2.size() > 0)
                extTruePos_SVD_U = trueHit_2[0]->getU();
              else
                extTruePos_SVD_U = -99;
              extClPhi_SVD_U = svdPhi_2;
              extClZ_SVD_U = svdZ_2;
              extTrkPos_SVD_U = svd_predIntersect_2[3];
              extTrkPosErr_SVD_U = sqrt(covMatrix_2[3][3]);
              extTrkQoP_SVD_U = svd_predIntersect_2[0];
              extTrkPrime_SVD_U = svd_predIntersect_2[1];
              extLayer_SVD_U = svd_Layer_2;
              extLadder_SVD_U = svd_Ladder_2;
              extSensor_SVD_U = svd_Sensor_2;
              extSize_SVD_U = strips_2;
              t_SVD_U->Fill();
            }
            //Fill histograms of residuals differences with SVD u clusters
            h_U_DeltaRes->Fill(over_U_SVD);
            h_U_DeltaRes_SVD->Fill(over_U_SVD);
            if (svd_1->getSize() < 3 && svd_2->getSize() < 3) { //Consider only clusters with 3 or less strips involved
              h_U_Cl1Cl2_DeltaRes[svd_1->getSize() * svd_2->getSize()]->Fill(over_U_SVD);
            }
            //Fill sensor hit-maps and 2D histograms with SVD u clusters
            if (svd_Layer_1 == 3 && svd_Layer_2 == 3) {
              h_Lyr3[svd_Ladder_1][svd_Sensor_1]->Fill(svd_1->getPosition(), 0.);
              h_Lyr3[svd_Ladder_2][svd_Sensor_2]->Fill(svd_2->getPosition(), 0.);
              h_U_DeltaRes_SVD_Lyr3->Fill(over_U_SVD);
              h_DeltaResUPhi_Lyr3->Fill(svdPhi_1, over_U_SVD);
              h_DeltaResUPhi_Lyr3->Fill(svdPhi_2, over_U_SVD);
              h_DeltaResUz_Lyr3->Fill(svdZ_1, over_U_SVD);
              h_DeltaResUz_Lyr3->Fill(svdZ_2, over_U_SVD);
            }
            if (svd_Layer_1 == 4 && svd_Layer_2 == 4) {
              h_Lyr4[svd_Ladder_1][svd_Sensor_1]->Fill(svd_1->getPosition(), 0.);
              h_Lyr4[svd_Ladder_2][svd_Sensor_2]->Fill(svd_2->getPosition(), 0.);
              h_U_DeltaRes_SVD_Lyr4->Fill(over_U_SVD);
              h_DeltaResUPhi_Lyr4->Fill(svdPhi_1, over_U_SVD);
              h_DeltaResUPhi_Lyr4->Fill(svdPhi_2, over_U_SVD);
              h_DeltaResUz_Lyr4->Fill(svdZ_1, over_U_SVD);
              h_DeltaResUz_Lyr4->Fill(svdZ_2, over_U_SVD);
            }
            if (svd_Layer_1 == 5 && svd_Layer_2 == 5) {
              h_Lyr5[svd_Ladder_1][svd_Sensor_1]->Fill(svd_1->getPosition(), 0.);
              h_Lyr5[svd_Ladder_2][svd_Sensor_1]->Fill(svd_1->getPosition(), 0.);
              h_U_DeltaRes_SVD_Lyr5->Fill(over_U_SVD);
              h_DeltaResUPhi_Lyr5->Fill(svdPhi_1, over_U_SVD);
              h_DeltaResUPhi_Lyr5->Fill(svdPhi_2, over_U_SVD);
              h_DeltaResUz_Lyr5->Fill(svdZ_1, over_U_SVD);
              h_DeltaResUz_Lyr5->Fill(svdZ_2, over_U_SVD);
            }
            if (svd_Layer_1 == 6 && svd_Layer_2 == 6) {
              h_Lyr6[svd_Ladder_1][svd_Sensor_1]->Fill(svd_1->getPosition(), 0.);
              h_Lyr6[svd_Ladder_2][svd_Sensor_2]->Fill(svd_2->getPosition(), 0.);
              h_U_DeltaRes_SVD_Lyr6->Fill(over_U_SVD);
              h_DeltaResUPhi_Lyr6->Fill(svdPhi_1, over_U_SVD);
              h_DeltaResUPhi_Lyr6->Fill(svdPhi_2, over_U_SVD);
              h_DeltaResUz_Lyr6->Fill(svdZ_1, over_U_SVD);
              h_DeltaResUz_Lyr6->Fill(svdZ_2, over_U_SVD);
            }
          }
          //Restricting to consecutive SVD v-clusters
          if (svd_1->isUCluster() != true && svd_2->isUCluster() != true) {
            const int strips_1 = svd_1->getSize();
            h_SVDstrips_Mult->Fill(strips_1);
            const int strips_2 = svd_2->getSize();
            h_SVDstrips_Mult->Fill(strips_2);
            const double res_V_1 = resUnBias_SVD_1.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
            const double res_V_2 = resUnBias_SVD_2.GetMatrixArray()[0] * Unit::convertValueToUnit(1.0, "um");
            const float over_V_SVD = res_V_2 - res_V_1;
            const TVector3 svdLocal_1(svd_predIntersect_1[3], svd_1->getPosition(), 0.);
            const TVector3 svdLocal_2(svd_predIntersect_2[3], svd_2->getPosition(), 0.);
            const VXD::SensorInfoBase& svdSensor_1 = geo.get(svd_id_1);
            const VXD::SensorInfoBase& svdSensor_2 = geo.get(svd_id_2);
            const TVector3& svdGlobal_1 = svdSensor_1.pointToGlobal(svdLocal_1);
            const TVector3& svdGlobal_2 = svdSensor_2.pointToGlobal(svdLocal_2);
            double svdPhi_1 = atan2(svdGlobal_1(1), svdGlobal_1(0));
            double svdPhi_2 = atan2(svdGlobal_2(1), svdGlobal_2(0));
            double svdZ_1 = svdGlobal_1(2);
            double svdZ_2 = svdGlobal_2(2);
            B2DEBUG(40, "SVD: difference of v-residuals =========> " << over_V_SVD);
            //Fill SVD tree for v-overlaps if required by the user
            if (m_ExpertLevel) {
              deltaRes_SVD_V = over_V_SVD;
              intRes_SVD_V = res_V_1;
              intClPos_SVD_V = svd_1->getPosition();
              intClPosErr_SVD_V = svd_1->getPositionSigma();
              if (isMC && trueHit_1.size() > 0)
                intTruePos_SVD_V = trueHit_1[0]->getV();
              else
                intTruePos_SVD_V = -99;
              intClPhi_SVD_V = svdPhi_1;
              intClZ_SVD_V = svdZ_1;
              intTrkPos_SVD_V = svd_predIntersect_1[4];
              intTrkPosErr_SVD_V = sqrt(covMatrix_1[4][4]);
              intTrkQoP_SVD_V = svd_predIntersect_1[0];
              intTrkPrime_SVD_V = svd_predIntersect_1[2];
              intLayer_SVD_V = svd_Layer_1;
              intLadder_SVD_V = svd_Ladder_1;
              intSensor_SVD_V = svd_Sensor_1;
              intSize_SVD_V = strips_1;
              extRes_SVD_V = res_V_2;
              extClPos_SVD_V = svd_2->getPosition();
              extClPosErr_SVD_V = svd_2->getPositionSigma();
              if (isMC && trueHit_2.size() > 0)
                extTruePos_SVD_V = trueHit_2[0]->getV();
              else
                extTruePos_SVD_V = -99;
              extClPhi_SVD_V = svdPhi_2;
              extClZ_SVD_V = svdZ_2;
              extTrkPos_SVD_V = svd_predIntersect_2[4];
              extTrkPosErr_SVD_V = sqrt(covMatrix_2[4][4]);
              extTrkQoP_SVD_V = svd_predIntersect_2[0];
              extTrkPrime_SVD_V = svd_predIntersect_2[2];
              extLayer_SVD_V = svd_Layer_2;
              extLadder_SVD_V = svd_Ladder_2;
              extSensor_SVD_V = svd_Sensor_2;
              extSize_SVD_V = strips_2;
              t_SVD_V->Fill();
            }
            //Fill histograms of residuals differences with SVD v clusters
            h_V_DeltaRes->Fill(over_V_SVD);
            h_V_DeltaRes_SVD->Fill(over_V_SVD);
            if (svd_1->getSize() < 3 && svd_2->getSize() < 3) {  //Consider only clusters with 3 or less strips involved
              h_V_Cl1Cl2_DeltaRes[svd_1->getSize() * svd_2->getSize()]->Fill(over_V_SVD);
            }
            //Fill sensor hit-maps and 2D histograms with SVD v clusters
            if (svd_Layer_1 == 3 && svd_Layer_2 == 3) {
              h_Lyr3[svd_Ladder_1][svd_Sensor_1]->Fill(0., svd_1->getPosition());
              h_Lyr3[svd_Ladder_2][svd_Sensor_2]->Fill(0., svd_2->getPosition());
              h_V_DeltaRes_SVD_Lyr3->Fill(over_V_SVD);
              h_DeltaResVPhi_Lyr3->Fill(svdPhi_1, over_V_SVD);
              h_DeltaResVPhi_Lyr3->Fill(svdPhi_2, over_V_SVD);
              h_DeltaResVz_Lyr3->Fill(svdZ_1, over_V_SVD);
              h_DeltaResVz_Lyr3->Fill(svdZ_2, over_V_SVD);
            }
            if (svd_Layer_1 == 4 && svd_Layer_2 == 4) {
              h_Lyr4[svd_Ladder_1][svd_Sensor_1]->Fill(0., svd_1->getPosition());
              h_Lyr4[svd_Ladder_2][svd_Sensor_2]->Fill(0., svd_2->getPosition());
              h_V_DeltaRes_SVD_Lyr4->Fill(over_V_SVD);
              h_DeltaResVPhi_Lyr4->Fill(svdPhi_1, over_V_SVD);
              h_DeltaResVPhi_Lyr4->Fill(svdPhi_2, over_V_SVD);
              h_DeltaResVz_Lyr4->Fill(svdZ_1, over_V_SVD);
              h_DeltaResVz_Lyr4->Fill(svdZ_2, over_V_SVD);
            }
            if (svd_Layer_1 == 5 && svd_Layer_2 == 5) {
              h_Lyr5[svd_Ladder_1][svd_Sensor_1]->Fill(0., svd_1->getPosition());
              h_Lyr5[svd_Ladder_2][svd_Sensor_2]->Fill(0., svd_2->getPosition());
              h_V_DeltaRes_SVD_Lyr5->Fill(over_V_SVD);
              h_DeltaResVPhi_Lyr5->Fill(svdPhi_1, over_V_SVD);
              h_DeltaResVPhi_Lyr5->Fill(svdPhi_2, over_V_SVD);
              h_DeltaResVz_Lyr5->Fill(svdZ_1, over_V_SVD);
              h_DeltaResVz_Lyr5->Fill(svdZ_2, over_V_SVD);
            }
            if (svd_Layer_1 == 6 && svd_Layer_2 == 6) {
              h_Lyr6[svd_Ladder_1][svd_Sensor_1]->Fill(0., svd_1->getPosition());
              h_Lyr6[svd_Ladder_2][svd_Sensor_2]->Fill(0., svd_2->getPosition());
              h_V_DeltaRes_SVD_Lyr6->Fill(over_V_SVD);
              h_DeltaResVPhi_Lyr6->Fill(svdPhi_1, over_V_SVD);
              h_DeltaResVPhi_Lyr6->Fill(svdPhi_2, over_V_SVD);
              h_DeltaResVz_Lyr6->Fill(svdZ_1, over_V_SVD);
              h_DeltaResVz_Lyr6->Fill(svdZ_2, over_V_SVD);
            }
          }
        }
      }
    }
  }
}


