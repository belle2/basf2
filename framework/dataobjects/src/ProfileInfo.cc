/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/dataobjects/ProfileInfo.h>

#include <framework/utilities/Utils.h>
#include <framework/gearbox/Unit.h>

using namespace Belle2;

void ProfileInfo::set(double timeOffset)
{
  m_virtualMemory = Utils::getVirtualMemoryKB();
  m_rssMemory = Utils::getRssMemoryKB();

  m_timeInSec = Utils::getClock() / Unit::s - timeOffset;
}
