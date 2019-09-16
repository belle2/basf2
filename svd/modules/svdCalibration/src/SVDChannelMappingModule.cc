/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Giulia Casarosa, Eugenio Paoloni                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdCalibration/SVDChannelMappingModule.h>
#include <vxd/geometry/GeoCache.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDChannelMapping)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDChannelMappingModule::SVDChannelMappingModule()
  : Module()
  , hInterDictionary(172, [](const Belle2::VxdID & vxdid) {return (size_t)vxdid.getID(); })
, n_events(0)
{
  //Set module properties
  setDescription("SVD Channel Mapping Verification Module");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDChannelMapping.root"));

  addParam("ShaperDigitsName", m_SVDShaperDigitsName,
           "name of the list of SVDShaperDigits", std::string(""));

  addParam("ClustersName", m_SVDClustersName,
           "name of the list of SVDClusters", std::string(""));

  addParam("InterceptsName", m_InterceptsName,
           "name of the list of interceptions", std::string(""));

}



void SVDChannelMappingModule::initialize()
{

  m_histoList_digits = new TList;
  m_histoList_clusters = new TList;

  createHistosDictionaries();

  m_shapers.isRequired(m_SVDShaperDigitsName);
  m_clusters.isRequired(m_SVDClustersName);
  m_Intercepts.isRequired(m_InterceptsName);

  n_events = 0;

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

}

void SVDChannelMappingModule::event()
{

  n_events++;



  for (auto& it : m_Intercepts)
    fillSensorInterHistos(&it);
}

