/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/calibration/SVDClusterTimeShifterAlgorithm.h>

#include <svd/dbobjects/SVDClusterTimeShifter.h>

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

SVDClusterTimeShifterAlgorithm::SVDClusterTimeShifterAlgorithm(const std::string& str) :
  CalibrationAlgorithm("SVDClusterTimeShifterCollector")
  , m_id(str)
{
  setDescription("SVDClusterTimeShifter calibration algorithm");
}

CalibrationAlgorithm::EResult SVDClusterTimeShifterAlgorithm::calibrate()
{

  gROOT->SetBatch(true);

  gStyle->SetFillColor(0);
  gStyle->SetFillStyle(0);
  gStyle->SetOptStat(1111);
  gStyle->SetOptFit(1111);

  auto payload = new Belle2::SVDClusterTimeShifter("SVDClusterTimeShifter_" + m_id);

  // single gaus fit function
  TF1* singleGaus = new TF1("singleGaus", mySingleGaus, -50., 50., 4);

  // double gaus fit function
  TF1* doubleGaus = new TF1("doubleGaus", myDoubleGaus, -50., 50., 7);
  doubleGaus->SetParName(0, "N");
  doubleGaus->SetParName(1, "f");
  doubleGaus->SetParName(2, "#mu_{1}");
  doubleGaus->SetParName(3, "#sigma_{1}");
  doubleGaus->SetParName(4, "#mu_{2}");
  doubleGaus->SetParName(5, "#sigma_{2}");
  doubleGaus->SetParName(6, "C");
  doubleGaus->SetParLimits(1, 0.01, 0.99);
  doubleGaus->SetParLimits(3, 0.5, 25.);
  doubleGaus->SetParLimits(5, 0.5, 25.);


  auto __hBinToSensorMap__ = getObjectPtr<TH1F>("__hBinToSensorMap__");

  for (auto alg : m_timeAlgorithms) {

    B2INFO("Calculating shift for algorithm " << alg);

    auto __hClusterSizeVsTimeResidual__ = getObjectPtr<TH3F>(("__hClusterSizeVsTimeResidual__" + alg).Data());

    int nSensors = __hClusterSizeVsTimeResidual__->GetNbinsZ();
    int maxClsSize = __hClusterSizeVsTimeResidual__->GetNbinsY();

    // map : shift values
    std::map< TString, std::vector<double> > shiftValues;

    // draw shift
    TH2F* hDrawShift = new TH2F(("hDrawShift_" + alg).Data(),
                                ("Cluster time shift in " + alg).Data(),
                                nSensors, +0.5, nSensors + 0.5, maxClsSize, +0.5, maxClsSize + 0.5);
    hDrawShift->GetZaxis()->SetTitle("Cluster Time Shift (in ns)");
    hDrawShift->GetYaxis()->SetTitle("Cluster Size");
    hDrawShift->GetXaxis()->SetTitle("Sensor");

    FileStat_t info;
    int cal_rev = 1;
    while (gSystem->GetPathInfo(Form("algorithm_svdClusterTimeShifter_%s_output_rev_%d.root", alg.Data(), cal_rev), info) == 0)
      cal_rev++;
    std::unique_ptr<TFile> f(new TFile(Form("algorithm_svdClusterTimeShifter_%s_output_rev_%d.root", alg.Data(), cal_rev), "RECREATE"));

    TString outPDF = Form("algorithm_svdClusterTimeShifter_%s_output_rev_%d.pdf", alg.Data(), cal_rev);
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

    for (int ij = 0; ij < nSensors; ij++) {

      auto binLabel = __hBinToSensorMap__->GetXaxis()->GetBinLabel(ij + 1);
      char side;
      int layer_num, sensor_num;
      std::sscanf(binLabel, "L%dS%dS%c", &layer_num, &sensor_num, &side);

      B2INFO("Projecting for Sensor: " << binLabel << " with Bin Number: " << ij + 1);

      __hClusterSizeVsTimeResidual__->GetZaxis()->SetRange(ij + 1, ij + 1);
      TH2D* hClusterSizeVsTimeResidual = (TH2D*)__hClusterSizeVsTimeResidual__->Project3D("yxe");
      hClusterSizeVsTimeResidual->SetName(Form("clusterSizeVsTimeResidual__L%dS%dS%c", layer_num, sensor_num, side));
      char sidePN = (side == 'U' ? 'P' : 'N');
      hClusterSizeVsTimeResidual->SetTitle(Form("ClusterSize vs Time Residual in %d.%d %c/%c", layer_num, sensor_num, side, sidePN));
      hClusterSizeVsTimeResidual->SetDirectory(0);

      for (int clSize = 1; clSize <= maxClsSize; clSize++) {

        TH1D* hist = (TH1D*)hClusterSizeVsTimeResidual->ProjectionX("tmp", clSize, clSize, "");
        hist->SetName(Form("clusterTimeResidual__L%dS%dS%c_Sz%d", layer_num, sensor_num, side, clSize));
        hist->SetTitle(Form("Cluster Time Residual for Size %d in %d.%d %c/%c", clSize, layer_num, sensor_num, side, sidePN));
        hist->SetDirectory(0);

        B2INFO("Histogram: " << hist->GetName() <<
               " Entries (n. clusters): " << hist->GetEntries());
        if (hist->GetEntries() < m_minEntries) {
          B2INFO("Histogram: " << hist->GetName() <<
                 " Entries (n. clusters): " << hist->GetEntries() <<
                 " Entries required: " << m_minEntries);
          B2WARNING("Not enough data, adding one run to the collector");
          return c_NotEnoughData;
        }

        onePad.Clear();
        onePad.cd();
        hist->Draw();

        if (hist->GetMaximum() < 200.) {
          int rebinValue = 200. / hist->GetMaximum() + 1.;
          while ((hist->GetNbinsX()) % rebinValue != 0) rebinValue++;
          hist->Rebin(rebinValue);
        }

        int fitCount = 0;
        bool singleStatus, doubleStatus;
        while (fitCount++ < 5) {
          double histMean = hist->GetMean();
          double histStd  = hist->GetStdDev();
          singleGaus->SetParameter(0, hist->GetSumOfWeights() * hist->GetBinWidth(1));
          singleGaus->SetParameter(1, histMean);
          singleGaus->SetParameter(2, histStd * 0.75);
          singleGaus->SetParameter(3, 1.);
          singleGaus->SetParLimits(1, histMean - histStd, histMean + histStd);
          auto statusSingleFull = hist->Fit("singleGaus", "SNLMEQ", "",
                                            histMean - 2. * histStd, histMean + 2. * histStd);
          singleStatus = statusSingleFull->IsValid();

          doubleGaus->SetParameter(0, hist->GetSumOfWeights() * hist->GetBinWidth(1));
          doubleGaus->SetParameter(1, 0.95);
          doubleGaus->SetParameter(2, singleGaus->GetParameter(1));
          doubleGaus->SetParameter(3, std::fabs(singleGaus->GetParameter(2)));
          doubleGaus->SetParameter(4, singleGaus->GetParameter(1) - 3.);
          doubleGaus->SetParameter(5, std::fabs(singleGaus->GetParameter(2)) + 5.);
          doubleGaus->SetParameter(6, 10.);
          doubleGaus->SetParLimits(2,
                                   doubleGaus->GetParameter(2) - m_allowedDeviationMean,
                                   doubleGaus->GetParameter(2) + m_allowedDeviationMean);
          doubleGaus->SetParLimits(4,
                                   doubleGaus->GetParameter(4) - m_allowedDeviationMean,
                                   doubleGaus->GetParameter(4) + m_allowedDeviationMean);

          auto statusDoubleFull = hist->Fit("doubleGaus", "SLMEQ");
          doubleStatus = statusDoubleFull->IsValid();
          if (doubleStatus) break;
          int rebinValue = 2;
          while ((hist->GetNbinsX()) % rebinValue != 0) rebinValue++;
          hist->Rebin(rebinValue);
        }
        c1.Print(outPDF, TString("Title:") + hist->GetName());
        f->cd();
        hist->Write();

        double fillShiftVal = m_allowedDeviationMean + 1.;

        if (doubleStatus)
          fillShiftVal = (doubleGaus->GetParameter(1) > 0.5 ?
                          doubleGaus->GetParameter(2) : doubleGaus->GetParameter(4));
        else if (singleStatus) {
          fillShiftVal = singleGaus->GetParameter(1);
          B2WARNING("Fit failed for " << hist->GetName() <<
                    "; using mean from single gaus fit. ");
        }

        if (std::fabs(fillShiftVal) > m_allowedDeviationMean) {
          B2WARNING("Shift valus is more than allowed in " <<
                    hist->GetName() << " : " <<
                    shiftValues[binLabel].back() <<
                    "; using mean of the histogram.");
          fillShiftVal = hist->GetMean();
        }

        shiftValues[binLabel].push_back(1. * int(1000. * fillShiftVal) / 1000.);
        hDrawShift->SetBinContent(ij + 1, clSize, shiftValues[binLabel].back());
        hDrawShift->GetXaxis()->SetBinLabel(ij + 1, binLabel);

        delete hist;
      } // loop over cluster size

      f->cd();
      hClusterSizeVsTimeResidual->Write();
      delete hClusterSizeVsTimeResidual;
    } // loop over sensors

    for (auto item : shiftValues)
      payload->setClusterTimeShift(alg, item.first, item.second);

    onePad.cd();
    hDrawShift->GetXaxis()->LabelsOption("v");
    hDrawShift->SetStats(0);
    hDrawShift->Draw("colz");
    c1.Print(outPDF, TString("Title:") + hDrawShift->GetName());

    f->cd();
    hDrawShift->Write();
    delete hDrawShift;

    c1.Print(outPDF + "]");
    f->Close();
  } // loop over algorithms

  saveCalibration(payload, "SVDClusterTimeShifter");

  return c_OK;
}
