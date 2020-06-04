/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <simulation/dbtools/ROIfindingConditionFromDBModule.h>
#include <framework/core/ModuleParam.templateDetails.h>

using namespace Belle2;

REG_MODULE(ROIfindingConditionFromDB)

ROIfindingConditionFromDBModule::ROIfindingConditionFromDBModule() : Module(), m_roiParameters()
{
  setDescription("Module which sets its return value based on the payload whether ROI-finding was enabled for the given run/exp interval or not.");
  setPropertyFlags(Module::EModulePropFlags::c_ParallelProcessingCertified);
}

void ROIfindingConditionFromDBModule::event()
{
  if (m_roiParameters.hasChanged()) {
    if (m_roiParameters) {
      m_roiEnabled = m_roiParameters->getROIfinding();
    } else {
      B2ERROR("No configuration for the current run found");
    }
  }
  setReturnValue(m_roiEnabled);
}

