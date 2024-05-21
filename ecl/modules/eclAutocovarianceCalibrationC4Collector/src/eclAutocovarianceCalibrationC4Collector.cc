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
REG_MODULE(eclAutocovarianceCalibrationC4Collector);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
eclAutocovarianceCalibrationC4CollectorModule::eclAutocovarianceCalibrationC4CollectorModule() : CalibrationCollectorModule(),
  m_ECLAutocovarianceCalibrationC1Threshold("ECLAutocovarianceCalibrationC1Threshold"),
  m_ECLAutocovarianceCalibrationC3Autocovariances("ECLAutoCovariance")
{
  // Set module properties
  setDescription("Module to test covariance matrix calibrations using waveforms from delayed Bhabha events");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void eclAutocovarianceCalibrationC4CollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  Chi2VsCrysID = new TH2F("Chi2VsCrysID", "", ECLElementNumbers::c_NCrystals, 0, ECLElementNumbers::c_NCrystals,
                          m_NbinsForChi2Histogram, 0, m_upperThresholdForChi2Histogram);
  registerObject<TH2F>("Chi2VsCrysID", Chi2VsCrysID);

  m_eclDsps.isRequired();

}

void eclAutocovarianceCalibrationC4CollectorModule::startRun()
{
  /**----------------------------------------------------------------------------------------*/
  B2INFO("eclAutocovarianceCalibrationC4Collector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " <<
         m_evtMetaData->getRun());

  m_PeakToPeakThresholds = m_ECLAutocovarianceCalibrationC1Threshold->getCalibVector();

  std::vector<double> buf(m_numberofADCPoints);

  m_NoiseMatrix.resize(ECLElementNumbers::c_NCrystals);

  for (int id = 0; id < ECLElementNumbers::c_NCrystals; id++) {
    m_ECLAutocovarianceCalibrationC3Autocovariances->getAutoCovariance(id + 1, buf.data());
    m_NoiseMatrix[id].ResizeTo(m_numberofADCPoints, m_numberofADCPoints);
    for (int i = 0; i < m_numberofADCPoints; i++) {
      for (int j = 0; j < m_numberofADCPoints; j++) {
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

        //Computing baselinw subtracted waveform. aECLDsp.getDspA() is the raw ECLDsp waveform.
        vector<double> waveform(m_numberofADCPoints);
        //for (int i = 0; i < m_numberofADCPoints; i++) waveform[i] = aECLDsp.getDspA()[i] - baseline;
        for (int i = 0; i < m_numberofADCPoints; i++) waveform[i] = aECLDsp.getDspA()[i];

        //After subtraccting baseline, expected value is 0.  Assuming noise is properly modelld by the cov. mat. (computed by this calibration) then a "fit" to a constant should give a good chi2.  Below computes the resulting chi2 for a fit to 0 and used the calibrated cov. mat. to model the noise.

        // chi^2(B) = a*B^2 - 2*B*b + c
        double aconst = 0, bconst = 0, cconst = 0;
        for (int i = 0; i < 31; i++) {
          double sum = 0, sumQ = 0;
          for (int j = 0; j < 31; j++) {
            sum += m_NoiseMatrix[id](i, j);
            sumQ += m_NoiseMatrix[id](i, j) * waveform[j];
          }
          aconst += sum;
          bconst += sum * waveform[i];
          cconst += sumQ * waveform[i];
        }

        const double Bconst = bconst / aconst;
        const double chi2val = ((aconst * Bconst - 2 * bconst) * Bconst + cconst);

        Chi2VsCrysID->Fill(id, chi2val);
      }

    }
  }
}

void eclAutocovarianceCalibrationC4CollectorModule::closeRun()
{
  for (int i = 0; i < ECLElementNumbers::c_NCrystals; i++) {
    for (int j = 0; j < m_NbinsForChi2Histogram; j++) {
      getObjectPtr<TH2>("Chi2VsCrysID")->SetBinContent(i + 1, j + 1, Chi2VsCrysID->GetBinContent(i + 1, j + 1));
    }
  }
}
