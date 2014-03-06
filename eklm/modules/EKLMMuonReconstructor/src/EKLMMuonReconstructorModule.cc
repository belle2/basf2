/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <framework/core/ModuleManager.h>
#include <eklm/modules/EKLMMuonReconstructor/EKLMMuonReconstructorModule.h>

using namespace Belle2;

REG_MODULE(EKLMMuonReconstructor)

EKLMMuonReconstructorModule::EKLMMuonReconstructorModule() : Module()
{
  setDescription("EKLM muon reconstruction module.");
  setPropertyFlags(c_ParallelProcessingCertified);
}

EKLMMuonReconstructorModule::~EKLMMuonReconstructorModule()
{
}

void EKLMMuonReconstructorModule::initialize()
{
}

void EKLMMuonReconstructorModule::beginRun()
{
}

void EKLMMuonReconstructorModule::event()
{
}

void EKLMMuonReconstructorModule::endRun()
{
}

void EKLMMuonReconstructorModule::terminate()
{
}

