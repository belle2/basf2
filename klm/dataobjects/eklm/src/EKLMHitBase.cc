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

EKLMHitBase::EKLMHitBase() :
  m_Section(-1),
  m_Layer(-1),
  m_Sector(-1)
{
}

EKLMHitBase::~EKLMHitBase()
{
}

EKLMHitBase::EKLMHitBase(int section, int layer, int sector) :
  m_Section(section),
  m_Layer(layer),
  m_Sector(sector)
{
}
