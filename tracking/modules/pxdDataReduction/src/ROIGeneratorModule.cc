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

  addParam("TrigDivider", m_divider, "Generates one ROI every TrigDivider events", 2);
  addParam("Layer"      , m_layer  , "on layer", 2);
  addParam("Ladder"     , m_ladder , " ladder " , 1);
  addParam("Sensor"     , m_sensor , " sensor " , 1);

  addParam("MinU"       , m_minU   , " min U (pixel column hopefully) ", 0);
  addParam("MaxU"       , m_maxU   , " max U (pixel column hopefully) ", 479);


  addParam("MinV"       , m_minV   , " min V (pixel column hopefully) ", 0);
  addParam("MaxV"       , m_maxV   , " max v (pixel column hopefully) ", 192);


}

ROIGeneratorModule::~ROIGeneratorModule()
{
}


void ROIGeneratorModule::initialize()
{
  StoreObjPtr<EventMetaData>::required();
  StoreArray<ROIid>::registerPersistent(m_ROIListName,
                                        DataStore::c_Event,
                                        false); // does not report error if ROIid exists
}

void ROIGeneratorModule::beginRun()
{
}


void ROIGeneratorModule::event()
{

  StoreArray<ROIid> ROIList(m_ROIListName);

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  int iROI = eventMetaDataPtr->getEvent(); // trigger number

  if (iROI % m_divider != 0)
    return ;

  ROIList.create(true);

  ROIid tmp_ROIid;

  VxdID sensorID;
  sensorID.setLayerNumber(m_layer);
  sensorID.setLadderNumber(m_ladder);
  sensorID.setSensorNumber(m_sensor);

  int minU;
  int minV;
  int maxU;
  int maxV;

  //  for (int iROI = 0; iROI < m_nROIs; iROI++) {

  //    minU =  (0 + iROI*16 ) % 250;
  //  minU =  iROI % 250;
  //  maxU = min(249, minU + 5);
  //    minV =  (0 + iROI*32 ) % 768;
  //  minV = (iROI / 250) % 768;
  //  maxV = min(767 , minV + 10);


  tmp_ROIid.setMinUid(m_minU);
  tmp_ROIid.setMinVid(m_minV);
  tmp_ROIid.setMaxUid(m_maxU);
  tmp_ROIid.setMaxVid(m_maxV);
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

