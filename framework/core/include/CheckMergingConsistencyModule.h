/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/dataobjects/MergedEventExtraInfo.h>
#include <framework/dataobjects/MergedEventConsistency.h>

#include <framework/core/Module.h>

#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  /**
   * If you want to merge two events with the 'MergeDataStoreModule',
   * it might be necessary to make sure that this combination of events make sense.
   * This module typically checks if the charge is consistent (otherwise events will be skipped later).
   */
  class CheckMergingConsistencyModule : public Module {

  public:

    /**
     * Constructor
     */
    CheckMergingConsistencyModule();

    /** setter for Path. */
    void init(std::string option, bool eventMixing);

    ~CheckMergingConsistencyModule();

    virtual void initialize() override;
    virtual void event() override;

  private:

    /** different options foreseen to check consistency (right now only charge) */
    std::string m_option = "charge";

    /** do event mixing (merge each event of main path with each event of independent path) */
    bool m_eventMixing = false;

    /**< object that stores whether events are consistent */
    StoreObjPtr<MergedEventConsistency> m_mergedEventConsistency;

    /**< MergedEventExtraInfo from main path */
    StoreObjPtr<MergedEventExtraInfo> m_mergedEventExtraInfo_main = StoreObjPtr<MergedEventExtraInfo>("MergedEventExtraInfo");
    /**< MergedEventExtraInfo from independent path */
    StoreObjPtr<MergedEventExtraInfo> m_mergedEventExtraInfo_indep =
      StoreObjPtr<MergedEventExtraInfo>("MergedEventExtraInfo_indepPath");

  };

} // Belle2 namespace

