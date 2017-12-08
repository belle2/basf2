/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tzu-An Sheng                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <trg/cdc/modules/unpacker/CDCTriggerUnpackerModule.h>


using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCTriggerUnpacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCTriggerUnpackerModule::CDCTriggerUnpackerModule() : Module(), m_rawTrigger("RawTRGs")
{
  // Set module properties
  setDescription("Unpack the data recorded in B2L");

  // Parameter definitions
  addParam("merger", m_merger, "flag to unpack merger data (recorded by Merger Reader / TSF)", false);

}

void CDCTriggerUnpackerModule::initialize()
{
  m_rawTrigger.isRequired();
}

void CDCTriggerUnpackerModule::event()
{
}


