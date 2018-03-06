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
  addParam("trackListName", m_trackListName, "track list name", std::string(""));
  addParam("shaperDigitsName", m_shapersListName, "SVDShaperDigits list name", std::string(""));
  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDLatencyCalibration_output.root"));


}

void SVDLatencyCalibrationModule::initialize()
{
  //  m_tracks.isRequired(); //for data taking
  m_digits.isRequired(m_shapersListName);
  m_truehits.isRequired();

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  m_histoList = new TList;

  TString NameOfHisto = "";
  TString TitleOfHisto = "";

  NameOfHisto = "trueHit";
  TitleOfHisto = "number  of TrueHits per cosmic ray";
  h_trueHits = createHistogram1D(NameOfHisto, TitleOfHisto, 15, -0.5, 14.5, "# TrueHits", m_histoList);

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



  NameOfHisto = "clSizeU";
  TitleOfHisto = "Cluster Size - U side";
  h_clSizeU = createHistogram1D(NameOfHisto, TitleOfHisto, 21, -0.5, 20.5, "cluster size", m_histoList);

  NameOfHisto = "clSizeV";
  TitleOfHisto = "Cluster Size - V side";
  h_clSizeV = createHistogram1D(NameOfHisto, TitleOfHisto, 21, -0.5, 20.5, "cluster size", m_histoList);

  NameOfHisto = "clChargeU";
  TitleOfHisto = "Cluster Charge - U side";
  h_clChargeU = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "cluster charge [e-]", m_histoList);

  NameOfHisto = "clChargeUL3fw";
  h_clChargeU_L3fw = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "cluster charge [e-]", m_histoList);
  NameOfHisto = "clChargeUL3bw";
  h_clChargeU_L3bw = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "cluster charge [e-]", m_histoList);
  NameOfHisto = "clChargeUL4bw";
  h_clChargeU_L4bw = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "cluster charge [e-]", m_histoList);
  NameOfHisto = "clChargeUL5bw";
  h_clChargeU_L5bw = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "cluster charge [e-]", m_histoList);
  NameOfHisto = "clChargeUL6bw";
  h_clChargeU_L6bw = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "cluster charge [e-]", m_histoList);


  NameOfHisto = "clChargeV";
  TitleOfHisto = "Cluster Charge - V side";
  h_clChargeV = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "cluster charge [e-]", m_histoList);

  NameOfHisto = "clChargeVL3fw";
  h_clChargeV_L3fw = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "cluster charge [e-]", m_histoList);
  NameOfHisto = "clChargeVL3bw";
  h_clChargeV_L3bw = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "cluster charge [e-]", m_histoList);
  NameOfHisto = "clChargeVL4bw";
  h_clChargeV_L4bw = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "cluster charge [e-]", m_histoList);
  NameOfHisto = "clChargeVL5bw";
  h_clChargeV_L5bw = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "cluster charge [e-]", m_histoList);
  NameOfHisto = "clChargeVL6bw";
  h_clChargeV_L6bw = createHistogram1D(NameOfHisto, TitleOfHisto, 500, 0, 100000, "cluster charge [e-]", m_histoList);


  NameOfHisto = "clTimeU";
  TitleOfHisto = "Cluster Time for clusters of  size > 1 - U side, ";
  h_clTimeU = createHistogram1D(NameOfHisto, TitleOfHisto, 200, -100, 100, "cluster time [ns]", m_histoList);

  NameOfHisto = "clTimeV";
  TitleOfHisto = "Cluster Time for clusters of  size > 1 - V side, ";
  h_clTimeV = createHistogram1D(NameOfHisto, TitleOfHisto, 200, -100, 100, "cluster time [ns]", m_histoList);


  NameOfHisto = "tracks";
  TitleOfHisto = "Number of Tracks";
  h_tracks = createHistogram1D(NameOfHisto, TitleOfHisto, 16, -0.5, 15.5, "# tracks", m_histoList);


}

void SVDLatencyCalibrationModule::event()
{

  //start loop on SVDSHaperDigits
  Belle2::SVDShaperDigit::APVFloatSamples samples;

  /*
  if(m_tracks.getEntries() < 1){
    B2INFO("no track, skipping event");
    return;
  }
  */
  h_trueHits->Fill(m_truehits.getEntries());
  h_tracks->Fill(m_tracks.getEntries());

  for (const SVDCluster& cluster : m_clusters) {

    VxdID sensorID = cluster.getSensorID();

    if (cluster.isUCluster()) {
      h_clSizeU->Fill(cluster.getSize());
      h_clChargeU->Fill(cluster.getCharge());
      if (cluster.getSize() > 1)
        h_clTimeU->Fill(cluster.getClsTime());

      if (sensorID.getLayerNumber() == 3) {
        if (sensorID.getSensorNumber() == 1)
          h_clChargeU_L3fw->Fill(cluster.getCharge());
        else
          h_clChargeU_L3bw->Fill(cluster.getCharge());
      }

      if (sensorID.getLayerNumber() == 4)
        if (sensorID.getSensorNumber() == 3)
          h_clChargeU_L4bw->Fill(cluster.getCharge());

      if (sensorID.getLayerNumber() == 5)
        if (sensorID.getSensorNumber() == 4)
          h_clChargeU_L5bw->Fill(cluster.getCharge());

      if (sensorID.getLayerNumber() == 6)
        if (sensorID.getSensorNumber() == 5)
          h_clChargeU_L6bw->Fill(cluster.getCharge());


    } else {
      h_clSizeV->Fill(cluster.getSize());
      h_clChargeV->Fill(cluster.getCharge());
      if (cluster.getSize() > 1)
        h_clTimeV->Fill(cluster.getClsTime());

      if (sensorID.getLayerNumber() == 3) {
        if (sensorID.getSensorNumber() == 1)
          h_clChargeV_L3fw->Fill(cluster.getCharge());
        else
          h_clChargeV_L3bw->Fill(cluster.getCharge());
      }

      if (sensorID.getLayerNumber() == 4)
        if (sensorID.getSensorNumber() == 3)
          h_clChargeV_L4bw->Fill(cluster.getCharge());

      if (sensorID.getLayerNumber() == 5)
        if (sensorID.getSensorNumber() == 4)
          h_clChargeV_L5bw->Fill(cluster.getCharge());

      if (sensorID.getLayerNumber() == 6)
        if (sensorID.getSensorNumber() == 5)
          h_clChargeV_L6bw->Fill(cluster.getCharge());

    }
  }
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
