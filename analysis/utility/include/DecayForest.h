/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *                                                                        *
 **************************************************************************/

#pragma once
#ifndef ANALYSIS_UTILITY_INCLUDE_DECAYFOREST_H_
#define ANALYSIS_UTILITY_INCLUDE_DECAYFOREST_H_

#include <analysis/utility/DecayTree.h>


#include <vector>
#include <list>
#include <string>
#include <algorithm>


namespace Belle2 {


  class DecayForest {
  public:
    /**
     * Create a DecayForest from a full decaystring outputted by ParticleMCDecayString
     * @param save_memory by storing only the reconstructed and first valid original DecayTree
     */
    DecayForest(const std::string& full_decaystring, bool save_memory = true);

    /**
     * Return nth tree (0 is the reconstructed tree)
     */
    DecayTree& getTree(unsigned int n) { return forest[n]; }

    /**
     * Get reconstructed tree
     */
    DecayTree& getReconstructedTree() { return forest[0]; }

    /**
     * Get first valid original tree
     */
    DecayTree& getOriginalTree() { return forest[m_first_valid_original]; }

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
    std::vector<DecayTree> forest;
    size_t m_first_valid_original;
  };


}


#endif /* ANALYSIS_UTILITY_INCLUDE_DECAYFOREST_H_ */
