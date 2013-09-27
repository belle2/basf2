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
    bool isPrimaryMcParticle(MCParticle& mcParticle);
    bool isChargedStable(MCParticle& mcParticle);

    void setVariablesToDefaultValue();

    /** find all interesting charged final state particles
     * if no decay chain is specified, take all charged stable ones.
     */
    void findChargedStableMcParticles();

    void findSignalMCParticles(StoreArray< MCParticle >& mcParticles);

    void setupTree();

    double calculateModulus(double xx, double yy, double zz);

    GFTrack* findRelatedTrack(MCParticle& mcParticle, StoreArray< GFTrack >& gfTracks);
    const TrackFitResult* findRelatedTrackFitResult(const GFTrack* gfTrack);

    bool findRelatedTrack(MCParticle& mcParticle);

    void addChargedStable(MCParticle* mcParticle);

    std::vector< MCParticle* > removeFinalStateRadiation(const std::vector< MCParticle* >& in_daughters);

    bool isSignalDecay(MCParticle& mcParticle);

    void createParticleFromTrack(Track* track, int pdg);

    void writeData();

    std::string m_outputFileName;
    std::string m_gfTrackColName;
    std::string m_trackColName;
    std::vector< int > m_signalDaughterPDGs;
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
