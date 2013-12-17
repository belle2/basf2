/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PRINTCOLLECTIONSMODULE_H_
#define PRINTCOLLECTIONSMODULE_H_

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>

namespace Belle2 {
  /** The module prints the collections of the DataStore to the log system.
   *
   */
  class PrintCollectionsModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    PrintCollectionsModule();

    /** Destructor of the module. */
    virtual ~PrintCollectionsModule();

    /** initialization. */
    virtual void initialize();

    /** Prints information for each collection in the DataStore. */
    virtual void event();


  protected:

    /** Prints information for each collection in the DataStore using the specified durability.
     * @param durability The durability type of which the DataStore content should be printed.
     */
    void printCollections(DataStore::EDurability durability);


  private:

  };
}

#endif /* PRINTCOLLECTIONSMODULE_H_ */
