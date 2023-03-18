/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/calibration/eclWaveformTemplateCalibrationC2Algorithm.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/digitization/OfflineFitFunction.h>

/* ROOT headers. */
#include <TFile.h>
#include <TGraph.h>
#include <TTree.h>
#include <TF1.h>

#include <iostream>

using namespace std;
using namespace Belle2;
using namespace ECL;

/**-----------------------------------------------------------------------------------------------*/
eclWaveformTemplateCalibrationC2Algorithm::eclWaveformTemplateCalibrationC2Algorithm():
  CalibrationAlgorithm("eclWaveformTemplateCalibrationC2Collector")
{
  setDescription(
    "Perform energy calibration of ecl crystals by fitting a Novosibirsk function to energy deposited by photons in e+e- --> gamma gamma"
  );

  m_lowestEnergyFraction = 0.1;
}

std::vector<TF1*> FitFunctions;

double fitf(double* x, double* par)
{

  double xtoeval = std::fmod(x[0], 31);
  int whichFitFunctions = x[0] / 31;

  for (int i = 0; i < FitFunctions.size(); i++) {
    FitFunctions[i]->SetParameter(0, par[i]);
    FitFunctions[i]->SetParameter(1, par[FitFunctions.size() + i]);
    FitFunctions[i]->SetParameter(2, par[(2 * FitFunctions.size()) + i]);
    FitFunctions[i]->FixParameter(3, 0);
    for (int k = 0; k < 10; k++) {
      FitFunctions[i]->SetParameter(4 + k, par[(3 * FitFunctions.size()) + k]);
    }
  }

  return FitFunctions[whichFitFunctions]->Eval(xtoeval * 0.5);
}

