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
  setDescription("This module is used to test the ROI chain");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  addParam("ROIListName", m_ROIListName, "name of the list of ROIs", std::string(""));
  addParam("nROIs", m_nROIs, "number of generated ROIs", 1);

}

ROIGeneratorModule::~ROIGeneratorModule()
{
}


void ROIGeneratorModule::initialize()
{

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
  sensorID.setSensorNumber(1);

  for (int iROI = 0; iROI < m_nROIs; iROI++) {
    //    cout<< " iROI = "<<iROI<<endl;

    sensorID.setLadderNumber((1 + iROI) % 8);
    //    sensorID.setLadderNumber(1);
    tmp_ROIid.setMinUid(1 + iROI);
    tmp_ROIid.setMinVid(1);
    tmp_ROIid.setMaxUid(4 + iROI);
    tmp_ROIid.setMaxVid(8);
    tmp_ROIid.setSensorID(sensorID);

    ROIList.appendNew(tmp_ROIid);
  }
}



void ROIGeneratorModule::endRun()
{
}


void ROIGeneratorModule::terminate()
{
}

