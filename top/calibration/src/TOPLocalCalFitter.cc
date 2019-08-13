/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Analyze a tree contaninig laser hit timing and channel, returning      *
 * a tree  with the fit results and the histograms for each channel.      *
 * It can be used to produce both channelT0 calibrations and to analyze   *
 * the daily, low statistics laser runs                                   *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/**************************************************************************                                                                                                                                        * BASF2 (Belle Analysis Framework 2)                                     *                                                                                                                                        * Copyright(C) 2019 - Belle II Collaboration                             *                                                                                                                                        *                                                                        *                                                                                                                                        * Author: The Belle II Collaboration                                     *                                                                                                                                        * Contributors: Umberto Tamponi                                          *                                                                                                                                        *                                                                        *                                                                                                                                        * This software is provided "as is" without any warranty.                *                                                                                                                                        **************************************************************************/


#include <top/calibration/TOPLocalCalFitter.h>

#include "TH2F.h"
#include "TFile.h"
#include "TMath.h"
#include "TF1.h"
#include "TMinuit.h"
#include "TROOT.h"
#include "TTree.h"
#include "math.h"
#include <limits>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace TOP;

double crystalball_function(double x, double alpha, double n, double sigma, double mean)
{
  // evaluate the crystal ball function
  if (sigma < 0.)     return 0.;
  double z = (x - mean) / sigma;
  if (alpha < 0) z = -z;
  double abs_alpha = std::abs(alpha);
  if (z  > - abs_alpha)
    return std::exp(- 0.5 * z * z);
  else {
    double nDivAlpha = n / abs_alpha;
    double AA =  std::exp(-0.5 * abs_alpha * abs_alpha);
    double B = nDivAlpha - abs_alpha;
    double arg = nDivAlpha / (B - z);
    return AA * std::pow(arg, n);
  }
}

double crystalball_pdf(double x, double alpha, double n, double sigma, double mean)
{
  // evaluation of the PDF ( is defined only for n >1)
  if (sigma < 0.)     return 0.;
  if (n <= 1) return std::numeric_limits<double>::quiet_NaN();   // pdf is not normalized for n <=1
  double abs_alpha = std::abs(alpha);
  double C = n / abs_alpha * 1. / (n - 1.) * std::exp(-alpha * alpha / 2.);
  double D = std::sqrt(M_PI / 2.) * (1. + erf(abs_alpha / std::sqrt(2.)));
  double N = 1. / (sigma * (C + D));
  return N * crystalball_function(x, alpha, n, sigma, mean);
}



// Two gaussians to model the TTS of the MCP-PMTs.
double TTSPDF(double x, double time, double deltaT, double sigma1, double sigma2, double f1)
{
  return f1 * TMath::Gaus(x, time, sigma1, kTRUE) + (1 - f1) * TMath::Gaus(x, time + deltaT, sigma2, kTRUE)  ;
}


// Full PDF to fit the laser, made of:
// 2 TTSPDF
// 1 crystal ball PDF for the extra peak at +1 ns we don't understand
// 1 gaussian to help modelling the tail
double laserPDF(double* x, double* p)
{
  double time = p[0];
  double sigma = p[1];
  double fraction = p[2];
  double deltaTLaser = p[3];
  double sigmaRatio = p[4];
  double deltaTTS = p[5];
  double f1 = p[6];
  double norm = p[7];

  double deltaTExtra = p[8];
  double sigmaExtra = p[9];
  double normExtra = p[10];

  double deltaTBkg  = p[11];
  double sigmaBkg = p[12];
  double bkg = p[13];

  double alpha = p[14];
  double n = p[15];

  double mainPeak = fraction * TTSPDF(x[0], time, deltaTTS, sigma, TMath::Sqrt(sigma * sigma + sigmaRatio * sigmaRatio), f1);
  double secondaryPeak = (1. - fraction) * TTSPDF(x[0], time + deltaTLaser, deltaTTS, sigma,
                                                  TMath::Sqrt(sigma * sigma + sigmaRatio * sigmaRatio), f1);
  double extraPeak = crystalball_pdf(x[0], alpha, n, sigmaExtra,  time + deltaTExtra);
  double background = TMath::Gaus(x[0], time + deltaTBkg + deltaTTS, sigmaBkg, kTRUE);

  return norm * (mainPeak + secondaryPeak) + normExtra * extraPeak + bkg * background;
}


