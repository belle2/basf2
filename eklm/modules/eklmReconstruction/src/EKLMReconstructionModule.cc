
#include <framework/core/ModuleManager.h>

#include <eklm/modules/eklmReconstruction/EKLMReconstructionModule.h>
#include <eklm/receklm/EKLMRecon.h>
#include <eklm/geoeklm/GeoEKLMBelleII.h>
#include <eklm/eklmutils/EKLMutils.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EKLMReconstructionModule, "EKLMReconstruction")

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EKLMReconstructionModule::EKLMReconstructionModule() : Module()
{
  setDescription("Eklm reconstruction simple module for tests");

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

  EKLMRecon * recon = new EKLMRecon();
  B2INFO("EKLMReconstructionModule::event() called")
  recon->readStripHits();
  recon->createSectorHits();
  recon->create2dHits();
  recon->store2dHits();

  delete recon;
}

void EKLMReconstructionModule::endRun()
{
}

void EKLMReconstructionModule::terminate()
{
}
