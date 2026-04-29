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
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TPaveStats.h>
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
  auto fn_singleGaus = std::make_unique<TF1>("fn_singleGaus", AbssingleGaus, -50., 50., 4);
  fn_singleGaus->SetLineColor(kGreen + 2);
  fn_singleGaus->SetParName(0, "N");
  fn_singleGaus->SetParName(1, "#mu");
  fn_singleGaus->SetParName(2, "#sigma");
  fn_singleGaus->SetParName(3, "C");

  // double gauss fit function
  auto fn_doubleGaus = std::make_unique<TF1>("fn_doubleGaus", AbsdoubleGaus, -50., 50., 7);
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

  int nBins = (m_outerLayer - m_innerLayer + 1) * 2;

  for (auto alg : m_timeAlgorithms) {

    B2INFO("Calculating shift for algorithm " << alg);

    auto h_ClustersOnTrack = getObjectPtr<TH2F>(("hClsTimeOnTracks_" + alg).Data());  // __hClsOnTrack__

    // auto __CDCEventT0__ = getObjectPtr<TH1F>("hCDCEventT0_");
    // map : shift values
    std::map< TString, Double_t > shiftValues;

    std::unique_ptr<TFile> f(new TFile(Form("algorithm_svdClusterAbsoluteTimeShifter_%s_output_rev_%d.root", alg.Data(), cal_rev),
                                       "RECREATE"));

    B2INFO("ROOT file created at: " << gSystem->WorkingDirectory() << "/" << f->GetName());

    TH1D* hShiftMean  = new TH1D("hShiftMean",
                                 Form("Fitted shift mean (%s);Bin;Mean (ns)", alg.Data()), nBins, 0.5, nBins + 0.5);
    TH1D* hShiftSigma = new TH1D("hShiftSigma",
                                 Form("Fitted shift sigma (%s);Bin;Sigma (ns)", alg.Data()), nBins, 0.5, nBins + 0.5);
    for (int l = m_innerLayer; l <= m_outerLayer; l++)
      for (int s = 0; s < 2; s++) {
        int b = 2 * (l - m_innerLayer) + s + 1;
        hShiftMean ->GetXaxis()->SetBinLabel(b, TString::Format("L%dS%c", l, s == 0 ? 'U' : 'V'));
        hShiftSigma->GetXaxis()->SetBinLabel(b, TString::Format("L%dS%c", l, s == 0 ? 'U' : 'V'));
      }

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


    for (int layer = m_innerLayer; layer <= m_outerLayer; layer++) {
      for (int side = 0; side < 2; side++) { // 0 for U, 1 for V
        int LayerSensorID = 2 * layer - side; // 1-based index



        TString binLabel = TString::Format("L%iS%c", layer, (side == 0 ? 'U' : 'V'));
        TH1F* hist = (TH1F*)h_ClustersOnTrack->ProjectionX(Form("hClsTimeOnTracks_L%dS%c", layer, (side == 0 ? 'U' : 'V')), LayerSensorID,
                                                           LayerSensorID, "");
        hist->SetTitle(Form("Cluster Time in L%dS%c", layer, (side == 0 ? 'U' : 'V')));
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

        // // Overlay CDCEventT0 histogram
        // if (__CDCEventT0__) {
        //   TH1F* cdcOverlay = (TH1F*)__CDCEventT0__->Clone(Form("cdc_%s", binLabel.Data()));
        //   cdcOverlay->SetLineColor(kMagenta);
        //   cdcOverlay->SetLineWidth(2);
        //   cdcOverlay->SetLineStyle(2);
        //   cdcOverlay->SetDirectory(0);

        //   // Scale to match histogram height
        //   double scale = hist->GetMaximum() / cdcOverlay->GetMaximum() * 0.8;
        //   // cdcOverlay->Scale(scale);
        //   // cdcOverlay->Draw("SAME");
        // }

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

        auto ptstats = std::make_unique<TPaveStats>(0.55, 0.73, 0.85, 0.88, "brNDC");
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
        ptstats = std::make_unique<TPaveStats>(0.55, 0.49, 0.85, 0.73, "brNDC");
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
        ptstats = std::make_unique<TPaveStats>(0.55, 0.43, 0.85, 0.49, "brNDC");
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

        double fillSigmaVal;
        if (isDoubleGausFitValid)
          fillSigmaVal = (fn_doubleGaus->GetParameter(1) > 0.5 ?
                          fn_doubleGaus->GetParameter(3) : fn_doubleGaus->GetParameter(5));
        else if (isSingleGausFitValid)
          fillSigmaVal = fn_singleGaus->GetParameter(2);
        else
          fillSigmaVal = hist->GetStdDev();

        int binIdx = 2 * (layer - 3) + side + 1;
        hShiftMean ->SetBinContent(binIdx, fillShiftVal);
        hShiftSigma->SetBinContent(binIdx, fillSigmaVal);

        // Ensure the plot is visually restricted to [-50, 50] after all fitting and drawing
        hist->GetXaxis()->SetRangeUser(-50, 50);
        gPad->Range(-50, 0, 50, hist->GetMaximum() * 1.2);
        gPad->Update();

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
    f->cd();
    hShiftMean->Write();
    hShiftSigma->Write();
    delete hShiftMean;
    delete hShiftSigma;
    f->Close();
  } // loop over algorithms

  saveCalibration(payload, "SVDAbsoluteClusterTimeShift");

  return c_OK;
}

bool SVDClusterAbsoluteTimeShifterAlgorithm::isBoundaryRequired(const Calibration::ExpRun& currentRun)
{

  TString alg = "CoG3";
  if (std::find(m_timeAlgorithms.begin(), m_timeAlgorithms.end(), "CoG3") == m_timeAlgorithms.end()) {
    alg = m_timeAlgorithms[0];
  }
  float meanTimeL3V = 0;
  auto h_ClustersOnTrack = getObjectPtr<TH2F>(("hClsTimeOnTracks_" + alg).Data());
  int layer = 3;
  int side = 0;
  int LayerSensorID = 2 * layer - side;
  auto timeL3V = (TH1F*)h_ClustersOnTrack->ProjectionX(Form("hClsTimeOnTracks_L%dS%c", layer, (side == 0 ? 'U' : 'V')), LayerSensorID,
                                                       LayerSensorID, "");
  if (!timeL3V) {
    if (m_previousTimeMeanL3V)
      meanTimeL3V = m_previousTimeMeanL3V.value();
  } else {
    if (timeL3V->GetEntries() > m_minEntries)
      meanTimeL3V = timeL3V->GetMean();
    else {
      if (m_previousTimeMeanL3V)
        meanTimeL3V = m_previousTimeMeanL3V.value();
    }
  }
  if (!m_previousTimeMeanL3V) {
    B2INFO("Setting start payload boundary to be the first run ("
           << currentRun.first << "," << currentRun.second << ")");
    m_previousTimeMeanL3V.emplace(meanTimeL3V);

    return true;
  } else if (abs(meanTimeL3V - m_previousTimeMeanL3V.value()) > m_allowedTimeShift) {
    B2INFO("Histogram mean has shifted from " << m_previousTimeMeanL3V.value()
           << " to " << meanTimeL3V << ". We are requesting a new payload boundary for ("
           << currentRun.first << "," << currentRun.second << ")");
    m_previousTimeMeanL3V.emplace(meanTimeL3V);
    return true;
  } else {
    return false;
  }
}

