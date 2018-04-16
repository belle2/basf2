/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: casarosa                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

  m_histoList = new TList;

  TString NameOfHisto = "";
  TString TitleOfHisto = "";

  NameOfHisto = "ladder";
  TitleOfHisto = "SVD Ladder Number";
  h_ladder = createHistogram1D(NameOfHisto, TitleOfHisto, 16, -0.5, 15.5, "ladder", m_histoList);

  NameOfHisto = "maxAmpliU";
  TitleOfHisto = "bin containing the max of the sampled Amplitude - U side";
  h_maxAmplitudeU = createHistogram1D(NameOfHisto, TitleOfHisto, 6, -0.5, 5.5, "max bin", m_histoList);

  NameOfHisto = "maxAmpliU_L3fw";
  h_maxAmplitudeU_L3fw = createHistogram1D(NameOfHisto, TitleOfHisto, 6, -0.5, 5.5, "max bin", m_histoList);
  NameOfHisto = "maxAmpliU_L3bw";
  h_maxAmplitudeU_L3bw = createHistogram1D(NameOfHisto, TitleOfHisto, 6, -0.5, 5.5, "max bin", m_histoList);
  NameOfHisto = "maxAmpliU_L4bw";
  h_maxAmplitudeU_L4bw = createHistogram1D(NameOfHisto, TitleOfHisto, 6, -0.5, 5.5, "max bin", m_histoList);
  NameOfHisto = "maxAmpliU_L5bw";
  h_maxAmplitudeU_L5bw = createHistogram1D(NameOfHisto, TitleOfHisto, 6, -0.5, 5.5, "max bin", m_histoList);
  NameOfHisto = "maxAmpliU_L6bw";
  h_maxAmplitudeU_L6bw = createHistogram1D(NameOfHisto, TitleOfHisto, 6, -0.5, 5.5, "max bin", m_histoList);

  NameOfHisto = "maxAmpliV";
  TitleOfHisto = "bin containing the max of the sampled Amplitude - V side";
  h_maxAmplitudeV = createHistogram1D(NameOfHisto, TitleOfHisto, 6, -0.5, 5.5, "max bin", m_histoList);

  NameOfHisto = "maxAmpliV_L3fw";
  h_maxAmplitudeV_L3fw = createHistogram1D(NameOfHisto, TitleOfHisto, 6, -0.5, 5.5, "max bin", m_histoList);
  NameOfHisto = "maxAmpliV_L3bw";
  h_maxAmplitudeV_L3bw = createHistogram1D(NameOfHisto, TitleOfHisto, 6, -0.5, 5.5, "max bin", m_histoList);
  NameOfHisto = "maxAmpliV_L4bw";
  h_maxAmplitudeV_L4bw = createHistogram1D(NameOfHisto, TitleOfHisto, 6, -0.5, 5.5, "max bin", m_histoList);
  NameOfHisto = "maxAmpliV_L5bw";
  h_maxAmplitudeV_L5bw = createHistogram1D(NameOfHisto, TitleOfHisto, 6, -0.5, 5.5, "max bin", m_histoList);
  NameOfHisto = "maxAmpliV_L6bw";
  h_maxAmplitudeV_L6bw = createHistogram1D(NameOfHisto, TitleOfHisto, 6, -0.5, 5.5, "max bin", m_histoList);




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
    if (!shaper.isUStrip()) {
      h_maxAmplitudeV->Fill(maxbin);

      if (sensorID.getLayerNumber() == 3) {
        if (sensorID.getSensorNumber() == 1)
          h_maxAmplitudeV_L3fw->Fill(maxbin);
        else
          h_maxAmplitudeV_L3bw->Fill(maxbin);
      }

      if (sensorID.getLayerNumber() == 4)
        if (sensorID.getSensorNumber() == 3)
          h_maxAmplitudeV_L4bw->Fill(maxbin);

      if (sensorID.getLayerNumber() == 5)
        if (sensorID.getSensorNumber() == 4)
          h_maxAmplitudeV_L5bw->Fill(maxbin);

      if (sensorID.getLayerNumber() == 6)
        if (sensorID.getSensorNumber() == 5)
          h_maxAmplitudeV_L6bw->Fill(maxbin);

    } else {
      h_maxAmplitudeU->Fill(maxbin);

      if (sensorID.getLayerNumber() == 3) {
        if (sensorID.getSensorNumber() == 1)
          h_maxAmplitudeU_L3fw->Fill(maxbin);
        else
          h_maxAmplitudeU_L3bw->Fill(maxbin);
      }

      if (sensorID.getLayerNumber() == 4)
        if (sensorID.getSensorNumber() == 3)
          h_maxAmplitudeU_L4bw->Fill(maxbin);

      if (sensorID.getLayerNumber() == 5)
        if (sensorID.getSensorNumber() == 4)
          h_maxAmplitudeU_L5bw->Fill(maxbin);

      if (sensorID.getLayerNumber() == 6)
        if (sensorID.getSensorNumber() == 5)
          h_maxAmplitudeU_L6bw->Fill(maxbin);
    }

    int ladder = shaper.getSensorID().getLadderNumber();
    h_ladder->Fill(ladder);
  }
}

void SVDLatencyCalibrationModule::terminate()
{

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    TObject* obj;

    TIter nextH(m_histoList);
    while ((obj = nextH()))
      obj->Write();

  }
  m_rootFilePtr->Close();

}


TH1F*  SVDLatencyCalibrationModule::createHistogram1D(const char* name, const char* title,
                                                      Int_t nbins, Double_t min, Double_t max,
                                                      const char* xtitle, TList* histoList)
{

  TH1F* h = new TH1F(name, title, nbins, min, max);

  h->GetXaxis()->SetTitle(xtitle);

  if (histoList)
    histoList->Add(h);

  return h;
}
