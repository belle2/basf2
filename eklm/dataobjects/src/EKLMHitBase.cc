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

const int EKLMHitBase::getEndcap()
{
  return m_Endcap;
}

void EKLMHitBase::setEndcap(const int Endcap)
{
  m_Endcap = Endcap;
}

const int EKLMHitBase::getLayer()
{
  return m_Layer;
}

void EKLMHitBase::setLayer(const int Layer)
{
  m_Layer = Layer;
}

const int EKLMHitBase::getSector()
{
  return m_Sector;
}

void EKLMHitBase::setSector(const int Sector)
{
  m_Sector = Sector;
}

const int EKLMHitBase::getPlane()
{
  return m_Plane;
}

void EKLMHitBase::setPlane(const int Plane)
{
  m_Plane = Plane;
}

const int EKLMHitBase::getStrip()
{
  return m_Strip;
}

void EKLMHitBase::setStrip(const int Strip)
{
  m_Strip = Strip;
}


