/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// dataobjects
#include <mdst/dataobjects/V0.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/EventExtraInfo.h>

#include <vector>
#include <tuple>
#include <string>
#include <memory>

namespace Belle2 {

  /**
   * Loads MDST dataobjects as Particle objects to the StoreArray<Particle> and collects them in specified ParticleList.
   * Charge conjugated particles are loaded as well and collected in the anti-ParticleList.
   *
   * The type of the particle to be loaded is specified via the decayString module parameter. The type of the MDST dataobject
   * that is used as an input is determined from the type of the particle.
   *
   * The following types of the particles can be loaded:
   *
   * o) charged final state particles (input MDST type = Tracks)
   *   - e+, mu+, pi+, K+, p, deuteron (and charge conjugated particles)
   *
   * o) neutral final state particles
   *   - gamma                            (input MDST type = ECLCluster with 'n photons' ECLCluster::Hypothesis::c_nPhotons)
   *   - K_S0, Lambda0, converted photons (input MDST type = V0)
   *   - K_L0, n0                         (input MDST type = KLMCluster, or ECLCluster with neutral hadron hypothesis)
   *
   * The following BASF2 relations are set by the ParticleLoader:
   *
   * o) in the case of charged final state particles
   *   - Particle <-> MCParticle
   *   - Particle <-> PIDLikelihood
   *
   * o) in the case of neutral final state particles
   *   - Particle <-> MCParticle
   *
   * In the case the useMCParticles module parameter is set to true the module loads specific MCParticle(s) as Particle(s) instead
   * of the MDST dataobjects. In this case any particle type can be specified via the decayString module parameter.
   *
   */
  class ParticleLoaderModule : public Module {

    /**
     * tuple for collecting everything we know about the ParticlList to be created.
     * The elements are: PDGCode, name, anti-list name, and isListSelfConjugated
     */
    typedef std::tuple<int, std::string, std::string, bool> PList;
    /**
     * Enum for describing each element in the above tuple
     */
    enum PListIndex {
      c_PListPDGCode, c_PListName, c_AntiPListName, c_IsPListSelfConjugated
    };

  public:

    /**
     * Constructor
     */
    ParticleLoaderModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * Terminate the Module.
     * This method is called at the end of data processing.
     */
    virtual void terminate() override;

  private:

    /**
     * Loads specified MCParticles as Particle to StoreArray<Particle>
     */
    void mcParticlesToParticles();

    /**
     * Loads specified MSDT object as Particle to StoreArray<Particle>
     */
    void mdstToParticle();

    /**
     * Loads Track object as Particle to StoreArray<Particle> and adds it to the ParticleList
     */
    void tracksToParticles();

    /**
     * Loads ECLCluster and KLMCluster object as Particle to StoreArray<Particle> and adds it to the ParticleList
     */
    void eclAndKLMClustersToParticles();

    /**
     * Loads V0 object as Particle of specified type to StoreArray<Particle> and adds it to the ParticleList
     */
    void v0sToParticles();

    /**
     * Loads ROE object as Particle of specified type to StoreArray<Particle> and adds it to the ParticleList
     */
    void roeToParticles();

    /**
     * Helper method to load ROE object as Particle
     */
    void addROEToParticleList(RestOfEvent* roe, int mdstIndex, int pdgCode = 0, bool isSelfConjugatedParticle = true);

    /**
     * returns true if the PDG code determined from the decayString is valid
     */
    bool isValidPDGCode(const int pdgCode);

    /**
     * recursively append bottom of a particle's decay chain (daughters of mother, granddaughters of daughter and so on).
     */
    void appendDaughtersRecursive(Particle* mother);

    StoreArray<Particle> m_particles; /**< StoreArray of Particles */
    StoreArray<MCParticle> m_mcparticles; /**< StoreArray of MCParticles */
    StoreArray<ECLCluster> m_eclclusters; /**< StoreArray of ECLCluster */
    StoreArray<KLMCluster> m_klmclusters; /**< StoreArray of KLMCluster */
    StoreArray<PIDLikelihood> m_pidlikelihoods; /**< StoreArray of PIDLikelihoods */
    StoreArray<Track> m_tracks; /**< StoreArray of Tracks */
    StoreArray<TrackFitResult> m_trackfitresults; /**< StoreArray of TrackFitResults */
    StoreObjPtr<EventExtraInfo> m_eventExtraInfo; /**< object pointer to event extra info */
    StoreObjPtr<ParticleExtraInfoMap> m_particleExtraInfoMap; /**< object pointer to extra info map */
    StoreArray<RestOfEvent> m_roes; /**< StoreArray of ROEs */
    StoreArray<V0> m_v0s; /**< StoreArray of V0s */

    bool m_useMCParticles;  /**< Load MCParticle as Particle instead of the corresponding MDST dataobject */

    bool m_useROEs;  /**< Switch to load ROE as Particle */

    DecayDescriptor m_decaydescriptor; /**< Decay descriptor for parsing the user specified DecayString */

    std::vector<std::string> m_decayStrings; /**< Input decay strings specifying the particles being created/loaded */

    std::vector<PList> m_MCParticles2Plists; /**< Collection of PLists that will collect Particles created from MCParticles */
    std::vector<PList> m_Tracks2Plists; /**< Collection of PLists that will collect Particles created from Tracks */
    std::vector<PList> m_V02Plists; /**< Collection of PLists that will collect Particles created from V0 */
    std::vector<PList> m_ROE2Plists; /**< Collection of PLists that will collect Particles created from V0 */
    std::vector<PList>
    m_ECLKLMClusters2Plists; /**< Collection of PLists that will collect Particles created from ECLClusters and KLMClusters */

    bool m_writeOut;  /**< toggle particle list btw. transient/persistent */
    bool m_addDaughters; /**< toggle addition of the bottom part of the particle's decay chain */
    bool m_skipNonPrimaryDaughters; /**< toggle skip of secondary MC daughters */
    std::string m_roeMaskName; /**< ROE mask name to load */
    std::string m_sourceParticleListName; /**< Particle list name from which we need to get related ROEs */
    bool m_useMissing; /**< Use missing momentum to build a particle */
    int m_trackHypothesis; /**< pdg code for track hypothesis that should be used to create the particle */

    bool m_enforceFitHypothesis =
      false; /**<If true, a Particle is only created if a track fit with the particle hypothesis passed to the ParticleLoader is available. */

    std::vector<int> m_chargeZeroTrackCounts; /**< internally used to count number of tracks with charge zero */
    std::vector<int> m_sameChargeDaughtersV0Counts; /**< internally used to count the number of V0s with same charge daughters*/
  };

} // Belle2 namespace
