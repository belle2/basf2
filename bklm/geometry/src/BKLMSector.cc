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

#include "bklm/geometry/BKLMSector.h"

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

BKLMSector::BKLMSector()
{
}

BKLMSector::BKLMSector(int                frontBack,
                       int                sector,
                       int                nLayer,
                       CLHEP::Hep3Vector  shift,
                       CLHEP::Hep3Vector  translation,
                       CLHEP::HepRotation rotation) :
    m_FrontBack(frontBack),
    m_Sector(sector),
    m_NLayer(nLayer),
    m_Shift(shift),
    m_Translation(translation),
    m_Rotation(rotation)
{
  m_RotationInverse = m_Rotation.inverse();
  m_RotationMatrix = m_Rotation;
  m_RotationInverseMatrix = m_RotationInverse;
  m_Normal = m_Rotation(CLHEP::Hep3Vector(1.0, 0.0, 0.0));
}

BKLMSector::BKLMSector(const BKLMSector& s) :
    m_FrontBack(s.m_FrontBack),
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

BKLMSector::~BKLMSector()
{
}

/*
BKLMSector::BKLMSector& operator=( const BKLMSector& s ) {
  if ( this != &s ) {
    m_FrontBack = s.m_FrontBack;
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


bool BKLMSector::operator<(const BKLMSector& s) const
{
  if (m_FrontBack != s.m_FrontBack) return (m_FrontBack == 0);
  if (m_Sector    != s.m_Sector)    return (m_Sector < s.m_Sector);
  return false;
}

void BKLMSector::addModule(BKLMModule* m)
{
  m_Modules.push_back(m);
}

const bool BKLMSector::isSameSector(int frontBack, int sector) const
{
  return (frontBack == m_FrontBack) && (sector == m_Sector);
}

const bool BKLMSector::isSameSector(const BKLMSector& s) const
{
  return (s.m_FrontBack == m_FrontBack) && (s.m_Sector == m_Sector);
}

const bool BKLMSector::isSameSector(const BKLMModule& m) const
{
  return (m.getFrontBack() == m_FrontBack) && (m.getSector() == m_Sector);
}

const BKLMModule* BKLMSector::findModule(int module) const
{
  std::vector<BKLMModule*>::const_iterator iM;
  for (iM = m_Modules.begin(); iM != m_Modules.end(); ++iM) {
    if ((*iM)->isSameModule(m_FrontBack, m_Sector, module)) {
      break;
    }
  }
  return (*iM);
}

const CLHEP::Hep3Vector BKLMSector::localToGlobal(const CLHEP::Hep3Vector& v) const
{
  return rotateToGlobal(v) + m_Translation + m_Shift;
}

const CLHEP::Hep3Vector BKLMSector::globalToLocal(const CLHEP::Hep3Vector& v) const
{
  return rotateToLocal(v - m_Translation - m_Shift);
}

const CLHEP::Hep3Vector BKLMSector::globalToLocal(double x, double y, double z) const
{
  return rotateToLocal(CLHEP::Hep3Vector(x, y, z) - m_Translation - m_Shift);
}

const CLHEP::HepMatrix BKLMSector::localToGlobal(const CLHEP::HepMatrix& m) const
{
  return (m_RotationInverseMatrix * m * m_RotationMatrix);
}

const CLHEP::HepMatrix BKLMSector::globalToLocal(const CLHEP::HepMatrix& m) const
{
  return (m_RotationMatrix * m * m_RotationInverseMatrix);
}

const BKLMRect BKLMSector::localToGlobal(const BKLMRect& r) const
{
  BKLMRect global;
  global.corner[0] = localToGlobal(r.corner[0]);
  global.corner[1] = localToGlobal(r.corner[1]);
  global.corner[2] = localToGlobal(r.corner[2]);
  global.corner[3] = localToGlobal(r.corner[3]);
  return global;
}

const BKLMRect BKLMSector::globalToLocal(const BKLMRect& r) const
{
  BKLMRect local;
  local.corner[0] = globalToLocal(r.corner[0]);
  local.corner[1] = globalToLocal(r.corner[1]);
  local.corner[2] = globalToLocal(r.corner[2]);
  local.corner[3] = globalToLocal(r.corner[3]);
  return local;
}

const CLHEP::Hep3Vector BKLMSector::rotateToGlobal(const CLHEP::Hep3Vector& v) const
{
  return m_Rotation * v;
}

const CLHEP::Hep3Vector BKLMSector::rotateToLocal(const CLHEP::Hep3Vector& v) const
{
  return m_RotationInverse * v;
}

const CLHEP::Hep3Vector BKLMSector::getNormal() const
{
  return m_Rotation * CLHEP::Hep3Vector(1.0, 0.0, 0.0);
}

void BKLMSector::printTree() const
{
  B2INFO("BKLMSector: BKLM-"   << (m_FrontBack == 0 ? 'F' : 'B')
         << "-S"   << m_Sector
         << "    " << m_Shift);
  std::vector<BKLMModule*>::const_iterator iM;
  for (iM = m_Modules.begin(); iM != m_Modules.end(); ++iM) {
    (*iM)->printTree();
  }
}

/*
std::ostream& operator<<( std::ostream& out, const BKLMSector& s )
{
  out << "BKLM-" << (s.m_FrontBack == 0 ? 'F' : 'B')
      << "-S"    << s.m_Sector
      << "    "  << s.m_Shift;

  return( out );
}
*/
