/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Ziegler                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef STANDARDTRACKINGPERFORMANCEMODULE_H_
#define STANDARDTRACKINGPERFORMANCEMODULE_H_

#include <framework/core/Module.h>
#include <tracking/modules/standardTrackingPerformance/ParticleProperties.h>

// forward declarations
class TTree;
class TFile;
namespace genfit { class Track; }

namespace Belle2 {
  class MCParticle;
  class Track;
  class TrackFitResult;

  template< class T >
  class StoreArray;

  /**
   *  \addtogroup modules
   *  @{
   *  \addtogroup tracking_modules
   *  \ingroup modules
   *  @{ StandardTrackingPerformanceModule @} @}
   */

  /** This module takes the MCParticle and the genfit::Track collection as input and
   * writes out a root file with some information of the reconstructed tracks.
   * If a generated track is not reconstructed, all output variables are set to
   * the default value (-999). With the output file, you are able to estimate the
   * reconstruction efficiency of tracks
   */
  class StandardTrackingPerformanceModule : public Module {
  public:
    StandardTrackingPerformanceModule();
    virtual ~StandardTrackingPerformanceModule();

    /** Use this to initialize resources or memory your module needs.
     *
     *  Also register any outputs of your module (StoreArrays, RelationArrays,
     *  StoreObjPtrs) here, see the respective class documentation for details. */
    virtual void initialize();

    /** Called once before a new run begins.
     *
     * This method gives you the chance to change run dependent constants like alignment parameters, etc. */
    virtual void beginRun();

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything. */
    virtual void event();

    /** Called once when a run ends.
     *
     *  Use this method to save run information, which you aggregated over the last run. */
    virtual void endRun();

    /** Clean up anything you created in initialize(). */
    virtual void terminate();

  private:
    /** tests if MCParticle is a primary one
     * @param mcParticle: tested MCParticle
     * @return: true if MCParticle is a primary, else false is returned
     */
    bool isPrimaryMcParticle(MCParticle& mcParticle);

    /** tests if MCPArticle is a charged stable particle
     * @param mcParticle: tester MCParticle
     * @return: true if MCParticle is charged stable, else false
     */
    bool isChargedStable(MCParticle& mcParticle);

    /** sets all variables to the default value, here -999 */
    void setVariablesToDefaultValue();

    /** find all interesting charged final state particles
     * if no decay chain is specified, take all charged stable ones.
     */
    void findChargedStableMcParticles();

    /** find a MCParticle of a decay chain specified by the user ( not implemented yet) */
    void findSignalMCParticles(StoreArray< MCParticle >& mcParticles);

    /** add branches to data tree */
    void setupTree();

    /** search for a genfit::Track which is related to a given MCParticle and return pointer to the genfit::Track if existing
     * @param mcParticle: MCParticle, to which the genfit::Track must belong
     * @param gfTracks: StoreArray with genfit::Track objects in the event
     * @return: pointer to the found, related track, else NULL is returned
     */
    genfit::Track* findRelatedTrack(MCParticle& mcParticle, StoreArray< genfit::Track >& gfTracks);

    /** search a valid TrackFitResult for a given genfit::Track
     * @param gfTrack: TrackFitResult must be related to this genfit::Track
     * @return: pointer to the found TrackFitResult if found, else NULL is returned
     */
    const TrackFitResult* findRelatedTrackFitResult(const genfit::Track* gfTrack);


    /** add all charged stable particles to a vector which originate from
     * @param mcParticle
     * */
    void addChargedStable(MCParticle* mcParticle);

    /** remove all photons from a MCParticle vector
     * @param daughters: vector is taken and photons returned
     * @return: MCParticle vector with no photons
     */
    std::vector< MCParticle* > removeFinalStateRadiation(const std::vector< MCParticle* >& in_daughters);

    /** tests if mcParticle has the searched decay chain
     * @param mcParticle: MCParticle of interest
     * @return: true if MCParticle decays in the given decay chain, else false
     */
    bool isSignalDecay(MCParticle& mcParticle);

    /** write root tree to output file and close the file */
    void writeData();

    void addVariableToTree(std::string varName, double& varReference);


    std::string m_outputFileName; /**< name of output root file */
    std::string m_gfTrackColName; /**< genfit::Track collection name */
    std::string m_trackColName; /**< Track collection name */
    std::vector< int > m_signalDaughterPDGs; /**< PDG codes of the B daughters of the interesting decay channel */


    TFile* m_outputFile; /**< output root file */
    TTree* m_dataTree; /**< root tree with all output data. Tree will be written to the output root file */

    /**< vector with all interesting charged stable MCParticles in the event */
    std::vector< MCParticle* > m_interestingChargedStableMcParcticles;

    /**< vector with all MCParticles of the searched signal decay */
    std::vector< MCParticle* > m_signalMCParticles;

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

  };


} // end of namespace



#endif /* STANDARDTRACKINGPERFORMANCEMODULE_H_ */
