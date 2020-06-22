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

#include <alignment/modules/AlignmentDQM/AlignDQMEventProcessor.h>
#include <alignment/modules/AlignmentDQM/AlignDQMModule.h>

using namespace Belle2;

void AlignDQMEventProcessor::FillCommonHistograms()
{
  DQMEventProcessorBase::FillCommonHistograms();

  (dynamic_cast<AlignDQMModule*>(m_histoModule))->FillPositionSensors(m_residual_um, m_position, m_sensorIndex);
  (dynamic_cast<AlignDQMModule*>(m_histoModule))->FillLayers(m_residual_um, m_phi_deg, m_theta_deg, m_layerIndex);
}