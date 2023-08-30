/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/calibration/SVDTimeValidationAlgorithm.h>

#include <svd/dbobjects/SVDCoGCalibrationFunction.h>
#include <svd/calibration/SVDCoGTimeCalibrations.h>

#include <TF1.h>
#include <TProfile.h>
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include <framework/logging/Logger.h>
#include <iostream>
#include <TString.h>
#include <TFitResult.h>

using namespace Belle2;

SVDTimeValidationAlgorithm::SVDTimeValidationAlgorithm() :
  CalibrationAlgorithm("SVDTimeValidationCollector")
{
  setDescription("SVDTimeValidation calibration algorithm");
}

CalibrationAlgorithm::EResult SVDTimeValidationAlgorithm::calibrate()
{

  gROOT->SetBatch(true);

  gStyle->SetFillColor(0);
  gStyle->SetFillStyle(0);
  gStyle->SetOptStat(1111);
  gStyle->SetOptFit(1111);

  char alg[100] = {};
  std::sscanf(getPrefix().data(), "SVDTimeValidationCollector_%98s", alg);

  FileStat_t info;
  int cal_rev = 1;
  while (gSystem->GetPathInfo(Form("algorithm_svdTimeValidation_%s_output_rev_%d.root", alg, cal_rev), info) == 0)
    cal_rev++;
  std::unique_ptr<TFile> f(new TFile(Form("algorithm_svdTimeValidation_%s_output_rev_%d.root", alg, cal_rev), "RECREATE"));

  TString outPDF = Form("algorithm_svdTimeValidation_%s_output_rev_%d.pdf", alg, cal_rev);
  TCanvas c1("c1", "c1", 640, 480);
  c1.Print(outPDF + "[");
  TPad onePad("onePad", "onePad", 0, 0, 1, 1, kWhite);
  onePad.SetFillColor(0);
  onePad.SetBorderMode(0);
  onePad.SetBorderSize(2);
  onePad.SetRightMargin(0.1339713);
  onePad.SetBottomMargin(0.15);
  onePad.SetFrameBorderMode(0);
  onePad.SetFrameBorderMode(0);
  onePad.SetNumber(1);
  onePad.Draw();

  auto hEventT0 = getObjectPtr<TH1F>("hEventT0");
  float eventT0_mean = 0;
  float eventT0_rms = 0;
  if (hEventT0) {
    eventT0_mean = hEventT0->GetMean();
    eventT0_rms = hEventT0->GetRMS();
  } else
    B2ERROR("Histogram with Event T0 not found");

  B2DEBUG(27, "Histogram: " << hEventT0->GetName() <<
          " Entries (n. clusters): " << hEventT0->GetEntries() <<
          " Mean: " << eventT0_mean);

  auto __hClsTimeOnTracks__     = getObjectPtr<TH2F>("__hClsTimeOnTracks__");
  // auto __hClsTimeAll__          = getObjectPtr<TH2F>("__hClsTimeAll__");
  // auto __hClsDiffTimeOnTracks__ = getObjectPtr<TH2F>("__hClsDiffTimeOnTracks__");
  auto __hBinToSensorMap__      = getObjectPtr<TH1F>("__hBinToSensorMap__");
  auto __hClusterSizeVsTimeResidual__ = getObjectPtr<TH3F>("__hClusterSizeVsTimeResidual__");

  // draw shift
  int nSensors   = __hClusterSizeVsTimeResidual__->GetNbinsZ();
  int maxClsSize = __hClusterSizeVsTimeResidual__->GetNbinsY();
  TH2F* hDrawShift = new TH2F("hDrawShift", "Cluster time shift",
                              nSensors, 0.5, nSensors + 0.5, maxClsSize, 0.5, maxClsSize + 0.5);
  hDrawShift->GetZaxis()->SetTitle("Cluster Time Shift (in ns)");
  hDrawShift->GetYaxis()->SetTitle("Cluster Size");
  hDrawShift->GetXaxis()->SetTitle("Sensor");

  for (int ij = 0; ij < (__hBinToSensorMap__->GetNbinsX()); ij++) {
    {
      {
        {

          auto binLabel = __hBinToSensorMap__->GetXaxis()->GetBinLabel(ij + 1);
          char side;
          int layer_num, ladder_num, sensor_num;
          std::sscanf(binLabel, "L%dL%dS%d%c", &layer_num, &ladder_num, &sensor_num, &side);

          B2INFO("Projecting for Sensor: " << binLabel << " with Bin Number: " << ij + 1);

          auto hClsTimeOnTracks = (TH1D*)__hClsTimeOnTracks__->ProjectionX("hClsTimeOnTracks_tmp", ij + 1, ij + 1);
          if (!hClsTimeOnTracks)
            B2ERROR("Histogram " << Form("clsTimeOnTracks__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side) << " not found");

          hClsTimeOnTracks->SetName(Form("clsTimeOnTracks__L%dL%dS%d%c", layer_num, ladder_num, sensor_num, side));
          char sidePN = (side == 'U' ? 'P' : 'N');
          hClsTimeOnTracks->SetTitle(Form("clsTimeOnTracks in %d.%d.%d %c/%c", layer_num, ladder_num, sensor_num, side, sidePN));
          hClsTimeOnTracks->SetDirectory(0);

          float clsTimeOnTracks_mean = hClsTimeOnTracks->GetMean();
          auto deviation = (clsTimeOnTracks_mean - eventT0_mean) / eventT0_rms;

          B2DEBUG(27, "Histogram: " << hClsTimeOnTracks->GetName() <<
                  " Entries (n. clusters): " << hClsTimeOnTracks->GetEntries() <<
                  " Mean: " << clsTimeOnTracks_mean <<
                  " Deviation: " << deviation << " EventT0 RMS");
          if (std::fabs(deviation) > m_allowedDeviationMean)
            B2ERROR("Histogram: " << hClsTimeOnTracks->GetName() << " deviates from EventT0 by" << deviation << " times the EventT0 RMS");

          __hClusterSizeVsTimeResidual__->GetZaxis()->SetRange(ij + 1, ij + 1);
          TH2D* hClusterSizeVsTimeResidual = (TH2D*)__hClusterSizeVsTimeResidual__->Project3D("yxe");
          hClusterSizeVsTimeResidual->SetName(Form("clusterSizeVsTimeResidual__L%dS%dS%c", layer_num, sensor_num, side));
          hClusterSizeVsTimeResidual->SetTitle(Form("ClusterSize vs Time Residual in %d.%d %c/%c", layer_num, sensor_num, side, sidePN));
          hClusterSizeVsTimeResidual->SetDirectory(0);
          onePad.Clear();
          onePad.cd();
          hClusterSizeVsTimeResidual->Draw("COLZ");
          c1.Print(outPDF, TString("Title:") + hClusterSizeVsTimeResidual->GetName());
          f->cd();
          hClusterSizeVsTimeResidual->Write();
          for (int clSize = 1; clSize <= maxClsSize; clSize++) {
            TH1D* hist = (TH1D*)hClusterSizeVsTimeResidual->ProjectionX("tmp", clSize, clSize, "");
            hist->SetDirectory(0);
            hDrawShift->SetBinContent(ij + 1, clSize, hist->GetMean());
            delete hist;
          }
          hDrawShift->GetXaxis()->SetBinLabel(ij + 1, binLabel);

          delete hClsTimeOnTracks;
          delete hClusterSizeVsTimeResidual;
        }
      }
    }
  }
  onePad.Clear();
  onePad.cd();
  hDrawShift->Draw("COLZ");
  c1.Print(outPDF, TString("Title:") + hDrawShift->GetName());
  f->cd();
  hDrawShift->Write();
  c1.Print(outPDF + "]");
  f->Close();
  return c_OK;
}
