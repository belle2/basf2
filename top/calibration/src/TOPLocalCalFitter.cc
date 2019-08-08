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


TOPLocalCalFitter::TOPLocalCalFitter(): CalibrationAlgorithm("TOPLaserCalibratorCollector"),  minEntries(50)
{
  setDescription(
    "Perform the fit of the laser and pulser runs"
  );
}



void  TOPLocalCalFitter::loadMCInfoTrees()
{
  std::cout << "Entering loadMCInfoTree" << std::endl;
  std::cout  << "Getting the  TTS parameters from " << m_TTSData << std::endl;
  TFile inputTTS(m_TTSData.c_str());
  inputTTS.cd();
  inputTTS.GetObject("tree", treeTTS);
  treeTTS->SetBranchAddress("mean2", &mean2);
  treeTTS->SetBranchAddress("sigma1", &sigma1);
  treeTTS->SetBranchAddress("sigma2", &sigma2);
  treeTTS->SetBranchAddress("fraction1", &f1);
  treeTTS->SetBranchAddress("fraction2", &f2);
  treeTTS->SetBranchAddress("pixelRow", &pixelRow);
  treeTTS->SetBranchAddress("pixelCol", &pixelCol);

  std::cout << "Getting the laser fit parameters from " << m_laserCorrections << std::endl;
  TFile inputLaser(m_laserCorrections.c_str());
  inputLaser.cd();
  inputLaser.GetObject("fitTree", treeLaser);
  treeLaser->SetBranchAddress("peakTime", &peakTimeLaser);
  treeLaser->SetBranchAddress("deltaT", &deltaTLaser);
  treeLaser->SetBranchAddress("fraction", &fractionLaser);

  std::cout << "Exiting loadMCInfoTree" << std::endl;

  treeLaser->Show(1);
  treeTTS->Show(1);

  return;
}


void  TOPLocalCalFitter::setupOutputTreeAndFile()
{
  std::cout << "Entering setupOutputTreeAndFile: Setting up output tree" << std::endl;
  histFile = new TFile(m_output.c_str(), "recreate");
  histFile->cd();
  fitTree = new TTree("fitTree", "fitTree");
  fitTree->Branch<short>("channel", &channel);
  fitTree->Branch<short>("slot", &slot);
  fitTree->Branch<short>("row", &row);
  fitTree->Branch<short>("col", &col);
  fitTree->Branch<float>("peakTime", &peakTime);
  fitTree->Branch<float>("peakTimeErr", &peakTimeErr);
  fitTree->Branch<float>("deltaT", &deltaT);
  fitTree->Branch<float>("deltaTErr", &deltaTErr);
  fitTree->Branch<float>("sigma", &sigma);
  fitTree->Branch<float>("sigmaErr", &sigmaErr);
  fitTree->Branch<float>("fraction", &fraction);
  fitTree->Branch<float>("fractionErr", &fractionErr);
  fitTree->Branch<float>("yieldLaser", &yieldLaser);
  fitTree->Branch<float>("yieldLaserErr", &yieldLaserErr);
  fitTree->Branch<float>("timeExtra", &timeExtra);
  fitTree->Branch<float>("sigmaExtra", &sigmaExtra);
  fitTree->Branch<float>("yieldLaserExtra", &yieldLaserExtra);
  fitTree->Branch<float>("fractionMC", &fractionMC);
  fitTree->Branch<float>("deltaTMC", &deltaTMC);
  fitTree->Branch<float>("peakTimeMC", &peakTimeMC);
  fitTree->Branch<float>("firstPulserTime", &firstPulserTime);
  fitTree->Branch<float>("firstPulserSigma", &firstPulserSigma);
  fitTree->Branch<float>("secondPulserTime", &secondPulserTime);
  fitTree->Branch<float>("secondPulserSigma", &secondPulserSigma);
  fitTree->Branch<float>("channelT0", &channelT0);
  fitTree->Branch<float>("channelT0Err", &channelT0Err);
  fitTree->Branch<short>("fitStatus", &fitStatus);

  fitTree->Branch<float>("chi2", &chi2);
  fitTree->Branch<float>("rms", &rms);
  std::cout << "Exiting setupOutputTreeAndFile: tree done" << std::endl;
  return;
}




