/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <ecl/modules/eclWaveformCalibCollector/eclWaveformCalibCollectorModule.h>

/* ECL headers. */
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLElementNumbers.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

/* Basf2 headers. */
#include <framework/gearbox/Const.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationVector.h>

/* ROOT headers. */
#include <TTree.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(eclWaveformCalibCollector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------

eclWaveformCalibCollectorModule::eclWaveformCalibCollectorModule() : Module()
{
  // Set module properties
  setDescription("Module to export waveforms to ntuple for template calibration.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("outputFileName", m_dataOutFileName, "Output root file name of this module", string("digistudy"));
  addParam("LowEnergyThresholdGeV", m_LowEnergyThresholdGeV, "Low Energy Threshold in GeV.", 1.0);
  addParam("HighEnergyThresholdGeV", m_HighEnergyThresholdGeV, "High Energy Threshold in GeV.", 5.5);
  addParam("IncludeWaveforms", m_includeWaveforms, "Flag to save ADC information.", true);
  addParam("selectCellID", m_selectCellID, "High Energy Threshold in GeV.", -1);
}



/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclWaveformCalibCollectorModule::initialize()
{
  // Initializing the output root file
  string dataFileName = m_dataOutFileName + ".root";
  m_rootFile = new TFile(dataFileName.c_str(), "RECREATE");

  // ECL dataobjects
  m_eclDSPs.registerInDataStore();
  m_eclDigits.registerInDataStore();

  tree = new TTree("tree", "");
  tree->Branch("CellID", &m_CellID,      "m_CellID/I");
  tree->Branch("runNum", &m_runNum,      "m_runNum/I");
  tree->Branch("expNum", &m_expNum,      "m_expNum/I");
  tree->Branch("OnlineE", &m_OnlineE,      "m_OnlineE/F");
  tree->Branch("OfflineE", &m_OfflineE,      "m_OfflineE/F");
  tree->Branch("OfflineHadE", &m_OfflineHadE,      "m_OfflineHadE/F");
  tree->Branch("FitType", &m_FitType,      "m_FitType/I");
  tree->Branch("Chi2", &m_Chi2,      "m_Chi2/F");
  tree->Branch("Chi2Save0", &m_Chi2Save0,      "m_Chi2Save0/F");
  tree->Branch("Chi2Save1", &m_Chi2Save1,      "m_Chi2Save1/F");
  tree->Branch("Chi2Save2", &m_Chi2Save2,      "m_Chi2Save2/F");
  tree->Branch("m_Baseline", &m_Baseline,      "m_Baseline/F");
  tree->Branch("m_BaselineRMS", &m_BaselineRMS,      "m_BaselineRMS/F");
  tree->Branch("m_calibConst", &m_calibConst,      "m_calibConst/F");

  if (m_includeWaveforms) {
    tree->Branch("ADC0", &m_ADC0,      "m_ADC0/I");
    tree->Branch("ADC1", &m_ADC1,      "m_ADC1/I");
    tree->Branch("ADC2", &m_ADC2,      "m_ADC2/I");
    tree->Branch("ADC3", &m_ADC3,      "m_ADC3/I");
    tree->Branch("ADC4", &m_ADC4,      "m_ADC4/I");
    tree->Branch("ADC5", &m_ADC5,      "m_ADC5/I");
    tree->Branch("ADC6", &m_ADC6,      "m_ADC6/I");
    tree->Branch("ADC7", &m_ADC7,      "m_ADC7/I");
    tree->Branch("ADC8", &m_ADC8,      "m_ADC8/I");
    tree->Branch("ADC9", &m_ADC9,      "m_ADC9/I");
    tree->Branch("ADC10", &m_ADC10,      "m_ADC10/I");
    tree->Branch("ADC11", &m_ADC11,      "m_ADC11/I");
    tree->Branch("ADC12", &m_ADC12,      "m_ADC12/I");
    tree->Branch("ADC13", &m_ADC13,      "m_ADC13/I");
    tree->Branch("ADC14", &m_ADC14,      "m_ADC14/I");
    tree->Branch("ADC15", &m_ADC15,      "m_ADC15/I");
    tree->Branch("ADC16", &m_ADC16,      "m_ADC16/I");
    tree->Branch("ADC17", &m_ADC17,      "m_ADC17/I");
    tree->Branch("ADC18", &m_ADC18,      "m_ADC18/I");
    tree->Branch("ADC19", &m_ADC19,      "m_ADC19/I");
    tree->Branch("ADC20", &m_ADC20,      "m_ADC20/I");
    tree->Branch("ADC21", &m_ADC21,      "m_ADC21/I");
    tree->Branch("ADC22", &m_ADC22,      "m_ADC22/I");
    tree->Branch("ADC23", &m_ADC23,      "m_ADC23/I");
    tree->Branch("ADC24", &m_ADC24,      "m_ADC24/I");
    tree->Branch("ADC25", &m_ADC25,      "m_ADC25/I");
    tree->Branch("ADC26", &m_ADC26,      "m_ADC26/I");
    tree->Branch("ADC27", &m_ADC27,      "m_ADC27/I");
    tree->Branch("ADC28", &m_ADC28,      "m_ADC28/I");
    tree->Branch("ADC29", &m_ADC29,      "m_ADC29/I");
    tree->Branch("ADC30", &m_ADC30,      "m_ADC30/I");
  }

  //called at beginning of run
  //used to convert ADC to GeV
  DBObjPtr<ECLCrystalCalib> Ael("ECLCrystalElectronics"), Aen("ECLCrystalEnergy");
  m_ADCtoEnergy.resize(ECLElementNumbers::c_NCrystals);
  if (Ael) for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) m_ADCtoEnergy[i] = Ael->getCalibVector()[i];
  if (Aen) for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) m_ADCtoEnergy[i] *= Aen->getCalibVector()[i];
}

void eclWaveformCalibCollectorModule::terminate()
{
  m_rootFile->cd();
  tree->Write();
  m_rootFile->Close();
}

/**----------------------------------------------------------------------------------------*/
/**----------------------------------------------------------------------------------------*/
void eclWaveformCalibCollectorModule::event()
{
  //intended to run over gamma gamma cdst skim.

  m_runNum = m_EventMetaData->getRun();
  m_expNum = m_EventMetaData->getExperiment();

  for (auto& aECLDigit : m_eclDigits) {

    int cellid = aECLDigit.getCellId();

    if (m_selectCellID > 0 && cellid != m_selectCellID)  continue;

    const int amplitude = aECLDigit.getAmp();

    double energy = amplitude * m_ADCtoEnergy[cellid - 1];

    if (energy > m_LowEnergyThresholdGeV && energy < m_HighEnergyThresholdGeV) {
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
          double baselinemean = 0.0;
          for (int i = 0; i < BaselineLimit; i++) baselinemean += aECLDsp.getDspA()[i];
          baselinemean /= BaselineLimit;
          double baselineRMS = 0.0;
          for (int i = 0; i < BaselineLimit; i++) {
            double temp = aECLDsp.getDspA()[i] - baselinemean;
            baselineRMS += (temp * temp);
          }
          baselineRMS /= BaselineLimit; //squared

          m_Baseline = baselinemean;
          m_BaselineRMS = baselineRMS;

          if (m_includeWaveforms) {
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
          }

          tree->Fill();
        }
      }
    }
  }
}
