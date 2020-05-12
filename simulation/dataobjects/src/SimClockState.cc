/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gian Luca Pinna Angioni                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <simulation/dataobjects/SimClockState.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <TRandom.h>
#include <iostream>

using namespace Belle2;

SimClockState::SimClockState()
{
  globalClockFreq = m_clock->getGlobalClockFreq();
}

void SimClockState::update()
{
  revo9Status = gRandom->Integer(revo9nbit);
}

Float_t SimClockState::getClockFreq(Const::EDetector detector, std::string label)
{
  return m_clock->getClockFreq(detector, label);
}

Float_t SimClockState::getTriggerOffset(Const::EDetector detector, std::string label)
{
  return (revo9Status % m_clock->getClockPrescale(detector, label)) / (globalClockFreq * 1e-3);
}

Int_t SimClockState::getRevo9Status()
{
  return revo9Status;
}
