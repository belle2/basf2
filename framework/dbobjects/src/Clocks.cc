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

Float_t  Clocks::getClockFrequency(Const::EDetector detector, std::string label) const
{
  return  m_AcceleratorRF / 4. / prescaleMap.at(detector).at(label);
}


Float_t  Clocks::getGlobalClockFrequency() const
{
  return  m_AcceleratorRF / 4.;
}


Float_t  Clocks::getAcceleratorRF() const
{
  return  m_AcceleratorRF;
}


Int_t Clocks::getClockPrescale(Const::EDetector detector, std::string label) const
{
  return prescaleMap.at(detector).at(label);
}


void Clocks::setClockPrescale(const Const::EDetector detector, std::string label, Int_t prescale)
{
  prescaleMap[detector][label] = prescale;
}


void Clocks::setAcceleratorRF(Float_t AcceleratorRF)
{
  m_AcceleratorRF = AcceleratorRF;
}
