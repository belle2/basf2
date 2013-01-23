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
#include <eklm/modules/EKLMK0LReconstructor/EKLMK0LReconstructorModule.h>

using namespace Belle2;

REG_MODULE(EKLMK0LReconstructor)

EKLMK0LReconstructorModule::EKLMK0LReconstructorModule() : Module()
{
  setDescription("EKLM K0L reconstruction module.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
}

EKLMK0LReconstructorModule::~EKLMK0LReconstructorModule()
{
}

void EKLMK0LReconstructorModule::initialize()
{
}

void EKLMK0LReconstructorModule::beginRun()
{
}

void EKLMK0LReconstructorModule::event()
{
}

void EKLMK0LReconstructorModule::endRun()
{
}

void EKLMK0LReconstructorModule::terminate()
{
}

