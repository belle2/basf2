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

#include <tracking/trackFindingCDC/hough/DynTree.h>
#include <tracking/trackFindingCDC/hough/WithWeightedItems.h>
#include <tracking/trackFindingCDC/hough/SharedMarkPtr.h>

#include <framework/logging/Logger.h>

#include <deque>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type of tree for paritioning the hough space
    template<class Item, class Domain, template <class Node> class ChildStructure>
    using WeightedParititioningDynTree = DynTree< WithWeightedItems<Domain, Item>, ChildStructure>;

    template<class Item, class Domain, template <class Node> class ChildrenStructure>
    class WeightedFastHough {
    private:
      /// Type of the Tree the partitions using markable items the hough space
      typedef WeightedParititioningDynTree<SharedMarkPtr<Item>, Domain, ChildrenStructure> FastHoughTree;

      /// Node type of the tree
      typedef typename FastHoughTree::Node Node;

    public:
      WeightedFastHough(const Domain& houghPlain) : m_tree(houghPlain) {;}

    public:
      /// Take the item set and insert them into the top node of the hough space.
      void seed(std::vector<Item>& items)
      {
        fell();
        Node& topNode = m_tree.getTopNode();
        for (Item& item : items) {
          m_marks.push_back(false);
          bool& markOfItem = m_marks.back();
          Weight weight = HIGHEST_WEIGHT;
          topNode.insert(SharedMarkPtr<Item>(&item, &markOfItem), weight);
        }
      }

      /// Take the item set and insert them into the top node of the hough space.
      void seed(std::vector<Item*>& items)
      {
        fell();
        Node& topNode = m_tree.getTopNode();
        for (Item* item : items) {
          m_marks.push_back(false);
          bool& markOfItem = m_marks.back();
          Weight weight = HIGHEST_WEIGHT;
          topNode.insert(SharedMarkPtr<Item>(item, &markOfItem), weight);
        }
      }

      template<class TreeWalker>
      void walk(TreeWalker& walker)
      {
        auto priority = [](Node * node) -> float {
          /// Clear items that have been marked as used before evaluating the weight.
          auto isMarked = [](const SharedMarkPtr<Item>& item) -> bool {
            return item.isMarked();
          };
          node->eraseIf(isMarked);
          return node->getWeight();
        };

        m_tree.walk(walker, priority);
      }

      template<class ItemInDomainMeasure>
      std::vector<std::pair<Domain,  std::vector<Item*> > >
      findHeavyLeavesDisjoint(ItemInDomainMeasure& weightItemInDomain,
                              const size_t maxLevel,
                              const double minWeight)
      {
        std::vector<std::pair<Domain,  std::vector<Item*> > > found;
        auto skipLowWeightNode = [minWeight](const Node * node) {
          return not(node->getWeight() >= minWeight);
        };
        return findLeavesDisjoint(weightItemInDomain, maxLevel, skipLowWeightNode);
      }

      template<class ItemInDomainMeasure, class SkipNodePredicate>
      std::vector<std::pair<Domain,  std::vector<Item*> > >
      findLeavesDisjoint(ItemInDomainMeasure& weightItemInDomain,
                         const size_t maxLevel,
                         SkipNodePredicate& skipNode)
      {

        std::vector<std::pair<Domain,  std::vector<Item*> > > found;

        auto walker = [&](Node * node) {
          // Node does not have enough items
          // Do not walk children
          if (skipNode(node)) {
            return false;
          }
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
            node->createChildren();
            children = node->getChildren();
            for (Node& childNode : *children) {
              auto measure = [&childNode, &weightItemInDomain](const Item * item) {
                return weightItemInDomain(item, &childNode);
              };
              childNode.insert(*node, measure);
            }
          }
          // Continue to walk the children.
          return true;
        };

        walk(walker);

        return found;
      }


      /// Fell to tree meaning deleting all child nodes from the tree. Keeps the top node.
      void fell()
      {
        m_tree.getTopNode().clear();
        m_marks.clear();
        m_tree.fell();
      }

      /// Like fell but also releases all memory the tree has aquired during long executions.
      void raze()
      {
        fell();
        m_tree.raze();
        m_marks.shrink_to_fit();
      }

    private:
      /// The tree into to insert the items.
      FastHoughTree m_tree;

      /// Memory of the used marks of the items.
      std::deque<bool> m_marks;
      // Note have to use a deque here because std::vector<bool> is special
      // std::vector<bool> m_marks;
    };
  }
}
