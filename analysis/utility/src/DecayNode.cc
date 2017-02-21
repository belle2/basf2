/* BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *                                                                        *
 **************************************************************************/

#include <analysis/utility/DecayNode.h>
#include <iostream>
#include <sstream>
#include <cassert>

namespace Belle2 {

  bool DecayNode::find_decay(const DecayNode& to_find) const
  {
    if (to_find == (*this))
      return true;
    for (const auto& node : daughters) {
      if (node.find_decay(to_find))
        return true;
    }
    return false;
  }

  std::string DecayNode::print_node(unsigned int indent) const
  {
    std::stringstream output;

    for (unsigned int i = 0; i < indent; ++i) {
      output << "    ";
    }

    output << std::to_string(pdg);
    output << std::endl;

    for (const auto& d : daughters) {
      output << d.print_node(indent + 1);
    }

    return output.str();
  }

  bool operator==(const DecayNode& node1, const DecayNode& node2)
  {
    if (node1.pdg == node2.pdg) {
      if (node1.daughters.size() > 0 and node2.daughters.size() > 0) {
        if (node1.daughters.size() != node2.daughters.size())
          return false;
        for (unsigned int i = 0; i < node1.daughters.size(); ++i) {
          if (node1.daughters[i] != node2.daughters[i])
            if (node1.daughters[i] != 0 and node2.daughters[i] != 0)
              return false;
        }
      }
      return true;
    }
    return false;
  }

  bool operator!=(const DecayNode& node1, const DecayNode& node2)
  {
    return not(node1 == node2);
  }

}
