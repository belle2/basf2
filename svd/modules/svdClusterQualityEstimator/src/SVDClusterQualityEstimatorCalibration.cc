/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdClusterQualityEstimator/ClusterQualityHelperFunctions.h>
#include <svd/modules/svdClusterQualityEstimator/SVDClusterQualityEstimatorCalibrationModule.h>

#include <TFile.h>

#include <vector>
#include <math.h>

using namespace std;
using namespace Belle2;


REG_MODULE(SVDClusterQualityEstimatorCalibration)

SVDClusterQualityEstimatorCalibrationModule::SVDClusterQualityEstimatorCalibrationModule() :
  Module()
{

  setDescription("Generate PDFs used in assigning quality to clusterss.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("SVDClusters", m_svdClustersName,
           "SVDCluster collection name", string(""));

  addParam("RecoTracks", m_recoTracksName,
           "RecoTracks collection name", string(""));

  // 2.Modification parameters:
  addParam("NameOfInstance", m_nameOfInstance,
           "allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module", string(""));
  addParam("binSizeCharge", m_binSizeCharge, "Number of bins in charge distribtuion.",
           int(100));
  addParam("binSizeTime", m_binSizeTime, "Number of bins in charge distribtuion.",
           int(40));

  addParam("maxClusterSize", m_maxClusterSize, "Max number of strips the PDF are separated into.",
           int(5));


  addParam("outputFileName", m_outputFileName,
           "Name of output file containing pdfs", std::string("clusterQICalibration.root"));

  addParam("useLegacyNaming", m_useLegacyNaming,
           "Use old PDF name convention?", bool(true));
}



void SVDClusterQualityEstimatorCalibrationModule::initialize()
{
  // prepare all store- and relationArrays:
  m_svdClusters.isRequired(m_svdClustersName);
  m_recoTracks.isRequired(m_recoTracksName);



  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (auto& layers : geo.getLayers(VXD::SensorInfoBase::SVD)) {
    for (auto& ladders : geo.getLadders(layers)) {
      for (auto& sensors : geo.getSensors(ladders)) {

        for (int side = 0; side <= 1; side++) {
          for (int size = 1; size <= m_maxClusterSize; size++) {
            std::string sensorName;
            std::string errorNameString;


            clusterPDFName(sensors, size, side, m_maxClusterSize, sensorName, errorNameString, m_useLegacyNaming);

            std::string signalHistName = sensorName + "signal";
            std::string backgroundHistName = sensorName + "background";

            if (signalHistMap.count(sensorName) == 0) {
              TH2F* sigHist  = new TH2F(signalHistName.c_str(), "", m_binSizeTime, -100, 100, m_binSizeCharge, 0, 200000);
              TH2F* bkgHist = new TH2F(backgroundHistName.c_str(), "", m_binSizeTime, -100, 100, m_binSizeCharge, 0, 200000);

              signalHistMap[sensorName] = sigHist;
              backgroundHistMap[sensorName] = bkgHist;
            }
          }
        }
      }
    }
  }



}



void SVDClusterQualityEstimatorCalibrationModule::event()
{
  for (auto& track : m_recoTracks) {
    if (track.wasFitSuccessful() == 1) {
      if (track.hasSVDHits() == 1) {
        for (auto& svdHit : track.getSVDHitList()) {
          std::string pdfName;
          std::string errorNameString;
          clusterPDFName(svdHit->getSensorID(), svdHit->getSize(), svdHit->isUCluster(), m_maxClusterSize, pdfName, errorNameString,
                         m_useLegacyNaming);

          auto sigHist = signalHistMap.at(pdfName);
          sigHist->Fill(svdHit->getClsTime(), svdHit->getCharge());


        }
      }
    }
  }


  for (auto& cluster : m_svdClusters) {
    std::string pdfName;
    std::string errorNameString;
    clusterPDFName(cluster.getSensorID(), cluster.getSize(), cluster.isUCluster(), m_maxClusterSize, pdfName, errorNameString,
                   m_useLegacyNaming);

    auto bkgHist = backgroundHistMap.at(pdfName);
    bkgHist->Fill(cluster.getClsTime(), cluster.getCharge());
  }
}



void SVDClusterQualityEstimatorCalibrationModule::terminate()
{
  //Open rootfile
  TFile* f = new TFile(m_outputFileName.c_str(), "RECREATE");
  f->cd();

  std::vector<std::string> usedSensors; //Store names to avoid double counting

  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (auto& layers : geo.getLayers(VXD::SensorInfoBase::SVD)) {
    for (auto& ladders : geo.getLadders(layers)) {
      for (auto& sensors : geo.getSensors(ladders)) {
        for (int side = 0; side <= 1; side++) {
          for (int size = 1; size <= m_maxClusterSize; size++) {
            std::string sensorName;
            std::string errorName;

            clusterPDFName(sensors, size, side, m_maxClusterSize, sensorName, errorName, m_useLegacyNaming);

            if (std::find(usedSensors.begin(), usedSensors.end(), sensorName.c_str()) == usedSensors.end()) {
              usedSensors.push_back(sensorName);
              TH2F* probHist  = new TH2F(sensorName.c_str(), "", m_binSizeTime, -100, 100, m_binSizeCharge, 0, 200000);
              TH2F* errorHist  = new TH2F(errorName.c_str(), "", m_binSizeTime, -100, 100, m_binSizeCharge, 0, 200000);
              calculateProb(signalHistMap[sensorName], backgroundHistMap[sensorName], probHist);
              calculateError(signalHistMap[sensorName], backgroundHistMap[sensorName], errorHist);

              probHist->Write();
              errorHist->Write();
            }
          }
        }
      }
    }
  }
  f->Close();
}


void SVDClusterQualityEstimatorCalibrationModule::calculateProb(TH2F* signal, TH2F* background, TH2F* probability)
{
  signal->Divide(background);
  probability->Add(signal);

}
void SVDClusterQualityEstimatorCalibrationModule::calculateError(TH2F* signal, TH2F* background, TH2F* error)
{
  int imax = signal->GetXaxis()->GetNbins();
  int jmax = signal->GetYaxis()->GetNbins();
  for (int i = 1; i <= imax; i++) {
    for (int j = 1; j <= jmax; j++) {
      int bkg = background->GetBinContent(i, j);
      int sig = signal->GetBinContent(i, j);
      double var = ((sig + 1) * (sig + 2)) / ((bkg + 2) * (bkg + 3)) -
                   ((sig + 1) * (sig + 1)) / ((bkg + 2) * (bkg + 2));
      double err = sqrt(var);
      error->SetBinContent(i, j, err);
    }
  }
}
