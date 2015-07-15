/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Oliver Frost                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

namespace Belle2 {
  namespace TrackFindingCDC {

    /// This mimics the original algorithm by Viktor Trusov et al.
    template<class InDomain>
    class MaxLevelLegendreHitProcessor {
      MaxLevelLegendreTreeWalker(size_t maxLevel,
                                 double minWeight,
                                 Box<DiscreteAngle, float> phi0OmegaSearchBox) :
        m_maxLevel(maxLevel),
        m_minWeight(minWeight),
        m_phi0OmegaSearchBox(phi0OmegaSearchBox)
      {
      }

      template<class Node>
      bool skip(Node* node)
      { return (node->getWeight() < m_minWeight or not m_phi0OmegaSearchBox.intersects(*node); }

             template<class Node>
      void createChildren(Node* node)
      {
        node->createChildren();
      }

      template<class Node>
      bool operator()(Node* node)
      {
        // Do not walk children
        if (skip(node)) return false;

        // Node is a leaf at the maximum level
        // Save its content
        // Do not walk children
        if (node->getLevel() >= maxLevel) {
          const Domain* domain = node;
          found.emplace_back(*domain, std::vector<Item*>(node->begin(), node->end()));
          for (SharedMarkPtr<Item>& markableItem : *node) {
            markableItem.mark();
          }
          return false;
        }

        // Node is not a leave.
        // Check if it has childen.
        // If children have not been created, create and fill them.
        typename Node::Children* children = node->getChildren();
        if (not children) {
          createChildren(node);
          children = node->getChildren();
          for (Node& childNode : *children) {
            childNode.insert(*node, weightIn);
          }
        }
        // Continue to walk the children.
        return true;
      }

    private:
      size_t m_maxLevel;
      Weight m_minWeight;
      Box<DiscreteAngle, float> m_phi0OmegaSearchBox;
      std::vector<std::pair<Domain, std::vector<Item*> > > m_foundItemGroups;
    };
  }
}
