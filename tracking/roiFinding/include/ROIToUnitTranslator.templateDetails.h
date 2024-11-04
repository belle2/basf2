/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/roiFinding/ROIToUnitTranslator.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <tracking/dataobjects/ROIid.h>

namespace Belle2 {

  template<class aIntercept>
  ROIToUnitTranslator<aIntercept>::ROIToUnitTranslator(const ROIinfo*  theROIinfo)
    : m_sigmaSystU(theROIinfo->sigmaSystU)
    , m_sigmaSystV(theROIinfo->sigmaSystV)
    , m_numSigmaTotU(theROIinfo->numSigmaTotU)
    , m_numSigmaTotV(theROIinfo->numSigmaTotV)
    , m_maxWidthU(theROIinfo->maxWidthU)
    , m_maxWidthV(theROIinfo->maxWidthV)
  {};

  template<class aIntercept>
  ROIToUnitTranslator<aIntercept>::ROIToUnitTranslator(double sigmaSystU, double sigmaSystV,
                                                       double numSigmaTotU, double numSigmaTotV,
                                                       double maxWidthU, double maxWidthV)
    : m_sigmaSystU(sigmaSystU)
    , m_sigmaSystV(sigmaSystV)
    , m_numSigmaTotU(numSigmaTotU)
    , m_numSigmaTotV(numSigmaTotV)
    , m_maxWidthU(maxWidthU)
    , m_maxWidthV(maxWidthV)
  {};

  template<class aIntercept>
  void ROIToUnitTranslator<aIntercept>::fillRoiIDList(StoreArray<aIntercept>* listOfIntercepts, StoreArray<ROIid>* ROIidList)
  {

    const VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

    for (int i = 0; i < listOfIntercepts->getEntries(); i++) {

      B2DEBUG(21, "  --->> a NEW INTERCEPT!");


      const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo((*listOfIntercepts)[i]->getSensorID());

      double widthTotU = std::min(m_maxWidthU,
                                  sqrt((*listOfIntercepts)[i]->getSigmaU() * (*listOfIntercepts)[i]->getSigmaU() + m_sigmaSystU * m_sigmaSystU) * m_numSigmaTotU);
      double widthTotV = std::min(m_maxWidthV,
                                  sqrt((*listOfIntercepts)[i]->getSigmaV() * (*listOfIntercepts)[i]->getSigmaV() + m_sigmaSystV * m_sigmaSystV) * m_numSigmaTotV);

      double minU = (*listOfIntercepts)[i]->getCoorU() - widthTotU / 2 ;
      double maxU = (*listOfIntercepts)[i]->getCoorU() + widthTotU / 2 ;
      const int nUnitsU = aSensorInfo.getUCells() - 1;

      double minV = (*listOfIntercepts)[i]->getCoorV() - widthTotV / 2;
      double maxV = (*listOfIntercepts)[i]->getCoorV() + widthTotV / 2;
      const int nUnitsV = aSensorInfo.getVCells() - 1;

      const int firstPixelID = 0;

      double bottomLeft_uID = aSensorInfo.getUCellID(minU, minV, false);
      double bottomLeft_vID = aSensorInfo.getVCellID(minV, false);
      double topRight_uID = aSensorInfo.getUCellID(maxU, maxV, false);
      double topRight_vID = aSensorInfo.getVCellID(maxV, false);

      B2DEBUG(21, "  LAYER = " << VxdID((*listOfIntercepts)[i]->getSensorID()).getLayerNumber()
              << " LADDER = " << VxdID((*listOfIntercepts)[i]->getSensorID()).getLadderNumber()
              << " SENSOR = " << VxdID((*listOfIntercepts)[i]->getSensorID()).getSensorNumber()
             );

      B2DEBUG(21, "  number of units (pixel or strip) (U,V) = (" << nUnitsU << "," << nUnitsV << ")");

      B2DEBUG(21, "  widthU = " << maxU - minU
              << "  minU = "  << minU
              << "  maxU = "  << maxU
              << "  lengthU = " << aSensorInfo.getUSize((*listOfIntercepts)[i]->getCoorV())
             );

      B2DEBUG(21, "  widthV = " << maxV - minV
              << "  minV = " << minV
              << "  maxV = " << maxV
              << "  lengthV = " << aSensorInfo.getVSize());

      B2DEBUG(21, "  bottom left unit (pixel or strip) (U,V) = (" << bottomLeft_uID << "," << bottomLeft_vID << ")");
      B2DEBUG(21, "  top right unit (pixel or strip) (U,V) = (" << topRight_uID << "," << topRight_vID << ")");


      //check that the pixel belong to the sensor
      bool inside = true;
      if (bottomLeft_uID > nUnitsU || topRight_uID < firstPixelID || bottomLeft_vID > nUnitsV || topRight_vID < firstPixelID) {
        B2DEBUG(21, "  OOOPS: this unit (pixel or strip) does NOT belong to the sensor");
        inside = false;
      }

      ROIid tmpROIid;

      if (inside) {
        tmpROIid.setMinUid(aSensorInfo.getUCellID(minU, minV, true));
        tmpROIid.setMinVid(aSensorInfo.getVCellID(minV, true));
        tmpROIid.setMaxUid(aSensorInfo.getUCellID(maxU, maxV, true));
        tmpROIid.setMaxVid(aSensorInfo.getVCellID(maxV, true));
        tmpROIid.setSensorID((*listOfIntercepts)[i]->getSensorID()) ;

        ROIidList->appendNew(tmpROIid);

        // this is the pointer to the transient copy of tmpROIid
        ROIid* transientROIid = (*ROIidList)[ROIidList->getEntries() - 1];

        (*listOfIntercepts)[i]->addRelationTo(transientROIid);
      }
    }
  }

} // namespace Belle2