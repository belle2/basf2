/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Giulia Casarosa                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/svdROIFinder/SVDROIDQMModule.h>
#include <vxd/geometry/GeoCache.h>

#include "time.h"
#include "TDirectory.h"

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDROIDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDROIDQMModule::SVDROIDQMModule()
  : HistoModule()
  , m_InterDir(NULL)
  , m_ROIDir(NULL)
  , hInterDictionary(172, [](const Belle2::VxdID & vxdid) {return (size_t)vxdid.getID(); })
, hROIDictionary(172, [](const Belle2::VxdID& vxdid) {return (size_t)vxdid.getID(); })
, hROIDictionaryEvt(172, [](const Belle2::VxdID& vxdid) {return (size_t)vxdid.getID(); })
, m_numModules(0)
, hnROIs(NULL)
, hnInter(NULL)
, harea(NULL)
, hredFactor(NULL)
, hCellU(NULL)
, hCellV(NULL)
, n_events(0)
{
  //Set module properties
  setDescription("Monitor of the  ROIs creation on HLT");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("SVDShaperDigitsName", m_SVDShaperDigitsName,
           "name of the list of SVDShaperDigits", std::string(""));
  addParam("SVDRecoDigitsName", m_SVDRecoDigitsName,
           "name of the list of SVDRecoDigits", std::string(""));

  addParam("InterceptsName", m_InterceptsName,
           "name of the list of interceptions", std::string(""));

  addParam("ROIsName", m_ROIsName,
           "name of the list of ROIs", std::string(""));

}

void SVDROIDQMModule::defineHisto()
{

  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  TDirectory* roiDir = oldDir->mkdir("SVDROIs");
  m_InterDir = roiDir->mkdir("intercept");
  m_ROIDir = roiDir->mkdir("roi");

  hCellU  = new TH1F("hCellU", "CellID U", 769, -0.5, 768.5);
  hCellU->GetXaxis()->SetTitle("U cell ID");
  hCellV  = new TH1F("hCellV", "CellID V", 769, -0.5, 768.5);
  hCellV->GetXaxis()->SetTitle("V cell ID");

  m_InterDir->cd();
  hnInter  = new TH1F("hnInter", "number of intercepts", 100, 0, 100);

  m_ROIDir->cd();
  hnROIs  = new TH1F("hnROIs", "number of ROIs", 100, 0, 100);
  harea = new TH1F("harea", "ROIs area", 100, 0, 100000);
  hredFactor = new TH1F("hredFactor", "ROI reduction factor", 1000, 0, 1);


  createHistosDictionaries();

  oldDir->cd();

}

void SVDROIDQMModule::initialize()
{
  REG_HISTOGRAM

  m_SVDShaperDigits.isOptional(m_SVDShaperDigitsName);
  m_SVDRecoDigits.isOptional(m_SVDRecoDigitsName);
  m_ROIs.isRequired(m_ROIsName);
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


  for (auto it = hROIDictionaryEvt.begin(); it != hROIDictionaryEvt.end(); ++it)
    (it->second).value = 0;

  int ROIarea = 0;
  double redFactor = 0;
  int minU;
  int minV;
  int maxU;
  int maxV;

  for (auto& it : m_ROIs) {

    fillSensorROIHistos(&it);

    const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
    const int nPixelsU = aSensorInfo.getUCells();
    const int nPixelsV = aSensorInfo.getVCells();

    minU = it.getMinUid();
    minV = it.getMinVid();
    maxU = it.getMaxUid();
    maxV = it.getMaxVid();

    int tmpROIarea = (maxU - minU) * (maxV - minV);
    ROIarea += tmpROIarea;
    redFactor += (double)tmpROIarea / (nPixelsU * nPixelsV * m_numModules);

  }

  hnROIs->Fill(m_ROIs.getEntries());

  harea->Fill((double)ROIarea);

  hredFactor->Fill((double)redFactor);


  for (auto it = hROIDictionaryEvt.begin(); it != hROIDictionaryEvt.end(); ++it) {
    ROIHistoAccumulateAndFill aROIHistoAccumulateAndFill = it->second;
    aROIHistoAccumulateAndFill.fill(aROIHistoAccumulateAndFill.hPtr, aROIHistoAccumulateAndFill.value);
  }

}