void  TOPLocalCalFitter::fitChannel(short iSlot, short iChannel, TH1* h_profile)
{
  std::cout << "Entering fitChannel" << std::endl;
  treeLaser->GetEntry(iChannel);
  treeTTS->GetEntry(iChannel + 512 * iSlot);
  treeLaser->Show(iChannel);
  treeTTS->Show(iChannel + 512 * iSlot);

  std::cout << "MC info loaded" << std::endl;

  double maxpos = h_profile->GetBinCenter(h_profile->GetMaximumBin());
  h_profile->GetXaxis()->SetRangeUser(maxpos - 1, maxpos + 2.);
  double integral = h_profile->Integral();
  std::cout << "histo OK" << std::endl;


  TF1 laser = TF1("laser", laserPDF, maxpos - 1, maxpos + 2., 16);

  laser.SetParameter(0, maxpos);
  laser.SetParLimits(0, maxpos - 0.06, maxpos + 0.06);

  laser.SetParameter(1, 0.1);
  laser.SetParLimits(1, 0.05, 0.25);
  laser.SetParameter(2, fractionLaser);
  laser.SetParLimits(2, 0.5, 1.);


  laser.FixParameter(3, deltaTLaser);

  if (deltaTLaser > -0.001) {
    laser.SetParameter(3, -0.3);
    laser.SetParLimits(3, -0.4, -0.2);
  }

  laser.FixParameter(4, TMath::Sqrt(sigma2 * sigma2 - sigma1 * sigma1));
  laser.FixParameter(5, mean2);

  laser.FixParameter(6, f1);

  laser.SetParameter(7,  integral * 0.005);
  laser.SetParLimits(7,  0.2 * integral * 0.005, 2.*integral * 0.005);

  laser.SetParameter(8, 1.);
  laser.SetParLimits(8, 0.3, 2.);
  laser.SetParameter(9, 0.2);
  laser.SetParLimits(9, 0.08, 1.);

  laser.SetParameter(10,  0.1 * integral * 0.005);
  laser.SetParLimits(10,  0., 0.2 * integral * 0.005);
  laser.SetParameter(14, -2.);
  laser.SetParameter(15, 2.);
  laser.SetParLimits(15, 1.01, 20.);

  laser.SetParameter(11, 1.);
  laser.SetParLimits(11, 0.1, 5.);
  laser.SetParameter(12, 0.8);
  laser.SetParLimits(12, 0., 5.);
  laser.SetParameter(13,  0.01 * integral * 0.005);
  laser.SetParLimits(13,  0., 0.2 * integral * 0.005);

  if (isMonitoringFit) {
    laser.FixParameter(2, fractionLaser);
    laser.FixParameter(3, deltaTLaser);
  }

  laser.SetNpx(2000);

  h_profile->Fit("laser", "R L Q");

  // Add by hand the different components, mostly for debugging/presentation purposes
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


  channel = iChannel;
  row = pixelRow;
  col = pixelCol;
  slot = iSlot + 1;
  peakTime = laser.GetParameter(0);
  peakTimeErr = laser.GetParError(0);
  deltaT = laser.GetParameter(3);
  deltaTErr = laser.GetParError(3);
  sigma = laser.GetParameter(1);
  sigmaErr = laser.GetParError(1);
  fraction = laser.GetParameter(2);
  fractionErr = laser.GetParError(2);
  yieldLaser = laser.GetParameter(7) / 0.005;
  yieldLaserErr = laser.GetParError(7) / 0.005;
  timeExtra = laser.GetParameter(8);
  sigmaExtra = laser.GetParameter(9);
  yieldLaserExtra = laser.GetParameter(10) / 0.005;

  fractionMC = fractionLaser;
  deltaTMC = deltaTLaser;
  peakTimeMC = peakTimeLaser;

  chi2 = laser.GetChisquare() / laser.GetNDF();


  channelT0 = peakTime - peakTimeMC;
  channelT0Err = peakTimeErr;
  std::cout << "Exiting fitChannel" << std::endl;
  return;
}


