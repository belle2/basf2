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

#include <memory>
#include <vector>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type of tree for paritioning the hough space
    template<class T, class Domain, class DomainDivsion>
    class  WeightedParititioningDynTree :
      public DynTree<  WithWeightedItems<Domain, T>, DomainDivsion> {
    private:
      /// Type of the base class
      using Super = DynTree<  WithWeightedItems<Domain, T>, DomainDivsion>;

    public:
      /// Constructor attaching a vector of the weigthed items to the top most node domain.
      WeightedParititioningDynTree(Domain topDomain, DomainDivsion domainDivsion) :
        Super(WithWeightedItems<Domain, T>(std::move(topDomain)), std::move(domainDivsion))
      {}
    };

    /** Dynamic tree structure with weighted items in each node which are markable through out the tree.
     *  Used to build fast hough type algorithms, where objects are allowed to carry weights relative to
     *  the hough space part (here called a Domain) they are contained in.
     *  The shared marks allow for interrative extraction of hough peaks such that other areas of the
     *  hough space notice that certain element have already been consumed.
     */
    template<class T, class Domain, class DomainDivsion>
    class WeightedFastHoughTree :
      public WeightedParititioningDynTree<WithSharedMark<T>, Domain, DomainDivsion> {

    private:
      /// Type of the Tree the partitions using markable items the hough space
      using Super = WeightedParititioningDynTree<WithSharedMark<T>, Domain, DomainDivsion>;

    public:
      /// Inheriting the constructor from the base class.
      using WeightedParititioningDynTree<WithSharedMark<T>, Domain, DomainDivsion>::WeightedParititioningDynTree;

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
        auto isLeaf = [&](Node * node) {
          // Skip the expansion and the filling of the children
          if (skipNode(node)) {
            return true;
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
            return true;
          }

          // Else to node has enough weight and is not at the lowest level
          // Signal that it is not a leaf
          // Continue to create and fill children.
          return false;
        };
        fillWalk(weightItemInDomain, isLeaf);
        return found;
      }


      template<class ItemInDomainMeasure>
      std::vector<std::pair<Domain,  std::vector<T> > >
      findHeaviestLeafRepeated(ItemInDomainMeasure& weightItemInDomain,
                               const size_t maxLevel,
                               const Weight minWeight = NAN)
      {
        auto skipLowWeightNode = [minWeight](const Node * node) {
          return not(node->getWeight() >= minWeight);
        };
        return findHeaviestLeafRepeated(weightItemInDomain, maxLevel, skipLowWeightNode);
      }

      template<class ItemInDomainMeasure, class SkipNodePredicate>
      std::vector<std::pair<Domain,  std::vector<T> > >
      findHeaviestLeafRepeated(ItemInDomainMeasure& weightItemInDomain,
                               const size_t maxLevel,
                               SkipNodePredicate& skipNode)
      {
        std::vector<std::pair<Domain,  std::vector<T> > > found;
        Node* node = findHeaviestLeaf(weightItemInDomain, maxLevel, skipNode);
        while (node) {
          const Domain* domain = node;
          found.emplace_back(*domain, std::vector<T>(node->begin(), node->end()));
          for (WithSharedMark<T>& markableItem : *node) {
            markableItem.mark();
          }
          node = findHeaviestLeaf(weightItemInDomain, maxLevel, skipNode);
        }
        return found;
      }

      template<class ItemInDomainMeasure, class SkipNodePredicate>
      std::unique_ptr<std::pair<Domain,  std::vector<T> > >
      findHeaviestLeafSingle(ItemInDomainMeasure& weightItemInDomain,
                             const size_t maxLevel,
                             SkipNodePredicate& skipNode)
      {
        using Result = std::pair<Domain,  std::vector<T> >;
        std::unique_ptr<Result> found = nullptr;
        Node* node = findHeaviestLeaf(weightItemInDomain, maxLevel, skipNode);
        if (node) {
          const Domain* domain = node;
          found.reset(new Result(*domain, std::vector<T>(node->begin(), node->end())));
          for (WithSharedMark<T>& markableItem : *node) {
            markableItem.mark();
          }
        }
        return found;
      }


      template<class ItemInDomainMeasure, class SkipNodePredicate>
      Node* findHeaviestLeaf(ItemInDomainMeasure& weightItemInDomain,
                             const size_t maxLevel,
                             SkipNodePredicate& skipNode)
      {
        Node* heaviestNode = nullptr;
        Weight heighestWeigth = NAN;
        auto isLeaf = [&heaviestNode, &heighestWeigth, maxLevel, &skipNode](Node * node) {
          // Skip the expansion and the filling of the children
          if (skipNode(node)) {
            return true;
          }

          Weight nodeWeight = node->getWeight();
          // Skip the expansion and filling of the children if the node has not enough weight
          if (not std::isnan(heighestWeigth) and not(nodeWeight > heighestWeigth)) {
            return true;
          }

          // Node is a leaf at the maximum level and is heavier than everything seen before.
          // Save its content
          // Do not walk children
          if (node->getLevel() >= maxLevel) {
            heaviestNode = node;
            heighestWeigth = nodeWeight;
            return true;
          }
          return false;
        };
        fillWalk(weightItemInDomain, isLeaf);
        return heaviestNode;
      }

    public:
      template<class ItemInDomainMeasure, class IsLeafPredicate>
      void fillWalk(ItemInDomainMeasure& weightItemInDomain,
                    IsLeafPredicate& isLeaf)
      {
        auto walker = [&weightItemInDomain, &isLeaf](Node * node) {
          // Check if node is a leaf
          // Do not create children in this case
          if (isLeaf(node)) {
            // Do not walk children.
            return false;
          }

          // Node is not a leaf.
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

              [&childNode, &weightItemInDomain](WithSharedMark<T>& item) -> Weight {
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
