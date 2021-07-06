/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdSimulation/SVDTriggerQualityGeneratorModule.h>
#include <root/TRandom.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDTriggerQualityGenerator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
SVDTriggerQualityGeneratorModule::SVDTriggerQualityGeneratorModule() : Module()
{
  B2DEBUG(1, "Constructor");
  // Set module properties
  setDescription("Generates a StoreObjPtr that contains random trigger quality chosen between FINE and CORSE");

  // Parameter definitions
  addParam("TRGSummaryName", m_trgQRGName, "TRGSummary name", m_trgQRGName);
}

SVDTriggerQualityGeneratorModule::~SVDTriggerQualityGeneratorModule()
{
}


void SVDTriggerQualityGeneratorModule::initialize()
{

  B2DEBUG(10, "TRGSummaryName: " << m_trgQRGName);

  //Register the new TRGQualityRG StoreObjPtr
  m_triggerQRG.registerInDataStore(m_trgQRGName, DataStore::c_ErrorIfAlreadyRegistered);
}


void SVDTriggerQualityGeneratorModule::beginRun()
{
}

void SVDTriggerQualityGeneratorModule::event()
{
  m_triggerQRG.create();
  if (gRandom->Integer(2) == 0)
    m_triggerQRG->setTimQuality(TRGSummary::TTYQ_CORS);
  else
    m_triggerQRG->setTimQuality(TRGSummary::TTYQ_FINE);
}

void SVDTriggerQualityGeneratorModule::endRun()
{
}

void SVDTriggerQualityGeneratorModule::terminate()
{
}
