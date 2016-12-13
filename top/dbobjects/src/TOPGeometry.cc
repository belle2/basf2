/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPGeometry.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <math.h>
#include <iostream>

using namespace std;
using namespace Belle2;

const TOPGeoModule& TOPGeometry::getModule(unsigned moduleID) const
{
  moduleID--;
  if (moduleID >= m_modules.size())
    B2FATAL("TOPGeometry::getModule: invalid module ID " << moduleID + 1);
  return m_modules[moduleID];
}

double TOPGeometry::getInnerRadius() const
{
  double R = 200 / s_unit;
  for (auto& module : m_modules) {
    double tmp = module.getRadius() + module.getBarThickness() / 2
                 - module.getPrism().getExitThickness();
    if (tmp < R) R = tmp;
  }
  return R;
}

double TOPGeometry::getOuterRadius() const
{
  double R = 0;
  for (auto& module : m_modules) {
    double x = module.getBarWidth() / 2;
    double y = module.getRadius() + module.getBarThickness() / 2;
    double tmp = sqrt(x * x + y * y);
    if (tmp > R) R = tmp;
  }
  return R;
}

double TOPGeometry::getRadius() const
{
  if (m_modules.empty()) return 0;

  double R = 0;
  for (auto& module : m_modules) {
    R += module.getRadius();
  }
  return R / m_modules.size();
}

double TOPGeometry::getBackwardZ() const
{
  double z = 0;
  for (auto& module : m_modules) {
    double tmp = module.getBackwardZ() - module.getPrism().getFullLength();
    if (tmp < z) z = tmp;
  }
  return z;
}

double TOPGeometry::getForwardZ() const
{
  double z = 0;
  for (auto& module : m_modules) {
    double tmp = module.getForwardZ();
    if (tmp > z) z = tmp;
  }
  return z;
}

bool TOPGeometry::isConsistent() const
{
  if (m_modules.empty()) return false;
  for (const auto& module : m_modules) {
    if (!module.isConsistent()) return false;
  }
  if (!m_pmtArray.isConsistent()) return false;
  if (!m_frontEnd.isConsistent()) return false;
  if (!m_QBB.isConsistent()) return false;
  if (m_numBoardStacks == 0) return false;
  if (!m_nominalQE.isConsistent()) return false;
  if (!m_nominalTTS.isConsistent()) return false;
  if (!m_nominalTDC.isConsistent()) return false;
  return true;
}

void TOPGeometry::print(const std::string& title) const
{
  TOPGeoBase::print(title);


}
