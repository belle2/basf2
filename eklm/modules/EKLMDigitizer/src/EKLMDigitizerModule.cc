/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* System headers. */
#include <math.h>

/* Belle2 headers. */
#include <eklm/modules/EKLMDigitizer/EKLMDigitizerModule.h>
#include <framework/core/ModuleManager.h>

using namespace Belle2;

REG_MODULE(EKLMDigitizer)

EKLMDigitizerModule::EKLMDigitizerModule() : Module()
{
  setDescription("EKLM digitization module");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("DiscriminatorThreshold", m_discriminatorThreshold,
           "Strip hits with npe lower this value will be marked as bad",
           double(7.));
  addParam("CreateSim2Hits", m_CreateSim2Hits,
           "Create merged EKLMSim2Hits", bool(false));
  addParam("Debug", m_digPar.debug,
           "Debug mode (generates additional output files with histograms).",
           bool(false));
}

EKLMDigitizerModule::~EKLMDigitizerModule()
{
}

void EKLMDigitizerModule::initialize()
{
  StoreArray<EKLMDigit>::registerPersistent();
  if (m_CreateSim2Hits)
    StoreArray<EKLMSim2Hit>::registerPersistent();
  EKLM::setDefDigitizationParams(&m_digPar);
}

void EKLMDigitizerModule::beginRun()
{
}

void EKLMDigitizerModule::event()
{
  EKLM::Digitizer digi(&m_digPar);
  digi.readAndSortSimHits();
  if (m_CreateSim2Hits)
    digi.makeSim2Hits();
  digi.mergeSimHitsToStripHits(m_discriminatorThreshold);
}

void EKLMDigitizerModule::endRun()
{
}

void EKLMDigitizerModule::terminate()
{
}
