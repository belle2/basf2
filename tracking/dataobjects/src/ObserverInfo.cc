/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/dataobjects/ObserverInfo.h>

using namespace Belle2;

ObserverInfo::ObserverInfo() :
  m_filterInfos(),
  m_hits(),
  m_mainMCParticleID(-1),
  m_mainPurity(-1.)
{
};

ObserverInfo::~ObserverInfo()
{
};
