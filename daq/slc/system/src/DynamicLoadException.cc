/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/DynamicLoadException.h"

#include <cstdarg>
#include <cstdio>

using namespace Belle2;

DynamicLoadException::DynamicLoadException(const std::string& format, ...)
{
  va_list ap;
  char ss[1024 * 100];
  va_start(ap, format);
  vsnprintf(ss, sizeof(ss), format.c_str(), ap);
  va_end(ap);
  m_comment = ss;
}