CalibrationAlgorithm::EResult eclWaveformTemplateCalibrationC2Algorithm::calibrate()
{

  /** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();

  /**-----------------------------------------------------------------------------------------------*/
  /** Histograms containing the data collected by eclGammaGammaECollectorModule */
  auto tree = getObjectPtr<TTree>("tree");
  int CellID;
  tree->SetBranchAddress("CellID", &CellID);

  std::vector<int> Waveform(31);
  std::vector<int> XValues(31);
  for (int i = 0; i < 31; i++) {
    tree->SetBranchAddress(Form("ADC%d", i), &Waveform[i]);
    XValues[i] = i;
  }

  int CollectorLimit = 20;

  std::vector<double> xValuesToFit;
  std::vector<double> yValuesToFit;

  std::vector<double> guessBaseline;
  std::vector<double> guessAmp;
  std::vector<double> guessTime;

  int id = 4000;

  int counter = 0;
  int counterWaveforms = 0;
  for (int i = 0; i < tree->GetEntries(); i++) {
    tree->GetEntry(i);

    if (CellID != id) continue;

    std::cout << CellID << std::endl;

    double maxval = 0;
    double maxIndex = 0;
    for (int j = 0; j < 31; j++) {
      xValuesToFit.push_back(counter);
      yValuesToFit.push_back(Waveform[j]);
      if (Waveform[j] > maxval) {
        maxval = Waveform[j];
        maxIndex = j;
      }
      counter++;
    }
    guessBaseline.push_back(Waveform[0]);
    guessAmp.push_back(maxval);
    guessTime.push_back((maxIndex - 4.5) * 0.5);
    counterWaveforms++;
    if (counterWaveforms == CollectorLimit)  break;
  }

  B2INFO("counterWaveforms = " << counterWaveforms);
  CollectorLimit = counterWaveforms;

  auto gWaveformToFit = new TGraph(xValuesToFit.size(), xValuesToFit.data(), yValuesToFit.data());
  gWaveformToFit->SetName("gWaveformToFit");

  //preparing fit function
  double ParMin11t[11];

  if (CellID > 7776 || CellID < 1153) {
    ParMin11t[0] = 20.3216;
    ParMin11t[1] = -0.0206266;
    ParMin11t[2] = 0.313928;
    ParMin11t[3] = 0.589646;
    ParMin11t[4] = 0.455526;
    ParMin11t[5] = 1.03656;
    ParMin11t[6] = 0.000822467;
    ParMin11t[7] = 45.1574;
    ParMin11t[8] = 0.716034;
    ParMin11t[9] = 0.616753;
    ParMin11t[10] = 0.0851222;
  } else {
    ParMin11t[0] = 24.6176;
    ParMin11t[1] = 0.00725002;
    ParMin11t[2] = 0.601578;
    ParMin11t[3] = 0.491976;
    ParMin11t[4] = 0.601034;
    ParMin11t[5] = 0.601684;
    ParMin11t[6] = -0.0103788;
    ParMin11t[7] = 2.22615;
    ParMin11t[8] = 0.671294;
    ParMin11t[9] = 0.529878;
    ParMin11t[10] = 0.0757927;
  }

  for (int i = 0; i < CollectorLimit; i++) {

    FitFunctions.push_back(new TF1(Form("Shp_%d", i), Belle2::ECL::WaveFuncTwoComponent, 0, 30.5, 26));
    FitFunctions[i]->SetNpx(10000);
    FitFunctions[i]->FixParameter(3, 0);
    for (int k = 0; k < 10; k++) {
      FitFunctions[i]->SetParameter(4 + k, ParMin11t[k + 1]);
      FitFunctions[i]->FixParameter(10 + 4 + k, ParMin11t[k + 1]);
    }
    FitFunctions[i]->FixParameter(24, ParMin11t[0]);
    FitFunctions[i]->FixParameter(25, 1);
  }

  TF1* TotalFitFunction = new TF1("TotalFitFunction", fitf, 0, CollectorLimit * 31, (3 * FitFunctions.size()) + 10);

  int FFsize = FitFunctions.size();
  for (int i = 0; i < FFsize; i++) {
    TotalFitFunction->SetParameter(i, guessTime[i]);
    TotalFitFunction->SetParameter(FFsize + i,  guessBaseline[i]);
    TotalFitFunction->SetParameter((2 * FFsize) + i, guessAmp[i]);
    for (int k = 0; k < 10; k++) {
      TotalFitFunction->SetParameter((3 * FFsize) + k, ParMin11t[k + 1]);
      TotalFitFunction->SetParLimits((3 * FFsize) + k, ParMin11t[k + 1]  - 0.25 * fabs(ParMin11t[k + 1]),
                                     ParMin11t[k + 1] + 0.25 * fabs(ParMin11t[k + 1]));
    }
  }

  gWaveformToFit->Fit("TotalFitFunction", " N 0 R", "", 0, CollectorLimit * 31);

  std::vector<int> FitResultY;
  std::vector<int> FitResultX;
  double npts = xValuesToFit.size();
  for (int k = 0; k < npts; k++) {
    double xVal = xValuesToFit[k];//(CollectorLimit * k * 31.0 / npts);
    double yVal = TotalFitFunction->Eval(xVal);
    FitResultX.push_back(xVal);
    FitResultY.push_back(yVal);
  }
  auto gFitResult = new TGraph(FitResultX.size(), FitResultX.data(), FitResultY.data());
  gFitResult->SetName("gFitResult");

  ///** Write out the basic histograms in all cases */
  TString fName = m_outputName;
  TFile* histfile = new TFile(fName, "recreate");
  gWaveformToFit->Write();
  gFitResult->Write();

  for (int i = 0; i < 10; i++)std::cout << i << " " << TotalFitFunction->GetParameter((3 * FFsize) + i) << std::endl;

  //ECLCrystalCalib* PPThreshold = new ECLCrystalCalib();
  //PPThreshold->setCalibVector(Varxs, cryIDs);
  //saveCalibration(PPThreshold, "eclWaveformTemplateCalibrationC2VarSq");
  //B2INFO("eclWaveformTemplateCalibrationC2Algorithm: successfully stored ECLAutocovarianceCalibrationC2Threshold constants");

  return c_OK;
}
