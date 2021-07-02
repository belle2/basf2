/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <alignment/modules/AlignmentDQM/AlignDQMEventProcessor.h>
#include <alignment/modules/AlignmentDQM/AlignDQMModule.h>

using namespace std;
using namespace Belle2;

void AlignDQMEventProcessor::FillCommonHistograms()
{
  DQMEventProcessorBase::FillCommonHistograms();

  (dynamic_cast<AlignDQMModule*>(m_histoModule))->FillPositionSensors(m_residual_um, m_position, m_sensorIndex);
  (dynamic_cast<AlignDQMModule*>(m_histoModule))->FillLayers(m_residual_um, m_phi_deg, m_theta_deg, m_layerIndex);
}
