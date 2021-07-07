/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/dbobjects/HardwareClockSettings.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <iostream>

using namespace Belle2;

HardwareClockSettings::HardwareClockSettings() {}


bool HardwareClockSettings::isPrescaleAvailable(Const::EDetector detector, std::string label) const
{
  if ((m_prescaleMap.find(detector) == m_prescaleMap.end())
      or (m_prescaleMap.at(detector).find(label) == m_prescaleMap.at(detector).end())) return false;
  else return true;
}

bool HardwareClockSettings::isFrequencyAvailable(Const::EDetector detector, std::string label) const
{
  if (((m_prescaleMap.find(detector) == m_prescaleMap.end()) or
       (m_prescaleMap.at(detector).find(label) == m_prescaleMap.at(detector).end()))
      and ((m_clocksMap.find(detector) == m_clocksMap.end()) or
           (m_clocksMap.at(detector).find(label) == m_clocksMap.at(detector).end()))) return false;
  else return true;
}

double HardwareClockSettings::getClockFrequency(Const::EDetector detector, std::string label) const
{
  bool isDetectorInPrescaleMap = true;

  if (m_prescaleMap.find(detector) == m_prescaleMap.end()) isDetectorInPrescaleMap = false;
  else if (m_prescaleMap.at(detector).find(label) != m_prescaleMap.at(detector).end())
    return  m_acceleratorRF / 4. / m_prescaleMap.at(detector).at(label);

  if (m_clocksMap.find(detector) == m_clocksMap.end()) {
    if (!isDetectorInPrescaleMap) B2ERROR("No clocks available for " << Const::parseDetectors(detector));
    else  B2ERROR("Clock named " << label << " not available for " << Const::parseDetectors(detector));
  } else if (m_clocksMap.at(detector).find(label) != m_clocksMap.at(detector).end())
    return  m_clocksMap.at(detector).at(label);
  else B2ERROR("Clock named " << label << " not available for " << Const::parseDetectors(detector));

  return std::numeric_limits<double>::quiet_NaN();
}

double HardwareClockSettings::getGlobalClockFrequency() const
{
  return  m_acceleratorRF / 4.;
}

double HardwareClockSettings::getAcceleratorRF() const
{
  return  m_acceleratorRF;
}

double HardwareClockSettings::getClockPrescale(Const::EDetector detector, std::string label) const
{
  if (m_prescaleMap.find(detector) == m_prescaleMap.end()) B2ERROR("No clocks available for " << Const::parseDetectors(detector));
  else if (m_prescaleMap.at(detector).find(label) != m_prescaleMap.at(detector).end())
    return m_prescaleMap.at(detector).at(label);
  else B2ERROR("Clock named " << label << " not available for " << Const::parseDetectors(detector));

  return std::numeric_limits<double>::quiet_NaN();
}

void HardwareClockSettings::setClockPrescale(const Const::EDetector detector, std::string label, double prescale)
{
  m_prescaleMap[detector][label] = prescale;
}


void HardwareClockSettings::setClockFrequency(const Const::EDetector detector, std::string label, double frequency)
{
  m_clocksMap[detector][label] = frequency;
}

void HardwareClockSettings::setAcceleratorRF(double acceleratorRF)
{
  m_acceleratorRF = acceleratorRF;
}

void HardwareClockSettings::print() const
{
  std::cout << std::endl;
  std::cout << "Accelerator RF: " << m_acceleratorRF / Unit::MHz << " MHz" << std::endl;
  std::cout << "Clock prescale factors:" << std::endl;
  std::cout << "===================================" << std::endl;

  for (const auto& det : m_prescaleMap) {
    std::cout << Const::parseDetectors(det.first) << ":" << std::endl;
    for (const auto& clock : det.second)  std::cout << " " << clock.first << " " << clock.second << std::endl;
  }
  std::cout << "===================================" << std::endl;

  std::cout << "Clock frequencies:" << std::endl;
  std::cout << "===================================" << std::endl;
  for (const auto& det : m_clocksMap) {
    std::cout << Const::parseDetectors(det.first) << ":" << std::endl;
    for (const auto& clock : det.second) {
      std::cout << " " << clock.first << " " << clock.second / Unit::MHz << " MHz" << std::endl;
    }
  }
  std::cout << "===================================" << std::endl;
}

