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
  addParam("Debug", m_digPar.debug,
           "Debug mode (generates additional output files with histograms).",
           bool(false));
}

EKLMDigitizerModule::~EKLMDigitizerModule()
{
}

void EKLMDigitizerModule::initialize()
{
  StoreArray<EKLMSim2Hit>::registerPersistent();
  StoreArray<EKLMDigit>::registerPersistent();
  if (m_geoDat.read() != 0)
    B2FATAL("Cannot read geometry data file.");
  EKLM::transformsToGlobal(&m_geoDat.transf);
  EKLM::setDefDigitizationParams(&m_digPar);
}

void EKLMDigitizerModule::beginRun()
{
}

void EKLMDigitizerModule::event()
{
  EKLM::Digitizer digi(&m_geoDat, &m_digPar);
  digi.readAndSortSimHits();
  digi.makeSimHits();
  digi.readAndSortSim2Hits();
  digi.mergeSimHitsToStripHits(m_discriminatorThreshold);
}

void EKLMDigitizerModule::endRun()
{
}

void EKLMDigitizerModule::terminate()
{
}
