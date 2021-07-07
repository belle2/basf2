/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    virtual void initialize() override;

    /** Prints information for each collection in the DataStore. */
    virtual void event() override;


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
