/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Timofey Uglov                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/



#include <framework/core/ModuleManager.h>

#include <eklm/modules/eklmDigitization/EKLMDigitizationModule.h>
#include <eklm/simeklm/EKLMDigitizer.h>
#include <eklm/geoeklm/EKLMTransformationFactory.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EKLMDigitization)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EKLMDigitizationModule::EKLMDigitizationModule() : Module()
{
  setDescription("EKLM digitization module");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
  addParam("DiscriminatorThreshold", m_discriminatorThreshold, "Strip hits with npe lower this value will be marked as bad", double(7.));
  addParam("StripInformationDB", m_stripInfromationDBFile, "File to read strip information", std::string("/tmp/out.dat"));
}

EKLMDigitizationModule::~EKLMDigitizationModule()
{
}

void EKLMDigitizationModule::initialize()
{
  B2INFO("EKLMDigitizationModule initialized");
  (EKLMTransformationFactory::getInstance())->readFromFile(m_stripInfromationDBFile.c_str());
}

void EKLMDigitizationModule::beginRun()
{
  B2DEBUG(1, "EKLMDigitizationModule : beginRun");
}

void EKLMDigitizationModule::event()
{
  B2DEBUG(1, "EKLMDigitizationModule : event");
  B2DEBUG(1, " START DIGITIZATION");
  EKLMDigitizer digi;
  digi.readAndSortStepHits();
  digi.makeSimHits();
  digi.readAndSortSimHits();
  digi.mergeSimHitsToStripHits(m_discriminatorThreshold);
}


void EKLMDigitizationModule::endRun()
{
}

void EKLMDigitizationModule::terminate()
{
}
