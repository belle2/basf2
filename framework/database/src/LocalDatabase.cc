/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2018 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/LocalDatabase.h>
#include <framework/database/Configuration.h>
#include <framework/logging/Logger.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

// we know all of this is deprecated, we don't want the warnings when compiling the service itself ...
#ifdef __INTEL_COMPILER
#pragma warning (disable:1478) //[[deprecated]]
#pragma warning (disable:1786) //[[deprecated("message")]]
#else
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif


namespace Belle2 {
  void LocalDatabase::createInstance(const std::string& fileName, const std::string& payloadDir, bool readOnly,
                                     LogConfig::ELogLevel logLevel, bool invertLogging)
  {
    if (!readOnly) {
      B2WARNING(R"warn(Local databases are now aways read only.
    If you want to change the location of newly created payload files please use

    >>> basf2.conditions.expert_settings(save_payloads=filename))warn");
    }
    if (!payloadDir.empty()) {
      boost::filesystem::path file(fileName);
      std::string absFileDir = boost::filesystem::absolute(file.parent_path()).string();
      std::string payloads = boost::filesystem::absolute(payloadDir).string();
      boost::algorithm::trim_right_if(absFileDir, boost::is_any_of("/"));
      boost::algorithm::trim_right_if(payloads, boost::is_any_of("/"));
      if (absFileDir != payloads) {
        B2FATAL(R"warn(Local databases with payloads in different files than the text file is no longer supported:
    Payloads need to be in the same file as the text file)warn"
                << LogVar("filename", fileName) << LogVar("payloaddir", payloadDir));
      }
    }
    Conditions::Configuration::getInstance().prependTestingPayloadLocation(fileName);
  }

}
