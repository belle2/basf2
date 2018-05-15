#pragma once
/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Keck                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <framework/core/Module.h>
#include <analysis/VariableManager/Manager.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RootMergeable.h>

#include <TNtuple.h>
#include <TFile.h>

#include <string>

namespace Belle2 {

  /** Module to calculate variables specified by the user for a given ParticleList
   *  and save them into a TTree. The Tree is event-based, meaning that the variables of each candidate for each event
   *  are saved in an array in a branch of the Tree.
   */
  class VariablesToEventBasedTreeModule : public Module {
  public:
    /** Constructor. */
    VariablesToEventBasedTreeModule();

    /** Initialises the module.
     *
     * Prepares variables and sets up ntuple columns.
     */
    virtual void initialize() override;
    /** Method called for each event. */
    virtual void event() override;
    /** Write TTree to file, and close file if necessary. */
    virtual void terminate() override;

  private:
    /**
     * Calculate inverse sampling rate weight. Event is skipped if returned weight is 0.
     */
    float getInverseSamplingRateWeight();

    /** Name of particle list with reconstructed particles. */
    std::string m_particleList;
    /** List of variables to save. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector. */
    std::vector<std::string> m_variables;
    /** List of event variables to save. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector. */
    std::vector<std::string> m_event_variables;
    /** maximum number of candidates which is written out */
    unsigned int m_maxCandidates;
    /** Name of ROOT file for output. */
    std::string m_fileName;
    /** Name of the TTree. */
    std::string m_treeName;

    /** ROOT file for output. */
    TFile* m_file;
    /** The ROOT TNtuple for output. */
    StoreObjPtr<RootMergeable<TTree>> m_tree;
    /** List of function pointers corresponding to given variables. */
    std::vector<Variable::Manager::FunctionPtr> m_functions;
    /** List of function pointers corresponding to given event variables. */
    std::vector<Variable::Manager::FunctionPtr> m_event_functions;
    /** number of candidates in this event */
    unsigned int m_ncandidates;
    /** weight of this event */
    float m_weight;
    /** Values corresponding to given variables. */
    std::vector<std::vector<double>> m_values;
    /** Values corresponding to given event variables. */
    std::vector<float> m_event_values;

    /** Tuple of variable name and a map of integer values and inverse sampling rate. E.g. (signal, {1: 0, 0:10}) selects all signal candidates and every 10th background candidate. */
    std::tuple<std::string, std::map<int, unsigned int>> m_sampling;
    /** Variable name of sampling variable */
    std::string m_sampling_name;
    /** Inverse sampling rates */
    std::map<int, unsigned int> m_sampling_rates;

    const Variable::Manager::Var* m_sampling_variable; /**< Variable Pointer to target variable */
    std::map<int, unsigned long int> m_sampling_counts; /**< Current number of samples with this value */

  };
} // end namespace Belle2
