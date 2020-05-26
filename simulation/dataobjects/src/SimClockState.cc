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
  globalClockFrequency = m_clock->getGlobalClockFrequency();
}

void SimClockState::update()
{
  revo9Status = gRandom->Integer(revo9nbit);
}

Float_t SimClockState::getClockFrequency(Const::EDetector detector, std::string label)
{
  return m_clock->getClockFrequency(detector, label);
}

Float_t SimClockState::getTriggerOffset(Const::EDetector detector, std::string label)
{
  return (revo9Status % m_clock->getClockPrescale(detector, label)) / (globalClockFrequency * 1e-3);
}

Int_t SimClockState::getRevo9Status()
{
  return revo9Status;
}
