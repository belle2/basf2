/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/modules/EKLMReconstructor/EKLMReconstructorModule.h>
#include <eklm/reconstruction/Reconstructor.h>
#include <framework/core/ModuleManager.h>

using namespace Belle2;

REG_MODULE(EKLMReconstructor)

static const char MemErr[] = "Memory allocation error.";

EKLMReconstructorModule::EKLMReconstructorModule() : Module()
{
  setDescription("EKLM reconstruction module.");
  setPropertyFlags(c_ParallelProcessingCertified);
}

EKLMReconstructorModule::~EKLMReconstructorModule()
{
}

void EKLMReconstructorModule::initialize()
{
  StoreArray<EKLMHit2d>::registerPersistent();
  if (m_geoDat.read() != 0)
    B2FATAL("Cannot read geometry data file.");
  EKLM::transformsToGlobal(&m_geoDat.transf);
}

void EKLMReconstructorModule::beginRun()
{
}

void EKLMReconstructorModule::event()
{
  EKLM::Reconstructor recon(&m_geoDat);
  recon.readStripHits();
  recon.createSectorHits();
  recon.create2dHits();
}

void EKLMReconstructorModule::endRun()
{
}

void EKLMReconstructorModule::terminate()
{
}
