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

namespace Belle2 {

  void TOPGeometry::appendModule(const TOPGeoModule& module)
  {
    if (isModuleIDValid(module.getModuleID())) {
      B2ERROR("TOPGeometry::appendModule: a module with ID = " << module.getModuleID()
              << "already appended");
      return;
    }
    m_modules.push_back(module);
  }


  bool TOPGeometry::isModuleIDValid(int moduleID) const
  {
    for (const auto& module : m_modules) {
      if (module.getModuleID() == moduleID) return true;
    }
    return false;
  }


  const TOPGeoModule& TOPGeometry::getModule(int moduleID) const
  {
    for (const auto& module : m_modules) {
      if (module.getModuleID() == moduleID) return module;
    }
    B2FATAL("TOPGeometry::getModule: invalid module ID " << moduleID);
  }


  double TOPGeometry::getInnerRadius() const
  {
    if (m_modules.empty()) return 0;

    double R = m_modules[0].getRadius();
    for (auto& module : m_modules) {
      double tmp = module.getRadius() + module.getBarThickness() / 2
                   - module.getPrism().getExitThickness();
      if (tmp < R) R = tmp;
    }
    return R;
  }

  double TOPGeometry::getOuterRadius() const
  {
    if (m_modules.empty()) return 0;

    double R = m_modules[0].getRadius();
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
    if (m_modules.empty()) return 0;

    double z = m_modules[0].getBackwardZ();
    for (auto& module : m_modules) {
      double tmp = module.getBackwardZ() - module.getPrism().getFullLength();
      if (tmp < z) z = tmp;
    }
    return z;
  }

  double TOPGeometry::getForwardZ() const
  {
    if (m_modules.empty()) return 0;

    double z = m_modules[0].getForwardZ();
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
    if (!m_frontEnd.isConsistent()) return false;
    if (!m_QBB.isConsistent()) return false;
    if (m_numBoardStacks == 0) return false;
    if (!m_nominalQE.isConsistent()) return false;
    if (!m_nominalTTS.isConsistent()) return false;
    if (!m_nominalTDC.isConsistent()) return false;
    return true;
  }

  void TOPGeometry::print(const std::string&) const
  {
    cout << endl;
    cout << "Geometry parameters of TOP counter:" << endl;
    cout << "===================================" << endl;
    cout << " name: " << m_name << endl;
    cout << " number of modules: " << m_modules.size() << endl << endl;

    for (const auto& module : m_modules) {
      module.print();
      cout << endl;
    }
    m_frontEnd.print();
    cout << " Number of board stacks: " << m_numBoardStacks << endl;
    cout << endl;
    m_QBB.print();
    cout << endl;
    m_nominalQE.print();
    cout << endl;
    m_nominalTTS.print();
    cout << endl;
    m_nominalTDC.print();
    cout << endl;

  }

} // end Belle2 namespace
