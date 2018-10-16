/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>

namespace Belle2 {
  /**
   * This module calculates some properties of ECL clusters.
   *
   * This module calculates for each ECL cluster the distance to the closest track and the depth of the shower.
   *
   */
  class ECLClusterPropertiesModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ECLClusterPropertiesModule();

    /** Destructor */
    virtual ~ECLClusterPropertiesModule();

    /** Initialize the required input arrays */
    virtual void initialize();

    /** Event loop */
    virtual void event();

    /** Delete pointer created during initialize */
    virtual void terminate();


  private:

    /** Minimal distance between track and shower. */
    double computeTrkMinDistance(const ECLShower&, StoreArray<Track>&) const;

    /** Compute depth. */
    void computeDepth(const ECLShower& shower, double& lTrk, double& lShower) const;

    // Required input
    StoreArray<ExtHit> m_extHits; /**< Required input array of ExtHits */
    StoreArray<Track> m_tracks; /** Required input array of Tracks */
    StoreArray<ECLCluster> m_eclClusters; /** Required input array of ECLClusters */
    StoreArray<ECLShower> m_eclShowers; /** Required input array of ECLShowers */
    StoreArray<ECLCalDigit> m_eclCalDigits; /** Required input array of ECLCalDigits */

  };
}