TOPLocalCalFitter::TOPLocalCalFitter(): CalibrationAlgorithm("TOPLaserCalibratorCollector")
{
  setDescription(
    "Perform the fit of the laser and pulser runs"
  );

}



void  TOPLocalCalFitter::loadMCInfoTrees()
{
  m_inputTTS = TFile::Open(m_TTSData.c_str());
  m_inputConstraints = TFile::Open(m_fitConstraints.c_str());

  B2INFO("Getting the  TTS parameters from " << m_TTSData);
  m_inputTTS->cd();
  m_inputTTS->GetObject("tree", m_treeTTS);
  m_treeTTS->SetBranchAddress("mean2", &m_mean2);
  m_treeTTS->SetBranchAddress("sigma1", &m_sigma1);
  m_treeTTS->SetBranchAddress("sigma2", &m_sigma2);
  m_treeTTS->SetBranchAddress("fraction1", &m_f1);
  m_treeTTS->SetBranchAddress("fraction2", &m_f2);
  m_treeTTS->SetBranchAddress("pixelRow", &m_pixelRow);
  m_treeTTS->SetBranchAddress("pixelCol", &m_pixelCol);

  std::cout << "Getting the laser fit parameters from " << m_fitConstraints << std::endl;
  m_inputConstraints->cd();
  m_inputConstraints->GetObject("fitTree", m_treeConstraints);
  m_treeConstraints->SetBranchAddress("peakTime", &m_peakTimeConstraints);
  m_treeConstraints->SetBranchAddress("deltaT", &m_deltaTConstraints);
  m_treeConstraints->SetBranchAddress("fraction", &m_fractionConstraints);

  //  m_treeConstraints->Show(0);
  //  m_treeTTS->Show(0);

  return;
}


void  TOPLocalCalFitter::setupOutputTreeAndFile()
{
  m_histFile = new TFile(m_output.c_str(), "recreate");
  m_histFile->cd();
  m_fitTree = new TTree("fitTree", "fitTree");
  m_fitTree->Branch<short>("channel", &m_channel);
  m_fitTree->Branch<short>("slot", &m_slot);
  m_fitTree->Branch<short>("row", &m_row);
  m_fitTree->Branch<short>("col", &m_col);
  m_fitTree->Branch<float>("peakTime", &m_peakTime);
  m_fitTree->Branch<float>("peakTimeErr", &m_peakTimeErr);
  m_fitTree->Branch<float>("deltaT", &m_deltaT);
  m_fitTree->Branch<float>("deltaTErr", &m_deltaTErr);
  m_fitTree->Branch<float>("sigma", &m_sigma);
  m_fitTree->Branch<float>("sigmaErr", &m_sigmaErr);
  m_fitTree->Branch<float>("fraction", &m_fraction);
  m_fitTree->Branch<float>("fractionErr", &m_fractionErr);
  m_fitTree->Branch<float>("yieldLaser", &m_yieldLaser);
  m_fitTree->Branch<float>("yieldLaserErr", &m_yieldLaserErr);
  m_fitTree->Branch<float>("timeExtra", &m_timeExtra);
  m_fitTree->Branch<float>("sigmaExtra", &m_sigmaExtra);
  m_fitTree->Branch<float>("nExtra", &m_nExtra);
  m_fitTree->Branch<float>("alphaExtra", &m_alphaExtra);
  m_fitTree->Branch<float>("yieldLaserExtra", &m_yieldLaserExtra);
  m_fitTree->Branch<float>("timeBackground", &m_timeBackground);
  m_fitTree->Branch<float>("sigmaBackground", &m_sigmaBackground);
  m_fitTree->Branch<float>("yieldLaserBackground", &m_yieldLaserBackground);

  m_fitTree->Branch<float>("fractionMC", &m_fractionMC);
  m_fitTree->Branch<float>("deltaTMC", &m_deltaTMC);
  m_fitTree->Branch<float>("peakTimeMC", &m_peakTimeMC);
  m_fitTree->Branch<float>("firstPulserTime", &m_firstPulserTime);
  m_fitTree->Branch<float>("firstPulserSigma", &m_firstPulserSigma);
  m_fitTree->Branch<float>("secondPulserTime", &m_secondPulserTime);
  m_fitTree->Branch<float>("secondPulserSigma", &m_secondPulserSigma);
  m_fitTree->Branch<float>("channelT0", &m_channelT0);
  m_fitTree->Branch<float>("channelT0Err", &m_channelT0Err);
  m_fitTree->Branch<short>("fitStatus", &m_fitStatus);

  m_fitTree->Branch<float>("chi2", &m_chi2);
  m_fitTree->Branch<float>("rms", &m_rms);
  return;
}




