/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Timofey Uglov, Kirill Chilikin                          *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/

#include <framework/core/ModuleManager.h>
#include <eklm/modules/eklmReconstruction/EKLMReconstructorModule.h>
#include <eklm/reconstruction/Reconstructor.h>

using namespace Belle2;

static const char MemErr[] = "Memory allocation error.";

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
}

EKLMReconstructorModule::~EKLMReconstructorModule()
{
}

void EKLMReconstructorModule::initialize()
{
  StoreArray<EKLMSectorHit>::registerPersistent();
  StoreArray<EKLMHit2d>::registerPersistent();
  if (EKLM::readTransforms(&m_transf) != 0)
    B2FATAL("Cannot read transformation data file.");
  EKLM::transformsToGlobal(&m_transf);
  B2INFO("EKLMReconstructorModule initialized");
}

void EKLMReconstructorModule::beginRun()
{
  B2DEBUG(1, "EKLMReconstructorModule : beginRun");
}

void EKLMReconstructorModule::event()
{
  EKLM::Reconstructor* recon;
  try {
    recon = new EKLM::Reconstructor(&m_transf);
  } catch (std::bad_alloc& ba) {
    B2FATAL(MemErr);
  }
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
