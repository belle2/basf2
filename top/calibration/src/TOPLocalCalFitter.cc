/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/calibration/TOPLocalCalFitter.h>

#include "TH2F.h"
#include "TFile.h"
#include "TMath.h"
#include "TF1.h"
#include "TROOT.h"
#include "TTree.h"
#include "math.h"
#include <limits>
#include <framework/logging/Logger.h>
#include <top/dbobjects/TOPCalChannelT0.h>


//using namespace std;
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

  if (m_fitterMode == "MC")
    std::cout << "Running in MC mode, not constraints will be set" << std::endl;
  else {
    B2INFO("Getting the laser fit parameters from " << m_fitConstraints);
    m_inputConstraints->cd();
    m_inputConstraints->GetObject("fitTree", m_treeConstraints);
    m_treeConstraints->SetBranchAddress("peakTime", &m_peakTimeConstraints);
    m_treeConstraints->SetBranchAddress("deltaT", &m_deltaTConstraints);
    m_treeConstraints->SetBranchAddress("fraction", &m_fractionConstraints);
    if (m_fitterMode == "monitoring") {
      m_treeConstraints->SetBranchAddress("timeExtra", &m_timeExtraConstraints);
      m_treeConstraints->SetBranchAddress("sigmaExtra", &m_sigmaExtraConstraints);
      m_treeConstraints->SetBranchAddress("alphaExtra", &m_alphaExtraConstraints);
      m_treeConstraints->SetBranchAddress("nExtra", &m_nExtraConstraints);
      m_treeConstraints->SetBranchAddress("timeBackground", &m_timeBackgroundConstraints);
      m_treeConstraints->SetBranchAddress("sigmaBackground", &m_sigmaBackgroundConstraints);
    }
  }
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
  m_fitTree->Branch<short>("fitStatus", &m_fitStatus);

  m_fitTree->Branch<float>("chi2", &m_chi2);
  m_fitTree->Branch<float>("rms", &m_rms);


  if (m_isFitInAmplitudeBins) {
    m_timewalkTree = new TTree("timewalkTree", "timewalkTree");

    m_timewalkTree->Branch<float>("binLowerEdge", &m_binLowerEdge);
    m_timewalkTree->Branch<float>("binUpperEdge", &m_binUpperEdge);
    m_timewalkTree->Branch<short>("channel", &m_channel);
    m_timewalkTree->Branch<short>("slot", &m_slot);
    m_timewalkTree->Branch<short>("row", &m_row);
    m_timewalkTree->Branch<short>("col", &m_col);
    m_timewalkTree->Branch<float>("histoIntegral", &m_histoIntegral);
    m_timewalkTree->Branch<float>("peakTime", &m_peakTime);
    m_timewalkTree->Branch<float>("peakTimeErr", &m_peakTimeErr);
    m_timewalkTree->Branch<float>("deltaT", &m_deltaT);
    m_timewalkTree->Branch<float>("deltaTErr", &m_deltaTErr);
    m_timewalkTree->Branch<float>("sigma", &m_sigma);
    m_timewalkTree->Branch<float>("sigmaErr", &m_sigmaErr);
    m_timewalkTree->Branch<float>("fraction", &m_fraction);
    m_timewalkTree->Branch<float>("fractionErr", &m_fractionErr);
    m_timewalkTree->Branch<float>("yieldLaser", &m_yieldLaser);
    m_timewalkTree->Branch<float>("yieldLaserErr", &m_yieldLaserErr);
    m_timewalkTree->Branch<float>("timeExtra", &m_timeExtra);
    m_timewalkTree->Branch<float>("sigmaExtra", &m_sigmaExtra);
    m_timewalkTree->Branch<float>("nExtra", &m_nExtra);
    m_timewalkTree->Branch<float>("alphaExtra", &m_alphaExtra);
    m_timewalkTree->Branch<float>("yieldLaserExtra", &m_yieldLaserExtra);
    m_timewalkTree->Branch<float>("timeBackground", &m_timeBackground);
    m_timewalkTree->Branch<float>("sigmaBackground", &m_sigmaBackground);
    m_timewalkTree->Branch<float>("yieldLaserBackground", &m_yieldLaserBackground);

    m_timewalkTree->Branch<float>("fractionMC", &m_fractionMC);
    m_timewalkTree->Branch<float>("deltaTMC", &m_deltaTMC);
    m_timewalkTree->Branch<float>("peakTimeMC", &m_peakTimeMC);
    m_timewalkTree->Branch<float>("firstPulserTime", &m_firstPulserTime);
    m_timewalkTree->Branch<float>("firstPulserSigma", &m_firstPulserSigma);
    m_timewalkTree->Branch<float>("secondPulserTime", &m_secondPulserTime);
    m_timewalkTree->Branch<float>("secondPulserSigma", &m_secondPulserSigma);
    m_timewalkTree->Branch<short>("fitStatus", &m_fitStatus);

    m_timewalkTree->Branch<float>("chi2", &m_chi2);
    m_timewalkTree->Branch<float>("rms", &m_rms);

  }

  return;
}




