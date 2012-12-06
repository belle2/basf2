/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Kirill Chilikin                                         *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/

/* Belle2 headers. */
#include <framework/core/ModuleManager.h>
#include <eklm/geoeklm/TransformData.h>
#include <eklm/modules/eklmAlignment/EKLMAlignmentModule.h>

using namespace Belle2;

REG_MODULE(EKLMAlignment)

EKLMAlignmentModule::EKLMAlignmentModule() : Module()
{
  setDescription("Module for generation of EKLM transformation and "
                 "alignment data.");
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
  struct EKLM::TransformData dat;
  EKLM::fillTransforms(&dat);
  if (EKLM::writeTransforms(&dat, m_out.c_str()) != 0) {
    B2ERROR("EKLMAlignmentModule: cannot fill output file.");
    return;
  }
  B2INFO("EKLMAlignmentModule: generated data file " << m_out);
}

void EKLMAlignmentModule::terminate()
{
}

