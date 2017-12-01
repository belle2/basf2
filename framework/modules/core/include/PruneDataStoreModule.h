/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                                             *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <vector>
#include <regex>

namespace Belle2 {
  /**
   * Clears the content of the DataStore while it keeps entries matching the regex expression in the matchEntries option.
   * Note: Also all Relations will be cleared if they are not matched by one entry in the matchEntries list.
   * Certain DataStore objects will always be kept, as it is required by the framework to properly work with the DataStore.
   * You have to ensure the objects referenced by kept relations are also matched by one entry in the
   * matchEntries list so a relation does not point to nirvana.
   * This logic can be inverted to remove only the entries matched by the regex in the matchEntries parameter
   * by setting the parameter to keepMatchedEntries to False.
   **/
  class PruneDataStoreModule: public Module {

  public:

    /** Constructor */
    PruneDataStoreModule();

    /** Virtual Constructor to prevent memory leaks */
    virtual ~PruneDataStoreModule() = default;

    /** Prepare regex checks */
    void initialize() override;

    /** Prune datastore */
    void event() override;

  protected:
    /** Storing the option of branches to keep */
    std::vector<std::string> m_matchEntries;

    /** Branches to always keep because the are required by the framework
     * to properly work with the datastore
     * EventMetaData is used by RootOutput and PrintCollection to get event
     * information
     */
    std::vector<std::string> m_keepEntriesImplicit = {{ "EventMetaData" }};

    /**
     * If true, all entries matched by the RegEx expression are kept.
     * If false, matched entries will be removed.
     */
    bool m_keepMatchedEntries = true;

    /** Caching the regex expression for the keep check */
    std::vector < std::regex > m_compiled_regex;

    /** Caching the regex expression for the keep check */
    std::vector < std::regex > m_compiled_regex_implicit;

  };
} // end namespace Belle2
