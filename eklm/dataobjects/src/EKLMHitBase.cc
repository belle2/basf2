/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

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
  m_GlobalPosition = TVector3(0., 0., 0.);
  m_LocalPosition = TVector3(0., 0., 0.);
}

EKLMHitBase::EKLMHitBase(int Endcap, int Layer, int Sector, int PDG, double Time, double EDep, TVector3  GlobalPosition, TVector3  LocalPosition)
{
  m_Endcap = Endcap;
  m_Layer = Layer;
  m_Sector = Sector;
  m_PDG = PDG;
  m_Time = Time;
  m_EDep = EDep;
  m_GlobalPosition = GlobalPosition;
  m_LocalPosition = LocalPosition;
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

const TVector3* EKLMHitBase::getPosition() const
{
  return &m_GlobalPosition;
}

void EKLMHitBase::setPosition(TVector3& position)
{
  m_GlobalPosition = position;
}

void EKLMHitBase::setPosition(const TVector3* position)
{
  m_GlobalPosition = *position;
}


const TVector3* EKLMHitBase::getLocalPosition() const
{
  return &m_LocalPosition;
}

void EKLMHitBase::setLocalPosition(TVector3& position)
{
  m_LocalPosition = position;
}

void EKLMHitBase::setLocalPosition(const TVector3* position)
{
  m_LocalPosition = *position;
}

