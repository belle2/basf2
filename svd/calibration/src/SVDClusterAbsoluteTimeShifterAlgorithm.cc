/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <svd/calibration/SVDClusterAbsoluteTimeShifterAlgorithm.h>

#include <svd/dbobjects/SVDAbsoluteClusterTimeShift.h>

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

SVDClusterAbsoluteTimeShifterAlgorithm::SVDClusterAbsoluteTimeShifterAlgorithm(const std::string& str) :
  CalibrationAlgorithm("SVDClusterAbsoluteTimeShifterCollector")
  , m_id(str)
{
  setDescription("SVDClusterAbsoluteTimeShifter calibration algorithm");
}

CalibrationAlgorithm::EResult SVDClusterAbsoluteTimeShifterAlgorithm::calibrate()
{

  gROOT->SetBatch(true);

  gStyle->SetFillColor(0);
  gStyle->SetFillStyle(0);
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);

  FileStat_t info;
  int cal_rev = 1;
  while (gSystem->GetPathInfo(Form("algorithm_svdClusterAbsoluteTimeShifter_%s_output_rev_%d.root", m_timeAlgorithms[0].Data(),
                                   cal_rev),
                              info) == 0)
    cal_rev++;

  auto payload = new Belle2::SVDAbsoluteClusterTimeShift(Form("SVDClusterAbsoluteTimeShifter_%s_rev_%d", m_id.data(), cal_rev));

  // single gauss fit function
  TF1* fn_singleGaus = new TF1("fn_singleGaus", AbssingleGaus, -50., 50., 4);
  fn_singleGaus->SetLineColor(kGreen + 2);
  fn_singleGaus->SetParName(0, "N");
  fn_singleGaus->SetParName(1, "#mu");
  fn_singleGaus->SetParName(2, "#sigma");
  fn_singleGaus->SetParName(3, "C");

  // double gauss fit function
  TF1* fn_doubleGaus = new TF1("fn_doubleGaus", AbsdoubleGaus, -50., 50., 7);
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

    auto __hClsOnTrack__ = getObjectPtr<TH2F>(("hClsTimeOnTracks_" + alg).Data());


    // map : shift values
    std::map< TString, Double_t > shiftValues;



    std::unique_ptr<TFile> f(new TFile(Form("algorithm_svdClusterAbsoluteTimeShifter_%s_output_rev_%d.root", alg.Data(), cal_rev),
                                       "RECREATE"));

    TString outPDF = Form("algorithm_svdClusterAbsoluteTimeShifter_%s_output_rev_%d.pdf", alg.Data(), cal_rev);
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


    for (int layer = 1; layer <= 6; layer++) {
      for (int side = 0; side < 2; side++) { // 0 for U, 1 for V
        int LayerSensorID = 2 * layer - side; // 1-based index

        TString binLabel = TString::Format("L%iS%c", layer, (side == 0 ? 'U' : 'V'));
        TH1F* hist = (TH1F*)__hClsOnTrack__->ProjectionX(Form("hClsTimeOnTracks_L%dS%c", layer, (side == 0 ? 'U' : 'V')), LayerSensorID,
                                                         LayerSensorID, "");

        hist->SetTitle(Form("Cluster Time in L%d%d", layer, (side == 0 ? 'U' : 'V')));
        hist->SetDirectory(0);

        B2INFO("Histogram: " << hist->GetName() <<
               " Entries (n. clusters): " << hist->GetEntries());
        if (hist->GetEntries() < m_minEntries) {
          B2INFO("Histogram: " << hist->GetName() <<
                 " Entries (n. clusters): " << hist->GetEntries() <<
                 " Entries required: " << m_minEntries);
          B2WARNING("Not enough data, adding one run to the collector");
          delete hist;
          c1.Print(outPDF + "]");
          f->Close();
          gSystem->Unlink(Form("algorithm_svdClusterAbsoluteTimeShifter_%s_output_rev_%d.root", m_timeAlgorithms[0].Data(), cal_rev));
          return c_NotEnoughData;
        }
        if (hist->GetEntries() == 0) {
          B2INFO("Histogram: " << hist->GetName() <<
                 " Entries (n. clusters): " << hist->GetEntries() <<
                 " No entries, setting shift to 0.");
          shiftValues[binLabel] = 0.;
          delete hist;
          continue;
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
        ptstats->AddText("Single Gauss");
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
        ptstats->AddText("Double Gauss");
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
          ptstats->AddText(TString::Format("#splitline{Shift Value from Double Gauss}{%.3f}", fillShiftVal));
        } else if (isSingleGausFitValid) {
          fillShiftVal = fn_singleGaus->GetParameter(1);
          B2WARNING("Fit failed for " << hist->GetName() <<
                    "; using mean from single gauss fit. ");
          ptstats->AddText(TString::Format("#splitline{Shift Value from Single Gauss}{%.3f}", fillShiftVal));
        }

        if (std::fabs(fillShiftVal) > m_maximumAllowedShift) {
          B2WARNING("Shift value is more than allowed or fit failed in " <<
                    hist->GetName() << " : " <<
                    shiftValues[binLabel] <<
                    "; using mean of the histogram.");
          fillShiftVal = hist->GetMean();
          ptstats->AddText(TString::Format("#splitline{Shift Value from Histogram Mean}{%.3f}", fillShiftVal));
        }

        shiftValues[binLabel] = 1. * int(1000. * fillShiftVal) / 1000.;

        c1.Print(outPDF, TString("Title:") + hist->GetName());
        f->cd();
        hist->Write();

        delete hist;
      } // loop over cluster size

      f->cd();

    } // loop over sensors

    for (auto item : shiftValues)
      payload->setClusterTimeShift(alg, item.first, item.second);
    B2INFO("Shift values for algorithm " + alg + " : ");
    for (const auto& item : shiftValues) {
      B2INFO("  " << item.first << " : " << item.second);

    }

    onePad.cd();

    f->cd();

    c1.Print(outPDF + "]");
    f->Close();
  } // loop over algorithms

  saveCalibration(payload, "SVDClusterAbsoluteTimeShifter");

  return c_OK;
}
