/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/dataobjects/EventExtraInfo.h>
#include <framework/dataobjects/MergedEventConsistency.h>

#include <framework/core/Module.h>

#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  /**
   * If you want to merge two events with the 'MergeDataStoreModule',
   * it might be necessary to make sure that this combination of events make sense.
   * This module typically checks if the charge is consistent (otherwise events will be skipped later).
   *
   * The main output of the module is the MergedEventConsistency datastore object that is used by the SteerRootInput module
   */
  class CreateConsistencyInfoModule : public Module {

  public:

    /**
     * Constructor
     */
    CreateConsistencyInfoModule();

    /** setter for Path. */
    void init(const std::string& option, bool eventMixing);

    ~CreateConsistencyInfoModule();

    virtual void initialize() override;
    virtual void event() override;

  private:

    /** different options foreseen to check consistency (right now only charge) */
    std::string m_option = "charge";

    /** do event mixing (merge each event of main path with each event of independent path) */
    bool m_eventMixing = false;

    /** object that stores whether events are consistent */
    StoreObjPtr<MergedEventConsistency> m_mergedEventConsistency;

    /** EventExtraInfo from main path */
    StoreObjPtr<EventExtraInfo> m_EventExtraInfo_main = StoreObjPtr<EventExtraInfo>("EventExtraInfo");
    /** EventExtraInfo from independent path */
    StoreObjPtr<EventExtraInfo> m_EventExtraInfo_indep =
      StoreObjPtr<EventExtraInfo>("EventExtraInfo_indepPath");

  };

} // Belle2 namespace