void  TOPLocalCalFitter::fitPulser(TH1* h_profileFirstPulser, TH1* h_profileSecondPulser)
{
  std::cout << "Entering fitPulser" << std::endl;

  float maxpos = h_profileFirstPulser->GetBinCenter(h_profileFirstPulser->GetMaximumBin());
  h_profileFirstPulser->GetXaxis()->SetRangeUser(maxpos - 1, maxpos + 1.);
  if (h_profileFirstPulser->Integral() > 1000) {
    TF1 pulser1 = TF1("pulser1", "[0]*TMath::Gaus(x, [1], [2], kTRUE)", maxpos - 1, maxpos + 1.);
    pulser1.SetParameter(0, 1.);
    pulser1.SetParameter(1, maxpos);
    pulser1.SetParameter(2, 0.05);
    h_profileFirstPulser->Fit("pulser1", "R Q");
    firstPulserTime = pulser1.GetParameter(1);
    firstPulserSigma = pulser1.GetParameter(2);
    h_profileFirstPulser->Write();
  } else {
    firstPulserTime = -999;
    firstPulserSigma = -999;
  }

  maxpos = h_profileSecondPulser->GetBinCenter(h_profileSecondPulser->GetMaximumBin());
  h_profileSecondPulser->GetXaxis()->SetRangeUser(maxpos - 1, maxpos + 1.);
  if (h_profileSecondPulser->Integral() > 1000) {
    TF1 pulser2 = TF1("pulser2", "[0]*TMath::Gaus(x, [1], [2], kTRUE)", maxpos - 1, maxpos + 1.);
    pulser2.SetParameter(0, 1.);
    pulser2.SetParameter(1, maxpos);
    pulser2.SetParameter(2, 0.05);
    h_profileSecondPulser->Fit("pulser2", "R Q");
    secondPulserTime = pulser2.GetParameter(1);
    secondPulserSigma = pulser2.GetParameter(2);
    h_profileSecondPulser->Write();
  } else {
    secondPulserTime = -999;
    secondPulserSigma = -999;
  }
  std::cout << "Exiting fitPulser" << std::endl;

  return;
}



void  TOPLocalCalFitter::determineFitStatus()
{
  if (chi2 < 4 && sigma < 0.2 && yieldLaser > 1000) {
    fitStatus = 0;
  } else {
    fitStatus = 1;
  }
  return;
}





CalibrationAlgorithm::EResult TOPLocalCalFitter::calibrate()
{

  std::cout << "Calibrating " << std::endl;

  gROOT->SetBatch();

  loadMCInfoTrees();

  setupOutputTreeAndFile();


  std::cout << "Loading hittree" << std::endl;

  // Loads the tree with the hits
  auto hitTree = getObjectPtr<TTree>("hitTree");
  TH2F* h_hitTime = new TH2F("h_hitTime", " ", 512 * 16, 0., 512 * 16, 20000, -60, 40.); // 10 ps bins
  std::cout << "Filling the time VS channel histogram" << std::endl;
  hitTree->Draw("hitTime:(channel+(slot-1)*512)>>h_hitTime", "dVdt > 80 && amplitude > 80");

  histFile->cd();

  for (short iSlot = 0; iSlot < 16; iSlot++) {
    std::cout << "Fitting slot " << iSlot + 1 << std::endl;
    for (short iChannel = 0; iChannel < 512; iChannel++) {
      TH1D* h_profile = h_hitTime->ProjectionY(("profile_" + std::to_string(iSlot + 1) + "_" + std::to_string(iChannel)).c_str(),
                                               iSlot * 512 + iChannel + 1, iSlot * 512 + iChannel + 1);
      h_profile->GetXaxis()->SetRangeUser(-100, -5);

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


      fitTree->Fill();

      h_profile->Write();
      h_profileFirstPulser->Write();
      h_profileSecondPulser->Write();
    }

    h_hitTime->Write();
  }

  fitTree->Write();
  histFile->Close();
  return c_OK;
}
