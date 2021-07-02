/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/DBObjPtr.h>

// MVA
#include <mva/interface/Expert.h>
#include <mva/interface/Dataset.h>

// ANALYSIS
#include <analysis/VariableManager/Manager.h>
#include <analysis/dbobjects/ChargedPidMVAWeights.h>

namespace Belle2 {

  /**
   *
   * This module evaluates the response of an MVA trained for binary charged particle identification between two hypotheses, S and B.
   *
   * For a given input set of (S,B) mass hypotheses, it takes the Particle objects in the appropriate charged stable particle's ParticleLists,
   * calculates the MVA score using the appropriate xml weight file,
   * and adds it as ExtraInfo to the Particle objects.
   *
   */
  class ChargedPidMVAModule : public Module {

    typedef std::vector<std::unique_ptr<MVA::Expert> > ExpertsList; /**< Typedef */
    typedef std::vector<std::unique_ptr<MVA::SingleDataset> > DatasetsList; /**< Typedef */
    typedef std::vector< std::vector<const Variable::Manager::Var*> > VariablesLists; /**< Typedef */

  public:

    /**
     * Constructor, for setting module description and parameters.
     */
    ChargedPidMVAModule();

    /**
     * Destructor, use this to clean up anything you created in the constructor.
     */
    virtual ~ChargedPidMVAModule();

    /**
     * Use this to initialize resources or memory your module needs.
     *
     * Also register any outputs of your module (StoreArrays, StoreObjPtrs, relations) here,
     * see the respective class documentation for details.
     */
    virtual void initialize() override;

    /**
     * Called once before a new run begins.
     *
     * This method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun() override;

    /**
     * Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event() override;

  private:

    /**
     * 1. Check if a payload is found in the database.
     * 2. Check the MVA weights for consistency every time they change in the database.
     * 3. Load MVA weight files for the given signal hypothesis, and set MVA::Expert and MVA::SingleDataset objects for each file found.
    */
    void initializeMVA();

  private:

    /**
     * The input signal mass hypothesis' pdgId.
     */
    int m_sig_pdg;

    /**
     * The input background mass hypothesis' pdgId.
     */
    int m_bkg_pdg;

    /**
     * The input list of names of ParticleList objects to which MVA weights will be applied.
     */
    std::vector<std::string> m_particle_lists;

    /**
     * The name of the database payload object with the MVA weights.
     */
    std::string m_payload_name;

    /**
     * Flag to specify if we use a charge-independent training.
     */
    bool m_charge_independent;

    /**
     * Flag to specify if we use an ECL-only based training.
     */
    bool m_ecl_only;

    /**
     * The lookup name of the MVA score variable, given the input S, B mass hypotheses for the algorithm.
     */
    std::string m_score_varname;

    /**
     * The event information. Used for debugging purposes.
     */
    StoreObjPtr<EventMetaData> m_event_metadata;

    /**
     * Interface to get the database payload with the MVA weight files.
     * The payload class has a method to retrieve the correct weightfile representation
     * given a reconstructed particle's (clusterTheta, p).
     */
    std::unique_ptr<DBObjPtr<ChargedPidMVAWeights>> m_weightfiles_representation;

    /**
     * List of MVA::Expert objects.
     * One Expert to be stored for each xml file found in the database for the given signal mass hypothesis.
     */
    ExpertsList m_experts;

    /**
     * List of MVA::SingleDataset objects.
     * One DS to be stored for each xml file found in the database for the given signal mass hypothesis.
     */
    DatasetsList m_datasets;

    /**
     * List of lists of feature variables.
     * One list of lists to be stored for each xml file found in the database for the given signal mass hypothesis.
     */
    VariablesLists m_variables;

    /**
     * List of lists of spectator variables.
     * One list of lists to be stored for each xml file found in the database for the given signal mass hypothesis.
     */
    VariablesLists m_spectators;

  };
}
