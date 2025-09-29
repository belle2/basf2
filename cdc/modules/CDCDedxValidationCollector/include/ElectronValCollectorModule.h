/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>

#include <calibration/CalibrationCollectorModule.h>

#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <framework/datastore/StoreArray.h>

#include <cdc/dataobjects/CDCDedxTrack.h>
#include <cdc/dbobjects/CDCDedxWireGain.h>



namespace Belle2 {
  /**
   * A collector module for CDC dE/dx electron calibrations
   *
   */
  class ElectronValCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ElectronValCollectorModule();

    /**
     * Create and book ROOT objects
     */
    virtual void prepare() override;

    /**
     * Fill ROOT objects
     */
    virtual void collect() override;

    /**
    * Set range for EoP cut
    */
    void setEoverPCutValue(double value) {m_setEoP = value;}

  private:
    /**
    * Electron collector variables
    */
    bool m_cuts; /**< Boolean to apply cuts during collection */

    /// required input
    StoreObjPtr<SoftwareTriggerResult> m_trgResult; /**< Store array for Trigger selection */
    StoreArray<CDCDedxTrack> m_dedxTracks; /**< Required array for CDCDedxTracks */
    StoreArray<Track> m_tracks; /**< Required array for Tracks */
    StoreArray<TrackFitResult> m_trackFitResults; /**< Required array for TrackFitResults */
    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB object */

    /// module params
    int m_maxHits; /**< maximum number of hits allowed */
    double m_setEoP; /**< Cut of E over P value */

    /// event level information
    double m_injRing{ -1}; /**< her or ler  */
    double m_injTime{ -1}; /**< time since last injection */

    /// track level information
    double m_pt = -1; /**< track transverse momentum */
    double m_dedx{ -1}; /**< dE/dx truncated mean */
    double m_costh{ -1}; /**< track cos(theta) */
    double m_p{ -1}; /**< track momentum */
    int m_charge{0}; /**< track momentum */
    int m_run{ -1}; /**<run number */
    int m_nhits{ -1}; /**< number of dE/dx hits on the track */

    /// hit level information
    std::vector<int> m_wire;       /**< wire number for the hit */
    std::vector<double> m_dedxhit; /**< dE/dx for the hit */
    std::vector<int> m_layer;      /**< continuous layer number for the hit */
    std::vector<double> m_enta;    /**< entrance angle for the hit */
    std::vector<double> m_entaRS;    /**< rescaled entrance angle for the hit */

  };
}
