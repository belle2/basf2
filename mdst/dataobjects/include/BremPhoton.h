/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 * Contributors: Patrick Ecker                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>

namespace Belle2 {
  /**
   * Class to make information about bremsstrahlung photons found by the ECLTrackBremFinder available
   * on mdst level.
   * In general this class is used to relate the track (i.e. electron) with an ECLCluster (bremsstrahlung photon) using
   * the detour of the BremPhoton to distinguish the relation from the default track cluster relation used track-cluster matching.
   * Track <-> BremPhoton <-> ECLCluster
   */
  class BremPhoton : public RelationsObject {
  public:
    /**
     * Default constructor
     */
    BremPhoton() = default;

    /**
     * Constructor which handles the relation between the bremsstrahlung cluster and the track to which the cluster was found.
     *
     */
    BremPhoton(const Track* track, const ECLCluster* bremCluster, double accFactor)
    {
      addRelationTo(track, accFactor);
      addRelationTo(bremCluster, accFactor);
    }

    ClassDef(BremPhoton, 1);
  };
}
