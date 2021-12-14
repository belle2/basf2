/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/dataobjects/MergedEventExtraInfo.h>
#include <analysis/dataobjects/EventExtraInfo.h>

#include <framework/core/Module.h>

#include <framework/datastore/StoreObjPtr.h>


namespace Belle2 {

  /**
   * TODO: This module could be a temporary solution if someone has a better idea....
   * I would probably move the EventExtraInfo from analysis to framework - it is anyways used all over our software
   * The whole DataStore merging business takes place in the framework package.
   * We want to be able to skip the merging of events if their charge is not consistent.
   * For that reason we have to somehow save this information on a 'framework' level
   * but the information about the charge is usually saved on the analysis level (EventExtraInfo).
   */
  class EventExtraInfoConverterModule : public Module {

  public:

    /**
     * Constructor
     */
    EventExtraInfoConverterModule();

    /**
     * Initialize the Module.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    StoreObjPtr<EventExtraInfo> m_eventExtraInfo; /**< the 'analysis' class storing the EventExtraInfo */
    StoreObjPtr<MergedEventExtraInfo> m_mergedEventExtraInfo; /**< corresponding 'framework' class */
  };

} // Belle2 namespace

