/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/Module.h>

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/algorithms/StandaloneCosmicsCollector.h>

namespace Belle2 {
  /**
   * The TrackFinderVXDCosmicsStandaloneModule
   * Track finder for linear cosmic tracks measured by the VXD without magnetic field and without other subdetectors.
   * The used linear fit uses a principal component analysis based on all SpacePoints in the event.
   * Thus this module assumes only one track in the event. Outlier SpacePoints due to noise and background can be
   * rejected by removing the SpacePoint with the largest contribution to the reduced chi squared value of the fit and
   * refitting afterwards. If the maximal required reduced chi square value is not reached after a given number of such
   * retries, the event is discarded.
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
    std::vector<std::string> m_spacePointsNames;

    /// The storeArray for SpacePoints
    std::vector<StoreArray<SpacePoint>> m_spacePoints;

    /// Name for StoreArray<SpacePointTrackCand> to be filled
    std::string m_PARAMSpacePointTrackCandArrayName;

    /// Line fit algorithm used for VXD standalone cosmics track finding
    StandaloneCosmicsCollector m_SCC;

    /** Sorting mode used for adding new SpacePoints of a candidate. Depending on set index, this will sort the SPs
     *  1: by radius
     *  2: by x value
     *  3: by y value
     */
    unsigned short m_sortingMode = 3;

    /// Module parameter to set maximal reduced chi2 value allowed for the fit
    double m_qualityCut = 0.0001;
    /// Module parameter setting the minimal required number of SpacePoints for the fit
    int m_minSPs = 3;
    /// Parameter for the maximal number of SpacePoints to be excluded for a refit before rejecting the event
    int m_maxRejectedSPs = 5;

    /// Output container: StoreArray for the TCs created in this module
    StoreArray<Belle2::SpacePointTrackCand> m_TCs;
  private:
  };
}
