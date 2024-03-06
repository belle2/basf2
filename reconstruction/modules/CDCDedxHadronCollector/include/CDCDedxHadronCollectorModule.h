/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once


#include <calibration/CalibrationCollectorModule.h>

#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <reconstruction/dbobjects/CDCDedxWireGain.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/utility/ReferenceFrame.h>

#include <reconstruction/dbobjects/CDCDedxInjectionTime.h>

#include <TTree.h>
#include <TString.h>
#include <TH1D.h>
#include <TH1I.h>
#include <TMath.h>
#include <vector>

namespace Belle2 {
  /**
   * A collector module for CDC dE/dx hadron calibrations
   *
   */
  class CDCDedxHadronCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    CDCDedxHadronCollectorModule();

    /**
     * Create and book ROOT objects
     */
    virtual void prepare() override;

    /**
     * Fill ROOT objects
     */
    virtual void collect() override;


  private:
    /**
    * Electron collector variables
    */
    bool m_cuts; /**< Boolean to apply cuts during collection */

    std::vector<std::string> m_strParticleList; /**< Vector of ParticleLists to write out */

    /// required input
    StoreObjPtr<SoftwareTriggerResult> m_trgResult; /**< Store array for Trigger selection */
    StoreArray<CDCDedxTrack> m_dedxTracks; /**< Required array for CDCDedxTracks */
    StoreArray<Track> m_tracks; /**< Required array for Tracks */
    StoreArray<TrackFitResult> m_trackFitResults; /**< Required array for TrackFitResults */

    std::map<std::string, std::string> m_pdgMap;

    /// module params
    int m_maxHits; /**< maximum number of hits allowed */
    double m_setEoP; /**< Cut of E over P value */

    /// track level information
    double m_dedx{ -1}; /**< dE/dx truncated mean */
    double m_dedxnosat{ -1}; /**< dE/dx truncated mean */
    double m_costh{ -1}; /**< track cos(theta) */
    double m_p{ -1}; /**< track momentum */
    int m_charge{0}; /**< track momentum */
    int m_nhits{ -1}; /**< number of dE/dx hits on the track */
    double m_timeReso{ -1};

    DBObjPtr<CDCDedxInjectionTime> m_DBInjectTime; /**< Injection time DB object */

  };
}