void SVDROIDQMModule::createHistosDictionaries()
{

  //  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  string name; //name of the histogram
  string title; //title of the histogram
  TH2F* tmp2D; //temporary 2D histo used to set axis title
  TH1F* tmp1D; //temporary 1D histo used to set axis title

  m_numModules = 0;

  std::set<Belle2::VxdID> svdLayers = m_aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();

  while (itSvdLayers != svdLayers.end()) {

    std::set<Belle2::VxdID> svdLadders = m_aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) {

      std::set<Belle2::VxdID> svdSensors = m_aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();

      while (itSvdSensors != svdSensors.end()) {

        m_numModules++; //counting the total number of modules

        const VXD::SensorInfoBase& wSensorInfo = m_aGeometry.getSensorInfo(*itSvdSensors);

        const int nPixelsU = wSensorInfo.getUCells();
        const int nPixelsV = wSensorInfo.getVCells();
        string sensorid = std::to_string(itSvdSensors->getLayerNumber()) + "_" + std::to_string(itSvdSensors->getLadderNumber()) + "_" +
                          std::to_string(itSvdSensors->getSensorNumber());


        // ------ HISTOGRAMS WITH AN ACCUMULATE PER ROI AND A FILL PER EVENT -------
        m_ROIDir->cd();

        name = "hNROIs_" + sensorid;
        title = "number of ROIs for sensor " + sensorid;
        double value = 0;
        ROIHistoAccumulateAndFill* aHAAF = new ROIHistoAccumulateAndFill {
          new TH1F(name.c_str(), title.c_str(), 25, 0, 25),
          [](const ROIid*, double & val) {val++;},
          [](TH1 * hPtr, double & val) { hPtr->Fill(val); },
          value
        };
        hROIDictionaryEvt.insert(pair< Belle2::VxdID, ROIHistoAccumulateAndFill& > ((Belle2::VxdID)*itSvdSensors, *aHAAF));




        // ------ HISTOGRAMS WITH A FILL PER INTERCEPT -------
        m_InterDir->cd();

        // coor U and V
        name = "hCoorU_" + sensorid;
        title = "U coordinate of the extrapolation in U for sensor " + sensorid;
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
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
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
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
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
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
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
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
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
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

        name = "hResidV_" + sensorid;
        title = "V residuals = intercept - digit,  for sensor " + sensorid;
        tmp1D = new TH1F(name.c_str(), title.c_str(), 1000, -5, 5);
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

        //residual U,V vs coordinate U,V
        name = "hResidU_vs_CoorU_" + sensorid;
        title = "U residual (cm) vs coor U (cm) " + sensorid;
        tmp2D = new TH2F(name.c_str(), title.c_str(), 1000, -5, 5, 1000, -5, 5);
        tmp2D->GetYaxis()->SetTitle("U resid (cm)");
        tmp2D->GetXaxis()->SetTitle("U coor (cm)");
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
              hPtr->Fill(inter->getCoorV(), resid);
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
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {
          StoreArray<SVDRecoDigit> SVDRecoDigits(this->m_SVDRecoDigitsName);

          for (auto& it : SVDRecoDigits)
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && (it.isUStrip())) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
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
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this](TH1 * hPtr, const SVDIntercept * inter) {
          StoreArray<SVDRecoDigit> SVDRecoDigits(this->m_SVDRecoDigitsName);

          for (auto& it : SVDRecoDigits)
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && (!it.isUStrip())) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              double resid = inter->getCoorV() - aSensorInfo.getVCellPosition(it.getCellID());
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
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itSvdSensors,
                                  InterHistoAndFill(
                                    tmp2D,
        [this, itSvdSensors](TH1 * hPtr, const SVDIntercept * inter) {
          StoreArray<SVDShaperDigit> SVDShaperDigits(this->m_SVDShaperDigitsName);

          for (auto& it : SVDShaperDigits) {
            if (((int)it.getSensorID() == (int)inter->getSensorID()) && (!it.isUStrip())) {
              const VXD::SensorInfoBase& aSensorInfo = m_aGeometry.getSensorInfo(it.getSensorID());
              hPtr->Fill(inter->getCoorV(), aSensorInfo.getVCellPosition(it.getCellID()));
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
        hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
                              (
                                (Belle2::VxdID)*itSvdSensors,
                                ROIHistoAndFill(
                                  new TH1F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU),
        [](TH1 * hPtr, const ROIid * roi) { hPtr->Fill(roi->getMinUid()); }
                                )
                              )
                             );
        name = "hminV_" + sensorid;
        title = "ROI min in V for sensor " + sensorid;
        hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
                              (
                                (Belle2::VxdID)*itSvdSensors,
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
        hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
                              (
                                (Belle2::VxdID)*itSvdSensors,
                                ROIHistoAndFill(
                                  new TH1F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU),
        [](TH1 * hPtr, const ROIid * roi) { hPtr->Fill(roi->getMaxUid()); }
                                )
                              )
                             );
        name = "hmaxV_" + sensorid;
        title = "ROI max in V for sensor " + sensorid;
        hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
                              (
                                (Belle2::VxdID)*itSvdSensors,
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
        hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
                              (
                                (Belle2::VxdID)*itSvdSensors,
                                ROIHistoAndFill(
                                  new TH1F(name.c_str(), title.c_str(), nPixelsU, 0, nPixelsU),
        [](TH1 * hPtr, const ROIid * roi) { hPtr->Fill(roi->getMaxUid() - roi->getMinUid()); }
                                )
                              )
                             );
        name = "hwidthV_" + sensorid;
        title = "ROI width in V for sensor " + sensorid;
        hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
                              (
                                (Belle2::VxdID)*itSvdSensors,
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
        hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
                              (
                                (Belle2::VxdID)*itSvdSensors,
                                ROIHistoAndFill(
                                  tmp2D,
        [](TH1 * hPtr, const ROIid * roi) { hPtr->Fill((roi->getMaxUid() + roi->getMinUid()) / 2, (roi->getMaxVid() + roi->getMinVid()) / 2); }
                                )
                              )
                             );

        //--------------------------

        itSvdSensors++;
      }
      itSvdLadders++;
    }
    itSvdLayers++;
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

void SVDROIDQMModule::fillSensorROIHistos(const ROIid* roi)
{

  auto its = hROIDictionary.equal_range(roi->getSensorID());

  for (auto it = its.first; it != its.second; ++it) {
    ROIHistoAndFill aROIHistoAndFill = it->second;
    aROIHistoAndFill.second(aROIHistoAndFill.first, roi);
  }

  auto itsEvt = hROIDictionaryEvt.equal_range(roi->getSensorID());
  for (auto it = itsEvt.first; it != itsEvt.second; ++it)
    (it->second).accumulate(roi, (it->second).value);
}

void SVDROIDQMModule::endRun()
{

  hCellU->Scale((double)1 / n_events);
  hCellV->Scale((double)1 / n_events);

  for (auto it = hROIDictionaryEvt.begin(); it != hROIDictionaryEvt.end(); ++it)
    delete &(it->second);
}
