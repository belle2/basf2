/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMDigitTimeShifter/KLMDigitTimeShifterModule.h>

/** C++ headers. */
#include <cstdint>

using namespace Belle2;

REG_MODULE(KLMDigitTimeShifter)

KLMDigitTimeShifterModule::KLMDigitTimeShifterModule() :
  Module()
{
  setDescription("Shift the time of KLMDigits according to the cable delay corrections.");
  setPropertyFlags(c_ParallelProcessingCertified);
}

KLMDigitTimeShifterModule::~KLMDigitTimeShifterModule()
{
}

void KLMDigitTimeShifterModule::initialize()
{
  m_Digits.isRequired();
}

void KLMDigitTimeShifterModule::beginRun()
{
  if (!m_TimeDelay.isValid())
    B2FATAL("KLM time cable delay data are not available.");
}

void KLMDigitTimeShifterModule::event()
{
  for (KLMDigit& digit : m_Digits) {
    KLMChannelNumber channel = digit.getUniqueChannelID();
    float digitTimeShifted = digit.getTime() - m_TimeDelay->getTimeDelay(channel);
    digit.setTime(digitTimeShifted);
  }
}
