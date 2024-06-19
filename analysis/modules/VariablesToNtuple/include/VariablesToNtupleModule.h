/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/RestOfEvent.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/EventExtraInfo.h>
#include <framework/pcore/RootMergeable.h>

#include <TTree.h>
#include <TFile.h>

#include <string>

namespace Belle2 {

  class StringWrapper;

  /** Module to calculate variables specified by the user for a given ParticleList
   *  and save them into a ROOT TTree.
   *  The ntuple is candidate-based, meaning the variables of each candidate are saved in a separate
   *  row of the ntuple
   */
  class VariablesToNtupleModule : public Module {
  public:
    /** Constructor. */
    VariablesToNtupleModule();

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
    float getInverseSamplingRateWeight(const Particle* particle);

    /** Name of particle list with reconstructed particles. */
    std::string m_particleList;
    /** List of variables to save. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector. */
    std::vector<std::string> m_variables;
    /** Name of ROOT file for output. */
    std::string m_fileName;
    /** Name of the TTree. */
    std::string m_treeName;
    /** Suffix to be appended to the output file name. */
    std::string m_fileNameSuffix;
    /** Use float type for floating-point numbers. */
    bool m_useFloat;
    /** Size of TBaskets in the output ROOT file in bytes. */
    int m_basketsize;

    /** ROOT file for output. */
    std::shared_ptr<TFile> m_file{nullptr};
    /** The ROOT TNtuple for output. */
    StoreObjPtr<RootMergeable<TTree>> m_tree;
    // Counter branch addresses (event number, candidate number etc)
    int m_event{ -1};                /**< event number */
    int m_run{ -1};                  /**< run number */
    int m_experiment{ -1};           /**< experiment number */
    int m_production{ -1};           /**< production ID (to distinguish MC samples) */
    int m_candidate{ -1};            /**< candidate counter */
    unsigned int m_ncandidates{0};   /**< total n candidates */

    /** Branch addresses of variables of type float. */
    std::vector<float> m_branchAddressesFloat;

    /** Branch addresses of variables of type double. */
    std::vector<double> m_branchAddressesDouble;

    /** Branch addresses of variables of type int (or bool) */
    std::vector<int> m_branchAddressesInt;
    /** List of pairs of function pointers and respective data type corresponding to given variables. */
    std::vector<std::pair<Variable::Manager::FunctionPtr, Variable::Manager::VariableDataType>> m_functions;

    /** Tuple of variable name and a map of integer values and inverse sampling rate. E.g. (signal, {1: 0, 0:10}) selects all signal candidates and every 10th background candidate. */
    std::tuple<std::string, std::map<int, unsigned int>> m_sampling;
    /** Variable name of sampling variable */
    std::string m_sampling_name;
    /** Inverse sampling rates */
    std::map<int, unsigned int> m_sampling_rates;

    const Variable::Manager::Var* m_sampling_variable{nullptr}; /**< Variable Pointer to target variable */
    std::map<int, unsigned long int> m_sampling_counts; /**< Current number of samples with this value */
    StoreObjPtr<EventMetaData> m_eventMetaData; /**< the event information */

    std::string m_MCDecayString; /**< MC decay string to be filled */
    StoreObjPtr<StringWrapper> m_stringWrapper; /**< string wrapper storing the MCDecayString */

    std::string m_signalSideParticleList;      /**< Name of signal-side particle list  */
    int m_signalSideCandidate{-1};             /**< signal-side candidate counter */
    unsigned int m_nSignalSideCandidates{0};   /**< total n signal-side candidates */
    StoreObjPtr<RestOfEvent> m_roe;            /**< ROE object */

    bool m_storeEventType;  /**< If true, the branch __eventType__ is added */
    StoreObjPtr<EventExtraInfo> m_eventExtraInfo; /**< pointer to EventExtraInfo  */
    std::string m_eventType; /**< EventType to be filled */

  };
} // end namespace Belle2
