/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMHitBase.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(EKLMHitBase);

EKLMHitBase::EKLMHitBase()
{
  m_Endcap = 0;
  m_Layer = 0;
  m_Sector = 0;
  m_PDG = 0;
  m_Time = 0;
  m_EDep = 0;
  m_globalX = 0;
  m_globalY = 0;
  m_globalZ = 0;
  m_localX = 0;
  m_localY = 0;
  m_localZ = 0;
}

EKLMHitBase::EKLMHitBase(int Endcap, int Layer, int Sector)
{
  m_Endcap = Endcap;
  m_Layer = Layer;
  m_Sector = Sector;
  m_PDG = 0;
  m_Time = 0;
  m_EDep = 0;
  m_globalX = 0;
  m_globalY = 0;
  m_globalZ = 0;
  m_localX = 0;
  m_localY = 0;
  m_localZ = 0;
}

int EKLMHitBase::getEndcap() const
{
  return m_Endcap;
}

void EKLMHitBase::setEndcap(int Endcap)
{
  m_Endcap = Endcap;
}

int EKLMHitBase::getLayer() const
{
  return m_Layer;
}

void EKLMHitBase::setLayer(int Layer)
{
  m_Layer = Layer;
}

int EKLMHitBase::getSector() const
{
  return m_Sector;
}

void EKLMHitBase::setSector(int Sector)
{
  m_Sector = Sector;
}

int EKLMHitBase::getPDG() const
{
  return m_PDG;
}

void EKLMHitBase::setPDG(int PDG)
{
  m_PDG = PDG;
}

double EKLMHitBase::getTime() const
{
  return m_Time;
}

void EKLMHitBase::setTime(double Time)
{
  m_Time = Time;
}

double EKLMHitBase::getEDep() const
{
  return m_EDep;
}

void EKLMHitBase::setEDep(double EDep)
{
  m_EDep = EDep;
}

void EKLMHitBase::increaseEDep(double deltaEDep)
{
  m_EDep += deltaEDep;
}

HepGeom::Point3D<double> EKLMHitBase::getGlobalPosition()
{
  return HepGeom::Point3D<double>(m_globalX, m_globalY, m_globalZ);
}

void EKLMHitBase::setGlobalPosition(HepGeom::Point3D<double> gpos)
{
  m_globalX = gpos.x();
  m_globalY = gpos.y();
  m_globalZ = gpos.z();
}

HepGeom::Point3D<double> EKLMHitBase::getLocalPosition()
{
  return HepGeom::Point3D<double>(m_localX, m_localY, m_localZ);
}

void EKLMHitBase::setLocalPosition(HepGeom::Point3D<double> lpos)
{
  m_localX = lpos.x();
  m_localY = lpos.y();
  m_localZ = lpos.z();
}

const TVector3* EKLMHitBase::getPosition() const
{
  return new TVector3(m_localX, m_localY, m_localZ);
}

const TVector3 EKLMHitBase::getGlobalPosition() const
{
  return TVector3(m_localX, m_localY, m_localZ);
}

