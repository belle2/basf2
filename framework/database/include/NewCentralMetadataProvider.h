/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/database/CentralMetadataProvider.h>

namespace Belle2::Conditions {
  /** Class to obtain payload metadata from the new central server via REST requests.
   *
   * This is essentially a specialized version of CentralMetadataProvider that
   * connects to the Belle II new central server instead of
   * the main Belle II conditions database server. It inherits all functionality
   * from CentralMetadataProvider and simply uses a different base URL.
   */
  class NewCentralMetadataProvider: public CentralMetadataProvider {
  public:
    /** Create using the default new central base URL and given usable tag states */
    explicit NewCentralMetadataProvider(const std::set<std::string>& usableTagStates);

    /** Create using a custom base URL for the new central server */
    NewCentralMetadataProvider(std::string baseUrl, const std::set<std::string>& usableTagStates);

    /** default destructor */
    virtual ~NewCentralMetadataProvider() = default;

  private:
    /** Default new central server base URL */
    static constexpr const char* s_defaultNewCentralUrl = "http://belle2-hsf.sdcc.bnl.gov/b2s/rest/";
  };
} // Belle2::Conditions namespace