/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Leo Piilonen                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <iostream>

#include "bklm/geometry/Sector.h"

#include <framework/logging/Logger.h>

using namespace std;
using namespace CLHEP;

namespace Belle2 {

  namespace bklm {

    Sector::Sector()
    {
    }

    Sector::Sector(bool        isForward,
                   int         sector,
                   int         nLayer,
                   Hep3Vector  shift,
                   Hep3Vector  translation,
                   HepRotation rotation) :
      m_IsForward(isForward),
      m_Sector(sector),
      m_NLayer(nLayer),
      m_Shift(shift),
      m_Translation(translation),
      m_Rotation(rotation)
    {
      m_RotationInverse = m_Rotation.inverse();
      m_RotationMatrix = m_Rotation;
      m_RotationInverseMatrix = m_RotationInverse;
      m_Normal = m_Rotation(Hep3Vector(1.0, 0.0, 0.0));
    }

    Sector::Sector(const Sector& s) :
      m_IsForward(s.m_IsForward),
      m_Sector(s.m_Sector),
      m_NLayer(s.m_NLayer),
      m_Shift(s.m_Shift),
      m_Translation(s.m_Translation),
      m_Rotation(s.m_Rotation),
      m_RotationInverse(s.m_RotationInverse),
      m_Normal(s.m_Normal),
      m_Modules(s.m_Modules)
    {
    }

    Sector::~Sector()
    {
    }

    /*
    Sector::Sector& operator=( const Sector& s ) {
      if ( this != &s ) {
        m_IsForward = s.m_IsForward;
        m_Sector = s.m_Sector;
        m_NLayer = s.m_NLayer;
        m_Shift = s.m_Shift;
        m_Translation = s.m_Translation;
        m_Rotation = s.m_Rotation;
        m_RotationInverse = s.m_RotationInverse;
        m_Normal = s.m_Normal;
        m_Modules = s.m_Modules;
      }
      return *this;
    }
    */

    /*
    bool Sector::operator<(const Sector& s) const
    {
      if (m_IsForward != s.m_IsForward) return (m_IsForward);
      if (m_Sector    != s.m_Sector)    return (m_Sector < s.m_Sector);
      return false;
    }
    */

    void Sector::addModule(Module* m)
    {
      m_Modules.push_back(m);
    }

    bool Sector::isSameSector(bool isForward, int sector) const
    {
      return (isForward == m_IsForward) && (sector == m_Sector);
    }

    bool Sector::isSameSector(const Sector& s) const
    {
      return (s.m_IsForward == m_IsForward) && (s.m_Sector == m_Sector);
    }

    bool Sector::isSameSector(const Module& m) const
    {
      return (m.isForward() == m_IsForward) && (m.getSector() == m_Sector);
    }

    const Module* Sector::findModule(int layer) const
    {
      vector<Module*>::const_iterator iM;
      for (iM = m_Modules.begin(); iM != m_Modules.end(); ++iM) {
        if ((*iM)->isSameModule(m_IsForward, m_Sector, layer)) {
          break;
        }
      }
      return (*iM);
    }

    const Hep3Vector Sector::localToGlobal(const Hep3Vector& v) const
    {
      return rotateToGlobal(v) + m_Translation + m_Shift;
    }

    const Hep3Vector Sector::globalToLocal(const Hep3Vector& v) const
    {
      return rotateToLocal(v - m_Translation - m_Shift);
    }

    const Hep3Vector Sector::globalToLocal(double x, double y, double z) const
    {
      return rotateToLocal(Hep3Vector(x, y, z) - m_Translation - m_Shift);
    }

    const HepMatrix Sector::localToGlobal(const HepMatrix& m) const
    {
      return (m_RotationInverseMatrix * m * m_RotationMatrix);
    }

    const HepMatrix Sector::globalToLocal(const HepMatrix& m) const
    {
      return (m_RotationMatrix * m * m_RotationInverseMatrix);
    }

    const Rect Sector::localToGlobal(const Rect& r) const
    {
      Rect global;
      global.corner[0] = localToGlobal(r.corner[0]);
      global.corner[1] = localToGlobal(r.corner[1]);
      global.corner[2] = localToGlobal(r.corner[2]);
      global.corner[3] = localToGlobal(r.corner[3]);
      return global;
    }

    const Rect Sector::globalToLocal(const Rect& r) const
    {
      Rect local;
      local.corner[0] = globalToLocal(r.corner[0]);
      local.corner[1] = globalToLocal(r.corner[1]);
      local.corner[2] = globalToLocal(r.corner[2]);
      local.corner[3] = globalToLocal(r.corner[3]);
      return local;
    }

    const Hep3Vector Sector::rotateToGlobal(const Hep3Vector& v) const
    {
      return m_Rotation * v;
    }

    const Hep3Vector Sector::rotateToLocal(const Hep3Vector& v) const
    {
      return m_RotationInverse * v;
    }

    const Hep3Vector Sector::getNormal() const
    {
      return m_Rotation * Hep3Vector(1.0, 0.0, 0.0);
    }

    void Sector::printTree() const
    {
      B2INFO("Sector: BKLM-"   << (m_IsForward == 0 ? 'F' : 'B')
             << "-S"   << m_Sector
             << "    " << m_Shift);
      vector<Module*>::const_iterator iM;
      for (iM = m_Modules.begin(); iM != m_Modules.end(); ++iM) {
        (*iM)->printTree();
      }
    }

  } // end of namespace bklm

} // end of namespace Belle2
