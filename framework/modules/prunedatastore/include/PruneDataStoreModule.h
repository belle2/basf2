/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                                             *
 *                                                                        *
 **************************************************************************/

#ifndef PruneDataStoreModule_H
#define PruneDataStoreModule_H

#include <framework/core/Module.h>
#include <string>
#include <vector>
#include <boost/regex.hpp>

namespace Belle2 {
  /**
   * Clears the content of the DataStore while it keeps entries listed in the keepEntries option.
   * Note: Also all Relations will be cleared if they are not matched by one entry in the keepEntries list.
   * Certain DataStore objects will always be kept, as it is required by the framework to properly work with the DataStore.
   * You have to ensure the objects referenced by kept relations are also matched by one entry in the
   * keepEntries list so a relation does not point to nirvana.
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
    std::vector<std::string> m_keepEntries;

    /** Branches to always keep because the are required by the framework
     * to properly work with the datastore
     * EventMetaData is used by RootOutput and PrintCollection to get event
     * information
     */
    std::vector<std::string> m_keepEntriesImplicit = {{ "EventMetaData" }};

    /** Caching the regex expression for the keep check */
    std::vector < boost::regex > m_compiled_regex;

  };
} // end namespace Belle2

#endif // PruneDataStoreModule_H
