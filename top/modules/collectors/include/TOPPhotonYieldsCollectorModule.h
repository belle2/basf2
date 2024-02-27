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
#include <framework/dataobjects/EventMetaData.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <top/dataobjects/TOPAsicMask.h>
#include <top/dataobjects/TOPAssociatedPDF.h>
#include <top/utilities/TrackSelector.h>
#include <top/dbobjects/TOPCalChannelMask.h>
#include <top/dbobjects/TOPCalChannelThresholdEff.h>
#include <vector>
#include <string>

namespace Belle2 {

  /**
   * Collector for photon pixel yields aimed for PMT ageing studies and for finding optically decoupled PMT's
   */
  class TOPPhotonYieldsCollectorModule : public CalibrationCollectorModule {

  public:
    /**
     * Constructor
     */
    TOPPhotonYieldsCollectorModule();

  private:

    /**
     * Replacement for initialize(). Register calibration dataobjects here as well
     */
    virtual void prepare() final;

    /**
     * Replacement for event(). Fill your calibration data objects here
     */
    virtual void collect() final;

    // steering parameters
    std::string m_sample; /**< sample type */
    double m_deltaEcms; /**< c.m.s energy window */
    double m_dr; /**< cut on POCA in r */
    double m_dz; /**< cut on POCA in z */
    double m_minThresholdEffi; /**< minimal threshold efficiency */

    // cuts (do not change the values)
    const double m_minZ = -130.0; /**< minimal local z of extrapolated track */
    const double m_maxZ = 130.0; /**< maximal local z of extrapolated track */
    const double m_excludedZ = 50.0; /**< excluded central region of extrapolated track for photon impact angle counting */
    const double m_timeWindow = 50.0; /**< time window for counting photon hits (half size) */

    // procedure
    TOP::TrackSelector m_selector; /**< track selection utility */
    std::vector<std::string> m_signalNames; /**< histogram names for signal window hit counts */
    std::vector<std::string> m_bkgNames; /**< histogram names for background window hit counts */
    std::vector<std::string> m_activeNames; /**< histogram names for active pixels count */
    std::vector<std::string> m_alphaLowNames; /**< histogram names for counting hits w/ low impact angle on photo cathode */
    std::vector<std::string> m_alphaHighNames; /**< histogram names for counting hits w/ high impact angle on photo cathode */
    std::vector<std::string> m_pulseHeightNames; /**< histogram names for pulse heights */
    std::vector<std::string> m_muonZNames; /**< histogram names for track z-distribution */

    // collections
    StoreObjPtr<EventMetaData> m_eventMetaData; /**< event meta data object */
    StoreArray<Track> m_tracks;    /**< collection of tracks */
    StoreArray<ExtHit> m_extHits;  /**< collection of extrapolated hits */
    StoreArray<TOPDigit> m_digits; /**< collection of TOP digits */
    StoreArray<TOPAssociatedPDF> m_associatedPDFs; /**< collection of PDF's associated to TOP digits */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */
    StoreObjPtr<TOPAsicMask> m_asicMask; /**< online masked Asics */

    // database
    DBObjPtr<TOPCalChannelMask> m_channelMask; /**< masked channels */
    DBObjPtr<TOPCalChannelThresholdEff> m_thresholdEff; /**< threshold efficiencies */

  };

} // end namespace Belle2
