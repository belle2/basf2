/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth, Patrick Ecker                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLTRACKBREMFINDER_H
#define ECLTRACKBREMFINDER_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <ecl/dataobjects/ECLShower.h>

namespace Belle2 {

  /**
   * Module to assign ECL Clusters resulting from Bremsstrahlung to the
   * primary electron track.
   */
  class ECLTrackBremFinderModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    ECLTrackBremFinderModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~ECLTrackBremFinderModule() = default;

    /** Use this to initialize resources or memory your module needs.
     *
     *  Also register any outputs of your module (StoreArrays, RelationArrays,
     *  StoreObjPtrs) here, see the respective class documentation for details.
     */
    virtual void initialize();

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event();

  private:

    /**
     * Factor which is multiplied onto the cluster position error to check for matches
     */
    double m_clusterAcceptanceFactor = 3.0f;

    /**
     * StoreArray name of the ECLClusters for brem matching
     */
    std::string m_param_eclClustersStoreArrayName = "";

    /**
     * StoreArray name of the Tracks for brem matching
     */
    std::string m_param_tracksStoreArrayName = "";

    /**
     * StoreArray name of the RecoTracks for brem matching
     */
    std::string m_param_recoTracksStoreArrayName = "";
  };

} //Belle2
#endif
