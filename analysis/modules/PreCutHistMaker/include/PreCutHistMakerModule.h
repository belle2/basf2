/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RootMergeable.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/ParticleCombiner/ParticleCombiner.h>
#include <analysis/VariableManager/Manager.h>

#include <TFile.h>

#include <string>
#include <vector>
#include <set>

namespace Belle2 {
  class Particle;
  class MCParticle;

  /**
   * Saves distribution of a variable of combined particles (from input ParticleLists) into histogram 'all'. If the daughters can be combined into a correctly reconstructed (!) particle of specified type, save variable value for this combination to a histogram called 'signal'. This is equivalent to running ParticleCombiner on the given lists and saving the variable value of Particles with isSignal == 1 and everything else, but much faster (since Particles don't need to be saved).
  */
  class PreCutHistMakerModule : public Module {

  public:

    /**
     * Constructor
     */
    PreCutHistMakerModule();

    /**
     * Destructor
     */
    virtual ~PreCutHistMakerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();


    virtual void event();

    virtual void terminate();


  private:

    /** reset m_tmpLists. */
    void clearTemporaryLists();

    /** fills m_tmpLists with Particles related to given MCParticles.
     *
     * Returns true if and only if all lists have entries (i.e. there's a chance of reconstructing this decay)
     */
    bool fillParticleLists(const std::vector<MCParticle*>& mcDaughters);
    /** Make Particle combinations using m_tmpLists, check MC truth and save true signals in histogram. */
    void saveCombinationsForSignal();
    /** Make all Particle combinations for given PDG and input lists, save into 'all' histogram. */
    void saveAllCombinations();

    std::string m_decayString; /**< decay string. */
    int m_pdg;                /**< PDG code of combined particles */
    std::string m_fileName; /**< save histograms in this file. */
    TFile* m_file; /**< owns histograms. */
    std::string m_channelName; /**< name of the channel which is reconstructed */
    std::string m_histName; /**< name for the histogram. */
    std::vector<StoreObjPtr<ParticleList>> m_inputLists; /**< input particle lists */
    typedef std::tuple<int, double, double> HistParams; /**< Tuple holding amount of bins, lower and upper boundary of a histogram */
    HistParams m_histParams; /**< nbins, lower and upper boundary of histogram. */
    std::vector<float> m_customBinning; /**< Custom binning */

    std::string m_variable; /**< Variable for which the distributions are calculated */
    const Variable::Manager::Var* m_var; /**< Pointer to variable stored in the variable manager */
    std::vector<StoreObjPtr<ParticleList>> m_tmpLists; /**< temporary particle lists (contain subsets of contents of input lists, filled from MC truth). */
    StoreObjPtr<RootMergeable<TH1F>> m_histogramSignal; /**< signal histogram for combined particle. */
    StoreObjPtr<RootMergeable<TH1F>> m_histogramAll; /**< signal histogram for combined particle. */

    ParticleGenerator* m_generator_signal; /**< Combines particles for signal (not everything is signal, so we run MCMatching for the limited number of candidates produced). */
    ParticleGenerator* m_generator_all; /**< Combines particles for signal+background. */

  };

} // Belle2 namespace
