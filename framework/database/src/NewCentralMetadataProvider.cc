/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/database/NewCentralMetadataProvider.h>
#include <framework/logging/Logger.h>

namespace Belle2::Conditions {

  NewCentralMetadataProvider::NewCentralMetadataProvider(const std::set<std::string>& usableTagStates):
    CentralMetadataProvider(s_defaultNewCentralUrl, usableTagStates)
  {
    // Constructor delegates to CentralMetadataProvider with default new central URL
    B2DEBUG(31, "Conditions Database: Created new central metadata provider with default URL: " << s_defaultNewCentralUrl);
  }

  NewCentralMetadataProvider::NewCentralMetadataProvider(std::string baseUrl, const std::set<std::string>& usableTagStates):
    CentralMetadataProvider(std::move(baseUrl), usableTagStates)
  {
    // Constructor delegates to CentralMetadataProvider with custom URL
    B2DEBUG(31, "Conditions Database: Created new central metadata provider with custom URL");
  }

} // Belle2::Conditions namespace