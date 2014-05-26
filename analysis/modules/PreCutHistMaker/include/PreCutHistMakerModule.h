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

#include <framework/pcore/HistModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/utility/VariableManager.h>

#include <string>
#include <vector>
#include <set>

class TH1F;

namespace Belle2 {
  class Particle;
  class MCParticle;

  /**
   * If the daughters in the given particle lists can be combined into a correctly reconstructed (!) particle of specified PDG code, save invariant mass for this combination to a histogram
   */
  /**
   * Saves distribution of a variable of combined particles (from input ParticleLists) into histogram 'all'. If the daughters in the given particle lists can be combined into a correctly reconstructed (!) particle of specified PDG code, save variable value for this combination to a histogram called 'signal'. This is equivalent to running ParticleCombiner on the given lists and saving the variable value of Particles with isSignal == 1 and everything else, but much faster (since Particles don't need to be saved)
  */
  class PreCutHistMakerModule : public HistModule {

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

    /** implement HistModule interface. */
    virtual void writeHists();


  private:

    /** reset m_tmpLists. */
    void clearParticleLists();

    /** fills m_tmpLists with Particles related to given MCParticles.
     *
     * Returns true if and only if all lists have entries (i.e. there's a chance of reconstructing this decay)
     */
    bool fillParticleLists(const std::vector<MCParticle*>& mcDaughters);
    /** Make Particle combinations using m_tmpLists, check MC truth and save true signals in histogram. */
    void saveCombinationsForSignal();
    /** Make all Particle combinations for given PDG and input lists, save into 'all' histogram. */
    void saveAllCombinations();

    int m_pdg;                /**< PDG code of combined particles */
    std::string m_fileName; /**< save histograms in this file. */
    std::string m_channelName; /**< name of the channel which is reconstructed */
    std::string m_histName; /**< name for the histogram. */
    std::vector<std::string> m_inputListNames; /**< input particle list names */
    typedef std::tuple<int, double, double> HistParams; /**< Tuple holding amount of bins, lower and upper boundary of a histogram */
    HistParams m_histParams; /**< nbins, lower and upper boundary of histogram. */
    std::vector<float> m_customBinning; /**< Custom binning */

    std::string m_variable; /**< Variable for which the distributions are calculated */
    const VariableManager::Var* m_var; /**< Pointer to variable stored in the variable manager */
    std::vector<StoreObjPtr<ParticleList>> m_tmpLists; /**< temporary particle lists (contain subsets of contents of input lists, filled from MC truth). */
    TH1F* m_histogramSignal; /**< signal histogram for combined particle. */
    TH1F* m_histogramAll; /**< signal histogram for combined particle. */

  };

} // Belle2 namespace
