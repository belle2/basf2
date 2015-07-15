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

#include <framework/logging/Logger.h>
#include <vector>
#include <deque>
#include <algorithm>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class Properties_, template <class Node> class ChildrenStructure_>
    class DynTree {

      /// Type of this class
      typedef DynTree<Properties_, ChildrenStructure_> This;

      /// Type of the Properties
      typedef Properties_ Properties;

    public:
      /// Class for a node in the tree
      class Node : public Properties_ {

      public:
        /// Type of the tree containing this node.
        typedef This Tree;

        /// Allow the tree access to the node constructor to create the top node.
        friend Tree;

        /// Type of this class
        typedef Node This;

        /// Type of the Properties
        typedef Properties_ Properties;

        /// Type of the container of the children of the node
        typedef ChildrenStructure_<Node> Children;

      private:
        /** Inheriting the constructors of the properties such that we can
         *  construct nodes as property objects.
         *  Only the top level node is constructed this way.
         */
        using Properties_::Properties_;

      public:
        /** Getter for the children.
         *  Get the dynamically created children.
         *  Returns nullptr if they have not yet been created
         */
        Children* getChildren()
        // { return m_children.get(); }
        { return m_children; }

        /** Const getter for the children.
         *  Get the dynamically created children.
         *  Returns nullptr if they have not yet been created
         */
        const Children* getChildren() const
        // { return m_children.get(); }
        { return m_children; }

        /// Creates the children of the node.
        void createChildren()
        {
          // ensure the level value fits into unsigned char
          B2ASSERT("DynTree datastructure only supports levels < 255", getLevel() < 255);

          const Node& parentNode = *this;
          // m_children = std::unique_ptr<Children>(new Children(parentNode));

          getTree()->m_children.emplace_back(parentNode);
          m_children = &(getTree()->m_children.back());

          for (Node& child : *m_children) {
            child.m_level = getLevel() + 1;
            child.m_parent = this;
            child.m_tree = getTree();
          }
        }

      private:
        /// Delete the children of the node.
        void deleteChildren()
        {
          // m_children.reset(nullptr);
          m_children = nullptr;
        }

      public:
        /** Calls the walker with each node starting with the top node and continues depth first
         *  The walker can signal to skip the children if false is returned.
         */
        template<class Walker>
        void walk(Walker& walker)
        //void walk(std::function<bool(Node*)> walker)
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
        //void walk(std::function<bool(Node*)> walker, std::function<float(Node*)> priority)
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
            for (int i = 0; i < nChildren; ++i) {
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
        /// Children that are created only if needed
        // std::unique_ptr<Children> m_children = nullptr;
        Children* m_children = nullptr;

        /// Level of the node within the tree
        std::size_t m_level = 0;

        /// Parent in the tree hierachy of this node.
        Node* m_parent = nullptr;

        /// Reference to the tree that contains this node
        Tree* m_tree = nullptr;
      };

    public:
      /// Constructor taking properties with which the top node of the tree is initialised.
      DynTree(const Properties& properties) :
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

      /// Forward walk to the top node
      template<class Walker>
      void walk(Walker& walker)
      // void walk(std::function<bool(Node*)> walker)
      { getTopNode().walk(walker); }

      /// Forward walk to the top node
      template<class Walker, class PriorityMeasure>
      void walk(Walker& walker, PriorityMeasure& priority)
      // void walk(std::function<bool(Node*)> walker, std::function<float(Node*)> priority)
      { getTopNode().walk(walker, priority); }

      /// Fell to tree meaning deleting all child nodes from the tree. Keeps the top node.
      void fell()
      {
        m_topNode.deleteChildren();
        m_children.clear();
      }

      /// Like fell but also releases all memory the tree has aquired during long execution.
      void raze()
      {
        fell();
        m_children.shrink_to_fit();
      }

    public:
      /// Memory for the top node of the tree
      Node m_topNode;

      /// Central point to provide memory for the child structures
      std::deque<typename Node::Children> m_children;

    };
  }
}
