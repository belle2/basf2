/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/logging/LogConfig.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Dump the current call stack to the logging with the given level
    void printBacktrace(LogConfig::ELogLevel logLevel, int nFrames = 100);
  }
}
