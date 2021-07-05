/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/StreamSizeCounter.h"

using namespace Belle2;

size_t StreamSizeCounter::write(const void*, size_t count)
{
  m_count += count;
  return count;
}
