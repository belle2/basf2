/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/eklmhit/EKLMHitBase.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(EKLMHitBase);

EKLMHitBase::EKLMHitBase()
{
  m_nEndcap = 0;
  m_nLayer = 0;
  m_nSector = 0;
  m_nPlane = 0;
  m_nStrip = 0;
}

EKLMHitBase::EKLMHitBase(char nEndcap, char nLayer, char nSector, char nPlane,
                         char nStrip)
{
  m_nEndcap = nEndcap;
  m_nLayer = nLayer;
  m_nSector = nSector;
  m_nPlane = nPlane;
  m_nStrip = nStrip;
}

int EKLMHitBase::get_nEndcap()
{
  return m_nEndcap;
}

void EKLMHitBase::set_nEndcap(char nEndcap)
{
  m_nEndcap = nEndcap;
}

int EKLMHitBase::get_nLayer()
{
  return m_nLayer;
}

void EKLMHitBase::set_nLayer(char nLayer)
{
  m_nLayer = nLayer;
}

int EKLMHitBase::get_nSector()
{
  return m_nSector;
}

void EKLMHitBase::set_nSector(char nSector)
{
  m_nSector = nSector;
}

int EKLMHitBase::get_nPlane()
{
  return m_nPlane;
}

void EKLMHitBase::set_nPlane(char nPlane)
{
  m_nPlane = nPlane;
}

int EKLMHitBase::get_nStrip()
{
  return m_nStrip;
}

void EKLMHitBase::set_nStrip(char nStrip)
{
  m_nStrip = nStrip;
}

void EKLMHitBase::Print()
{
  B2INFO("Not implemented yet");
}

