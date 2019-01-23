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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <svd/dataobjects/SVDCluster.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <genfit/TrackPoint.h>
#include <framework/gearbox/Const.h>

#include <set>

#include <root/TObject.h>
#include <root/TROOT.h>

#include <boost/foreach.hpp>

#include <typeinfo>
#include <cxxabi.h>

#include <TMath.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;


//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(OverlapResiduals)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

OverlapResidualsModule::OverlapResidualsModule() : Module()
{
  // Set module properties
  setDescription("The module studies consecutive hits in overlapping sensors of a same VXD layer, and the differences of their residuals, to monitor the detector alignment.");

  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string(""));


}

void OverlapResidualsModule::initialize()
{
  //set the ROOT File

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  h_U_Res = new TH1F("h_U_Res", "Histrogram of overlapping hits residuals", 100, -0.1, 0.1);
  h_V_Res = new TH1F("h_V_Res", "Histrogram of overlapping hits residuals", 100, -0.1, 0.1);
  h_SVDstrips_Mult = new TH1F("h_SVDstrips_Mult", "SVD strips multipicity for SVD clusters in overlapping sensors", 15, 0.5, 15.5);

  for (int i = 1; i < 10; i++) {

    //The name is the product of cluster sizes for 2 consecutive hits
    TString h_name_U = "h_U_Cl1Cl2_" + std::to_string(i);
    TString h_name_V = "h_V_Cl1Cl2_" + std::to_string(i);
    TString title_U = "U residuals: SVDClusterSize_1 x SVDClusterSize_2 = " + std::to_string(i);
    TString title_V = "V residuals: SVDClusterSize_1 x SVDClusterSize_2 = " + std::to_string(i);
    h_U_Cl1Cl2_Res[i] = new TH1F(h_name_U, title_U, 100, -0.1, 0.1);
    h_V_Cl1Cl2_Res[i] = new TH1F(h_name_V, title_V, 100, -0.1, 0.1);

  }

  for (int i = 1; i <= 5; i++) {
    for (int j = 1; j <= 16; j++) {

      TString h_name = "h_6" + std::to_string(j) + std::to_string(i);
      TString title = "Layer:Ladder:Sensor = 6:" + std::to_string(j) + ":" + std::to_string(i);
      h_Lyr6[j][i] = new TH2F(h_name, title, 100, -2.88, 2.88, 100, -6.1, 6.1);
      h_Lyr6[j][i]->GetXaxis()->SetTitle("u (10 x mm)");
      h_Lyr6[j][i]->GetYaxis()->SetTitle("v (10 x mm)");
    }
  }

  for (int i = 1; i <= 4; i++) {
    for (int j = 1; j <= 12; j++) {

      TString h_name = "h_5" + std::to_string(j) + std::to_string(i);
      TString title = "Layer:Ladder:Sensor = 5:" + std::to_string(j) + ":" + std::to_string(i);
      h_Lyr5[j][i] = new TH2F(h_name, title, 100, -2.8, 2.8, 100, -6.1, 6.1);
      h_Lyr5[j][i]->GetXaxis()->SetTitle("u (10 x mm)");
      h_Lyr5[j][i]->GetYaxis()->SetTitle("v (10 x mm)");

    }
  }

  for (int i = 1; i <= 3; i++) {
    for (int j = 1; j <= 10; j++) {

      TString h_name = "h_4" + std::to_string(j) + std::to_string(i);
      TString title = "Layer:Ladder:Sensor = 4:" + std::to_string(j) + ":" + std::to_string(i);
      h_Lyr4[j][i] = new TH2F(h_name, title, 100, -2.8, 2.8, 100, -6.1, 6.1);
      h_Lyr4[j][i]->GetXaxis()->SetTitle("u (10 x mm)");
      h_Lyr4[j][i]->GetYaxis()->SetTitle("v (10 x mm)");

    }
  }

  for (int i = 1; i <= 2; i++) {
    for (int j = 1; j <= 7; j++) {

      TString h_name = "h_3" + std::to_string(j) + std::to_string(i);
      TString title = "Layer:Ladder:Sensor = 3:" + std::to_string(j) + ":" + std::to_string(i);
      h_Lyr3[j][i] = new TH2F(h_name, title, 100, -2.8, 2.8, 100, -6.1, 6.1);
      h_Lyr3[j][i]->GetXaxis()->SetTitle("u (10 x mm)");
      h_Lyr3[j][i]->GetYaxis()->SetTitle("v (10 x mm)");

    }
  }

  for (int i = 1; i <= 2; i++) {
    for (int j = 1; j <= 12; j++) {

      TString h_name = "h_2" + std::to_string(j) + std::to_string(i);
      TString title = "Layer:Ladder:Sensor = 2:" + std::to_string(j) + ":" + std::to_string(i);
      h_Lyr2[j][i] = new TH2F(h_name, title, 100, -0.8, 0.8, 100, -2.5, 2.5);
      h_Lyr2[j][i]->GetXaxis()->SetTitle("u (10 x mm)");
      h_Lyr2[j][i]->GetYaxis()->SetTitle("v (10 x mm)");
    }
  }

  for (int i = 1; i <= 2; i++) {
    for (int j = 1; j <= 8; j++) {

      TString h_name = "h_1" + std::to_string(j) + std::to_string(i);
      TString title = "Layer:Ladder:Sensor = 1:" + std::to_string(j) + ":" + std::to_string(i);
      h_Lyr1[j][i] = new TH2F(h_name, title, 100, -0.8, 0.8, 100, -2.5, 2.5);
      h_Lyr1[j][i]->GetXaxis()->SetTitle("u (10 x mm)");
      h_Lyr1[j][i]->GetYaxis()->SetTitle("v (10 x mm)");

    }
  }



  //HISTOGRAMS OF PREDICTED HITS FROM THE TRACK FIT


  for (int i = 1; i <= 5; i++) {
    for (int j = 1; j <= 16; j++) {

      TString h_name = "h_Fit_6" + std::to_string(j) + std::to_string(i);
      TString title = "Layer:Ladder:Sensor = 6:" + std::to_string(j) + ":" + std::to_string(i);
      h_Fit_Lyr6[j][i] = new TH2F(h_name, title, 100, -2.88, 2.88, 100, -6.1, 6.1);
      h_Fit_Lyr6[j][i]->GetXaxis()->SetTitle("u (10 x mm)");
      h_Fit_Lyr6[j][i]->GetYaxis()->SetTitle("v (10 x mm)");
    }
  }


  for (int i = 1; i <= 4; i++) {
    for (int j = 1; j <= 12; j++) {

      TString h_name = "h_Fit_5" + std::to_string(j) + std::to_string(i);
      TString title = "Layer:Ladder:Sensor = 5:" + std::to_string(j) + ":" + std::to_string(i);
      h_Fit_Lyr5[j][i] = new TH2F(h_name, title, 100, -2.8, 2.8, 100, -6.1, 6.1);
      h_Fit_Lyr5[j][i]->GetXaxis()->SetTitle("u (10 x mm)");
      h_Fit_Lyr5[j][i]->GetYaxis()->SetTitle("v (10 x mm)");

    }
  }

  for (int i = 1; i <= 3; i++) {
    for (int j = 1; j <= 10; j++) {

      TString h_name = "h_Fit_4" + std::to_string(j) + std::to_string(i);
      TString title = "Layer:Ladder:Sensor = 4:" + std::to_string(j) + ":" + std::to_string(i);
      h_Fit_Lyr4[j][i] = new TH2F(h_name, title, 100, -2.8, 2.8, 100, -6.1, 6.1);
      h_Fit_Lyr4[j][i]->GetXaxis()->SetTitle("u (10 x mm)");
      h_Fit_Lyr4[j][i]->GetYaxis()->SetTitle("v (10 x mm)");

    }
  }


  for (int i = 1; i <= 2; i++) {
    for (int j = 1; j <= 7; j++) {

      TString h_name = "h_Fit_3" + std::to_string(j) + std::to_string(i);
      TString title = "Layer:Ladder:Sensor = 3:" + std::to_string(j) + ":" + std::to_string(i);
      h_Fit_Lyr3[j][i] = new TH2F(h_name, title, 100, -2.8, 2.8, 100, -6.1, 6.1);
      h_Fit_Lyr3[j][i]->GetXaxis()->SetTitle("u (10 x mm)");
      h_Fit_Lyr3[j][i]->GetYaxis()->SetTitle("v (10 x mm)");

    }
  }

  for (int i = 1; i <= 2; i++) {
    for (int j = 1; j <= 12; j++) {

      TString h_name = "h_Fit_2" + std::to_string(j) + std::to_string(i);
      TString title = "Layer:Ladder:Sensor = 2:" + std::to_string(j) + ":" + std::to_string(i);
      h_Fit_Lyr2[j][i] = new TH2F(h_name, title, 100, -0.8, 0.8, 100, -2.1, 2.1);
      h_Fit_Lyr2[j][i]->GetXaxis()->SetTitle("u (10 x mm)");
      h_Fit_Lyr2[j][i]->GetYaxis()->SetTitle("v (10 x mm)");

    }
  }

  for (int i = 1; i <= 2; i++) {
    for (int j = 1; j <= 8; j++) {

      TString h_name = "h_Fit_1" + std::to_string(j) + std::to_string(i);
      TString title = "Layer:Ladder:Sensor = 1:" + std::to_string(j) + ":" + std::to_string(i);
      h_Fit_Lyr1[j][i] = new TH2F(h_name, title, 100, -0.8, 0.8, 100, -2.1, 2.1);
      h_Fit_Lyr1[j][i]->GetXaxis()->SetTitle("u (10 x mm)");
      h_Fit_Lyr1[j][i]->GetYaxis()->SetTitle("v (10 x mm)");

    }
  }

}

