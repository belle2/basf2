#include <eklm/modules/eklmDigitalizer/EKLMDigitalizerModule.h>
#include <framework/core/ModuleManager.h>


#include <eklm/eklmhit/EKLMSimHit.h>


using namespace std;
using namespace Belle2;

REG_MODULE(EKLMDigitalizerModule)




EKLMDigitalizerModule::EKLMDigitalizerModule() : Module(string("EKLMDigitalizerModule"))
{
  setDescription("Hit reader simple module for tests");
  addParam("InputColName", m_inColName, string("EKLMSD_Collection"), "Input collection name");

}

EKLMDigitalizerModule::~EKLMDigitalizerModule()
{
}

void EKLMDigitalizerModule::initialize()
{
  DEBUG(1, "EKLMDigitalizerModule initialized");
}

void EKLMDigitalizerModule::beginRun()
{
  DEBUG(1, "EKLMDigitalizerModule : beginRun");
}

void EKLMDigitalizerModule::event()
{



}

void EKLMDigitalizerModule::endRun()
{
}

void EKLMDigitalizerModule::terminate()
{
}
