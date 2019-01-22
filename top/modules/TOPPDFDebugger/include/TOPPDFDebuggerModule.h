/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jan strube, Marko Staric                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/Track.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPPDFCollection.h>

#include <string>
#include <top/reconstruction/TOPreco.h>

namespace Belle2 {

  /**
   * TOP reconstruction module.
   */
  class TOPPDFDebuggerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPPDFDebuggerModule();

    /**
     * Destructor
     */
    virtual ~TOPPDFDebuggerModule();

    /**
     * Initialize the Module.
     *
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     *
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     *
     */
    virtual void event() override;

    /**
     * End-of-run action.
     *
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     *
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;


  private:

    // Module steering parameters
    double m_minBkgPerBar = 0; /**< minimal assumed background photons per bar */
    double m_scaleN0 = 0;      /**< scale factor for N0 */
    double m_maxTime = 0;      /**< optional time limit for photons */
    double m_minTime = 0;      /**< optional time limit for photons */
    int m_writeNPdfs = 0;      /**< write out pdfs for the first N events */
    int m_writeNPulls = 0;     /**< write out pulls for the furst N events */
    std::string m_pdfOption;   /**< PDF option name */
    std::vector<int> m_pdgCodes;   /**< particle codes */

    // others
    int m_debugLevel = 0;       /**< debug level from logger */
    long long m_iEvent = -1;   /**< count events in the current process */
    TOP::TOPreco::PDFoption m_PDFOption = TOP::TOPreco::c_Rough; /**< PDF option */

    // Masses of particle hypotheses

    std::vector<double> m_masses;  /**< particle masses */

    // collections

    StoreArray<TOPPDFCollection> m_pdfCollection; /**< collection of analytic PDF's */
    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreArray<Track> m_tracks;  /**< collection of tracks */

  };

} // Belle2 namespace

