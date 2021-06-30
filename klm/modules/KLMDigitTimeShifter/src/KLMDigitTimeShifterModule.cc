/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
    uint16_t channel = digit.getUniqueChannelID();
    float digitTimeShifted = digit.getTime() - m_TimeDelay->getTimeDelay(channel);
    digit.setTime(digitTimeShifted);
  }
}
