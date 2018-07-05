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

#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>

#include "TFile.h"
#include "TTree.h"

namespace Belle2 {

  /** The module saves information on ExtHits and related cluster into a file.
   *  Using this information a parametrization for the RMS of the difference
   *  between the hit's and the cluster's polar and azimuthal angle can be found.
   */
  class ECLTrackClusterMatchingParametrizationExpertModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    ECLTrackClusterMatchingParametrizationExpertModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~ECLTrackClusterMatchingParametrizationExpertModule();

    /** Use this to initialize resources or memory your module needs.
     *
     *  Also register any outputs of your module (StoreArrays, RelationArrays,
     *  StoreObjPtrs) here, see the respective class documentation for details.
     */
    virtual void initialize();

    /** Called once before a new run begins.
     *
     * This method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun();

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event();

    /** Called once when a run ends.
     *
     *  Use this method to save run information, which you aggregated over the last run.
     */
    virtual void endRun();

    /** Clean up anything you created in initialize(). */
    virtual void terminate();

  private:

    /** Check if extrapolated hit is inside ECL and matches one of the desired categories. */
    bool isECLHit(const ExtHit& extHit) const;

    // required input
    StoreArray<ExtHit> m_extHits; /**< Required input array of ExtHits */
    StoreArray<Track> m_tracks; /**< Required input array of Tracks */
    StoreArray<TrackFitResult> m_trackFitResults; /**< Required input array of TrackFitResults */
    StoreArray<ECLCluster> m_eclClusters; /**< Required input array of ECLClusters */

    // optional input
    StoreObjPtr<EventMetaData> m_eventMetaData; /**< Optional input array of EventMetaData */
    StoreArray<MCParticle> m_mcParticles; /**< Optional input array of MCParticles */

    /** members of ECLTrackClusterMatching Module */

    TFile* m_rootFilePtr; /**< pointer at root file used for storing info */
    std::string m_rootFileName; /**< name of the root file */
    bool m_writeToRoot; /**< if true, a rootFile named by m_rootFileName will be filled with info */
    bool m_useArray; /**< if true, info is stored event-wise using array, otherwise hit-wise */
    TTree* m_tree; /**< Root tree for saving the output */

    // variables
    int m_iExperiment; /**< Experiment number */
    int m_iRun; /**< Run number */
    int m_iEvent; /**< Event number */

    int m_trackNo; /**< counter to distinguish tracks */
    double m_trackMomentum; /**< momentum of track */
    double m_pT; /**< transverse momentum of track */
    double m_deltaPhi; /**< difference in azimuthal angle between hit position and cluster */
    double m_phiCluster; /**< azimuthal angle of cluster */
    double m_phiHit; /**< azimuthal angle of hit position */
    double m_errorPhi; /**< uncertainty on azimuthal angle of hit */
    double m_deltaTheta; /**< difference in polar angle between hit position and cluster */
    double m_thetaCluster; /**< polar angle of cluster */
    double m_thetaHit; /**< polar angle of hit position */
    double m_errorTheta; /**< uncertainty on polar angle of hit */
    int m_hitstatus; /**< status of hit */
    int m_true_track_pdg; /**< PDG ID of track according to MC */

    std::vector<int>* m_trackNo_array; /**< array of track numbers */
    std::vector<double>* m_trackMomentum_array; /**< array of track momenta */
    std::vector<double>* m_deltaPhi_array; /**< array of differences in azimuthal angle between hit and cluster */
    std::vector<double>* m_phiCluster_array; /**< array of azimuthal angles of cluster */
    std::vector<double>* m_phiHit_array; /**< array of azimuthal angles of hit */
    std::vector<double>* m_errorPhi_array; /**< array of uncertainties on azimuthal angle of hit */
    std::vector<double>* m_deltaTheta_array; /**< array of differences in polar angle between hit and cluster */
    std::vector<double>* m_thetaCluster_array; /**< array of polar angles of cluster */
    std::vector<double>* m_thetaHit_array; /**< array of polar angles of hit */
    std::vector<double>* m_errorTheta_array; /**< array of uncertainties on polar angle of hit */
    std::vector<int>* m_hitstatus_array; /**< array of hit status */
    std::vector<int>* m_true_track_pdg_array; /**< array of true PDG IDs of track */
  };
} //namespace Belle2
