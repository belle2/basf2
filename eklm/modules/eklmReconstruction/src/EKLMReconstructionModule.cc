
#include <framework/core/ModuleManager.h>

#include <eklm/geoeklm/EKLMTransformationFactory.h>

#include <eklm/modules/eklmReconstruction/EKLMReconstructionModule.h>
#include <eklm/receklm/EKLMRecon.h>
#include <eklm/geoeklm/GeoEKLMBelleII.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

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
  addParam("StripInformationDB", m_stripInfromationDBFile, "File to read strip information", std::string("/tmp/out.dat"));
}

EKLMReconstructionModule::~EKLMReconstructionModule()
{
}

void EKLMReconstructionModule::initialize()
{
  B2INFO("EKLMReconstructionModule initialized");
  (EKLMTransformationFactory::getInstance())->readFromFile(m_stripInfromationDBFile.c_str());
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
