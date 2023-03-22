/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//This module`
#include <ecl/modules/eclAutocovarianceCalibrationC4Collector/eclAutocovarianceCalibrationC4Collector.h>

//Root
#include <TDecompChol.h>

//Framework
#include <framework/dataobjects/EventMetaData.h>

//ECL
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Modules
//-----------------------------------------------------------------
REG_MODULE(eclAutocovarianceCalibrationC4Collector)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
eclAutocovarianceCalibrationC4CollectorModule::eclAutocovarianceCalibrationC4CollectorModule() : CalibrationCollectorModule(),
  m_ECLAutocovarianceCalibrationC1Threshold("ECLAutocovarianceCalibrationC1Threshold"),
  m_ECLAutocovarianceCalibrationC2Baseline("ECLAutocovarianceCalibrationC2Baseline"),
  m_ECLAutocovarianceCalibrationC3Autocovariances("ECLAutocovarianceCalibrationC3Autocovariances")
{
  // Set module properties
  setDescription("Module to test covariance matrix calibrations using waveforms from random trigger events");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void eclAutocovarianceCalibrationC4CollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  B2INFO("eclAutocovarianceCalibrationC4Collector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " <<
         m_evtMetaData->getRun());

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  Chi2VsCrysID = new TH2F("Chi2VsCrysID", "", 8736, 0, 8736, 1000, 0, 1000);
  registerObject<TH2F>("Chi2VsCrysID", Chi2VsCrysID);

  m_PeakToPeakThresholds = m_ECLAutocovarianceCalibrationC1Threshold->getCalibVector();
  m_Baselines = m_ECLAutocovarianceCalibrationC2Baseline->getCalibVector();

  m_eclDsps.registerInDataStore();

  constexpr int N = 31;
  std::vector<double> buf(N);

  m_NoiseMatrix.resize(ECLElementNumbers::c_NCrystals);

  for (int id = 0; id < ECLElementNumbers::c_NCrystals; id++) {
    m_ECLAutocovarianceCalibrationC3Autocovariances->getAutoCovariance(id + 1, buf.data());
    m_NoiseMatrix[id].ResizeTo(31, 31);
    for (int i = 0; i < 31; i++) {
      for (int j = 0; j < 31; j++) {
        m_NoiseMatrix[id](i, j) = buf[abs(i - j)];
      }
    }
    TDecompChol dc(m_NoiseMatrix[id]);
    bool InvertStatus = dc.Invert(m_NoiseMatrix[id]);

    if (InvertStatus == 0) {
      B2INFO("Invert Failed for " << id);
    }

  }

}


void eclAutocovarianceCalibrationC4CollectorModule::collect()
{

  const int NumDsp = m_eclDsps.getEntries();

  //Random Trigger Events have waveform for each crystal
  if (NumDsp == ECLElementNumbers::c_NCrystals) {

    for (auto& aECLDsp : m_eclDsps) {

      const int id = aECLDsp.getCellId() - 1;

      //Peak to peak amplitude used to gauge noise level
      float PeakToPeak = (float) aECLDsp.computePeaktoPeakAmp();

      if (PeakToPeak < m_PeakToPeakThresholds[id]) {

        float baseline = m_Baselines[id];

        vector<double> waveform(31);
        for (int i = 0; i < 31; i++) waveform[i] = aECLDsp.getDspA()[i] - baseline;

        vector<double> temp(31, 0);
        for (int i = 0; i < 31; i++) {
          for (int j = 0; j < 31; j++) {
            temp[i] += m_NoiseMatrix[id](i, j) * (waveform[j]);
          }
        }
        double chi2val = 0;
        for (int i = 0; i < 31; i++) chi2val += (temp[i] * waveform[i]);

        Chi2VsCrysID->Fill(id, chi2val);
      }

    }
  }
}

void eclAutocovarianceCalibrationC4CollectorModule::closeRun()
{
  for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) {
    for (int j = 0; j < 1000; j++) {
      getObjectPtr<TH2>("Chi2VsCrysID")->SetBinContent(i + 1, j + 1, Chi2VsCrysID->GetBinContent(i + 1, j + 1));
    }
  }
}
