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
#include "TPaveStats.h"
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
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);

  FileStat_t info;
  int cal_rev = 1;
  while (gSystem->GetPathInfo(Form("algorithm_svdClusterTimeShifter_%s_output_rev_%d.root", m_timeAlgorithms[0].Data(), cal_rev),
                              info) == 0)
    cal_rev++;

  auto payload = new Belle2::SVDClusterTimeShifter(Form("SVDClusterTimeShifter_%s_rev_%d", m_id.data(), cal_rev));

  // single gaus fit function
  TF1* fn_singleGaus = new TF1("fn_singleGaus", singleGaus, -50., 50., 4);
  fn_singleGaus->SetLineColor(kGreen + 2);
  fn_singleGaus->SetParName(0, "N");
  fn_singleGaus->SetParName(1, "#mu");
  fn_singleGaus->SetParName(2, "#sigma");
  fn_singleGaus->SetParName(3, "C");

  // double gaus fit function
  TF1* fn_doubleGaus = new TF1("fn_doubleGaus", doubleGaus, -50., 50., 7);
  fn_doubleGaus->SetLineColor(kRed + 2);
  fn_doubleGaus->SetParName(0, "N");
  fn_doubleGaus->SetParName(1, "f");
  fn_doubleGaus->SetParName(2, "#mu_{1}");
  fn_doubleGaus->SetParName(3, "#sigma_{1}");
  fn_doubleGaus->SetParName(4, "#mu_{2}");
  fn_doubleGaus->SetParName(5, "#sigma_{2}");
  fn_doubleGaus->SetParName(6, "C");
  fn_doubleGaus->SetParLimits(1, 0.01, 0.99);
  fn_doubleGaus->SetParLimits(3, 0.5, 25.);
  fn_doubleGaus->SetParLimits(5, 0.5, 25.);


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
      hClusterSizeVsTimeResidual->SetTitle(Form("ClusterSize vs Time Residual in L%d.S%d %c/%c", layer_num, sensor_num, side, sidePN));
      hClusterSizeVsTimeResidual->SetDirectory(0);

      for (int clSize = 1; clSize <= maxClsSize; clSize++) {

        TH1D* hist = (TH1D*)hClusterSizeVsTimeResidual->ProjectionX("tmp", clSize, clSize, "");
        hist->SetName(Form("clusterTimeResidual__L%dS%dS%c_Sz%d", layer_num, sensor_num, side, clSize));
        hist->SetTitle(Form("Cluster Time Residual for Size %d in L%d.S%d %c/%c", clSize, layer_num, sensor_num, side, sidePN));
        hist->SetDirectory(0);

        B2INFO("Histogram: " << hist->GetName() <<
               " Entries (n. clusters): " << hist->GetEntries());
        if (hist->GetEntries() < m_minEntries) {
          B2INFO("Histogram: " << hist->GetName() <<
                 " Entries (n. clusters): " << hist->GetEntries() <<
                 " Entries required: " << m_minEntries);
          B2WARNING("Not enough data, adding one run to the collector");
          delete hDrawShift;
          c1.Print(outPDF + "]");
          f->Close();
          gSystem->Unlink(Form("algorithm_svdClusterTimeShifter_%s_output_rev_%d.root", m_timeAlgorithms[0].Data(), cal_rev));
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
        bool isSingleGausFitValid, isDoubleGausFitValid;
        while (fitCount++ < 5) {
          double histMean = hist->GetMean();
          double histStd  = hist->GetStdDev();
          fn_singleGaus->SetParameter(0, hist->GetSumOfWeights() * hist->GetBinWidth(1));
          fn_singleGaus->SetParameter(1, histMean);
          fn_singleGaus->SetParameter(2, histStd * 0.75);
          fn_singleGaus->SetParameter(3, 1.);
          fn_singleGaus->SetParLimits(1, histMean - histStd, histMean + histStd);
          auto singleGausFitStatus = hist->Fit("fn_singleGaus", "SQ", "",
                                               histMean - 2. * histStd, histMean + 2. * histStd);
          isSingleGausFitValid = singleGausFitStatus->IsValid();

          fn_doubleGaus->SetParameter(0, hist->GetSumOfWeights() * hist->GetBinWidth(1));
          fn_doubleGaus->SetParameter(1, 0.95);
          fn_doubleGaus->SetParameter(2, fn_singleGaus->GetParameter(1));
          fn_doubleGaus->SetParameter(3, std::fabs(fn_singleGaus->GetParameter(2)));
          fn_doubleGaus->SetParameter(4, fn_singleGaus->GetParameter(1) - 3.);
          fn_doubleGaus->SetParameter(5, std::fabs(fn_singleGaus->GetParameter(2)) + 5.);
          fn_doubleGaus->SetParameter(6, 10.);
          fn_doubleGaus->SetParLimits(2,
                                      fn_doubleGaus->GetParameter(2) - m_maximumAllowedShift,
                                      fn_doubleGaus->GetParameter(2) + m_maximumAllowedShift);
          fn_doubleGaus->SetParLimits(4,
                                      fn_doubleGaus->GetParameter(4) - m_maximumAllowedShift,
                                      fn_doubleGaus->GetParameter(4) + m_maximumAllowedShift);

          auto doubleGausFitStatus = hist->Fit("fn_doubleGaus", "SQ+");
          isDoubleGausFitValid = doubleGausFitStatus->IsValid();
          if (isDoubleGausFitValid) break;
          int rebinValue = 2;
          while ((hist->GetNbinsX()) % rebinValue != 0) rebinValue++;
          hist->Rebin(rebinValue);
        }

        TPaveStats* ptstats = new TPaveStats(0.55, 0.73, 0.85, 0.88, "brNDC");
        ptstats->SetName("stats1");
        ptstats->SetBorderSize(1);
        ptstats->SetFillColor(0);
        ptstats->SetTextAlign(12);
        ptstats->SetTextFont(42);
        ptstats->SetTextColor(kGreen + 2);
        ptstats->SetOptStat(11);
        ptstats->SetParent(hist);
        ptstats->Draw();
        ptstats->AddText("Single Gaus");
        for (int npar = 0; npar < (fn_singleGaus->GetNpar()); npar++)
          ptstats->AddText(TString::Format("%s = %.3f #pm %.4f",
                                           fn_singleGaus->GetParName(npar),
                                           fn_singleGaus->GetParameter(npar),
                                           fn_singleGaus->GetParError(npar)));
        ptstats = new TPaveStats(0.55, 0.49, 0.85, 0.73, "brNDC");
        ptstats->SetName("stats2");
        ptstats->SetBorderSize(1);
        ptstats->SetFillColor(0);
        ptstats->SetTextAlign(12);
        ptstats->SetTextFont(42);
        ptstats->SetTextColor(kRed + 2);
        ptstats->SetOptStat(11);
        ptstats->SetParent(hist);
        ptstats->Draw();
        ptstats->AddText("Double Gaus");
        for (int npar = 0; npar < (fn_doubleGaus->GetNpar()); npar++)
          ptstats->AddText(TString::Format("%s = %.3f #pm %.4f",
                                           fn_doubleGaus->GetParName(npar),
                                           fn_doubleGaus->GetParameter(npar),
                                           fn_doubleGaus->GetParError(npar)));
        ptstats = new TPaveStats(0.55, 0.43, 0.85, 0.49, "brNDC");
        ptstats->SetName("stats3");
        ptstats->SetBorderSize(1);
        ptstats->SetFillColor(0);
        ptstats->SetTextAlign(12);
        ptstats->SetTextFont(42);
        ptstats->SetTextColor(kBlue + 2);
        ptstats->SetOptStat(11);
        ptstats->SetParent(hist);
        ptstats->Draw();

        // setting `fillShiftVal` a higher than `m_maximumAllowedShift`. If both fit statuses are invalid,
        // then this condition will set shift value to the histogram mean.
        double fillShiftVal = m_maximumAllowedShift + 1.;

        if (isDoubleGausFitValid) {
          fillShiftVal = (fn_doubleGaus->GetParameter(1) > 0.5 ?
                          fn_doubleGaus->GetParameter(2) : fn_doubleGaus->GetParameter(4));
          ptstats->AddText(TString::Format("#splitline{Shift Value from Double Gaus}{%.3f}", fillShiftVal));
        } else if (isSingleGausFitValid) {
          fillShiftVal = fn_singleGaus->GetParameter(1);
          B2WARNING("Fit failed for " << hist->GetName() <<
                    "; using mean from single gaus fit. ");
          ptstats->AddText(TString::Format("#splitline{Shift Value from Single Gaus}{%.3f}", fillShiftVal));
        }

        if (std::fabs(fillShiftVal) > m_maximumAllowedShift) {
          B2WARNING("Shift value is more than allowed or fit failed in " <<
                    hist->GetName() << " : " <<
                    shiftValues[binLabel].back() <<
                    "; using mean of the histogram.");
          fillShiftVal = hist->GetMean();
          ptstats->AddText(TString::Format("#splitline{Shift Value from Histogram Mean}{%.3f}", fillShiftVal));
        }

        shiftValues[binLabel].push_back(1. * int(1000. * fillShiftVal) / 1000.);
        hDrawShift->SetBinContent(ij + 1, clSize, shiftValues[binLabel].back());
        hDrawShift->GetXaxis()->SetBinLabel(ij + 1, binLabel);

        c1.Print(outPDF, TString("Title:") + hist->GetName());
        f->cd();
        hist->Write();

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
