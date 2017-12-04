/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 *                                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <mdst/dataobjects/KlId.h>

using namespace Belle2;

KlId::KlId() : m_KlId(0), m_isKLM(false), m_isECL(false)
{
}

KlId::KlId(float klid, bool isKLM, bool isECL) :
  m_KlId(klid), m_isKLM(isKLM), m_isECL(isECL)
{
}