void OverlapResidualsModule::event()
{

  gROOT->SetBatch();
  for (const auto& trk : recoTrack) {
    if (! trk.wasFitSuccessful()) {
      continue;
    }
    const vector<PXDCluster* > pxdClusters = trk.getPXDHitList();
    const vector<SVDCluster* > svdClusters = trk.getSVDHitList();
    B2INFO("FITTED TRACK:   NUMBER OF PXD HITS = " << pxdClusters.size() << "    NUMBER OF SVD HITS = " << svdClusters.size());

    //LOOKING FOR 2 CONSECUTIVE PXD HITS IN OVERLAPPING MODULES OF A SAME LAYER
    for (int i = 0; i < pxdClusters.size(); i++) {

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
      for (int l = i + 1; l < pxdClusters.size(); l++) {

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
        if (pxd_Layer_1 == pxd_Layer_2) {
          B2INFO(" ========== 2 consecutive PXD hits in a same layer ========= ");
          const TVectorD& pxd_predIntersect_1 = trk.getMeasuredStateOnPlaneFromRecoHit(infoPXD_1).getState();
          const double res_U_1 = pxd_1->getU() - pxd_predIntersect_1[3];
          const double res_V_1 = pxd_1->getV() - pxd_predIntersect_1[4];
          const TVectorD& pxd_predIntersect_2 = trk.getMeasuredStateOnPlaneFromRecoHit(infoPXD_2).getState();
          const double res_U_2 = pxd_2->getU() - pxd_predIntersect_2[3];
          const double res_V_2 = pxd_2->getV() - pxd_predIntersect_2[4];
          float over_U_PXD = res_U_2 - res_U_1;
          float over_V_PXD = res_V_2 - res_V_1;
          B2INFO("PXD residuals " << over_U_PXD << "   " << over_V_PXD);
          h_U_Res->Fill(over_U_PXD);
          h_V_Res->Fill(over_V_PXD);
          if (pxd_Layer_1 == 1 && pxd_Layer_2 == 1) {
            h_Lyr1[pxd_Ladder_1][pxd_Sensor_1]->Fill(pxd_1->getU(), pxd_1->getV());
            h_Fit_Lyr1[pxd_Ladder_1][pxd_Sensor_1]->Fill(pxd_predIntersect_1[3], pxd_predIntersect_1[4]);
            h_Lyr1[pxd_Ladder_2][pxd_Sensor_2]->Fill(pxd_2->getU(), pxd_2->getV());
            h_Fit_Lyr1[pxd_Ladder_2][pxd_Sensor_2]->Fill(pxd_predIntersect_2[3], pxd_predIntersect_2[4]);
          }
          if (pxd_Layer_1 == 2 && pxd_Layer_2 == 2) {
            h_Lyr2[pxd_Ladder_1][pxd_Sensor_1]->Fill(pxd_1->getU(), pxd_1->getV());
            h_Fit_Lyr2[pxd_Ladder_1][pxd_Sensor_1]->Fill(pxd_predIntersect_1[3], pxd_predIntersect_1[4]);
            h_Lyr2[pxd_Ladder_2][pxd_Sensor_2]->Fill(pxd_2->getU(), pxd_2->getV());
            h_Fit_Lyr2[pxd_Ladder_2][pxd_Sensor_2]->Fill(pxd_predIntersect_2[3], pxd_predIntersect_2[4]);
          }
        }
      }

    }

    //LOOKING FOR 2 CONSECUTIVE SVD HITS IN OVERLAPPING MODULES OF A SAME LAYER
    for (int i = 0; i < svdClusters.size(); i++) {
      const SVDCluster* svd_1 = svdClusters[i];
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
      for (int l = i + 1; l < svdClusters.size(); l++) {
        const SVDCluster* svd_2 = svdClusters[l];
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
        if (svd_Layer_1 == svd_Layer_2) {
          B2INFO(" ========== 2 consecutive SVD hits in a same layer ========= ");
          const TVectorD& svd_predIntersect_1 = trk.getMeasuredStateOnPlaneFromRecoHit(infoSVD_1).getState();
          const TVectorD& svd_predIntersect_2 = trk.getMeasuredStateOnPlaneFromRecoHit(infoSVD_2).getState();
          //Restricting to consecutive SVD u-clusters
          if (svd_1->isUCluster() == true && svd_2->isUCluster() == true) {
            const int strips_1 = svd_1->getSize();
            h_SVDstrips_Mult->Fill(strips_1);
            const int strips_2 = svd_2->getSize();
            h_SVDstrips_Mult->Fill(strips_2);
            const double res_U_1 = svd_1->getPosition() - svd_predIntersect_1[3];
            const double res_U_2 = svd_2->getPosition() - svd_predIntersect_2[3];
            float over_U_SVD = res_U_2 - res_U_1;
            B2INFO("SVD U residual =========> " << over_U_SVD);
            h_U_Res->Fill(over_U_SVD);
            if (svd_1->getSize() < 3 && svd_2->getSize() < 3) { //Consider only clusters with 3 or less strips involved
              h_U_Cl1Cl2_Res[svd_1->getSize() * svd_2->getSize()]->Fill(over_U_SVD);
            }
            if (svd_Layer_1 == 3 && svd_Layer_2 == 3) {
              h_Lyr3[svd_Ladder_1][svd_Sensor_1]->Fill(svd_1->getPosition(), 0.);
              h_Fit_Lyr3[svd_Ladder_1][svd_Sensor_1]->Fill(svd_predIntersect_1[3], 0.);
              h_Lyr3[svd_Ladder_2][svd_Sensor_2]->Fill(svd_2->getPosition(), 0.);
              h_Fit_Lyr3[svd_Ladder_2][svd_Sensor_2]->Fill(svd_predIntersect_2[3], 0.);
            }
            if (svd_Layer_1 == 4 && svd_Layer_2 == 4) {
              h_Lyr4[svd_Ladder_1][svd_Sensor_1]->Fill(svd_1->getPosition(), 0.);
              h_Fit_Lyr4[svd_Ladder_1][svd_Sensor_1]->Fill(svd_predIntersect_1[3], 0.);
              h_Lyr4[svd_Ladder_2][svd_Sensor_2]->Fill(svd_2->getPosition(), 0.);
              h_Fit_Lyr4[svd_Ladder_2][svd_Sensor_2]->Fill(svd_predIntersect_2[3], 0.);
            }
            if (svd_Layer_1 == 5 && svd_Layer_2 == 5) {
              h_Lyr5[svd_Ladder_1][svd_Sensor_1]->Fill(svd_1->getPosition(), 0.);
              h_Fit_Lyr5[svd_Ladder_1][svd_Sensor_1]->Fill(svd_predIntersect_1[3], 0.);
              h_Lyr5[svd_Ladder_2][svd_Sensor_1]->Fill(svd_1->getPosition(), 0.);
              h_Fit_Lyr5[svd_Ladder_2][svd_Sensor_2]->Fill(svd_predIntersect_2[3], 0.);
            }
            if (svd_Layer_1 == 6 && svd_Layer_2 == 6) {
              h_Lyr6[svd_Ladder_1][svd_Sensor_1]->Fill(svd_1->getPosition(), 0.);
              h_Fit_Lyr6[svd_Ladder_1][svd_Sensor_1]->Fill(svd_predIntersect_1[3], 0.);
              h_Lyr6[svd_Ladder_2][svd_Sensor_2]->Fill(svd_2->getPosition(), 0.);
              h_Fit_Lyr6[svd_Ladder_2][svd_Sensor_2]->Fill(svd_predIntersect_2[3], 0.);
            }
          }
          //Restricting to consecutive SVD v-clusters
          if (svd_1->isUCluster() != true && svd_2->isUCluster() != true) {
            const int strips_1 = svd_1->getSize();
            h_SVDstrips_Mult->Fill(strips_1);
            const int strips_2 = svd_2->getSize();
            h_SVDstrips_Mult->Fill(strips_2);
            const double res_V_1 = svd_1->getPosition() - svd_predIntersect_1[4];
            const double res_V_2 = svd_2->getPosition() - svd_predIntersect_2[4];
            float over_V_SVD = res_V_2 - res_V_1;
            B2INFO("SVD V residual =========> " << over_V_SVD);
            B2INFO("SVD V residual =========> " << over_V_SVD);
            h_V_Res->Fill(over_V_SVD);
            if (svd_1->getSize() < 3 && svd_2->getSize() < 3) {  //Consider only clusters with 3 or less strips involved
              h_V_Cl1Cl2_Res[svd_1->getSize() * svd_2->getSize()]->Fill(over_V_SVD);
            }
            if (svd_Layer_1 == 3 && svd_Layer_2 == 3) {
              h_Lyr3[svd_Ladder_1][svd_Sensor_1]->Fill(0., svd_1->getPosition());
              h_Fit_Lyr3[svd_Ladder_1][svd_Sensor_1]->Fill(0., svd_predIntersect_1[4]);
              h_Lyr3[svd_Ladder_2][svd_Sensor_2]->Fill(0., svd_2->getPosition());
              h_Fit_Lyr3[svd_Ladder_2][svd_Sensor_2]->Fill(0., svd_predIntersect_2[4]);
            }
            if (svd_Layer_1 == 4 && svd_Layer_2 == 4) {
              h_Lyr4[svd_Ladder_1][svd_Sensor_1]->Fill(0., svd_1->getPosition());
              h_Fit_Lyr4[svd_Ladder_1][svd_Sensor_1]->Fill(0., svd_predIntersect_1[4]);
              h_Lyr4[svd_Ladder_2][svd_Sensor_2]->Fill(0., svd_2->getPosition());
              h_Fit_Lyr4[svd_Ladder_2][svd_Sensor_2]->Fill(0., svd_predIntersect_2[4]);
            }
            if (svd_Layer_1 == 5 && svd_Layer_2 == 5) {
              h_Lyr5[svd_Ladder_1][svd_Sensor_1]->Fill(0., svd_1->getPosition());
              h_Fit_Lyr5[svd_Ladder_1][svd_Sensor_1]->Fill(0., svd_predIntersect_1[4]);
              h_Lyr5[svd_Ladder_2][svd_Sensor_2]->Fill(0., svd_2->getPosition());
              h_Fit_Lyr5[svd_Ladder_2][svd_Sensor_2]->Fill(0., svd_predIntersect_2[4]);
            }
            if (svd_Layer_1 == 6 && svd_Layer_2 == 6) {
              h_Lyr6[svd_Ladder_1][svd_Sensor_1]->Fill(0., svd_1->getPosition());
              h_Fit_Lyr6[svd_Ladder_1][svd_Sensor_1]->Fill(0., svd_predIntersect_1[4]);
              h_Lyr6[svd_Ladder_2][svd_Sensor_2]->Fill(0., svd_2->getPosition());
              h_Fit_Lyr6[svd_Ladder_2][svd_Sensor_2]->Fill(0., svd_predIntersect_2[4]);
            }
          }

        }
      }
    }

  }

}




