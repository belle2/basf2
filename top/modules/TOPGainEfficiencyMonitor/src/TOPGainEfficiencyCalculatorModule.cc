/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Maeda Yosuke                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// own include
#include <top/modules/TOPGainEfficiencyMonitor/TOPGainEfficiencyCalculatorModule.h>

// standard libraries
#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>
#include <map>

// ROOT
#include <TROOT.h>
#include <TObject.h>
#include <TFile.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TAxis.h>
#include <TLine.h>
#include <TArrow.h>
#include <TLatex.h>
#include <TMath.h>

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------

  REG_MODULE(TOPGainEfficiencyCalculator)


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPGainEfficiencyCalculatorModule::TOPGainEfficiencyCalculatorModule() : HistoModule()
  {
    // Set description()
    setDescription("Calculate pixel gain and efficiency for a given PMT from 2D histogram of hit timing and pulse height, "
                   "created by TOPLaserHitSelectorModule.");

    // Add parameters
    addParam("inputFile", m_inputFile, "input file name containing 2D histogram", std::string(""));
    addParam("outputPDFFile", m_outputPDFFile, "output PDF file to store plots", std::string(""));
    addParam("targetSlotId", m_targetSlotId, "TOP module ID in slot number (1-16)", (short)0);
    addParam("targetPmtId", m_targetPmtId, "PMT number (1-32)", (short)0);
    addParam("fitHalfWidth", m_fitHalfWidth, "half fit width for direct laser hit peak in [ns] unit", (float)1.0);
    addParam("threshold", m_threshold,
             "pulse height (or integrated charge) threshold in fitting its distribution and calculating efficiency", (float)100.);
    addParam("fracFit", m_fracFit, "fraction of events to be used in fitting. "
             "An upper limit of a fit range is given to cover this fraction of events "
             "pulse height distribution", (float)0.99);
    addParam("initialP0", m_initialP0, "initial value of the fit parameter p0 divided by histogram entries", (float)1e-6);
    addParam("initialP1", m_initialP1, "initial value of the fit parameter p1", (float)1.0);
    addParam("initialP2", m_initialP2, "initial value of the fit parameter p2", (float)1.0);
    addParam("initialX0", m_initialX0, "initial value of the fit parameter x0 divided by histogram bin width", (float)100.);
    addParam("pedestalSigma", m_pedestalSigma, "sigma of pedestal width", (float)10.);
  }

  TOPGainEfficiencyCalculatorModule::~TOPGainEfficiencyCalculatorModule() {}

  void TOPGainEfficiencyCalculatorModule::initialize()
  {
    REG_HISTOGRAM;
  }

  void TOPGainEfficiencyCalculatorModule::defineHisto()
  {
    m_tree = new TTree("tree", "TTree for gain/efficiency monitor summary");

    m_tree->Branch("slotId", &m_targetSlotId, "slotId/S");
    m_tree->Branch("pmtId", &m_targetPmtId, "pmtId/S");
    m_tree->Branch("pixelId", &m_pixelId, "pixelId/S");
    m_tree->Branch("pmtChId", &m_pmtChId, "pmtChId/S");
    m_tree->Branch("hitTiming", &m_hitTiming, "hitTiming/F");
    m_tree->Branch("hitTimingSigma", &m_hitTimingSigma, "hitTimingSigma/F");
    m_tree->Branch("nEntries", &m_nEntries, "nEntries/I");
    m_tree->Branch("nCalPulse", &m_nCalPulse, "nCalPulse/I");
    m_tree->Branch("nOverflowEvents", &m_nOverflowEvents, "nOverflowEvents/I");
    m_tree->Branch("meanPulseHeight", &m_meanPulseHeight, "meanPulseHeight/F");
    m_tree->Branch("threshold", &m_threshold, "threshold/F");
    m_tree->Branch("fitMax", &m_fitMax, "fitMax/F");
    m_tree->Branch("gain", &m_gain, "gain/F");
    m_tree->Branch("efficiency", &m_efficiency, "efficiency/F");
    m_tree->Branch("p0", &m_p0, "p0/F");
    m_tree->Branch("p1", &m_p1, "p1/F");
    m_tree->Branch("p2", &m_p2, "p2/F");
    m_tree->Branch("x0", &m_x0, "x0/F");
    m_tree->Branch("p0Error", &m_p0Error, "p0Error/F");
    m_tree->Branch("p1Error", &m_p1Error, "p1Error/F");
    m_tree->Branch("p2Error", &m_p2Error, "p2Error/F");
    m_tree->Branch("x0Error", &m_x0Error, "x0Error/F");
    m_tree->Branch("chisquare", &m_chisquare, "chisquare/F");
    m_tree->Branch("ndf", &m_ndf, "ndf/I");
    m_tree->Branch("funcFullRangeIntegral", &m_funcFullRangeIntegral, "funcFullRangeIntegral/F");
    m_tree->Branch("funcFitRangeIntegral", &m_funcFitRangeIntegral, "funcFitRangeIntegral/F");
    m_tree->Branch("histoFitRangeIntegral", &m_histoFitRangeIntegral, "histoFitRangeIntegral/F");
    m_tree->Branch("histoMeanAboveThre", &m_histoMeanAboveThre, "histoMeanAboveThre/F");
  }

  void TOPGainEfficiencyCalculatorModule::beginRun()
  {
  }

  void TOPGainEfficiencyCalculatorModule::event()
  {
  }

  void TOPGainEfficiencyCalculatorModule::endRun()
  {
  }


  void TOPGainEfficiencyCalculatorModule::terminate()
  {
    //first, check validity of input parameters
    bool areGoodParameters = true;
    if (m_targetSlotId < 1 || m_targetSlotId > c_NModule) {
      B2ERROR("TOPGainEfficiencyCalculator : invalid slotID : " << m_targetSlotId);
      areGoodParameters = false;
    }
    if (m_targetPmtId < 1 || m_targetPmtId > c_NPMTPerModule) {
      B2ERROR("TOPGainEfficiencyCalculator : invalid PMT ID : " << m_targetPmtId);
      areGoodParameters = false;
    }
    if (m_initialX0 < 1e-6) {
      B2ERROR("TOPGainEfficiencyCalculator : initial x0 value must be non-zero positive value");
      areGoodParameters = false;
    }
    if (m_pedestalSigma < 1e-6) {
      B2ERROR("TOPGainEfficiencyCalculator : pedestal sigma must be non-zero positive value");
      areGoodParameters = false;
    }

    //do not proceed to the main process unless all the input parameters are not valid
    if (areGoodParameters) {
      if (m_outputPDFFile.size() == 0) {
        if (m_inputFile.rfind(".") == std::string::npos)
          m_outputPDFFile = m_inputFile + "_gain.pdf";
        else
          m_outputPDFFile = m_inputFile.substr(0, m_inputFile.rfind(".")) + "_gain.pdf";
      }

      LoadHistograms();
      FitHistograms();
      DrawResult();
    }
  }


  void TOPGainEfficiencyCalculatorModule::LoadHistograms()
  {

    TFile* f = new TFile(m_inputFile.c_str());
    if (!f->IsOpen()) {
      B2ERROR("TOPGainEfficiencyCalculator : fail to open input file \"" << m_inputFile << "\"");
      return;
    }

    for (int iHisto = 0 ; iHisto < c_NChannelPerPMT ; iHisto++) {
      std::ostringstream pixelstr;
      pixelstr << "s" << std::setw(2) << std::setfill('0') << m_targetSlotId << "_PMT"
               << std::setw(2) << std::setfill('0') << m_targetPmtId
               << "_" << std::setw(2) << std::setfill('0') << (iHisto + 1);
      std::ostringstream hname;
      hname << "hTimeHeight_" << pixelstr.str();

      //first get 2D histogram from a given input (=an output file of TOPLaserHitSelector)
      m_timeHeightHistogram[iHisto] = (TH2F*)f->Get(hname.str().c_str());
      TH2F* h2D = m_timeHeightHistogram[iHisto];
      if (!h2D) continue;

      //create a projection histogram along the x-axis and fit the distribution (hit timing) to get direct laser hit timing
      std::ostringstream hnameProj[2];
      hnameProj[0] << "hTime_" << pixelstr.str();
      hnameProj[1] << "hHeight_" << pixelstr.str();
      TH1D* hTime = (TH1D*)h2D->ProjectionX(hnameProj[0].str().c_str());
      m_timeHistogram[iHisto] = hTime;
      double peakTime = FindPeakForSmallerXThan(hTime, 0);
      //double peakTime = hTime->GetXaxis()->GetBinCenter(hTime->GetMaximumBin());
      double fitMin = peakTime - m_fitHalfWidth;
      double fitMax = peakTime + m_fitHalfWidth;
      TF1* funcLaser = new TF1(std::string(std::string("func_") + hnameProj[1].str()).c_str(),
                               "gaus(0)", fitMin, fitMax);
      funcLaser->SetParameters(hTime->GetBinContent(hTime->GetXaxis()->FindBin(peakTime)), peakTime, m_fitHalfWidth);
      funcLaser->SetParLimits(1, fitMin, fitMax);
      hTime->Fit(funcLaser, "Q", "", fitMin, fitMax);
      if (funcLaser->GetNDF() < 1) continue;
      m_funcForLaser[iHisto] = funcLaser;

      //if the fitting is successful, create y-projection histogram with timing cut
      m_hitTiming = funcLaser->GetParameter(1);
      int binNumMin = hTime->GetXaxis()->FindBin(m_hitTiming - 2 * m_fitHalfWidth);
      int binNumMax = hTime->GetXaxis()->FindBin(m_hitTiming + 2 * m_fitHalfWidth);
      TH1D* hHeight = (TH1D*)h2D->ProjectionY(hnameProj[1].str().c_str(),
                                              binNumMin, binNumMax);
      m_heightHistogram[iHisto] = hHeight;
    }

    m_nCalPulseHistogram = (TH1F*)f->Get("hNCalPulse");
    if (!m_nCalPulseHistogram)
      B2WARNING("TOPGainEfficiencyCalculator : no histogram for the number of events with calibration pulses identified in the given input file");

    return;
  }

  void TOPGainEfficiencyCalculatorModule::FitHistograms()
  {

    for (int iHisto = 0 ; iHisto < c_NChannelPerPMT ; iHisto++) {

      TH1D* hHeight = m_heightHistogram[iHisto];
      if (!hHeight) continue;

      std::cout << " ***** fitting height distribution for " << hHeight->GetName() << " *****" << std::endl;
      int nBins = hHeight->GetXaxis()->GetNbins();
      double binWidth = hHeight->GetXaxis()->GetBinUpEdge(1) - hHeight->GetXaxis()->GetBinLowEdge(1);
      double histoMax = hHeight->GetXaxis()->GetBinUpEdge(nBins);
      m_fitMax = m_threshold;
      double wholeIntegral = hHeight->Integral(0, hHeight->GetXaxis()->GetNbins() + 1);
      while (hHeight->Integral(0, hHeight->GetXaxis()->FindBin(m_fitMax - 0.01 * binWidth)) / wholeIntegral < m_fracFit)
        m_fitMax += binWidth;
      if (m_fitMax < m_threshold + c_NParameterGainFit * binWidth) {
        B2WARNING("TOPGainEfficiencyCalculator : no enough entries for fitting...");
        continue;
      }

      std::ostringstream fname;
      fname << "func_" << (iHisto + 1);
      TObject* object = gROOT->FindObject(fname.str().c_str());
      if (object) delete object;
      TF1* func = new TF1(fname.str().c_str(), TOPGainFunc, m_threshold, m_fitMax, c_NParameterGainFit);
      func->SetParameter(0, m_initialP0 * hHeight->Integral()*binWidth);
      func->SetParameter(1, m_initialP1);
      func->SetParameter(2, m_initialP2);
      func->SetParameter(3, m_initialX0 * binWidth);
      func->FixParameter(4, 0);
      func->FixParameter(5, m_pedestalSigma);
      func->SetParName(0, "#it{p}_{0}");
      func->SetParName(1, "#it{p}_{1}");
      func->SetParName(2, "#it{p}_{2}");
      func->SetParName(3, "#it{x}_{0}");
      func->SetParName(4, "pedestal");
      func->SetParName(5, "pedestal #sigma");
      func->SetParLimits(3, 1e-6, 1e8);
      func->SetLineColor(2);
      func->SetLineWidth(1);
      hHeight->Fit(func, "R", "", m_threshold, m_fitMax);
      if (func->GetNDF() < 2) continue;

      double funcFullMax = histoMax * 2;
      TF1* funcFull = new TF1((fname.str() + "_full").c_str(), TOPGainFunc, (-1)*func->GetParameter(5), funcFullMax, c_NParameterGainFit);
      for (int iPar = 0 ; iPar < c_NParameterGainFit ; iPar++)
        funcFull->SetParameter(iPar, func->GetParameter(iPar));
      funcFull->SetLineColor(3);
      funcFull->SetLineWidth(2);

      double totalWeight = 0;
      double weightedIntegral = 0;
      double x = (-1) * func->GetParameter(5);
      while (x < funcFullMax) {
        double funcVal = funcFull->Eval(x);
        totalWeight += funcVal;
        weightedIntegral += funcVal * x;
        x += binWidth / 5.;
      }

      //fill results to the output TTree
      m_pixelId = ((m_targetPmtId - 1) % c_NPMTPerRow) * c_NChannelPerPMTRow
                  + ((m_targetPmtId - 1) / c_NPMTPerRow) * c_NPixelPerModule / 2
                  + (iHisto / c_NChannelPerPMTRow) * c_NPixelPerRow + (iHisto % c_NChannelPerPMTRow) + 1;
      m_pmtChId = (iHisto + 1);
      int globalAsicId = ((m_targetSlotId - 1) * c_NPixelPerModule + (m_pixelId - 1)) / c_NChannelPerAsic;

      m_nEntries = hHeight->GetEntries();
      m_nCalPulse = (m_nCalPulseHistogram ? m_nCalPulseHistogram->GetBinContent(globalAsicId + 1) : -1);
      m_nOverflowEvents = TMath::FloorNint(hHeight->GetBinContent(nBins + 1));
      m_meanPulseHeight = hHeight->GetMean();
      m_gain = weightedIntegral / totalWeight;
      m_efficiency = funcFull->Integral(m_threshold, funcFullMax) / funcFull->Integral((-1) * func->GetParameter(5), funcFullMax);
      m_p0 = func->GetParameter(0);
      m_p1 = func->GetParameter(1);
      m_p2 = func->GetParameter(2);
      m_x0 = func->GetParameter(3);
      m_p0Error = func->GetParError(0);
      m_p1Error = func->GetParError(1);
      m_p2Error = func->GetParError(2);
      m_x0Error = func->GetParError(3);
      m_chisquare = func->GetChisquare();
      m_ndf = func->GetNDF();
      m_funcFullRangeIntegral = funcFull->Integral((-1) * func->GetParameter(5), funcFullMax) / binWidth;
      m_funcFitRangeIntegral = funcFull->Integral(m_threshold, m_fitMax) / binWidth;
      int threBin = hHeight->GetXaxis()->FindBin(m_threshold + 0.01 * binWidth);
      int fitMaxBin = hHeight->GetXaxis()->FindBin(m_fitMax - 0.01 * binWidth);
      m_histoFitRangeIntegral = hHeight->Integral(threBin, fitMaxBin);

      m_histoMeanAboveThre = 0;
      for (int iBin = threBin ; iBin < nBins + 1 ; iBin++) {
        m_histoMeanAboveThre += (hHeight->GetBinContent(iBin) * hHeight->GetXaxis()->GetBinCenter(iBin));
      }
      m_histoMeanAboveThre /= hHeight->Integral(threBin, nBins);

      m_hitTiming = 0;
      m_hitTimingSigma = -1;
      TF1* funcLaser = m_funcForLaser[iHisto];
      if (m_timeHistogram[iHisto] && funcLaser) {
        m_hitTiming = funcLaser->GetParameter(1);
        m_hitTimingSigma = funcLaser->GetParameter(2);
      }

      m_funcForFitRange[iHisto] = func;
      m_funcForFullRange[iHisto] = funcFull;


      m_tree->Fill();
      std::cout << std::endl;
    }

    return;
  }



  void TOPGainEfficiencyCalculatorModule::DrawResult()
  {
    gStyle->SetFrameFillStyle(0);
    gStyle->SetFillStyle(0);
    gStyle->SetStatStyle(0);
    gStyle->SetOptStat(112210);
    gStyle->SetOptFit(1110);
    TCanvas* canvas = new TCanvas();
    canvas->SetFillStyle(0);
    canvas->Print((m_outputPDFFile + "[").c_str());

    TLine* line = new TLine();
    line->SetLineWidth(1);
    line->SetLineStyle(1);
    line->SetLineColor(4);
    TArrow* arrow = new TArrow();
    arrow->SetLineWidth(1);
    arrow->SetLineStyle(1);
    arrow->SetLineColor(3);
    TLatex* latex = new TLatex();
    latex->SetNDC();
    latex->SetTextFont(22);
    latex->SetTextSize(0.05);
    latex->SetTextAlign(32);
    TObject* object;

    for (int iHisto = 0 ; iHisto < c_NChannelPerPMT ; iHisto++) {

      if ((iHisto % c_NChannelPerPage) == 0) {
        canvas->Clear();
        canvas->Divide(c_NPlotsPerChannel, c_NChannelPerPage);
      }

      //2D (time vs pulse height) histogram
      canvas->cd(3 * (iHisto % c_NChannelPerPage) + 1);
      gPad->SetFrameFillStyle(0);
      gPad->SetFillStyle(0);
      TH2F* h2D = m_timeHeightHistogram[iHisto];
      if (h2D) {
        h2D->Draw("colz");
        h2D->GetXaxis()->SetTitle("hit timing [ns]");
        h2D->GetYaxis()->SetTitle("pulse height [ADC count]");
      }

      //timing histogram
      canvas->cd(c_NPlotsPerChannel * (iHisto % c_NChannelPerPage) + 2);
      gPad->SetFrameFillStyle(0);
      gPad->SetFillStyle(0);
      TH1D* hTime = m_timeHistogram[iHisto];
      if (hTime) {
        gPad->SetLogy();
        hTime->Draw();
        hTime->SetLineColor(1);
        hTime->GetXaxis()->SetTitle("hit timing [ns]");
        float binWidth = hTime->GetXaxis()->GetBinUpEdge(1) - hTime->GetXaxis()->GetBinLowEdge(1);
        std::ostringstream ytitle;
        ytitle << "Entries [/(" << binWidth << " ns)]";
        hTime->GetYaxis()->SetTitle(ytitle.str().c_str());

        TF1* funcLaser = m_funcForLaser[iHisto];
        if (funcLaser) {
          double height = funcLaser->GetParameter(0);
          double peakTime = funcLaser->GetParameter(1);
          float xMin = hTime->GetXaxis()->GetBinLowEdge(hTime->GetXaxis()->FindBin(peakTime - 2 * m_fitHalfWidth));
          float xMax = hTime->GetXaxis()->GetBinUpEdge(hTime->GetXaxis()->FindBin(peakTime + 2 * m_fitHalfWidth));
          line->DrawLine(xMin, 0.5, xMin, height * 2.);
          line->DrawLine(xMax, 0.5, xMax, height * 2.);
          arrow->DrawArrow(xMin, height * 1.5, xMax, height * 1.5, 0.01, "<>");
        }
      }

      //height histogram with fit result (after timing cut)
      canvas->cd(c_NPlotsPerChannel * (iHisto % c_NChannelPerPage) + 3);
      gPad->SetFrameFillStyle(0);
      gPad->SetFillStyle(0);
      TH1D* hHeight = m_heightHistogram[iHisto];
      if (hHeight) {
        gPad->SetLogy();
        hHeight->Draw();
        hHeight->SetLineColor(1);
        hHeight->GetXaxis()->SetTitle("hit timing [ADC counts]");
        float binWidth = hHeight->GetXaxis()->GetBinUpEdge(1) - hHeight->GetXaxis()->GetBinLowEdge(1);
        std::ostringstream ytitle;
        ytitle << "Entries [/(" << binWidth << " ADC counts)]";
        hHeight->GetYaxis()->SetTitle(ytitle.str().c_str());

        if (m_funcForFitRange[iHisto] && m_funcForFullRange[iHisto]) {
          m_funcForFullRange[iHisto]->Draw("same");
          m_funcForFitRange[iHisto]->Draw("same");
          double height = hHeight->GetBinContent(hHeight->GetMaximumBin());
          line->DrawLine(m_threshold, 0.5, m_threshold, height * 2.);

          if ((object = gROOT->FindObject("dummy"))) delete object;
          std::ostringstream cut;
          cut << "pmtChId==" << (iHisto + 1);
          long nEntries = m_tree->Project("dummy", "gain:efficiency", cut.str().c_str());
          if (nEntries == 1) {
            std::ostringstream summarystr[2];
            summarystr[0] << "gain = " << std::setiosflags(std::ios::fixed) << std::setprecision(1)
                          << m_tree->GetV1()[0];
            summarystr[1] << "efficiency = " << std::setiosflags(std::ios::fixed) << std::setprecision(1)
                          << (m_tree->GetV2()[0] * 100) << " %";
            latex->DrawLatex(0.875, 0.34, summarystr[0].str().c_str());
            latex->DrawLatex(0.875, 0.29, summarystr[1].str().c_str());
          } else if (nEntries > 1) {
            B2WARNING("TOPGainEfficiencyCalculator : mutliple entries with the same channel ID ("
                      << m_pmtChId << ") in the output TTree");
          }
        }
      }

      if (((iHisto + 1) % c_NChannelPerPage) == 0)
        canvas->Print(m_outputPDFFile.c_str());
    }
    for (int iHisto = (c_NChannelPerPMT - 1) % c_NChannelPerPage + 1 ; iHisto < c_NChannelPerPage ; iHisto++) {
      for (int iPad = 0 ; iPad < c_NPlotsPerChannel ; iPad++) {
        canvas->cd(c_NPlotsPerChannel * (iHisto % c_NChannelPerPage) + iPad + 1);
        gPad->SetFrameFillStyle(0);
        gPad->SetFillStyle(0);
      }
      if (((iHisto + 1) % c_NChannelPerPage) == 0)
        canvas->Print(m_outputPDFFile.c_str());
    }

    canvas->Print((m_outputPDFFile + "]").c_str());

    delete latex;
    delete arrow;
    delete line;
    delete canvas;
    if ((object = gROOT->FindObject("dummy"))) delete object;

    return;
  }

  double TOPGainEfficiencyCalculatorModule::TOPGainFunc(double* var, double* par)
  {

    double pedestal = par[4];
    double pedestalWidth = par[5];
    double x = (var[0] - pedestal);

    double output = 0;
    double step = pedestalWidth / 100.;
    double t = TMath::Max(step, x - pedestalWidth * 10);
    //double t = TMath::Max( g_xStep, x ); //for test
    while (t < x + pedestalWidth * 10) {
      output += TMath::Gaus(x, t, pedestalWidth) * TMath::Power(t / par[3], par[1])
                * TMath::Exp((-1) * TMath::Power(t / par[3], par[2]));
      t += step;
    }
    //  TF1* func = new TF1( "func", "[0]*pow(x-[4],[1])*exp(-pow(x-[4],[2])/[3])",

    return par[0] * output * step;
  }

  double TOPGainEfficiencyCalculatorModule::FindPeakForSmallerXThan(TH1* histo, double xmax)
  {
    double histo_xmax = histo->GetXaxis()->GetBinUpEdge(histo->GetXaxis()->GetNbins() - 1);
    if (xmax > histo_xmax) xmax = histo_xmax;

    int iBin = 1;
    double peakPos = histo->GetXaxis()->GetBinCenter(iBin);
    double peakHeight = histo->GetBinContent(iBin);
    while (true) {
      iBin++;
      double x = histo->GetXaxis()->GetBinCenter(iBin);
      if (x > xmax) break;

      double binEntry = histo->GetBinContent(iBin);
      if (binEntry > peakHeight) {
        peakPos = x;
        peakHeight = binEntry;
      }
    }

    return peakPos;
  }

} // end Belle2 namespace