void  TOPLocalCalFitter::fitChannel(short iSlot, short iChannel, TH1* h_profile)
{

  // loads the TTS infos and the fit constraint for the given channel and slot
  m_treeConstraints->GetEntry(iChannel);
  m_treeTTS->GetEntry(iChannel + 512 * iSlot);
  // finds the maximum of the hit timing histogram and adjust the histogram range around it (3 ns window)
  double maxpos = h_profile->GetBinCenter(h_profile->GetMaximumBin());
  h_profile->GetXaxis()->SetRangeUser(maxpos - 1, maxpos + 2.);

  // gets the histogram integral to give a starting value to the fitter
  double integral = h_profile->Integral();

  // creates the fit function
  TF1 laser = TF1("laser", laserPDF, maxpos - 1, maxpos + 2., 16);

  // par[0] = peakTime
  laser.SetParameter(0, maxpos);
  laser.SetParLimits(0, maxpos - 0.06, maxpos + 0.06);

  // par[1] = sigma
  laser.SetParameter(1, 0.1);
  laser.SetParLimits(1, 0.05, 0.25);

  // par[2] = fraction of the main peak respect to the total
  laser.SetParameter(2, m_fractionConstraints);
  laser.SetParLimits(2, 0.5, 1.);

  // par[3]= time difference between the main and secondary path. fixed to the MC value
  laser.FixParameter(3, m_deltaTConstraints);

  // This is an hack: in some channels the MC sees one peak only, while in the data there are clearly
  // two well distinguished peaks. This will disappear if we'll ever get a better laser simulation.
  if (m_deltaTConstraints > -0.001) {
    laser.SetParameter(3, -0.3);
    laser.SetParLimits(3, -0.4, -0.2);
  }

  // par[4] is the quadratic difference of the sigmas of the two TTS gaussians (tail - core)
  laser.FixParameter(4, TMath::Sqrt(m_sigma2 * m_sigma2 - m_sigma1 * m_sigma1));
  // par[5] is the position of the second TTS gaussian w/ respect to the first one
  laser.FixParameter(5, m_mean2);
  // par[6] is the relative contribution of the second TTS gaussian
  laser.FixParameter(6, m_f1);

  // par[7] is the PDF normalization, = integral*bin width
  laser.SetParameter(7,  integral * 0.005);
  laser.SetParLimits(7,  0.2 * integral * 0.005, 2.*integral * 0.005);

  // par[8-10] are the relative position, the sigma and the integral of the extra peak
  laser.SetParameter(8, 1.);
  laser.SetParLimits(8, 0.3, 2.);
  laser.SetParameter(9, 0.2);
  laser.SetParLimits(9, 0.08, 1.);
  laser.SetParameter(10,  0.1 * integral * 0.005);
  laser.SetParLimits(10,  0., 0.2 * integral * 0.005);
  // par[14-15] are the tail parameters of the crystal ball function used to describe the extra peak
  laser.SetParameter(14, -2.);
  laser.SetParameter(15, 2.);
  laser.SetParLimits(15, 1.01, 20.);

  // par[11-13] are relative position, sigma and integral of the broad gaussian added to better describe the tail at high times
  laser.SetParameter(11, 1.);
  laser.SetParLimits(11, 0.1, 5.);
  laser.SetParameter(12, 0.8);
  laser.SetParLimits(12, 0., 5.);
  laser.SetParameter(13,  0.01 * integral * 0.005);
  laser.SetParLimits(13,  0., 0.2 * integral * 0.005);

  // if it's a monitoring fit, fix a buch more parameters. THIS IS WORK IN PROGRESS
  if (m_isMonitoringFit) {
    laser.FixParameter(2, m_fractionConstraints);
    laser.FixParameter(3, m_deltaTConstraints);
  }

  // make the plot of the fit function nice setting 2000 sampling points
  laser.SetNpx(2000);


  // do the fit!
  h_profile->Fit("laser", "R L Q");


  // Add by hand the different fit components to the histogram, mostly for debugging/presentation purposes
  TF1* peak1 = new TF1("peak1", laserPDF, maxpos - 1, maxpos + 2., 16);
  TF1* peak2 = new TF1("peak2", laserPDF, maxpos - 1, maxpos + 2., 16);
  TF1* extra = new TF1("extra", laserPDF, maxpos - 1, maxpos + 2., 16);
  TF1* background = new TF1("background", laserPDF, maxpos - 1, maxpos + 2., 16);
  for (int iPar = 0; iPar < 16; iPar++) {
    peak1->FixParameter(iPar, laser.GetParameter(iPar));
    peak2->FixParameter(iPar, laser.GetParameter(iPar));
    extra->FixParameter(iPar, laser.GetParameter(iPar));
    background->FixParameter(iPar, laser.GetParameter(iPar));
  }
  peak1->FixParameter(2, 0.);
  peak1->FixParameter(7, (1 - laser.GetParameter(2))*laser.GetParameter(7));
  peak1->FixParameter(10, 0.);
  peak1->FixParameter(13, 0.);

  peak2->FixParameter(2, 1.);
  peak2->FixParameter(7, laser.GetParameter(2)*laser.GetParameter(7));
  peak2->FixParameter(10, 0.);
  peak2->FixParameter(13, 0.);

  extra->FixParameter(7, 0.);
  extra->FixParameter(13, 0.);

  background->FixParameter(7, 0.);
  background->FixParameter(10, 0.);

  h_profile->GetListOfFunctions()->Add(peak1);
  h_profile->GetListOfFunctions()->Add(peak2);
  h_profile->GetListOfFunctions()->Add(extra);
  h_profile->GetListOfFunctions()->Add(background);


  // save the results in the variables linked to the tree branches
  m_channel = iChannel;
  m_row = m_pixelRow;
  m_col = m_pixelCol;
  m_slot = iSlot + 1;
  m_peakTime = laser.GetParameter(0);
  m_peakTimeErr = laser.GetParError(0);
  m_deltaT = laser.GetParameter(3);
  m_deltaTErr = laser.GetParError(3);
  m_sigma = laser.GetParameter(1);
  m_sigmaErr = laser.GetParError(1);
  m_fraction = laser.GetParameter(2);
  m_fractionErr = laser.GetParError(2);
  m_yieldLaser = laser.GetParameter(7) / 0.005;
  m_yieldLaserErr = laser.GetParError(7) / 0.005;
  m_timeExtra = laser.GetParameter(8);
  m_sigmaExtra = laser.GetParameter(9);
  m_yieldLaserExtra = laser.GetParameter(10) / 0.005;
  m_alphaExtra = laser.GetParameter(14);
  m_nExtra = laser.GetParameter(15);
  m_timeBackground = laser.GetParameter(11);
  m_sigmaBackground = laser.GetParameter(12);
  m_yieldLaserBackground = laser.GetParameter(13) / 0.005;
  m_channelT0 = m_peakTime - m_peakTimeMC;
  m_channelT0Err = m_peakTimeErr;
  m_chi2 = laser.GetChisquare() / laser.GetNDF();

  // copy some MC information to the output tree
  m_fractionMC = m_fractionConstraints;
  m_deltaTMC = m_deltaTConstraints;
  m_peakTimeMC = m_peakTimeConstraints;

  return;
}


