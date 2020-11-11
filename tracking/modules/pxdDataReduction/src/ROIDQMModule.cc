/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Giulia Casarosa, Eugenio Paoloni, Bjoern Spruck         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/ROIDQMModule.h>
#include <vxd/geometry/GeoCache.h>

#include <TDirectory.h>
#include <TH2F.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ROIDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ROIDQMModule::ROIDQMModule()
  : HistoModule()
  , m_InterDir(nullptr)
  , m_ROIDir(nullptr)
  , m_hInterDictionary(40, [](const Belle2::VxdID & vxdid) {return (size_t)vxdid.getID(); })
, m_hROIDictionary(40, [](const Belle2::VxdID& vxdid) {return (size_t)vxdid.getID(); })
, m_hROIDictionaryEvt(40, [](const Belle2::VxdID& vxdid) {return (size_t)vxdid.getID(); })
, m_numModules(0)
, m_hnROIs(nullptr)
, m_hnInter(nullptr)
, m_harea(nullptr)
, m_hredFactor(nullptr)
{
  //Set module properties
  setDescription("Monitor of the ROI creation on HLT");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("PXDDigitsName", m_PXDDigitsName,
           "name of the list of PXDDigits", std::string(""));

  addParam("InterceptsName", m_InterceptsName,
           "name of the list of Interceptions", std::string(""));

  addParam("ROIsName", m_ROIsName,
           "name of the list of ROIs", std::string(""));

}

void ROIDQMModule::defineHisto()
{

  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir("intercept");
  oldDir->cd("intercept");
  m_InterDir = gDirectory;
  oldDir->mkdir("roi");
  oldDir->cd("roi");
  m_ROIDir =  gDirectory;

  m_InterDir->cd();
  m_hnInter  = new TH1F("hnInter", "number of intercepts", 100, 0, 100);

  m_ROIDir->cd();
  m_hnROIs  = new TH1F("hnROIs", "number of ROIs", 100, 0, 100);
  m_harea = new TH1F("harea", "ROI area", 100, 0, 100000);
  m_hredFactor = new TH1F("hredFactor", "ROI reduction factor", 1000, 0, 1);

  createHistosDictionaries();

  oldDir->cd();

}

void ROIDQMModule::initialize()
{
  REG_HISTOGRAM

  m_pxdDigits.isOptional();
  m_roiIDs.isRequired(m_ROIsName);
  m_pxdIntercept.isRequired(m_InterceptsName);

}

void ROIDQMModule::event()
{

  m_hnInter->Fill(m_pxdIntercept.getEntries());

  for (auto& it : m_pxdIntercept)
    fillSensorInterHistos(&it);


  for (auto it = m_hROIDictionaryEvt.begin(); it != m_hROIDictionaryEvt.end(); ++it)
    (it->second).value = 0;

  int ROIarea = 0;
  double redFactor = 0;

  for (auto& it : m_roiIDs) {
    fillSensorROIHistos(&it);

    const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
    const int nPixelsU = aSensorInfo.getUCells();
    const int nPixelsV = aSensorInfo.getVCells();

    const int minU = it.getMinUid();
    const int minV = it.getMinVid();
    const int maxU = it.getMaxUid();
    const int maxV = it.getMaxVid();

    int tmpROIarea = (maxU - minU) * (maxV - minV);
    ROIarea += tmpROIarea;
    redFactor += (double)tmpROIarea / (nPixelsU * nPixelsV * m_numModules);

  }

  m_hnROIs->Fill(m_roiIDs.getEntries());

  m_harea->Fill((double)ROIarea);

  m_hredFactor->Fill((double)redFactor);


  for (auto it = m_hROIDictionaryEvt.begin(); it != m_hROIDictionaryEvt.end(); ++it) {
    ROIHistoAccumulateAndFill aROIHistoAccumulateAndFill = it->second;
    aROIHistoAccumulateAndFill.fill(aROIHistoAccumulateAndFill.hPtr, aROIHistoAccumulateAndFill.value);
  }

}

