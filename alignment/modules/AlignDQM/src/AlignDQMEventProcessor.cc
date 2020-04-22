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