/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPGeoModule.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <math.h>
#include <iostream>

using namespace std;

namespace Belle2 {

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
        B2ERROR("TOPGeoModule::setBrokenGlue: invalid glue ID " << glueID);
    }
  }


  void TOPGeoModule::setPeelOffRegions(double thickness, const std::string& material)
  {
    double size = 2 * m_pmtArray.getDx();
    double offset = (m_pmtArray.getX(1) + m_pmtArray.getX(2)) / 2 +
                    m_arrayDisplacement.getX();
    m_prism.setPeelOffRegions(size, offset, thickness, material);
  }


  TVector3 TOPGeoModule::pointToGlobal(const TVector3& point) const
  {
    if (!m_rotation) setTransformation();
    return (*m_rotation) * point + (*m_translation);
  }

  TVector3 TOPGeoModule::momentumToGlobal(const TVector3& momentum) const
  {
    if (!m_rotation) setTransformation();
    return (*m_rotation) * momentum;
  }

  TVector3 TOPGeoModule::pointToLocal(const TVector3& point) const
  {
    if (!m_rotation) setTransformation();
    return (*m_rotationInverse) * (point - (*m_translation));
  }

  TVector3 TOPGeoModule::momentumToLocal(const TVector3& momentum) const
  {
    if (!m_rotation) setTransformation();
    return (*m_rotationInverse) * momentum;
  }

  void TOPGeoModule::setTransformation() const
  {

    TRotation Rphi;
    Rphi.RotateZ(m_phi - M_PI / 2);
    TVector3 translation(0, m_radius, getZc() * s_unit);

    TRotation Rot = Rphi * m_moduleDisplacement.getRotation();
    translation += m_moduleDisplacement.getTranslation();
    m_rotation =  new TRotation(Rot);
    m_rotationInverse = new TRotation(Rot.Inverse());
    m_translation = new TVector3(Rphi * translation);
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
