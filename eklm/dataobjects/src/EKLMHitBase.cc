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
}

EKLMHitBase::~EKLMHitBase()
{
}

EKLMHitBase::EKLMHitBase(int Endcap, int Layer, int Sector)
{
  m_Endcap = Endcap;
  m_Layer = Layer;
  m_Sector = Sector;
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

float EKLMHitBase::getTime() const
{
  return m_Time;
}

void EKLMHitBase::setTime(float Time)
{
  m_Time = Time;
}

float EKLMHitBase::getEDep() const
{
  return m_EDep;
}

void EKLMHitBase::setEDep(float EDep)
{
  m_EDep = EDep;
}

void EKLMHitBase::increaseEDep(float deltaEDep)
{
  m_EDep += deltaEDep;
}


bool EKLMHitBase::operator==(const EKLMHitBase& right)  const // to be justified later
{
  if (m_Endcap != right.getEndcap())
    return false;
  if (m_Layer != right.getLayer())
    return false;
  if (m_Sector != right.getSector())
    return false;
  if (abs(m_Time - right.getTime()) > 1)
    return false;
  return true;
}

