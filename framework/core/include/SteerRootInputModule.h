/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/MergedEventConsistency.h>

namespace Belle2 {
  /** Internal module used by Path.add_independent_merge_path(). Don't use it directly.
  The module controls event flow for event embedding (reading two separate input files) and mixing (merging events from the same file).
  The main decision is taken based on the event metadata (experiment, run, event number),
  some additional input is read from MergedEventConsistency datastore object.
  A valid combination is based on the return value of the event() method.
  */
  class SteerRootInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description, the properties and the parameters of the module.
     */
    SteerRootInputModule();

    /** setter for Path. */
    void init(bool eventMixing, bool mergeSameFile);

    ~SteerRootInputModule();

    virtual void initialize() override;
    virtual void event() override;

  private:

    /** do event mixing (merge each event of main path with each event of independent path) */
    bool m_eventMixing = false;

    /** if you want to merge a file with itself, mixing evts (1,3) is the same as (3,1) */
    bool m_mergeSameFile = false;

    /** set if current event is the last one to be processed  */
    bool m_processedLastEvent = false;

    /** We want to use the EventMetaData for some basic checks */
    StoreObjPtr<EventMetaData> m_eventMetaData_main = StoreObjPtr<EventMetaData>("EventMetaData");

    /** We want to use the EventMetaData for some basic checks */
    StoreObjPtr<EventMetaData> m_eventMetaData_indep = StoreObjPtr<EventMetaData>("EventMetaData_indepPath");

    /** Check if charge of both events is consistent (otherwise skip) */
    StoreObjPtr<MergedEventConsistency> m_mergedEventConsistency;

  };
}
