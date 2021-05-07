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

  TOPGeoModule::TOPGeoModule(const TOPGeoModule& module): TOPGeoBase(module.getName())
  {
    *this = module;
    m_rotation = 0;
    m_rotationInverse = 0;
    m_translation = 0;
    m_rotationNominal = 0;
    m_rotationNominalInverse = 0;
    m_translationNominal = 0;
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
      if (m_rotation) delete m_rotation;
      if (m_rotationInverse) delete m_rotationInverse;
      if (m_translation) delete m_translation;
      if (m_rotationNominal) delete m_rotationNominal;
      if (m_rotationNominalInverse) delete m_rotationNominalInverse;
      if (m_translationNominal) delete m_translationNominal;
      m_rotation = 0;
      m_rotationInverse = 0;
      m_translation = 0;
      m_rotationNominal = 0;
      m_rotationNominalInverse = 0;
      m_translationNominal = 0;
    }
    return *this;
  }

  TOPGeoModule::~TOPGeoModule()
  {
    if (m_rotation) delete m_rotation;
    if (m_rotationInverse) delete m_rotationInverse;
    if (m_translation) delete m_translation;
    if (m_rotationNominal) delete m_rotationNominal;
    if (m_rotationNominalInverse) delete m_rotationNominalInverse;
    if (m_translationNominal) delete m_translationNominal;
  }

  void TOPGeoModule::setTransformation() const
  {
    TRotation Rphi;
    Rphi.RotateZ(m_phi - M_PI / 2);
    TVector3 translation(0, m_radius, getZc() * s_unit);
    m_rotationNominal =  new TRotation(Rphi);
    m_rotationNominalInverse = new TRotation(Rphi.Inverse());
    m_translationNominal = new TVector3(Rphi * translation);

    TRotation Rot = Rphi * m_moduleDisplacement.getRotation();
    translation += m_moduleDisplacement.getTranslation();
    m_rotation =  new TRotation(Rot);
    m_rotationInverse = new TRotation(Rot.Inverse());
    m_translation = new TVector3(Rphi * translation);
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

  TVector3 TOPGeoModule::pointNominalToGlobal(const TVector3& point) const
  {
    if (!m_rotation) setTransformation();
    return (*m_rotationNominal) * point + (*m_translationNominal);
  }

  TVector3 TOPGeoModule::momentumNominalToGlobal(const TVector3& momentum) const
  {
    if (!m_rotation) setTransformation();
    return (*m_rotationNominal) * momentum;
  }

  TVector3 TOPGeoModule::pointGlobalToNominal(const TVector3& point) const
  {
    if (!m_rotation) setTransformation();
    return (*m_rotationNominalInverse) * (point - (*m_translationNominal));
  }

  TVector3 TOPGeoModule::momentumGlobalToNominal(const TVector3& momentum) const
  {
    if (!m_rotation) setTransformation();
    return (*m_rotationNominalInverse) * momentum;
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
