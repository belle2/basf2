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
#include <tracking/modules/standardTrackingPerformance/ParticleProperties.h>

// forward declarations
class TTree;
class TFile;

namespace Belle2 {

  /** This module takes the MCParticle collection as input and checks if the
   * related reconstructed track is matched to an ECLCluster. This information
   * as well as some track properties are written out to a root file.
   */

  class ECLTrackClusterMatchingPerformanceModule : public Module {
  public:
    ECLTrackClusterMatchingPerformanceModule();

    /** Register the needed StoreArrays and open th output TFile. */
    void initialize() override;

    /** Fill the tree with the event data.  */
    void event() override;

    /** Write the tree into the opened root file. */
    void terminate() override;

  private:
    std::string m_outputFileName; /**< name of output root file */
    double m_minClusterEnergy; /**< minimal cluster energy in units of particle's true energy */
    double m_minWeight; /**< fraction of cluster energy */
    std::string m_trackClusterRelationName; /**< name of relation array between tracks and ECL clusters */

    // Required input
    StoreArray<ECLCluster> m_eclClusters; /**< Required input array of ECLClusters */
    StoreArray<MCParticle> m_mcParticles; /**< Required input array of MCParticles */
    StoreArray<Track> m_tracks; /**< Required input array of Tracks */
    StoreArray<TrackFitResult> m_trackFitResults; /**< Required input array of TrackFitResults */

    /** Event metadata. */
    StoreObjPtr<EventMetaData> m_EventMetaData;

    TFile* m_outputFile{nullptr}; /**< output root file */
    TTree* m_tracksTree{nullptr}; /**< MCParticle based root tree with all output data. Tree will be written to the output root file */
    TTree* m_clusterTree{nullptr}; /**< root tree containing information on all truth-matched photon clusters. Tree will be written to the output root file */

    /** properties of a reconstructed track */
    ParticleProperties m_trackProperties;

    /** Experiment number */
    int m_iExperiment = 0;

    /** Run number */
    int m_iRun = 0;

    /** Event number */
    int m_iEvent = 0;

    /** pValue of track fit */
    double m_pValue = 0.0;

    /** charge */
    int m_charge = 0;

    /** signed distance of the track to the IP in the r-phi plane */
    double m_d0 = 0.0;

    /** distance of the track to the IP along the beam axis */
    double m_z0 = 0.0;

    /** number of last CDC layer used for track fit */
    int m_lastCDCLayer = 0;

    /** detector region of cluster with photon hypothesis matched to track */
    int m_matchedPhotonHypothesisClusterDetectorRegion = 0;

    /** theta of cluster with photon hypothesis matched to track */
    double m_matchedPhotonHypothesisClusterTheta = 0.0;

    /** phi of cluster with photon hypothesis matched to track */
    double m_matchedPhotonHypothesisClusterPhi = 0.0;

    /** minimal distance between cluster with photon hypothesis and track (not necessarily the matched one) */
    double m_matchedPhotonHypothesisClusterMinTrkDistance = 0.0;

    /** delta l of cluster with photon hypothesis*/
    double m_matchedPhotonHypothesisClusterDeltaL = 0.0;

    /** detector region of cluster with hadron hypothesis matched to track */
    int m_matchedHadronHypothesisClusterDetectorRegion = 0;

    /** theta of cluster with hadron hypothesis matched to track */
    double m_matchedHadronHypothesisClusterTheta = 0.0;

    /** phi of cluster with hadron hypothesis matched to track */
    double m_matchedHadronHypothesisClusterPhi = 0.0;

    /** minimal distance between cluster with hadron hypothesis and track (not necessarily the matched one) */
    double m_matchedHadronHypothesisClusterMinTrkDistance = 0.0;

    /** delta l of cluster with hadron hypothesis*/
    double m_matchedHadronHypothesisClusterDeltaL = 0.0;

    /** detector region of cluster matched to MCParticle */
    int m_mcparticle_cluster_detectorregion = 0;

    /** theta of cluster matched to MCParticle */
    double m_mcparticle_cluster_theta = 0.0;

    /** phi of cluster matched to MCParticle */
    double m_mcparticle_cluster_phi = 0.0;

    /** amount of particle energy contained in cluster matched to MCParticle */
    double m_mcparticle_cluster_energy = 0.0;

    /** boolean for match between MCParticle and ECL cluster */
    int m_mcparticle_cluster_match = 0;

    /** boolean for match between track and ECL cluster with photon hypothesis */
    int m_matchedToPhotonHypothesisECLCluster = 0;

    /** boolean for match between track and ECL cluster with hadron hypothesis */
    int m_matchedToHadronHypothesisECLCluster = 0;

    /** boolean whether matched to ECL cluster with highest weight */
    int m_sameclusters = 0;

    /** azimuthal angle of cluster */
    double m_clusterPhi = 0.0;

    /** polar angle of cluster */
    double m_clusterTheta = 0.0;

    /** hypothesis ID of cluster */
    int m_clusterHypothesis = 0;

    /** cluster is matched to track */
    int m_clusterIsTrack = 0;

    /** cluster fulfills requirements for being product of a photon */
    int m_clusterIsPhoton = 0;

    /** cluster has related MCParticle which is charged and stable */
    int m_clusterIsChargedStable = 0;

    /** cluster energy */
    double m_clusterEnergy = 0.0;

    /** photon energy */
    double m_photonEnergy = 0.0;

    /** energy sum of central crystal over 3x3 array around central crystal */
    double m_clusterE1E9 = 0.0;

    /** cluster detection region */
    int m_clusterDetectorRegion = 0;

    /** cluster's timing uncertainty containing 99% of true photons */
    double m_clusterErrorTiming = 0.0;

    /** distance to closest track */
    double m_clusterMinTrkDistance = 0.0;

    /** delta l */
    double m_clusterDeltaL = 0.0;

    /** Sets all variables to the default value, here -999. */
    void setVariablesToDefaultValue();

    /** sets cluster related variables to default values */
    void setClusterVariablesToDefaultValue();

    /** add branches to data tree */
    void setupTree();

    /** write root tree to output file and close the file */
    void writeData();

    /** add a variable with double format */
    void addVariableToTree(const std::string& varName, double& varReference, TTree* tree);

    /** add a variable with int format */
    void addVariableToTree(const std::string& varName, int& varReference, TTree* tree);

    /**
     * Tests if MCParticle is a primary one.
     * @param mcParticle: tested MCParticle
     * @return: true if MCParticle is a primary, else false is returned
     */
    bool isPrimaryMcParticle(const MCParticle& mcParticle);

    /**
     * Tests if MCParticle is a charged stable particle.
     * @param mcParticle: tested MCParticle
     * @return: true if MCParticle is charged stable, else false
     */
    bool isChargedStable(const MCParticle& mcParticle);
  };

} // end of namespace
