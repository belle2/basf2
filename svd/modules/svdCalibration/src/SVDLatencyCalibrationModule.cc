/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdCalibration/SVDLatencyCalibrationModule.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDLatencyCalibration)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDLatencyCalibrationModule::SVDLatencyCalibrationModule() : Module()
{
  // Set module properties
  setDescription("this module perfoms an analysis to find the APV25 latency");

  // Parameter definitions
  addParam("ShaperDigitsName", m_shapersListName, "SVDShaperDigits list name", std::string(""));
  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDLatencyCalibration_output.root"));


}


void SVDLatencyCalibrationModule::initialize()
{
  m_digits.isRequired(m_shapersListName);

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  //  m_histoList = new TList;

}

void SVDLatencyCalibrationModule::beginRun()
{
  TH1F h_maxAmplitude("maxAmpl_L@layerL@ladderS@sensor@view",
                      "bin containing the max of the sampled Amplitude for L@layerL@ladder@sensor@view", 6, -0.5, 5.5);
  h_maxAmplitude.GetXaxis()->SetTitle("max bin");

  m_histo_maxAmplitude = new SVDHistograms<TH1F>(h_maxAmplitude);
}

void SVDLatencyCalibrationModule::event()
{

  //start loop on SVDShaperDigits
  Belle2::SVDShaperDigit::APVFloatSamples samples;

  for (const SVDShaperDigit& shaper : m_digits) {

    samples = shaper.getSamples();

    VxdID sensorID = shaper.getSensorID();

    float amplitude = 0;
    int maxbin = 0 ;
    const int nAPVSamples = 6;
    for (int k = 0; k < nAPVSamples; k ++) {
      if (samples[k] > amplitude) {
        amplitude = samples[k];
        maxbin = k;
      }
    }

    m_histo_maxAmplitude->fill(sensorID, shaper.isUStrip() ? 1 : 0, maxbin);

  }
}

void SVDLatencyCalibrationModule::endRun()
{

  if (m_rootFilePtr != nullptr) {
    m_rootFilePtr->cd();

    VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();
    for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD))
      for (auto ladder : geoCache.getLadders(layer))
        for (Belle2::VxdID sensor :  geoCache.getSensors(ladder))
          for (int view = SVDHistograms<TH1F>::VIndex ; view < SVDHistograms<TH1F>::UIndex + 1; view++)
            (m_histo_maxAmplitude->getHistogram(sensor, view))->Write();
  }

  m_rootFilePtr->Close();

  //  m_histo_maxAmplitude->clean();

}

