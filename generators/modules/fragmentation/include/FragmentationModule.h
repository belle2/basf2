/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <Pythia8/Pythia.h>
#include <generators/modules/fragmentation/EvtGenDecays.h>

#include <string>

namespace Belle2 {

  /** Minimal class for external random generator to be used in PYTHIA.
  */
  class FragmentationRndm : public Pythia8::RndmEngine {

  public:
    // Constructor
    FragmentationRndm();

    // in PYTHIA, there is one pure virtual method in RndmEngine,
    // to generate one random number flat in the range between 0 and 1:
    double flat(); /**< flat random generator. */

  private:

  };


  /** The Fragmentation module.
  */
  class FragmentationModule : public Module {

  public:
    /**
    * Constructor.
    * Sets the module parameters.
    */
    FragmentationModule();

    /** Destructor. */
    virtual ~FragmentationModule();

    virtual void event() override;      /**< Event method (process events) */
    virtual void initialize() override; /**< Initialize the module */
    virtual void terminate() override;  /**< terminate the module */

  protected:

    /**
     * Load EvtGen particle data.
     * @param[in,out] pythia Pythia generator.
     */
    void loadEvtGenParticleData(Pythia8::Pythia* pythia);

    /**
     * Add particle to Pythia event.
     * @param[in] mcParticle MC particle.
     */
    int addParticleToPYTHIA(const MCParticle& mcParticle);

    /** Pythia generator. */
    Pythia8::Pythia* m_Pythia = nullptr;

    /** Pythia event. */
    Pythia8::Event* m_PythiaEvent = nullptr;

    EvtGenDecays* evtgen; /**< EvtGen decay engine inside PYTHIA8 */

    /** Module parameters */
    std::string m_parameterfile; /**< PYTHIA input parameter file. */
    int m_listEvent;  /**< list event generated by PYTHIA. */
    int m_useEvtGen;  /**< use EvtGen for some decays. */
    std::string m_DecFile;  /**< EvtGen decay file */
    std::string m_UserDecFile;  /**< User EvtGen decay file */
    bool m_coherentMixing;  /**< decay the B0-B0bar coherently. */
    int m_quarkPairMotherParticle;  /**< PDG Code of the mother particle of the quark pair. */
    std::vector<int> m_additionalPDGCodes;  /**< Additional particles used in Pythia. */

    MCParticleGraph mcParticleGraph; /**< An instance of the MCParticle graph. */

    int nAll;   /**< number of events created. */
    int nGood;   /**< number of events with successful fragmentation. */

    int nAdded;  /**< number of added particles. */
    int nQuarks; /**< number of quarks. */
    int nVpho; /**< number of virtual exchange particles. */

    std::string m_particleList; /**< The name of the MCParticle collection. */
    StoreArray<MCParticle> m_mcparticles; /**< store array for the MCParticles */
  };

} // end namespace Belle2
