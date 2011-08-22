
#include <framework/core/ModuleManager.h>

#include <bklm/modules/bklmReconstruction/BKLMReconstructionModule.h>
#include <bklm/geometry/GeoBKLMCreator.h>

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
}

void BKLMReconstructionModule::beginRun()
{
}

void BKLMReconstructionModule::event()
{
}

void BKLMReconstructionModule::endRun()
{
}

void BKLMReconstructionModule::terminate()
{
}
