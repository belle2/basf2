/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Metzner                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <utility>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/Module.h>

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/tcTools/SpacePointTrackCandCreator.h>
#include <tracking/trackFindingVXD/algorithms/StandaloneCosmicsCollector.h>


namespace Belle2 {
  /**
   * The TrackFinderVXDCosmicsStandaloneModule
   * Track finder for linear cosmic tracks measured by the VXD without magnetic field and without other subdetectors.
   * The used linear fit uses a principal component analysis based on all SpacePoints in the event.
   * Thus this module assumes only one track in the event. Outlier SpacePoints due to noise and background can be
   * rejected by removing the SpacePoint with the largest contribution to the chi squared value of the fit and refitting
   * afterwards. If the maximal required chi square value is not reached after a given number of such retries, the event
   * is discarded.
   * If successful, the obtained track candidate starts at the outermost hit (based on x^2+y^2) and proceeds towards the
   * next inner hits. The seed position for the track is set to this outermost SpacePoint. The seed momentum is set to
   * be in the direction of the obtained line, with an arbitrary, but rather large magnitude. The charge and PDG code
   * of the Candidate is set arbitrarily.
   */
  class TrackFinderVXDCosmicsStandaloneModule final : public Module {

  public:

    /// Constructor
    TrackFinderVXDCosmicsStandaloneModule();

    /// Initialize
    void initialize() override;

    /// event
    void event() override;


  protected:
    /// SpacePoint collection name
    std::string m_spacePointsName;

    /// The storeArray for SpacePoints
    StoreArray<SpacePoint> m_spacePoints;

    /// Name for StoreArray<SpacePointTrackCand> to be filled
    std::string m_PARAMSpacePointTrackCandArrayName;

    /// Line fit algorithm used for VXD standalone cosmics track finding
    StandaloneCosmicsCollector m_SCC;

    /// Module parameter to set maximal chi2 value allowed for the fit
    double m_qualityCut = 0.0001;
    /// Module parameter setting the minimal required number of SpacePoints for the fit
    int m_minSPs = 3;
    /// Parameter for the maximal number of SpacePoints to be excluded for a refit before rejecting the event
    int m_maxRejectedSPs = 5;

    /// Event number counter
    unsigned int m_eventCounter = 0;

    /// Output container: StoreArray for the TCs created in this module
    StoreArray<Belle2::SpacePointTrackCand> m_TCs;
  private:
  };
}
