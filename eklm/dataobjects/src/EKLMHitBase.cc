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

using namespace Belle2;

EKLMHitBase::EKLMHitBase()
{
  m_PDG = -1;
  m_Time = -1;
  m_EDep = -1;
  m_Section = -1;
  m_Layer = -1;
  m_Sector = -1;
}

EKLMHitBase::~EKLMHitBase()
{
}

EKLMHitBase::EKLMHitBase(int Section, int Layer, int Sector)
{
  m_PDG = -1;
  m_Time = -1;
  m_EDep = -1;
  m_Section = Section;
  m_Layer = Layer;
  m_Sector = Sector;
}

int EKLMHitBase::getSection() const
{
  return m_Section;
}

void EKLMHitBase::setSection(int Section)
{
  m_Section = Section;
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

