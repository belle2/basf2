/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/dataobjects/ProfileInfo.h>

#include <framework/utilities/Utils.h>
#include <framework/gearbox/Unit.h>

using namespace Belle2;

ClassImp(ProfileInfo)


void ProfileInfo::set(double timeOffset)
{
  m_memory = Utils::getMemoryKB();

  m_timeInSec = Utils::getClock() / Unit::s - timeOffset;
}
