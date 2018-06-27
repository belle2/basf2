/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: James Webb                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdClusterQualityEstimator/SVDClusterQualityEstimatorModule.h>

using namespace std;
using namespace Belle2;

REG_MODULE(SVDClusterQualityEstimator)

SVDClusterQualityEstimatorModule::SVDClusterQualityEstimatorModule() : Module()

{
  setDescription("Assignment of probability for a cluster being generated from signal hit.");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDClusters", m_svdClustersName,
           "SVDCluster collection name", string(""));

  addParam("inputPDF", m_inputPDF,
           "Path containing pdf root file", std::string("/data/svd/clusterQICalibration.root"));
}


void SVDClusterQualityEstimatorModule::initialize()
{

  if (m_inputPDF.empty()) {
    B2ERROR("PDF File" << m_inputPDF << "not found");
  } else {
    std::string fullPath = FileSystem::findFile(m_inputPDF);
    if (fullPath.empty()) {
      B2ERROR("PDF file" << m_inputPDF << "not found");
    }
    m_inputPDF = fullPath;
  }

  m_calibrationFile = new TFile(m_inputPDF.c_str(), "READ");

  if (!m_calibrationFile->IsOpen())
    B2FATAL("Couldn't open pdf file:" << m_inputPDF);

  // prepare storeArray
  m_svdClusters.isRequired(m_svdClustersName);

}

void SVDClusterQualityEstimatorModule::event()
{

  for (auto& svdCluster : m_svdClusters) {

    //Pdfs divided up by side (u,v), sensor (l3,barrel,slanted) and cluster size (1-5)
    double charge = svdCluster.getCharge();
    double time = svdCluster.getClsTime();
    int size =  svdCluster.getSize();

    //Pdfs only divided up to size 5-strips
    if (size > 5) size = 5;

    int layerNum =  svdCluster.getSensorID().getLayerNumber();
    int sensorNum =  svdCluster.getSensorID().getSensorNumber();

    std::string sensor = (layerNum == 3) ? "l3" : (sensorNum == 1) ? "trap" : "large";
    std::string side = (svdCluster.isUCluster() == 1) ? "u" : "v";

    std::string probInputNameTemp = side + sensor + std::to_string(size);
    const char* probInputName = probInputNameTemp.c_str();
    std::string errorInputNameTemp = side + sensor + std::to_string(size) + "Error";
    const char* errorInputName = errorInputNameTemp.c_str();

    TH2F* probPDF = 0;
    TH2F* errorPDF = 0;
    m_calibrationFile->GetObject(probInputName, probPDF);
    m_calibrationFile->GetObject(errorInputName, errorPDF);


    int xBin = probPDF->GetXaxis()->FindFixBin(time);
    int yBin = probPDF->GetYaxis()->FindFixBin(charge);

    double signalProb = probPDF->GetBinContent(xBin, yBin);
    double signalProbError = errorPDF->GetBinContent(xBin, yBin);

    svdCluster.setQualityIndicator(signalProb);
    svdCluster.setQualityIndicatorError(signalProbError);
  }

}

void SVDClusterQualityEstimatorModule::terminate()
{
  B2INFO("SVDClusterQualityEstimatorModule::terminate");
  m_calibrationFile->Close();
}
