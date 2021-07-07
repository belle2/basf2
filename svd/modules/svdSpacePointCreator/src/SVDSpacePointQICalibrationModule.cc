/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdSpacePointCreator/SpacePointHelperFunctions.h>
#include <svd/modules/svdSpacePointCreator/SVDSpacePointQICalibrationModule.h>
//#include <svd/modules/svdSpacePointCreator/SVDSpacePointCreatorModule.h>

#include <math.h>

using namespace std;
using namespace Belle2;


REG_MODULE(SVDSpacePointQICalibration)

SVDSpacePointQICalibrationModule::SVDSpacePointQICalibrationModule() :
  Module()
{

  setDescription("Generate PDFs used in assigning quality to SpacePoints.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("SVDClusters", m_svdClustersName,
           "SVDCluster collection name", string(""));

  addParam("RecoTracks", m_recoTracksName,
           "RecoTracks collection name", string(""));

  // 2.Modification parameters:
  addParam("NameOfInstance", m_nameOfInstance,
           "allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module", string(""));
  addParam("binSize", m_binSize, "Number of bins in charge distribution.",
           int(50));

  addParam("maxClusterSize", m_maxClusterSize, "Max number of strips the PDF are separated into.",
           int(5));

  addParam("useLegacyNaming", m_useLegacyNaming,
           "Use legacy pdf naming", bool(true));

  addParam("outputFileName", m_outputFileName,
           "Name of output file containing pdfs", std::string("spacePointQICalibration.root"));
}



void SVDSpacePointQICalibrationModule::initialize()
{
  // prepare all store- and relationArrays:
  m_svdClusters.isRequired(m_svdClustersName);
  m_recoTracks.isRequired(m_recoTracksName);



  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (auto& layers : geo.getLayers(VXD::SensorInfoBase::SVD)) {
    for (auto& ladders : geo.getLadders(layers)) {
      for (auto& sensors : geo.getSensors(ladders)) {

        for (int uSize = 1; uSize <= m_maxClusterSize; uSize++) {
          for (int vSize = 1; vSize <= m_maxClusterSize; vSize++) {
            std::string sensorName = "";
            std::string errorStringName = "";


            spPDFName(sensors, uSize, vSize, m_maxClusterSize, sensorName, errorStringName, m_useLegacyNaming);

            std::string signalHistName = sensorName + "signal";
            std::string backgroundHistName = sensorName + "background";

            if (signalHistMap.count(sensorName) == 0) {
              TH2F* sigHist  = new TH2F(signalHistName.c_str(), "", m_binSize, 0, 200000, m_binSize, 0, 200000);
              TH2F* bkgHist = new TH2F(backgroundHistName.c_str(), "", m_binSize, 0, 200000, m_binSize, 0, 200000);

              signalHistMap[sensorName] = sigHist;
              backgroundHistMap[sensorName] = bkgHist;
            }
          }
        }
      }
    }
  }
  TH2F* timeSignal = new TH2F("timeSignal", "", 40, -100, 100, 40, -100, 100);
  TH2F* timeBackground = new TH2F("timeBackground", "", 40, -100, 100, 40, -100, 100);
  TH2F* sizeSignal = new TH2F("sizeSignal", "", 20, 0, 20, 20, 0, 20);
  TH2F* sizeBackground = new TH2F("sizeBackground", "", 20, 0, 20, 20, 0, 20);

  signalHistMap["timeSignal"] = timeSignal;
  signalHistMap["sizeSignal"] = sizeSignal;
  backgroundHistMap["timeBackground"] = timeBackground;
  backgroundHistMap["sizeBackground"] = sizeBackground;

}



