/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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




