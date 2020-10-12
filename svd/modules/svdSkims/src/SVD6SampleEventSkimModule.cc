/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdSkims/SVD6SampleEventSkimModule.h>
#include <svd/dataobjects/SVDEventInfo.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVD6SampleEventSkim)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVD6SampleEventSkimModule::SVD6SampleEventSkimModule() : Module()
{
// Set module properties
  setDescription("Skim Module for the SVD 6-sample acquired events, in either the 6-sample only or the 3-mixed-6 sample DAQ modes. ");
  setPropertyFlags(c_ParallelProcessingCertified);

}


void SVD6SampleEventSkimModule::event()
{

  int retvalue = 0;

  //first check SVDEventInfo name
  StoreObjPtr<SVDEventInfo> temp_eventinfo("SVDEventInfo");
  std::string m_svdEventInfoName = "SVDEventInfo";
  if (!temp_eventinfo.isValid())
    m_svdEventInfoName = "SVDEventInfoSim";
  StoreObjPtr<SVDEventInfo> eventinfo(m_svdEventInfoName);
  if (!eventinfo) B2ERROR("No SVDEventInfo!");

  if (eventinfo->getNSamples() == 6)
    retvalue = 1;

  setReturnValue(retvalue);

}




