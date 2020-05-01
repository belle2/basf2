/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jachym Bartik                                            *
 *                                                                        *
 * Prepared for Alignment DQM                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/AlignDQM/AlignDQMEventProcessor.h>
#include <alignment/modules/AlignDQM/AlignDQMModule.h>

using namespace Belle2;

void AlignDQMEventProcessor::FillCommonHistograms()
{
  DQMEventProcessorBase::FillCommonHistograms();

  (dynamic_cast<AlignDQMModule*>(m_histoModule))->FillPositionSensors(m_UBResidualU_um, m_UBResidualV_um, m_positionU, m_positionV,
      m_sensorIndex);
  (dynamic_cast<AlignDQMModule*>(m_histoModule))->FillLayers(m_UBResidualU_um, m_UBResidualV_um, m_phi_deg, m_theta_deg,
                                                             m_layerIndex);
}