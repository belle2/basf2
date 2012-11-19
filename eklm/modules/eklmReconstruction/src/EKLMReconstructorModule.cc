
#include <framework/core/ModuleManager.h>

#include <eklm/geoeklm/EKLMTransformationFactory.h>

#include <eklm/modules/eklmReconstruction/EKLMReconstructorModule.h>
#include <eklm/receklm/EKLMRecon.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EKLMReconstructor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EKLMReconstructorModule::EKLMReconstructorModule() : Module()
{

  setDescription("EKLM reconstruction simple module for tests");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
  addParam("StripInformationDB", m_stripInfromationDBFile,
           "File to read strip information", std::string("/tmp/out.dat"));
}

EKLMReconstructorModule::~EKLMReconstructorModule()
{
}

void EKLMReconstructorModule::initialize()
{
  StoreArray<EKLMSectorHit>::registerPersistent();
  StoreArray<EKLMHit2d>::registerPersistent();
  (EKLMTransformationFactory::getInstance())->
  readFromFile(m_stripInfromationDBFile.c_str());
  B2INFO("EKLMReconstructorModule initialized");
}

void EKLMReconstructorModule::beginRun()
{
  B2DEBUG(1, "EKLMReconstructorModule : beginRun");
}

void EKLMReconstructorModule::event()
{

  EKLMRecon* recon = new EKLMRecon();
  B2INFO("EKLMReconstructorModule::event() called")
  recon->readStripHits();
  recon->createSectorHits();
  recon->create2dHits();
  delete recon;
}

void EKLMReconstructorModule::endRun()
{
}

void EKLMReconstructorModule::terminate()
{
}
