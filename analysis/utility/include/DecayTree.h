/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/utility/DecayNode.h>

#include <vector>
#include <string>

namespace Belle2 {

  /**
   * Consists of a tree of DecayNodes
   * Can be constructed from the output of the ParticleMCDecayString module
   */
  class DecayTree {

  public:
    /**
     * Create a new Decay tree from a decaystring
     */
    explicit DecayTree(const std::string& decaystring, bool removeRadiativeGammaFlag = false);

    /**
     * Copy constructor
    * Required because we need to rebuild the node cache
     */
    DecayTree(const DecayTree& tree);

    /**
     * Assign operator
     * Required because we need to rebuild the node cache
     */
    DecayTree& operator=(const DecayTree& tree);

    /**
     * Recursively build a new tree
     * @param decaystring as outputted by the ParticleMCDecayString module (after splitting by | )
     * @param removeRadiativeGammaFlag remove radiative photons from decay string. Handle with care:
     * In the decay B+ --> e+ nu_e gamma, the gamma would be removed although this might be your signal.
     */
    std::vector<DecayNode> build_tree(const std::string& decaystring, bool removeRadiativeGammaFlag);

    /**
     * Check if the decay tree contains the given decay tree.
     * @param tree DecayTree describing the decay
     */
    bool find_decay(const DecayTree& tree) const;

    /**
     * Output string representation of DecayTree
     */
    std::string to_string() const;

    /**
     * Returns position of match symbol
     */
    int getMatchSymbolPosition() const { return m_match_symbol_position; }

    /**
     * Returns if the decay tree is valid (constructed from a decay string which has a match)
     */
    bool isValid() const { return m_valid; }

    /**
     * Returns n-th node as const
     * @param n nth node
     */
    const DecayNode& getDecayNode(unsigned int n) const;

    /**
     * Returns n-th node
     * @param n nth node
     */
    DecayNode& getDecayNode(unsigned int n);

    /**
     * Return number of nodes in this tree
     */
    size_t getNumberOfDecayNodes() const { return m_nodes_cache.size(); }

  private:
    /**
     * Build nodes_cache in order of appearance in the decay string for fast access
     * @param node current node
     */
    void build_cache(DecayNode& node);

  private:
    bool m_valid; /**< True if the tree is valid (invalid can happen if is constructed from a node with "No match") */
    size_t m_i; /**< Current position in the building of the DecayTree */
    unsigned int m_token_count; /**< Count current tokens */
    int m_match_symbol_position; /**< Position of the token with the match symbol ^ */
    DecayNode m_root_node; /**< root DecayNode */
    std::vector<DecayNode*>
    m_nodes_cache; /**< Vector of decay nodes of the particles in the order of their appearance in the decay string for fast access */
  };
}
