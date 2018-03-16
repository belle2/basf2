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

#ifndef ANALYSIS_UTILITY_INCLUDE_DECAYNODE_H_
#define ANALYSIS_UTILITY_INCLUDE_DECAYNODE_H_

#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <memory>

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


#endif /* ANALYSIS_UTILITY_INCLUDE_DECAYNODE_H_ */
