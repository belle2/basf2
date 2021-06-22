/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/MetadataProvider.h>
#include <framework/logging/Logger.h>

#include <boost/algorithm/string.hpp>

namespace Belle2::Conditions {
  bool MetadataProvider::setTags(const std::vector<std::string>& tags)
  {
    m_tags = tags;
    const size_t validTags = std::count_if(m_tags.begin(), m_tags.end(), [this](const auto & name) {
      // dispatch obtaining the state to the implementation
      auto status = getGlobaltagStatus(name);
      // empty status: Unspecified error already dealt with
      if (status.empty()) return false;
      // otherwise check for valid states
      if (m_usableTagStates.count(status) == 0) {
        B2ERROR("The globaltag has a status which is not permitted for use. This is for your own protection"
                << LogVar("globaltag", name)
                << LogVar("status", status)
                << LogVar("allowed states", boost::algorithm::join(m_usableTagStates, ", ")));
        return false;
      }
      return true;
    });
    return validTags == tags.size();
  }

  bool MetadataProvider::getPayloads(int exp, int run, std::vector<PayloadMetadata>& info)
  {
    if (info.empty()) return true;
    for (const auto& tag : m_tags) {
      // Check whether we already know the payloads for this exp,run ...
      bool present{false};
      std::tie(present, m_payloads) = m_cache[tag].get(exp, run);
      // Apparently not, try to get them
      if (!present && !updatePayloads(tag, exp, run)) {
        // Error obtaining payloads means something is wrong (server/network/file problem).
        // In this case we want to fall back to next metadata provider ...
        throw std::runtime_error("Problem updating metadata");
      }
      // And once we got them let's update the info for all requested payloads
      const auto& existing = *m_payloads;
      // and check if we found all payloads we're looking for.
      // Warning: **DO NOT** use `std::all_of` here which looks better but stops
      // iteration on the first missing payload instead of going through all the
      // remaining as well.
      const size_t found = std::count_if(info.begin(), info.end(), [&existing](auto & payload) {
        // already filled by previous gt or has a filename from testing payloads ... so don't do anything
        if (payload.revision > 0 or !payload.filename.empty()) return true;
        // otherwise look for the payload in the list of existing payloads for this run
        if (auto && it = existing.find(payload.name); it != existing.end()) {
          payload.update(it->second);
          B2DEBUG(35, "Found requested payload metadata"
                  << LogVar("globaltag", payload.globaltag)
                  << LogVar("name", payload.name)
                  << LogVar("revision", payload.revision)
                  << LogVar("checksum", payload.checksum));
          return true;
        }
        // Not found, fine, not a big problem yet, there can be multiple global tags
        return false;
      });
      // If we found all we don't need to check the next global tag at all ...
      if (found == info.size()) break;
    }
    // Check that all payloads could be found (unless they're optional)
    // Again, we don't use std::all_of here because then it stops early and we
    // don't get errors for all missing payloads. But at least it would still be correct.
    const int missing = std::count_if(info.begin(), info.end(), [this, exp, run](const auto & p) {
      if (p.revision == 0 and p.filename.empty() and p.required) {
        B2ERROR("Cannot find payload in any of the configured global tags"
                << LogVar("name", p.name)
                << LogVar("globaltags", boost::algorithm::join(m_tags, ", "))
                << LogVar("experiment", exp) << LogVar("run", run));
        return true;
      }
      return false;
    });
    // Only happy without missing payloads
    return missing == 0;
  }

  void MetadataProvider::addPayload(PayloadMetadata&& payload, const std::string& messagePrefix)
  {
    const auto [it, inserted] = m_payloads->emplace(payload.name, payload);
    if (!inserted) {
      auto& existing = it->second;
      if (existing.revision < payload.revision) {
        std::swap(existing, payload);
      }
      B2DEBUG(36, messagePrefix << (messagePrefix.empty() ? "" : ": ") << "Found duplicate payload. Discarding one of them"
              << LogVar("globaltag", existing.globaltag)
              << LogVar("name", existing.name)
              << LogVar("revision", existing.revision)
              << LogVar("checksum", existing.checksum)
              << LogVar("discarded revision", payload.revision));
    } else {
      B2DEBUG(37, messagePrefix << (messagePrefix.empty() ? "" : ": ") << "Found payload"
              << LogVar("globaltag", payload.globaltag)
              << LogVar("name", payload.name)
              << LogVar("revision", payload.revision)
              << LogVar("checksum", payload.checksum));
    }
  }

  std::string NullMetadataProvider::getGlobaltagStatus([[maybe_unused]] const std::string& name)
  {
    if (!m_errorShown) B2ERROR("No Metadata provider configured, globaltags cannot be used");
    m_errorShown = true;
    return "";
  }
} // Belle2::Conditions namespace
