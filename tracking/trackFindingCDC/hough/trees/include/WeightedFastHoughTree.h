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

#include <tracking/trackFindingCDC/hough/trees/DynTree.h>
#include <tracking/trackFindingCDC/hough/baseelements/WithWeightedItems.h>
#include <tracking/trackFindingCDC/hough/baseelements/WithSharedMark.h>

#include <vector>
#include <memory>
#include <cassert>
#include <cfloat>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Type of tree for paritioning the hough space
    template<class T, class ADomain, class ADomainDivsion>
    class  WeightedParititioningDynTree :
      public DynTree<  WithWeightedItems<ADomain, T>, ADomainDivsion> {
    private:
      /// Type of the base class
      using Super = DynTree<  WithWeightedItems<ADomain, T>, ADomainDivsion>;

    public:
      /// Constructor attaching a vector of the weigthed items to the top most node domain.
      WeightedParititioningDynTree(ADomain topDomain, ADomainDivsion domainDivsion) :
        Super(WithWeightedItems<ADomain, T>(std::move(topDomain)), std::move(domainDivsion))
      {}
    };

    /** Dynamic tree structure with weighted items in each node which are markable through out the tree.
     *  Used to build fast hough type algorithms, where objects are allowed to carry weights relative to
     *  the hough space part (here called a ADomain) they are contained in.
     *  The shared marks allow for interrative extraction of hough peaks such that other areas of the
     *  hough space notice that certain element have already been consumed.
     */
    template<class T, class ADomain, class ADomainDivsion>
    class WeightedFastHoughTree :
      public WeightedParititioningDynTree<WithSharedMark<T>, ADomain, ADomainDivsion> {

    private:
      /// Type of the Tree the partitions using markable items the hough space
      using Super = WeightedParititioningDynTree<WithSharedMark<T>, ADomain, ADomainDivsion>;

    public:
      /// Inheriting the constructor from the base class.
      using WeightedParititioningDynTree<WithSharedMark<T>, ADomain, ADomainDivsion>::WeightedParititioningDynTree;

      /// Type of the node in the tree.
      using Node = typename Super::Node;

    public:
      /// Take the item set and insert them into the top node of the hough space.
      template<class Ts>
      void seed(const Ts& items)
      {
        this->fell();
        Node& topNode = this->getTopNode();
        for (auto && item : items) {
          m_marks.push_back(false);
          bool& markOfItem = m_marks.back();
          Weight weight = DBL_MAX;
          topNode.insert(WithSharedMark<T>(T(item), &markOfItem), weight);
        }
      }

      /// Find all children node at maximum level and add them to the result list. Skip nodes if their weight is below minWeight.
      template <class AItemInDomainMeasure>
      std::vector<std::pair<ADomain, std::vector<T>>>
      findHeavyLeavesDisjoint(AItemInDomainMeasure& weightItemInDomain,
                              int maxLevel,
                              double minWeight)
      {
        auto skipLowWeightNode = [minWeight](const Node * node) {
          return not(node->getWeight() >= minWeight);
        };
        return findLeavesDisjoint(weightItemInDomain, maxLevel, skipLowWeightNode);
      }

      /// Find all children node at maximum level and add them to the result list. Skip nodes if skipNode returns true.
      template <class AItemInDomainMeasure, class ASkipNodePredicate>
      std::vector<std::pair<ADomain, std::vector<T>>>
      findLeavesDisjoint(AItemInDomainMeasure& weightItemInDomain,
                         int maxLevel,
                         ASkipNodePredicate& skipNode)
      {
        std::vector<std::pair<ADomain,  std::vector<T> > > found;
        auto isLeaf = [&found, &skipNode, maxLevel](Node * node) {
          // Skip the expansion and the filling of the children
          if (skipNode(node)) {
            return true;
          }

          // Node is a leaf at the maximum level
          // Save its content
          // Do not walk children
          if (node->getLevel() >= maxLevel) {
            const ADomain* domain = node;
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

      /**
       * Go through all children until maxLevel is reached and find the heaviest leaves.
       *
       * For this, the single heaviest leaf is found and added to an internal list.
       * The process is repeated until no leaf can be found anymore.
       * A node is skipped if the weight is below minWeight.
       */
      template <class AItemInDomainMeasure>
      std::vector<std::pair<ADomain, std::vector<T>>>
      findHeaviestLeafRepeated(AItemInDomainMeasure& weightItemInDomain,
                               int maxLevel,
                               const Weight minWeight = NAN)
      {
        auto skipLowWeightNode = [minWeight](const Node * node) {
          return not(node->getWeight() >= minWeight);
        };
        return findHeaviestLeafRepeated(weightItemInDomain, maxLevel, skipLowWeightNode);
      }

      /**
       * Go through all children until maxLevel is reached and find the heaviest leaves.
       *
       * For this, the single heaviest leaf is found and added to an internal list.
       * The process is repeated until no leaf can be found anymore.
       * A node is skipped if skipNode is returns true for this node.
       */
      template <class AItemInDomainMeasure, class ASkipNodePredicate>
      std::vector<std::pair<ADomain, std::vector<T>>>
      findHeaviestLeafRepeated(AItemInDomainMeasure& weightItemInDomain,
                               int maxLevel,
                               ASkipNodePredicate& skipNode)
      {
        std::vector<std::pair<ADomain,  std::vector<T> > > found;
        Node* node = findHeaviestLeaf(weightItemInDomain, maxLevel, skipNode);
        while (node) {
          const ADomain* domain = node;
          found.emplace_back(*domain, std::vector<T>(node->begin(), node->end()));
          for (WithSharedMark<T>& markableItem : *node) {
            markableItem.mark();
          }
          node = findHeaviestLeaf(weightItemInDomain, maxLevel, skipNode);
        }
        return found;
      }

      /**
       * Go through all children until the maxLevel is reached and find the leaf with the highest weight.
       * If no node could be found, return an empty list, otherwise return a list with just on element.
       * A node is skipped if skipNode is returns true for this node.
       */
      template <class AItemInDomainMeasure, class ASkipNodePredicate>
      std::unique_ptr<std::pair<ADomain, std::vector<T>>>
      findHeaviestLeafSingle(AItemInDomainMeasure& weightItemInDomain,
                             int maxLevel,
                             ASkipNodePredicate& skipNode)
      {
        using Result = std::pair<ADomain,  std::vector<T> >;
        std::unique_ptr<Result> found = nullptr;
        Node* node = findHeaviestLeaf(weightItemInDomain, maxLevel, skipNode);
        if (node) {
          const ADomain* domain = node;
          found.reset(new Result(*domain, std::vector<T>(node->begin(), node->end())));
          for (WithSharedMark<T>& markableItem : *node) {
            markableItem.mark();
          }
        }
        return found;
      }

      /**
       * Go through all children until the maxLevel is reached and find the leaf with the highest weight.
       * If no node could be found, return a nullptr.
       * A node is skipped if skipNode is returns true for this node.
       */
      template <class AItemInDomainMeasure, class ASkipNodePredicate>
      Node* findHeaviestLeaf(AItemInDomainMeasure& weightItemInDomain,
                             int maxLevel,
                             ASkipNodePredicate& skipNode)
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
      /**
       * Walk through the children and fill them if necessary until isLeaf returns true.
       * Uses the weightItemInDomain to create weights for the items (or decide if an item belongs to a mode or not).
       */
      template<class AItemInDomainMeasure, class AIsLeafPredicate>
      void fillWalk(AItemInDomainMeasure& weightItemInDomain,
                    AIsLeafPredicate& isLeaf)
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
              [&childNode, &weightItemInDomain](WithSharedMark<T>& markableItem) -> Weight {
                // Weighting function should not see the mark, but only the item itself.
                T & item(markableItem);
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
      template<class ATreeWalker>
      void walkHeighWeightFirst(ATreeWalker& walker)
      {
        auto priority = [](Node * node) -> float {
          /// Clear items that have been marked as used before evaluating the weight.
          auto isMarked = [](const WithSharedMark<T>& markableItem) -> bool {
            return markableItem.isMarked();
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
        this->fell();
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
