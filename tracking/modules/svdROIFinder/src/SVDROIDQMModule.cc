/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/svdROIFinder/SVDROIDQMModule.h>

#include <TDirectory.h>
#include <TH2F.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDROIDQM);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDROIDQMModule::SVDROIDQMModule()
  : HistoModule()
  , m_InterDir(nullptr)
  , hInterDictionary(172, [](const Belle2::VxdID & vxdid) {return (size_t)vxdid.getID(); })
, m_numModules(0)
, hnInter(nullptr)
, harea(nullptr)
, hredFactor(nullptr)
, hCellU(nullptr)
, hCellV(nullptr)
, n_events(0)
{
  //Set module properties
  setDescription("Monitor of the  ROIs creation on HLT");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDShaperDigitsName", m_SVDShaperDigitsName,
           "name of the list of SVDShaperDigits", std::string(""));
  addParam("SVDRecoDigitsName", m_SVDRecoDigitsName,
           "name of the list of SVDRecoDigits", std::string(""));
  addParam("SVDClustersName", m_SVDClustersName,
           "name of the list of SVDClusters", std::string(""));

  addParam("InterceptsName", m_InterceptsName,
           "name of the list of interceptions", std::string(""));

  addParam("specificLayer", m_specificLayer,
           "Layer number, if you want the plots only for a specific SVD layer. If it is not a SVD layer (3, 4, 5, 6) than the plots for all SVD layers are produced. Default is (-1), i.e. plots for all SVD layers are produced.",
           m_specificLayer);

  addParam("plotRecoDigits", m_plotRecoDigits,
           "Set true to produce the plots for RecoDigits (false by default)", m_plotRecoDigits);

}

void SVDROIDQMModule::defineHisto()
{

  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  TDirectory* roiDir = oldDir->mkdir("SVDROIs");
  m_InterDir = roiDir->mkdir("intercept");

  hCellU  = new TH1F("hCellU", "CellID U", 769, -0.5, 768.5);
  hCellU->GetXaxis()->SetTitle("U cell ID");
  hCellV  = new TH1F("hCellV", "CellID V", 769, -0.5, 768.5);
  hCellV->GetXaxis()->SetTitle("V cell ID");

  m_InterDir->cd();
  hnInter  = new TH1F("hnInter", "number of intercepts", 100, 0, 100);

  createHistosDictionaries();

  oldDir->cd();

}

void SVDROIDQMModule::initialize()
{
  REG_HISTOGRAM

  m_SVDShaperDigits.isOptional(m_SVDShaperDigitsName);
  m_SVDRecoDigits.isOptional(m_SVDRecoDigitsName);
  m_SVDClusters.isOptional(m_SVDClustersName);
  m_Intercepts.isRequired(m_InterceptsName);

  n_events = 0;


}

void SVDROIDQMModule::event()
{

  n_events++;

  for (auto& it : m_SVDShaperDigits)
    if (it.isUStrip())
      hCellU->Fill(it.getCellID());
    else
      hCellV->Fill(it.getCellID());

  hnInter->Fill(m_Intercepts.getEntries());

  for (auto& it : m_Intercepts)
    fillSensorInterHistos(&it);

}

