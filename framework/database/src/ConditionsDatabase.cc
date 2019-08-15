/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/ConditionsDatabase.h>
#include <framework/database/Configuration.h>
#include <framework/logging/Logger.h>

// we know all of this is deprecated, we don't want the warnings when compiling the service itself ...
#ifdef __INTEL_COMPILER
#pragma warning (disable:1478) //[[deprecated]]
#pragma warning (disable:1786) //[[deprecated("message")]]
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace Belle2 {

  void ConditionsDatabase::createDefaultInstance(const std::string& globalTag, LogConfig::ELogLevel,
                                                 const std::string& payloadDir)
  {
    if (!payloadDir.empty()) {
      Conditions::Configuration::getInstance().prependPayloadLocation(payloadDir);
    }
    Conditions::Configuration::getInstance().prependGlobalTag(globalTag);
  }

  void ConditionsDatabase::createInstance(const std::string& globalTag, const std::string& restBaseName,
                                          const std::string& fileBaseName,
                                          const std::string& fileBaseLocal, LogConfig::ELogLevel,
                                          bool)
  {
    if (!(restBaseName.empty() and fileBaseName.empty() and fileBaseLocal.empty())) {
      B2FATAL(R"fatal(Expert settings for central databases are no longer supported".
    Please use the conditions configuration object for these seetings

    >>> basf2.conditions.metadata_provider = [restBaseName]
    >>> basf2.conditions.prepend_globaltag(globalTag)
    >>> basf2.conditions.payload_locations = [fileBaseLocal, fileBaseName])fatal");
    }
    Conditions::Configuration::getInstance().prependGlobalTag(globalTag);
  }

}
