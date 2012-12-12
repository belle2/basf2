/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/modules/eklmDigitization/EKLMDigitizerModule.h>
#include <eklm/simulation/Digitizer.h>
#include <framework/core/ModuleManager.h>

using namespace Belle2;

REG_MODULE(EKLMDigitizer)

EKLMDigitizerModule::EKLMDigitizerModule() : Module()
{
  setDescription("EKLM digitization module");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
  addParam("DiscriminatorThreshold", m_discriminatorThreshold,
           "Strip hits with npe lower this value will be marked as bad",
           double(7.));
}

EKLMDigitizerModule::~EKLMDigitizerModule()
{
}

void EKLMDigitizerModule::initialize()
{
  StoreArray<EKLMSimHit>::registerPersistent();
  StoreArray<EKLMDigit>::registerPersistent();
  if (EKLM::readTransforms(&m_transf) != 0)
    B2FATAL("Cannot read transformation data file.");
  EKLM::transformsToGlobal(&m_transf);
  B2INFO("EKLMDigitizationModule initialized");
}

void EKLMDigitizerModule::beginRun()
{
  B2DEBUG(1, "EKLMDigitizationModule : beginRun");
}

void EKLMDigitizerModule::event()
{
  B2DEBUG(1, "EKLMDigitizationModule : event");
  B2DEBUG(1, " START DIGITIZATION");
  EKLM::Digitizer digi(&m_transf);
  digi.readAndSortStepHits();
  digi.makeSimHits();
  digi.readAndSortSimHits();
  digi.mergeSimHitsToStripHits(m_discriminatorThreshold);
}

void EKLMDigitizerModule::endRun()
{
}

void EKLMDigitizerModule::terminate()
{
}
