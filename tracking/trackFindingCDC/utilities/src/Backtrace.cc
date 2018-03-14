/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/utilities/Backtrace.h>

#include <framework/logging/Logger.h>
#include <framework/logging/LogConfig.h>

#include <vector>

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace Belle2;

void TrackFindingCDC::printBacktrace(LogConfig::ELogLevel logLevel, int nFrames)
{
  std::vector<void*> buffer(nFrames);
  void** rawBuffer = buffer.data();
  char** traceLines;

  int traceSize = backtrace(rawBuffer, nFrames);
  B2LOG(logLevel, 0, "backtrace() returned " << traceSize << " addresses");

  traceLines = backtrace_symbols(rawBuffer, traceSize);

  if (traceLines == NULL) {
    B2LOG(logLevel, 0, "no backtrace obtained");
  }

  for (int iLine = 0; iLine < traceSize; ++iLine) {
    B2LOG(logLevel, 0, traceLines[iLine]);
  }

  free(traceLines);
}
