/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdClusterQualityEstimator/SVDClusterQualityEstimatorModule.h>
#include <svd/modules/svdClusterQualityEstimator/ClusterQualityHelperFunctions.h>
#include <framework/utilities/FileSystem.h>

#include <TH2F.h>

using namespace std;
using namespace Belle2;

REG_MODULE(SVDClusterQualityEstimator)

SVDClusterQualityEstimatorModule::SVDClusterQualityEstimatorModule() : Module()

{
  setDescription("Assignment of probability for a cluster being generated from signal hit.");

  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDClusters", m_svdClustersName,
           "SVDCluster collection name", string(""));

  addParam("useQualityEstimator", m_useQualityEstimator,
           "Standard is true. If turned off clusters will not be assigned a quality.", bool(true));

  addParam("inputPDF", m_inputPDF,
           "Path containing pdf root file", std::string("/data/svd/clusterQICalibration.root"));

  addParam("useLegacyNaming", m_useLegacyNaming,
           "Use old PDF name convention?", bool(true));
}


void SVDClusterQualityEstimatorModule::initialize()
{

  if (m_useQualityEstimator == true) {
    if (m_inputPDF.empty()) {
      B2ERROR("Input PDF filename not set!");
    } else {
      std::string fullPath = FileSystem::findFile(m_inputPDF);
      if (fullPath.empty()) {
        B2ERROR("PDF file:" << m_inputPDF << "not located! Check filename input matches name of PDF file!");
      }
      m_inputPDF = fullPath;
    }

    m_calibrationFile = new TFile(m_inputPDF.c_str(), "READ");

    if (!m_calibrationFile->IsOpen())
      B2FATAL("Couldn't open pdf file:" << m_inputPDF);
  }

  // prepare storeArray
  m_svdClusters.isRequired(m_svdClustersName);

}

void SVDClusterQualityEstimatorModule::event()
{

  if (m_useQualityEstimator == true) {
    for (auto& svdCluster : m_svdClusters) {

      double charge = svdCluster.getCharge();
      double time = svdCluster.getClsTime();

      int pdfEntries = m_calibrationFile->GetListOfKeys()->GetSize();
      int maxSize;
      if (m_useLegacyNaming == 1) {
        maxSize = floor(pdfEntries / 12); // 2(sides)*3(sensorType)*2(prob/error)=12
      } else {
        maxSize = floor(pdfEntries / 688); // 2(sides)*172(sensors)*2(prob/error)=688
      }

      std::string probInputName;
      std::string errorInputName;

      clusterPDFName(svdCluster.getSensorID(), svdCluster.getSize(), svdCluster.isUCluster(), maxSize, probInputName, errorInputName,
                     m_useLegacyNaming);

      TH2F* probPDF = nullptr;
      TH2F* errorPDF = nullptr;
      m_calibrationFile->GetObject(probInputName.c_str(), probPDF);
      m_calibrationFile->GetObject(errorInputName.c_str(), errorPDF);


      int xBin = probPDF->GetXaxis()->FindFixBin(time);
      int yBin = probPDF->GetYaxis()->FindFixBin(charge);

      double signalProb = probPDF->GetBinContent(xBin, yBin);
      double signalProbError = errorPDF->GetBinContent(xBin, yBin);

      svdCluster.setQualityIndicator(signalProb);
      svdCluster.setQualityIndicatorError(signalProbError);
    }
  }
}

void SVDClusterQualityEstimatorModule::terminate()
{
  B2INFO("SVDClusterQualityEstimatorModule::terminate");

  if (m_useQualityEstimator == true) {
    m_calibrationFile->Delete();
  }
}