void SVDChannelMappingModule::createHistosDictionaries()
{

  //  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  string name; //name of the histogram
  string title; //title of the histogram
  TH2F* tmp2D; //temporary 2D histo used to set axis title
  TH1F* tmp1D; //temporary 1D histo used to set axis title


  std::set<Belle2::VxdID> svdLayers = m_aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();

  while (itSvdLayers != svdLayers.end()) {

    std::set<Belle2::VxdID> svdLadders = m_aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) {

      std::set<Belle2::VxdID> svdSensors = m_aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();

      while (itSvdSensors != svdSensors.end()) {

        string sensorid = std::to_string(itSvdSensors->getLayerNumber()) + "_" + std::to_string(itSvdSensors->getLadderNumber()) + "_" +
                          std::to_string(itSvdSensors->getSensorNumber());


        // ------ HISTOGRAMS WITH A FILL PER INTERCEPT -------

        // coor U and V
        name = "hCoorU_" + sensorid;
        title = "U coordinate of the extrapolation in U for sensor " + sensorid;
        tmp1D = new TH1F(name.c_str(), title.c_str(), 100, -2.9, 2.9);
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp1D,
        [](TH1 * hPtr, const SVDIntercept * inter) { hPtr->Fill(inter->getCoorU()); }
                                  )
                                )
                               );
        m_histoList_digits->Add(tmp1D);

        name = "hCoorV_" + sensorid;
        title = "V coordinate of the extrapolation in V for sensor " + sensorid;
        tmp1D = new TH1F(name.c_str(), title.c_str(), 100, -6.15, 6.15);
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp1D,
        [](TH1 * hPtr, const SVDIntercept * inter) { hPtr->Fill(inter->getCoorV()); }
                                  )
                                )
                               );
        m_histoList_digits->Add(tmp1D);

        // Intercept U vs V coordinate
        name = "hCoorU_vs_CoorV_" + sensorid;
        title = "U vs V intercept (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 100, -2.9, 2.9, 100, -6.15, 6.15);
        tmp2D->GetXaxis()->SetTitle("intercept U coor (cm)");
        tmp2D->GetYaxis()->SetTitle("intercept V coor (cm)");
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [](TH1 * hPtr, const SVDIntercept * inter) { hPtr->Fill(inter->getCoorU(), inter->getCoorV()); }
                                  )
                                )
                               );
        m_histoList_digits->Add(tmp2D);

        // sigma U and V
        name = "hStatErrU_" + sensorid;
        title = "stat error of the extrapolation in U for sensor " + sensorid;
        tmp1D = new TH1F(name.c_str(), title.c_str(), 100, 0, 0.35);
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp1D,
        [](TH1 * hPtr, const SVDIntercept * inter) { hPtr->Fill(inter->getSigmaU()); }
                                  )
                                )
                               );
        m_histoList_digits->Add(tmp1D);

        name = "hStatErrV_" + sensorid;
        title = "stat error of the extrapolation in V for sensor " + sensorid;
        tmp1D = new TH1F(name.c_str(), title.c_str(), 100, 0, 0.35);
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp1D,
        [](TH1 * hPtr, const SVDIntercept * inter) { hPtr->Fill(inter->getSigmaV()); }
                                  )
                                )
                               );
        m_histoList_digits->Add(tmp1D);

        //---------------
        //digits
        //1D residuals
        name = "hDigitResidU_" + sensorid;
        title = "U residuals = intercept - digit,  for sensor " + sensorid;
        tmp1D = new TH1F(name.c_str(), title.c_str(), 1000, -2.9, 2.9);
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp1D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {
          StoreArray<SVDShaperDigit> SVDShaperDigits(this->m_SVDShaperDigitsName);

          for (auto& it : SVDShaperDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              if (it.isUStrip()) {
                const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
                hPtr->Fill(inter->getCoorU() - aSensorInfo.getUCellPosition(it.getCellID()));
              }
            }
        }
                                  )
                                )
                               );
        m_histoList_digits->Add(tmp1D);


        name = "hDigitResidV_" + sensorid;
        title = "V residuals = intercept - digit,  for sensor " + sensorid;
        tmp1D = new TH1F(name.c_str(), title.c_str(), 1000, -6.15, 6.15);
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp1D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {
          StoreArray<SVDShaperDigit> SVDShaperDigits(this->m_SVDShaperDigitsName);

          for (auto& it : SVDShaperDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              if (!it.isUStrip()) {
                const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
                hPtr->Fill(inter->getCoorV() - aSensorInfo.getVCellPosition(it.getCellID()));
              }
            }
        }
                                  )
                                )
                               );
        m_histoList_digits->Add(tmp1D);

        //residual U,V vs coordinate U,V
        name = "hDigitResidU_vs_DigitU_" + sensorid;
        title = "U residual (cm) vs digit U (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -2.9, 2.9, 1000, -2.9, 2.9);
        tmp2D->GetYaxis()->SetTitle("U resid (cm)");
        tmp2D->GetXaxis()->SetTitle("U digit (cm)");
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {
          StoreArray<SVDShaperDigit> SVDShaperDigits(this->m_SVDShaperDigitsName);

          for (auto& it : SVDShaperDigits)
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && it.isUStrip()) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              double resid = inter->getCoorU() - aSensorInfo.getUCellPosition(it.getCellID());
              hPtr->Fill(aSensorInfo.getUCellPosition(it.getCellID()), resid);
            }
        }
                                  )
                                )
                               );
        m_histoList_digits->Add(tmp2D);

        name = "hDigitResidV_vs_DigitV_" + sensorid;
        title = "V residual (cm) vs digit V (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -6.15, 6.15, 1000, -6.15, 6.15);
        tmp2D->GetYaxis()->SetTitle("V resid (cm)");
        tmp2D->GetXaxis()->SetTitle("V digit (cm)");
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {
          StoreArray<SVDShaperDigit> SVDShaperDigits(this->m_SVDShaperDigitsName);

          for (auto& it : SVDShaperDigits)
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && (!it.isUStrip())) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              double resid = inter->getCoorV() - aSensorInfo.getVCellPosition(it.getCellID());
              hPtr->Fill(aSensorInfo.getVCellPosition(it.getCellID()), resid);
            }
        }
                                  )
                                )
                               );
        m_histoList_digits->Add(tmp2D);


        // scatter plot: U,V intercept in cm VS U,V cell position
        name = "hCoorU_vs_DigitU_" + sensorid;
        title = "U intercept (cm) vs U digit (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -2.9, 2.9, 1000, -2.9, 2.9);
        tmp2D->GetXaxis()->SetTitle("intercept U coor (cm)");
        tmp2D->GetYaxis()->SetTitle("digit U coor (cm)");
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {
          StoreArray<SVDShaperDigit> SVDShaperDigits(this->m_SVDShaperDigitsName);

          for (auto& it : SVDShaperDigits)
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && (it.isUStrip())) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              hPtr->Fill(inter->getCoorU(), aSensorInfo.getUCellPosition(it.getCellID()));
            }
        }
                                  )
                                )
                               );
        m_histoList_digits->Add(tmp2D);

        name = "hCoorV_vs_DigitV_" + sensorid;
        title = "V intercept (cm) vs V digit (ID) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -6.15, 6.15, 1000, -6.15, 6.15);
        tmp2D->GetXaxis()->SetTitle("intercept V coor (cm)");
        tmp2D->GetYaxis()->SetTitle("digi V coor (cm)");
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {
          StoreArray<SVDShaperDigit> SVDShaperDigits(this->m_SVDShaperDigitsName);

          for (auto& it : SVDShaperDigits) {
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && (!it.isUStrip())) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              hPtr->Fill(inter->getCoorV(), aSensorInfo.getVCellPosition(it.getCellID()));
            }
          }
        }
                                  )
                                )
                               );
        m_histoList_digits->Add(tmp2D);

        //-----------------
        //clusters
        //1D residuals
        name = "hClusterResidU_" + sensorid;
        title = "U residuals = intercept - cluster,  for sensor " + sensorid;
        tmp1D = new TH1F(name.c_str(), title.c_str(), 1000, -2.9, 2.9);
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp1D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {
          StoreArray<SVDCluster> SVDClusters(this->m_SVDClustersName);

          for (auto& it : SVDClusters)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              if (it.isUCluster()) {
                hPtr->Fill(inter->getCoorU() - it.getPosition());
              }
            }
        }
                                  )
                                )
                               );
        m_histoList_clusters->Add(tmp1D);


        name = "hClusterResidV_" + sensorid;
        title = "V residuals = intercept - cluster,  for sensor " + sensorid;
        tmp1D = new TH1F(name.c_str(), title.c_str(), 1000, -6.15, 6.15);
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp1D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {
          StoreArray<SVDCluster> SVDClusters(this->m_SVDClustersName);

          for (auto& it : SVDClusters)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              if (!it.isUCluster()) {
                hPtr->Fill(inter->getCoorV() - it.getPosition());
              }
            }
        }
                                  )
                                )
                               );
        m_histoList_clusters->Add(tmp1D);

        //residual U,V vs coordinate U,V
        name = "hClusterResidU_vs_ClusterU_" + sensorid;
        title = "U residual (cm) vs cluster U (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -2.9, 2.9, 1000, -2.9, 2.9);
        tmp2D->GetYaxis()->SetTitle("U resid (cm)");
        tmp2D->GetXaxis()->SetTitle("U cluster (cm)");
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {
          StoreArray<SVDCluster> SVDClusters(this->m_SVDClustersName);

          for (auto& it : SVDClusters)
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && it.isUCluster()) {
              double resid = inter->getCoorU() - it.getPosition();
              hPtr->Fill(it.getPosition(), resid);
            }
        }
                                  )
                                )
                               );
        m_histoList_clusters->Add(tmp2D);

        name = "hClusterResidV_vs_ClusterV_" + sensorid;
        title = "V residual (cm) vs cluster V (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -6.15, 6.15, 1000, -6.15, 6.15);
        tmp2D->GetYaxis()->SetTitle("V resid (cm)");
        tmp2D->GetXaxis()->SetTitle("V cluster (cm)");
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {
          StoreArray<SVDCluster> SVDClusters(this->m_SVDClustersName);

          for (auto& it : SVDClusters)
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && (!it.isUCluster())) {
              double resid = inter->getCoorV() - it.getPosition();
              hPtr->Fill(it.getPosition(), resid);
            }
        }
                                  )
                                )
                               );
        m_histoList_clusters->Add(tmp2D);


        // scatter plot: U,V intercept in cm VS U,V cell position
        name = "hCoorU_vs_ClusterU_" + sensorid;
        title = "U intercept (cm) vs U cluster (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -2.9, 2.9, 1000, -2.9, 2.9);
        tmp2D->GetXaxis()->SetTitle("intercept U coor (cm)");
        tmp2D->GetYaxis()->SetTitle("cluster U coor (cm)");
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {
          StoreArray<SVDCluster> SVDClusters(this->m_SVDClustersName);

          for (auto& it : SVDClusters)
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && (it.isUCluster())) {
              hPtr->Fill(inter->getCoorU(), it.getPosition());
            }
        }
                                  )
                                )
                               );
        m_histoList_clusters->Add(tmp2D);

        name = "hCoorV_vs_ClusterV_" + sensorid;
        title = "V intercept (cm) vs V cluster (ID) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -6.15, 6.15, 1000, -6.15, 6.15);
        tmp2D->GetXaxis()->SetTitle("intercept V coor (cm)");
        tmp2D->GetYaxis()->SetTitle("cluster V coor (cm)");
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {
          StoreArray<SVDCluster> SVDClusters(this->m_SVDClustersName);

          for (auto& it : SVDClusters) {
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && (!it.isUCluster())) {
              hPtr->Fill(inter->getCoorV(), it.getPosition());
            }
          }
        }
                                  )
                                )
                               );
        m_histoList_clusters->Add(tmp2D);

        itSvdSensors++;
      }
      itSvdLadders++;
    }
    itSvdLayers++;
  }

}

void SVDChannelMappingModule::fillSensorInterHistos(const SVDIntercept* inter)
{

  auto its = hInterDictionary.equal_range(inter->getSensorID());

  for (auto it = its.first; it != its.second; ++it) {
    InterHistoAndFill aInterHistoAndFill = it->second;
    aInterHistoAndFill.second(aInterHistoAndFill.first, inter);
  }

}

void SVDChannelMappingModule::terminate()
{

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();


    TDirectory* oldDir = gDirectory;
    TObject* obj;

    TDirectory* dir_digits = oldDir->mkdir("digits");
    dir_digits->cd();
    TIter nextH_digits(m_histoList_digits);
    while ((obj = nextH_digits()))
      obj->Write();

    TDirectory* dir_clusters = oldDir->mkdir("clusters");
    dir_clusters->cd();
    TIter nextH_clusters(m_histoList_clusters);
    while ((obj = nextH_clusters()))
      obj->Write();

  }
}
