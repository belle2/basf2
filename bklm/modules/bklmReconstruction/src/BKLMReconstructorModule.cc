#include <framework/core/ModuleManager.h>

#include <bklm/modules/bklmReconstruction/BKLMReconstructorModule.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BKLMReconstructor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BKLMReconstructorModule::BKLMReconstructorModule() : Module()
{
  setDescription("BKLM reconstruction simple module for tests");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
}

BKLMReconstructorModule::~BKLMReconstructorModule()
{
}

void BKLMReconstructorModule::initialize()
{
}

void BKLMReconstructorModule::beginRun()
{
}

void BKLMReconstructorModule::event()
{
}

void BKLMReconstructorModule::endRun()
{
}

void BKLMReconstructorModule::terminate()
{
}
