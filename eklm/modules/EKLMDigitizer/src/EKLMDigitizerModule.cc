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
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
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
  StoreArray<EKLMSimHit>::registerPersistent();
  StoreArray<EKLMDigit>::registerPersistent();
  if (m_geoDat.read() != 0)
    B2FATAL("Cannot read geometry data file.");
  EKLM::transformsToGlobal(&m_geoDat.transf);
  /* Fill digitization parameters. */
  m_digPar.ADCSamplingTime = 1.0;
  m_digPar.nDigitizations = 200;
  m_digPar.nPEperMeV = 22;
  m_digPar.minCosTheta = cos(26.7 / 180.0 * M_PI);
  m_digPar.mirrorReflectiveIndex = 0.95;
  m_digPar.scintillatorDeExcitationTime = 3.0;
  m_digPar.fiberDeExcitationTime = 10.0;
  m_digPar.firstPhotonlightSpeed = 17.0;
  m_digPar.attenuationLength = 300.0;
  m_digPar.PEAttenuationFreq = 3.0;
  m_digPar.meanSiPMNoise = -1;
  m_digPar.enableConstBkg = false;
}

void EKLMDigitizerModule::beginRun()
{
}

void EKLMDigitizerModule::event()
{
  EKLM::Digitizer digi(&m_geoDat, &m_digPar);
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
