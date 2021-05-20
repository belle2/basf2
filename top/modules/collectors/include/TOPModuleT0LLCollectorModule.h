/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019, 2021 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
#include <top/dbobjects/TOPCalModuleT0.h>

#include <string>
#include <vector>

namespace Belle2 {

  /**
   * Collector for module T0 calibration with neg. log likelihood minimization
   * (method LL).
   *
   * Aimed for the final (precise) calibration after the initial (rough) calibration with
   * DeltaT method was done.
   */
  class TOPModuleT0LLCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    TOPModuleT0LLCollectorModule();

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
          c_numSets = 4,  /**< number of statistically independent subsamples */
         };

    // steering parameters
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
    std::vector<std::string> m_names[c_numSets]; /**< histogram names of chi2 scans */

    // collections
    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreArray<Track> m_tracks;    /**< collection of tracks */
    StoreArray<ExtHit> m_extHits;  /**< collection of extrapolated hits */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */

    // database
    DBObjPtr<TOPCalModuleT0> m_moduleT0;   /**< module T0 calibration constants */

  };

} // end namespace Belle2
