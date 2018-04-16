/*
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * This module is used to compute the two component (photon+hadron)       *
 * fit to ecl waveforms stored offline.  Hadron component energy          *
 * measured from fit is used to perform pulse shape discrimination        *
 * for particle id.                                                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Savino Longo (longos@uvic.ca)                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <stdlib.h>
#include <iostream>

// ECL
#include <ecl/modules/eclWaveformFit/ECLWaveformFit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include "ecl/utility/ECLChannelMapper.h"
#include <ecl/digitization/OfflineFitFunction.h>
#include <ecl/digitization/EclConfiguration.h>

// FRAMEWORK
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/dbobjects/ECLDigitWaveformParameters.h>
#include <framework/database/DBObjPtr.h>
#include <framework/utilities/FileSystem.h>

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Modules
//-----------------------------------------------------------------
REG_MODULE(ECLWaveformFit)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
ECLWaveformFitModule::ECLWaveformFitModule()
{
  // Set module properties
  setDescription("Module to fit offline waveforms and measure hadron scintillation component light output.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("TriggerThreshold", m_TriggerThreshold,
           "Energy threshold of waveform trigger to ensure corresponding eclDigit is avaliable (GeV).", 0.01);
  addParam("EnergyThreshold", m_EnergyThreshold, "Energy threshold of online fit result for Fitting Waveforms (GeV).", 0.05);
  addParam("FitType", m_FitType, "Fit Type Flag for second component. 0 = Hadron, 1 = Diode.", 0);
}

// destructor
ECLWaveformFitModule::~ECLWaveformFitModule()
{
}

// initialize
void ECLWaveformFitModule::initialize()
{
  // ECL dataobjects
  StoreArray<ECLDsp> eclDsps(eclDspArrayName());
  StoreArray<ECLDigit> eclDigits(eclDigitArrayName());

  eclDsps.registerInDataStore(eclDspArrayName());
  eclDigits.registerInDataStore(eclDigitArrayName());
}

// begin run
void ECLWaveformFitModule::beginRun()
{

  DBObjPtr<ECLCrystalCalib> Ael("ECLCrystalElectronics"), Aen("ECLCrystalEnergy");
  m_ADCtoEnergy.resize(8736);
  if (Ael) for (int i = 0; i < 8736; i++) m_ADCtoEnergy[i] = Ael->getCalibVector()[i];
  if (Aen) for (int i = 0; i < 8736; i++) m_ADCtoEnergy[i] *= Aen->getCalibVector()[i];

  DBObjPtr<ECLDigitWaveformParameters>  WavePars("ECLDigitWaveformParameters");
  m_PhotonTemplates.resize(8736);
  m_SecondComponentTemplates.resize(8736);
  for (int i = 0; i < 8736; i++) {
    m_PhotonTemplates[i] = WavePars->getPhotonParameters(i + 1);
    if (m_FitType == 0) {
      m_SecondComponentTemplates[i] = WavePars->getHadronParameters(i + 1);
    } else {
      m_SecondComponentTemplates[i] = WavePars->getDiodeParameters(i + 1);
    }
    if (m_PhotonTemplates[i][0] == 0 || m_SecondComponentTemplates[i][0] == 0) {
      B2WARNING("Warning cellID: " << i + 1 << " has no waveforms." << std::endl);
    }
  }
}

std::vector<double> ECLWaveformFitModule::FitWithROOT(double InitialAmp,
                                                      std::vector<double> PhotonPars11,
                                                      std::vector<double> HadronPars11,
                                                      int ComponentNumber)
{

  //InitialAmp: = Starting Fit Amp
  //PhotonPars11: 11 Photon Template Pars
  //HadronPars11: 11 Hadron Template Pars

  const double convertToADCTime = 0.5;

  std::vector<double> WaveformAmp;
  std::vector<double> WaveformTime;
  std::vector<double> WaveformTimeError;
  std::vector<double> WaveformAmpError;

  for (int k = 0; k < EclConfiguration::m_nsmp; k++) {
    WaveformAmp.push_back((double)(m_CurrentPulseArray31[k]));
    WaveformAmpError.push_back(10);
    WaveformTime.push_back(k * convertToADCTime);
    WaveformTimeError.push_back(0.1);
  }

  TGraphErrors* WaveformGraph = new TGraphErrors(WaveformTime.size(),
                                                 &WaveformTime[0],
                                                 &WaveformAmp[0],
                                                 &WaveformTimeError[0],
                                                 &WaveformAmpError[0]);

  int LowFitLimit = 12 * convertToADCTime;
  int HighFitLimit = 32 * convertToADCTime;

  TF1* FitROOTHadron = new TF1("FitROOTHadron", WaveFuncTwoComponent, LowFitLimit, HighFitLimit, 26);
  FitROOTHadron->SetNpx(1000);
  FitROOTHadron->SetParameter(0, 8.);
  FitROOTHadron->SetParameter(1, WaveformAmp[1]);
  FitROOTHadron->SetParameter(2, InitialAmp);
  FitROOTHadron->SetParameter(3, 0);
  FitROOTHadron->FixParameter(24, PhotonPars11[0]);
  FitROOTHadron->FixParameter(25, HadronPars11[0]);
  for (int k = 0; k < 10; k++) {
    FitROOTHadron->FixParameter(4 + k, PhotonPars11[k + 1]);
    FitROOTHadron->FixParameter(10 + 4 + k, HadronPars11[k + 1]);
  }
  if (ComponentNumber == 1) {
    for (int k = 0; k < 10; k++)FitROOTHadron->FixParameter(10 + 4 + k, PhotonPars11[k + 1]);
    FitROOTHadron->FixParameter(3, 0);
  }
  WaveformGraph->Fit("FitROOTHadron", "Q N 0 R W", "", LowFitLimit, HighFitLimit);

  double OfflineFitChiSquareTwoComponent = 0;
  for (int k = 12; k < 31;
       k++)  OfflineFitChiSquareTwoComponent += (pow((WaveformAmp[k] - FitROOTHadron->Eval(WaveformTime[k])),
                                                       2) / (pow(WaveformAmpError[k],
                                                             2)));

  std::vector<double> tempResult(5);
  tempResult[0] = FitROOTHadron->GetParameter(2) + FitROOTHadron->GetParameter(3); //Total Amp
  tempResult[1] = FitROOTHadron->GetParameter(3); //Hadron Amp
  tempResult[2] = OfflineFitChiSquareTwoComponent; //Chi2
  tempResult[3] = FitROOTHadron->GetParameter(0); //Time
  tempResult[4] = FitROOTHadron->GetParameter(1); //Basline

  WaveformGraph->Delete();
  FitROOTHadron->Delete();

  return tempResult;
}

void ECLWaveformFitModule::event()
{

  StoreArray<ECLDsp> eclDsps(eclDspArrayName());
  StoreArray<ECLDigit> eclDigits(eclDigitArrayName());

  for (auto& aECLDsp : eclDsps) {

    aECLDsp.setTwoComponentTotalAmp(-1);
    aECLDsp.setTwoComponentHadronAmp(-1);
    aECLDsp.setTwoComponentChi2(-1);
    aECLDsp.setTwoComponentTime(-1);
    aECLDsp.setTwoComponentBaseline(-1);

    if (aECLDsp.getIsData() == false)  continue; //Currently for data only

    int CurrentCellID = aECLDsp.getCellId();

    if (aECLDsp.getNADCPoints() > 0) {

      //Filling array with ADC values.
      for (int j = 0; j < EclConfiguration::m_nsmp; j++)  m_CurrentPulseArray31[j] = aECLDsp.getDspA()[j];

      //Trigger check to remove noise pulses in random trigger events.
      //In random trigger events all eclDSP saved but only eclDigits above online threshold are saved.
      //Set 10 MeV threshold for now.
      //Trigger amplitude is computed with algorithm described in slide 5 of:
      //https://kds.kek.jp/indico/event/22581/session/20/contribution/236
      //note the trigger check is a temporary workaround to ensure all eclDsp's have a corresponding eclDigit.
      //
      double baselineADC = 0;
      for (int i = 0; i < 4; i++)  baselineADC += m_CurrentPulseArray31[i + 12];
      baselineADC /= 4.0;
      double maxADC = (m_CurrentPulseArray31[20] + m_CurrentPulseArray31[21]) / 2.0;

      double triggerCheck = (maxADC - baselineADC) * m_ADCtoEnergy[CurrentCellID - 1];

      if (triggerCheck < m_TriggerThreshold) continue;

      //setting relation of eclDSP to aECLDigit
      bool relationSet = false;
      double OnlineAmp = 0;
      for (auto& aECLDigit : eclDigits) {
        if (aECLDigit.getCellId() == CurrentCellID) {
          aECLDsp.addRelationTo(&aECLDigit);
          OnlineAmp = aECLDigit.getAmp();// Used for inital Fit Par
          relationSet = true;
          break;
        }
      }

      if (relationSet == false) {
        B2WARNING("Could not set eclDsp relation to eclDigit. ECLDsp CellID:" << CurrentCellID << " triggerCheck:" << triggerCheck <<
                  "eclDsps.getEntries():" << eclDsps.getEntries());
        continue;
      }

      const double OnlineEnergy = OnlineAmp *= m_ADCtoEnergy[CurrentCellID - 1];
      if (OnlineEnergy < m_EnergyThreshold)  continue;

      //Fit using ROOT::Fit with Photon + Hadron or Diode Templates
      std::vector<double> theROOTFit;
      theROOTFit = FitWithROOT(OnlineAmp, m_PhotonTemplates[CurrentCellID - 1], m_SecondComponentTemplates[CurrentCellID - 1], 2);

      aECLDsp.setTwoComponentTotalAmp(theROOTFit[0]);
      aECLDsp.setTwoComponentHadronAmp(theROOTFit[1]);
      aECLDsp.setTwoComponentChi2(theROOTFit[2]);
      aECLDsp.setTwoComponentTime(theROOTFit[3]);
      aECLDsp.setTwoComponentBaseline(theROOTFit[4]);

    }
  }
}

// end run
void ECLWaveformFitModule::endRun()
{
}

// terminate
void ECLWaveformFitModule::terminate()
{
}
