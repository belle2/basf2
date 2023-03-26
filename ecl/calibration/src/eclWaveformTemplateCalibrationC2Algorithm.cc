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
#include <ecl/digitization/OfflineFitFunction.h>
#include <ecl/dbobjects/ECLDigitWaveformParameters.h>

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

}


namespace {

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

}

CalibrationAlgorithm::EResult eclWaveformTemplateCalibrationC2Algorithm::calibrate()
{

  /** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();
  std::vector<double> cellIDArray;
  std::vector<double> maxResidualArray;

  TString fName = m_outputName;
  TFile* histfile = new TFile(fName, "recreate");

  TFile* f = new TFile(Form("PhotonShapes_Low%d_High%d.root", m_firstCellID, m_lastCellID), "RECREATE");
  TTree* mtree = new TTree("mtree", "");
  std::vector<double> PhotonWaveformArray(100000);
  mtree->Branch("PhotonArray", PhotonWaveformArray.data(), "PhotonWaveformArray[100000]/D");

  auto tree = getObjectPtr<TTree>("tree");
  int CellID;
  tree->SetBranchAddress("CellID", &CellID);

  std::vector<int> Waveform(31);
  std::vector<int> XValues(31);
  for (int i = 0; i < 31; i++) {
    tree->SetBranchAddress(Form("ADC%d", i), &Waveform[i]);
    XValues[i] = i;
  }

  int CollectorLimit = 6;
  double resLimit = 1.01;

  double ParamLimitFactor = 0.25;
  const int AttemptLimit = 10;
  int AttemptCounter = 0;

  ECLDigitWaveformParameters* PhotonParameters = new ECLDigitWaveformParameters();

  for (int cellid = m_firstCellID; cellid <= m_lastCellID; cellid++) {

    std::vector<int> EntriesToSkip;
    double maxResidual = 10.0;

    bool PASS = false;
    while (PASS == false) {

      std::vector<double> xValuesToFit;
      std::vector<double> yValuesToFit;

      std::vector<double> guessBaseline;
      std::vector<double> guessAmp;
      std::vector<double> guessTime;
      std::vector<int> NtupleEntries;

      int counter = 0;
      int counterWaveforms = 0;

      for (int i = 0; i < tree->GetEntries(); i++) {

        bool skipEvent = false;
        for (int k = 0; k < EntriesToSkip.size(); k++) {
          if (EntriesToSkip[k] == i) skipEvent = true;
        }
        if (skipEvent) continue;

        tree->GetEntry(i);

        if (CellID != cellid) continue;

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
        NtupleEntries.push_back(i);
        B2INFO("Entry: " << i);
        counterWaveforms++;
        if (counterWaveforms == CollectorLimit)  break;
      }

      B2INFO("CellID " << cellid << " counterWaveforms = " << counterWaveforms);

      auto gWaveformToFit = new TGraph(xValuesToFit.size(), xValuesToFit.data(), yValuesToFit.data());
      gWaveformToFit->SetName(Form("gWaveformToFit_%d", int(cellid)));

      //preparing fit function

      // ParMin11t defined below represents typical fit parameters for barrel and endcaps.  Note these are only used as the inital guess for the fit.
      double ParMin11t[11];
      if (cellid > 7776 || cellid < 1153) {
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

      FitFunctions.clear();
      for (int i = 0; i < counterWaveforms; i++) {

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

      TF1* TotalFitFunction = new TF1("TotalFitFunction", fitf, 0, counterWaveforms * 31, (3 * FitFunctions.size()) + 10);

      int FFsize = FitFunctions.size();
      for (int i = 0; i < FFsize; i++) {
        TotalFitFunction->SetParameter(i, guessTime[i]);
        TotalFitFunction->SetParameter(FFsize + i,  guessBaseline[i]);
        TotalFitFunction->SetParameter((2 * FFsize) + i, guessAmp[i]);
        for (int k = 0; k < 10; k++) {
          TotalFitFunction->SetParameter((3 * FFsize) + k, ParMin11t[k + 1]);
          if (ParamLimitFactor < 2) {
            TotalFitFunction->SetParLimits((3 * FFsize) + k, ParMin11t[k + 1]  - ParamLimitFactor * fabs(ParMin11t[k + 1]),
                                           ParMin11t[k + 1] + ParamLimitFactor * fabs(ParMin11t[k + 1]));
          } else {
            TotalFitFunction->ReleaseParameter((3 * FFsize) + k);
          }
        }
      }

      //performing the fit
      gWaveformToFit->Fit("TotalFitFunction", "Q M W N 0 R", "", 0, counterWaveforms  * 31);

      //next checking fit result by computing maximum value of Data/Fit
      std::vector<int> FitResultY;
      std::vector<int> FitResultX;
      double npts = xValuesToFit.size();

      int maxResidualWaveformID = 0; // Used to remove noisy waveforms
      maxResidual = 0.0;
      for (int k = 0; k < npts; k++) {
        double xVal = xValuesToFit[k];
        double yVal = TotalFitFunction->Eval(xVal);
        FitResultX.push_back(xVal);
        FitResultY.push_back(yVal);
        double diff = fabs(yValuesToFit[k] / yVal);
        if (diff > maxResidual) {
          maxResidual = diff;
          maxResidualWaveformID = (k / 31);
        }
      }

      // Checking if fit matches the data.
      if (maxResidual > resLimit) {

        B2INFO("FAIL: cellid " << cellid << " maxResidual " << maxResidual << " removing entry: " << NtupleEntries[maxResidualWaveformID] <<
               " which was waveform number " << maxResidualWaveformID << " resLimit was " << resLimit);

        EntriesToSkip.push_back(NtupleEntries[maxResidualWaveformID]);

        AttemptCounter++;

        if (counterWaveforms < 3)  AttemptCounter = AttemptLimit;

        // If fit is not successful after several attempts, parameter limits are increased.
        if (AttemptCounter == AttemptLimit) {

          ParamLimitFactor += 0.5;

          B2INFO("AttemptCounter reach limit: " << AttemptCounter << " counterWaveforms: " << counterWaveforms);
          B2INFO("Increasing ParamLimitFactor to " << ParamLimitFactor);

          // reseting for next round of fits with larger parameter limits
          EntriesToSkip.clear();
          AttemptCounter = 0;

        }

        // If fit is still not successful after several incrases to the parameter limits then the resLimit is relaxed.
        if (AttemptCounter == AttemptLimit) {
          if (ParamLimitFactor > 2.1) {
            resLimit += 0.001;
            B2INFO("Increasing resLimit to " << resLimit);
            ParamLimitFactor = 0.25;
          }
        }

      } else {

        B2INFO("PASS: cellid " << cellid << " maxResidual " << maxResidual << " number of waveforms used was " << counterWaveforms <<
               " resLimit was " << resLimit);

        PASS = true;
        AttemptCounter = 0;
        ParamLimitFactor = 0.25;
        resLimit = 1.01;

        auto gFitResult = new TGraph(FitResultX.size(), FitResultX.data(), FitResultY.data());
        gFitResult->SetName(Form("gFitResult_%d", int(cellid)));

        cellIDArray.push_back(cellid);
        maxResidualArray.push_back(maxResidual);

        histfile->cd();
        gWaveformToFit->Write();
        gFitResult->Write();

        float tempPhotonPar11[11];
        tempPhotonPar11[0] = ParMin11t[0];
        for (unsigned int k = 0; k < 10; k++) {
          tempPhotonPar11[k + 1] = TotalFitFunction->GetParameter((3 * FFsize) + k);
        }

        //Defining a single normalized function with final parameters
        FitFunctions[0]->SetParameter(0, 0);
        FitFunctions[0]->SetParameter(1, 0);
        FitFunctions[0]->SetParameter(2, 1);
        for (int k = 0; k < 10; k++) {
          FitFunctions[0]->SetParameter(4 + k, tempPhotonPar11[k + 1]);
          FitFunctions[0]->SetParameter(10 + 4 + k, tempPhotonPar11[k + 1]);
        }
        FitFunctions[0]->FixParameter(24, ParMin11t[0]);
        FitFunctions[0]->FixParameter(25, 1);

        //Computing normalization parameter
        double MaxVal = -1.0;
        const double cnpts = 2000;
        for (int k = 0; k < cnpts; k++) {
          double xVal = (k * 31.0 / cnpts);
          double yVal = FitFunctions[0]->Eval(xVal);
          if (yVal > MaxVal) MaxVal = yVal;
        }
        B2INFO("MaxVal " << MaxVal);
        tempPhotonPar11[0] /= MaxVal;
        FitFunctions[0]->FixParameter(24, tempPhotonPar11[0]);

        PhotonParameters->setTemplateParameters(cellid, tempPhotonPar11, tempPhotonPar11, tempPhotonPar11);

        for (unsigned int k = 0; k < PhotonWaveformArray.size();
             k++) PhotonWaveformArray[k] = FitFunctions[0]->Eval(((double)k) * (1. / 1000.)) ;
        mtree->Fill();

        TotalFitFunction->Delete() ;
      }
    }
  }
  histfile->cd();
  auto gmaxResidual = new TGraph(cellIDArray.size(), cellIDArray.data(), maxResidualArray.data());
  gmaxResidual->SetName("gmaxResidual");
  gmaxResidual->Write();
  f->cd();
  mtree->Write();

  saveCalibration(PhotonParameters, Form("PhotonParameters_CellID%d_CellID%d", m_firstCellID, m_lastCellID));
  B2INFO("eclWaveformTemplateCalibrationC2Algorithm: successfully stored " << Form("PhotonParameters_CellID%d_CellID%d",
         m_firstCellID, m_lastCellID) << " constants");

  return c_OK;
}
