/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2019 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Marco Milesi (marco.milesi@unimelb.edu.au)               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/DBObjPtr.h>

// MVA
#include <mva/interface/Expert.h>
#include <mva/interface/Interface.h>
#include <mva/interface/Weightfile.h>
#include <mva/interface/Dataset.h>

// ANALYSIS
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ParticleExtraInfoMap.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dbobjects/ChargedPidMVAWeights.h>

//C++
#include <unordered_map>


namespace Belle2 {

  /**
   *
   * This module evaluates the response of an MVA trained for charged particle identification between two hypotheses, S and B.
   *
   * It takes a set of Particle objects in a standard charged particle's ParticleList, calculates the MVA score
   * using the appropriate xml weight file for a given input set of (S,B) mass hypotheses,
   * and adds it as ExtraInfo to the Particle objects.
   *
   */
  class ChargedPidMVAModule : public Module {

    typedef std::vector<std::unique_ptr<MVA::Expert> > ExpertsList; /**< Typedef */
    typedef std::vector<std::unique_ptr<MVA::SingleDataset> > DatasetsList; /**< Typedef */
    typedef std::vector< std::vector<const Variable::Manager::Var*> > VariablesList; /**< Typedef */

    typedef std::unordered_map<int, ExpertsList > ExpertsByParticle; /**< Typedef */
    typedef std::unordered_map<int, DatasetsList > DatasetsByParticle; /**< Typedef */
    typedef std::unordered_map<int, VariablesList > VariablesListsByParticle; /**< Typedef */

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

    /**
     * Called once when a run ends.
     *
     * Use this method to save run information, which you aggregated over the last run.
     */
    virtual void endRun() override;

    /**
     * Clean up anything you created in initialize().
     */
    virtual void terminate() override;

  private:

    /**
     * 1. Check if a payload is found in the database.
     * 2. Check the MVA weights for consistency every time they change in the database.
     * 3. Load MVA weight files, and set MVA::Expert and MVA::SingleDataset objects for each file.
    */
    void initializeMVA();

  private:

    /**
     * The signal mass hypothesis (unsigned) pdgId.
     */
    int m_sig_pdg;

    /**
     * The background mass hypothesis (unsigned) pdgId.
     */
    int m_bkg_pdg;

    /**
     * The lookup name of the MVA score variable, given the input S, B mass hypotheses for the algorithm.
     */
    std::string m_score_varname;

    /**
     * The pdgIds and names of the standard charged particle lists.
     * This is not supposed to change at any time (?).
     */
    const std::unordered_map<int, std::string> m_charged_particle_lists = {
      { 11, "e-:electrons" },
      { -11, "e+:electrons" },
      { 13, "mu-:muons" },
      { -13, "mu+:muons" },
      { 211, "pi+:pions" },
      { -211, "pi-:pions" },
      { 321, "K+:kaons" },
      { -321, "K-:kaons" },
      { 2212, "p+:protons" },
      { -2212, "p-:protons" },
      { 1000010020, "d+:deuterons" },
      { -1000010020, "d-:deuterons" }
    };

    /**
     * The event information. Used for debugging purposes.
     */
    StoreObjPtr<EventMetaData> m_event_metadata;

    /**
     * Interface to get the database payload w/ the MVA weight files etc.
     * The payload class has a method to retrieve the correct weightfile representation
     * given a particle (pdgId, clusterTheta, p).
     */
    DBObjPtr<ChargedPidMVAWeights> m_weightfiles_representation;

    /**
     * This map contains - for each charged particle mass hypothesis' pdgId - a list of MVA::Expert objects.
     * One Expert to be stored for each xml file found in the database.
     */
    ExpertsByParticle m_experts;

    /**
     * This map contains - for each charged particle mass hypothesis' pdgId - a list of MVA::SingleDataset objects.
     * One DS to be stored for each xml file found in the database.
     */
    DatasetsByParticle m_datasets;

    /**
     * This map contains - for each charged particle mass hypothesis' pdgId - a list of lists of feature variables.
     * One list of lists to be stored for each xml file found in the database.
     */
    VariablesListsByParticle m_variables;

    /**
     * This map contains - for each charged particle mass hypothesis' pdgId - a list of lists of spectator variables.
     * One list of lists to be stored for each xml file found in the database.
     */
    VariablesListsByParticle m_spectators;

  };
}

