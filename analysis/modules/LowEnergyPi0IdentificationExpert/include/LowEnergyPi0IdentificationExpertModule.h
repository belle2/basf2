/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Analysis headers. */
#include <analysis/dataobjects/ParticleList.h>

/* Basf2 headers. */
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Expert.h>
#include <mva/interface/Weightfile.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  /**
   * Calculation of low-energy pi0 identification value.
   */
  class LowEnergyPi0IdentificationExpertModule : public Module {

  public:

    /**
     * Constructor.
     */
    LowEnergyPi0IdentificationExpertModule();

    /**
     * Destructor.
     */
    ~LowEnergyPi0IdentificationExpertModule();

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

    /** Pi0 particle list name. */
    std::string m_Pi0ListName;

    /** Belle 1 data analysis. */
    bool m_Belle1;

    /** pi0 candidates. */
    StoreObjPtr<ParticleList> m_ListPi0;

    /** Database identifier or file used to load the weights. */
    std::string m_identifier = "LowEnergyPi0Identification";

    /** Database pointer to the database representation of the weightfile. */
    std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>> m_weightfile_representation;

    /** Pointer to the current MVA expert. */
    std::unique_ptr<MVA::Expert> m_expert;

    /** Pointer to the current dataset. */
    std::unique_ptr<MVA::SingleDataset> m_dataset;

  };

}
