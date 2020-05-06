/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gian Luca Pinna Angioni                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <simulation/dataobjects/TriggerTimeOffset.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <TRandom.h>
#include <iostream>

using namespace Belle2;

TriggerTimeOffset::TriggerTimeOffset()
{
  globalClock = m_clock->getGlobalClock();
}

void TriggerTimeOffset::update()
{
  triggerBitPosWrtRevo9 = gRandom->Integer(revo9nbit);
}

Float_t TriggerTimeOffset::getClock(Const::EDetector detector, std::string label) //del
{
  return m_clock->getClock(detector, label);
}

Float_t TriggerTimeOffset::getTriggerOffset(Const::EDetector detector, std::string label)
{
  return (triggerBitPosWrtRevo9 % m_clock->getISub(detector, label)) / (globalClock * 1e-3);
}

Int_t TriggerTimeOffset::getTriggerBitWrtRevo9()
{
  return triggerBitPosWrtRevo9;
}
