/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPGeoModule.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <math.h>
#include <iostream>
#include <Math/RotationZ.h>


using namespace std;
using namespace ROOT::Math;

namespace Belle2 {

  TOPGeoModule::TOPGeoModule(const TOPGeoModule& module): TOPGeoBase(module.getName())
  {
    *this = module;
    m_transform = 0;
    m_transformNominal = 0;
  }

  TOPGeoModule& TOPGeoModule::operator=(const TOPGeoModule& module)
  {
    if (this != &module) {
      TOPGeoBase::operator=(module);
      m_moduleID = module.getModuleID();
      m_radius = module.getRadius();
      m_phi = module.getPhi();
      m_backwardZ = module.getBackwardZ();
      m_moduleCNumber = module.getModuleCNumber();
      m_bar1 = module.getBarSegment1();
      m_bar2 = module.getBarSegment2();
      m_mirror = module.getMirrorSegment();
      m_prism = module.getPrism();
      m_pmtArray = module.getPMTArray();
      m_arrayDisplacement = module.getPMTArrayDisplacement();
      m_moduleDisplacement = module.getModuleDisplacement();

      if (m_transform) delete m_transform;
      if (m_transformNominal) delete m_transformNominal;
      m_transform = 0;
      m_transformNominal = 0;
    }
    return *this;
  }

  TOPGeoModule::~TOPGeoModule()
  {
    if (m_transform) delete m_transform;
    if (m_transformNominal) delete m_transformNominal;
  }

  void TOPGeoModule::setTransformation() const
  {
    RotationZ Rz(m_phi - M_PI / 2);
    m_transformNominal = new Transform3D(Rz, Rz * XYZVector(0, m_radius, getZc() * s_unit));
    m_transform = new Transform3D(*m_transformNominal * m_moduleDisplacement.getTransformation());
  }


  void TOPGeoModule::setBrokenGlue(int glueID, double fraction, double angle,
                                   const std::string& material)
  {
    switch (glueID) {
      case 1:
        m_mirror.setGlueDelamination(fraction, angle, material);
        break;
      case 2:
        m_bar1.setGlueDelamination(fraction, angle, material);
        break;
      case 3:
        m_bar2.setGlueDelamination(fraction, angle, material);
        break;
      default:
        B2ERROR("TOPGeoModule::setBrokenGlue: invalid glue ID."
                << LogVar("glue ID", glueID));
    }
  }


  void TOPGeoModule::setPeelOffRegions(double thickness, const std::string& material)
  {
    double size = 2 * m_pmtArray.getDx();
    double offset = (m_pmtArray.getX(1) + m_pmtArray.getX(2)) / 2 +
                    m_arrayDisplacement.getX();
    m_prism.setPeelOffRegions(size, offset, thickness, material);
  }


  XYZPoint TOPGeoModule::pointToGlobal(const XYZPoint& point) const
  {
    if (not m_transform) setTransformation();
    return *m_transform * point;
  }

  XYZVector TOPGeoModule::momentumToGlobal(const XYZVector& momentum) const
  {
    if (not m_transform) setTransformation();
    return *m_transform * momentum;
  }

  XYZPoint TOPGeoModule::pointToLocal(const XYZPoint& point) const
  {
    if (not m_transform) setTransformation();
    return m_transform->ApplyInverse(point);
  }

  XYZVector TOPGeoModule::momentumToLocal(const XYZVector& momentum) const
  {
    if (not m_transform) setTransformation();
    return m_transform->ApplyInverse(momentum);
  }

  XYZPoint TOPGeoModule::pointNominalToGlobal(const XYZPoint& point) const
  {
    if (not m_transformNominal) setTransformation();
    return *m_transformNominal * point;
  }

  XYZVector TOPGeoModule::momentumNominalToGlobal(const XYZVector& momentum) const
  {
    if (not m_transformNominal) setTransformation();
    return *m_transformNominal * momentum;
  }

  XYZPoint TOPGeoModule::pointGlobalToNominal(const XYZPoint& point) const
  {
    if (not m_transformNominal) setTransformation();
    return m_transformNominal->ApplyInverse(point);
  }

  XYZVector TOPGeoModule::momentumGlobalToNominal(const XYZVector& momentum) const
  {
    if (not m_transformNominal) setTransformation();
    return m_transformNominal->ApplyInverse(momentum);
  }


  bool TOPGeoModule::isConsistent() const
  {
    if (m_moduleID <= 0) return false;
    if (!m_bar1.isConsistent()) return false;
    if (!m_bar2.isConsistent()) return false;
    if (!m_mirror.isConsistent()) return false;
    if (!m_prism.isConsistent()) return false;
    if (!m_pmtArray.isConsistent()) return false;
    return true;
  }


  void TOPGeoModule::print(const std::string&) const
  {
    cout << "Slot " << getModuleID() << " geometry parameters:" << endl;
    cout << "---------------------------" << endl;
    cout << " name: " << m_name << endl;
    cout << " moduleID = " << getModuleID();
    cout << ", radius = " << getRadius() << " " << s_unitName;
    cout << ", phi = " << getPhi() / Unit::deg << " deg";
    cout << ", backward z = " << getBackwardZ() << " " << s_unitName;
    cout << ", construction number = " << getModuleCNumber() << endl;
    cout << endl;

    m_prism.print();
    cout << endl;
    m_bar2.print("Bar segment 2 (backward) geometry parameters");
    cout << endl;
    m_bar1.print("Bar segment 1 (forward) geometry parameters");
    cout << endl;
    m_mirror.print();
    cout << endl;
    m_pmtArray.print();
    cout << endl;
    m_arrayDisplacement.print();
    cout << endl;
    m_moduleDisplacement.print();
    cout << endl;

  }

} // end Belle2 namespace
