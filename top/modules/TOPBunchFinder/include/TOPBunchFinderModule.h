/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <string>
#include <map>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <top/dataobjects/TOPDigit.h>
#include <mdst/dataobjects/Track.h>
#include <top/dataobjects/TOPRecBunch.h>

namespace Belle2 {

  class CDCDedxLikelihood;
  class VXDDedxLikelihood;

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
    virtual void initialize();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    /**
     * Return mass of the most probable charged stable particle according to dEdx
     * and predefined prior probabilities
     * @param cdcdedx dE/dx likelihoods from CDC
     * @param vxddedx dE/dx likelihoods from VXD
     * @return mass
     */
    double getMostProbableMass(const CDCDedxLikelihood* cdcdedx,
                               const VXDDedxLikelihood* vxddedx);

    // steering parameters
    int m_numBins;      /**< number of bins to which search region is divided */
    double m_timeRange; /**< time range in which to do fine search [ns] */
    double m_sigmaSmear;  /**< additional smearing of PDF in [ns] */
    double m_minSignal;   /**< minimal number of signal photons */
    double m_minSBRatio;  /**< minimal signal-to-background ratio */
    double m_minDERatio;  /**< minimal ratio of detected over expected photons */
    double m_maxDERatio;  /**< maximal ratio of detected over expected photons */
    bool m_useMCTruth;    /**< use MC truth for mass instead of dEdx most probable */
    bool m_saveHistograms; /**< flag to save histograms */
    double m_tau; /**< first order filter time constant [events] */
    bool m_fineSearch; /**< use fine search */
    bool m_correctDigits; /**< subtract bunch time in TOPDigits */
    bool m_addOffset; /**< add running average offset to bunch time */
    double m_bias; /**< bias to be subtracted */
    int m_bunchesPerSSTclk; /**< number of bunches per SST clock */

    // internal variables shared between events
    double m_bunchTimeSep; /**< time between two filled bunches */
    std::map<int, double> m_priors; /**< map of PDG codes to prior probabilities */
    double m_offset = 0; /**< running average offset to the reconstructed bunch */
    double m_error = 0; /**< error on running average offset */
    unsigned m_eventCount = 0; /**< event counter */
    unsigned m_processed = 0; /**< processed events */
    unsigned m_success = 0; /**< events with reconstructed bunch */

    // collections
    StoreArray<TOPDigit> m_topDigits; /**< collection of TOP digits */
    StoreArray<Track> m_tracks; /**< collection of tracks */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */

  };

} // Belle2 namespace

