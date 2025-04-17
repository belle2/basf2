/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {
  /** The PartialSelect module.
   *
   * This module helps restrict the processing to set window of events.
   * 'entryStart' and 'entryStop' parameters can be set to define the range.
   * This module should be used with a conditional basf2 path to control the
   * limit the processing to the set range.
   */
  class PartialSelectModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of
     * the module.
     */
    PartialSelectModule();

    /** Initialization fetches the event count in the input file (m_nEvents).*/
    void initialize() override;

    /**
     * Checks if we are within the passed window to set return value to True,
     * and False if not.
     */
    void event() override;


  private:

    /** Total number of events in the input file.*/
    int m_nTotal = 0;

    /**
     * Input parameter [0.0 -> 1.0) used to determine event start
     * id: (m_entryStart*m_nTotal).
     */
    double m_entryStart;

    /**
     * Input parameter (0.0 -> 1.0] used to determine event stop
     * id: (m_entryStop*m_nTotal).
     */
    double m_entryStop;

    /**
     * Flag that will be returned by the module. Gets set to True inside the
     * window [(m_entryStart*m_nTotal), (m_entryStop*m_nTotal)] and False
     * outside.
     */
    bool m_returnValue = true;

    /**
     * Keeps track of how many events have been processed for so far.
     * Helps determine if we are inside the window or not.
     */
    int m_events = 0;

  };
}
