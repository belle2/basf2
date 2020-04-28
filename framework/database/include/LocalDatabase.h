/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/logging/LogConfig.h>
#include <framework/database/Database.h>

#include <string>

namespace Belle2 {
  /**
   * Implentation of a database backend that uses local root files for the
   * payload storage and a text file for the assignment of IoVs to payloads.
   */
  class [[deprecated("Please use the Conditions::Configuration object")]] LocalDatabase {
  public:

    /**
     * Method to set the database instance to a local database.
     *
     * @param fileName   The name of the database text file with the IoV assignments.
     * @param payloadDir The name of the directory in which the payloads are stored.
     *                   By default the same directory as the one containing the database text file is used.
     * @param readOnly   If this flag is set the database is opened in read only mode without locking.
     * @param logLevel   The level of log messages about not-found payloads.
     * @param invertLogging  If true log messages will be created when a payload is
     *                   found. This is intended for override setups where a
     *                   few payloads are taken from non standard locations
     */
    static void createInstance(const std::string& fileName = "database.txt", const std::string& payloadDir = "",
                               bool readOnly = false, LogConfig::ELogLevel logLevel = LogConfig::c_Warning,
                               bool invertLogging = false);

    /** Facade the reset as some code calls the reset function on this class which used to inherit from Database */
    static void reset() { Database::reset(); }
  };
} // namespace Belle2
