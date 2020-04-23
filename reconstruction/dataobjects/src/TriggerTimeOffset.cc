/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <reconstruction/dataobjects/TriggerTimeOffset.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <TRandom.h>
#include <iostream>

using namespace Belle2;

TriggerTimeOffset::TriggerTimeOffset()
{
  globalClock = 2.;//m_clock->getGlobalClock();
  triggerBitPosWrtRevo9 = gRandom->Integer(revo9nbit);
  phase = gRandom->Uniform(1);
}

Float_t TriggerTimeOffset::getClock(Const::EDetector detector, std::string label) //del
{
  return m_clock->getClock(detector, label);
}

//Float_t TriggerTimeOffset::getPhase(const Const::EDetector detector)
//{
// return phase;
//}

Float_t TriggerTimeOffset::getTriggerBit(Const::EDetector detector, std::string label)
{
  return triggerBitPosWrtRevo9 % m_clock->getISub(detector, label);
}

Int_t TriggerTimeOffset::getTriggerBitWrtRevo9()
{
  return triggerBitPosWrtRevo9;
}
