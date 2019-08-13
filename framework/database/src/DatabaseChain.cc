/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/database/DatabaseChain.h>

// we know all of this is deprecated, we don't want the warnings when compiling the service itself ...
#ifdef __INTEL_COMPILER
#pragma warning (disable:1478) //[[deprecated]]
#pragma warning (disable:1786) //[[deprecated("message")]]
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace Belle2 {

  void DatabaseChain::createInstance(bool resetIoVs, LogConfig::ELogLevel logLevel, bool invertLogging)
  {
    B2WARNING("DatabaseChain is obsolete and doesn't do anything anymore");
  }
} // Belle2 namespace
