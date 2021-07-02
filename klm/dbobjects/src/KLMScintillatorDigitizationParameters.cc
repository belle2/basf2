/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/KLMScintillatorDigitizationParameters.h>

using namespace Belle2;

KLMScintillatorDigitizationParameters::KLMScintillatorDigitizationParameters() :
  m_ADCRange(0),
  m_ADCSamplingTDCPeriods(0),
  m_NDigitizations(0),
  m_ADCPedestal(0),
  m_ADCPEAmplitude(0),
  m_ADCThreshold(0),
  m_ADCSaturation(0),
  m_NPEperMeV(0),
  m_MinCosTheta(0),
  m_MirrorReflectiveIndex(0),
  m_ScintillatorDeExcitationTime(0),
  m_FiberDeExcitationTime(0),
  m_FiberLightSpeed(0),
  m_AttenuationLength(0),
  m_PEAttenuationFrequency(0),
  m_MeanSiPMNoise(0),
  m_EnableConstBkg(false)
{
}

KLMScintillatorDigitizationParameters::~KLMScintillatorDigitizationParameters()
{
}
