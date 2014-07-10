#pragma once
/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Pulvermacher                                   *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <framework/core/Module.h>
#include <analysis/VariableManager/Manager.h>

#include <TNtuple.h>
#include <TFile.h>

#include <string>

namespace Belle2 {

  /** Module to calculate variables specified by the user for a given ParticleList
   *  and save them into an Ntuple.
   */
  class VariablesToNtupleModule : public Module {
  public:
    /** Constructor. */
    VariablesToNtupleModule();
    /** Destructor. */
    ~VariablesToNtupleModule() {}
    /** Initialises the module.
     *
     * Prepares variables and sets up ntuple columns.
     */
    void initialize();
    /** Method called for each event. */
    void event();
    /** Write TTree to file, and close file if necessary. */
    void terminate();
  private:
    /** Name of particle list with reconstructed particles. */
    std::string m_particleList;
    /** List of variables to save. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector. */
    std::vector<std::string> m_variables;
    /** Name of ROOT file for output. */
    std::string m_fileName;
    /** Name of the TTree. */
    std::string m_treeName;

    /** ROOT file for output. */
    TFile* m_file;
    /** The ROOT TNtuple for output. */
    TNtuple* m_tree;
    /** List of function pointers corresponding to given variables. */
    std::vector<Variable::Manager::FunctionPtr> m_functions;

  };
} // end namespace Belle2
