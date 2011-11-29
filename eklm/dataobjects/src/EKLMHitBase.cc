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
  m_Plane = 0;
  m_Strip = 0;
}

EKLMHitBase::EKLMHitBase(const int Endcap, const int Layer, const int Sector, const int Plane,
                         const int Strip)
{
  m_Endcap = Endcap;
  m_Layer = Layer;
  m_Sector = Sector;
  m_Plane = Plane;
  m_Strip = Strip;
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

int EKLMHitBase::getPlane() const
{
  return m_Plane;
}

void EKLMHitBase::setPlane(int Plane)
{
  m_Plane = Plane;
}

int EKLMHitBase::getStrip() const
{
  return m_Strip;
}

void EKLMHitBase::setStrip(int Strip)
{
  m_Strip = Strip;
}