void  TOPLocalCalFitter::fitPulser(TH1* h_profileFirstPulser, TH1* h_profileSecondPulser)
{
  float maxpos = h_profileFirstPulser->GetBinCenter(h_profileFirstPulser->GetMaximumBin());
  h_profileFirstPulser->GetXaxis()->SetRangeUser(maxpos - 1, maxpos + 1.);
  if (h_profileFirstPulser->Integral() > 1000) {
    TF1 pulser1 = TF1("pulser1", "[0]*TMath::Gaus(x, [1], [2], kTRUE)", maxpos - 1, maxpos + 1.);
    pulser1.SetParameter(0, 1.);
    pulser1.SetParameter(1, maxpos);
    pulser1.SetParameter(2, 0.05);
    h_profileFirstPulser->Fit("pulser1", "R Q");
    m_firstPulserTime = pulser1.GetParameter(1);
    m_firstPulserSigma = pulser1.GetParameter(2);
    h_profileFirstPulser->Write();
  } else {
    m_firstPulserTime = -999;
    m_firstPulserSigma = -999;
  }

  maxpos = h_profileSecondPulser->GetBinCenter(h_profileSecondPulser->GetMaximumBin());
  h_profileSecondPulser->GetXaxis()->SetRangeUser(maxpos - 1, maxpos + 1.);
  if (h_profileSecondPulser->Integral() > 1000) {
    TF1 pulser2 = TF1("pulser2", "[0]*TMath::Gaus(x, [1], [2], kTRUE)", maxpos - 1, maxpos + 1.);
    pulser2.SetParameter(0, 1.);
    pulser2.SetParameter(1, maxpos);
    pulser2.SetParameter(2, 0.05);
    h_profileSecondPulser->Fit("pulser2", "R Q");
    m_secondPulserTime = pulser2.GetParameter(1);
    m_secondPulserSigma = pulser2.GetParameter(2);
    h_profileSecondPulser->Write();
  } else {
    m_secondPulserTime = -999;
    m_secondPulserSigma = -999;
  }
  return;
}



