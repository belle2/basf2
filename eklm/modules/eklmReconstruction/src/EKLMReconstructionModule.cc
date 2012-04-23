
#include <framework/core/ModuleManager.h>

#include <eklm/modules/eklmReconstruction/EKLMReconstructionModule.h>
#include <eklm/receklm/EKLMRecon.h>
#include <eklm/geoeklm/GeoEKLMBelleII.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EKLMReconstruction)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EKLMReconstructionModule::EKLMReconstructionModule() : Module()
{
  setDescription("EKLM reconstruction simple module for tests");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
}

EKLMReconstructionModule::~EKLMReconstructionModule()
{
}

void EKLMReconstructionModule::initialize()
{
  B2INFO("EKLMReconstructionModule initialized");
}

void EKLMReconstructionModule::beginRun()
{
  B2DEBUG(1, "EKLMReconstructionModule : beginRun");
}

void EKLMReconstructionModule::event()
{

  EKLMRecon* recon = new EKLMRecon();
  B2INFO("EKLMReconstructionModule::event() called")
  recon->readStripHits();
  recon->createSectorHits();
  recon->create2dHits();
  delete recon;
}

void EKLMReconstructionModule::endRun()
{
}

void EKLMReconstructionModule::terminate()
{
}
