/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// ECL
#include <ecl/modules/eclWaveformCalibCollector/ECLWaveformCalibCollector.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>
#include <ecl/digitization/EclConfiguration.h>
#include <ecl/dbobjects/ECLAutoCovariance.h>

//FRAMEWORK
#include <framework/core/Environment.h>

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Modules
//-----------------------------------------------------------------
REG_MODULE(ECLWaveformCalibCollector)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
ECLWaveformCalibCollectorModule::ECLWaveformCalibCollectorModule()
{
  // Set module properties
  setDescription("Module to export waveforms to ntuple for template calibration.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("OutputFileName", m_OutputFileName, "Output file directory.", std::string("input"));
  addParam("EnergyThresholdGeV", m_EnergyThresholdGeV, "Energy Threshold in GeV.", 1.0);
}

// destructor
ECLWaveformCalibCollectorModule::~ECLWaveformCalibCollectorModule()
{
}

void ECLWaveformCalibCollectorModule::beginRun()
{

  //used to convert ADC to GeV
  DBObjPtr<ECLCrystalCalib> Ael("ECLCrystalElectronics"), Aen("ECLCrystalEnergy");
  m_ADCtoEnergy.resize(8736);
  if (Ael) for (int i = 0; i < 8736; i++) m_ADCtoEnergy[i] = Ael->getCalibVector()[i];
  if (Aen) for (int i = 0; i < 8736; i++) m_ADCtoEnergy[i] *= Aen->getCalibVector()[i];

}

void ECLWaveformCalibCollectorModule::initialize()
{
  // ECL dataobjects
  m_eclDSPs.registerInDataStore();
  m_eclDigits.registerInDataStore();

  m_EventsProcessed = 0;

  m_rootFilePtr = new TFile(m_OutputFileName.c_str(), "RECREATE");
  m_tree = new TTree("m_tree", "");
  m_tree->Branch("CellID", &m_CellID,      "m_CellID/I");
  m_tree->Branch("OnlineE", &m_OnlineE,      "m_OnlineE/D");
  m_tree->Branch("OfflineE", &m_OfflineE,      "m_OfflineE/D");
  m_tree->Branch("OfflineHadE", &m_OfflineHadE,      "m_OfflineHadE/D");
  m_tree->Branch("FitType", &m_FitType,      "m_FitType/I");
  m_tree->Branch("Chi2", &m_Chi2,      "m_Chi2/D");
  m_tree->Branch("Chi2Save0", &m_Chi2Save0,      "m_Chi2Save0/D");
  m_tree->Branch("Chi2Save1", &m_Chi2Save1,      "m_Chi2Save1/D");
  m_tree->Branch("Chi2Save2", &m_Chi2Save2,      "m_Chi2Save2/D");
  m_tree->Branch("m_Baseline", &m_Baseline,      "m_Baseline/D");
  m_tree->Branch("m_BaselineRMS", &m_BaselineRMS,      "m_BaselineRMS/D");
  m_tree->Branch("m_calibConst", &m_calibConst,      "m_calibConst/F");
  m_tree->Branch("ADC0", &m_ADC0,      "m_ADC0/I");
  m_tree->Branch("ADC1", &m_ADC1,      "m_ADC1/I");
  m_tree->Branch("ADC2", &m_ADC2,      "m_ADC2/I");
  m_tree->Branch("ADC3", &m_ADC3,      "m_ADC3/I");
  m_tree->Branch("ADC4", &m_ADC4,      "m_ADC4/I");
  m_tree->Branch("ADC5", &m_ADC5,      "m_ADC5/I");
  m_tree->Branch("ADC6", &m_ADC6,      "m_ADC6/I");
  m_tree->Branch("ADC7", &m_ADC7,      "m_ADC7/I");
  m_tree->Branch("ADC8", &m_ADC8,      "m_ADC8/I");
  m_tree->Branch("ADC9", &m_ADC9,      "m_ADC9/I");
  m_tree->Branch("ADC10", &m_ADC10,      "m_ADC10/I");
  m_tree->Branch("ADC11", &m_ADC11,      "m_ADC11/I");
  m_tree->Branch("ADC12", &m_ADC12,      "m_ADC12/I");
  m_tree->Branch("ADC13", &m_ADC13,      "m_ADC13/I");
  m_tree->Branch("ADC14", &m_ADC14,      "m_ADC14/I");
  m_tree->Branch("ADC15", &m_ADC15,      "m_ADC15/I");
  m_tree->Branch("ADC16", &m_ADC16,      "m_ADC16/I");
  m_tree->Branch("ADC17", &m_ADC17,      "m_ADC17/I");
  m_tree->Branch("ADC18", &m_ADC18,      "m_ADC18/I");
  m_tree->Branch("ADC19", &m_ADC19,      "m_ADC19/I");
  m_tree->Branch("ADC20", &m_ADC20,      "m_ADC20/I");
  m_tree->Branch("ADC21", &m_ADC21,      "m_ADC21/I");
  m_tree->Branch("ADC22", &m_ADC22,      "m_ADC22/I");
  m_tree->Branch("ADC23", &m_ADC23,      "m_ADC23/I");
  m_tree->Branch("ADC24", &m_ADC24,      "m_ADC24/I");
  m_tree->Branch("ADC25", &m_ADC25,      "m_ADC25/I");
  m_tree->Branch("ADC26", &m_ADC26,      "m_ADC26/I");
  m_tree->Branch("ADC27", &m_ADC27,      "m_ADC27/I");
  m_tree->Branch("ADC28", &m_ADC28,      "m_ADC28/I");
  m_tree->Branch("ADC29", &m_ADC29,      "m_ADC29/I");
  m_tree->Branch("ADC30", &m_ADC30,      "m_ADC30/I");

}

void ECLWaveformCalibCollectorModule::event()
{

  //intended to run over gamma gamma cdst skim.

  const int NumDsp = m_eclDSPs.getEntries();

  for (auto& aECLDigit : m_eclDigits) {

    int cellid = aECLDigit.getCellId();

    const int amplitude = aECLDigit.getAmp();

    double energy = amplitude * m_ADCtoEnergy[cellid - 1];

    if (energy > m_EnergyThresholdGeV) {
      for (auto& aECLDsp : m_eclDSPs) {
        if (aECLDsp.getCellId() == cellid) {
          m_Chi2 = aECLDsp.getTwoComponentChi2();
          m_Chi2Save0 = aECLDsp.getTwoComponentSavedChi2(ECLDsp::photonHadron);
          m_Chi2Save1 = aECLDsp.getTwoComponentSavedChi2(ECLDsp::photonHadronBackgroundPhoton);
          m_Chi2Save2 = aECLDsp.getTwoComponentSavedChi2(ECLDsp::photonDiodeCrossing);
          m_CellID = cellid;
          m_OnlineE = amplitude;
          m_OfflineE = aECLDsp.getTwoComponentTotalAmp();
          m_FitType = aECLDsp.getTwoComponentFitType();
          m_OfflineHadE = aECLDsp.getTwoComponentHadronAmp();
          m_calibConst = m_ADCtoEnergy[cellid - 1];

          //computing baseline noise
          const double BaselineLimit = 10.0;
          double iLimit = 1. / BaselineLimit;
          double baselinemean = 0.0;
          for (int i = 0; i < BaselineLimit; i++) baselinemean += aECLDsp.getDspA()[i];
          baselinemean = baselinemean * iLimit;
          double baslineRMS = 0.0;
          for (int i = 0; i < BaselineLimit; i++) {
            double temp = aECLDsp.getDspA()[i] - baselinemean;
            baslineRMS += (temp * temp);
          }
          baslineRMS *= iLimit;//squared

          m_Baseline = baselinemean;
          m_BaselineRMS = baslineRMS;

          m_ADC0 = aECLDsp.getDspA()[0];
          m_ADC1 = aECLDsp.getDspA()[1];
          m_ADC2 = aECLDsp.getDspA()[2];
          m_ADC3 = aECLDsp.getDspA()[3];
          m_ADC4 = aECLDsp.getDspA()[4];
          m_ADC5 = aECLDsp.getDspA()[5];
          m_ADC6 = aECLDsp.getDspA()[6];
          m_ADC7 = aECLDsp.getDspA()[7];
          m_ADC8 = aECLDsp.getDspA()[8];
          m_ADC9 = aECLDsp.getDspA()[9];
          m_ADC10 = aECLDsp.getDspA()[10];
          m_ADC11 = aECLDsp.getDspA()[11];
          m_ADC12 = aECLDsp.getDspA()[12];
          m_ADC13 = aECLDsp.getDspA()[13];
          m_ADC14 = aECLDsp.getDspA()[14];
          m_ADC15 = aECLDsp.getDspA()[15];
          m_ADC16 = aECLDsp.getDspA()[16];
          m_ADC17 = aECLDsp.getDspA()[17];
          m_ADC18 = aECLDsp.getDspA()[18];
          m_ADC19 = aECLDsp.getDspA()[19];
          m_ADC20 = aECLDsp.getDspA()[20];
          m_ADC21 = aECLDsp.getDspA()[21];
          m_ADC22 = aECLDsp.getDspA()[22];
          m_ADC23 = aECLDsp.getDspA()[23];
          m_ADC24 = aECLDsp.getDspA()[24];
          m_ADC25 = aECLDsp.getDspA()[25];
          m_ADC26 = aECLDsp.getDspA()[26];
          m_ADC27 = aECLDsp.getDspA()[27];
          m_ADC28 = aECLDsp.getDspA()[28];
          m_ADC29 = aECLDsp.getDspA()[29];
          m_ADC30 = aECLDsp.getDspA()[30];

          m_tree->Fill();
        }
      }
    }

  }

}

// end run
void ECLWaveformCalibCollectorModule::endRun()
{
}

// terminate
void ECLWaveformCalibCollectorModule::terminate()
{
  m_rootFilePtr->cd();
  m_tree->Write();
  m_rootFilePtr->Close();
}
