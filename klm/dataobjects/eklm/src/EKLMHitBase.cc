/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/eklm/EKLMHitBase.h>

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
