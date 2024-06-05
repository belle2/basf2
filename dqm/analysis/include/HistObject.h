/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TH1.h>

namespace Belle2 {

  /**
   * Class to keep track of delta histograms
   */
  class HistObject {
  public:
    TH1* m_hist{};/**< Pointer to histogram */
    bool m_updated = false; /**< flag if update since last event */
    double m_entries = -1; /**< entries in previous/current for update check */
  public:

    /** Constructor
     */
    HistObject(void) : m_hist(nullptr), m_updated(false), m_entries(-1) {};

    /** Check if update of histogram is necessary
     * @param hist pointer to histogram
     * @return histogram was updated flag (return m_updated)
     */
    bool update(TH1* hist);

    /** Reset histogram and update flag, not the entries
     */
    void resetBeforeEvent(void);

    /** Check if hist was updated
     * @return it was updated
     */
    bool isUpdated(void) { return m_updated;};

    /** Get hist pointer
    * @return hist ptr
    */
    TH1* getHist(void) { return m_hist;};

  };
}
