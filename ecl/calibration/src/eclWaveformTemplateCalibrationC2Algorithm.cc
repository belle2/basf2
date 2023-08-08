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
#include <ecl/dbobjects/ECLCrystalCalib.h>

/* ROOT headers. */
#include <TFile.h>
#include <TGraph.h>
#include <TTree.h>
#include <TF1.h>

#include<ctime>

using namespace std;
using namespace Belle2;
using namespace ECL;
using namespace Calibration;

/**-----------------------------------------------------------------------------------------------*/
eclWaveformTemplateCalibrationC2Algorithm::eclWaveformTemplateCalibrationC2Algorithm():
  CalibrationAlgorithm("eclWaveformTemplateCalibrationC2Collector")
{
  setDescription(
    "Perform the photon template shape calibration using waveforms from high energy crystals from e+e- --> gamma gamma events"
  );

}


namespace {

  // Used to perform simultaneous fits of multiple waveforms
  std::vector<TF1*> FitFunctions;
  const double numberofADCPoints = 31.0;

  double fitf(double* x, double* par)
  {

    double xtoeval = std::fmod(x[0], numberofADCPoints);
    int whichFitFunctions = x[0] / numberofADCPoints;

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

  B2INFO("Reading ECLCrystalCalib payload: eclWaveformTemplateCalibrationC1MaxResLimit");
  DBObjPtr<ECLCrystalCalib> existingeclWaveformTemplateCalibrationC1MaxResLimit("eclWaveformTemplateCalibrationC1MaxResLimit");
  auto runs = getRunList();
  ExpRun chosenRun = runs.front();
  // After here your DBObjPtrs are correct
  updateDBObjPtrs(1, chosenRun.second, chosenRun.first);

  /** Put root into batch mode so that we don't try to open a graphics window */
  gROOT->SetBatch();

  std::vector<double> cellIDArray;
  std::vector<double> maxResidualArray;  // used to quantify fit result
  std::vector<double> limitResidualArray;  // what was final resLimit used
  std::vector<double> parLimitFactorArray;  // what was final parLimitFactorArray used

  /** File to save waveform information, mainly for debugging */
  TFile* histfile = new TFile(m_outputName.c_str(), "recreate");

  /** File to save arrays of photon templates, once determined. Needed in for C3 algorithm to follow */
  TFile* f_PhotonTemplateOutput = new TFile(Form("PhotonShapes_Low%d_High%d.root", m_firstCellID, m_lastCellID), "RECREATE");
  TTree* mtree = new TTree("mtree", "");
  std::vector<double> PhotonWaveformArray(100000);
  mtree->Branch("PhotonArray", PhotonWaveformArray.data(), "PhotonWaveformArray[100000]/D");

  /** Initalizing tree to read collector information  */
  auto tree = getObjectPtr<TTree>("tree");
  int CellID;
  tree->SetBranchAddress("CellID", &CellID);
  std::vector<int> Waveform(m_NumberofADCPoints);
  std::vector<int> XValues(m_NumberofADCPoints);
  for (int i = 0; i < m_NumberofADCPoints; i++) {
    tree->SetBranchAddress(Form("ADC%d", i), &Waveform[i]);
    XValues[i] = i;
  }

  std::time_t t = std::time(0);

  /** Count number of fit attempts */
  int AttemptCounter = 0;

  /** DBobject to store photon shape parameters computed by this algorithm */
  ECLDigitWaveformParameters* PhotonParameters = new ECLDigitWaveformParameters();

  /** ParMin11t defined below represents typical fit parameters for barrel and endcaps.
      Note these are only used as the inital guess for the fit.*/
  double ParMin11t[11];

  /** Computing photon templates for CellID range specified (recommend batches of 100) */
  for (int cellid = m_firstCellID; cellid <= m_lastCellID; cellid++) {

    /** Typical parameters used for inital guess in fit. Note endcaps and barrel have different shapes */
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

    double resLimit = 2 * existingeclWaveformTemplateCalibrationC1MaxResLimit->getCalibVector()[cellid -
                             1]; /** Maximum Data/Fit value to classify fit as successful */
    double resLimitOriginal = resLimit;

    /** Waveforms from collector to skip (likely to have pile-up noise outside baseline region) */
    std::vector<int> EntriesToSkip;

    /** Initializing max data/Fit value.  Must reach below resLimit for fit to be successful */
    double maxResidual = 1000.0;

    /** While PASS == false, resLimit has not been reached for the particular cellID*/
    bool PASS = false;
    while (PASS == false) {

      /** Array of multiple waveforms to  simultaneously fit */
      std::vector<double> xValuesToFit;
      std::vector<double> yValuesToFit;

      /** Initial guesses for variables unique to each waveform (unlike template parameters) */
      std::vector<double> guessBaseline;
      std::vector<double> guessAmp;
      std::vector<double> guessTime;

      /** Saving how many waveforms passed collector cuts for each crystal */
      std::vector<int> NtupleEntries;

      int counter = 0; // counts entry number in xValuesToFit
      int counterWaveforms = 0; // counts numver of waveforms selected

      for (int i = 0; i < tree->GetEntries(); i++) {

        /** skiping waveforms identified later in the calibration to potentially have pile-up noise outside baseline */
        bool skipEvent = false;
        for (int k = 0; k < EntriesToSkip.size(); k++) {
          if (EntriesToSkip[k] == i) skipEvent = true;
        }
        if (skipEvent) continue;

        tree->GetEntry(i);

        if (CellID != cellid) continue;

        double maxval = 0;
        double maxIndex = 0;
        for (int j = 0; j < m_NumberofADCPoints; j++) {
          xValuesToFit.push_back(counter);
          yValuesToFit.push_back(Waveform[j]);
          if (Waveform[j] > maxval) {
            maxval = Waveform[j];
            maxIndex = j;
          }
          counter++;
        }

        /** guess arrays have one enrgy per waveform */
        guessBaseline.push_back(Waveform[0]);
        guessAmp.push_back(maxval);
        guessTime.push_back((maxIndex - 4.5) * 0.5);  /** rough estimate where t0 should occur */

        NtupleEntries.push_back(i);
        B2INFO("Entry: " << i);

        counterWaveforms++;

        if (counterWaveforms == m_CollectorLimit)  break;

      }

      /** waveforms to fit have been extracted from ttree  */
      B2INFO("CellID " << cellid << " counterWaveforms = " << counterWaveforms);

      if (counterWaveforms < m_TotalCountsThreshold) {
        B2INFO("eclWaveformTemplateCalibrationC2Algorithm: warning total entries for cell ID " <<  cellid << " is only: " <<
               counterWaveforms <<
               " Requirement is : " << m_TotalCountsThreshold);
        /** We require all crystals to have a minimum number of waveforms available.  If c_NotEnoughData is returned then the next run will be appended.  */
        return c_NotEnoughData;
      }


      /** TGraph to fit */
      auto gWaveformToFit = new TGraph(xValuesToFit.size(), xValuesToFit.data(), yValuesToFit.data());
      gWaveformToFit->SetName(Form("gWaveformToFit_%d", int(cellid)));

      /** preparing the fit function */

      /** Fit function is an array of photon templates */
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

      /** fitf defined in namespace above calls FitFunctions[i] */
      TF1* TotalFitFunction = new TF1("TotalFitFunction", fitf, 0, counterWaveforms * m_NumberofADCPoints,
                                      (3 * FitFunctions.size()) + 10);

      /** Initializing parameters for TotalFitFunction */
      int FFsize = FitFunctions.size();
      for (int i = 0; i < FFsize; i++) {
        TotalFitFunction->SetParameter(i, guessTime[i]);
        TotalFitFunction->SetParameter(FFsize + i,  guessBaseline[i]);
        TotalFitFunction->SetParameter((2 * FFsize) + i, guessAmp[i]);
        for (int k = 0; k < 10; k++) {
          TotalFitFunction->SetParameter((3 * FFsize) + k, ParMin11t[k + 1]);
          if (m_ParamLimitFactor < 2) {
            TotalFitFunction->SetParLimits((3 * FFsize) + k, ParMin11t[k + 1]  - m_ParamLimitFactor * fabs(ParMin11t[k + 1]),
                                           ParMin11t[k + 1] + m_ParamLimitFactor * fabs(ParMin11t[k + 1]));
          } else {
            TotalFitFunction->ReleaseParameter((3 * FFsize) + k);
          }
        }
      }

      /** Performing the simultaneous fit */
      gWaveformToFit->Fit("TotalFitFunction", "Q M W N 0 R", "", 0, counterWaveforms  * m_NumberofADCPoints);

      /** next checking fit result by computing maximum value of Data/Fit */
      std::vector<int> FitResultY;
      std::vector<int> FitResultX;
      int maxResidualWaveformID = 0; // Used to remove waveforms with potential pile-up outside baseline

      /** computing maximum value of Data/Fit */
      maxResidual = 0.0;
      double npts = xValuesToFit.size();
      double maxResidualOld = 0.0;
      for (int k = 0; k < npts; k++) {
        double xVal = xValuesToFit[k];
        double yVal = TotalFitFunction->Eval(xVal);
        FitResultX.push_back(xVal);
        FitResultY.push_back(yVal);
        double diff = fabs(yValuesToFit[k] - yVal);
        if (diff > maxResidual) {
          maxResidual = diff;
          maxResidualWaveformID = (k / m_NumberofADCPoints);
          maxResidualOld = fabs(yValuesToFit[k] / yVal);
        }
      }

      // Checking if fit matches the data.
      if (maxResidual > resLimit) {

        B2INFO("FAIL: cellid " << cellid << " maxResidual " << maxResidual << " removing entry: " << NtupleEntries[maxResidualWaveformID] <<
               " which was waveform number " << maxResidualWaveformID << " resLimit was " << resLimit << " , resLimit started at " <<
               resLimitOriginal);
        B2INFO("Old maxResidual of Data/Fit was " << maxResidualOld);

        B2INFO("Iter Time = " << std::time(0) - t << std::endl);
        t = std::time(0);

        std::cout << "FAIL: cellid " << cellid << " maxResidual " << maxResidual << " removing entry: " <<
                  NtupleEntries[maxResidualWaveformID] <<
                  " which was waveform number " << maxResidualWaveformID << " resLimit was " << resLimit << " , resLimit started at " <<
                  resLimitOriginal << std::endl;
        std::cout << "wave = [";
        for (int k = 0; k < npts; k++) {
          std::cout << yValuesToFit[k];
          if (k < (npts - 1)) {
            std::cout << ",";
          } else {
            std::cout << "]" << std::endl;
          }
        }
        std::cout << "fitRes = [";
        for (int k = 0; k < npts; k++) {
          std::cout << TotalFitFunction->Eval(xValuesToFit[k]);
          if (k < (npts - 1)) {
            std::cout << ",";
          } else {
            std::cout << "]" << std::endl;
          }
        }

        /** In next attempt skip waveform containing largest Data/Fit value */
        EntriesToSkip.push_back(NtupleEntries[maxResidualWaveformID]);

        AttemptCounter++;

        /** Ensure at least 3 waveforms used in simultaneous fit  */
        if (counterWaveforms < m_SimutaniousFitLimit)  AttemptCounter = m_AttemptLimit;

        /** If fit is not successful after several attempts, parameter limits are increased.  */
        if (AttemptCounter == m_AttemptLimit) {

          m_ParamLimitFactor += m_ParLimitFactorIterator;

          B2INFO("AttemptCounter reach limit: " << AttemptCounter << " counterWaveforms: " << counterWaveforms);
          B2INFO("Increasing m_ParamLimitFactor to " << m_ParamLimitFactor);

          /** reseting for next round of fits with larger parameter limits  */
          EntriesToSkip.clear();
          AttemptCounter = 0;

          /** If fit is still not successful after several increases to the parameter limits, then resLimit is relaxed.  */
          if (m_ParamLimitFactor > m_ParLimitFactorLimit) {
            resLimit *= m_ResLimitIterator;
            B2INFO("Increasing resLimit to " << resLimit);
            m_ParamLimitFactor = m_BaseParamLimitFactor;
          }
        }

      } else {

        B2INFO("PASS: cellid " << cellid << " maxResidual " << maxResidual << " number of waveforms used was " << counterWaveforms <<
               " resLimit was " << resLimit);

        PASS = true;

        limitResidualArray.push_back(resLimit);
        parLimitFactorArray.push_back(m_ParamLimitFactor);

        /** reseting for next crystal  */
        AttemptCounter = 0;
        m_ParamLimitFactor = m_BaseParamLimitFactor;

        auto gFitResult = new TGraph(FitResultX.size(), FitResultX.data(), FitResultY.data());
        gFitResult->SetName(Form("gFitResult_%d", int(cellid)));

        /** Saving value of maximum data/Fit */
        cellIDArray.push_back(cellid);
        maxResidualArray.push_back(maxResidual);

        /** Saving fit result */
        histfile->cd();
        gWaveformToFit->Write();
        gFitResult->Write();

        /** Extracting result for photon template shape parameters */
        float tempPhotonPar11[11];
        tempPhotonPar11[0] = ParMin11t[0];
        for (unsigned int k = 0; k < 10; k++) {
          tempPhotonPar11[k + 1] = TotalFitFunction->GetParameter((3 * FFsize) + k);

          /** Use result as starting point for next crystal  */
          //ParMin11t[k+1] = tempPhotonPar11[k + 1];
        }

        /** Defining a single normalized function with final parameters */
        FitFunctions[0]->SetParameter(0, 0);
        FitFunctions[0]->SetParameter(1, 0);
        FitFunctions[0]->SetParameter(2, 1);
        for (int k = 0; k < 10; k++) {
          FitFunctions[0]->SetParameter(4 + k, tempPhotonPar11[k + 1]);
          FitFunctions[0]->SetParameter(10 + 4 + k, tempPhotonPar11[k + 1]);
        }
        FitFunctions[0]->FixParameter(24, ParMin11t[0]);
        FitFunctions[0]->FixParameter(25, 1);

        /** Computing normalization parameter */
        double MaxVal = -1.0;
        const double cnpts = 2000;
        for (int k = 0; k < cnpts; k++) {
          double xVal = (k * double(m_NumberofADCPoints) / cnpts);
          double yVal = FitFunctions[0]->Eval(xVal);
          if (yVal > MaxVal) MaxVal = yVal;
        }
        B2INFO("MaxVal " << MaxVal);
        tempPhotonPar11[0] /= MaxVal;
        FitFunctions[0]->FixParameter(24, tempPhotonPar11[0]);

        /** Saving result for photon template shape parameters to db object*/
        PhotonParameters->setTemplateParameters(cellid, tempPhotonPar11, tempPhotonPar11, tempPhotonPar11);

        /** Saving photon shape to tree to be used in next stage of calibration (C3) */
        for (unsigned int k = 0; k < PhotonWaveformArray.size();
             k++) PhotonWaveformArray[k] = FitFunctions[0]->Eval(((double)k) * (1. / 1000.)) ;
        mtree->Fill();

      }
      for (int w = 0; w < FitFunctions.size(); w++) FitFunctions[w]->Delete();
      TotalFitFunction->Delete() ;
      gWaveformToFit->Delete();
    }
  }

  /** Template parameters now computed for input cellID range */
  histfile->cd();
  auto gmaxResidual = new TGraph(cellIDArray.size(), cellIDArray.data(), maxResidualArray.data());
  gmaxResidual->SetName("gmaxResidual");
  auto glimitResidualArray = new TGraph(cellIDArray.size(), cellIDArray.data(), limitResidualArray.data());
  glimitResidualArray->SetName("glimitResidualArray");
  auto gparLimitFactorArray = new TGraph(cellIDArray.size(), cellIDArray.data(), parLimitFactorArray.data());
  gparLimitFactorArray->SetName("gparLimitFactorArray");

  gmaxResidual->Write();
  glimitResidualArray->Write();
  gparLimitFactorArray->Write();
  histfile->Write();
  histfile->Close();
  delete histfile;

  f_PhotonTemplateOutput->cd();
  mtree->Write();
  f_PhotonTemplateOutput->Write();
  f_PhotonTemplateOutput->Close();
  delete f_PhotonTemplateOutput;

  /** Storing dbobject.  Will be accessed in merging stage (C4). */
  saveCalibration(PhotonParameters, Form("PhotonParameters_CellID%d_CellID%d", m_firstCellID, m_lastCellID));
  B2INFO("eclWaveformTemplateCalibrationC2Algorithm: successfully stored " << Form("PhotonParameters_CellID%d_CellID%d",
         m_firstCellID, m_lastCellID) << " constants");

  return c_OK;
}
