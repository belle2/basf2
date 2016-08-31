/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                                             *
 *                                                                        *
 **************************************************************************/

#ifndef PruneDatastoreModule_H
#define PruneDatastoreModule_H

#include <framework/core/Module.h>
#include <string>
#include <vector>

namespace Belle2 {
  /**
   * Clears the content of the DataStore while it keeps entries listed in the keepEntries option
   */
  class PruneDatastoreModule: public Module {

  public:

    /** Constructor */
    PruneDatastoreModule();

    /** Virtual Constructor to prevent memory leaks */
    virtual ~PruneDatastoreModule() = default;

    /** Prune datastore */
    virtual void event();

  protected:
    std::vector<std::string> m_keepEntries;

  };
} // end namespace Belle2

#endif // PruneDatastoreModule_H