void ROIDQMModule::createHistosDictionaries()
{

  //  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  string name; //name of the histogram
  string title; //title of the histogram
  TH2F* tmp2D; //temporary 2D histo used to set axis title
  TH1F* tmp1D; //temporary 1D histo used to set axis title

  m_numModules = 0;

  std::set<Belle2::VxdID> pxdLayers = m_aGeometry.getLayers(VXD::SensorInfoBase::PXD);
  std::set<Belle2::VxdID>::iterator itPxdLayers = pxdLayers.begin();

  while (itPxdLayers != pxdLayers.end()) {

    std::set<Belle2::VxdID> pxdLadders = m_aGeometry.getLadders(*itPxdLayers);
    std::set<Belle2::VxdID>::iterator itPxdLadders = pxdLadders.begin();

    while (itPxdLadders != pxdLadders.end()) {

      std::set<Belle2::VxdID> pxdSensors = m_aGeometry.getSensors(*itPxdLadders);
      std::set<Belle2::VxdID>::iterator itPxdSensors = pxdSensors.begin();

      while (itPxdSensors != pxdSensors.end()) {

        m_numModules++; //counting the total number of modules

        const VXD::SensorInfoBase& wSensorInfo = m_aGeometry.getSensorInfo(*itPxdSensors);

        const int nPixelsU = wSensorInfo.getUCells();
        const int nPixelsV = wSensorInfo.getVCells();
        string sensorid = std::to_string(itPxdSensors->getLayerNumber()) + "_" + std::to_string(itPxdSensors->getLadderNumber()) + "_" +
                          std::to_string(itPxdSensors->getSensorNumber());


        // ------ HISTOGRAMS WITH AN ACCUMULATE PER ROI AND A FILL PER EVENT -------
        m_ROIDir->cd();

        name = "hNROIs_" + sensorid;
        title = "number of m_roiIDs for sensor " + sensorid;
        double value = 0;
        ROIHistoAccumulateAndFill* aHAAF = new ROIHistoAccumulateAndFill {
          new TH1F(name.c_str(), title.c_str(), 25, 0, 25),
          [](const ROIid*, double & val) {val++;},
          [](TH1 * hPtr, double & val) { hPtr->Fill(val); },
          value
        };
        m_hROIDictionaryEvt.insert(pair< Belle2::VxdID, ROIHistoAccumulateAndFill& > ((Belle2::VxdID)*itPxdSensors, *aHAAF));




        // ------ HISTOGRAMS WITH A FILL PER INTERCEPT -------
        m_InterDir->cd();

        // coor U and V
        name = "hCoorU_" + sensorid;
        title = "U coordinate of the extrapolation in U for sensor " + sensorid;
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      new TH1F(name.c_str(), title.c_str(), 100, -5, 5),
        [](TH1 * hPtr, const PXDIntercept * inter) { hPtr->Fill(inter->getCoorU()); }
                                    )
                                  )
                                 );

        name = "hCoorV_" + sensorid;
        title = "V coordinate of the extrapolation in V for sensor " + sensorid;
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      new TH1F(name.c_str(), title.c_str(), 100, -5, 5),
        [](TH1 * hPtr, const PXDIntercept * inter) { hPtr->Fill(inter->getCoorV()); }
                                    )
                                  )
                                 );

        // Intercept U vs V coordinate
        name = "hCoorU_vs_CoorV_" + sensorid;
        title = "U vs V intercept (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 100, -5, 5, 100, -5, 5);
        tmp2D->GetXaxis()->SetTitle("intercept U coor (cm)");
        tmp2D->GetYaxis()->SetTitle("intercept V coor (cm)");
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      tmp2D,
        [](TH1 * hPtr, const PXDIntercept * inter) { hPtr->Fill(inter->getCoorU(), inter->getCoorV()); }
                                    )
                                  )
                                 );


        // sigma U and V
        name = "hStatErrU_" + sensorid;
        title = "stat error of the extrapolation in U for sensor " + sensorid;
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      new TH1F(name.c_str(), title.c_str(), 100, 0, 0.35),
        [](TH1 * hPtr, const PXDIntercept * inter) { hPtr->Fill(inter->getSigmaU()); }
                                    )
                                  )
                                 );
        name = "hStatErrV_" + sensorid;
        title = "stat error of the extrapolation in V for sensor " + sensorid;
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      new TH1F(name.c_str(), title.c_str(), 100, 0, 0.35),
        [](TH1 * hPtr, const PXDIntercept * inter) { hPtr->Fill(inter->getSigmaV()); }
                                    )
                                  )
                                 );

        //1D residuals
        name = "hResidU_" + sensorid;
        title = "U residuals = intercept - digit,  for sensor " + sensorid;
        tmp1D = new TH1F(name.c_str(), title.c_str(), 1000, -5, 5);
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      tmp1D,
        [this](TH1 * hPtr, const PXDIntercept * inter) {
          for (auto& it : m_pxdDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              hPtr->Fill(inter->getCoorU() - aSensorInfo.getUCellPosition(it.getUCellID(), it.getVCellID()));
            }
        }
                                    )
                                  )
                                 );

        name = "hResidV_" + sensorid;
        title = "V residuals = intercept - digit,  for sensor " + sensorid;
        tmp1D = new TH1F(name.c_str(), title.c_str(), 1000, -5, 5);
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      tmp1D,
        [this](TH1 * hPtr, const PXDIntercept * inter) {
          for (auto& it : m_pxdDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              hPtr->Fill(inter->getCoorV() - aSensorInfo.getVCellPosition(it.getVCellID()));
            }
        }
                                    )
                                  )
                                 );

        name = "hResidV_vs_ResidU_" + sensorid;
        title = "V vs U residuals = intercept - digit,  for sensor " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -5, 5, 1000, -5, 5);
        tmp2D->GetXaxis()->SetTitle("U resid (cm)");
        tmp2D->GetYaxis()->SetTitle("V resid (cm)");
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      tmp2D,
        [this](TH1 * hPtr, const PXDIntercept * inter) {
          for (auto& it : m_pxdDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              double residU = inter->getCoorU() - aSensorInfo.getUCellPosition(it.getUCellID(), it.getVCellID());
              double residV = inter->getCoorV() - aSensorInfo.getVCellPosition(it.getVCellID());
              hPtr->Fill(residU, residV);
            }
        }
                                    )
                                  )
                                 );

        name = "hResidVm_vs_ResidU_" + sensorid;
        title = "V vs U residuals = intercept - digit,  for sensor " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -5, 5, 1000, -5, 5);
        tmp2D->GetXaxis()->SetTitle("U resid (cm)");
        tmp2D->GetYaxis()->SetTitle("V* resid (cm)");
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      tmp2D,
        [this](TH1 * hPtr, const PXDIntercept * inter) {
          for (auto& it : m_pxdDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              double residU = inter->getCoorU() - aSensorInfo.getUCellPosition(it.getUCellID(), it.getVCellID());
              double residV = inter->getCoorV() + aSensorInfo.getVCellPosition(it.getVCellID());
              hPtr->Fill(residU, residV);
            }
        }
                                    )
                                  )
                                 );

        name = "hResidV_vs_ResidUm_" + sensorid;
        title = "V vs U residuals = intercept - digit,  for sensor " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -5, 5, 1000, -5, 5);
        tmp2D->GetXaxis()->SetTitle("U* resid (cm)");
        tmp2D->GetYaxis()->SetTitle("V resid (cm)");
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      tmp2D,
        [this](TH1 * hPtr, const PXDIntercept * inter) {
          for (auto& it : m_pxdDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              double residU = inter->getCoorU() + aSensorInfo.getUCellPosition(it.getUCellID(), it.getVCellID());
              double residV = inter->getCoorV() - aSensorInfo.getVCellPosition(it.getVCellID());
              hPtr->Fill(residU, residV);
            }
        }
                                    )
                                  )
                                 );

        name = "hResidVm_vs_ResidUm_" + sensorid;
        title = "V vs U residuals = intercept - digit,  for sensor " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -5, 5, 1000, -5, 5);
        tmp2D->GetXaxis()->SetTitle("U* resid (cm)");
        tmp2D->GetYaxis()->SetTitle("V* resid (cm)");
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      tmp2D,
        [this](TH1 * hPtr, const PXDIntercept * inter) {
          for (auto& it : m_pxdDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              double residU = inter->getCoorU() + aSensorInfo.getUCellPosition(it.getUCellID(), it.getVCellID());
              double residV = inter->getCoorV() + aSensorInfo.getVCellPosition(it.getVCellID());
              hPtr->Fill(residU, residV);
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
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      tmp2D,
        [this](TH1 * hPtr, const PXDIntercept * inter) {
          for (auto& it : m_pxdDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              double resid = inter->getCoorU() - aSensorInfo.getUCellPosition(it.getUCellID(), it.getVCellID());
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
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      tmp2D,
        [this](TH1 * hPtr, const PXDIntercept * inter) {
          for (auto& it : m_pxdDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              double resid = inter->getCoorV() - aSensorInfo.getVCellPosition(it.getVCellID());
              hPtr->Fill(inter->getCoorV(), resid);
            }
        }
                                    )
                                  )
                                 );

        //residual U,V vs coordinate V,U
        name = "hResidU_vs_CoorV_" + sensorid;
        title = "U residual (cm) vs coor V (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -5, 5, 1000, -5, 5);
        tmp2D->GetYaxis()->SetTitle("U resid (cm)");
        tmp2D->GetXaxis()->SetTitle("V coor (cm)");
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      tmp2D,
        [this](TH1 * hPtr, const PXDIntercept * inter) {
          for (auto& it : m_pxdDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              double resid = inter->getCoorU() - aSensorInfo.getUCellPosition(it.getUCellID(), it.getVCellID());
              hPtr->Fill(inter->getCoorV(), resid);
            }
        }
                                    )
                                  )
                                 );

        name = "hResidV_vs_CoorU_" + sensorid;
        title = "V residual (cm) vs coor U (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -5, 5, 1000, -5, 5);
        tmp2D->GetYaxis()->SetTitle("V resid (cm)");
        tmp2D->GetXaxis()->SetTitle("U coor (cm)");
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      tmp2D,
        [this](TH1 * hPtr, const PXDIntercept * inter) {
          for (auto& it : m_pxdDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              double resid = inter->getCoorV() - aSensorInfo.getVCellPosition(it.getVCellID());
              hPtr->Fill(inter->getCoorU(), resid);
            }
        }
                                    )
                                  )
                                 );



        //residual vs charge
        name = "hResidU_vs_charge_" + sensorid;
        title = "U residual (cm) vs charge " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 250, 0, 250, 100, -5, 5);
        tmp2D->GetYaxis()->SetTitle("U resid (cm)");
        tmp2D->GetXaxis()->SetTitle("charge");
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      tmp2D,
        [this](TH1 * hPtr, const PXDIntercept * inter) {
          for (auto& it : m_pxdDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              double resid = inter->getCoorU() - aSensorInfo.getUCellPosition(it.getUCellID(), it.getVCellID());
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
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      tmp2D,
        [this](TH1 * hPtr, const PXDIntercept * inter) {
          for (auto& it : m_pxdDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              double resid = inter->getCoorV() - aSensorInfo.getVCellPosition(it.getVCellID());
              hPtr->Fill(it.getCharge(), resid);
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
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      tmp2D,
        [this](TH1 * hPtr, const PXDIntercept * inter) {
          for (auto& it : m_pxdDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              hPtr->Fill(inter->getCoorU(), aSensorInfo.getUCellPosition(it.getUCellID(), it.getVCellID()));
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
        m_hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                  (
                                    (Belle2::VxdID)*itPxdSensors,
                                    InterHistoAndFill(
                                      tmp2D,
        [this](TH1 * hPtr, const PXDIntercept * inter) {
          for (auto& it : m_pxdDigits) {
            if ((int)it.getSensorID() == (int)inter->getSensorID()) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              hPtr->Fill(inter->getCoorV(), aSensorInfo.getVCellPosition(it.getVCellID()));
              //        hPtr->Fill( inter->getCoorV(), it.getUCellID()*50e-4  );
            }
          }
        }
                                    )
                                  )
                                 );



        // ------ HISTOGRAMS WITH A FILL PER ROI -------
        m_ROIDir->cd();

        // MIN in U and V
        name = "hminU_" + sensorid;
        title = "ROI min in U for sensor " + sensorid;
        m_hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
                                (
                                  (Belle2::VxdID)*itPxdSensors,
                                  ROIHistoAndFill(
                                    new TH1F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU),
        [](TH1 * hPtr, const ROIid * roi) { hPtr->Fill(roi->getMinUid()); }
                                  )
                                )
                               );
        name = "hminV_" + sensorid;
        title = "ROI min in V for sensor " + sensorid;
        m_hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
                                (
                                  (Belle2::VxdID)*itPxdSensors,
                                  ROIHistoAndFill(
                                    new TH1F(name.c_str(), title.c_str(), nPixelsV, 0, nPixelsV),
        [](TH1 * hPtr, const ROIid * roi) { hPtr->Fill(roi->getMinVid()); }
                                  )
                                )
                               );
        //--------------------------
        // MAX in U and V
        name = "hmaxU_" + sensorid;
        title = "ROI max in U for sensor " + sensorid;
        m_hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
                                (
                                  (Belle2::VxdID)*itPxdSensors,
                                  ROIHistoAndFill(
                                    new TH1F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU),
        [](TH1 * hPtr, const ROIid * roi) { hPtr->Fill(roi->getMaxUid()); }
                                  )
                                )
                               );
        name = "hmaxV_" + sensorid;
        title = "ROI max in V for sensor " + sensorid;
        m_hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
                                (
                                  (Belle2::VxdID)*itPxdSensors,
                                  ROIHistoAndFill(
                                    new TH1F(name.c_str(), title.c_str(), nPixelsV, 0, nPixelsV),
        [](TH1 * hPtr, const ROIid * roi) { hPtr->Fill(roi->getMaxVid()); }
                                  )
                                )
                               );
        //--------------------------

        // WIDTH in U and V
        name = "hwidthU_" + sensorid;
        title = "ROI width in U for sensor " + sensorid;
        m_hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
                                (
                                  (Belle2::VxdID)*itPxdSensors,
                                  ROIHistoAndFill(
                                    new TH1F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU),
        [](TH1 * hPtr, const ROIid * roi) { hPtr->Fill(roi->getMaxUid() - roi->getMinUid()); }
                                  )
                                )
                               );
        name = "hwidthV_" + sensorid;
        title = "ROI width in V for sensor " + sensorid;
        m_hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
                                (
                                  (Belle2::VxdID)*itPxdSensors,
                                  ROIHistoAndFill(
                                    new TH1F(name.c_str(), title.c_str(), nPixelsV, 0, nPixelsV),
        [](TH1 * hPtr, const ROIid * roi) { hPtr->Fill(roi->getMaxVid() - roi->getMinVid()); }
                                  )
                                )
                               );

        // ROI center
        name = "hROIcenter_" + sensorid;
        title = "ROI center " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU, nPixelsV, 0, nPixelsV);
        tmp2D->GetXaxis()->SetTitle(" U (ID)");
        tmp2D->GetYaxis()->SetTitle(" V (ID)");
        m_hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
                                (
                                  (Belle2::VxdID)*itPxdSensors,
                                  ROIHistoAndFill(
                                    tmp2D,
        [](TH1 * hPtr, const ROIid * roi) { hPtr->Fill((roi->getMaxUid() + roi->getMinUid()) / 2, (roi->getMaxVid() + roi->getMinVid()) / 2); }
                                  )
                                )
                               );

        //--------------------------

        ++itPxdSensors;
      }
      ++itPxdLadders;
    }
    ++itPxdLayers;
  }

}

void ROIDQMModule::fillSensorInterHistos(const PXDIntercept* inter)
{

  auto its = m_hInterDictionary.equal_range(inter->getSensorID());

  for (auto it = its.first; it != its.second; ++it) {
    InterHistoAndFill aInterHistoAndFill = it->second;
    aInterHistoAndFill.second(aInterHistoAndFill.first, inter);
  }

}

void ROIDQMModule::fillSensorROIHistos(const ROIid* roi)
{

  auto its = m_hROIDictionary.equal_range(roi->getSensorID());

  for (auto it = its.first; it != its.second; ++it) {
    ROIHistoAndFill aROIHistoAndFill = it->second;
    aROIHistoAndFill.second(aROIHistoAndFill.first, roi);
  }

  auto itsEvt = m_hROIDictionaryEvt.equal_range(roi->getSensorID());
  for (auto it = itsEvt.first; it != itsEvt.second; ++it)
    (it->second).accumulate(roi, (it->second).value);
}

void ROIDQMModule::terminate()
{
  for (auto it = m_hROIDictionaryEvt.begin(); it != m_hROIDictionaryEvt.end(); ++it)
    delete &(it->second);
}
