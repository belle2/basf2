/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>
#include <string>

namespace Belle2 {
  /**
   * DecayNode describes the decay of a particle identified by its pdg code,
   * into list of daughters.
   */
  class DecayNode {

  public:
    /**
     * Create new Decay node
     * @param _pdg code of the particle
     * @param _daughters of the particle
     */
    DecayNode(int _pdg = 0, const std::vector<DecayNode>& _daughters = {}) : pdg(_pdg), daughters(_daughters) { }

    /**
     * Check if the decay node contains the given decay tree.
     * @param to_find DecayNode object describing the decay
     */
    bool find_decay(const DecayNode& to_find) const;

    /**
     * Output a single node.
     * Used by to_string to convert tree into a string
     * @param indent -ion level
     */
    std::string print_node(unsigned int indent = 0) const;


    int pdg; /**< pdg code of the particle */
    std::vector<DecayNode> daughters; /**< daughter decay nodes */
  };

  /**
   * Compare two Decay Nodes: They are equal if
   * All daughter decay nodes are equal
   * or one of the daughter lists is empty (which means "inclusive")
   *
   * In particular the order of the daughter particles matter!
   *   423 (--> 421 13) is not the same as 423 (--> 13 421)
   *
   * @param node1 first node
   * @param node2 second node
   */
  bool operator==(const DecayNode& node1, const DecayNode& node2);

  /**
   * Not equal: See operator==
   */
  bool operator!=(const DecayNode& node1, const DecayNode& node2);
}
