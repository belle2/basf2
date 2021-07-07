/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/VXDOverlaps/OverlapResidualsModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/core/Environment.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDShaperDigit.h>
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
#include <algorithm>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

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
    t_SVD_U->Branch("svdDeltaRes", &svdDeltaRes_U, "svdDeltaRes/F");
    t_SVD_U->Branch("svdTrkPXDHits", &svdTrkPXDHits, "svdTrkPXDHits/i");
    t_SVD_U->Branch("svdTrkSVDHits", &svdTrkSVDHits, "svdTrkSVDHits/i");
    t_SVD_U->Branch("svdTrkCDCHits", &svdTrkCDCHits, "svdTrkCDCHits/i");
    t_SVD_U->Branch("svdTrkd0", &svdTrkd0, "svdTrkd0/F");
    t_SVD_U->Branch("svdTrkz0", &svdTrkz0, "svdTrkz0/F");
    t_SVD_U->Branch("svdTrkpT", &svdTrkpT, "svdTrkpT/F");
    t_SVD_U->Branch("svdTrkpCM", &svdTrkpCM, "svdTrkpCM/F");
    // Internal ladder variables
    t_SVD_U->Branch("svdClSNR_int", &svdClSNR_U_int, "svdClSNR_int/F");
    t_SVD_U->Branch("svdClCharge_int", &svdClCharge_U_int, "svdClCharge_int/F");
    t_SVD_U->Branch("svdStripCharge_int", &svdStripCharge_U_int);
    t_SVD_U->Branch("svdStrip6Samples_int", &svdStrip6Samples_U_int);
    t_SVD_U->Branch("svdClTime_int", &svdClTime_U_int, "svdClTime_int/F");
    t_SVD_U->Branch("svdStripTime_int", &svdStripTime_U_int);
    t_SVD_U->Branch("svdStripPosition_int", &svdStripPosition_U_int);
    t_SVD_U->Branch("svdRes_int", &svdRes_U_int, "svdRes_int/F");
    t_SVD_U->Branch("svdClIntStrPos_int", &svdClIntStrPos_U_int, "svdClIntStrPos_int/F");
    t_SVD_U->Branch("svdClPos_int", &svdClPos_U_int, "svdClPos_int/F");
    t_SVD_U->Branch("svdClPosErr_int", &svdClPosErr_U_int, "svdClPosErr_int/F");
    t_SVD_U->Branch("svdTruePos_int", &svdTruePos_U_int, "svdTruePos_int/F");
    t_SVD_U->Branch("svdClPhi_int", &svdClPhi_U_int, "svdClPhi_int/F");
    t_SVD_U->Branch("svdClZ_int", &svdClZ_U_int, "svdClZ_int/F");
    t_SVD_U->Branch("svdTrkTraversedLength_int", &svdTrkTraversedLength_U_int, "svdTrkTraversedLength_int/F");
    t_SVD_U->Branch("svdTrkPos_int", &svdTrkPos_U_int, "svdTrkPos_int/F");
    t_SVD_U->Branch("svdTrkPosOS_int", &svdTrkPosOS_U_int, "svdTrkPosOS_int/F");
    t_SVD_U->Branch("svdTrkPosErr_int", &svdTrkPosErr_U_int, "svdTrkPosErr_int/F");
    t_SVD_U->Branch("svdTrkPosErrOS_int", &svdTrkPosErrOS_U_int, "svdTrkPosErrOS_int/F");
    t_SVD_U->Branch("svdTrkQoP_int", &svdTrkQoP_U_int, "svdTrkQoP_int/F");
    t_SVD_U->Branch("svdTrkPrime_int", &svdTrkPrime_U_int, "svdTrkPrime_int/F");
    t_SVD_U->Branch("svdTrkPrimeOS_int", &svdTrkPrimeOS_U_int, "svdTrkPrimeOS_int/F");
    t_SVD_U->Branch("svdTrkPosUnbiased_int", &svdTrkPosUnbiased_U_int, "svdTrkPosUnbiased_int/F");
    t_SVD_U->Branch("svdTrkPosErrUnbiased_int", &svdTrkPosErrUnbiased_U_int, "svdTrkPosErrUnbiased_int/F");
    t_SVD_U->Branch("svdTrkQoPUnbiased_int", &svdTrkQoPUnbiased_U_int, "svdTrkQoPUnbiased_int/F");
    t_SVD_U->Branch("svdTrkPrimeUnbiased_int", &svdTrkPrimeUnbiased_U_int, "svdTrkPrimeUnbiased_int/F");
    t_SVD_U->Branch("svdLayer_int", &svdLayer_U_int, "svdLayer_int/i");
    t_SVD_U->Branch("svdLadder_int", &svdLadder_U_int, "svdLadder_int/i");
    t_SVD_U->Branch("svdSensor_int", &svdSensor_U_int, "svdSensor_int/i");
    t_SVD_U->Branch("svdSize_int", &svdSize_U_int, "svdSize_int/i");
    // External ladder variables
    t_SVD_U->Branch("svdClSNR_ext", &svdClSNR_U_ext, "svdClSNR_ext/F");
    t_SVD_U->Branch("svdClCharge_ext", &svdClCharge_U_ext, "svdClCharge_ext/F");
    t_SVD_U->Branch("svdStripCharge_ext", &svdStripCharge_U_ext);
    t_SVD_U->Branch("svdStrip6Samples_ext", &svdStrip6Samples_U_ext);
    t_SVD_U->Branch("svdClTime_ext", &svdClTime_U_ext, "svdClTime_ext/F");
    t_SVD_U->Branch("svdStripTime_ext", &svdStripTime_U_ext);
    t_SVD_U->Branch("svdStripPosition_ext", &svdStripPosition_U_ext);
    t_SVD_U->Branch("svdRes_ext", &svdRes_U_ext, "svdRes_ext/F");
    t_SVD_U->Branch("svdClIntStrPos_ext", &svdClIntStrPos_U_ext, "svdClIntStrPos_ext/F");
    t_SVD_U->Branch("svdClPos_ext", &svdClPos_U_ext, "svdClPos_ext/F");
    t_SVD_U->Branch("svdClPosErr_ext", &svdClPosErr_U_ext, "svdClPosErr_ext/F");
    t_SVD_U->Branch("svdTruePos_ext", &svdTruePos_U_ext, "svdTruePos_ext/F");
    t_SVD_U->Branch("svdClPhi_ext", &svdClPhi_U_ext, "svdClPhi_ext/F");
    t_SVD_U->Branch("svdClZ_ext", &svdClZ_U_ext, "svdClZ_ext/F");
    t_SVD_U->Branch("svdTrkTraversedLength_ext", &svdTrkTraversedLength_U_ext, "svdTrkTraversedLength_ext/F");
    t_SVD_U->Branch("svdTrkPos_ext", &svdTrkPos_U_ext, "svdTrkPos_ext/F");
    t_SVD_U->Branch("svdTrkPosOS_ext", &svdTrkPosOS_U_ext, "svdTrkPosOS_ext/F");
    t_SVD_U->Branch("svdTrkPosErr_ext", &svdTrkPosErr_U_ext, "svdTrkPosErr_ext/F");
    t_SVD_U->Branch("svdTrkPosErrOS_ext", &svdTrkPosErrOS_U_ext, "svdTrkPosErrOS_ext/F");
    t_SVD_U->Branch("svdTrkQoP_ext", &svdTrkQoP_U_ext, "svdTrkQoP_ext/F");
    t_SVD_U->Branch("svdTrkPrime_ext", &svdTrkPrime_U_ext, "svdTrkPrime_ext/F");
    t_SVD_U->Branch("svdTrkPrimeOS_ext", &svdTrkPrimeOS_U_ext, "svdTrkPrimeOS_ext/F");
    t_SVD_U->Branch("svdTrkPosUnbiased_ext", &svdTrkPosUnbiased_U_ext, "svdTrkPosUnbiased_ext/F");
    t_SVD_U->Branch("svdTrkPosErrUnbiased_ext", &svdTrkPosErrUnbiased_U_ext, "svdTrkPosErrUnbiased_ext/F");
    t_SVD_U->Branch("svdTrkQoPUnbiased_ext", &svdTrkQoPUnbiased_U_ext, "svdTrkQoPUnbiased_ext/F");
    t_SVD_U->Branch("svdTrkPrimeUnbiased_ext", &svdTrkPrimeUnbiased_U_ext, "svdTrkPrimeUnbiased_ext/F");
    t_SVD_U->Branch("svdLayer_ext", &svdLayer_U_ext, "svdLayer_ext/i");
    t_SVD_U->Branch("svdLadder_ext", &svdLadder_U_ext, "svdLadder_ext/i");
    t_SVD_U->Branch("svdSensor_ext", &svdSensor_U_ext, "svdSensor_ext/i");
    t_SVD_U->Branch("svdSize_ext", &svdSize_U_ext, "svdSize_ext/i");
    //Tree for SVD v overlapping clusters
    t_SVD_V = new TTree("t_SVD_V", "Tree for SVD v-overlaps");
    t_SVD_V->Branch("svdDeltaRes", &svdDeltaRes_V, "svdDeltaRes/F");
    t_SVD_V->Branch("svdTrkPXDHits", &svdTrkPXDHits, "svdTrkPXDHits/i");
    t_SVD_V->Branch("svdTrkSVDHits", &svdTrkSVDHits, "svdTrkSVDHits/i");
    t_SVD_V->Branch("svdTrkCDCHits", &svdTrkCDCHits, "svdTrkCDCHits/i");
    t_SVD_V->Branch("svdTrkd0", &svdTrkd0, "svdTrkd0/F");
    t_SVD_V->Branch("svdTrkz0", &svdTrkz0, "svdTrkz0/F");
    t_SVD_V->Branch("svdTrkpT", &svdTrkpT, "svdTrkpT/F");
    t_SVD_V->Branch("svdTrkpCM", &svdTrkpCM, "svdTrkpCM/F");
    // Internal ladder variables
    t_SVD_V->Branch("svdClSNR_int", &svdClSNR_V_int, "svdClSNR_int/F");
    t_SVD_V->Branch("svdClCharge_int", &svdClCharge_V_int, "svdClCharge_int/F");
    t_SVD_V->Branch("svdStripCharge_int", &svdStripCharge_V_int);
    t_SVD_V->Branch("svdStrip6Samples_int", &svdStrip6Samples_V_int);
    t_SVD_V->Branch("svdClTime_int", &svdClTime_V_int, "svdClTime_int/F");
    t_SVD_V->Branch("svdStripTime_int", &svdStripTime_V_int);
    t_SVD_V->Branch("svdStripPosition_int", &svdStripPosition_V_int);
    t_SVD_V->Branch("svdRes_int", &svdRes_V_int, "svdRes_int/F");
    t_SVD_V->Branch("svdClIntStrPos_int", &svdClIntStrPos_V_int, "svdClIntStrPos_int/F");
    t_SVD_V->Branch("svdClPos_int", &svdClPos_V_int, "svdClPos_int/F");
    t_SVD_V->Branch("svdClPosErr_int", &svdClPosErr_V_int, "svdClPosErr_int/F");
    t_SVD_V->Branch("svdTruePos_int", &svdTruePos_V_int, "svdTruePos_int/F");
    t_SVD_V->Branch("svdClPhi_int", &svdClPhi_V_int, "svdClPhi_int/F");
    t_SVD_V->Branch("svdClZ_int", &svdClZ_V_int, "svdClZ_int/F");
    t_SVD_V->Branch("svdTrkTraversedLength_int", &svdTrkTraversedLength_V_int, "svdTrkTraversedLength_int/F");
    t_SVD_V->Branch("svdTrkPos_int", &svdTrkPos_V_int, "svdTrkPos_int/F");
    t_SVD_V->Branch("svdTrkPosOS_int", &svdTrkPosOS_V_int, "svdTrkPosOS_int/F");
    t_SVD_V->Branch("svdTrkPosErr_int", &svdTrkPosErr_V_int, "svdTrkPosErr_int/F");
    t_SVD_V->Branch("svdTrkPosErrOS_int", &svdTrkPosErrOS_V_int, "svdTrkPosErrOS_int/F");
    t_SVD_V->Branch("svdTrkQoP_int", &svdTrkQoP_V_int, "svdTrkQoP_int/F");
    t_SVD_V->Branch("svdTrkPrime_int", &svdTrkPrime_V_int, "svdTrkPrime_int/F");
    t_SVD_V->Branch("svdTrkPrimeOS_int", &svdTrkPrimeOS_V_int, "svdTrkPrimeOS_int/F");
    t_SVD_V->Branch("svdTrkPosUnbiased_int", &svdTrkPosUnbiased_V_int, "svdTrkPosUnbiased_int/F");
    t_SVD_V->Branch("svdTrkPosErrUnbiased_int", &svdTrkPosErrUnbiased_V_int, "svdTrkPosErrUnbiased_int/F");
    t_SVD_V->Branch("svdTrkQoPUnbiased_int", &svdTrkQoPUnbiased_V_int, "svdTrkQoPUnbiased_int/F");
    t_SVD_V->Branch("svdTrkPrimeUnbiased_int", &svdTrkPrimeUnbiased_V_int, "svdTrkPrimeUnbiased_int/F");
    t_SVD_V->Branch("svdLayer_int", &svdLayer_V_int, "svdLayer_int/i");
    t_SVD_V->Branch("svdLadder_int", &svdLadder_V_int, "svdLadder_int/i");
    t_SVD_V->Branch("svdSensor_int", &svdSensor_V_int, "svdSensor_int/i");
    t_SVD_V->Branch("svdSize_int", &svdSize_V_int, "svdSize_int/i");
    // External ladder variables
    t_SVD_V->Branch("svdClSNR_ext", &svdClSNR_V_ext, "svdClSNR_ext/F");
    t_SVD_V->Branch("svdClCharge_ext", &svdClCharge_V_ext, "svdClCharge_ext/F");
    t_SVD_V->Branch("svdStripCharge_ext", &svdStripCharge_V_ext);
    t_SVD_V->Branch("svdStrip6Samples_ext", &svdStrip6Samples_V_ext);
    t_SVD_V->Branch("svdClTime_ext", &svdClTime_V_ext, "svdClTime_ext/F");
    t_SVD_V->Branch("svdStripTime_ext", &svdStripTime_V_ext);
    t_SVD_V->Branch("svdStripPosition_ext", &svdStripPosition_V_ext);
    t_SVD_V->Branch("svdRes_ext", &svdRes_V_ext, "svdRes_ext/F");
    t_SVD_V->Branch("svdClIntStrPos_ext", &svdClIntStrPos_V_ext, "svdClIntStrPos_ext/F");
    t_SVD_V->Branch("svdClPos_ext", &svdClPos_V_ext, "svdClPos_ext/F");
    t_SVD_V->Branch("svdClPosErr_ext", &svdClPosErr_V_ext, "svdClPosErr_ext/F");
    t_SVD_V->Branch("svdTruePos_ext", &svdTruePos_V_ext, "svdTruePos_ext/F");
    t_SVD_V->Branch("svdClPhi_ext", &svdClPhi_V_ext, "svdClPhi_ext/F");
    t_SVD_V->Branch("svdClZ_ext", &svdClZ_V_ext, "svdClZ_ext/F");
    t_SVD_V->Branch("svdTrkTraversedLength_ext", &svdTrkTraversedLength_V_ext, "svdTrkTraversedLength_ext/F");
    t_SVD_V->Branch("svdTrkPos_ext", &svdTrkPos_V_ext, "svdTrkPos_ext/F");
    t_SVD_V->Branch("svdTrkPosOS_ext", &svdTrkPosOS_V_ext, "svdTrkPosOS_ext/F");
    t_SVD_V->Branch("svdTrkPosErr_ext", &svdTrkPosErr_V_ext, "svdTrkPosErr_ext/F");
    t_SVD_V->Branch("svdTrkPosErrOS_ext", &svdTrkPosErrOS_V_ext, "svdTrkPosErrOS_ext/F");
    t_SVD_V->Branch("svdTrkQoP_ext", &svdTrkQoP_V_ext, "svdTrkQoP_ext/F");
    t_SVD_V->Branch("svdTrkPrime_ext", &svdTrkPrime_V_ext, "svdTrkPrime_ext/F");
    t_SVD_V->Branch("svdTrkPrimeOS_ext", &svdTrkPrimeOS_V_ext, "svdTrkPrimeOS_ext/F");
    t_SVD_V->Branch("svdTrkPosUnbiased_ext", &svdTrkPosUnbiased_V_ext, "svdTrkPosUnbiased_ext/F");
    t_SVD_V->Branch("svdTrkPosErrUnbiased_ext", &svdTrkPosErrUnbiased_V_ext, "svdTrkPosErrUnbiased_ext/F");
    t_SVD_V->Branch("svdTrkQoPUnbiased_ext", &svdTrkQoPUnbiased_V_ext, "svdTrkQoPUnbiased_ext/F");
    t_SVD_V->Branch("svdTrkPrimeUnbiased_ext", &svdTrkPrimeUnbiased_V_ext, "svdTrkPrimeUnbiased_ext/F");
    t_SVD_V->Branch("svdLayer_ext", &svdLayer_V_ext, "svdLayer_ext/i");
    t_SVD_V->Branch("svdLadder_ext", &svdLadder_V_ext, "svdLadder_ext/i");
    t_SVD_V->Branch("svdSensor_ext", &svdSensor_V_ext, "svdSensor_ext/i");
    t_SVD_V->Branch("svdSize_ext", &svdSize_V_ext, "svdSize_ext/i");
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
    RelationVector<Track> theTK = DataStore::getRelationsWithObj<Track>(&trk);

    const TrackFitResult*  tfr = theTK[0]->getTrackFitResultWithClosestMass(Const::pion);
    if (tfr) {
      svdTrkd0 = tfr->getD0();
      svdTrkz0 = tfr->getZ0();
      svdTrkpT = tfr->getMomentum().Perp();
      TLorentzVector pStar = tfr->get4Momentum();
      pStar.Boost(0, 0, 3. / 11);
      svdTrkpCM = pStar.P();
    }

    svdTrkPXDHits = (trk.getPXDHitList()).size();
    svdTrkSVDHits = (trk.getSVDHitList()).size();
    svdTrkCDCHits = (trk.getCDCHitList()).size();

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
          genfit::MeasuredStateOnPlane state_unbiased_1 = fittedResult_1->getFittedState(false);
          genfit::MeasuredStateOnPlane state_unbiased_2 = fittedResult_2->getFittedState(false);
          const TVectorD& svd_predIntersect_unbiased_1 = state_unbiased_1.getState();
          const TVectorD& svd_predIntersect_unbiased_2 = state_unbiased_2.getState();
          const TMatrixDSym& covMatrix_unbiased_1 = state_unbiased_1.getCov();
          const TMatrixDSym& covMatrix_unbiased_2 = state_unbiased_2.getCov();
          genfit::MeasuredStateOnPlane state_1 = trk.getMeasuredStateOnPlaneFromRecoHit(infoSVD_1);
          genfit::MeasuredStateOnPlane state_2 = trk.getMeasuredStateOnPlaneFromRecoHit(infoSVD_2);
          const TVectorD& svd_predIntersect_1 = state_1.getState();
          const TVectorD& svd_predIntersect_2 = state_2.getState();
          const TMatrixDSym& covMatrix_1 = state_1.getCov();
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
              svdDeltaRes_U = over_U_SVD;
              svdRes_U_int = res_U_1;
              svdClTime_U_int = svd_1->getClsTime();
              svdClSNR_U_int = svd_1->getSNR();
              svdClCharge_U_int = svd_1->getCharge();
              svdClPosErr_U_int = svd_1->getPositionSigma();
              if (isMC && trueHit_1.size() > 0)
                svdTruePos_U_int = trueHit_1[0]->getU();
              else
                svdTruePos_U_int = -99;
              svdClPhi_U_int = svdPhi_1;
              svdClZ_U_int = svdZ_1;
              svdTrkPos_U_int = svd_predIntersect_1[3];
              svdTrkPosOS_U_int = svd_predIntersect_1[4];
              svdTrkPosErr_U_int = sqrt(covMatrix_1[3][3]);
              svdTrkPosErrOS_U_int = sqrt(covMatrix_1[4][4]);
              svdTrkQoP_U_int = svd_predIntersect_1[0];
              svdTrkPrime_U_int = svd_predIntersect_1[1];
              svdTrkPrimeOS_U_int = svd_predIntersect_1[2];
              svdTrkTraversedLength_U_int = svdSensor_1.getThickness() * sqrt(1 + svdTrkPrimeOS_U_int * svdTrkPrimeOS_U_int + svdTrkPrime_U_int *
                                            svdTrkPrime_U_int);
              svdTrkPosUnbiased_U_int = svd_predIntersect_unbiased_1[3];
              svdClPos_U_int = svdRes_U_int / 1e4 + svdTrkPosUnbiased_U_int;
              svdTrkPosErrUnbiased_U_int = sqrt(covMatrix_unbiased_1[3][3]);
              svdTrkQoPUnbiased_U_int = svd_predIntersect_unbiased_1[0];
              svdTrkPrimeUnbiased_U_int = svd_predIntersect_unbiased_1[1];
              svdLayer_U_int = svd_Layer_1;
              svdLadder_U_int = svd_Ladder_1;
              svdSensor_U_int = svd_Sensor_1;
              svdSize_U_int = strips_1;

              float pitch = 50e-4;
              float halfLength = 1.92;
              if (svdLayer_U_int > 3) {
                pitch = 75e-4;
                halfLength = 2.88;
              }
              svdClIntStrPos_U_int = fmod(svdClPos_U_int + halfLength, pitch) / pitch;

              svdStripCharge_U_int.clear();
              svdStrip6Samples_U_int.clear();
              svdStripTime_U_int.clear();
              svdStripPosition_U_int.clear();
              //retrieve relations and set strip charges and times
              RelationVector<SVDRecoDigit> theRecoDigits_1 = DataStore::getRelationsWithObj<SVDRecoDigit>(svd_1);
              if ((theRecoDigits_1.size() != svdSize_U_int) && (svdSize_U_int != 128)) //virtual cluster
                B2ERROR(" Inconsistency with cluster size! # recoDigits = " << theRecoDigits_1.size() << " != " << svdSize_U_int <<
                        " cluster size");

              //skip clusters created beacuse of missing APV
              if (svdSize_U_int < 128)
                for (unsigned int d = 0; d < svdSize_U_int; d++) {
                  svdStripCharge_U_int.push_back(theRecoDigits_1[d]->getCharge());
                  SVDShaperDigit* ShaperDigit_1 = theRecoDigits_1[d]->getRelated<SVDShaperDigit>();
                  array<float, 6> Samples_1 = ShaperDigit_1->getSamples();
                  std::copy(std::begin(Samples_1), std::end(Samples_1), std::back_inserter(svdStrip6Samples_U_int));
                  svdStripTime_U_int.push_back(theRecoDigits_1[d]->getTime());
                  double misalignedStripPos = svdSensor_1.getUCellPosition(theRecoDigits_1[d]->getCellID());
                  //aligned strip pos = misaligned strip - ( misaligned cluster - aligned cluster)
                  svdStripPosition_U_int.push_back(misalignedStripPos - svd_1->getPosition() + svdClPos_U_int);
                }

              svdRes_U_ext = res_U_2;
              svdClTime_U_ext = svd_2->getClsTime();
              svdClSNR_U_ext = svd_2->getSNR();
              svdClCharge_U_ext = svd_2->getCharge();
              svdClPosErr_U_ext = svd_2->getPositionSigma();
              if (isMC && trueHit_2.size() > 0)
                svdTruePos_U_ext = trueHit_2[0]->getU();
              else
                svdTruePos_U_ext = -99;
              svdClPhi_U_ext = svdPhi_2;
              svdClZ_U_ext = svdZ_2;
              svdTrkPos_U_ext = svd_predIntersect_2[3];
              svdTrkPosOS_U_ext = svd_predIntersect_2[4];
              svdTrkPosErr_U_ext = sqrt(covMatrix_2[3][3]);
              svdTrkPosErrOS_U_ext = sqrt(covMatrix_2[4][4]);
              svdTrkQoP_U_ext = svd_predIntersect_2[0];
              svdTrkPrime_U_ext = svd_predIntersect_2[1];
              svdTrkPrimeOS_U_ext = svd_predIntersect_2[2];
              svdTrkTraversedLength_U_ext = svdSensor_2.getThickness() * sqrt(1 + svdTrkPrimeOS_U_ext * svdTrkPrimeOS_U_ext + svdTrkPrime_U_ext *
                                            svdTrkPrime_U_ext);
              svdTrkPosUnbiased_U_ext = svd_predIntersect_unbiased_2[3];
              svdClPos_U_ext = svdRes_U_ext / 1e4 + svdTrkPosUnbiased_U_ext;
              svdTrkPosErrUnbiased_U_ext = sqrt(covMatrix_unbiased_2[3][3]);
              svdTrkQoPUnbiased_U_ext = svd_predIntersect_unbiased_2[0];
              svdTrkPrimeUnbiased_U_ext = svd_predIntersect_unbiased_2[1];
              svdLayer_U_ext = svd_Layer_2;
              svdLadder_U_ext = svd_Ladder_2;
              svdSensor_U_ext = svd_Sensor_2;
              svdSize_U_ext = strips_2;

              svdClIntStrPos_U_ext = fmod(svdClPos_U_ext + halfLength, pitch) / pitch;

              svdStripCharge_U_ext.clear();
              svdStrip6Samples_U_ext.clear();
              svdStripTime_U_ext.clear();
              svdStripPosition_U_ext.clear();
              //retrieve relations and set strip charges and times
              RelationVector<SVDRecoDigit> theRecoDigits_2 = DataStore::getRelationsWithObj<SVDRecoDigit>(svd_2);
              if ((theRecoDigits_2.size() != svdSize_U_ext) && (svdSize_U_ext != 128)) //virtual cluster
                B2ERROR(" Inconsistency with cluster size! # recoDigits = " << theRecoDigits_2.size() << " != " << svdSize_U_ext <<
                        " cluster size");
              //skip clusters created beacuse of missing APV
              if (svdSize_U_ext < 128)
                for (unsigned int d = 0; d < svdSize_U_ext; d++) {
                  svdStripCharge_U_ext.push_back(theRecoDigits_2[d]->getCharge());
                  SVDShaperDigit* ShaperDigit_2 = theRecoDigits_2[d]->getRelated<SVDShaperDigit>();
                  array<float, 6> Samples_2 = ShaperDigit_2->getSamples();
                  std::copy(std::begin(Samples_2), std::end(Samples_2), std::back_inserter(svdStrip6Samples_U_ext));
                  svdStripTime_U_ext.push_back(theRecoDigits_2[d]->getTime());
                  double misalignedStripPos = svdSensor_2.getUCellPosition(theRecoDigits_2[d]->getCellID());
                  //aligned strip pos = misaligned strip - ( misaligned cluster - aligned cluster)
                  svdStripPosition_U_ext.push_back(misalignedStripPos - svd_2->getPosition() + svdClPos_U_ext);
                }
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
              svdDeltaRes_V = over_V_SVD;
              svdRes_V_int = res_V_1;
              svdClTime_V_int = svd_1->getClsTime();
              svdClSNR_V_int = svd_1->getSNR();
              svdClCharge_V_int = svd_1->getCharge();
              svdClPosErr_V_int = svd_1->getPositionSigma();
              if (isMC && trueHit_1.size() > 0)
                svdTruePos_V_int = trueHit_1[0]->getV();
              else
                svdTruePos_V_int = -99;
              svdClPhi_V_int = svdPhi_1;
              svdClZ_V_int = svdZ_1;
              svdTrkPos_V_int = svd_predIntersect_1[4];
              svdTrkPosOS_V_int = svd_predIntersect_1[3];
              svdTrkPosErr_V_int = sqrt(covMatrix_1[4][4]);
              svdTrkPosErrOS_V_int = sqrt(covMatrix_1[3][3]);
              svdTrkQoP_V_int = svd_predIntersect_1[0];
              svdTrkPrime_V_int = svd_predIntersect_1[2];
              svdTrkPrimeOS_V_int = svd_predIntersect_1[1];
              svdTrkTraversedLength_V_int = svdSensor_1.getThickness() * sqrt(1 + svdTrkPrimeOS_V_int * svdTrkPrimeOS_V_int + svdTrkPrime_V_int *
                                            svdTrkPrime_V_int);
              svdTrkPosUnbiased_V_int = svd_predIntersect_unbiased_1[4];
              svdClPos_V_int = svdRes_V_int / 1e4 + svdTrkPosUnbiased_V_int;
              svdTrkPosErrUnbiased_V_int = sqrt(covMatrix_unbiased_1[4][4]);
              svdTrkQoPUnbiased_V_int = svd_predIntersect_unbiased_1[0];
              svdTrkPrimeUnbiased_V_int = svd_predIntersect_unbiased_1[2];
              svdLayer_V_int = svd_Layer_1;
              svdLadder_V_int = svd_Ladder_1;
              svdSensor_V_int = svd_Sensor_1;
              svdSize_V_int = strips_1;

              float pitch = 160e-4;
              float halfLength = 6.144;
              if (svdLayer_V_int > 3) {
                pitch = 240e-4;
              }
              svdClIntStrPos_V_int = fmod(svdClPos_V_int + halfLength, pitch) / pitch;

              svdStripCharge_V_int.clear();
              svdStrip6Samples_V_int.clear();
              svdStripTime_V_int.clear();
              svdStripPosition_V_int.clear();
              //retrieve relations and set strip charges and times
              RelationVector<SVDRecoDigit> theRecoDigits_1 = DataStore::getRelationsWithObj<SVDRecoDigit>(svd_1);
              if ((theRecoDigits_1.size() != svdSize_V_int) && (svdSize_V_int != 128)) //virtual cluster
                B2ERROR(" Inconsistency with cluster size! # recoDigits = " << theRecoDigits_1.size() << " != " << svdSize_V_int <<
                        " cluster size");
              //skip clusters created beacuse of missing APV
              if (svdSize_V_int < 128)
                for (unsigned int d = 0; d < svdSize_V_int; d++) {
                  svdStripCharge_V_int.push_back(theRecoDigits_1[d]->getCharge());
                  SVDShaperDigit* ShaperDigit_1 = theRecoDigits_1[d]->getRelated<SVDShaperDigit>();
                  array<float, 6> Samples_1 = ShaperDigit_1->getSamples();
                  std::copy(std::begin(Samples_1), std::end(Samples_1), std::back_inserter(svdStrip6Samples_V_int));
                  svdStripTime_V_int.push_back(theRecoDigits_1[d]->getTime());
                  double misalignedStripPos = svdSensor_1.getVCellPosition(theRecoDigits_1[d]->getCellID());
                  //aligned strip pos = misaligned strip - ( misaligned cluster - aligned cluster)
                  svdStripPosition_V_int.push_back(misalignedStripPos - svd_1->getPosition() + svdClPos_V_int);
                }

              svdRes_V_ext = res_V_2;
              svdClTime_V_ext = svd_2->getClsTime();
              svdClSNR_V_ext = svd_2->getSNR();
              svdClCharge_V_ext = svd_2->getCharge();
              svdClPosErr_V_ext = svd_2->getPositionSigma();
              if (isMC && trueHit_2.size() > 0)
                svdTruePos_V_ext = trueHit_2[0]->getV();
              else
                svdTruePos_V_ext = -99;
              svdClPhi_V_ext = svdPhi_2;
              svdClZ_V_ext = svdZ_2;
              svdTrkPos_V_ext = svd_predIntersect_2[4];
              svdTrkPosOS_V_ext = svd_predIntersect_2[3];
              svdTrkPosErr_V_ext = sqrt(covMatrix_2[4][4]);
              svdTrkPosErrOS_V_ext = sqrt(covMatrix_1[3][3]);
              svdTrkQoP_V_ext = svd_predIntersect_2[0];
              svdTrkPrime_V_ext = svd_predIntersect_2[2];
              svdTrkPrimeOS_V_ext = svd_predIntersect_2[1];
              svdTrkTraversedLength_V_ext = svdSensor_2.getThickness() * sqrt(1 + svdTrkPrimeOS_V_ext * svdTrkPrimeOS_V_ext + svdTrkPrime_V_ext *
                                            svdTrkPrime_V_ext);
              svdTrkPosUnbiased_V_ext = svd_predIntersect_unbiased_2[4];
              svdClPos_V_ext = svdRes_V_ext / 1e4 + svdTrkPosUnbiased_V_ext;
              svdTrkPosErrUnbiased_V_ext = sqrt(covMatrix_unbiased_2[4][4]);
              svdTrkQoPUnbiased_V_ext = svd_predIntersect_unbiased_2[0];
              svdTrkPrimeUnbiased_V_ext = svd_predIntersect_unbiased_2[2];
              svdLayer_V_ext = svd_Layer_2;
              svdLadder_V_ext = svd_Ladder_2;
              svdSensor_V_ext = svd_Sensor_2;
              svdSize_V_ext = strips_2;

              svdClIntStrPos_V_ext = fmod(svdClPos_V_ext + halfLength, pitch) / pitch;

              svdStripCharge_V_ext.clear();
              svdStrip6Samples_V_ext.clear();
              svdStripTime_V_ext.clear();
              svdStripPosition_V_ext.clear();
              //retrieve relations and set strip charges and times
              RelationVector<SVDRecoDigit> theRecoDigits_2 = DataStore::getRelationsWithObj<SVDRecoDigit>(svd_2);
              if ((theRecoDigits_2.size() != svdSize_V_ext) && (svdSize_V_ext != 128)) //virtual cluster
                B2ERROR(" Inconsistency with cluster size! # recoDigits = " << theRecoDigits_2.size() << " != " << svdSize_V_ext <<
                        " cluster size");
              //skip clusters created beacuse of missing APV
              if (svdSize_V_ext < 128)
                for (unsigned int d = 0; d < svdSize_V_ext; d++) {
                  svdStripCharge_V_ext.push_back(theRecoDigits_2[d]->getCharge());
                  SVDShaperDigit* ShaperDigit_2 = theRecoDigits_2[d]->getRelated<SVDShaperDigit>();
                  array<float, 6> Samples_2 = ShaperDigit_2->getSamples();
                  std::copy(std::begin(Samples_2), std::end(Samples_2), std::back_inserter(svdStrip6Samples_V_ext));
                  svdStripTime_V_ext.push_back(theRecoDigits_2[d]->getTime());
                  double misalignedStripPos = svdSensor_2.getVCellPosition(theRecoDigits_2[d]->getCellID());
                  //aligned strip pos = misaligned strip - ( misaligned cluster - aligned cluster)
                  svdStripPosition_V_ext.push_back(misalignedStripPos - svd_2->getPosition() + svdClPos_V_ext);
                }

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


