/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Maeda Yosuke, Okuto Rikuya                               *
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
    setDescription("Calculate pixel gain and efficiency for a given PMT from 2D histogram of hit timing and pulse charge, "
                   "created by TOPLaserHitSelectorModule.");

    // Add parameters
    addParam("inputFile", m_inputFile, "input file name containing 2D histogram", std::string(""));
    addParam("outputPDFFile", m_outputPDFFile, "output PDF file to store plots", std::string(""));
    addParam("targetSlotId", m_targetSlotId, "TOP module ID in slot number (1-16)", (short)0);
    addParam("targetPmtId", m_targetPmtId, "PMT number (1-32)", (short)0);
    addParam("targetPmtChId", m_targetPmtChId, "PMT channel number (1-16)", (short) - 1);
    addParam("hvDiff", m_hvDiff, "HV difference from nominal HV value", (short)0);
    addParam("fitHalfWidth", m_fitHalfWidth, "half fit width for direct laser hit peak in [ns] unit", (float)1.0);
    addParam("threshold", m_threshold,
             "pulse height (or integrated charge) threshold in fitting its distribution and calculating efficiency", (float)100.);
    addParam("p0HeightIntegral", m_p0HeightIntegral,
             "Parameter from p0 + x*p1 function fitting height-integral distribtion.", (float) - 50.0);
    addParam("p1HeightIntegral", m_p1HeightIntegral,
             "Parameter from p0 + x*p1 function fitting height-integral distribtion.", (float)6.0);
    addParam("fracFit", m_fracFit, "fraction of events to be used in fitting. "
             "An upper limit of a fit range is given to cover this fraction of events. "
             "Set negative value to calculate the fraction to exclude only 10 events in tail.", (float)(-1)); //,(float)0.99);
    addParam("initialP0", m_initialP0, "initial value of the fit parameter p0 divided by histogram entries."
             "Set negative value to calculate from histogram inforamtion automatically.", (float)(0.0001)); //,(float)1e-6);
    addParam("initialP1", m_initialP1, "initial value of the fit parameter p1."
             "Set negative value to calculate from histogram inforamtion automatically.", (float)(1.0)); //,(float)1.0);
    addParam("initialP2", m_initialP2, "initial value of the fit parameter p2."
             "Set negative value to calculate from histogram inforamtion automatically.", (float)(1.0)); //,(float)1.0);
    addParam("initialX0", m_initialX0, "initial value of the fit parameter x0 divided by histogram bin width."
             "Set negative value to calculate from histogram inforamtion automatically.", (float)(100)); //, (float)100.);
    addParam("pedestalSigma", m_pedestalSigma, "sigma of pedestal width", (float)10.);
    addParam("fitoption", m_fitoption, "fit option likelihood: default chisquare: R", std::string("L"));

  }

  TOPGainEfficiencyCalculatorModule::~TOPGainEfficiencyCalculatorModule() {}

  void TOPGainEfficiencyCalculatorModule::initialize()
  {
    REG_HISTOGRAM;
  }

  void TOPGainEfficiencyCalculatorModule::defineHisto()
  {
    m_tree = new TTree("tree", "TTree for gain/efficiency monitor summary");
    m_branch[0].push_back(m_tree->Branch("slotId", &m_targetSlotId, "slotId/S"));
    m_branch[0].push_back(m_tree->Branch("pmtId", &m_targetPmtId, "pmtId/S"));
    m_branch[0].push_back(m_tree->Branch("pixelId", &m_pixelId, "pixelId/S"));
    m_branch[0].push_back(m_tree->Branch("pmtChId", &m_pmtChId, "pmtChId/S"));
    m_branch[0].push_back(m_tree->Branch("hvDiff", &m_hvDiff, "hvDiff/S"));
    m_branch[0].push_back(m_tree->Branch("threshold", &m_threshold, "threshold/F"));
    m_branch[0].push_back(m_tree->Branch("thresholdForIntegral", &m_thresholdForIntegral, "thresholdForIntegral/F"));

    m_branch[1].push_back(m_tree->Branch("nCalPulse", &m_nCalPulse, "nCalPulse/I"));
    m_branch[1].push_back(m_tree->Branch("hitTimingForGain", &m_hitTiming, "hitTimingForGain/F"));
    m_branch[1].push_back(m_tree->Branch("hitTimingSigmaForGain", &m_hitTimingSigma, "hitTimingSigmaForGain/F"));
    m_branch[1].push_back(m_tree->Branch("nEntriesForGain", &m_nEntries, "nEntriesForGain/I"));
    m_branch[1].push_back(m_tree->Branch("nOverflowEventsForGain", &m_nOverflowEvents, "nOverflowEventsForGain/I"));
    m_branch[1].push_back(m_tree->Branch("meanPulseHeightForGain", &m_meanPulseHeight, "meanPulseHeightForGain/F"));
    m_branch[1].push_back(m_tree->Branch("meanPulseHeightErrorForGain", &m_meanPulseHeightError, "meanPulseHeightErrorForGain/F"));
    m_branch[1].push_back(m_tree->Branch("fitMaxForGain", &m_fitMax, "fitMaxForGain/F"));
    m_branch[1].push_back(m_tree->Branch("gain", &m_gain, "gain/F"));
    m_branch[1].push_back(m_tree->Branch("efficiency", &m_efficiency, "efficiency/F"));
    m_branch[1].push_back(m_tree->Branch("p0ForGain", &m_p0, "p0ForGain/F"));
    m_branch[1].push_back(m_tree->Branch("p1ForGain", &m_p1, "p1ForGain/F"));
    m_branch[1].push_back(m_tree->Branch("p2ForGain", &m_p2, "p2ForGain/F"));
    m_branch[1].push_back(m_tree->Branch("x0ForGain", &m_x0, "x0ForGain/F"));
    m_branch[1].push_back(m_tree->Branch("p0ErrorForGain", &m_p0Error, "p0ErrorForGain/F"));
    m_branch[1].push_back(m_tree->Branch("p1ErrorForGain", &m_p1Error, "p1ErrorForGain/F"));
    m_branch[1].push_back(m_tree->Branch("p2ErrorForGain", &m_p2Error, "p2ErrorForGain/F"));
    m_branch[1].push_back(m_tree->Branch("x0ErrorForGain", &m_x0Error, "x0ErrorForGain/F"));
    m_branch[1].push_back(m_tree->Branch("chisquareForGain", &m_chisquare, "chisquareForGain/F"));
    m_branch[1].push_back(m_tree->Branch("ndfForGain", &m_ndf, "ndfForGain/I"));
    m_branch[1].push_back(m_tree->Branch("funcFullRangeIntegralForGain", &m_funcFullRangeIntegral, "funcFullRangeIntegralForGain/F"));
    m_branch[1].push_back(m_tree->Branch("funcFitRangeIntegralForGain", &m_funcFitRangeIntegral, "funcFitRangeIntegralForGain/F"));
    m_branch[1].push_back(m_tree->Branch("histoFitRangeIntegralForGain", &m_histoFitRangeIntegral, "histoFitRangeIntegralForGain/F"));
    m_branch[1].push_back(m_tree->Branch("histoMeanAboveThreForGain", &m_histoMeanAboveThre, "histoMeanAboveThreForGain/F"));

    m_branch[2].push_back(m_tree->Branch("hitTimingForEff", &m_hitTiming, "hitTimingForEff/F"));
    m_branch[2].push_back(m_tree->Branch("hitTimingSigmaForEff", &m_hitTimingSigma, "hitTimingSigmaForEff/F"));
    m_branch[2].push_back(m_tree->Branch("nEntriesForEff", &m_nEntries, "nEntriesForEff/I"));
    m_branch[2].push_back(m_tree->Branch("nOverflowEventsForEff", &m_nOverflowEvents, "nOverflowEventsForEff/I"));
    m_branch[2].push_back(m_tree->Branch("meanPulseHeightForEff", &m_meanPulseHeight, "meanPulseHeightForEff/F"));
    m_branch[2].push_back(m_tree->Branch("meanPulseHeightErrorForEff", &m_meanPulseHeightError, "meanPulseHeightErrorForEff/F"));
    m_branch[2].push_back(m_tree->Branch("histoMeanAboveThreForEff", &m_histoMeanAboveThre, "histoMeanAboveThreForEff/F"));

    m_branch[3].push_back(m_tree->Branch("meanIntegral", &m_meanPulseHeight, "meanIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("meanIntegralError", &m_meanPulseHeightError, "meanIntegralError/F"));
    m_branch[3].push_back(m_tree->Branch("nOverflowEventsUseIntegral", &m_nOverflowEvents, "nOverflowEventsUseIntegral/I"));
    m_branch[3].push_back(m_tree->Branch("fitMaxUseIntegral", &m_fitMax, "fitMaxUseIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("gainUseIntegral", &m_gain, "gainUseIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("efficiencyUseIntegral", &m_efficiency, "efficiencyUseIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("p0UseIntegral", &m_p0, "p0UseIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("p1UseIntegral", &m_p1, "p1UseIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("p2UseIntegral", &m_p2, "p2UseIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("x0UseIntegral", &m_x0, "x0UseIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("p0ErrorUseIntegral", &m_p0Error, "p0ErrorUseIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("p1ErrorUseIntegral", &m_p1Error, "p1ErrorUseIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("p2ErrorUseIntegral", &m_p2Error, "p2ErrorUseIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("x0ErrorUseIntegral", &m_x0Error, "x0ErrorUseIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("chisquareUseIntegral", &m_chisquare, "chisquareUseIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("ndfUseIntegral", &m_ndf, "ndfUseIntegral/I"));
    m_branch[3].push_back(m_tree->Branch("funcFullRangeIntegralUseIntegral", &m_funcFullRangeIntegral,
                                         "funcFullRangeIntegralUseIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("funcFitRangeIntegralUseIntegral", &m_funcFitRangeIntegral,
                                         "funcFitRangeIntegralUseIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("histoFitRangeIntegralUseIntegral", &m_histoFitRangeIntegral,
                                         "histoFitRangeIntegralUseIntegral/F"));
    m_branch[3].push_back(m_tree->Branch("histoMeanAboveThreUseIntegral", &m_histoMeanAboveThre, "histoMeanAboveThreUseIntegral/F"));

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
    if (m_pedestalSigma < 1e-6) {
      B2ERROR("TOPGainEfficiencyCalculator : pedestal sigma must be non-zero positive value");
      areGoodParameters = false;
    }

    //do not proceed to the main process unless all the input parameters are not valid
    if (areGoodParameters) {
      if (m_outputPDFFile.size() == 0) {
        if (m_inputFile.rfind(".") == std::string::npos)
          m_outputPDFFile = m_inputFile;
        else
          m_outputPDFFile = m_inputFile.substr(0, m_inputFile.rfind("."));
      }

      //gain run using height distribution
      LoadHistograms("Height_gain");
      FitHistograms(c_LoadForFitHeight);
      DrawResult("Height_gain", c_LoadForFitHeight);

      //efficiency run
      LoadHistograms("Height_efficiency");
      FitHistograms(c_LoadHitRateHeight);
      DrawResult("Height_efficiency", c_LoadHitRateHeight);

      //gain run using integral distribution
      LoadHistograms("Integral_gain");
      FitHistograms(c_LoadForFitIntegral);
      DrawResult("Integral_gain", c_LoadForFitIntegral);

    }
  }


  void TOPGainEfficiencyCalculatorModule::LoadHistograms(std::string histotype)
  {

    TFile* f = new TFile(m_inputFile.c_str());
    if (!f->IsOpen()) {
      B2ERROR("TOPGainEfficiencyCalculator : fail to open input file \"" << m_inputFile << "\"");
      return;
    }

    for (int iHisto = 0 ; iHisto < c_NChannelPerPMT ; iHisto++) {
      if (m_targetPmtChId != -1 && iHisto + 1 != m_targetPmtChId) continue;
      std::ostringstream pixelstr;
      pixelstr << histotype << "_"
               << "s" << std::setw(2) << std::setfill('0') << m_targetSlotId << "_PMT"
               << std::setw(2) << std::setfill('0') << m_targetPmtId
               << "_" << std::setw(2) << std::setfill('0') << (iHisto + 1);
      std::ostringstream hname;
      hname << "hTime" << pixelstr.str();

      //first get 2D histogram from a given input (=an output file of TOPLaserHitSelector)
      m_timeChargeHistogram[iHisto] = (TH2F*)f->Get(hname.str().c_str());
      TH2F* h2D = m_timeChargeHistogram[iHisto];
      if (!h2D) continue;

      //create a projection histogram along the x-axis and fit the distribution (hit timing) to get direct laser hit timing
      std::ostringstream hnameProj[2];
      hnameProj[0] << "hTime_" << pixelstr.str();
      hnameProj[1] << "hCharge_" << pixelstr.str();
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
      //if (funcLaser->GetNDF() < 1) continue;
      m_funcForLaser[iHisto] = funcLaser;

      //if the fitting is successful, create y-projection histogram with timing cut
      m_hitTiming = funcLaser->GetParameter(1);
      int binNumMin = hTime->GetXaxis()->FindBin(m_hitTiming - 2 * m_fitHalfWidth);
      int binNumMax = hTime->GetXaxis()->FindBin(m_hitTiming + 2 * m_fitHalfWidth);
      TH1D* hCharge = (TH1D*)h2D->ProjectionY(hnameProj[1].str().c_str(),
                                              binNumMin, binNumMax);
      m_chargeHistogram[iHisto] = hCharge;
    }

    m_nCalPulseHistogram = (TH1F*)f->Get("hNCalPulse");
    if (!m_nCalPulseHistogram)
      B2WARNING("TOPGainEfficiencyCalculator : no histogram for the number of events with calibration pulses identified in the given input file");
    m_thresholdForIntegral = m_threshold * m_p1HeightIntegral + m_p0HeightIntegral;
    return;
  }

  void TOPGainEfficiencyCalculatorModule::FitHistograms(EHistogramType LoadHisto)
  {
    float threshold = m_threshold;
    int globalAsicId = 0;
    if (LoadHisto == c_LoadForFitIntegral || LoadHisto == c_LoadHitRateIntegral) threshold = m_thresholdForIntegral;
    else threshold = m_threshold;

    for (int iHisto = 0 ; iHisto < c_NChannelPerPMT ; iHisto++) {
      if (m_targetPmtChId != -1 && iHisto + 1 != m_targetPmtChId) continue;

      m_pixelId = ((m_targetPmtId - 1) % c_NPMTPerRow) * c_NChannelPerPMTRow
                  + ((m_targetPmtId - 1) / c_NPMTPerRow) * c_NPixelPerModule / 2
                  + (iHisto / c_NChannelPerPMTRow) * c_NPixelPerRow + (iHisto % c_NChannelPerPMTRow) + 1;
      m_pmtChId = (iHisto + 1);
      globalAsicId = ((m_targetSlotId - 1) * c_NPixelPerModule + (m_pixelId - 1)) / c_NChannelPerAsic;
      if (LoadHisto == c_LoadForFitHeight) {
        for (auto itr = m_branch[0].begin(); itr != m_branch[0].end(); ++itr) {
          (*itr)->Fill();
        }
      }

      TH1D* hCharge = m_chargeHistogram[iHisto];
      if (!hCharge) { DummyFillBranch(LoadHisto); continue;}

      std::cout << " ***** fitting charge distribution for " << hCharge->GetName() << " *****" << std::endl;
      int nBins = hCharge->GetXaxis()->GetNbins();
      double binWidth = hCharge->GetXaxis()->GetBinUpEdge(1) - hCharge->GetXaxis()->GetBinLowEdge(1);
      double histoMax = hCharge->GetXaxis()->GetBinUpEdge(nBins);
      m_fitMax = threshold;
      double wholeIntegral = hCharge->Integral(0, hCharge->GetXaxis()->GetNbins() + 1);
      double fitRangeFraction = (m_fracFit > 0 ? m_fracFit : 1. - 10. / wholeIntegral);
      while (hCharge->Integral(0, hCharge->GetXaxis()->FindBin(m_fitMax - 0.01 * binWidth)) / wholeIntegral < fitRangeFraction)
        m_fitMax += binWidth;
      if (m_fitMax < threshold + c_NParameterGainFit * binWidth) {
        std::ostringstream fitFailedWarning;
        fitFailedWarning << "TOPGainEfficiencyCalculator : no enough entries for fitting at"
                         << " slot" << std::setw(2) << std::setfill('0') << m_targetSlotId
                         << " PMT"  << std::setw(2) << std::setfill('0') << m_targetPmtId
                         << " Ch"   << std::setw(2) << std::setfill('0') << m_pmtChId;
        B2WARNING(fitFailedWarning.str().c_str());
        DummyFillBranch(LoadHisto); continue;
      }

      std::ostringstream fname;
      fname << "func_" << (iHisto + 1);
      TObject* object = gROOT->FindObject(fname.str().c_str());
      if (object) delete object;
      TF1* func = new TF1(fname.str().c_str(), TOPGainFunc, threshold, m_fitMax, c_NParameterGainFit);
      double initGain = TMath::Max(hCharge->GetMean(), 26.1) - 25;
      double initP1 = TMath::Min(4.0, TMath::Max(10000.*TMath::Power(initGain - 25, -2), 0.01));
      double initP2 = TMath::Min(0.8 + 0.005 * TMath::Power(initP1, -3), 4.);
      double initX0 = TMath::Max(initGain * 2 - 150, 10.);
      //if (initP1 > initP2) initX0 = initX0 / 10.
      double initP1overP2 = initP1 / initP2;
      double initP0 = hCharge->GetBinContent(hCharge->GetMaximumBin())
                      / (TMath::Power(initP1overP2, initP1overP2) * TMath::Exp(-initP1overP2)) / binWidth;
      if (m_initialX0 < 0)func->SetParameter(3, initX0);
      else if (LoadHisto == c_LoadForFitHeight)
        func->SetParameter(3, 150 + 0.5 * hCharge->GetMean());
      else if (LoadHisto == c_LoadForFitIntegral)
        func->SetParameter(3, 1000 + 0.5 * hCharge->GetMean());
      if (m_initialP2 < 0)func->SetParameter(2, initP2);
      else              func->SetParameter(2, m_initialP2);
      if (m_initialP1 < 0)func->SetParameter(1, initP1);
      else              func->SetParameter(1, m_initialP1);
      if (m_initialP0 < 0)func->SetParameter(0, initP0);
      else              func->SetParameter(0, m_initialP0 * hCharge->GetEntries()*binWidth);

      func->FixParameter(4, 0);
      func->FixParameter(5, m_pedestalSigma);
      func->SetParName(0, "#it{p}_{0}");
      func->SetParName(1, "#it{p}_{1}");
      func->SetParName(2, "#it{p}_{2}");
      func->SetParName(3, "#it{x}_{0}");
      func->SetParName(4, "pedestal");
      func->SetParName(5, "pedestal #sigma");
      func->SetParLimits(0, 1e-8, 1e8);
      func->SetParLimits(1, 1e-8, 10);
      func->SetParLimits(2, 1e-8, 10);
      func->SetParLimits(3, 1e-8, 1e8);
      func->SetLineColor(2);
      func->SetLineWidth(1);
      TF1* funcFull = NULL;
      if (LoadHisto == c_LoadForFitHeight or LoadHisto == c_LoadForFitIntegral) {
        hCharge->Fit(func, m_fitoption.c_str(), "", threshold , m_fitMax);

        if (func->GetNDF() < 2) { DummyFillBranch(LoadHisto); continue;}

        double funcFullMax = histoMax * 2;
        funcFull = new TF1((fname.str() + "_full").c_str(), TOPGainFunc, (-1)*func->GetParameter(5), funcFullMax, c_NParameterGainFit);
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
        m_gain = weightedIntegral / totalWeight;
        m_efficiency = funcFull->Integral(threshold, funcFullMax) / funcFull->Integral((-1) * func->GetParameter(5), funcFullMax);
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
        m_funcFitRangeIntegral = funcFull->Integral(threshold, m_fitMax) / binWidth;
      } else std::cout << "*****fitting is skipped***** " << std::endl;
      int threBin = hCharge->GetXaxis()->FindBin(threshold + 0.01 * binWidth);
      int fitMaxBin = hCharge->GetXaxis()->FindBin(m_fitMax - 0.01 * binWidth);
      m_histoFitRangeIntegral = hCharge->Integral(threBin, fitMaxBin);

      m_histoMeanAboveThre = 0;
      for (int iBin = threBin ; iBin < nBins + 1 ; iBin++) {
        m_histoMeanAboveThre += (hCharge->GetBinContent(iBin) * hCharge->GetXaxis()->GetBinCenter(iBin));
      }
      m_histoMeanAboveThre /= hCharge->Integral(threBin, nBins);
      m_nEntries = hCharge->GetEntries();
      m_nCalPulse = (m_nCalPulseHistogram ? m_nCalPulseHistogram->GetBinContent(globalAsicId + 1) : -1);
      m_nOverflowEvents = TMath::FloorNint(hCharge->GetBinContent(nBins + 1));
      m_meanPulseHeight = hCharge->GetMean();
      m_meanPulseHeightError = hCharge->GetMeanError();
      m_hitTiming = 0;
      m_hitTimingSigma = -1;

      TF1* funcLaser = m_funcForLaser[iHisto];
      if (m_timeHistogram[iHisto] && funcLaser) {
        m_hitTiming = funcLaser->GetParameter(1);
        m_hitTimingSigma = funcLaser->GetParameter(2);
      }

      m_funcForFitRange[iHisto] = func;
      m_funcForFullRange[iHisto] = funcFull;

      for (auto itr = m_branch[LoadHisto].begin(); itr != m_branch[LoadHisto].end(); ++itr) {
        (*itr)->Fill();
      }

      std::cout << std::endl;
    }
    if (m_targetPmtChId == -1) m_tree->SetEntries(c_NChannelPerPMT);
    else m_tree->SetEntries(1);

    return;
  }

  void TOPGainEfficiencyCalculatorModule::DummyFillBranch(EHistogramType LoadHisto)
  {
    m_fitMax = -1;
    m_hitTiming = -1;
    m_hitTimingSigma = -1;
    m_nEntries = -1;
    m_nCalPulse = -1;
    m_nOverflowEvents = -1;
    m_meanPulseHeight = -1;
    m_meanPulseHeightError = -1;
    m_gain = -1;
    m_efficiency = -1;
    m_p0 = -1;
    m_p1 = -1;
    m_p2 = -1;
    m_x0 = -1;
    m_p0Error = -1;
    m_p1Error = -1;
    m_p2Error = -1;
    m_x0Error = -1;
    m_chisquare = -1;
    m_ndf = -1;
    m_funcFullRangeIntegral = -1;
    m_funcFitRangeIntegral = -1;
    m_histoFitRangeIntegral = -1;
    m_histoMeanAboveThre = -1;

    for (auto itr = m_branch[LoadHisto].begin(); itr != m_branch[LoadHisto].end(); ++itr) {
      (*itr)->Fill();
    }
  }


  void TOPGainEfficiencyCalculatorModule::DrawResult(std::string histotype, EHistogramType LoadHisto)
  {
    std::ostringstream pdfFilename;
    pdfFilename << m_outputPDFFile << "_" << histotype;
    if (m_targetPmtChId != -1)
      pdfFilename << "_" << "ch" << std::setw(2) << std::setfill('0') << m_targetPmtChId;
    pdfFilename << ".pdf";

    gStyle->SetFrameFillStyle(0);
    gStyle->SetFillStyle(0);
    gStyle->SetStatStyle(0);
    gStyle->SetOptStat(112210);
    gStyle->SetOptFit(1110);
    TCanvas* canvas = new TCanvas();
    canvas->SetFillStyle(0);
    canvas->Print((pdfFilename.str() + "[").c_str());

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

    float threshold;
    if (LoadHisto == c_LoadForFitIntegral) threshold = m_thresholdForIntegral;
    else threshold = m_threshold;

    for (int iHisto = 0 ; iHisto < c_NChannelPerPMT ; iHisto++) {

      if ((iHisto % c_NChannelPerPage) == 0) {
        canvas->Clear();
        canvas->Divide(c_NPlotsPerChannel, c_NChannelPerPage);
      }

      //2D (time vs pulse charge) histogram
      canvas->cd(3 * (iHisto % c_NChannelPerPage) + 1);
      gPad->SetFrameFillStyle(0);
      gPad->SetFillStyle(0);
      TH2F* h2D = m_timeChargeHistogram[iHisto];
      if (h2D) {
        h2D->Draw("colz");
        h2D->GetXaxis()->SetTitle("hit timing [ns]");
        h2D->GetYaxis()->SetTitle("pulse charge [ADC count]");
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
          double charge = funcLaser->GetParameter(0);
          double peakTime = funcLaser->GetParameter(1);
          float xMin = hTime->GetXaxis()->GetBinLowEdge(hTime->GetXaxis()->FindBin(peakTime - 2 * m_fitHalfWidth));
          float xMax = hTime->GetXaxis()->GetBinUpEdge(hTime->GetXaxis()->FindBin(peakTime + 2 * m_fitHalfWidth));
          line->DrawLine(xMin, 0.5, xMin, charge * 2.);
          line->DrawLine(xMax, 0.5, xMax, charge * 2.);
          arrow->DrawArrow(xMin, charge * 1.5, xMax, charge * 1.5, 0.01, "<>");
        }
      }

      //charge histogram with fit result (after timing cut)
      canvas->cd(c_NPlotsPerChannel * (iHisto % c_NChannelPerPage) + 3);
      gPad->SetFrameFillStyle(0);
      gPad->SetFillStyle(0);
      TH1D* hCharge = m_chargeHistogram[iHisto];
      if (hCharge) {
        gPad->SetLogy();
        hCharge->Draw();
        hCharge->SetLineColor(1);
        hCharge->GetXaxis()->SetTitle("charge [ADC counts]");
        float binWidth = hCharge->GetXaxis()->GetBinUpEdge(1) - hCharge->GetXaxis()->GetBinLowEdge(1);
        std::ostringstream ytitle;
        ytitle << "Entries [/(" << binWidth << " ADC counts)]";
        hCharge->GetYaxis()->SetTitle(ytitle.str().c_str());

        if (m_funcForFitRange[iHisto] && m_funcForFullRange[iHisto]) {
          m_funcForFullRange[iHisto]->Draw("same");
          m_funcForFitRange[iHisto]->Draw("same");
          double charge = hCharge->GetBinContent(hCharge->GetMaximumBin());
          line->DrawLine(threshold, 0.5, threshold, charge * 2.);

          if ((object = gROOT->FindObject("dummy"))) delete object;
          std::ostringstream cut;
          cut << "pmtChId==" << (iHisto + 1);
          long nEntries = 0;
          std::ostringstream summarystr[2];
          if (LoadHisto == c_LoadForFitHeight) {
            nEntries = m_tree->Project("dummy", "gain:efficiency", cut.str().c_str());
            if (nEntries == 1) {
              summarystr[0] << "gain = " << std::setiosflags(std::ios::fixed) << std::setprecision(1)
                            << m_tree->GetV1()[0];
              latex->DrawLatex(0.875, 0.34, summarystr[0].str().c_str());
              summarystr[1] << "efficiency = " << std::setiosflags(std::ios::fixed) << std::setprecision(1)
                            << (m_tree->GetV2()[0] * 100) << " %";
              latex->DrawLatex(0.875, 0.29, summarystr[1].str().c_str());
            }
          } else if (LoadHisto == c_LoadForFitIntegral) {
            nEntries = m_tree->Project("dummy", "gainUseIntegral:efficiencyUseIntegral", cut.str().c_str());
            if (nEntries == 1) {
              summarystr[0] << "gain = " << std::setiosflags(std::ios::fixed) << std::setprecision(1)
                            << m_tree->GetV1()[0];
              latex->DrawLatex(0.875, 0.34, summarystr[0].str().c_str());
              summarystr[1] << "efficiency = " << std::setiosflags(std::ios::fixed) << std::setprecision(1)
                            << (m_tree->GetV2()[0] * 100) << " %";
              latex->DrawLatex(0.875, 0.29, summarystr[1].str().c_str());
            }
          }

          if (nEntries > 1) {
            B2WARNING("TOPGainEfficiencyCalculator : mutliple entries with the same channel ID ("
                      << m_pmtChId << ") in the output TTree");
          }
        }
      }

      if (((iHisto + 1) % c_NChannelPerPage) == 0)
        canvas->Print(pdfFilename.str().c_str());
    }
    for (int iHisto = (c_NChannelPerPMT - 1) % c_NChannelPerPage + 1 ; iHisto < c_NChannelPerPage ; iHisto++) {
      for (int iPad = 0 ; iPad < c_NPlotsPerChannel ; iPad++) {
        canvas->cd(c_NPlotsPerChannel * (iHisto % c_NChannelPerPage) + iPad + 1);
        gPad->SetFrameFillStyle(0);
        gPad->SetFillStyle(0);
      }
      if (((iHisto + 1) % c_NChannelPerPage) == 0)
        canvas->Print(pdfFilename.str().c_str());
    }

    canvas->Print((pdfFilename.str() + "]").c_str());

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
    double peakCharge = histo->GetBinContent(iBin);
    while (true) {
      iBin++;
      double x = histo->GetXaxis()->GetBinCenter(iBin);
      if (x > xmax) break;

      double binEntry = histo->GetBinContent(iBin);
      if (binEntry > peakCharge) {
        peakPos = x;
        peakCharge = binEntry;
      }
    }

    return peakPos;
  }



} // end Belle2 namespace
