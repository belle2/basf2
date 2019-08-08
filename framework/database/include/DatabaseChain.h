/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/logging/LogConfig.h>

namespace Belle2 {
  /**
   * Implementation of a database backend that uses a chain of other
   * database backends to obtain the payloads.
   */
  class [[deprecated("Please use the Conditions::Configuration object")]] DatabaseChain {
  public:
    /**
     * Method to set the database instance to a local database.
     *
     * @param resetIoVs A flag to indicate whether IoVs from non-primary databases should be set to the current run
     * @param logLevel The level of log messages about not-found payloads.
     * @param invertLogging If true log messages will be created when a
     *   payload is found. This is intended for the local database to notify
     *   the user that a non-standard payload from a local directory is used.
     */
    static void createInstance(bool resetIoVs = false, LogConfig::ELogLevel logLevel = LogConfig::c_Warning,
                               bool invertLogging = false);
  };
} // namespace Belle2
