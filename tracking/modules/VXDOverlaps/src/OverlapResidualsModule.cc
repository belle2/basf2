/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Filippo Dattola                                          *
 *                                                                        *

 * Module to study VXD hits from overlapping sensors of a same VXD layer. *                                                                     *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/VXDOverlaps/OverlapResidualsModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <mdst/dataobjects/Track.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <genfit/TrackPoint.h>
#include <framework/gearbox/Const.h>
#include <vxd/dataobjects/VxdID.h>

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

  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("OverlapResiduals.root"));


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

  int n_reco = recoTrack.getEntries();

  gROOT->SetBatch();

  for (int j = 0 ; j < n_reco; j++) {

    if (! recoTrack[j]->wasFitSuccessful()) continue;

    const vector<PXDCluster* > pxdClusters = recoTrack[j]->getPXDHitList();

    const vector<SVDCluster* > svdClusters = recoTrack[j]->getSVDHitList();


    B2INFO("FITTED TRACK:   NUMBER OF PXD HITS = " << pxdClusters.size() << "    NUMBER OF SVD HITS = " << svdClusters.size());

    //LOOP ON PXD HITS OF j-TRACK
    for (int i = 0; i < pxdClusters.size(); i++) {

      const PXDCluster* PXD_1 = pxdClusters[i];

      const RecoHitInformation* infoPXD_1 = recoTrack[j]->getRecoHitInformation(PXD_1);

      if (!infoPXD_1) continue;

      const auto* hitTrackPoint_1 = recoTrack[j]->getCreatedTrackPoint(infoPXD_1);

      const auto* fittedResult_1 = hitTrackPoint_1->getFitterInfo();

      if (!fittedResult_1) continue;

      const VxdID PXDid_1 = PXD_1->getSensorID();

      const unsigned short PXDLayer_1 = PXDid_1.getLayerNumber();

      const unsigned short PXDLadder_1 = PXDid_1.getLadderNumber();

      const unsigned short PXDSensor_1 = PXDid_1.getSensorNumber();

      for (int l = i + 1; l < pxdClusters.size(); l++) {

        const PXDCluster* PXD_2 = pxdClusters[l];

        const RecoHitInformation* infoPXD_2 = recoTrack[j]->getRecoHitInformation(PXD_2);

        if (!infoPXD_2) continue;

        const auto* hitTrackPoint_2 = recoTrack[j]->getCreatedTrackPoint(infoPXD_2);

        const auto* fittedResult_2 = hitTrackPoint_2->getFitterInfo();

        if (!fittedResult_2) continue;

        const VxdID PXDid_2 = PXD_2->getSensorID();

        const unsigned short PXDLayer_2 = PXDid_2.getLayerNumber();

        const unsigned short PXDLadder_2 = PXDid_2.getLadderNumber();

        const unsigned short PXDSensor_2 = PXDid_2.getSensorNumber();

        if (PXDLayer_1 == PXDLayer_2) {

          B2INFO(" ========== 2 consecutive PXD hits in a same layer ========= ");

          const TVectorD& PXDpredIntersect_1 = recoTrack[j]->getMeasuredStateOnPlaneFromRecoHit(infoPXD_1).getState();

          const double Res_U_1 = PXD_1->getU() - PXDpredIntersect_1[3];

          const double Res_V_1 = PXD_1->getV() - PXDpredIntersect_1[4];


          const TVectorD& PXDpredIntersect_2 = recoTrack[j]->getMeasuredStateOnPlaneFromRecoHit(infoPXD_2).getState();

          const double Res_U_2 = PXD_2->getU() - PXDpredIntersect_2[3];

          const double Res_V_2 = PXD_2->getV() - PXDpredIntersect_2[4];

          float Over_U_PXD = Res_U_2 - Res_U_1;

          float Over_V_PXD = Res_V_2 - Res_V_1;

          B2INFO("PXD residuals " << Over_U_PXD << "   " << Over_V_PXD);

          h_U_Res->Fill(Over_U_PXD);

          h_V_Res->Fill(Over_V_PXD);

          if (PXDLayer_1 == 1 && PXDLayer_2 == 1) {

            h_Lyr1[PXDLadder_1][PXDSensor_1]->Fill(PXD_1->getU(), PXD_1->getV());

            h_Fit_Lyr1[PXDLadder_1][PXDSensor_1]->Fill(PXDpredIntersect_1[3], PXDpredIntersect_1[4]);

            h_Lyr1[PXDLadder_2][PXDSensor_2]->Fill(PXD_2->getU(), PXD_2->getV());

            h_Fit_Lyr1[PXDLadder_2][PXDSensor_2]->Fill(PXDpredIntersect_2[3], PXDpredIntersect_2[4]);


          }


          if (PXDLayer_1 == 2 && PXDLayer_2 == 2) {

            h_Lyr2[PXDLadder_1][PXDSensor_1]->Fill(PXD_1->getU(), PXD_1->getV());

            h_Fit_Lyr2[PXDLadder_1][PXDSensor_1]->Fill(PXDpredIntersect_1[3], PXDpredIntersect_1[4]);

            h_Lyr2[PXDLadder_2][PXDSensor_2]->Fill(PXD_2->getU(), PXD_2->getV());

            h_Fit_Lyr2[PXDLadder_2][PXDSensor_2]->Fill(PXDpredIntersect_2[3], PXDpredIntersect_2[4]);


          }


        }

      }

    }

    for (int i = 0; i < svdClusters.size(); i++) {

      const SVDCluster* SVD_1 = svdClusters[i];

      const RecoHitInformation* infoSVD_1 = recoTrack[j]->getRecoHitInformation(SVD_1);

      if (!infoSVD_1) continue;

      const auto* hitTrackPoint_1 = recoTrack[j]->getCreatedTrackPoint(infoSVD_1);

      const auto* fittedResult_1 = hitTrackPoint_1->getFitterInfo();

      if (!fittedResult_1) continue;

      const VxdID SVDid_1 = SVD_1->getSensorID();

      const unsigned short SVDLayer_1 = SVDid_1.getLayerNumber();

      const unsigned short SVDLadder_1 = SVDid_1.getLadderNumber();

      const unsigned short SVDSensor_1 = SVDid_1.getSensorNumber();

      for (int l = i + 1; l < svdClusters.size(); l++) {

        const SVDCluster* SVD_2 = svdClusters[l];

        const RecoHitInformation* infoSVD_2 = recoTrack[j]->getRecoHitInformation(SVD_2);

        if (!infoSVD_2) continue;

        const auto* hitTrackPoint_2 = recoTrack[j]->getCreatedTrackPoint(infoSVD_2);

        const auto* fittedResult_2 = hitTrackPoint_2->getFitterInfo();

        if (!fittedResult_2) continue;

        const VxdID SVDid_2 = SVD_2->getSensorID();

        const unsigned short SVDLayer_2 = SVDid_2.getLayerNumber();

        const unsigned short SVDLadder_2 = SVDid_2.getLadderNumber();

        const unsigned short SVDSensor_2 = SVDid_2.getSensorNumber();

        if (SVDLayer_1 == SVDLayer_2) {

          B2INFO(" ========== 2 consecutive SVD hits in a same layer ========= ");

          const TVectorD& SVDpredIntersect_1 = recoTrack[j]->getMeasuredStateOnPlaneFromRecoHit(infoSVD_1).getState();

          const TVectorD& SVDpredIntersect_2 = recoTrack[j]->getMeasuredStateOnPlaneFromRecoHit(infoSVD_2).getState();

          if (SVD_1->isUCluster() == true && SVD_2->isUCluster() == true) {

            const int strips_1 = SVD_1->getSize();

            h_SVDstrips_Mult->Fill(strips_1);

            const int strips_2 = SVD_2->getSize();

            h_SVDstrips_Mult->Fill(strips_2);

            const double Res_U_1 = SVD_1->getPosition() - SVDpredIntersect_1[3];

            const double Res_U_2 = SVD_2->getPosition() - SVDpredIntersect_2[3];

            float Over_U_SVD = Res_U_2 - Res_U_1;

            B2INFO("SVD U residual =========> " << Over_U_SVD);

            h_U_Res->Fill(Over_U_SVD);

            if (SVD_1->getSize() < 3 && SVD_2->getSize() < 3) {

              h_U_Cl1Cl2_Res[SVD_1->getSize() * SVD_2->getSize()]->Fill(Over_U_SVD);

            }


            if (SVDLayer_1 == 3 && SVDLayer_2 == 3) {

              h_Lyr3[SVDLadder_1][SVDSensor_1]->Fill(SVD_1->getPosition(), 0.);

              h_Fit_Lyr3[SVDLadder_1][SVDSensor_1]->Fill(SVDpredIntersect_1[3], 0.);

              h_Lyr3[SVDLadder_2][SVDSensor_2]->Fill(SVD_2->getPosition(), 0.);

              h_Fit_Lyr3[SVDLadder_2][SVDSensor_2]->Fill(SVDpredIntersect_2[3], 0.);

            }

            if (SVDLayer_1 == 4 && SVDLayer_2 == 4) {

              h_Lyr4[SVDLadder_1][SVDSensor_1]->Fill(SVD_1->getPosition(), 0.);

              h_Fit_Lyr4[SVDLadder_1][SVDSensor_1]->Fill(SVDpredIntersect_1[3], 0.);

              h_Lyr4[SVDLadder_2][SVDSensor_2]->Fill(SVD_2->getPosition(), 0.);

              h_Fit_Lyr4[SVDLadder_2][SVDSensor_2]->Fill(SVDpredIntersect_2[3], 0.);

            }

            if (SVDLayer_1 == 5 && SVDLayer_2 == 5) {

              h_Lyr5[SVDLadder_1][SVDSensor_1]->Fill(SVD_1->getPosition(), 0.);

              h_Fit_Lyr5[SVDLadder_1][SVDSensor_1]->Fill(SVDpredIntersect_1[3], 0.);

              h_Lyr5[SVDLadder_2][SVDSensor_1]->Fill(SVD_1->getPosition(), 0.);

              h_Fit_Lyr5[SVDLadder_2][SVDSensor_2]->Fill(SVDpredIntersect_2[3], 0.);

            }

            if (SVDLayer_1 == 6 && SVDLayer_2 == 6) {

              h_Lyr6[SVDLadder_1][SVDSensor_1]->Fill(SVD_1->getPosition(), 0.);

              h_Fit_Lyr6[SVDLadder_1][SVDSensor_1]->Fill(SVDpredIntersect_1[3], 0.);

              h_Lyr6[SVDLadder_2][SVDSensor_2]->Fill(SVD_2->getPosition(), 0.);

              h_Fit_Lyr6[SVDLadder_2][SVDSensor_2]->Fill(SVDpredIntersect_2[3], 0.);


            }

          }


          if (SVD_1->isUCluster() != true && SVD_2->isUCluster() != true) {


            const int strips_1 = SVD_1->getSize();

            h_SVDstrips_Mult->Fill(strips_1);

            const int strips_2 = SVD_2->getSize();

            h_SVDstrips_Mult->Fill(strips_2);

            const double Res_V_1 = SVD_1->getPosition() - SVDpredIntersect_1[4];

            const double Res_V_2 = SVD_2->getPosition() - SVDpredIntersect_2[4];

            float Over_V_SVD = Res_V_2 - Res_V_1;


            B2INFO("SVD V residual =========> " << Over_V_SVD);

            B2INFO("SVD V residual =========> " << Over_V_SVD);


            h_V_Res->Fill(Over_V_SVD);

            if (SVD_1->getSize() < 3 && SVD_2->getSize() < 3) {

              h_V_Cl1Cl2_Res[SVD_1->getSize() * SVD_2->getSize()]->Fill(Over_V_SVD);

            }

            if (SVDLayer_1 == 3 && SVDLayer_2 == 3) {

              h_Lyr3[SVDLadder_1][SVDSensor_1]->Fill(0., SVD_1->getPosition());

              h_Fit_Lyr3[SVDLadder_1][SVDSensor_1]->Fill(0., SVDpredIntersect_1[4]);

              h_Lyr3[SVDLadder_2][SVDSensor_2]->Fill(0., SVD_2->getPosition());

              h_Fit_Lyr3[SVDLadder_2][SVDSensor_2]->Fill(0., SVDpredIntersect_2[4]);


            }

            if (SVDLayer_1 == 4 && SVDLayer_2 == 4) {

              h_Lyr4[SVDLadder_1][SVDSensor_1]->Fill(0., SVD_1->getPosition());

              h_Fit_Lyr4[SVDLadder_1][SVDSensor_1]->Fill(0., SVDpredIntersect_1[4]);

              h_Lyr4[SVDLadder_2][SVDSensor_2]->Fill(0., SVD_2->getPosition());

              h_Fit_Lyr4[SVDLadder_2][SVDSensor_2]->Fill(0., SVDpredIntersect_2[4]);

            }

            if (SVDLayer_1 == 5 && SVDLayer_2 == 5) {

              h_Lyr5[SVDLadder_1][SVDSensor_1]->Fill(0., SVD_1->getPosition());

              h_Fit_Lyr5[SVDLadder_1][SVDSensor_1]->Fill(0., SVDpredIntersect_1[4]);

              h_Lyr5[SVDLadder_2][SVDSensor_2]->Fill(0., SVD_2->getPosition());

              h_Fit_Lyr5[SVDLadder_2][SVDSensor_2]->Fill(0., SVDpredIntersect_2[4]);


            }

            if (SVDLayer_1 == 6 && SVDLayer_2 == 6) {

              h_Lyr6[SVDLadder_1][SVDSensor_1]->Fill(0., SVD_1->getPosition());

              h_Fit_Lyr6[SVDLadder_1][SVDSensor_1]->Fill(0., SVDpredIntersect_1[4]);

              h_Lyr6[SVDLadder_2][SVDSensor_2]->Fill(0., SVD_2->getPosition());

              h_Fit_Lyr6[SVDLadder_2][SVDSensor_2]->Fill(0., SVDpredIntersect_2[4]);


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

      if (i == 5 || i == 7 || i == 8) continue; //The product of cluster sizes cannot be 5, 7, 9

      h_U_Cl1Cl2_Res[i]->Scale(1. / h_U_Cl1Cl2_Res[i]->Integral());
      h_U_Cl1Cl2_Res[i]->Write();
      h_V_Cl1Cl2_Res[i]->Scale(1. / h_V_Cl1Cl2_Res[i]->Integral());
      h_V_Cl1Cl2_Res[i]->Write();

    }

    m_rootFilePtr->Close();

  }
}
