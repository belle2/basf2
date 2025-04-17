/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>

#include <string>

namespace Belle2 {
  /** This dataobject is used by EventLimiter module to set event limit based on the value from the database.
   *
   * The main use case for this dataobject is to cut off bad data from the run
   * that was stopped due to data quality problems.
   */
  class EventLimit : public TObject {
  public:

    /** Constructor. */
    EventLimit(long event = -1) : m_event_limit(event) {}

    /** Destructor. */
    ~EventLimit() {}

    /** Event Limit Setter.
     */
    void setEventLimit(long event) { m_event_limit = event; }

    /** Event Limit Getter.
     */
    long getEventLimit() const { return m_event_limit; }

  private:
    /** Event limit.
     * All events > m_event_limit are excluded.
     * If m_event_limit is -1, the value is ignored.
     */
    long m_event_limit;

    /** This dataobject is used by EventLimiter module to set event limit based on the value from the database. */
    ClassDef(EventLimit, 1);
  }; //class
} // namespace Belle2
