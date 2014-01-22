/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Giulia Casarosa, Eugenio Paoloni                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/ROIDQMModule.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <vxd/geometry/GeoCache.h>

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
  ,   hInterDictionary(40, [](const Belle2::VxdID& vxdid) {return (size_t)vxdid.getID(); })
,   hROIDictionary(40, [](const Belle2::VxdID& vxdid) {return (size_t)vxdid.getID(); })
,   hROIDictionaryEvt(40, [](const Belle2::VxdID& vxdid) {return (size_t)vxdid.getID(); })
{
  //Set module properties
  setDescription("Monitor of the  ROIs creation on HLT");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("PXDDigitsName", m_PXDDigitsName,
           "name of the list of PXDDigits", std::string(""));

  addParam("InterceptsName", m_InterceptsName,
           "name of the list of interceptions", std::string(""));

  addParam("ROIsName", m_ROIsName,
           "name of the list of ROIs", std::string(""));


}

void ROIDQMModule::defineHisto()
{

  createHistosDictionaries();

  hnInter  = new TH1F("hnInter", "number of intercepts", 1000, 0, 10000);
  hnROIs  = new TH1F("hnROIs", "number of ROIs", 100, 0, 100);
  harea = new TH1F("harea", "ROIs area", 100, 0, 100000);

  hredFactor = new TH1F("hredFactor", "ROI reduction factor", 1000, 0, 1);

}

void ROIDQMModule::initialize()
{
  REG_HISTOGRAM

  StoreArray<PXDDigit>::optional();
  StoreArray<ROIid>::required(m_ROIsName);
  StoreArray<PXDIntercept>::required(m_InterceptsName);

}

