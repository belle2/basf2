/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//This module`
#include <ecl/modules/eclAutocovarianceCalibrationC1Collector/eclAutocovarianceCalibrationC1Collector.h>

//Root
#include <TH2I.h>

//Framework
#include <framework/dataobjects/EventMetaData.h>

//ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Modules
//-----------------------------------------------------------------
REG_MODULE(eclAutocovarianceCalibrationC1Collector)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
eclAutocovarianceCalibrationC1CollectorModule::eclAutocovarianceCalibrationC1CollectorModule()
{
  // Set module properties
  setDescription("Module to export histogram of noise in waveforms from random trigger events");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void eclAutocovarianceCalibrationC1CollectorModule::prepare()
{

  /**----------------------------------------------------------------------------------------*/
  B2INFO("eclAutocovarianceCalibrationC1Collector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " <<
         m_evtMetaData->getRun());

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  auto PPVsCrysID = new TH2I("PPVsCrysID", "Peak to peak amplitude for each crystal;crystal ID;Peak to peak Amplitud (ADC)", 8736, 0,
                             8736, 2000, 0, 2000);
  registerObject<TH2I>("PPVsCrysID", PPVsCrysID);

  m_eclDsps.registerInDataStore();
  m_eclDigits.registerInDataStore();
}


void eclAutocovarianceCalibrationC1CollectorModule::collect()
{

  const int NumDsp = m_eclDsps.getEntries();

  //Random Trigger Event
  if (NumDsp == 8736) {
    //if (NumDsp >0) {

    for (auto& aECLDsp : m_eclDsps) {

      const int id = aECLDsp.getCellId() - 1;

      int minADC = aECLDsp.getDspA()[0];
      int maxADC = aECLDsp.getDspA()[0];

      for (int i = 0; i < 31; i++) {

        if (aECLDsp.getDspA()[i] < minADC) minADC = aECLDsp.getDspA()[i];
        if (aECLDsp.getDspA()[i] > maxADC) maxADC = aECLDsp.getDspA()[i];

      }

      int PeakToPeak = maxADC - minADC;

      //B2INFO(PeakToPeak);

      getObjectPtr<TH2I>("PPVsCrysID")->Fill(id, PeakToPeak);

    }
  }
}
