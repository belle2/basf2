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

#include <ecl/dataobjects/ECLShower.h>
#include <framework/core/Module.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/modules/standardTrackingPerformance/ParticleProperties.h>

// forward declarations
class TTree;
class TFile;

namespace Belle2 {
  class MCParticle;
  class Track;
  class TrackFitResult;
  class RecoTrack;
  class ECLCluster;
  class ECLShower;

  template< class T >
  class StoreArray;


  /** This module takes the MCParticle collection as input and checks if the
   * related reconstructed track is matched to an ECLCluster. This information
   * as well as some track properties are written out to a root file.
   */

  class ECLTrackingPerformanceModule : public Module {
  public:
    ECLTrackingPerformanceModule();

    /** Register the needed StoreArrays and open th output TFile. */
    void initialize();

    /** Fill the tree with the event data.  */
    void event();

    /** Write the tree into the opened root file. */
    void terminate();

  private:
    std::string m_outputFileName; /**< name of output root file */
    std::string m_recoTracksStoreArrayName; /**< genfit::Track collection name */

    // Required input
    StoreArray<ECLCluster> m_eclClusters; /** Required input array of ECLClusters */
    StoreArray<ECLShower> m_eclShowers; /** Required input array of ECLShowers */
    StoreArray<MCParticle> m_mcParticles; /** Required input array of MCParticles */
    StoreArray<RecoTrack> m_recoTracks; /** Required input array of RecoTracks */
    StoreArray<Track> m_tracks; /** Required input array of Tracks */
    StoreArray<TrackFitResult> m_trackFitResults; /** Required input array of TrackFitResults */

    TFile* m_outputFile; /**< output root file */
    TTree* m_dataTree; /**< root tree with all output data. Tree will be written to the output root file */

    /**< properties of a reconstructed track */
    ParticleProperties m_trackProperties;

    /**< Experiment number */
    int m_iExperiment;

    /**< Run number */
    int m_iRun;

    /**< Event number */
    int m_iEvent;

    /**< pValue of track fit */
    double m_pValue;

    /**< charge */
    int m_charge;

    /**< signed distance of the track to the IP in the r-phi plane */
    double m_d0;

    /**< distance of the track to the IP along the beam axis */
    double m_z0;

    /**< energy of ECLCluster belonging to matched MCParticle */
    double m_mcparticle_cluster_energy;

    /**< boolean for match between MCParticle and ECL cluster */
    int m_mcparticle_cluster_match;

    /**< boolean for match between track and ECL cluster */
    int m_matchedToECLCluster;

    /**< hypothesis of matched ECL cluster */
    int m_hypothesisOfMatchedECLCluster;

    /**< boolean for photon being related to matched ECL cluster */
    int m_photonCluster;

    /**< boolean whether matched to ECL cluster with highest weight */
    int m_sameclusters;

    /**
     * Sets all variables to the default value, here -999.
     * */
    void setVariablesToDefaultValue();

    /** add branches to data tree */
    void setupTree();

    /** write root tree to output file and close the file */
    void writeData();

    /** add a variable with double format */
    void addVariableToTree(const std::string& varName, double& varReference);

    /** add a variable with int format */
    void addVariableToTree(const std::string& varName, int& varReference);

    /**
     * Tests if MCParticle is a primary one.
     * @param mcParticle: tested MCParticle
     * @return: true if MCParticle is a primary, else false is returned
     */
    bool isPrimaryMcParticle(const MCParticle& mcParticle);

    /**
     * Tests if MCPArticle is a charged stable particle.
     * @param mcParticle: tester MCParticle
     * @return: true if MCParticle is charged stable, else false
     */
    bool isChargedStable(const MCParticle& mcParticle);
  };

} // end of namespace
