/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/database/MetadataProvider.h>
#include <framework/database/Downloader.h>
#include <nlohmann/json.hpp>

namespace Belle2::Conditions {
  /** Class to obtain payload metadata from the central database server via REST requests */
  class CentralMetadataProvider: public MetadataProvider {
  public:
    /** Create using a base rest url to find the server endpoints */
    CentralMetadataProvider(std::string baseUrl, const std::set<std::string>& usableTagStates);
    /** default destructor */
    virtual ~CentralMetadataProvider() = default;
    /** Check the status of a given globaltag \
     * @returns the status of the globaltag (like "OPEN", "INVALID", "PUBLISHED").
     *          Returns empty string on any errors or if the tag doesn't exist.
     */
    std::string getGlobaltagStatus(const std::string& name) override;
    /** Update the list of known payloads for the given globaltag/exp/run. This
     * basically just calls addPayload() for each payload it finds in the
     * globaltag for the given exp/run.
     * @returns true on sucess, false on any error
     */
    bool updatePayloads(const std::string& globaltag, int exp, int run) override;
  private:
    /** Downlad a given relative url (the baseUrl will be prependend) and return the json description.
     * Will raise exceptions on error */
    nlohmann::json get(const std::string& url);
    /** Reference to the downloader instance for convenience */
    Downloader& m_downloader{Downloader::getDefaultInstance()};
    /** base url of the server */
    std::string m_baseUrl;
  };
} // Belle2::Conditions namespace
