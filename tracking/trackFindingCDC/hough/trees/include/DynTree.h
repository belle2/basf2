/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth, Oliver Frost                *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <framework/logging/Logger.h>
#include <vector>
#include <deque>
#include <map>
#include <functional>
#include <type_traits>

#include <cmath>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * This is the base class for all hough trees. It stores its children trees (each tree has children trees itself to reuse this class)
     * as nodes, and has basic functionality to fill and go through ( = walk) all its children and the chdilren of its children etc.
     */

    template<class AProperties, class ASubPropertiesFactory>
    class DynTree {

      /// Type of this class
      using This = DynTree<AProperties, ASubPropertiesFactory>;

      /// Type of the Properties
      using Properties = AProperties;

      /// Type of the factory for the sub node properties
      using SubPropertiesFactory = ASubPropertiesFactory;

    public:
      /// Class for a node in the tree
      class Node : public AProperties {

      public:
        /// Type of the tree containing this node.
        using Tree = This;

        /// Allow the tree access to the node constructor to create the top node.
        friend Tree;

        /// Type of the Properties
        using Properties = AProperties;

        /// Type of the container of the children of the node
        using Children = std::vector<Node>;

      private:
        /** Inheriting the constructors of the properties such that we can
         *  construct nodes as property objects.
         *  Only the top level node is constructed this way.
         */
        using AProperties::AProperties;

      public:
        using AProperties::operator=;

      public:
        /** Getter for the children.
         *  Get the dynamically created children.
         *  Returns nullptr if they have not yet been created
         */
        Children* getChildren()
        { return m_children; }

        /** Const getter for the children.
         *  Get the dynamically created children.
         *  Returns nullptr if they have not yet been created
         */
        const Children* getChildren() const
        { return m_children; }

        /// Creates the children of the node.
        void createChildren()
        {
          // ensure the level value fits into unsigned char
          B2ASSERT("DynTree datastructure only supports levels < 255", getLevel() < 255);

          // Call out to the tree, which is providing the memory for the nodes.
          Node* parentNode = this;
          m_children = getTree()->createChildren(parentNode);

          for (Node& child : *m_children) {
            child.m_level = getLevel() + 1;
            child.m_parent = this;
            child.m_tree = getTree();
          }
        }

      private:
        /// Remove to node from the tree hierachy.
        void unlink()
        {
          m_parent = nullptr;
          m_level = 0;
          m_children = nullptr;
          m_tree = nullptr;
        }

      public:
        /** Calls the walker with each node starting with the top node and continues depth first
         *  The walker can signal to skip the children if false is returned.
         */
        template<class AWalker>
        void walk(AWalker& walker)
        {
          static_assert(std::is_assignable<std::function<bool(Node*)>, AWalker>(), "");

          bool walkChildren = walker(this);
          Children* children = getChildren();
          if (children and walkChildren) {
            for (Node& childNode : *children) {
              childNode.walk(walker);
            }
          }
        }

        /** Calls the walker with each node starting with the top node and continues depth first
         *  The walker can signal to skip the children if false is returned.
         *  Additionally this version allows for a priority measure that determines which child is
         *  traversed first.
         */
        template<class AWalker, class APriorityMeasure>
        void walk(AWalker& walker, APriorityMeasure& priority)
        {
          static_assert(std::is_assignable<std::function<bool(Node*)>, AWalker>(), "");
          static_assert(std::is_assignable<std::function<float(Node*)>, APriorityMeasure>(), "");

          bool walkChildren = walker(this);
          Children* children = getChildren();
          if (children and walkChildren) {
            std::vector<std::pair<float, Node*> > prioritisedChildNodes;
            size_t nChildren = children->size();

            prioritisedChildNodes.reserve(nChildren);
            // Priorities the child nodes with the priority function.
            for (Node& childNode : *children) {
              float childPriority = priority(&childNode);
              if (std::isnan(childPriority)) continue;
              prioritisedChildNodes.push_back(std::make_pair(childPriority, &childNode));
            }

            std::make_heap(prioritisedChildNodes.begin(), prioritisedChildNodes.end());

            while (not prioritisedChildNodes.empty()) {
              std::pop_heap(prioritisedChildNodes.begin(), prioritisedChildNodes.end());
              Node* priorityChildNode = prioritisedChildNodes.back().second;
              prioritisedChildNodes.pop_back();

              priorityChildNode->walk(walker, priority);

              // After the walking the children we reevaluate the priority
              bool reheap = false;
              erase_remove_if(prioritisedChildNodes,
              [&reheap, &priority](std::pair<float, Node*>& prioritisedChildNode) {
                float childPriority = priority(prioritisedChildNode.second);
                if (std::isnan(childPriority)) return true;
                // Check if weights changed and a reordering is due.
                reheap |= prioritisedChildNode.first != childPriority;
                prioritisedChildNode.first = childPriority;
                return false;
              });

              if (reheap) {
                std::make_heap(prioritisedChildNodes.begin(), prioritisedChildNodes.end());
              }
            }
            assert(prioritisedChildNodes.empty());
          }
        }

        /// Get the level of the node
        int getLevel() const { return m_level; }

        /** Getter of the node one up the hierarchy.
         *  Usually the highest node of level 0 has no parent.
         *  Return nullptr in this case.
         */
        Node* getParent() { return m_parent; }

        /** Const getter of the node one up the hierarchy.
         *  Usually the highest node of level 0 has no parent.
         *  Return nullptr in this case.
         */
        const Node* getParent() const { return m_parent; }

        /// Getter for the tree containing this node.
        Tree* getTree() const { return m_tree; }

      private:
        /// Reference to the tree that contains this node
        Tree* m_tree = nullptr;

        /// Children that are created only if needed
        Children* m_children = nullptr;

        /// Level of the node within the tree
        int m_level = 0;

        /// Parent in the tree hierachy of this node.
        Node* m_parent = nullptr;
      };

    public:
      /// Constructor taking properties with which the top node of the tree is initialised.
      DynTree(const Properties& properties,
              const SubPropertiesFactory& subPropertiesFactory = SubPropertiesFactory()) :
        m_subPropertiesFactory(subPropertiesFactory),
        m_topNode(properties),
        m_children()
      {
        m_topNode.m_tree = this;
      }

      /// Forbid copy construction
      DynTree(const DynTree& node) = delete;

      /// Forbid copy assignment
      DynTree& operator=(const DynTree&) = delete;

      /// Getter for the top node of the tree
      Node& getTopNode()
      { return m_topNode; }

      /// Constant getter for the top node of the tree
      const Node& getTopNode() const
      { return m_topNode; }

      /**
       *  Gets the number of nodes currently contained in the tree
       *  Also demonstrates how to walk over the tree.
       */
      int getNNodes() const
      {
        int nNodes = 0;
        auto countNodes = [&nNodes](const Node*) -> bool {
          ++nNodes;
          return true;
        };
        const_cast<DynTree&>(*this).walk(countNodes);
        //walk(countNodes);
        return nNodes;
      }

      /**
       *  Gets the number of nodes by level in the tree
       *  Also demonstrates how to walk over the tree.
       */
      std::map<int, int> getNNodesByLevel() const
      {
        std::map<int, int> nNodesByLevel;
        auto countNodes = [&nNodesByLevel](const Node * node) -> bool {
          if (nNodesByLevel.count(node->getLevel()) == 0)
          {
            nNodesByLevel[node->getLevel()] = 1;
          } else {
            nNodesByLevel[node->getLevel()]++;
          }
          return true;
        };
        const_cast<DynTree&>(*this).walk(countNodes);
        //walk(countNodes);
        return nNodesByLevel;
      }

    private:
      /// Create child nodes for the given parents.
      std::vector<Node>* createChildren(Node* parentNode)
      {
        std::vector<Node>* result = getUnusedChildren();
        auto subProperties = m_subPropertiesFactory(*parentNode);
        if (subProperties.empty()) {
          result->clear();
        } else {
          // Initialize new elements with dummy property.
          result->resize(subProperties.size(), Node(subProperties.back()));
          size_t iSubNode = 0;
          for (auto& properties : subProperties) {
            clearIfApplicable(result->at(iSubNode));
            result->at(iSubNode) = properties;
            ++iSubNode;
          }
        }
        return result;
      }

      /// Aquire the next unused child node structure, recycling all memory.
      std::vector<Node>* getUnusedChildren()
      {
        if (m_nUsedChildren >= m_children.size()) {
          m_children.emplace_back();
        }
        ++m_nUsedChildren;
        return &(m_children[m_nUsedChildren - 1]);
      }

    public:
      /// Forward walk to the top node
      template<class AWalker>
      void walk(AWalker& walker)
      {
        static_assert(std::is_assignable<std::function<bool(Node*)>, AWalker>(), "");

        getTopNode().walk(walker);
      }

      /// Forward walk to the top node
      template<class AWalker, class APriorityMeasure>
      void walk(AWalker& walker, APriorityMeasure& priority)
      {
        static_assert(std::is_assignable<std::function<bool(Node*)>, AWalker>(), "");
        static_assert(std::is_assignable<std::function<float(Node*)>, APriorityMeasure>(), "");

        getTopNode().walk(walker, priority);
      }

      /// Fell to tree meaning deleting all child nodes from the tree. Keeps the top node.
      void fell()
      {
        clearIfApplicable(m_topNode);
        m_topNode.unlink();
        m_topNode.m_tree = this;
        for (typename Node::Children& children : m_children) {
          for (Node& node : children) {
            clearIfApplicable(node);
            node.unlink();
          }
        }
        m_nUsedChildren = 0;
      }

      /// Like fell but also releases all memory the tree has aquired during long execution.
      void raze()
      {
        this->fell();
        m_children.clear();
        m_children.shrink_to_fit();
      }

    public:
      /// Instance of the properties factory for the sub nodes
      SubPropertiesFactory m_subPropertiesFactory;

      /// Memory for the top node of the tree
      Node m_topNode;

      /// Central point to provide memory for the child structures
      std::deque<typename Node::Children> m_children;

      /// Last index of used children.
      size_t m_nUsedChildren = 0;
    };
  }
}
