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

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/ParticleList.h>

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
  class MCDecayHistMakerModule : public HistModule {

  public:

    /**
     * Constructor
     */
    MCDecayHistMakerModule();

    /**
     * Destructor
     */
    virtual ~MCDecayHistMakerModule();

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

    void clearParticleLists();
    void saveCombinationsForSignal();
    void saveAllCombinations();
    bool fillParticleLists(const std::vector<MCParticle*>& mcDaughters);

    int m_pdg;                /**< PDG code of combined particles */
    std::string m_fileName; /**< save histograms in this file. */
    std::string m_channelName; /**< name of the channel which is reconstructed */
    std::string m_histName; /**< name for the histogram. */
    std::vector<std::string> m_inputListNames; /**< input particle list names */
    typedef std::tuple<int, double, double> HistParams; /**< Tuple holding amount of bins, lower and upper boundary of a histogram */
    HistParams m_histParams; /**< nbins, lower and upper boundary of histogram. */

    std::vector<StoreObjPtr<ParticleList>> m_tmpLists; /**< temporary particle lists (contain subsets of contents of input lists, filled from MC truth). */
    TH1F* m_histogramSignal; /**< signal histogram for combined particle. */
    TH1F* m_histogramAll; /**< signal histogram for combined particle. */

  };

} // Belle2 namespace
