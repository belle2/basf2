/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Thomas Keck                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/VariableManager/Manager.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RootMergeable.h>

#include <TH1D.h>
#include <TFile.h>

#include <string>
#include <vector>

namespace Belle2 {

  /** Module to calculate variables specified by the user for a given ParticleList
   *  and save them into an Histogram.
   */
  class VariablesToHistogramModule : public Module {
  public:
    /** Constructor. */
    VariablesToHistogramModule();

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
    /** Name of particle list with reconstructed particles. */
    std::string m_particleList;
    /** List of variables to save. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector. */
    std::vector<std::tuple<std::string, int, float, float>> m_variables;
    /** List of pairs of variables to save. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector. */
    std::vector<std::tuple<std::string, int, float, float, std::string, int, float, float>> m_variables_2d;
    /** Name of ROOT file for output. */
    std::string m_fileName;
    /** Name of the Directory. */
    std::string m_directory;

    /** ROOT file for output. */
    std::shared_ptr<TFile> m_file{nullptr};
    /** The ROOT TH1Ds for output. */
    std::vector<std::unique_ptr<StoreObjPtr<RootMergeable<TH1D>>>> m_hists;
    /** The ROOT TH2Ds for output. */
    std::vector<std::unique_ptr<StoreObjPtr<RootMergeable<TH2D>>>> m_2d_hists;
    /** List of function pointers corresponding to given variables. */
    std::vector<Variable::Manager::FunctionPtr> m_functions;
    /** List of function pointers corresponding to given variables. */
    std::vector<Variable::Manager::FunctionPtr> m_functions_2d_1;
    /** List of function pointers corresponding to given variables. */
    std::vector<Variable::Manager::FunctionPtr> m_functions_2d_2;

  };
} // end namespace Belle2
