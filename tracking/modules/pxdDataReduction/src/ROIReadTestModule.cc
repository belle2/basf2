/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/StoreObjPtr.h>
#include <tracking/modules/pxdDataReduction/ROIReadTestModule.h>
#include <tracking/dataobjects/ROIpayload.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(ROIReadTest)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ROIReadTestModule::ROIReadTestModule() : Module()
{
  //Set module properties
  setDescription("check the payload produced by the ROIPayloadAssembler Module");
  //  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("outfileName", m_outfileName, "name of the output file", std::string("ROipayload.txt"));
  addParam("ROIpayloadName", m_ROIpayloadName, "name of the payload of ROIs", std::string(""));

}

ROIReadTestModule::~ROIReadTestModule()
{
}


void ROIReadTestModule::initialize()
{

  StoreObjPtr<ROIpayload> roiPayloads;
  roiPayloads.isRequired(m_ROIpayloadName);

  m_pFile = fopen(m_outfileName.c_str(), "w");
}

void ROIReadTestModule::beginRun()
{
}


void ROIReadTestModule::event()
{

  StoreObjPtr<ROIpayload> payloadPtr(m_ROIpayloadName);
  int length = payloadPtr->getLength();
  unsigned char* rootdata = (unsigned char*) payloadPtr->getRootdata();

  for (int i = 0; i < 4 * length; i++) {
    //    if ((i % 4) == 0) printf(" ");
    //    printf("%02x", rootdata[i]);
    fputc(rootdata[i], m_pFile);
  }

  //  cout << endl;

}


void ROIReadTestModule::endRun()
{
}


void ROIReadTestModule::terminate()
{
  fclose(m_pFile);
}

