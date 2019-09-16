/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <tracking/svdROIFinder/ROIStripTranslator.h>
#include <framework/logging/Logger.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <list>

using namespace std;
using namespace Belle2;

ROIStripTranslator::ROIStripTranslator(double user_sigmaSystU, double user_sigmaSystV, double user_numSigmaTotU,
                                       double user_numSigmaTotV, double user_maxWidthU, double user_maxWidthV)
  : m_sigmaSystU(user_sigmaSystU)
  , m_sigmaSystV(user_sigmaSystV)
  , m_numSigmaTotU(user_numSigmaTotU)
  , m_numSigmaTotV(user_numSigmaTotV)
  , m_maxWidthU(user_maxWidthU)
  , m_maxWidthV(user_maxWidthV)
{}


ROIStripTranslator::ROIStripTranslator(const ROIinfo* theROIinfo)
  : m_sigmaSystU(theROIinfo->sigmaSystU)
  , m_sigmaSystV(theROIinfo->sigmaSystV)
  , m_numSigmaTotU(theROIinfo->numSigmaTotU)
  , m_numSigmaTotV(theROIinfo->numSigmaTotV)
  , m_maxWidthU(theROIinfo->maxWidthU)
  , m_maxWidthV(theROIinfo->maxWidthV)
{}


ROIStripTranslator::~ROIStripTranslator()
{}


void
ROIStripTranslator::fillRoiIDList(StoreArray<SVDIntercept>* listOfIntercepts,
                                  StoreArray<ROIid>* ROIidList)
{

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  for (int i = 0; i < listOfIntercepts->getEntries(); i++) {

    B2DEBUG(10, "  --->> a NEW INTERCEPT!");


    const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo((*listOfIntercepts)[i]->getSensorID());

    double widthTotU = std::min(m_maxWidthU ,
                                sqrt((*listOfIntercepts)[i]->getSigmaU() * (*listOfIntercepts)[i]->getSigmaU() + m_sigmaSystU * m_sigmaSystU) * m_numSigmaTotU);
    double widthTotV = std::min(m_maxWidthV ,
                                sqrt((*listOfIntercepts)[i]->getSigmaV() * (*listOfIntercepts)[i]->getSigmaV() + m_sigmaSystV * m_sigmaSystV) * m_numSigmaTotV);

    double minU = (*listOfIntercepts)[i]->getCoorU() - widthTotU / 2 ;
    double maxU = (*listOfIntercepts)[i]->getCoorU() + widthTotU / 2 ;
    const int nStripsU = aSensorInfo.getUCells() - 1;

    double minV = (*listOfIntercepts)[i]->getCoorV() - widthTotV / 2;
    double maxV = (*listOfIntercepts)[i]->getCoorV() + widthTotV / 2;
    const int nStripsV = aSensorInfo.getVCells() - 1;

    const int firstStripID = 0;

    double min_uID = aSensorInfo.getUCellID(minU, minV, false);
    double min_vID = aSensorInfo.getVCellID(minV, false);
    double max_uID = aSensorInfo.getUCellID(maxU, maxV, false);
    double max_vID = aSensorInfo.getVCellID(maxV, false);

    B2DEBUG(10, "  LAYER = " << VxdID((*listOfIntercepts)[i]->getSensorID()).getLayerNumber()
            << " LADDER = " << VxdID((*listOfIntercepts)[i]->getSensorID()).getLadderNumber()
            << " SENSOR = " << VxdID((*listOfIntercepts)[i]->getSensorID()).getSensorNumber()
           );

    B2DEBUG(10, "  nStrips (U,V) = (" << nStripsU << "," << nStripsV << ")");

    B2DEBUG(10, "  widthU = " << maxU - minU
            << "  minU = "  << minU
            << "  maxU = "  << maxU
            << "  lengthU = " << aSensorInfo.getUSize((*listOfIntercepts)[i]->getCoorV())
           );

    B2DEBUG(10, "  widthV = " << maxV - minV
            << "  minV = " << minV
            << "  maxV = " << maxV
            << "  lengthV = " << aSensorInfo.getVSize());

    B2DEBUG(10, " U strips in (" << min_uID << "," << max_uID << ")");
    B2DEBUG(10, " V strips in (" << min_vID << "," << max_vID << ")");


    //check that the pixel belong to the sensor
    bool inside = true;
    if (min_uID > nStripsU || max_uID < firstStripID || min_vID > nStripsV || max_vID < firstStripID) {
      B2DEBUG(5, "  OOOPS: this pixel does NOT belong to the sensor");
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

