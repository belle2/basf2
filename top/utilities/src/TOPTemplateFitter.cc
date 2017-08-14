/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Weber                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/utilities/TOPTemplateFitter.h>

using namespace Belle2::TOP;

TOPTemplateFitter::TOPTemplateFitter(const TOPRawWaveform& wf,
                                     const TOPSampleTimes& sampleTimes,
                                     const double averageRMS)
  : m_wf(wf), m_sampleTimes(sampleTimes), m_averageRMS(averageRMS)
{
  m_chisq = -1.;
}

void TOPTemplateFitter::PerformTemplateFit(const double risingEdgeStart,
                                           const double fitRange)
{

}
