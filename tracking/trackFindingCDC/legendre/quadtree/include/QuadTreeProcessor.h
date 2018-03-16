/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth, Nils Braun                  *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeNode.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeItem.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <map>
#include <vector>
#include <deque>
#include <utility>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * This abstract class serves as a base class for all implementations of track processors.
     * It provides some functions to create, fill, clear and postprocess a quad tree.
     * If you want to use your own class as a quad tree item, you have to overload this processor.
     * You have provide only the two functions isInNode and createChild.
     */
    template<typename AX, typename AY, class AData>
    class QuadTreeProcessor {

    public:
      /// The QuadTree will only see items of this type
      using Item = QuadTreeItem<AData>;

      /// The used QuadTree
      using QuadTree = QuadTreeNode<AX, AY, Item>;

      /// This pair describes the span in X for a node
      using XSpan = typename QuadTree::XSpan;

      /// This pair describes the span in Y for a node
      using YSpan = typename QuadTree::YSpan;

      /// This pair of spans describes the span of a node
      using XYSpans = std::pair<XSpan, YSpan>;

      /// Alias for the QuadTree Children
      using QuadTreeChildren = typename QuadTree::Children;

      /// This lambda function can be used for postprocessing
      using CandidateReceiver = std::function<void(const std::vector<AData*>&, QuadTree*)>;

    public:
      /**
       * Constructor is very simple. The QuadTree has to be constructed elsewhere.
       * @param lastLevel describing the last search level for the quad tree creation.
       * @param spans spans of the QuadTree at the top level
       * @param setUsedFlag Set the used flag after every lambda function call
       */
      QuadTreeProcessor(int lastLevel,
                        int seedLevel,
                        const XYSpans& xySpans,
                        bool debugOutput = false)
        : m_quadTree{std::make_unique<QuadTree>(xySpans.first, xySpans.second, 0, nullptr)}
        , m_lastLevel(lastLevel)
        , m_seedLevel(seedLevel)
        , m_debugOutput(debugOutput)
        , m_debugOutputMap()
      {
      }

      /**
       * Destructor deletes the quad tree.
       */
      virtual ~QuadTreeProcessor()
      {
        clear();
      }

      /**
       * Delete all the QuadTreeItems in the tree and clear the tree.
       */
      void clear()
      {
        m_seededTrees.clear();
        m_quadTree->clearChildren();
        m_quadTree->clearItems();
        m_items.clear();
      }

      /**
       * Fill in the items in the given vector. They are transformed to QuadTreeItems internally.
       */
      void seed(std::vector<AData*>& datas)
      {
        // Create the items
        for (AData* data : datas) {
          m_items.emplace_back(data);
        }

        // Creating the seed level
        long nSeedBins = pow(2, m_seedLevel);
        m_seededTrees.reserve(nSeedBins * nSeedBins);

        // Expand the first levels to the seed sectors
        m_seededTrees.push_back(m_quadTree.get());
        std::vector<QuadTree*> nextSeededTrees;

        for (int level = 0; level < m_seedLevel; ++level) {
          for (QuadTree* node : m_seededTrees) {
            if (node->getChildren().empty()) {
              this->createChildren(node, node->getChildren());
            }
            for (QuadTree& child :  node->getChildren()) {
              nextSeededTrees.push_back(&child);
            }
          }
          std::swap(nextSeededTrees, m_seededTrees);
          nextSeededTrees.clear();
        }

        // Fill the seed level with the items
        for (QuadTree* seededTree : m_seededTrees) {
          seededTree->reserveItems(m_items.size());

          for (Item& item : m_items) {
            if (item.isUsed()) continue;
            if (isInNode(seededTree, item.getPointer())) {
              seededTree->insertItem(&item);
            }
          }
        }
      }

    public:
      /**
       * Get items that have been assigned to the seed level
       * The returned elements are unique even if items are assigned multiple times.
       */
      std::vector<AData*> getAssignedItems()
      {
        std::vector<const CDCWireHit*> result;
        for (QuadTree* seededTree : m_seededTrees) {
          for (Item* item : seededTree->getItems()) {
            result.push_back(item->getPointer());
          }
        }
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
      }

    public:
      /**
       * Start filling the already created tree.
       * @param candidateReceiver the lambda function to call after a node was selected
       * @param nHitsThreshold the threshold on the number of items
       */
      void fill(const CandidateReceiver& candidateReceiver, int nHitsThreshold)
      {
        fill(candidateReceiver, nHitsThreshold, std::numeric_limits<AY>::max());
      }

      /**
       * Fill vector of QuadTree instances with hits.
       * @param candidateReceiver the lambda function to call after a node was selected
       * @param nHitsThreshold the threshold on the number of items
       * @param yLimit the threshold in the rho (curvature) variable
       */
      void fill(const CandidateReceiver& candidateReceiver, int nHitsThreshold, float yLimit)
      {
        std::vector<QuadTree*> quadTrees = m_seededTrees;
        std::sort(quadTrees.begin(), quadTrees.end(), [](QuadTree * quadTree1, QuadTree * quadTree2) {
          return quadTree1->getNItems() > quadTree2->getNItems();
        });

        for (QuadTree* tree : quadTrees) {
          erase_remove_if(tree->getItems(), [](Item * hit) { return hit->isUsed(); });
          fillGivenTree(tree, candidateReceiver, nHitsThreshold, yLimit);
        }
      }

    private:
      /**
       * Internal function to do the real quad tree search: fill the nodes, check which of the n*m bins we need to
       * process further and go one level deeper.
       */
      void fillGivenTree(QuadTree* node,
                         const CandidateReceiver& candidateReceiver,
                         int nItemsThreshold,
                         AY yLimit)
      {
        if (node->getNItems() < nItemsThreshold) {
          return;
        }

        if ((node->getYMin() > yLimit) or (-node->getYMax() > yLimit)) {
          return;
        }

        if (isLeaf(node)) {
          callResultFunction(node, candidateReceiver);
          return;
        }

        if (node->getChildren().empty()) {
          this->createChildren(node, node->getChildren());
        }

        if (!node->checkFilled()) {
          fillChildren(node, node->getItems());
          node->setFilled();
        }

        std::vector<QuadTree*> children;
        for (QuadTree& child : node->getChildren()) {
          children.push_back(&child);
        }
        const auto compareNItems = [](const QuadTree * lhs, const QuadTree * rhs) {
          return lhs->getNItems() < rhs->getNItems();
        };

        // Explicitly count down the children
        const int nChildren = children.size();
        for (int iChild = 0; iChild < nChildren; ++iChild) {
          auto itHeaviestChild = std::max_element(children.begin(), children.end(), compareNItems);
          QuadTree* heaviestChild = *itHeaviestChild;
          children.erase(itHeaviestChild);
          // After we have processed some children we need to get rid of the already used hits in all the children,
          // because this can change the number of items drastically
          erase_remove_if(heaviestChild->getItems(), [&](Item * hit) { return hit->isUsed(); });
          this->fillGivenTree(heaviestChild, candidateReceiver, nItemsThreshold, yLimit);
        }
      }

      /**
       * Creates the sub node of a given node. This function is called by fillGivenTree.
       * To calculate the spans of the children nodes the user-defined function createChiildWithParent is used.
       */
      void createChildren(QuadTree* node, QuadTreeChildren& m_children) const
      {
        for (int i = 0; i < node->getXNbins(); ++i) {
          for (int j = 0; j < node->getYNbins(); ++j) {
            const XYSpans& xySpans = createChild(node, i, j);
            const XSpan& xSpan = xySpans.first;
            const YSpan& ySpan = xySpans.second;
            m_children.push_back(QuadTree(xSpan, ySpan, node->getLevel() + 1, node));
          }
        }
      }

      /**
       * This function is called by fillGivenTree and fills the items into the corresponding children.
       * For this the user-defined method isInNode is called.
       */
      void fillChildren(QuadTree* node, std::vector<Item*>& items)
      {
        const size_t neededSize = 2 * items.size();
        for (QuadTree& child : node->getChildren()) {
          child.reserveItems(neededSize);
        }

        for (Item* item : items) {
          if (item->isUsed()) continue;

          for (QuadTree& child : node->getChildren()) {
            if (isInNode(&child, item->getPointer())) {
              child.insertItem(item);
            }
          }
        }
        afterFillDebugHook(node->getChildren());
      }

      /**
       * When a node is accepted as a result, we extract a vector with the items (back transformed to AData*)
       * and pass it together with the result node to the candidate receiver function.
       */
      void callResultFunction(QuadTree* node, const CandidateReceiver& candidateReceiver) const
      {
        const std::vector<Item*>& foundItems = node->getItems();
        std::vector<AData*> candidate;
        candidate.reserve(foundItems.size());

        for (Item* item : foundItems) {
          item->setUsedFlag();
          candidate.push_back(item->getPointer());
        }

        candidateReceiver(candidate, node);
      }

    protected: // Section of specialisable functions
      /**
       * Implement that function if you want to provide a new processor. It decides which node-spans the n * m children of the node should have.
       * It is called when creating the nodes. The two indices iX and iY tell you where the new node will be created (as node.children[iX][iY]).
       * You can check some information on the level or the x- or y-values by using the methods implemented for node.
       * @return a XYSpan pair of a x- and a y-span that the new child should have.
       * If you don nt want to provide custom spans, just return XYSpans(XSpan(node->getXBinBound(iX), node->getXBinBound(iX + 1)),
       * YSpan(node->getYBinBound(iY), node->getYBinBound(iY + 1)));
       */
      virtual XYSpans createChild(QuadTree* node, int iX, int iY) const
      {
        AX xMin = node->getXLowerBound(iX);
        AX xMax = node->getXUpperBound(iX);
        AY yMin = node->getYLowerBound(iY);
        AY yMax = node->getYUpperBound(iY);
        return XYSpans({xMin, xMax}, {yMin, yMax});
      }

      /**
       * Implement that function if you want to provide a new processor. It is called when filling the quad tree after creation.
       * For every item in a node and every child node this function gets called and should decide, if the item should go into this child node or not.
       * @param node  child node
       * @param item  item to be filled into the child node or not
       * @return true if this item belongs into this node.
       */
      virtual bool isInNode(QuadTree* node, AData* item) const = 0;

      /**
       * Function which checks if given node is leaf
       * Implemented as virtual to keep possibility of changing lastLevel values depending on region is phase-space
       * (i.e. setting lastLevel as a function of Y-variable)
       */
      virtual bool isLeaf(QuadTree* node) const
      {
        if (node->getLevel() >= m_lastLevel) {
          return true;
        } else {
          return false;
        }
      }

      /**
       * Return the parameter last level.
       */
      int getLastLevel() const
      {
        return m_lastLevel;
      }

    public: // debug stuff
      /**
       * Override that function if you want to receive debug output whenever the children of a node are filled the first time
       * Maybe you want to make some nice plots or statistics.
       */
      virtual void afterFillDebugHook(QuadTreeChildren& children)
      {
        if (not m_debugOutput) return;
        for (QuadTree& childNode : children) {
          if (childNode.getLevel() != getLastLevel()) continue; // Only write the lowest level
          //m_debugOutputMap[ {childNode.getXMean(), childNode.getYMean()}] = childNode.getItems();
        }
      }

      /**
       * Return the debug information if collected
       */
      const std::map<std::pair<AX, AY>, std::vector<Item*>>& getDebugInformation() const
      {
        return m_debugOutputMap;
      }

    protected:
      /// The quad tree we work with
      std::unique_ptr<QuadTree> m_quadTree;

      /// Storage space for the items that are referenced by the quad tree nodes
      std::deque<Item> m_items;

      /**
       * Vector of QuadTrees
       * QuadTree instances (which are filled in the vector) cover the whole Legendre phase-space;
       * each instance is processes independently.
       */
      std::vector<QuadTree*> m_seededTrees;

    private:
      /// The last level to be filled
      int m_lastLevel;

      /// The first level to be filled, effectivelly skip forward to this higher granularity level
      int m_seedLevel;

      /// A flag to control the creation of the debug output
      bool m_debugOutput;

      /// The calculated debug map
      std::map<std::pair<AX, AY>, std::vector<Item*>> m_debugOutputMap;
    };
  }
}
