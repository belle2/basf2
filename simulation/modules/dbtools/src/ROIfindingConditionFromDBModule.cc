/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <simulation/modules/dbtools/ROIfindingConditionFromDBModule.h>
#include <framework/core/ModuleParam.templateDetails.h>

using namespace Belle2;

REG_MODULE(ROIfindingConditionFromDB);

ROIfindingConditionFromDBModule::ROIfindingConditionFromDBModule() : Module(), m_ROISimulationParameters()
{
  setDescription("Module which sets its return value based on the payload whether ROI-finding was enabled for the given run/exp interval or not.");
  setPropertyFlags(Module::EModulePropFlags::c_ParallelProcessingCertified);
}

void ROIfindingConditionFromDBModule::event()
{
  if (m_ROISimulationParameters.hasChanged()) {
    if (m_ROISimulationParameters) {
      m_roiEnabled = m_ROISimulationParameters->getROIfinding();
    } else {
      B2ERROR("No configuration for the current run found");
    }
  }
  setReturnValue(m_roiEnabled);
}

