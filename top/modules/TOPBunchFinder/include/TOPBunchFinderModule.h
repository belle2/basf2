/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <map>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRawDigit.h>
#include <mdst/dataobjects/Track.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <top/dataobjects/TOPTimeZero.h>
#include <framework/dataobjects/MCInitialParticles.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPCalCommonT0.h>
#include <top/dbobjects/TOPFrontEndSetting.h>

#include <top/reconstruction_cpp/PDFConstructor.h>
#include <top/utilities/Chi2MinimumFinder1D.h>


namespace Belle2 {

  /**
   * Bunch finder: searches for the bunch crossing where the interaction happened
   * using track-based TOP likelihood
   */
  class TOPBunchFinderModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPBunchFinderModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     */
    virtual void beginRun() override;

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

    /**
     * Returns most probable charged stable particle according to dEdx
     * and predefined prior probabilities
     * @param track reconstructed track
     * @return charged stable
     */
    Const::ChargedStable getMostProbable(const Track& track);

    /**
     * Sets finder object with chi2 values
     * @param finder finder object
     * @param reco reconstruction object
     * @param timeMin lower edge of photon time window
     * @param timeMax upper edge of photon time window
     * @return number of photons in the time window
     */
    int setFinder(TOP::Chi2MinimumFinder1D& finder, const TOP::PDFConstructor& reco, double timeMin, double timeMax);

    // steering parameters
    int m_numBins;      /**< number of bins to which the fine search region is divided */
    double m_timeRangeFine; /**< time range in which to do fine search [ns] */
    double m_timeRangeCoarse; /**< time range in which to do coarse search if autoRange turned off [ns] */
    bool m_autoRange; /**< determine coarse range automatically */
    double m_sigmaSmear;  /**< additional smearing of PDF in [ns] */
    double m_minSignal;   /**< minimal number of signal photons */
    double m_minSBRatio;  /**< minimal signal-to-background ratio */
    double m_minDERatio;  /**< minimal ratio of detected over expected photons */
    double m_maxDERatio;  /**< maximal ratio of detected over expected photons */
    double m_minPt; /**< minimal p_T of track */
    double m_maxPt; /**< maximal p_T of track */
    double m_maxD0; /**< maximal absolute value of helix perigee distance */
    double m_maxZ0; /**< maximal absolute value of helix perigee z coordnate */
    int m_minNHitsCDC; /**< minimal number of hits in CDC */
    bool m_useMCTruth;    /**< use MC truth for mass instead of dEdx most probable */
    bool m_saveHistograms; /**< flag to save histograms */
    double m_tau; /**< first order filter time constant [events] */
    bool m_fineSearch; /**< use fine search */
    bool m_correctDigits; /**< subtract bunch time in TOPDigits */
    bool m_subtractRunningOffset; /**< subtract running offset when running in HLT mode */
    int m_bunchesPerSSTclk; /**< number of bunches per SST clock */
    bool m_usePIDLikelihoods; /**< if true, use PIDLikelihoods (only on cdst files) */
    unsigned m_nTrackLimit; /**< maximum number of tracks (inclusive) to use three particle hypotheses in fine search */

    // internal variables shared between events
    double m_bunchTimeSep = 0; /**< time between two bunches */
    std::map<int, double> m_priors; /**< map of PDG codes to prior probabilities */
    double m_runningOffset = 0; /**< running average of bunch offset */
    double m_runningError = 0; /**< error on running average */
    bool m_HLTmode = false; /**< use running average to correct digits */
    unsigned m_processed = 0; /**< processed events */
    unsigned m_success = 0; /**< events with reconstructed bunch */
    int m_nodEdxCount = 0; /**< counter of tracks with no dEdx, reset at each event */

    // collections
    StoreArray<TOPDigit> m_topDigits; /**< collection of TOP digits */
    StoreArray<TOPRawDigit> m_topRawDigits; /**< collection of TOP raw digits */
    StoreArray<Track> m_tracks; /**< collection of tracks */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */
    StoreObjPtr<MCInitialParticles> m_initialParticles; /**< simulated beam particles */
    StoreArray<TOPTimeZero> m_timeZeros; /**< collection of T0 of individual tracks */
    StoreObjPtr<EventT0> m_eventT0; /**< event T0 */

    // database
    DBObjPtr<TOPCalCommonT0> m_commonT0;   /**< common T0 calibration constants */
    DBObjPtr<TOPFrontEndSetting> m_feSetting;  /**< front-end settings */

  };

} // Belle2 namespace

