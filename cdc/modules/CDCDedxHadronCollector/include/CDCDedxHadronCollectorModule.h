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

#include <cdc/dataobjects/CDCDedxTrack.h>
#include <cdc/dbobjects/CDCDedxWireGain.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/utility/ReferenceFrame.h>

#include <cdc/dbobjects/CDCDedxInjectionTime.h>

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
    * Hadron collector variables
    */
    std::vector<std::string> m_strParticleList; /**< Vector of ParticleLists to write out */

    std::map<std::string, std::string> m_pdgMap;   /**< map to relate input strings to the right particle type */

    /// module params
    double m_minCut; /**< low cut dedx curve for proton sample */
    double m_maxCut; /**< high cut dedx curve for proton sample */

    /// track level information
    double m_dedx{ -1}; /**< dE/dx truncated mean */
    double m_dedxnosat{ -1}; /**< dE/dx truncated mean no-saturation */
    double m_costh{ -1}; /**< track cos(theta) */
    double m_p{ -1}; /**< track momentum */
    int m_charge{0}; /**< track charge */
    int m_nhits{ -1}; /**< number of dE/dx hits on the track */

    /// event level information
    double m_injRing{ -1}; /**< her or ler  */
    double m_injTime{ -1}; /**< time since last injection */
    double m_timeReso{ -1}; /**< injection time resolution */

    DBObjPtr<CDCDedxInjectionTime> m_DBInjectTime; /**< Injection time DB object */

  };
}
