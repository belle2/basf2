/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Hauth                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>

namespace Belle2 {
  /** Prints the contents of DataStore in each event, listing all objects and arrays (including size). */
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

    /** Parameter to set the event number for which the collections should be printed.
     * -1 means that only the first event will be printer, 0 means that the collections will be printed for all events. */
    int m_printForEvent = -1;

    /** boolean to check if the current event is the first encountered */
    bool m_firstEvent = true;
  };
}
