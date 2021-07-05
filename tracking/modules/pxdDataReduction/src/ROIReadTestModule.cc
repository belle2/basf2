/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

  addParam("outfileName", m_outfileName, "name of the output file", std::string("ROIpayload.txt"));
  addParam("ROIpayloadName", m_ROIpayloadName, "name of the payload of ROIs", std::string(""));

}


void ROIReadTestModule::initialize()
{

  StoreObjPtr<ROIpayload> roiPayloads;
  roiPayloads.isRequired(m_ROIpayloadName);

  m_pFile = fopen(m_outfileName.c_str(), "w+");
  if (!m_pFile) {
    B2FATAL("Could not open " << m_outfileName);
  }
}


void ROIReadTestModule::event()
{

  StoreObjPtr<ROIpayload> payloadPtr(m_ROIpayloadName);
  int length = payloadPtr->getLength();
  unsigned char* rootdata = (unsigned char*) payloadPtr->getRootdata();

  if (!m_pFile) return;

  for (int i = 0; i < 4 * length; i++) {
    //    if ((i % 4) == 0) printf(" ");
    //    printf("%02x", rootdata[i]);
    fputc(rootdata[i], m_pFile);
  }

  //  cout << endl;

}


void ROIReadTestModule::terminate()
{
  fclose(m_pFile);
  m_pFile = nullptr;
}

