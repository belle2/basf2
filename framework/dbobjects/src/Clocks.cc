/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/dbobjects/Clocks.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <TRandom.h>

using namespace Belle2;

Clocks::Clocks()
{

}

Float_t  Clocks::getClock(Const::EDetector detector, std::string label) const
{
  if (iSubMap.count(detector) == 0) {
    throw "Division by zero condition!";
  }
  return  m_globalClock / iSubMap.at(detector).at(label);
}


Float_t  Clocks::getGlobalClock() const
{
  return  m_globalClock;
}


Int_t Clocks::getISub(Const::EDetector detector, std::string label) const
{
  return iSubMap.at(detector).at(label);
}


void Clocks::setClock(const Const::EDetector detector, std::string label, Int_t iSub)
{

  if (iSubMap.count(detector) == 0) {
    iSubMap[detector] = std::map<std::string, int>();
  }
  iSubMap[detector][label] = iSub;
}


void Clocks::setGlobalClock(Float_t globalClock)
{
  m_globalClock = globalClock;
}