void  TOPLocalCalFitter::fitChannel(short iSlot, short iChannel, TH1* h_profile)
{

  // loads the TTS infos and the fit constraint for the given channel and slot
  if (m_fitterMode == "monitoring")
    m_treeConstraints->GetEntry(iChannel + 512 * iSlot);
  else if (m_fitterMode == "calibration") // The MC-based constraint file has only slot 1 at the moment
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
  if (m_fitterMode == "MC") {
    laser.SetParameter(1, 0.02);
    laser.SetParLimits(1, 0., 0.04);
  }
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
  if (m_fitterMode == "MC")
    laser.FixParameter(6, 0);

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

  // if it's a monitoring fit, fix a buch more parameters.
  if (m_fitterMode == "monitoring") {
    laser.FixParameter(2, m_fractionConstraints);
    laser.FixParameter(3, m_deltaTConstraints);
    laser.FixParameter(8, m_timeExtraConstraints);
    laser.FixParameter(9, m_sigmaExtraConstraints);
    laser.FixParameter(14, m_alphaExtraConstraints);
    laser.FixParameter(15, m_nExtraConstraints);
    laser.FixParameter(11, m_timeBackgroundConstraints);
    laser.FixParameter(12, m_sigmaBackgroundConstraints);
  }


  // if it's a MC fit, fix a buch more parameters.
  if (m_fitterMode == "MC") {
    laser.SetParameter(2, 0.8);
    laser.SetParLimits(2, 0., 1.);
    laser.SetParameter(3, -0.1);
    laser.SetParLimits(3, -0.4, -0.);
    // The following are just random reasonable number, only to pin-point the tail components to some value and remove them form the fit
    laser.FixParameter(8, 0);
    laser.FixParameter(9, 0.1);
    laser.FixParameter(14, -2.);
    laser.FixParameter(15, 2);
    laser.FixParameter(11, 1.);
    laser.FixParameter(12, 0.1);
    laser.FixParameter(13, 0.);
    laser.FixParameter(10, 0.);
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


void TOPLocalCalFitter::calculateChennelT0()
{
  int nEntries = m_fitTree->GetEntries();
  if (nEntries != 8192) {
    B2ERROR("fitTree does not contain an entry wit a fit result for each channel. Found " << nEntries <<
            " instead of 8192. Perhaps you tried to run the commonT0 calculation before finishing the fitting?");
    return;
  }

  // Create and fill the TOPCalChannelT0 object.
  // This part is mostly copy-pasted from the DB importer used up to Jan 2020
  auto* channelT0 = new TOPCalChannelT0();
  short nCal[16] = {0};
  for (int i = 0; i < nEntries; i++) {
    m_fitTree->GetEntry(i);
    channelT0->setT0(m_slot, m_channel, m_peakTime - m_peakTimeMC, m_peakTimeErr);
    if (m_fitStatus == 0) {
      nCal[m_slot - 1]++;
    } else {
      channelT0->setUnusable(m_slot, m_channel);
    }
  }

  // Normalize the constants
  channelT0->suppressAverage();

  // create the localDB
  saveCalibration(channelT0);

  short nCalTot = 0;
  B2INFO("Summary: ");
  for (int iSlot = 1; iSlot < 17; iSlot++) {
    B2INFO("--> Number of calibrated channels on Slot " << iSlot << " : " << nCal[iSlot - 1] << "/512");
    B2INFO("--> Cal on ch 1, 256 and 511:    " << channelT0->getT0(iSlot, 0) << ", " << channelT0->getT0(iSlot,
           257) << ", " << channelT0->getT0(iSlot, 511));
    nCalTot += nCal[iSlot - 1];
  }

  B2RESULT("Channel T0 calibration constants imported to database, calibrated channels: " << nCalTot << "/ 8192");


  // Loop again on the output tree to save the constants there too, adding two more branches.
  TBranch* channelT0Branch = m_fitTree->Branch<float>("channelT0", &m_channelT0);
  TBranch* channelT0ErrBranch = m_fitTree->Branch<float>("channelT0Err", &m_channelT0Err);

  for (int i = 0; i < nEntries; i++) {
    m_fitTree->GetEntry(i);
    m_channelT0 = channelT0->getT0(m_slot, m_channel);
    m_channelT0Err = channelT0->getT0Error(m_slot, m_channel);
    channelT0Branch->Fill();
    channelT0ErrBranch->Fill();
  }
  return ;

}



CalibrationAlgorithm::EResult TOPLocalCalFitter::calibrate()
{
  gROOT->SetBatch();

  loadMCInfoTrees();


  setupOutputTreeAndFile();

  // Loads the tree with the hits
  auto hitTree = getObjectPtr<TTree>("hitTree");
  TH2F* h_hitTime = new TH2F("h_hitTime", " ", 512 * 16, 0., 512 * 16, 22000, -70, 40.); // 5 ps bins

  float amplitude, hitTime;
  short channel, slot;
  bool refTimeValid;
  hitTree->SetBranchAddress("amplitude", &amplitude);
  hitTree->SetBranchAddress("hitTime", &hitTime);
  hitTree->SetBranchAddress("channel", &channel);
  hitTree->SetBranchAddress("slot", &slot);
  hitTree->SetBranchAddress("refTimeValid", &refTimeValid);

  // An attempt to speed things up looping over the tree only once.
  std::vector<TH2F*> h_hitTimeLaserHistos = {};
  for (int iLowerEdge = 0; iLowerEdge < (int)m_binEdges.size() - 1; iLowerEdge++) {
    TH2F* h_hitTimeLaser = new TH2F(("h_hitTimeLaser_" + std::to_string(iLowerEdge + 1)).c_str(), " ", 512 * 16, 0., 512 * 16, 14000,
                                    -70, 0.); // 5 ps bins
    h_hitTimeLaserHistos.push_back(h_hitTimeLaser);
  }

  for (unsigned int i = 0; i < hitTree->GetEntries(); i++) {
    auto onepc = (unsigned int)(hitTree->GetEntries() / 100);
    if (i % onepc == 0)
      std::cout << "processing hit " << i << " of " << hitTree->GetEntries() << " (" << i / (onepc * 10) << " %)" << std::endl;
    hitTree->GetEntry(i);
    auto it = std::lower_bound(m_binEdges.cbegin(),  m_binEdges.cend(), amplitude);
    int iLowerEdge = std::distance(m_binEdges.cbegin(), it) - 1;

    if (iLowerEdge >= 0 && iLowerEdge < static_cast<int>(m_binEdges.size()) - 1 && refTimeValid)
      h_hitTimeLaserHistos[iLowerEdge]->Fill(channel + (slot - 1) * 512, hitTime);
    if (amplitude > 80. &&  refTimeValid)
      h_hitTime->Fill(channel + (slot - 1) * 512, hitTime);
  }

  m_histFile->cd();
  h_hitTime->Write();

  for (short iSlot = 0; iSlot < 16; iSlot++) {
    std::cout << "fitting slot " << iSlot + 1 << std::endl;
    for (short iChannel = 0; iChannel < 512; iChannel++) {
      TH1D* h_profile = h_hitTime->ProjectionY(("profile_" + std::to_string(iSlot + 1) + "_" + std::to_string(iChannel)).c_str(),
                                               iSlot * 512 + iChannel + 1, iSlot * 512 + iChannel + 1);
      if (m_fitterMode == "MC")
        h_profile->GetXaxis()->SetRangeUser(-10, -10);
      else
        h_profile->GetXaxis()->SetRangeUser(-65,
                                            -5); // if you will even change it, make sure not to include the h_hitTime overflow bins in this range
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

  calculateChennelT0();

  m_fitTree->Write();


  if (m_isFitInAmplitudeBins) {
    std::cout << "Fitting in bins" << std::endl;
    for (int iLowerEdge = 0; iLowerEdge < (int)m_binEdges.size() - 1; iLowerEdge++) {
      m_binLowerEdge = m_binEdges[iLowerEdge];
      m_binUpperEdge = m_binEdges[iLowerEdge + 1];
      std::cout << "Fitting the amplitude interval (" <<  m_binLowerEdge << ", " << m_binUpperEdge << " )" << std::endl;

      for (short iSlot = 0; iSlot < 16; iSlot++) {
        std::cout << "   Fitting slot " << iSlot + 1 << std::endl;
        for (short iChannel = 0; iChannel < 512; iChannel++) {
          TH1D* h_profile = h_hitTimeLaserHistos[iLowerEdge]->ProjectionY(("profile_" + std::to_string(iSlot + 1) + "_" + std::to_string(
                              iChannel) + "_"  + std::to_string(iLowerEdge)).c_str(),
                            iSlot * 512 + iChannel + 1, iSlot * 512 + iChannel + 1);
          if (m_fitterMode == "MC")
            h_profile->GetXaxis()->SetRangeUser(-10, -10);
          else
            h_profile->GetXaxis()->SetRangeUser(-65,
                                                -5); // if you will even change it, make sure not to include the h_hitTime overflow bins in this range
          fitChannel(iSlot, iChannel, h_profile);
          m_histoIntegral = h_profile->Integral();
          determineFitStatus();

          m_timewalkTree->Fill();
          h_profile->Write();
        }
      }
    }

    m_timewalkTree->Write();
  }

  m_histFile->Close();

  return c_OK;
}