void  TOPLocalCalFitter::determineFitStatus()
{
  if (m_chi2 < 4 && m_sigma < 0.2 && m_yieldLaser > 1000) {
    m_fitStatus = 0;
  } else {
    m_fitStatus = 1;
  }
  return;
}





CalibrationAlgorithm::EResult TOPLocalCalFitter::calibrate()
{
  gROOT->SetBatch();

  loadMCInfoTrees();


  setupOutputTreeAndFile();

  // Loads the tree with the hits
  auto hitTree = getObjectPtr<TTree>("hitTree");
  TH2F* h_hitTime = new TH2F("h_hitTime", " ", 512 * 16, 0., 512 * 16, 20000, -60, 40.); // 10 ps bins
  hitTree->Draw("hitTime:(channel+(slot-1)*512)>>h_hitTime", "dVdt > 80 && amplitude > 80");

  m_histFile->cd();

  for (short iSlot = 0; iSlot < 16; iSlot++) {
    std::cout << "fitting slot " << iSlot + 1 << std::endl;
    for (short iChannel = 0; iChannel < 512; iChannel++) {
      std::cout << "fitting channel " << iChannel << std::endl;
      TH1D* h_profile = h_hitTime->ProjectionY(("profile_" + std::to_string(iSlot + 1) + "_" + std::to_string(iChannel)).c_str(),
                                               iSlot * 512 + iChannel + 1, iSlot * 512 + iChannel + 1);
      h_profile->GetXaxis()->SetRangeUser(-100, -5);
      std::cout << "entries: " << h_profile->GetEntries() << std::endl;
      fitChannel(iSlot, iChannel, h_profile);
      determineFitStatus();

      // Now let's fit the pulser
      TH1D* h_profileFirstPulser = h_hitTime->ProjectionY(("profileFirstPulser_" + std::to_string(iSlot + 1) + "_" + std::to_string(
                                                             iChannel)).c_str(),  iSlot * 512 + iChannel + 1, iSlot * 512 + iChannel + 1);
      TH1D* h_profileSecondPulser = h_hitTime->ProjectionY(("profileSecondPulser_" + std::to_string(iSlot + 1) + "_" + std::to_string(
                                                              iChannel)).c_str(),  iSlot * 512 + iChannel + 1, iSlot * 512 + iChannel + 1);
      h_profileFirstPulser->GetXaxis()->SetRangeUser(-10, 10);
      h_profileSecondPulser->GetXaxis()->SetRangeUser(10, 40);

      fitPulser(h_profileFirstPulser, h_profileSecondPulser);

      m_fitTree->Fill();

      h_profile->Write();
      h_profileFirstPulser->Write();
      h_profileSecondPulser->Write();
    }

    h_hitTime->Write();
  }

  m_fitTree->Write();
  m_histFile->Close();
  return c_OK;
}
