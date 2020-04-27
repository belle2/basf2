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

  (dynamic_cast<AlignDQMModule*>(m_histoModule))->FillSensorIndex(m_residUPlaneRHUnBias, m_residVPlaneRHUnBias, m_posU, m_posV,
      m_sensorIndex);
  (dynamic_cast<AlignDQMModule*>(m_histoModule))->FillLayers(m_residUPlaneRHUnBias, m_residVPlaneRHUnBias, m_fPosSPU, m_fPosSPV,
                                                             m_layerIndex);
}