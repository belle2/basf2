/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/ROIGeneratorModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <pxd/geometry/SensorInfo.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/ROIid.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(ROIGenerator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ROIGeneratorModule::ROIGeneratorModule() : Module()
{
  //Set module properties
  setDescription("This module is used to generate a certain number of ROIs");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  addParam("ROIListName", m_ROIListName, "name of the list of ROIs", std::string(""));
  addParam("nROIs", m_nROIs, "number of generated ROIs", 1);

}

ROIGeneratorModule::~ROIGeneratorModule()
{
}


void ROIGeneratorModule::initialize()
{
  StoreObjPtr<EventMetaData>::required();
  StoreArray<ROIid>::registerPersistent(m_ROIListName);
}

void ROIGeneratorModule::beginRun()
{
}


void ROIGeneratorModule::event()
{

  StoreArray<ROIid> ROIList(m_ROIListName);
  ROIList.create();

  ROIid tmp_ROIid;

  VxdID sensorID;
  sensorID.setLayerNumber(1);
  sensorID.setLadderNumber(0);
  sensorID.setSensorNumber(1);

  int minU;
  int minV;
  int maxU;
  int maxV;

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  int iROI = eventMetaDataPtr->getEvent();
  //  for (int iROI = 0; iROI < m_nROIs; iROI++) {

  //    minU =  (0 + iROI*16 ) % 250;
  //  minU =  iROI % 250;
  //  maxU = min(249, minU + 5);
  //    minV =  (0 + iROI*32 ) % 768;
  //  minV = (iROI / 250) % 768;
  //  maxV = min(767 , minV + 10);

  minU = 0;
  maxU = 479;
  minV = 0;
  maxV = 127;

  tmp_ROIid.setMinUid(minU) ;
  tmp_ROIid.setMinVid(minV);
  tmp_ROIid.setMaxUid(maxU);
  tmp_ROIid.setMaxVid(maxV);
  tmp_ROIid.setSensorID(sensorID);

  ROIList.appendNew(tmp_ROIid);
  //  }
}



void ROIGeneratorModule::endRun()
{
}


void ROIGeneratorModule::terminate()
{
}

