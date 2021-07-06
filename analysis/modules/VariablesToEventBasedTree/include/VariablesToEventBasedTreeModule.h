/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/core/Module.h>
#include <analysis/VariableManager/Manager.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/pcore/RootMergeable.h>

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
    std::shared_ptr<TFile> m_file{nullptr};
    /** The ROOT TNtuple for output. */
    StoreObjPtr<RootMergeable<TTree>> m_tree;
    /** List of function pointers corresponding to given variables. */
    std::vector<Variable::Manager::FunctionPtr> m_functions;
    /** List of function pointers corresponding to given event variables. */
    std::vector<Variable::Manager::FunctionPtr> m_event_functions;
    int m_event{ -1};                /**< event number */
    int m_run{ -1};                  /**< run number */
    int m_experiment{ -1};           /**< experiment number */
    int m_production{ -1};           /**< production ID (to distinguish MC samples) */
    unsigned int m_ncandidates{ 0};  /**< number of candidates in this event */
    float m_weight{0.0};             /**< weight of this event */
    /** Values corresponding to given variables. */
    std::vector<std::vector<double>> m_values;
    /** Values corresponding to given event variables. */
    std::vector<double> m_event_values;

    /** Tuple of variable name and a map of integer values and inverse sampling rate. E.g. (signal, {1: 0, 0:10}) selects all signal candidates and every 10th background candidate. */
    std::tuple<std::string, std::map<int, unsigned int>> m_sampling;
    /** Variable name of sampling variable */
    std::string m_sampling_name;
    /** Inverse sampling rates */
    std::map<int, unsigned int> m_sampling_rates;

    const Variable::Manager::Var* m_sampling_variable{nullptr}; /**< Variable Pointer to target variable */
    std::map<int, unsigned long int> m_sampling_counts; /**< Current number of samples with this value */

    /** event metadata (get event number etc) */
    StoreObjPtr<EventMetaData> m_eventMetaData;

  };
} // end namespace Belle2
