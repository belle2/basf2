/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gian Luca Pinna Angioni                                  *
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

Float_t  Clocks::getClockFreq(Const::EDetector detector, std::string label) const
{
  if (prescaleMap.count(detector) == 0) {
    throw "Detector not found";
  }
  return  m_globalClockFreq / prescaleMap.at(detector).at(label);
}


Float_t  Clocks::getGlobalClockFreq() const
{
  return  m_globalClockFreq;
}


Int_t Clocks::getClockPrescale(Const::EDetector detector, std::string label) const
{
  return prescaleMap.at(detector).at(label);
}


void Clocks::setClockPrescale(const Const::EDetector detector, std::string label, Int_t prescale)
{

  if (prescaleMap.count(detector) == 0) {
    prescaleMap[detector] = std::map<std::string, int>();
  }
  prescaleMap[detector][label] = prescale;
}


void Clocks::setGlobalClockFreq(Float_t globalClockFreq)
{
  m_globalClockFreq = globalClockFreq;
}