void ROIDQMModule::event()
{

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  StoreArray<ROIid> ROIs(m_ROIsName);
  StoreArray<PXDIntercept> Intercepts(m_InterceptsName);

  hnInter->Fill(Intercepts.getEntries());

  for (auto & it : Intercepts)
    fillSensorInterHistos(&it);

  for (auto it = hROIDictionaryEvt.begin(); it != hROIDictionaryEvt.end(); ++it)
    (it->second).value = 0;

  int ROIarea = 0;
  double redFactor = 0;
  int minU;
  int minV;
  int maxU;
  int maxV;

  for (auto & it : ROIs) {

    fillSensorROIHistos(&it);

    const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(it.getSensorID());
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

  hnROIs->Fill(ROIs.getEntries());

  harea->Fill((double)ROIarea);

  hredFactor->Fill((double)redFactor);
  //  cout << " o  DQM: area = " << ROIarea << "  redFactor = " << redFactor << endl;

  for (auto it = hROIDictionaryEvt.begin(); it != hROIDictionaryEvt.end(); ++it) {
    ROIHistoAccumulateAndFill aROIHistoAccumulateAndFill = it->second;
    aROIHistoAccumulateAndFill.fill(aROIHistoAccumulateAndFill.hPtr, aROIHistoAccumulateAndFill.value);
  }

}

void ROIDQMModule::createHistosDictionaries()
{

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  string name; //name of the histogram
  string title; //title of the histogram

  m_numModules = 0;

  std::set<Belle2::VxdID> pxdLayers = aGeometry.getLayers(VXD::SensorInfoBase::PXD);
  std::set<Belle2::VxdID>::iterator itPxdLayers = pxdLayers.begin();

  while (itPxdLayers != pxdLayers.end()) {

    std::set<Belle2::VxdID> pxdLadders = aGeometry.getLadders(*itPxdLayers);
    std::set<Belle2::VxdID>::iterator itPxdLadders = pxdLadders.begin();

    while (itPxdLadders != pxdLadders.end()) {

      std::set<Belle2::VxdID> pxdSensors = aGeometry.getSensors(*itPxdLadders);
      std::set<Belle2::VxdID>::iterator itPxdSensors = pxdSensors.begin();

      while (itPxdSensors != pxdSensors.end()) {

        m_numModules++; //counting the total number of modules

        const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(*itPxdSensors);

        const int nPixelsU = aSensorInfo.getUCells();
        const int nPixelsV = aSensorInfo.getVCells();
        string sensorid = std::to_string(itPxdSensors->getLayerNumber()) + "_" + std::to_string(itPxdSensors->getLadderNumber()) + "_" + std::to_string(itPxdSensors->getSensorNumber());


        // ------ HISTOGRAMS WITH AN ACCUMULATE PER ROI AND A FILL PER EVENT -------

        name = "hNROIs_" + sensorid;
        title = "number of ROIs for sensor " + sensorid;
        double value = 0;
        ROIHistoAccumulateAndFill* aHAAF = new ROIHistoAccumulateAndFill {
          new TH1F(name.c_str(), title.c_str(), 25, 0, 25),
          [](const ROIid*, double & value) {value++;},
          [](TH1 * hPtr, double & value) { hPtr->Fill(value); },
          value
        };
        hROIDictionaryEvt.insert(pair< Belle2::VxdID, ROIHistoAccumulateAndFill& > ((Belle2::VxdID)*itPxdSensors, *aHAAF));


        // ------ HISTOGRAMS WITH A FILL PER INTERCEPT -------

        // sigma U and V
        name = "hsigmaU_" + sensorid;
        title = "stat error of the extrapolation in U for sensor " + sensorid;
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itPxdSensors,
                                  InterHistoAndFill(
                                    new TH1F(name.c_str(), title.c_str(), 100, 0, 0.35),
        [](TH1 * hPtr, const PXDIntercept * inter) { hPtr->Fill(inter->getSigmaU()); }
                                  )
                                )
                               );
        name = "hsigmaV_" + sensorid;
        title = "stat error of the extrapolation in V for sensor " + sensorid;
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itPxdSensors,
                                  InterHistoAndFill(
                                    new TH1F(name.c_str(), title.c_str(), 100, 0, 0.35),
        [](TH1 * hPtr, const PXDIntercept * inter) { hPtr->Fill(inter->getSigmaV()); }
                                  )
                                )
                               );
        // scatter plot: U,V intercept in cm VS U,V cell ID

        name = "hUIntercept_vs_UDigit_" + sensorid;
        title = "U intercept (cm) vs U Digit (ID) " + sensorid;
        TH2F* tmp = new TH2F(name.c_str(), title.c_str(), 100, -5, 5, 1024, 0, 1024);
        tmp->GetXaxis()->SetTitle("intercept U coor (cm)");
        tmp->GetYaxis()->SetTitle("U cell ID");
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itPxdSensors,
                                  InterHistoAndFill(
                                    tmp,
        [this, itPxdSensors](TH1 * hPtr, const PXDIntercept * inter) {
          StoreArray<PXDDigit> PXDDigits(this->m_PXDDigitsName);

          for (auto & it : PXDDigits)
            if ((int)it.getSensorID() == (int)inter->getSensorID())
              hPtr->Fill(inter->getCoorU(), it.getUCellID());
        }
                                  )
                                )
                               );

        name = "hVIntercept_vs_VDigit_" + sensorid;
        title = "V intercept (cm) vs V Digit (ID) " + sensorid;
        tmp = new TH2F(name.c_str(), title.c_str(), 100, -5, 5, 1024, 0, 1024);
        tmp->GetXaxis()->SetTitle("intercept V coor (cm)");
        tmp->GetYaxis()->SetTitle("V cell ID");
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itPxdSensors,
                                  InterHistoAndFill(
                                    tmp,
        [this, itPxdSensors](TH1 * hPtr, const PXDIntercept * inter) {
          StoreArray<PXDDigit> PXDDigits(this->m_PXDDigitsName);

          for (auto & it : PXDDigits) {
            //              cout<<"   pxd sensor "<< (int)it.getSensorID()<<" vs " <<(int)inter->getSensorID()<<endl;
            if ((int)it.getSensorID() == (int)inter->getSensorID())
              hPtr->Fill(inter->getCoorV(), it.getVCellID());
          }
        }
                                  )
                                )
                               );

        // Intercept U vs V coordinate
        name = "hUIntercept_vs_VIntercept_" + sensorid;
        title = "U vs V intercept (cm) " + sensorid;
        tmp = new TH2F(name.c_str(), title.c_str(), 100, -5, 5, 100, -5, 5);
        tmp->GetXaxis()->SetTitle("intercept U coor (cm)");
        tmp->GetYaxis()->SetTitle("intercept V coor (cm)");
        hInterDictionary.insert(pair< Belle2::VxdID, InterHistoAndFill >
                                (
                                  (Belle2::VxdID)*itPxdSensors,
                                  InterHistoAndFill(
                                    tmp,
        [](TH1 * hPtr, const PXDIntercept * inter) { hPtr->Fill(inter->getCoorU(), inter->getCoorV()); }
                                  )
                                )
                               );





        // ------ HISTOGRAMS WITH A FILL PER ROI -------

        // MIN in U and V
        name = "hminU_" + sensorid;
        title = "ROI min in U for sensor " + sensorid;
        hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
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
        hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
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
        hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
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
        hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
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
        hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
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
        hROIDictionary.insert(pair< Belle2::VxdID, ROIHistoAndFill >
                              (
                                (Belle2::VxdID)*itPxdSensors,
                                ROIHistoAndFill(
                                  new TH1F(name.c_str(), title.c_str(), nPixelsV, 0, nPixelsV),
        [](TH1 * hPtr, const ROIid * roi) { hPtr->Fill(roi->getMaxVid() - roi->getMinVid()); }
                                )
                              )
                             );
        //--------------------------

        itPxdSensors++;
      }
      itPxdLadders++;
    }
    itPxdLayers++;
  }

}

void ROIDQMModule::fillSensorInterHistos(const PXDIntercept* inter)
{

  auto its = hInterDictionary.equal_range(inter->getSensorID());

  for (auto it = its.first; it != its.second; ++it) {
    InterHistoAndFill aInterHistoAndFill = it->second;
    aInterHistoAndFill.second(aInterHistoAndFill.first, inter);
  }

}

void ROIDQMModule::fillSensorROIHistos(const ROIid* roi)
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

void ROIDQMModule::endRun()
{

  for (auto it = hROIDictionaryEvt.begin(); it != hROIDictionaryEvt.end(); ++it)
    delete &(it->second);
}
