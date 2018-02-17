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

// ECL
#include <ecl/modules/eclWaveformFit/ECLWaveformFit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLEventInformation.h>
#include "ecl/utility/ECLChannelMapper.h"
#include <ecl/digitization/OfflineFitFunction.h>
#include <ecl/digitization/EclConfiguration.h>

#include <stdlib.h>
#include <iostream>

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
  addParam("EnergyThreshold", m_EnergyThreshold, "Energy Threshold for Fitting Waveforms", 0.05);
  addParam("FitType", m_FitType, "Fit Type Flag for second component. 0 = Hadron, 1 = Diode.", 0);
}

// destructor
ECLWaveformFitModule::~ECLWaveformFitModule()
{
}
//
// initialize
void ECLWaveformFitModule::initialize()
{
  // ECL dataobjects
  StoreArray<ECLDsp> eclDsps(eclDspArrayName());
  StoreArray<ECLDigit> eclDigits(eclDigitArrayName());
  //
  eclDsps.registerInDataStore(eclDspArrayName());
  eclDigits.registerInDataStore(eclDigitArrayName());
  //
  StoreObjPtr<ECLEventInformation> eclEventInformationPtr(eclEventInformationName());
  eclEventInformationPtr.registerInDataStore(eclEventInformationName());
  //
}
//
// begin run
void ECLWaveformFitModule::beginRun()
{
  //
  DBObjPtr<ECLCrystalCalib> Ael("ECLCrystalElectronics"), Aen("ECLCrystalEnergy");
  m_ADCtoEnergy.resize(8736);
  if (Ael) for (int i = 0; i < 8736; i++) m_ADCtoEnergy[i] = Ael->getCalibVector()[i];
  if (Aen) for (int i = 0; i < 8736; i++) m_ADCtoEnergy[i] *= Aen->getCalibVector()[i];
  //
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
//
std::vector<double> ECLWaveformFitModule::FitWithROOT(double InitialAmp,
                                                      std::vector<double> PhotonPars11,
                                                      std::vector<double> HadronPars11,
                                                      int ComponentNumber)
{
  //
  //InitialAmp: = Starting Fit Amp
  //PhotonPars11: 11 Photon Template Pars
  //HadronPars11: 11 Hadron Template Pars
  //ComponentNumber: If 1 use only photon template, else use both components
  //
  const double convertToADCTime = 0.5;
  //
  std::vector<double> WaveformAmp;
  std::vector<double> WaveformTime;
  std::vector<double> WaveformTimeError;
  std::vector<double> WaveformAmpError;
  //
  for (int k = 0; k < EclConfiguration::m_nsmp; k++) {
    WaveformAmp.push_back((double)(m_CurrentPulseArray31[k]));
    WaveformAmpError.push_back(10);
    WaveformTime.push_back(k * convertToADCTime);
    WaveformTimeError.push_back(0.1);
  }
  //
  TGraphErrors* WaveformGraph = new TGraphErrors(WaveformTime.size(),
                                                 &WaveformTime[0],
                                                 &WaveformAmp[0],
                                                 &WaveformTimeError[0],
                                                 &WaveformAmpError[0]);
  //
  int LowFitLimit = 12 * convertToADCTime;
  int HighFitLimit = 32 * convertToADCTime;
  //
  TF1* FitROOTHadron = new TF1("FitROOTHadron", WaveFuncTwoComp, LowFitLimit, HighFitLimit, 26);
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
  //
  double OfflineFitChiSquareTwoComp = 0;
  for (int k = 12; k < 31;
       k++)  OfflineFitChiSquareTwoComp += (pow((WaveformAmp[k] - FitROOTHadron->Eval(WaveformTime[k])), 2) / (pow(WaveformAmpError[k],
                                              2)));
  //
  std::vector<double> tempResult(5);
  tempResult[0] = FitROOTHadron->GetParameter(2) + FitROOTHadron->GetParameter(3); //Total Amp
  tempResult[1] = FitROOTHadron->GetParameter(3); //Hadron Amp
  tempResult[2] = OfflineFitChiSquareTwoComp; //Chi2
  tempResult[3] = FitROOTHadron->GetParameter(0); //Time
  tempResult[4] = FitROOTHadron->GetParameter(1); //Basline
  //
  WaveformGraph->Delete();
  FitROOTHadron->Delete();
  //
  return tempResult;
}
//
void ECLWaveformFitModule::event()
{
//
  StoreArray<ECLDsp> eclDsps(eclDspArrayName());
  StoreArray<ECLDigit> eclDigits(eclDigitArrayName());
  //
  for (auto& aECLDsp : eclDsps) {
    //
    aECLDsp.setTwoCompTotalAmp(-1);
    aECLDsp.setTwoCompHadronAmp(-1);
    aECLDsp.setTwoCompChi2(-1);
    aECLDsp.setTwoCompTime(-1);
    aECLDsp.setTwoCompBaseline(-1);
    //
    if (aECLDsp.getDataMCFlag() == false)  continue; //Currently for data only
    //
    int CurrentCellID = aECLDsp.getCellId();
    //
    //setting relation of eclDSP to aECLDigit
    bool RelationSet = false;
    double OnlineAmp = 0;
    for (auto& aECLDigit : eclDigits) {
      if (aECLDigit.getCellId() == CurrentCellID) {
        aECLDsp.addRelationTo(&aECLDigit);
        OnlineAmp = aECLDigit.getAmp();// Used for inital Fit Par
        RelationSet = true;
        break;
      }
    }
    //
    if (RelationSet == false) {
      B2WARNING("Could not set relation to eclDigit. ECLDsp CellID:" << CurrentCellID);
      continue;
    }
    //
    const double OnlineEnergy = OnlineAmp *= m_ADCtoEnergy[CurrentCellID - 1];
    if (OnlineEnergy < m_EnergyThreshold)  continue;
    //
    if (aECLDsp.getNADCPoints() > 0) {
      //
      //Filling array with ADC values.
      //TriggerCheck used to skip saved waveforms with no pulses (noise)
      double TriggerCheck = 0;
      for (int j = 0; j < EclConfiguration::m_nsmp; j++) {
        m_CurrentPulseArray31[j] = aECLDsp.getDspA()[j];
        if (j > 0) {
          double te = m_CurrentPulseArray31[j] - m_CurrentPulseArray31[j - 1];
          if (te > TriggerCheck)  TriggerCheck = te;
        }
      }
      if (TriggerCheck < 100) continue;
      //
      //Fit using ROOT::Fit with Photon + Hadron or Diode Templates
      std::vector<double> theROOTFit;
      theROOTFit = FitWithROOT(OnlineAmp, m_PhotonTemplates[CurrentCellID - 1], m_SecondComponentTemplates[CurrentCellID - 1], 2);
      //
      aECLDsp.setTwoCompTotalAmp(theROOTFit[0]);
      aECLDsp.setTwoCompHadronAmp(theROOTFit[1]);
      aECLDsp.setTwoCompChi2(theROOTFit[2]);
      aECLDsp.setTwoCompTime(theROOTFit[3]);
      aECLDsp.setTwoCompBaseline(theROOTFit[4]);
      //
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
