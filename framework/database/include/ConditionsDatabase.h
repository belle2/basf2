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
#include <framework/database/Database.h>
#include <string>

namespace Belle2 {
  /**
   * Database backend that uses the conditions service.
   */
  class [[deprecated("Please use the Conditions::Configuration object")]] ConditionsDatabase {
  public:

    /**
     * Method to set the database instance to the central database with default parameters.
     *
     * @param globalTag   The name of the global tag
     * @param logLevel    The level of log messages about not-found payloads.
     * @param payloadDir  Directory for local copies of the payloads
     */
    static void createDefaultInstance(const std::string& globalTag, LogConfig::ELogLevel logLevel = LogConfig::c_Warning,
                                      const std::string& payloadDir = "centraldb");

    /**
     * Method to set the database instance to the central database.
     *
     * @param globalTag      The name of the global tag
     * @param restBaseName   Base name for REST services
     * @param fileBaseName   Base name for conditions files
     * @param fileBaseLocal  Directory name for local conditions files copies
     * @param logLevel       The level of log messages about not-found payloads.
     * @param invertLogging  If true log messages will be created when a
     *                       payload is found. This is intended for override
     *                       setups where a few payloads are taken from non
     *                       standard locations
     */
    static void createInstance(const std::string& globalTag, const std::string& restBaseName, const std::string& fileBaseName,
                               const std::string& fileBaseLocal, LogConfig::ELogLevel logLevel = LogConfig::c_Warning,
                               bool invertLogging = false);

    /** Facade the reset as some code calls the reset function on this class which used to inherit from Database */
    static void reset() { Database::reset(); }
  };
} // namespace Belle2
