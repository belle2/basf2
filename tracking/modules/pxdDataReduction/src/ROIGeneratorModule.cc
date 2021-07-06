/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/pxdDataReduction/ROIGeneratorModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
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

  addParam("Random"       , m_random   , "dont use fix position, move pseudo randomly", false);
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
  int minU = m_minU, maxU = m_maxU, minV = m_minV, maxV = m_maxV;
  int w = maxU - minU;
  int h = maxV - minV;
  if (m_nROIs == 1 && m_random) {
    switch (tNr % 9) {
      case 0:
        minU = 0;
        maxU = w;
        minV = 0;
        maxV = h;
        break;
      case 1:
        minU = 0;
        maxU = w;
        break;
      case 2:
        minU = 0;
        maxU = w;
        minV = 768 - 1 - h;
        maxV = 768 - 1;
        break;
      case 3:
        minV = 0;
        maxV = h;
        break;
      case 4:
        break;
      case 5:
        minV = 768 - 1 - h;
        maxV = 768 - 1;
        break;
      case 6:
        minU = 250 - 1 - w;
        maxU = 250 - 1;
        minV = 0;
        maxV = h;
        break;
      case 7:
        minU = 250 - 1 - w;
        maxU = 250 - 1;
        break;
      case 8:
        minU = 250 - 1 - w;
        maxU = 250 - 1;
        minV = 768 - 1 - h;
        maxV = 768 - 1;
        break;
      default:
        break;
    }
  }
  tmp_ROIid.setMinUid(minU);
  tmp_ROIid.setMinVid(minV);
  tmp_ROIid.setMaxUid(maxU);
  tmp_ROIid.setMaxVid(maxV);
  tmp_ROIid.setSensorID(sensorID);

  ROIList.appendNew(tmp_ROIid);

  if (m_nROIs > 1) {
    // ... plus additional ones for debugging.
    // maybe we should do it depending on the triggernr lateron...
    int dU = (m_maxU - m_minU) / (m_nROIs + 1);
    int dV = (m_maxV - m_minV) / (m_nROIs + 1);
    for (int iROI = 1; iROI < m_nROIs; iROI++) {
      // Create a chain of ROIs from top left to bottom right
      minU = m_minU + dU * iROI;
      maxU = minU + dU;
      minV = m_minV + dV * iROI;
      maxV = minV + dV;

      tmp_ROIid.setMinUid(minU);
      tmp_ROIid.setMinVid(minV);
      tmp_ROIid.setMaxUid(maxU);
      tmp_ROIid.setMaxVid(maxV);
      tmp_ROIid.setSensorID(sensorID);

      ROIList.appendNew(tmp_ROIid);
    }
  }
}
