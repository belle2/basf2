/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gian Luca Pinna Angioni                                  *
 *                                                                        *
 * Database object containing the nominal RF value and                    *
 * the prescales to derive the clock frequencies of the sub-detectors.    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/dbobjects/HardwareClockSettings.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

HardwareClockSettings::HardwareClockSettings() {}

Double_t  HardwareClockSettings::getClockFrequency(Const::EDetector detector, std::string label) const
{
  bool isDetectorInPrescaleMap = true;

  if (m_prescaleMap.find(detector) == m_prescaleMap.end()) isDetectorInPrescaleMap = false;
  else if (m_prescaleMap.at(detector).find(label) != m_prescaleMap.at(detector).end())
    return  m_acceleratorRF / 4. / m_prescaleMap.at(detector).at(label);

  if (m_clocksMap.find(detector) == m_clocksMap.end()) {
    if (!isDetectorInPrescaleMap) B2ERROR("No clocks available for the given detector");
    else  B2ERROR("Clock named " << label << " not available");
  } else if (m_clocksMap.at(detector).find(label) != m_clocksMap.at(detector).end())
    return  m_clocksMap.at(detector).at(label);
  else B2ERROR("Clock named " << label << " not available");

  return std::numeric_limits<float>::quiet_NaN();
}


Double_t  HardwareClockSettings::getGlobalClockFrequency() const
{
  return  m_acceleratorRF / 4.;
}


Double_t  HardwareClockSettings::getAcceleratorRF() const
{
  return  m_acceleratorRF;
}


Int_t HardwareClockSettings::getClockPrescale(Const::EDetector detector, std::string label) const
{
  if (m_prescaleMap.find(detector) == m_prescaleMap.end()) B2ERROR("No clocks available for the given detector");
  else if (m_prescaleMap.at(detector).find(label) != m_prescaleMap.at(detector).end())
    return m_prescaleMap.at(detector).at(label);
  else B2ERROR("Clock named " << label << " not available");

  return std::numeric_limits<Int_t>::quiet_NaN();
}

void HardwareClockSettings::setClockPrescale(const Const::EDetector detector, std::string label, Int_t prescale)
{
  m_prescaleMap[detector][label] = prescale;
}


void HardwareClockSettings::setClockFrequency(const Const::EDetector detector, std::string label, float frequency)
{
  m_clocksMap[detector][label] = frequency;
}

void HardwareClockSettings::setAcceleratorRF(Double_t acceleratorRF)
{
  m_acceleratorRF = acceleratorRF;
}
