/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric,  Dan Santel                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPDQMMODULE_H
#define TOPDQMMODULE_H

// I copied 6 lines below from PXDDQMModule.h - is it realy needed?
#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif

#include <framework/core/Module.h>
#include <top/geometry/TOPGeometryPar.h>
#include <string>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /**
   * TOP DQM histogrammer
   */
  class TOPDQMModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    TOPDQMModule();

    /**
     * Destructor
     */
    virtual ~TOPDQMModule();

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    virtual void defineHisto();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    // module parameters
    std::string m_storeDigitsName;        /**< TOPDigits StoreArray name */
    std::string m_histogramDirectoryName; /**< histogram directory in ROOT file */

    // TOP geometry parameters
    TOP::TOPGeometryPar* m_topgp;   /**< geometry parameters */

    // histograms
    TH1F* m_barHits;  /**< number of hits per bar */
    std::vector<TH1F*> m_channelHits;   /**< number of hits per channel for each bar */
    std::vector<TH1F*> m_hitTimes;   /**< time distribution for each bar */

  };

} // Belle2 namespace

#endif
