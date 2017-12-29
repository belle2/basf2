/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLELOADERMODULE_H
#define PARTICLELOADERMODULE_H

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>

#include <analysis/dataobjects/ParticleList.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/VariableManager/Utility.h>

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
   *   - gamma                            (input MDST type = ECLCluster with hypothesis N1 only at the moment)
   *   - K_S0, Lambda0, converted photons (input MDST type = V0)
   *   - K_L0                             (input MDST type = KLMCluster)
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
     * The elements are: PDGCode, name, anti-list name, isListSelfConjugated, and the cut sequence
     */
    typedef std::tuple<int, std::string, std::string, bool, std::shared_ptr<Variable::Cut>> PList;
    /**
     * Enum for describing each element in the above tuple
     */
    enum PListIndex {
      c_PListPDGCode, c_PListName, c_AntiPListName, c_IsPListSelfConjugated, c_CutPointer
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
     * Loads ECLCluster object as Particle to StoreArray<Particle> and adds it to the ParticleList
     */
    void eclClustersToParticles();

    /**
     * Loads KLMCluster object as Particle to StoreArray<Particle> and adds it to the ParticleList
     */
    void klmClustersToParticles();

    /**
     * Loads V0 object as Particle of specified type to StoreArray<Particle> and adds it to the ParticleList
     */
    void v0sToParticles();

    /**
     * returns true if the PDG code determined from the decayString is valid
     */
    bool isValidPDGCode(const int pdgCode);

    /**
     * recursively append bottom of a particle's decay chain (daughters of mother, granddaughters of daughter and so on).
     */
    void appendDaughtersRecursive(Particle* mother);

    bool m_useMCParticles;  /**< Load MCParticle as Particle instead of the corresponding MDST dataobject */

    DecayDescriptor m_decaydescriptor; /**< Decay descriptor for parsing the user specifed DecayString */

    std::vector<std::tuple<std::string, std::string>>
                                                   m_decayStringsWithCuts; /**< Input DecayString specifying the particle being created/loaded. Particles need as well pass the selection criteria */


    std::vector<PList> m_MCParticles2Plists; /**< Collection of PLists that will collect Particles created from MCParticles */
    std::vector<PList> m_Tracks2Plists; /**< Collection of PLists that will collect Particles created from Tracks */
    std::vector<PList> m_V02Plists; /**< Collection of PLists that will collect Particles created from V0 */
    std::vector<PList> m_ECLClusters2Plists; /**< Collection of PLists that will collect Particles created from ECLClusters */
    std::vector<PList> m_KLMClusters2Plists; /**< Collection of PLists that will collect Particles created from KLMClusters */

    bool m_writeOut;  /**< toggle particle list btw. transient/persistent */
    bool m_addDaughters; /**< toggle addition of the bottom part of the particle's decay chain */

    int m_trackHypothesis; /**< pdg code for track hypothesis that should be used to create the particle */

    bool m_enforceFitHypothesis =
      false; /**<If true, a Particle is only created if a track fit with the particle hypothesis passed to the ParticleLoader is available. */
  };

} // Belle2 namespace

#endif
