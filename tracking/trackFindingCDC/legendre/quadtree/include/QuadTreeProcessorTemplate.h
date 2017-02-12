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

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeItem.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * This abstract class serves as a base class for all implementations of track processors.
     * It provides some functions to create, fill, clear and postprocess a quad tree.
     * If you want to use your own class as a quad tree item, you have to overload this processor (not the quad tree itself as it is templated).
     * You have provide only the two functions insertItemInNode and createChildWithParent.
     */
    template<typename typeX, typename typeY, class typeData, int binCountX, int binCountY>
    class QuadTreeProcessorTemplate {

    public:
      /// The QuadTree will only see items of this type
      using ItemType = QuadTreeItem<typeData>;

      /// The type of the list of result items returned to the lambda function
      using ReturnList = std::vector<typeData*>;

      /// The used QuadTree
      using QuadTree = QuadTreeTemplate<typeX, typeY, ItemType, binCountX, binCountY>;

      /// This lambda function can be used for postprocessing
      using CandidateProcessorLambda = std::function<void(const ReturnList&, QuadTree*)>;

      /// Alias for the QuadTree Children
      using QuadTreeChildren = typename QuadTree::Children;

      /// This pair describes the range in X for a node
      using rangeX = std::pair<typeX, typeX>;

      /// This pair describes the range in Y for a node
      using rangeY = std::pair<typeY, typeY>;

      /// This pair of ranges describes the range of a node
      using ChildRanges = std::pair<rangeX, rangeY>;

      /// QuadTree can access private members
      friend QuadTree;

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
      const std::map<std::pair<typeX, typeY>, std::vector<ItemType*>>& getDebugInformation()
      {
        return m_debugOutputMap;
      }

    protected:
      /**
       * Implement that function if you want to provide a new processor. It decides which node-ranges the n * m children of the node should have.
       * It is called when creating the nodes. The two indices iX and iY tell you where the new node will be created (as node.children[iX][iY]).
       * You can check some information on the level or the x- or y-values by using the methods implemented for node.
       * @return a ChildRange pair of a x- and a y-range that the new child range should have.
       * If you don nt want to provide custom ranges, just return ChildRanges(rangeX(node->getXBin(iX), node->getXBin(iX + 1)),
       * rangeY(node->getYBin(iY), node->getYBin(iY + 1)));
       */
      virtual ChildRanges createChildWithParent(QuadTree* node, unsigned int iX, unsigned int iY) const
      {
        typeX xMin = node->getXBin(iX);
        typeX xMax = node->getXBin(iX + 1);
        typeY yMin = node->getYBin(iY);
        typeY yMax = node->getYBin(iY + 1);
        return ChildRanges(rangeX(xMin, xMax), rangeY(yMin, yMax));
      }

      /**
       * Implement that function if you want to provide a new processor. It is called when filling the quad tree after creation.
       * For every item in a node and every child node this function gets called and should decide, if the item should go into this child node or not.
       * @param node  child node
       * @param item  item to be filled into the child node or not
       * @param iX    index in x axes of the node
       * @param iY    index in y axes of the node
       * @return true if this item belongs into this node.
       */
      virtual bool insertItemInNode(QuadTree* node, typeData* item) const = 0;

      /**
       * Override that function if you want to receive debug output whenever the children of a node are filled the first time
       * Maybe you want to make some nice plots or statistics.
       */
      virtual void afterFillDebugHook(QuadTreeChildren* children)
      {
        if (m_debugOutput) {
          children->apply(
          [&](QuadTree * childNode) {
            if (childNode->getLevel() == getLastLevel()) {
              m_debugOutputMap.insert(std::make_pair(std::make_pair(childNode->getXMean(), childNode->getYMean()), childNode->getItemsVector()));
            }
          }
          );
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
                         unsigned int nHitsThreshold, typeY rThreshold)
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
        fillGivenTree(m_quadTree, lmdProcessor, nHitsThreshold, static_cast<typeY>(0), false);
      }


      /**
       * Fill in the items in the given vector. They are transformed to QuadTreeItems internally.
       */
      virtual void provideItemsSet(std::vector<typeData*>& itemsVector)
      {
        std::vector<ItemType*>& quadtreeItemsVector = m_quadTree->getItemsVector();
        quadtreeItemsVector.reserve(itemsVector.size());
        for (typeData* item : itemsVector) {
          quadtreeItemsVector.push_back(new ItemType(item));
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
        node->getChildren()->apply([neededSize](QuadTree * qt) {qt->reserveHitsVector(neededSize);});

        //Voting within the four bins
        for (ItemType* item : items) {
          if (item->isUsed()) {
            continue;
          }

          for (int t_index = 0; t_index < binCountX; ++t_index) {
            for (int r_index = 0; r_index < binCountY; ++r_index) {
              if (insertItemInNode(node->getChildren()->get(t_index, r_index), item->getPointer())) {
                node->getChildren()->get(t_index, r_index)->insertItem(item);
              }
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
      void fillGivenTree(QuadTree* node, CandidateProcessorLambda& lmdProcessor, unsigned int nItemsThreshold, typeY rThreshold,
                         bool checkThreshold)
      {
        B2DEBUG(100, "startFillingTree with " << node->getItemsVector().size() << " hits at level " << static_cast<unsigned int>
                (node->getLevel()) << " (" << node->getXMean() << "/ " << node->getYMean() << ")");
        if (node->getItemsVector().size() < nItemsThreshold) {
          return;
        }
        if (checkThreshold) {
          if ((node->getYMin() * node->getYMax() >= 0) && (fabs(node->getYMin()) > rThreshold) &&
              (fabs(node->getYMax()) > rThreshold)) {
            return;
          }
        }

        if (checkIfLastLevel(node)) {
          callResultFunction(node, lmdProcessor);
          return;
        }

        if (node->getChildren() == nullptr) {
          node->initializeChildren(*this);
        }

        if (!node->checkFilled()) {
          fillChildren(node, node->getItemsVector());
          node->setFilled();
        }

        constexpr int m_nbins_theta = binCountX;
        constexpr int m_nbins_r = binCountY;

        bool binUsed[m_nbins_theta][m_nbins_r];
        for (int iX = 0; iX < m_nbins_theta; iX++) {
          for (int iY = 0; iY < m_nbins_r; iY++) {
            binUsed[iX][iY] = false;
          }
        }

        //Processing, which bins are further investigated
        for (int bin_loop = 0; bin_loop < binCountX * binCountY; bin_loop++) {

          // Search for the bin with the highest bin content.
          int xIndexMax = 0;
          int yIndexMax = 0;
          size_t maxValue = 0;
          for (int xIndexLoop = 0; xIndexLoop < binCountX; ++xIndexLoop) {
            for (int yIndexLoop = 0; yIndexLoop < binCountY; ++yIndexLoop) {
              if ((maxValue < node->getChildren()->get(xIndexLoop, yIndexLoop)->getNItems())
                  && (!binUsed[xIndexLoop][yIndexLoop])) {
                maxValue = node->getChildren()->get(xIndexLoop, yIndexLoop)->getNItems();
                xIndexMax = xIndexLoop;
                yIndexMax = yIndexLoop;
              }
            }
          }

          // Go down one level for the bin with the maximum number of items in it
          binUsed[xIndexMax][yIndexMax] = true;

          // After we have processed the children we need to get rid of the already used hits in all the children, because this can change the number of items drastically
          node->getChildren()->get(xIndexMax, yIndexMax)->cleanUpItems(*this);

          this->fillGivenTree(node->getChildren()->get(xIndexMax, yIndexMax), lmdProcessor, nItemsThreshold, rThreshold, checkThreshold);

        }
      }

      /**
       * Delete all the QuadTreeItems in the tree and clear the tree.
       */
      void clear()
      {
        const std::vector<ItemType*>& quadtreeItemsVector = m_quadTree->getItemsVector();
        for (ItemType* item : quadtreeItemsVector) {
          delete item;
        }
        m_quadTree->clearTree();
      }


      /**
       * When a node is accepted as a result, we extract a vector with the items (back transformed to typeData*)
       * and pass it together with the result node to the given lambda function.
       */
      void callResultFunction(QuadTree* node, CandidateProcessorLambda& lambda) const
      {
        ReturnList resultItems;
        const std::vector<ItemType*>& quadTreeItems = node->getItemsVector();
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
        m_quadTree = new QuadTree(x.first, x.second, y.first, y.second, 0, nullptr);
      }



      /**
       * Creates the sub node of a given node. This function is called by fillGivenTree.
       * To calculate the ranges of the children nodes the user-defined function createChiildWithParent is used.
       */
      void initializeChildren(QuadTree* node, QuadTreeChildren* m_children) const
      {
        for (unsigned int i = 0; i < binCountX; ++i) {
          for (unsigned int j = 0; j < binCountY; ++j) {
            const ChildRanges& childRanges = createChildWithParent(node, i, j);
            const rangeX& rangeX = childRanges.first;
            const rangeY& rangeY = childRanges.second;
            m_children->set(i, j, new QuadTree(rangeX.first, rangeX.second, rangeY.first, rangeY.second, node->getLevel() + 1, node));
          }
        }
      }

      unsigned int m_lastLevel; /**< The last level to be filled */
      bool m_debugOutput; /**< A flag to control the creation of the debug output */
      std::map<std::pair<typeX, typeY>, std::vector<ItemType*>> m_debugOutputMap; /**< The calculated debug map */
      bool m_param_setUsedFlag; /**< Set the used flag after every lambda function call */
    };
  }
}
