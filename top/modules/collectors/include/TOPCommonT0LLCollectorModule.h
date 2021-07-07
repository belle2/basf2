/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <top/utilities/TrackSelector.h>
#include <top/reconstruction_cpp/PDFConstructor.h>

#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPCalCommonT0.h>

#include <string>
#include <vector>

namespace Belle2 {

  /**
   * Collector for common T0 calibration with neg. log likelihood minimization (method LL)
   */
  class TOPCommonT0LLCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    TOPCommonT0LLCollectorModule();

  private:

    /**
     * Replacement for initialize(). Register calibration dataobjects here as well
     */
    virtual void prepare() final;

    /**
     * Replacement for event(). Fill your calibration data objects here
     */
    virtual void collect() final;

    /** Enumerator for sizes */
    enum {c_numModules = 16, /**< number of modules */
          c_numSets = 32,  /**< number of statistically independent subsamples */
         };

    // steering parameters
    int m_bunchesPerSSTclk; /**< number of bunches per SST clock */
    int m_numBins;      /**< number of bins to which search region is divided */
    double m_timeRange; /**< time range in which to search for the minimum [ns] */
    double m_sigmaSmear;  /**< additional smearing of PDF in [ns] */
    std::string m_sample; /**< sample type */
    double m_deltaEcms; /**< c.m.s energy window */
    double m_dr; /**< cut on POCA in r */
    double m_dz; /**< cut on POCA in z */
    double m_minZ; /**< minimal local z of extrapolated hit */
    double m_maxZ; /**< maximal local z of extrapolated hit */
    std::string m_pdfOption;   /**< PDF option name */

    // procedure
    TOP::TrackSelector m_selector; /**< track selection utility */
    TOP::PDFConstructor::EPDFOption m_PDFOption = TOP::PDFConstructor::c_Rough; /**< PDF option */
    std::vector<std::string> m_names; /**< histogram names of chi2 scans */

    // collections
    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreArray<Track> m_tracks;    /**< collection of tracks */
    StoreArray<ExtHit> m_extHits;  /**< collection of extrapolated hits */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */

    // database
    DBObjPtr<TOPCalCommonT0> m_commonT0;   /**< common T0 calibration constants */

    // other
    double m_bunchTimeSep = 0; /**< bunch separation in time [ns] */

  };

} // end namespace Belle2
