
#include <framework/core/ModuleManager.h>

#include <bklm/modules/bklmReconstruction/BKLMReconstructionModule.h>
#include <bklm/geobklm/GeoBKLMBelleII.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BKLMReconstruction)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BKLMReconstructionModule::BKLMReconstructionModule() : Module()
{
  setDescription("BKLM reconstruction simple module for tests");

}

BKLMReconstructionModule::~BKLMReconstructionModule()
{
}

void BKLMReconstructionModule::initialize()
{
  B2INFO("BKLMReconstructionModule initialized");
}

void BKLMReconstructionModule::beginRun()
{
  B2DEBUG(1, "BKLMReconstructionModule : beginRun");
}

void BKLMReconstructionModule::event()
{
  B2INFO("BKLMReconstructionModule::event() called")
}

void BKLMReconstructionModule::endRun()
{
}

void BKLMReconstructionModule::terminate()
{
}
