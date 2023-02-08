/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Basf2 headers. */
#include <analysis/dataobjects/ParticleList.h>
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Expert.h>
#include <mva/interface/Weightfile.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  /**
   * Calculation of low-energy pi0 veto value.
   */
  class LowEnergyPi0VetoExpertModule : public Module {

  public:

    /**
     * Constructor.
     */
    LowEnergyPi0VetoExpertModule();

    /**
     * Destructor.
     */
    ~LowEnergyPi0VetoExpertModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override;

    /**
     * This method is called at the end of the event processing.
     */
    void terminate() override;

  private:

    /**
     * Initialize mva expert, dataset and features
     * Called everytime the weightfile in the database changes in begin run
     */
    void init_mva(MVA::Weightfile& weightfile);

    /**
     * Get maximum veto value over all gamma pairs including the photon gamma1.
     * If pi0Gamma is not nullptr, then it is excluded from the search.
     * @param[in] gamma1   This photon.
     * @param[in] pi0Gamma Photon to be excluded (other pi0 daughter).
     */
    float getMaximumVeto(const Particle* gamma1, const Particle* pi0Gamma);

    /**
     * Calculate veto for pi0 daughter photons
     * (maximum over all pairs excluding this pi0).
     * If false, the veto is calculated for photons.
     */
    bool m_VetoPi0Daughters;

    /** Gamma particle list name. */
    std::string m_GammaListName;

    /** Pi0 particle list name. */
    std::string m_Pi0ListName;

    /** Belle 1 data analysis. */
    bool m_Belle1;

    /** Gamma candidates. */
    StoreObjPtr<ParticleList> m_ListGamma;

    /** Pi0 candidates. */
    StoreObjPtr<ParticleList> m_ListPi0;

    /** Database identifier or file used to load the weights. */
    std::string m_identifier = "LowEnergyPi0Veto";

    /** Database pointer to the database representation of the weightfile. */
    std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>> m_weightfile_representation;

    /** Pointer to the current MVA expert. */
    std::unique_ptr<MVA::Expert> m_expert;

    /** Pointer to the current dataset. */
    std::unique_ptr<MVA::SingleDataset> m_dataset;

  };

}
