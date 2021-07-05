/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPFrontEndSetting.h>

namespace Belle2 {

  /**
   * Event T0 finder for global cosmic runs
   */
  class TOPCosmicT0FinderModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPCosmicT0FinderModule();

    /**
     * Destructor
     */
    virtual ~TOPCosmicT0FinderModule()
    {}

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

  private:

    // steering parameters
    bool m_useIncomingTrack; /**< if true use incoming track, otherwise use outcoming */
    unsigned m_minHits; /**< minimal number of hits on TOP module */
    double m_minSignal; /**< minimal number of expected signal photons */
    bool m_applyT0; /**< if true, subtract T0 in TOPDigits */
    int m_numBins;      /**< number of bins to which time range is divided */
    double m_timeRange; /**< time range in which to search [ns] */
    double m_sigma;     /**< additional time spread added to PDF [ns] */
    bool m_saveHistograms; /**< flag to save histograms */

    // database
    DBObjPtr<TOPFrontEndSetting> m_feSetting;  /**< front-end settings */

    // other
    int m_num = 0; /**< histogram number */
    int m_acceptedCount = 0;  /**< counter for accepted events */
    int m_successCount = 0; /**< counter for successfully determined T0 */

  };

} // Belle2 namespace

