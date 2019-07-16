/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/database/PayloadMetadata.h>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace Belle2::Conditions {
  /** Base class for a payload metadata provider.
   *
   * This class is supposed to know what payloads exist and if asked will update
   * a given list of PayloadMetadata instances with the actual values valid for
   * the given exp/run.
   */
  class MetadataProvider {
  public:
    /** Type for the internal list of payloads */
    using PayloadMap = std::unordered_map<std::string, PayloadMetadata>;

    /** Simple caching structure to keep the payload information for this and
     * the last exp/run.
     *
     * This is mostly done to not request the payload information each time a
     * new payload is created (during initialize) but also to make sure that
     * flapping of run number doesn't excessively hammer the database. So we
     * save the current and the last list of metadata
     */
    class PayloadMetadataCache {
    public:
      /** Each cache entry */
      struct CacheEntry {
        int exp{ -1}; /**< experiment number */
        int run{ -1}; /**< run number */
        PayloadMap map; /**< Map of all known name -> PayloadMetadata entries */
      };
      /** Get the list of map payloads from the cache
       * This returns a tuple containing two elements:
       * 1. A bool whether the map was found in cache or whether a new empty map
       *    was returned.
       * 2. A pointer to the map either containing the payloads or where the
       *    payloads should be stored, always valid and owned by the cache.
       */
      std::tuple<bool, PayloadMap*> get(int exp, int run)
      {
        using std::get;
        // if run mismatch swap current and last entry
        if (current.exp != exp || current.run != run) std::swap(current, previous);
        // still mismatch? if so clear, otherwise just return
        const bool found = current.exp == exp and current.run == run;
        if (!found) {
          current.exp = exp;
          current.run = run;
          current.map.clear();
        }
        return {found, &current.map};
      }
    private:
      CacheEntry current; /**< currently valid conditions */
      CacheEntry previous; /**< previously valid conditions */
    };

    /** Default constructible */
    MetadataProvider() = default;
    /** Default destructible */
    virtual ~MetadataProvider() = default;
    /** Set the list of globaltag names to be considered for payloads.
     *
     * This should be called before requesting any payloads but after calling
     * setValidTagStates().
     *
     * @return true if all globaltags can be found and are in a valid state */
    bool setTags(const std::vector<std::string>& tags);
    /** Update the information in the vector of metadata instances with the
     * actual values.
     *
     * The input is a list of metadata instances and the metadata provider will
     * try to modify them in place and add all the missing information it can
     * find.
     *
     * * It will try to find iov, revision, checksum, url for each entry which
     *   doesn't have a valid revision number (revision=0)
     * * It will ignore any entries with a valid revision number (revision>0)
     * * any payload which cannot be found in any globaltag will raise an error
     *   as long as required=true
     * * the modified list can still contain payloads without a valid revision
     *   some payloads could not be found
     *
     * @param exp the experiment number
     * @param run the run number
     * @param[in,out] info a list of metadata instances which we try to find the
     *     missing metadata for.
     * @return true if all required payloads could be found, false if at least
     *     one payload which still didn't have a valid revision number and is
     *     required could not be found.
     */
    bool getPayloads(int exp, int run, std::vector<PayloadMetadata>& info);
    /** Get the valid tag states when checking globaltag status */
    std::set<std::string> getValidTagStates() { return m_validTagStates; }
    /** Set the valid tag states for this provider when checking globaltag status.
     * Should be called before setTags() if necessary */
    void setValidTagStates(const std::set<std::string>& states) { m_validTagStates = states; }
  protected:
    /** Check the status of a global tag with the given name.
     * Returns "" if the tag doesn't exist or any other error occured */
    virtual std::string getGlobaltagStatus(const std::string& name) = 0;
    /** Update the list of existing payloads from a given globaltag, exp and run combination.
     * Supposed to call addPayload() for each payload it finds.
     * @return true on success, false on failure (e.g. network or file problems)
     */
    virtual bool updatePayloads(const std::string& globaltag, int exp, int run) = 0;
    /** Add a payload information to the internal list. This should be called by
     * implementations during updatePayloads() for each payload found.
     * @param payload payload information filled from the globaltag
     * @param messagePrefix a message prefix to be shown for possible log
     *    messages to indicate the correct metadata provider
     */
    void addPayload(PayloadMetadata&& payload, const std::string& messagePrefix = "");

    /** List of globaltags to consider */
    std::vector<std::string> m_tags;
    /** Map of globaltag->known metadata */
    std::unordered_map<std::string, PayloadMetadataCache> m_cache;
    /** Map of known payloads for current conditions */
    PayloadMap* m_payloads{nullptr};
    /** Set of global tag states to consider valid (except for 'INVALID' which is always considered invalid) */
    std::set<std::string> m_validTagStates{"TESTING", "VALIDATED", "RUNNING", "PUBLISHED"};
  };

  /** Fallback provider if no providers are given: Will raise an error if used
   * but allows processing if no payloads are requested or all are taken
   * locally rom testing payload storage. */
  class NullMetadataProvider: public MetadataProvider {
    /** Nope, no update */
    bool updatePayloads([[maybe_unused]] const std::string& globaltag, [[maybe_unused]] int exp,
                        [[maybe_unused]] int run) override
    {
      return false;
    }
    /** When we get asked to check a global tag we raise an error ... but only once */
    std::string getGlobaltagStatus(const std::string& name) override;
    /** Have we already shown the error? */
    bool m_errorShown{false};
  };
} // namespace Belle2::Conditions