void SVDROIDQMModule::createHistosDictionaries()
{

  //  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  std::string name; //name of the histogram
  std::string title; //title of the histogram
  TH2F* tmp2D; //temporary 2D histo used to set axis title
  TH1F* tmp1D; //temporary 1D histo used to set axis title

  m_numModules = 0;

  std::set<Belle2::VxdID> svdLayers = m_geoCache.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();

  if (m_specificLayer >= 3 && m_specificLayer <= 6) {
    B2INFO("Producing plots for layer: " << m_specificLayer);
    svdLayers.clear();
    svdLayers.insert(Belle2::VxdID(m_specificLayer, 0, 0));
    itSvdLayers = svdLayers.begin();
  } else {
    B2INFO("No specific SVD layer (3,4,5,6) selected (m_specificLayer = " << m_specificLayer <<
           "). Producing plots for all SVD layers.");
  }

  while (itSvdLayers != svdLayers.end()) {

    std::set<Belle2::VxdID> svdLadders = m_geoCache.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) {

      std::set<Belle2::VxdID> svdSensors = m_geoCache.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();

      while (itSvdSensors != svdSensors.end()) {

        m_numModules++; //counting the total number of modules

        std::string sensorid = std::to_string(itSvdSensors->getLayerNumber()) + "_" + std::to_string(
                                 itSvdSensors->getLadderNumber()) + "_" +
                               std::to_string(itSvdSensors->getSensorNumber());


        // ------ HISTOGRAMS WITH A FILL PER INTERCEPT -------
        m_InterDir->cd();

        // coor U and V
        name = "hCoorU_" + sensorid;
        title = "U coordinate of the extrapolation in U for sensor " + sensorid;
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    new TH1F(name.c_str(), title.c_str(), 100, -5, 5),
        [](TH1 * hPtr, const SVDIntercept * inter) { hPtr->Fill(inter->getCoorU()); }
                                  )
                                )
                               );

        name = "hCoorV_" + sensorid;
        title = "V coordinate of the extrapolation in V for sensor " + sensorid;
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    new TH1F(name.c_str(), title.c_str(), 100, -5, 5),
        [](TH1 * hPtr, const SVDIntercept * inter) { hPtr->Fill(inter->getCoorV()); }
                                  )
                                )
                               );

        // Intercept U vs V coordinate
        name = "hCoorU_vs_CoorV_" + sensorid;
        title = "U vs V intercept (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 100, -5, 5, 100, -5, 5);
        tmp2D->GetXaxis()->SetTitle("intercept U coor (cm)");
        tmp2D->GetYaxis()->SetTitle("intercept V coor (cm)");
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [](TH1 * hPtr, const SVDIntercept * inter) { hPtr->Fill(inter->getCoorU(), inter->getCoorV()); }
                                  )
                                )
                               );


        // sigma U and V
        name = "hStatErrU_" + sensorid;
        title = "stat error of the extrapolation in U for sensor " + sensorid;
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    new TH1F(name.c_str(), title.c_str(), 100, 0, 0.35),
        [](TH1 * hPtr, const SVDIntercept * inter) { hPtr->Fill(inter->getSigmaU()); }
                                  )
                                )
                               );
        name = "hStatErrV_" + sensorid;
        title = "stat error of the extrapolation in V for sensor " + sensorid;
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    new TH1F(name.c_str(), title.c_str(), 100, 0, 0.35),
        [](TH1 * hPtr, const SVDIntercept * inter) { hPtr->Fill(inter->getSigmaV()); }
                                  )
                                )
                               );

        //1D residuals
        name = "hResidU_" + sensorid;
        title = "U residuals = intercept - digit,  for sensor " + sensorid;
        tmp1D = new TH1F(name.c_str(), title.c_str(), 1000, -5, 5);
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp1D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {

          for (auto& it : this->m_SVDShaperDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              if (it.isUStrip()) {
                const VXD::SensorInfoBase& aSensorInfo = m_geoCache.getSensorInfo(it.getSensorID());
                hPtr->Fill(inter->getCoorU() - aSensorInfo.getUCellPosition(it.getCellID()));
              }
            }
        }
                                  )
                                )
                               );

        name = "hResidV_" + sensorid;
        title = "V residuals = intercept - digit,  for sensor " + sensorid;
        tmp1D = new TH1F(name.c_str(), title.c_str(), 1000, -5, 5);
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp1D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {

          for (auto& it : this->m_SVDShaperDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              if (!it.isUStrip()) {
                const VXD::SensorInfoBase& aSensorInfo = m_geoCache.getSensorInfo(it.getSensorID());
                hPtr->Fill(inter->getCoorV() - aSensorInfo.getVCellPosition(it.getCellID()));
              }
            }
        }
                                  )
                                )
                               );


        //residual U,V vs coordinate U,V
        name = "hResidU_vs_CoorU_" + sensorid;
        title = "U residual (cm) vs coor U (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -5, 5, 1000, -5, 5);
        tmp2D->GetYaxis()->SetTitle("U resid (cm)");
        tmp2D->GetXaxis()->SetTitle("U coor (cm)");
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {

          for (auto& it : this->m_SVDShaperDigits)
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && it.isUStrip()) {
              const VXD::SensorInfoBase& aSensorInfo = m_geoCache.getSensorInfo(it.getSensorID());
              double resid = inter->getCoorU() - aSensorInfo.getUCellPosition(it.getCellID());
              hPtr->Fill(inter->getCoorU(), resid);
            }
        }
                                  )
                                )
                               );

        name = "hResidV_vs_CoorV_" + sensorid;
        title = "V residual (cm) vs coor V (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -5, 5, 1000, -5, 5);
        tmp2D->GetYaxis()->SetTitle("V resid (cm)");
        tmp2D->GetXaxis()->SetTitle("V coor (cm)");
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {

          for (auto& it : this->m_SVDShaperDigits)
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && (!it.isUStrip())) {
              const VXD::SensorInfoBase& aSensorInfo = m_geoCache.getSensorInfo(it.getSensorID());
              double resid = inter->getCoorV() - aSensorInfo.getVCellPosition(it.getCellID());
              hPtr->Fill(inter->getCoorV(), resid);
            }
        }
                                  )
                                )
                               );


        // RecoDigits
        //residual vs charge
        if (m_plotRecoDigits) {
          name = "hResidU_vs_charge_" + sensorid;
          title = "U residual (cm) vs charge " + sensorid;
          tmp2D = new TH2F(name.c_str(), title.c_str(), 250, 0, 250, 100, -5, 5);
          tmp2D->GetYaxis()->SetTitle("U resid (cm)");
          tmp2D->GetXaxis()->SetTitle("charge");
          hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itSvdSensors,
                                    InterHistoAndFill(
                                      tmp2D,
          [this](TH1 * hPtr, const SVDIntercept * inter) {

            for (auto& it : this->m_SVDRecoDigits)
              if (((int)it.getSensorID() == (int)inter->getSensorID()) && (it.isUStrip())) {
                const VXD::SensorInfoBase& aSensorInfo = m_geoCache.getSensorInfo(it.getSensorID());
                double resid = inter->getCoorU() - aSensorInfo.getUCellPosition(it.getCellID());
                hPtr->Fill(it.getCharge(), resid);
              }
          }
                                    )
                                  )
                                 );

          name = "hResidV_vs_charge_" + sensorid;
          title = "V residual (cm) vs charge " + sensorid;
          tmp2D = new TH2F(name.c_str(), title.c_str(), 250, 0, 250, 100, -5, 5);
          tmp2D->GetYaxis()->SetTitle("V resid (cm)");
          tmp2D->GetXaxis()->SetTitle("charge");
          hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itSvdSensors,
                                    InterHistoAndFill(
                                      tmp2D,
          [this](TH1 * hPtr, const SVDIntercept * inter) {

            for (auto& it : this->m_SVDRecoDigits)
              if (((int)it.getSensorID() == (int)inter->getSensorID()) && (!it.isUStrip())) {
                const VXD::SensorInfoBase& aSensorInfo = m_geoCache.getSensorInfo(it.getSensorID());
                double resid = inter->getCoorV() - aSensorInfo.getVCellPosition(it.getCellID());
                hPtr->Fill(it.getCharge(), resid);
              }
          }
                                    )
                                  )
                                 );
        }

        // 1D residual for clusters
        name = "hClusterResidU_" + sensorid;
        title = "Cluster U residuals = intercept - cluster,  for sensor " + sensorid;
        tmp1D = new TH1F(name.c_str(), title.c_str(), 1000, -5, 5);
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp1D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {

          for (auto& it : this->m_SVDClusters)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              if (it.isUCluster()) {
                hPtr->Fill(inter->getCoorU() - it.getPosition(inter->getCoorV()));
              }
            }
        }
                                  )
                                )
                               );

        name = "hClusterResidV_" + sensorid;
        title = "Cluster V residuals = intercept - cluster,  for sensor " + sensorid;
        tmp1D = new TH1F(name.c_str(), title.c_str(), 1000, -5, 5);
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp1D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {

          for (auto& it : this->m_SVDClusters)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              if (!it.isUCluster()) {
                hPtr->Fill(inter->getCoorV() - it.getPosition());
              }
            }
        }
                                  )
                                )
                               );

        //residual U,V vs coordinate U,V for clusters
        name = "hClusterResidU_vs_CoorU_" + sensorid;
        title = "Cluster U residual (cm) vs coor U (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -5, 5, 1000, -5, 5);
        tmp2D->GetYaxis()->SetTitle("Cluster U resid (cm)");
        tmp2D->GetXaxis()->SetTitle("U coor (cm)");
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {

          for (auto& it : this->m_SVDClusters)
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && it.isUCluster()) {
              double resid = inter->getCoorU() - it.getPosition(inter->getCoorV());
              hPtr->Fill(inter->getCoorU(), resid);
            }
        }
                                  )
                                )
                               );

        name = "hClusterResidV_vs_CoorV_" + sensorid;
        title = "Cluster V residual (cm) vs coor V (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -5, 5, 1000, -5, 5);
        tmp2D->GetYaxis()->SetTitle("Cluster V resid (cm)");
        tmp2D->GetXaxis()->SetTitle("V coor (cm)");
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {

          for (auto& it : this->m_SVDClusters)
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && (!it.isUCluster())) {
              double resid = inter->getCoorV() - it.getPosition();
              hPtr->Fill(inter->getCoorV(), resid);
            }
        }
                                  )
                                )
                               );


        //residual vs charge for clusters
        name = "hClusterResidU_vs_charge_" + sensorid;
        title = "Cluster U residual (cm) vs charge " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 250, 0, 250, 100, -5, 5);
        tmp2D->GetYaxis()->SetTitle("U resid (cm)");
        tmp2D->GetXaxis()->SetTitle("charge (ke-)");
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {

          for (auto& it : this->m_SVDClusters)
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && (it.isUCluster())) {
              double resid = inter->getCoorU() - it.getPosition(inter->getCoorV());
              hPtr->Fill(it.getCharge() / 1000., resid);
            }
        }
                                  )
                                )
                               );

        name = "hClusterResidV_vs_charge_" + sensorid;
        title = "Cluster V residual (cm) vs charge " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 250, 0, 250, 100, -5, 5);
        tmp2D->GetYaxis()->SetTitle("Cluster V resid (cm)");
        tmp2D->GetXaxis()->SetTitle("charge (ke-)");
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {

          for (auto& it : this->m_SVDClusters)
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && (!it.isUCluster())) {
              double resid = inter->getCoorV() - it.getPosition();
              hPtr->Fill(it.getCharge() / 1000., resid);
            }
        }
                                  )
                                )
                               );


        // scatter plot: U,V intercept in cm VS U,V cell position
        name = "hCoorU_vs_UDigit_" + sensorid;
        title = "U intercept (cm) vs U Digit (ID) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -5, 5, 1000, -5, 5);
        tmp2D->GetXaxis()->SetTitle("intercept U coor (cm)");
        tmp2D->GetYaxis()->SetTitle("digit U coor (cm)");
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {

          for (auto& it : this->m_SVDShaperDigits)
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && (it.isUStrip())) {
              const VXD::SensorInfoBase& aSensorInfo = m_geoCache.getSensorInfo(it.getSensorID());
              hPtr->Fill(inter->getCoorU(), aSensorInfo.getUCellPosition(it.getCellID()));
              //        hPtr->Fill( inter->getCoorU(), it.getVCellID()*75e-4 );
            }
        }
                                  )
                                )
                               );

        name = "hCoorV_vs_VDigit_" + sensorid;
        title = "V intercept (cm) vs V Digit (ID) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -5, 5, 1000, -5, 5);
        tmp2D->GetXaxis()->SetTitle("intercept V coor (cm)");
        tmp2D->GetYaxis()->SetTitle("digi V coor (cm)");
        hInterDictionary.insert(std::pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {

          for (auto& it : this->m_SVDShaperDigits) {
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && (!it.isUStrip())) {
              const VXD::SensorInfoBase& aSensorInfo = m_geoCache.getSensorInfo(it.getSensorID());
              hPtr->Fill(inter->getCoorV(), aSensorInfo.getVCellPosition(it.getCellID()));
              //        hPtr->Fill( inter->getCoorV(), it.getUCellID()*50e-4  );
            }
          }
        }
                                  )
                                )
                               );


        //--------------------------

        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }

}

void SVDROIDQMModule::fillSensorInterHistos(const SVDIntercept* inter)
{

  auto its = hInterDictionary.equal_range(inter->getSensorID());

  for (auto it = its.first; it != its.second; ++it) {
    InterHistoAndFill aInterHistoAndFill = it->second;
    aInterHistoAndFill.second(aInterHistoAndFill.first, inter);
  }

}


void SVDROIDQMModule::endRun()
{

  hCellU->Scale((double)1 / n_events);
  hCellV->Scale((double)1 / n_events);
}