void SVDSpacePointQICalibrationModule::event()
{
  for (auto& track : m_recoTracks) {
    if (track.wasFitSuccessful() == 1) {
      if (track.hasSVDHits() == 1) {
        for (auto& svdHit : track.getSVDHitList()) {
          if (svdHit->isUCluster()) {
            for (auto& svdHit2 : track.getSVDHitList()) {
              if (svdHit2->isUCluster() == 0) {
                if (svdHit->getSensorID() == svdHit2->getSensorID()) {
                  std::string pdfName;
                  std::string errorStringName;
                  spPDFName(svdHit->getSensorID(), svdHit->getSize(), svdHit2->getSize(), m_maxClusterSize, pdfName, errorStringName,
                            m_useLegacyNaming);
                  auto sigHist = signalHistMap.at(pdfName);
                  sigHist->Fill(svdHit->getCharge(), svdHit2->getCharge());
                  auto timeSigHist = signalHistMap.at("timeSignal");
                  timeSigHist->Fill(svdHit->getClsTime(), svdHit2->getClsTime());
                  auto sizeSigHist = signalHistMap.at("sizeSignal");
                  sizeSigHist->Fill(svdHit->getSize(), svdHit2->getSize());
                }
              }
            }
          }
        }
      }
    }
  }


  for (auto& uCluster : m_svdClusters) {
    if (uCluster.isUCluster() == 1) {
      for (auto& vCluster : m_svdClusters) {
        if (vCluster.isUCluster() == 0) {
          if (uCluster.getSensorID() == vCluster.getSensorID()) {
            std::string pdfName;
            std::string errorStringName;
            spPDFName(uCluster.getSensorID(), uCluster.getSize(), vCluster.getSize(), m_maxClusterSize, pdfName, errorStringName,
                      m_useLegacyNaming);
            auto bkgHist = backgroundHistMap.at(pdfName);
            bkgHist->Fill(uCluster.getCharge(), vCluster.getCharge());
            auto timeBkgHist = backgroundHistMap.at("timeBackground");
            timeBkgHist->Fill(uCluster.getClsTime(), vCluster.getClsTime());
            auto sizeBkgHist = backgroundHistMap.at("sizeBackground");
            sizeBkgHist->Fill(uCluster.getSize(), vCluster.getSize());
          }
        }
      }
    }
  }
}



void SVDSpacePointQICalibrationModule::terminate()
{
  //Open rootfile
  TFile* f = new TFile(m_outputFileName.c_str(), "RECREATE");
  f->cd();
  std::vector<std::string> usedSensors;
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  for (auto& layers : geo.getLayers(VXD::SensorInfoBase::SVD)) {
    for (auto& ladders : geo.getLadders(layers)) {
      for (auto& sensors : geo.getSensors(ladders)) {
        for (int uSize = 1; uSize <= m_maxClusterSize; uSize++) {
          for (int vSize = 1; vSize <= m_maxClusterSize; vSize++) {
            std::string sensorName;
            std::string errorName;


            spPDFName(sensors, uSize, vSize, m_maxClusterSize, sensorName, errorName, m_useLegacyNaming);

            if (std::find(usedSensors.begin(), usedSensors.end(), sensorName.c_str()) == usedSensors.end()) {
              usedSensors.push_back(sensorName);
              TH2F* probHist  = new TH2F(sensorName.c_str(), "", m_binSize, 0, 200000, m_binSize, 0, 200000);
              TH2F* errorHist  = new TH2F(errorName.c_str(), "", m_binSize, 0, 200000, m_binSize, 0, 200000);
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
  TH2F* timeProb = new TH2F("timeProb", "", 40, -100, 100, 40, -100, 100);
  TH2F* timeError = new TH2F("timeError", "", 40, -100, 100, 40, -100, 100);
  TH2F* sizeProb = new TH2F("sizeProb", "", 20, 0, 20, 20, 0, 20);
  TH2F* sizeError = new TH2F("sizeError", "", 20, 0, 20, 20, 0, 20);
  calculateProb(signalHistMap["timeSignal"], backgroundHistMap["timeBackground"], timeProb);
  calculateError(signalHistMap["timeSignal"], backgroundHistMap["timeBackground"], timeError);
  calculateProb(signalHistMap["sizeSignal"], backgroundHistMap["sizeBackground"], sizeProb);
  calculateError(signalHistMap["sizeSignal"], backgroundHistMap["sizeBackground"], sizeError);
  timeProb->Write();
  timeError->Write();
  sizeProb->Write();
  sizeError->Write();

  f->Close();
}


void SVDSpacePointQICalibrationModule::calculateProb(TH2F* signal, TH2F* background, TH2F* probability)
{
  signal->Divide(background);
  probability->Add(signal);

}
void SVDSpacePointQICalibrationModule::calculateError(TH2F* signal, TH2F* background, TH2F* error)
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
