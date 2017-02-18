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

#include <algorithm>
#include <functional>
#include <map>
#include <vector>
#include <utility>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * This abstract class serves as a base class for all implementations of track processors.
     * It provides some functions to create, fill, clear and postprocess a quad tree.
     * If you want to use your own class as a quad tree item, you have to overload this processor.
     * You have provide only the two functions insertItemInNode and createChildWithParent.
     */
    template<typename AX, typename AY, class AData>
    class QuadTreeProcessorTemplate {

    public:
      /// The QuadTree will only see items of this type
      using ItemType = QuadTreeItem<AData>;

      /// The type of the list of result items returned to the lambda function
      using ReturnList = std::vector<AData*>;

      /// The used QuadTree
      using QuadTree = QuadTreeNode<AX, AY, ItemType>;

      /// This lambda function can be used for postprocessing
      using CandidateProcessorLambda = std::function<void(const ReturnList&, QuadTree*)>;

      /// Alias for the QuadTree Children
      using QuadTreeChildren = typename QuadTree::Children;

      /// This pair describes the range in X for a node
      using rangeX = std::pair<AX, AX>;

      /// This pair describes the range in Y for a node
      using rangeY = std::pair<AY, AY>;

      /// This pair of ranges describes the range of a node
      using ChildRanges = std::pair<rangeX, rangeY>;

    public:
      /**
       * Constructor is very simple. The QuadTree has to be constructed elsewhere.
       * @param lastLevel describing the last search level for the quad tree creation.
       * @param ranges ranges of the QuadTree at the top level
       * @param setUsedFlag Set the used flag after every lambda function call
       */
      QuadTreeProcessorTemplate(unsigned char lastLevel, const ChildRanges& ranges, bool debugOutput = false, bool setUsedFlag = true) :
        m_lastLevel(lastLevel), m_debugOutput(debugOutput), m_debugOutputMap(), m_param_setUsedFlag(setUsedFlag)
      {
        createQuadTree(ranges);
      }

      /**
       * Destructor deletes the quad tree.
       */
      virtual ~QuadTreeProcessorTemplate()
      {
        clear();
        delete m_quadTree;
      }

      /**
       * Return the debug information if collected
       */
      const std::map<std::pair<AX, AY>, std::vector<ItemType*>>& getDebugInformation()
      {
        return m_debugOutputMap;
      }

    protected:
      /**
       * Implement that function if you want to provide a new processor. It decides which node-ranges the n * m children of the node should have.
       * It is called when creating the nodes. The two indices iX and iY tell you where the new node will be created (as node.children[iX][iY]).
       * You can check some information on the level or the x- or y-values by using the methods implemented for node.
       * @return a ChildRange pair of a x- and a y-range that the new child range should have.
       * If you don nt want to provide custom ranges, just return ChildRanges(rangeX(node->getXBinBound(iX), node->getXBinBound(iX + 1)),
       * rangeY(node->getYBinBound(iY), node->getYBinBound(iY + 1)));
       */
      virtual ChildRanges createChildWithParent(QuadTree* node, unsigned int iX, unsigned int iY) const
      {
        AX xMin = node->getXBinBound(iX);
        AX xMax = node->getXBinBound(iX + 1);
        AY yMin = node->getYBinBound(iY);
        AY yMax = node->getYBinBound(iY + 1);
        return ChildRanges(rangeX(xMin, xMax), rangeY(yMin, yMax));
      }

      /**
       * Implement that function if you want to provide a new processor. It is called when filling the quad tree after creation.
       * For every item in a node and every child node this function gets called and should decide, if the item should go into this child node or not.
       * @param node  child node
       * @param item  item to be filled into the child node or not
       * @return true if this item belongs into this node.
       */
      virtual bool insertItemInNode(QuadTree* node, AData* item) const = 0;

      /**
       * Override that function if you want to receive debug output whenever the children of a node are filled the first time
       * Maybe you want to make some nice plots or statistics.
       */
      virtual void afterFillDebugHook(QuadTreeChildren* children)
      {
        if (not m_debugOutput) return;
        for (QuadTree* childNode : *children) {
          if (childNode->getLevel() != getLastLevel()) continue; // Only write the lowest level
          m_debugOutputMap[ {childNode->getXMean(), childNode->getYMean()}] = childNode->getItems();
        }
      }

      /**
       * Before making a new search we have to clean up the items that are already used from the items list.
       */
      void cleanUpItems(std::vector<ItemType*>& items) const
      {
        items.erase(std::remove_if(items.begin(), items.end(),
        [&](ItemType * hit) {
          return hit->isUsed();
        }),
        items.end());
      };


    public:
      /**
       * Start filling the already created tree.
       * @param lmdProcessor the lambda function to call after a node was selected
       * @param nHitsThreshold the threshold on the number of items
       * @param rThreshold the threshold in the y variable
       */
      void fillGivenTree(CandidateProcessorLambda& lmdProcessor,
                         unsigned int nHitsThreshold, AY rThreshold)
      {
        fillGivenTree(m_quadTree, lmdProcessor, nHitsThreshold, rThreshold, true);
      }

      /**
       * Start filling the already created tree.
       * @param lmdProcessor the lambda function to call after a node was selected
       * @param nHitsThreshold the threshold on the number of items
       */
      void fillGivenTree(CandidateProcessorLambda& lmdProcessor,
                         unsigned int nHitsThreshold)
      {
        fillGivenTree(m_quadTree, lmdProcessor, nHitsThreshold, static_cast<AY>(0), false);
      }


      /**
       * Fill in the items in the given vector. They are transformed to QuadTreeItems internally.
       */
      virtual void provideItemsSet(std::vector<AData*>& itemsVector)
      {
        std::vector<ItemType*>& quadtreeItems = m_quadTree->getItems();
        quadtreeItems.reserve(itemsVector.size());
        for (AData* item : itemsVector) {
          quadtreeItems.push_back(new ItemType(item));
        }
      }

    protected:
      /**
       * Return the parameter last level.
       */
      unsigned int getLastLevel() const
      {
        return m_lastLevel;
      }

      /**
       * This function is called by fillGivenTree and fills the items into the corresponding children.
       * For this the user-defined method insertItemInNode is called.
       */
      virtual void fillChildren(QuadTree* node, std::vector<ItemType*>& items)
      {
        const size_t neededSize = 2 * items.size();
        for (QuadTree* child : *node->getChildren()) {
          child->reserveItems(neededSize);
        }

        //Voting within the four bins
        for (ItemType* item : items) {
          if (item->isUsed()) continue;

          for (QuadTree* child : *node->getChildren()) {
            if (insertItemInNode(child, item->getPointer())) {
              child->insertItem(item);
            }
          }
        }

        afterFillDebugHook(node->getChildren());
      }

      /**
       * Function which checks if given node is leaf
       * Implemented as virtual to keep possibility of changing lastLevel values depending on region is phase-space
       * (i.e. setting lastLevel as a function of Y-variable)
       */
      virtual bool checkIfLastLevel(QuadTree* node)
      {
        if (node->getLevel() >= getLastLevel()) {
          return true;
        } else {
          return false;
        }
      }


      /**
       * Internal function to do the real quad tree search: fill the nodes, check which of the n*m bins we need to
       * process further and go one level deeper.
       */
      void fillGivenTree(QuadTree* node,
                         CandidateProcessorLambda& lmdProcessor,
                         unsigned int nItemsThreshold,
                         AY rThreshold,
                         bool checkThreshold)
      {
        B2DEBUG(100, "startFillingTree with " << node->getItems().size() << " hits at level " << static_cast<unsigned int>
                (node->getLevel()) << " (" << node->getXMean() << "/ " << node->getYMean() << ")");
        if (node->getItems().size() < nItemsThreshold) {
          return;
        }
        if (checkThreshold) {
          if ((node->getYMin() * node->getYMax() >= 0) && (std::fabs(node->getYMin()) > rThreshold) &&
              (fabs(node->getYMax()) > rThreshold)) {
            return;
          }
        }

        if (checkIfLastLevel(node)) {
          callResultFunction(node, lmdProcessor);
          return;
        }

        if (node->getChildren() == nullptr) {
          node->createChildren();
          this->initializeChildren(node, node->getChildren());
        }

        if (!node->checkFilled()) {
          fillChildren(node, node->getItems());
          node->setFilled();
        }

        std::vector<QuadTree*> children(node->getChildren()->begin(), node->getChildren()->end());
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
          this->cleanUpItems(heaviestChild->getItems());
          this->fillGivenTree(heaviestChild, lmdProcessor, nItemsThreshold, rThreshold, checkThreshold);
        }
      }

      /**
       * Delete all the QuadTreeItems in the tree and clear the tree.
       */
      void clear()
      {
        const std::vector<ItemType*>& quadtreeItems = m_quadTree->getItems();
        for (ItemType* item : quadtreeItems) {
          delete item;
        }
        m_quadTree->clearChildren();
      }


      /**
       * When a node is accepted as a result, we extract a vector with the items (back transformed to AData*)
       * and pass it together with the result node to the given lambda function.
       */
      void callResultFunction(QuadTree* node, CandidateProcessorLambda& lambda) const
      {
        ReturnList resultItems;
        const std::vector<ItemType*>& quadTreeItems = node->getItems();
        resultItems.reserve(quadTreeItems.size());

        for (ItemType* quadTreeItem : quadTreeItems) {
          quadTreeItem->setUsedFlag(m_param_setUsedFlag);
          resultItems.push_back(quadTreeItem->getPointer());
        }

        lambda(resultItems, node);
      }


      QuadTree* m_quadTree; /**< The quad tree we work with */

    private:

      /**
       * Create a quad tree with the given parameters ranges.
       */
      void createQuadTree(const ChildRanges& ranges)
      {
        const rangeX& x = ranges.first;
        const rangeY& y = ranges.second;
        m_quadTree = new QuadTree({x.first, x.second}, {y.first, y.second}, 0, nullptr);
      }

      /**
       * Creates the sub node of a given node. This function is called by fillGivenTree.
       * To calculate the ranges of the children nodes the user-defined function createChiildWithParent is used.
       */
      void initializeChildren(QuadTree* node, QuadTreeChildren* m_children) const
      {
        for (int i = 0; i < node->getXNbins(); ++i) {
          for (int j = 0; j < node->getYNbins(); ++j) {
            const ChildRanges& childRanges = createChildWithParent(node, i, j);
            const rangeX& rangeX = childRanges.first;
            const rangeY& rangeY = childRanges.second;
            m_children->set(i, j, new QuadTree({rangeX.first, rangeX.second}, {rangeY.first, rangeY.second}, node->getLevel() + 1, node));
          }
        }
      }

      unsigned int m_lastLevel; /**< The last level to be filled */
      bool m_debugOutput; /**< A flag to control the creation of the debug output */
      std::map<std::pair<AX, AY>, std::vector<ItemType*>> m_debugOutputMap; /**< The calculated debug map */
      bool m_param_setUsedFlag; /**< Set the used flag after every lambda function call */
    };
  }
}
