/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
