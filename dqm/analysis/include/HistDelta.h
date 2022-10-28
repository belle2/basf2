/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TH1.h>
#include <queue>
#include <vector>

namespace Belle2 {

  /**
   * Class to keep track of delta histograms
   */
  class HistDelta {
  public:
    /** enum definition for delta algo
     * Disabled: nothing
     * Entries: use nr histogram entries
     * Underflow: use entries in histogram underflow bin
     * Events: use nr processed events from framework (but: unreliable for
     * normalization as the number of events for this specific histogram may differ)
    */
    enum EDeltaType { c_Disabled = 0, c_Entries = 1, c_Underflow = 2, c_Events = 3};
    EDeltaType m_type{}; /**< type of delta algo */
    int m_parameter{}; /**< parameter depending on algo, e.g. nr of entries or events */
    unsigned int m_amountDeltas{}; /**< amount of past histograms, at least 1*/
    TH1* m_lastHist{};/**< Pointer to last histogram state for check */
    int m_lastValue{}; /**< last value for comparison, depending on type */
    std::vector<TH1*> m_deltaHists;/**< vector of histograms (max m_amountDeltas) */
    bool m_updated{};/**< if any delta was updated in this event */
  public:

    /** Construktor
     * @param t type
     * @param p parameter for type
     * @param a amount of deletas in the past
     */
    HistDelta(EDeltaType t = c_Disabled, int p = 0, unsigned int a = 0);

    /** Parameter setter
     * @param t type
     * @param p parameter for type
     * @param a amount of deltas in the past
     */
    void set(EDeltaType t, int p, unsigned int a);

    /** Set not-updated yet status
     */
    void setNotUpdated(void) {m_updated = false;};

    /** Check if update of delta histogram is necessary
     * @param hist pointer to histogram
     */
    void update(TH1* hist);

    /** Reset histogram and deltas, not the parameters
     */
    void reset(void);

    /** Get Delta Histogram
     * @param n number of delta into the past, 0 is most recent one
     * @param onlyIfUpdated req only updated deltas, return nullptr otherwise
     * @return Found histogram or nullptr
     */
    TH1* getDelta(unsigned int n = 0, bool onlyIfUpdated = true);
  };
}
