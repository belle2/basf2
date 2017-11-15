#pragma once
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <vector>

namespace Belle2 {
  /**
   * Module to set event, run, experiment numbers.
   *
   * This module generates the event meta data information
   * like experiment, run and event number. It is meant to
   * be used in cases where no module generating this information
   * (e.g. an input module loading a file) is available.
   *
   * @sa EventMetaData
   */
  class EventInfoSetterModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description, the properties and the parameters of the module.
     */
    EventInfoSetterModule();

    /** Destructor. */
    virtual ~EventInfoSetterModule();

    /** Initializes the Module.
     *
     * Does a check of the parameters and gives an error of the parameters
     * were not correctly set by the user.
     */
    virtual void initialize();

    /** Advances member variables to the next event (which is given by m_evtNumber). */
    bool advanceEventCounter();

    /** Stores the event meta data into the DataStore.
     *
     * Based on the parameters set by the user the current event meta data
     * is calculated (event number, run number, experiment number) and stored
     * into the event part of the DataStore.
     *
     * No object is created when the end of of processing is reached.
     */
    virtual void event();


  protected:


  private:

    StoreObjPtr<EventMetaData> m_eventMetaDataPtr; /**< Output object. */

    unsigned int m_evtNumber{1}; /**< The current event number. */
    unsigned int m_eventsToSkip{0u}; /**< skip this many events before starting. */
    int m_colIndex{0};  /**< The current index for the exp and run lists. */
    int m_production{0};  /**< The production number. */

    std::vector<int> m_expList{0};      /**< The list (column) of experiments. */
    std::vector<int> m_runList{0};      /**< The list (column) of runs. */
    std::vector<unsigned int> m_evtNumList{1u};   /**< The list (column) of the number of events which should be processed. */
    std::vector<int> m_skipToEvent;   /**< The (expNo, runNo, evtNo) tuple to skip to, empty if not used */
  };
}
