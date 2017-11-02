/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/logging/LogConfig.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Dump the current call stack to the logging with the given level
    void printBacktrace(LogConfig::ELogLevel logLevel, int nFrames = 100);
  }
}
