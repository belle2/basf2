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

#include <tracking/trackFindingCDC/utilities/CallIfApplicable.h>
#include <framework/logging/Logger.h>
#include <vector>
#include <deque>
#include <algorithm>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class Properties_, class SubPropertiesFactory_>
    class DynTree {

      /// Type of this class
      typedef DynTree<Properties_, SubPropertiesFactory_> This;

      /// Type of the Properties
      typedef Properties_ Properties;

      /// Type of the factory for the sub node properties
      typedef SubPropertiesFactory_ SubPropertiesFactory;

    public:
      /// Class for a node in the tree
      class Node : public Properties_ {

      public:
        /// Type of the tree containing this node.
        typedef This Tree;

        /// Allow the tree access to the node constructor to create the top node.
        friend Tree;

        /// Type of the Properties
        typedef Properties_ Properties;

        /// Type of the container of the children of the node
        typedef std::vector<Node> Children;

      private:
        /** Inheriting the constructors of the properties such that we can
         *  construct nodes as property objects.
         *  Only the top level node is constructed this way.
         */
        using Properties_::Properties_;

      public:
        using Properties_::operator=;

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
        template<class Walker>
        void walk(Walker& walker)
        {
          bool walkChildren = walker(this);
          Children* children = getChildren();
          if (children and walkChildren) {
            for (Node* childNode : children) {
              childNode->walk(walker);
            }
          }
        }

        /** Calls the walker with each node starting with the top node and continues depth first
         *  The walker can signal to skip the children if false is returned.
         *  Additionally this version allows for a priority measure that determines which child is
         *  traversed first.
         */
        template<class Walker, class PriorityMeasure>
        void walk(Walker& walker, PriorityMeasure& priority)
        {
          bool walkChildren = walker(this);
          Children* children = getChildren();
          if (children and walkChildren) {
            std::vector<std::pair<float, Node*> > prioritisedChildNodes;
            size_t nChildren = children->size();

            prioritisedChildNodes.reserve(nChildren);
            // Priorities the child nodes with the priority function.
            for (Node& childNode : *children) {
              float childPriority = priority(&childNode);
              prioritisedChildNodes.push_back(std::make_pair(childPriority, &childNode));
            }

            std::make_heap(prioritisedChildNodes.begin(),
                           prioritisedChildNodes.end());

            // while( not prioritisedChildNode.empty() ) {
            // We know the number of children so we can make the loop counter explicit
            for (std::size_t i = 0; i < nChildren; ++i) {
              std::pop_heap(prioritisedChildNodes.begin(),
                            prioritisedChildNodes.end());

              Node* priorityChildNode = prioritisedChildNodes.back().second;
              prioritisedChildNodes.pop_back();
              priorityChildNode->walk(walker, priority);

              // After the walking the children we reevaluate the priority
              bool reheap = false;
              for (std::pair<float, Node*>& prioritisedChildNode : prioritisedChildNodes) {
                float childPriority = priority(prioritisedChildNode.second);
                /// Check if weights changed and a reordering is due.
                reheap |= prioritisedChildNode.first != childPriority;
                prioritisedChildNode.first = childPriority;
              }
              if (reheap) {
                std::make_heap(prioritisedChildNodes.begin(),
                               prioritisedChildNodes.end());
              }
            }
            assert(prioritisedChildNodes.empty());
          }
        }

        /// Get the level of the node
        std::size_t getLevel() const { return m_level; }

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
        std::size_t m_level = 0;

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

    private:
      /// Create child nodes for the given parents.
      std::vector<Node>* createChildren(Node* parentNode)
      {
        Properties& parentProperties = *parentNode;
        std::vector<Node>* result = getUnusedChildren();

        auto subProperties = m_subPropertiesFactory(parentProperties);
        if (subProperties.empty()) {
          result->clear();
        } else {
          // Initialize new elements with dummy property.
          result->resize(subProperties.size(), subProperties.back());
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
      template<class Walker>
      void walk(Walker& walker)
      { getTopNode().walk(walker); }

      /// Forward walk to the top node
      template<class Walker, class PriorityMeasure>
      void walk(Walker& walker, PriorityMeasure& priority)
      { getTopNode().walk(walker, priority); }

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
        fell();
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
