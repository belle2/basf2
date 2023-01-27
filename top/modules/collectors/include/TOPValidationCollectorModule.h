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

#include <framework/database/DBObjPtr.h>
#include <top/dbobjects/TOPCalTimebase.h>
#include <top/dbobjects/TOPCalChannelT0.h>
#include <top/dbobjects/TOPCalChannelMask.h>
#include <top/dbobjects/TOPCalChannelThresholdEff.h>
#include <top/dbobjects/TOPCalAsicShift.h>
#include <top/dbobjects/TOPCalEventT0Offset.h>
#include <top/dbobjects/TOPCalFillPatternOffset.h>

#include <top/utilities/TrackSelector.h>
#include <top/utilities/Chi2MinimumFinder1D.h>
#include <top/reconstruction_cpp/PDFConstructor.h>
#include <top/calibration/ValidationTreeStruct.h>

#include <string>
#include <vector>

namespace Belle2 {

  /**
   * Collector for automatic validation of calibration with dimuon events
   */
  class TOPValidationCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    TOPValidationCollectorModule();

  private:

    /**
     * Replacement for initialize(). Register calibration dataobjects here as well
     */
    virtual void prepare() final;

    /**
     * Replacement for event(). Fill your calibration data objects here
     */
    virtual void collect() final;

    /**
     * Replacement for beginRun(). Do anything you would normally do in beginRun here
     */
    virtual void startRun() final;

    /**
     * Replacement for endRun(). Do anything you would normally do in endRun here.
     */
    virtual void closeRun() final;

    /**
     * Sizes
     */
    enum {c_numModules = TOP::ValidationTreeStruct::c_numModules,  /**< number of modules */
          c_numChannels = 512, /**< number of channels per module */
          c_numSets = 2,  /**< number of statistically independent subsamples */
         };

    // steering parameters
    int m_numBins;      /**< number of bins to which search region is divided */
    double m_timeRange; /**< time range in which to search for the minimum [ns] */
    double m_sigmaSmear;  /**< additional smearing of PDF in [ns] */
    std::string m_sample; /**< sample type */
    double m_deltaEcms; /**< c.m.s energy window if sample is "dimuon" or "bhabha" */
    double m_dr; /**< cut on POCA in r */
    double m_dz; /**< cut on POCA in z */
    double m_minZ; /**< minimal local z of extrapolated hit */
    double m_maxZ; /**< maximal local z of extrapolated hit */
    std::string m_pdfOption;   /**< PDF option name */

    // procedure
    TOP::TrackSelector m_selector; /**< track selection utility */
    TOP::PDFConstructor::EPDFOption m_PDFOption = TOP::PDFConstructor::c_Rough; /**< PDF option */
    std::vector<std::string> m_namesChi; /**< histogram names of chi2 scans */
    std::vector<std::string> m_namesHit; /**< histogram names of photon hits (time vs. channel) */
    std::vector<TOP::Chi2MinimumFinder1D> m_finders[c_numSets]; /**< minimum finders, vector index = slot - 1 */

    // collections
    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreArray<Track> m_tracks;    /**< collection of tracks */
    StoreArray<ExtHit> m_extHits;  /**< collection of extrapolated hits */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */

    // payloads
    DBObjPtr<TOPCalTimebase> m_timebase;   /**< sample time calibration constants */
    DBObjPtr<TOPCalChannelT0> m_channelT0; /**< channel T0 calibration constants */
    DBObjPtr<TOPCalChannelMask> m_channelMask; /**< list of dead/noisy channels */
    DBObjPtr<TOPCalChannelThresholdEff> m_thresholdEff; /**< channel threshold effi. */
    DBObjPtr<TOPCalAsicShift> m_asicShift; /**< ASIC shifts calibration constants */
    DBObjPtr<TOPCalEventT0Offset> m_eventT0Offset; /**< detector components offsets w.r.t TOP */
    DBObjPtr<TOPCalFillPatternOffset> m_fillPatternOffset; /**< fill pattern offset */

    // tree variables
    TOP::ValidationTreeStruct m_treeEntry; /**< tree entry */
  };

} // end namespace Belle2
