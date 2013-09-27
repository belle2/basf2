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
class GFTrack;

namespace Belle2 {
  class MCParticle;
  class Track;
  class TrackFitResult;
  class Particle;

  template< class T >
  class StoreArray;

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

    /** search for a GFTrack which is related to a given MCParticle and return pointer to the GFTrack if existing
     * @param mcParticle: MCParticle, to which the GFTrack must belong
     * @param gfTracks: StoreArray with GFTrack objects in the event
     * @return: pointer to the found, related track, else NULL is returned
     */
    GFTrack* findRelatedTrack(MCParticle& mcParticle, StoreArray< GFTrack >& gfTracks);

    /** search a valid TrackFitResult for a given GFTrack
     * @param gfTrack: TrackFitResult must be related to this GFTrack
     * @return: pointer to the found TrackFitResult if found, else NULL is returned
     */
    const TrackFitResult* findRelatedTrackFitResult(const GFTrack* gfTrack);

    /** find a related Track to a given MCParticle
     * @param mcParticle: Track must be related to this MCParticle
     * @return: true is returned if a related track was found, else false is returned
     */
    bool findRelatedTrack(MCParticle& mcParticle);

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

    /** take a Track and PDG code and create a Particle object. The Particle is added to a vector of Particles
     * @param track: pointer to a Track
     * @param pdg: particle hypothesis
     */
    void createParticleFromTrack(Track* track, int pdg);

    /** write root tree to output file and close the file */
    void writeData();


    std::string m_outputFileName; /**< name of output root file */
    std::string m_gfTrackColName; /**< GFTrack collection name */
    std::string m_trackColName; /**< Track collection name */
    std::vector< int > m_signalDaughterPDGs; /**< PDG codes of the B daughters of the interesting decay channel */


    TFile* m_outputFile;
    TTree* m_dataTree;

    std::vector< MCParticle* > m_interestingChargedStableMcParcticles;
    std::vector< MCParticle* > m_signalMCParticles;

    std::vector< Particle > m_chargedStableParticles;

    ParticleProperties m_mcParticleProperties;
    ParticleProperties m_trackProperties;

    double m_nGeneratedChargedStableMcParticles;
    double m_nFittedChargedStabletracks;

  };


} // end of namespace



#endif /* STANDARDTRACKINGPERFORMANCEMODULE_H_ */