void OverlapResidualsModule::terminate()
{
  B2INFO("________________________ FILL OUTPUT FILE ____________________");
  h_SVDstrips_Mult->Scale(1. / h_SVDstrips_Mult->Integral());
  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    for (int i = 1; i <= 5; i++) {
      for (int j = 1; j <= 16; j++) {
        h_Lyr6[j][i]->Write();
        h_Fit_Lyr6[j][i]->Write();
      }
    }

    for (int i = 1; i <= 4; i++) {
      for (int j = 1; j <= 12; j++) {
        h_Lyr5[j][i]->Write();
        h_Fit_Lyr5[j][i]->Write();
      }
    }

    for (int i = 1; i <= 3; i++) {
      for (int j = 1; j <= 10; j++) {
        h_Lyr4[j][i]->Write();
        h_Fit_Lyr4[j][i]->Write();
      }
    }

    for (int i = 1; i <= 2; i++) {

      for (int j = 1; j <= 7; j++) {
        h_Lyr3[j][i]->Write();
        h_Fit_Lyr3[j][i]->Write();
      }

      for (int j = 1; j <= 12; j++) {
        h_Lyr2[j][i]->Write();
        h_Fit_Lyr2[j][i]->Write();
      }

      for (int j = 1; j <= 8; j++) {
        h_Lyr1[j][i]->Write();
        h_Fit_Lyr1[j][i]->Write();
      }

    }

    h_U_Res->Scale(1. / h_U_Res->Integral());
    h_U_Res->Write();
    h_V_Res->Scale(1. / h_V_Res->Integral());
    h_V_Res->Write();
    h_SVDstrips_Mult->Write();
    for (int i = 1; i <= 9; i++) {
      if (i == 5 || i == 7 || i == 8) {
        continue; //The product of cluster sizes cannot be 5, 7, 9
      }
      h_U_Cl1Cl2_Res[i]->Scale(1. / h_U_Cl1Cl2_Res[i]->Integral());
      h_U_Cl1Cl2_Res[i]->Write();
      h_V_Cl1Cl2_Res[i]->Scale(1. / h_V_Cl1Cl2_Res[i]->Integral());
      h_V_Cl1Cl2_Res[i]->Write();
    }

    m_rootFilePtr->Close();
  }
}
