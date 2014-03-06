/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* System headers. */
#include <fcntl.h>
#include <unistd.h>

/* Belle2 headers. */
#include <framework/core/ModuleManager.h>
#include <eklm/geometry/GeometryData.h>
#include <eklm/modules/EKLMAlignment/EKLMAlignmentModule.h>

using namespace Belle2;

REG_MODULE(EKLMAlignment)

EKLMAlignmentModule::EKLMAlignmentModule() : Module()
{
  setDescription("Module for generation of EKLM transformation and "
                 "alignment data.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("OutputFile", m_out, "Output file.",
           std::string("eklm_alignment.dat"));
}

EKLMAlignmentModule::~EKLMAlignmentModule()
{
}

void EKLMAlignmentModule::initialize()
{
}

void EKLMAlignmentModule::beginRun()
{
}

void EKLMAlignmentModule::event()
{
}

void EKLMAlignmentModule::endRun()
{
  EKLM::GeometryData dat;
  if (dat.save(m_out.c_str()) != 0)
    B2ERROR("Cannot save geometry data.");
}

void EKLMAlignmentModule::terminate()
{
}

