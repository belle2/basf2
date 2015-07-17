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
#include <tracking/trackFindingCDC/hough/WithSharedMark.h>

#include <framework/logging/Logger.h>

#include <deque>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type of tree for paritioning the hough space
    template<class T, class Domain, class DomainDivsion>
    using WeightedParititioningDynTree = DynTree< WithWeightedItems<Domain, T>, DomainDivsion>;

    template<class T, class Domain, class DomainDivsion>
    class WeightedFastHoughTree :
      public WeightedParititioningDynTree<WithSharedMark<T>, Domain, DomainDivsion> {

    private:
      /// Type of the Tree the partitions using markable items the hough space
      using Super = WeightedParititioningDynTree<WithSharedMark<T>, Domain, DomainDivsion>;

    public:
      /// Inheriting the constructor from the base class.
      using Super::Super;

      /// Type of the node in the tree.
      using Node = typename Super::Node;

    public:
      /// Take the item set and insert them into the top node of the hough space.
      template<class Ts>
      void seed(Ts& items)
      {
        fell();
        Node& topNode = this->getTopNode();
        for (const auto& item : items) {
          m_marks.push_back(false);
          bool& markOfItem = m_marks.back();
          Weight weight = HIGHEST_WEIGHT;
          topNode.insert(WithSharedMark<T>(item, &markOfItem), weight);
        }
      }

      /// Walk the tree investigating the heaviest children with priority.
      template<class TreeWalker>
      void walkHeighWeightFirst(TreeWalker& walker)
      {
        auto priority = [](Node * node) -> float {
          /// Clear items that have been marked as used before evaluating the weight.
          auto isMarked = [](const WithSharedMark<T>& item) -> bool {
            return item.isMarked();
          };
          node->eraseIf(isMarked);
          return node->getWeight();
        };

        this->walk(walker, priority);
      }

      template<class ItemInDomainMeasure>
      std::vector<std::pair<Domain,  std::vector<T> > >
      findHeavyLeavesDisjoint(ItemInDomainMeasure& weightItemInDomain,
                              const size_t maxLevel,
                              const double minWeight)
      {
        auto skipLowWeightNode = [minWeight](const Node * node) {
          return not(node->getWeight() >= minWeight);
        };
        return findLeavesDisjoint(weightItemInDomain, maxLevel, skipLowWeightNode);
      }

      template<class ItemInDomainMeasure, class SkipNodePredicate>
      std::vector<std::pair<Domain,  std::vector<T> > >
      findLeavesDisjoint(ItemInDomainMeasure& weightItemInDomain,
                         const size_t maxLevel,
                         SkipNodePredicate& skipNode)
      {
        std::vector<std::pair<Domain,  std::vector<T> > > found;

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
            found.emplace_back(*domain, std::vector<T>(node->begin(), node->end()));
            for (WithSharedMark<T>& markableItem : *node) {
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
              assert(childNode.getChildren() == nullptr);
              assert(childNode.size() == 0);
              auto measure =
              [&childNode, &weightItemInDomain](const WithSharedMark<T>& item) -> Weight {
                // Weighting function should not see the mark, but only the item itself.
                return weightItemInDomain(item, &childNode);
              };
              childNode.insert(*node, measure);
            }
          }
          // Continue to walk the children.
          return true;
        };

        walkHeighWeightFirst(walker);

        return found;
      }


      /// Fell to tree meaning deleting all child nodes from the tree. Keeps the top node.
      void fell()
      {
        this->getTopNode().clear();
        m_marks.clear();
        Super::fell();
      }

      /// Like fell but also releases all memory the tree has aquired during long executions.
      void raze()
      {
        fell();
        Super::raze();
        m_marks.shrink_to_fit();
      }

    private:
      /// Memory of the used marks of the items.
      std::deque<bool> m_marks;
      // Note: Have to use a deque here because std::vector<bool> is special
      // std::vector<bool> m_marks;
    };
  }
}
