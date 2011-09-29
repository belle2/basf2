
#include <framework/core/ModuleManager.h>

#include <eklm/modules/eklmDigitization/EKLMDigitizationModule.h>
#include <eklm/simeklm/EKLMDigitizer.h>

using namespace std;
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

}

EKLMDigitizationModule::~EKLMDigitizationModule()
{
}

void EKLMDigitizationModule::initialize()
{
  B2INFO("EKLMDigitizationModule initialized");
}

void EKLMDigitizationModule::beginRun()
{
  B2DEBUG(1, "EKLMDigitizationModule : beginRun");
}

void EKLMDigitizationModule::event()
{
  B2DEBUG(1, "EKLMDigitizationModule : event");

  B2DEBUG(1, " START DIGITIZATION");


  EKLMDigitizer *digi = new EKLMDigitizer();
  digi->readSimHits();
  digi->sortSimHits();
  digi->mergeSimHitsToStripHits();
  delete digi;
  B2DEBUG(1, " DELETE DIGITIZER OBJECT");
}


void EKLMDigitizationModule::endRun()
{
}

void EKLMDigitizationModule::terminate()
{
}
