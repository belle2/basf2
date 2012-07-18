
#include <framework/core/ModuleManager.h>

#include <eklm/modules/eklmStepHitToSimHitConverter/EKLMStepHitToSimHitConverterModule.h>
#include <eklm/simeklm/EKLMDigitizer.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EKLMStepHitToSimHitConverter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EKLMStepHitToSimHitConverterModule::EKLMStepHitToSimHitConverterModule() : Module()
{
  setDescription("EKLM test module to handle background events");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

}

EKLMStepHitToSimHitConverterModule::~EKLMStepHitToSimHitConverterModule()
{
}

void EKLMStepHitToSimHitConverterModule::initialize()
{
  B2INFO("EKLMStepHitToSimHitConverterModule initialized");
}

void EKLMStepHitToSimHitConverterModule::beginRun()
{
  B2DEBUG(1, "EKLMStepHitToSimHitConverterModule : beginRun");
}

void EKLMStepHitToSimHitConverterModule::event()
{
  B2DEBUG(1, "EKLMStepHitToSimHitConverterModule : event");
  B2DEBUG(1, " START CONVERSION");
  EKLMDigitizer* digi = new EKLMDigitizer();
  digi->readAndSortStepHits();
  digi->makeSimHits();
  delete digi;
  B2DEBUG(1, " DELETE DIGITIZER OBJECT");
}


void EKLMStepHitToSimHitConverterModule::endRun()
{
}

void EKLMStepHitToSimHitConverterModule::terminate()
{
}
