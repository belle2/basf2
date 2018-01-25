/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Ziegler                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef ECLTRACKINGPERFORMANCEMODULE_H_
#define ECLTRACKINGPERFORMANCEMODULE_H_

#include <framework/core/Module.h>
#include <tracking/modules/standardTrackingPerformance/ParticleProperties.h>

// forward declarations
class TTree;
class TFile;

namespace Belle2 {
  class MCParticle;
  class Track;
  class TrackFitResult;
  class RecoTrack;
  class ExtHit;
  class ECLCluster;

  template< class T >
  class StoreArray;


  /** This module takes the MCParticle and the genfit::Track collection as input and
   * writes out a root file with some information of the reconstructed tracks.
   * If a generated track is not reconstructed, all output variables are set to
   * the default value (-999). With the output file, you are able to estimate the
   * reconstruction efficiency of tracks
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
    std::vector< int > m_signalDaughterPDGs; /**< PDG codes of the B daughters of the interesting decay channel */

    TFile* m_outputFile; /**< output root file */
    TTree* m_dataTree; /**< root tree with all output data. Tree will be written to the output root file */

    /**< vector with all interesting charged stable MCParticles in the event */
    std::vector<const MCParticle*> m_interestingChargedStableMcParcticles;

    /**< vector with all MCParticles of the searched signal decay */
    std::vector<MCParticle*> m_signalMCParticles;

    /**< properties of a reconstructed track */
    ParticleProperties m_trackProperties;

    /**< pValue of track fit */
    double m_pValue;

    /**< total number of genrated charged stable MCParticles */
    double m_nGeneratedChargedStableMcParticles;

    /**< total number of reconstructed track candidates */
    double m_nReconstructedChargedStableTracks;

    /**< total number of fitted tracks */
    double m_nFittedChargedStabletracks;

    /**< boolean for matched ECL cluster */
    double m_photonCluster;

    /**< boolean whether matched to ECL cluster with highest weight */
    int m_matchedToECLClusterWithHighestWeight;

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

    /**
     * Find all interesting charged final state particles.
     * If no decay chain is specified, take all charged stable ones.
     */
    void findChargedStableMcParticles();

    /** Find a MCParticle of a decay chain specified by the user (not implemented yet). */
    void findSignalMCParticles(const StoreArray< MCParticle >& mcParticles);

    /**
     * Add all charged stable particles to a vector which originate from.
     * */
    void addChargedStable(const MCParticle& mcParticle);

    /**
     * Remove all photons from a MCParticle vector.
     * @param daughters: vector is taken and photons returned
     * @return: MCParticle vector with no photons
     */
    std::vector<MCParticle*> removeFinalStateRadiation(const std::vector<MCParticle*>& in_daughters);

    /**
     * Tests if mcParticle has the searched decay chain.
     * @param mcParticle: MCParticle of interest
     * @return: true if MCParticle decays in the given decay chain, else false
     */
    bool isSignalDecay(const MCParticle& mcParticle);
  };


} // end of namespace



#endif /* ECLTRACKINGPERFORMANCEMODULE_H_ */
