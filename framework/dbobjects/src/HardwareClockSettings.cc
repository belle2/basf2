/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gian Luca Pinna Angioni                                  *
 *                                                                        *
 * Database object containing the nominal RF value and                    *
 * the prescales to derive the clock frequencies of the sub-detectors.     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/dbobjects/HardwareClockSettings.h>

using namespace Belle2;

HardwareClockSettings::HardwareClockSettings()
{

}

Float_t  HardwareClockSettings::getClockFrequency(Const::EDetector detector, std::string label) const
{
  return  m_acceleratorRF / 4. / m_prescaleMap.at(detector).at(label);
}


Float_t  HardwareClockSettings::getGlobalClockFrequency() const
{
  return  m_acceleratorRF / 4.;
}


Float_t  HardwareClockSettings::getAcceleratorRF() const
{
  return  m_acceleratorRF;
}


Int_t HardwareClockSettings::getClockPrescale(Const::EDetector detector, std::string label) const
{
  return m_prescaleMap.at(detector).at(label);
}


void HardwareClockSettings::setClockPrescale(const Const::EDetector detector, std::string label, Int_t prescale)
{
  m_prescaleMap[detector][label] = prescale;
}


void HardwareClockSettings::setAcceleratorRF(Float_t acceleratorRF)
{
  m_acceleratorRF = acceleratorRF;
}
