/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/utilities/Backtrace.h>

#include <framework/logging/Logger.h>
#include <framework/logging/LogConfig.h>

#include <vector>

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

using namespace Belle2;

void TrackFindingCDC::printBacktrace(LogConfig::ELogLevel logLevel, int nFrames)
{
  std::vector<void*> buffer(nFrames);
  void** rawBuffer = buffer.data();
  char** traceLines;

  int traceSize = backtrace(rawBuffer, nFrames);
  B2LOG(logLevel, 0, "backtrace() returned " << traceSize << " addresses");

  traceLines = backtrace_symbols(rawBuffer, traceSize);

  if (traceLines == nullptr) {
    B2LOG(logLevel, 0, "no backtrace obtained");
  }

  for (int iLine = 0; iLine < traceSize; ++iLine) {
    B2LOG(logLevel, 0, traceLines[iLine]);
  }

  free(traceLines);
}
