/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/utility/DecayTree.h>

#include <vector>
#include <string>

namespace Belle2 {
  /**
   * Contains several DecayTree objects, which belong all to the same candidate
   */
  class DecayForest {
  public:
    /**
     * Create a DecayForest from a full decaystring outputted by ParticleMCDecayString
     * @param full_decaystring full decay string
     * @param save_memory by storing only the reconstructed and first valid original DecayTree
     * @param removeRadiativeGammaFlag remove radiative photons from decay string. Handle with care:
     * In the decay B+ --> e+ nu_e gamma, the gamma would be removed although this might be your signal.
     */
    explicit DecayForest(const std::string& full_decaystring, bool save_memory = true, bool removeRadiativeGammaFlag = false);

    /**
     * Return nth tree (0 is the reconstructed tree)
     */
    const DecayTree& getTree(unsigned int n) const { return forest.at(n); }

    /**
     * Get reconstructed tree
     */
    const DecayTree& getReconstructedTree() const { return forest.at(0); }

    /**
     * Get first valid original tree
     */
    const DecayTree& getOriginalTree() const { return forest.at(m_first_valid_original); }

    /**
     * Convert DecayHashes outputted by ParticleMCDecayString module to an integer
     */
    static int decayHashFloatToInt(float decayHash, float decayHashExtended);

    /**
     * Return number of trees
     */
    unsigned int getNumberOfTrees() const { return forest.size(); }

    /**
     * Return first valid original tree number
     */
    unsigned int getOriginalTreeNumber() const { return m_first_valid_original; }

  private:
    std::vector<DecayTree> forest; /**< vector of DecayTrees */
    size_t m_first_valid_original; /**< The first valid DecayTree */
  };
}
