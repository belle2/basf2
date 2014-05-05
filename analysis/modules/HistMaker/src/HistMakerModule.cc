/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/HistMaker/HistMakerModule.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <TGraph.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TFile.h>

#include <boost/algorithm/string.hpp>

#include <fstream>


namespace Belle2 {


  REG_MODULE(HistMaker)


  HistMakerModule::HistMakerModule() : m_truth(nullptr)
  {
    setDescription("Writes out signal and background distributions for given variables and given reconstructed ParticleLists.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    std::vector<std::string> defaultList;
    addParam("listNames", m_listNames, "Input particle list names as list", defaultList);
    addParam("file", m_identifierName, "Name of the TFile where the histogramms are saved.");
    std::vector< std::tuple<std::string, int, double, double> > defaultVariables = { std::make_tuple("prodChildProb", 10, 0, 1), std::make_tuple("M", 100, 0, 100)};
    addParam("histVariables", m_hist_variables, "Variable for which the signal and background distributions are determined as list of tuples of the form (name, amount, a, b)", defaultVariables);
    addParam("truthVariable", m_truth_variable, "Variable which determines if given Particle is considered as signal (> 0.5) or background (< 0.5) default is truth", std::string("truth"));
    addParam("make2dHists", m_make2dHists, "Create a 2D Hist for every pair of the given variables.", false);

  }

  HistMakerModule::~HistMakerModule()
  {
  }

  void HistMakerModule::initialize()
  {

    setFilename(m_identifierName);

    VariableManager& manager = VariableManager::Instance();

    std::vector< Range> hist_variables;
    for (auto & var : m_hist_variables) {

      const VariableManager::Var* var_pointer  = manager.getVariable(std::get<0>(var));
      if (var_pointer == nullptr) {
        B2WARNING("Couldn't find variable " << std::get<0>(var) << " via the VariableManager. Check the name!")
        continue;
      }
      m_variables.push_back(var_pointer);
      m_values.push_back(0.0);
      hist_variables.push_back(var);
    }
    m_hist_variables = hist_variables;

    m_truth  = manager.getVariable(m_truth_variable);
    if (m_truth == nullptr) {
      B2ERROR("Couldn't find truth variable " << m_truth_variable << " via the VariableManager. Check the name!")
    }

    for (auto & name : m_listNames) {
      StoreObjPtr<ParticleList>::required(name);

      m_hists[name] = std::vector< SBHists >();
      for (auto & var : m_hist_variables) {
        int N; double a, b;
        std::tie(std::ignore, N, a, b) = var;
        TH1F* signal     = new TH1F((name + "_" + std::get<0>(var) + "_signal_histogram").c_str(), "Signal Histogram", N, a, b);
        TH1F* background = new TH1F((name + "_" + std::get<0>(var) + "_background_histogram").c_str(), "Background Histogram", N, a, b);
        m_hists[name].push_back(std::make_tuple(signal, background));
      }
    }

    if (m_make2dHists) {
      for (auto & name : m_listNames) {
        m_hists2d[name] = std::vector< SBHists >();
        for (auto & var1 : m_hist_variables) {
          int N1; double a1, b1;
          std::tie(std::ignore, N1, a1, b1) = var1;
          for (auto & var2 : m_hist_variables) {
            if (var1 == var2)
              break;
            int N2; double a2, b2;
            std::tie(std::ignore, N2, a2, b2) = var2;
            TH2F* signal     = new TH2F((name + "_" + std::get<0>(var1) + ":" + std::get<0>(var2) + "_signal_histogram").c_str(), "Signal Histogram", N1, a1, b1, N2, a2, b2);
            TH2F* background = new TH2F((name + "_" + std::get<0>(var1) + ":" + std::get<0>(var2) + "_background_histogram").c_str(), "Background Histogram", N1, a1, b1, N2, a2, b2);
            m_hists2d[name].push_back(std::make_tuple(signal, background));
          }
        }
      }
    }

  }


  void HistMakerModule::beginRun()
  {
  }

  void HistMakerModule::endRun()
  {
  }

  void HistMakerModule::writeHists()
  {

    for (auto & pair : m_hists) {
      for (auto & hist : pair.second) {
        std::get<0>(hist)->Write();
        std::get<1>(hist)->Write();
      }
    }

    for (auto & pair : m_hists2d) {
      for (auto & hist : pair.second) {
        std::get<0>(hist)->Write();
        std::get<1>(hist)->Write();
      }
    }

  }

  void HistMakerModule::terminate()
  {

    saveHists();

    for (auto & pair : m_hists) {
      for (auto & hist : pair.second) {
        delete std::get<0>(hist);
        delete std::get<1>(hist);
      }
    }

    for (auto & pair : m_hists2d) {
      for (auto & hist : pair.second) {
        delete std::get<0>(hist);
        delete std::get<1>(hist);
      }
    }

  }

  void HistMakerModule::event()
  {

    for (auto & name : m_listNames) {
      StoreObjPtr<ParticleList> list(name);

      for (unsigned i = 0; i < list->getListSize(); ++i) {
        const Particle* particle = list->getParticle(i);
        float truth = m_truth->function(particle);

        for (unsigned int i = 0; i < m_variables.size(); ++i) {
          m_values[i] = m_variables[i]->function(particle);
        }

        int q = 0;
        for (auto & hist : m_hists[name]) {
          if (truth > 0.5)
            std::get<0>(hist)->Fill(m_values[q++]);
          else
            std::get<1>(hist)->Fill(m_values[q++]);
        }

        if (m_make2dHists) {
          int x  = 0;
          for (unsigned int j = 0; j < m_values.size(); ++j) {
            for (unsigned int k = 0; k < j; ++k) {
              if (truth > 0.5)
                std::get<0>(m_hists2d[name][x++])->Fill(m_values[j], m_values[k]);
              else
                std::get<1>(m_hists2d[name][x++])->Fill(m_values[j], m_values[k]);
            }
          }
        }
      }
    }
  }

  void HistMakerModule::printModuleParams() const
  {
  }

} // Belle2 namespace

