/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once
#include <framework/core/Module.h>
#include <analysis/modules/MCDecayFinder/DecayTree.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>

namespace Belle2 {
  /** Find decays in MCParticle list matching a given DecayString.
  Matched MCParticles are saved as Particle in a ParticleList. At the
  moment skipping of intermediate resonances is not implemented. */
  class MCDecayFinderModule : public Module {
  private:

    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of decays to look for. */
    std::string m_strDecay; /**< Decay string to build the decay descriptor. */
    std::string m_listName; /**< Name of output particle list. */
    std::string m_antiListName; /**< Name of output anti-particle list. */
    bool m_isSelfConjugatedParticle; /**< Is the particle list for a self-conjugated particle */
    bool m_writeOut;  /**< toggle output particle list btw. transient/writeOut */
    std::string m_particleStore; /**< name of Particle store array */

    StoreObjPtr<ParticleList> m_outputList; /**< output particle list */
    StoreObjPtr<ParticleList> m_antiOutputList; /**< output anti-particle list */
    StoreArray<Particle> m_particles; /**< StoreArray of Particles */
    StoreObjPtr<ParticleExtraInfoMap> m_extraInfoMap; /**< object pointer to ParticleExtraInfoMaps */
    StoreArray<MCParticle> m_mcparticles; /**< StoreArray of MCParticles */

  public:
    /** Constructor. */
    MCDecayFinderModule();
    /** Destructor. */
    ~MCDecayFinderModule() {}
    /** Initialises the module.*/
    virtual void initialize() override;
    /** Method called for each event. */
    virtual void event() override;
    /** Search for MCParticles matching the given DecayString. */
    DecayTree<MCParticle>* match(const MCParticle* mcp, const DecayDescriptor* d, bool isCC);
    /** Create Particle from matched MCParticle and write to Particle list. */
    int write(DecayTree<MCParticle>* decay);
    /** Recursively gather all MC daughters of gen **/
    void appendParticles(const MCParticle* gen, std::vector<const MCParticle*>& children);
    /** Recursively get number of daughters of given DecayDescriptor **/
    int getNDaughtersRecursive(const DecayDescriptor* d);
  };
}
