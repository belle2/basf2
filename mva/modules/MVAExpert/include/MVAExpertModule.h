/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Weightfile.h>
#include <mva/interface/Expert.h>

#include <framework/database/DBObjPtr.h>

#include <analysis/VariableManager/Manager.h>

#include <vector>
#include <string>
#include <memory>

namespace Belle2 {

  class Particle;

  /**
   * This module adds an ExtraInfo to the Particle objects in a given ParticleList.
   * The ExtraInfo is calculated by an MVA method loaded from the database.
   */
  class MVAExpertModule : public Module {
  public:

    /**
     * Constructor
     */
    MVAExpertModule();

    /**
     * Initialize the module.
     */
    virtual void initialize() override;

    /**
     * Called at the beginning of a new run
     */
    virtual void beginRun() override;

    /**
     * Called for each event.
     */
    virtual void event() override;

    /**
     * Called at the end of the event processing.
     */
    virtual void terminate() override
    {
      m_expert.reset();
      m_dataset.reset();
    }

  private:
    /**
     * Calculates expert output for given Particle pointer
     */
    float analyse(Particle*);

    /**
     * Initialize mva expert, dataset and features
     * Called everytime the weightfile in the database changes in begin run
     */
    void init_mva(MVA::Weightfile& weightfile);


  private:

    std::vector<std::string> m_listNames; /**< input particle list names */
    std::string m_identifier; /**< weight-file */
    std::string m_extraInfoName; /**< Name under which the SignalProbability is stored in the extraInfo of the Particle object. */
    double m_signal_fraction_override; /**< Signal Fraction which should be used. < 0 means use signal fraction of training sample */

    std::vector<const Variable::Manager::Var*> m_feature_variables; /**< Pointers to the feature variables */

    std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>
                                                               m_weightfile_representation; /**< Database pointer to the Database representation of the weightfile */
    std::unique_ptr<MVA::Expert> m_expert; /**< Pointer to the current MVA Expert */
    std::unique_ptr<MVA::SingleDataset> m_dataset; /**< Pointer to the current dataset */
  };

} // Belle2 namespace

