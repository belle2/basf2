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
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("ROIListName", m_ROIListName, "name of the list of ROIs", std::string(""));
  addParam("nROIs", m_nROIs, "number of generated ROIs", 1);

  addParam("TrigDivider", m_divider, "Generates one ROI every TrigDivider events", 2);
  addParam("Layer"      , m_layer  , "on layer", 1);
  addParam("Ladder"     , m_ladder , " ladder " , 1);
  addParam("Sensor"     , m_sensor , " sensor " , 1);

  addParam("MinU"       , m_minU   , " min U (pixel column hopefully) ", 0);
  addParam("MaxU"       , m_maxU   , " max U (pixel column hopefully) ", 250 - 1);


  addParam("MinV"       , m_minV   , " min V (pixel column hopefully) ", 0);
  addParam("MaxV"       , m_maxV   , " max v (pixel column hopefully) ", 768 - 1);


}

void ROIGeneratorModule::initialize()
{
  StoreObjPtr<EventMetaData> eventMetaData;
  eventMetaData.isRequired();

  StoreArray<ROIid> roiIDs;
  roiIDs.registerInDataStore(m_ROIListName); // does not report error if ROIid exists
}

void ROIGeneratorModule::event()
{

  StoreArray<ROIid> ROIList(m_ROIListName);

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  int tNr = eventMetaDataPtr->getEvent(); // trigger number

  // Only if divider tells us to...
  if (m_divider != 0 && (tNr % m_divider) != 0)
    return ;

  //  ROIList.create(true);

  ROIid tmp_ROIid;

  VxdID sensorID;
  sensorID.setLayerNumber(m_layer);
  sensorID.setLadderNumber(m_ladder);
  sensorID.setSensorNumber(m_sensor);

  // Always create one FULL size ROI
  tmp_ROIid.setMinUid(m_minU);
  tmp_ROIid.setMinVid(m_minV);
  tmp_ROIid.setMaxUid(m_maxU);
  tmp_ROIid.setMaxVid(m_maxV);
  tmp_ROIid.setSensorID(sensorID);

  ROIList.appendNew(tmp_ROIid);

  if (m_nROIs > 1) {
    // ... plus additional ones for debugging.
    // maybe we should do it depending on the triggernr lateron...
    int dU = (m_maxU - m_minU) / (m_nROIs + 1);
    int dV = (m_maxV - m_minV) / (m_nROIs + 1);
    for (int iROI = 1; iROI < m_nROIs; iROI++) {
      // Create a chain of ROIs from top left to bottom right
      int minU = m_minU + dU * iROI;
      int maxU = minU + dU;
      int minV = m_minV + dV * iROI;
      int maxV = minV + dV;

      tmp_ROIid.setMinUid(minU);
      tmp_ROIid.setMinVid(minV);
      tmp_ROIid.setMaxUid(maxU);
      tmp_ROIid.setMaxVid(maxV);
      tmp_ROIid.setSensorID(sensorID);

      ROIList.appendNew(tmp_ROIid);
    }
  }
}
