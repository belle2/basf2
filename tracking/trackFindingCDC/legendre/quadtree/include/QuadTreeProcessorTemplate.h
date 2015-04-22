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

#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeItem.h>

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
      typedef QuadTreeItem<typeData> ItemType; /**< The QuadTree will only see items of this type */
      typedef std::vector<typeData*> ReturnList; /**< The type of the list of result items returned to the lambda function */
      typedef QuadTreeTemplate<typeX, typeY, ItemType, binCountX, binCountY> QuadTree;  /**< The used QuadTree */
      typedef std::function< void(const ReturnList&, QuadTree*) >
      CandidateProcessorLambda;   /**< This lambda function can be used for postprocessing */
      typedef typename QuadTree::Children QuadTreeChildren; /**< A typedef for the QuadTree Children */

      typedef std::pair<typeX, typeX> rangeX;   /**< This pair describes the range in X for a node */
      typedef std::pair<typeY, typeY> rangeY;   /**< This pair describes the range in Y for a node */
      typedef std::pair<rangeX, rangeY> ChildRanges; /**< This pair of ranges describes the range of a node */

      friend QuadTree;

    public:
      /**
       * Constructor is very simple. The QuadTree has to be constructed elsewhere.
       * @param lastLevel describing the last search level for the quad tree creation.
       */
      QuadTreeProcessorTemplate(unsigned char lastLevel, const ChildRanges& ranges) : m_lastLevel(lastLevel)
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

    protected:
      /**
       * Implement that function if you want to provide a new processor. It decides which node-ranges the n * m children of the node should have.
       * It is called when creating the nodes. The two indices iX and iY tell you where the new node will be created (as node.children[iX][iY]).
       * You can check some information on the level or the x- or y-values by using the methods implemented for node.
       * @return a ChildRange pair of a x- and a y-range that the new child range should have.
       * If you don nt want to provide custom ranges, just return ChildRanges(rangeX(node->getXBin(iX), node->getXBin(iX + 1)),
       * rangeY(node->getYBin(iY), node->getYBin(iY + 1)));
       */
      virtual ChildRanges createChildWithParent(QuadTree* node, unsigned int iX, unsigned int iY) const = 0;

      /**
       * Implement that function if you want to provide a new processor. It is called when filling the quad tree after creation.
       * For every item in a node and every child node this function gets called and should decide, if the item should go into this child node or not.
       * @param node = child node
       * @param item = item to be filled into the child node or not
       * @param iX, iY the indices of the child node.
       * @return true if this item belongs into this node.
       */
      virtual bool insertItemInNode(QuadTree* node, typeData* item, unsigned int iX, unsigned int iY) const = 0;

    public:
      /**
       * Start filling the already created tree.
       * @param lmdProcessor the lambda function to call after a node was selected
       * @param nHitsThreshold the threshold on the number of items
       * @param rThreshold the threshold in the y variable
       */
      virtual void fillGivenTree(CandidateProcessorLambda& lmdProcessor,
                                 unsigned int nHitsThreshold, typeY rThreshold) const final
      {
        fillGivenTree(m_quadTree, lmdProcessor, nHitsThreshold, rThreshold, true);
      }

      /**
       * Start filling the already created tree.
       * @param lmdProcessor the lambda function to call after a node was selected
       * @param nHitsThreshold the threshold on the number of items
       */
      virtual void fillGivenTree(CandidateProcessorLambda& lmdProcessor,
                                 unsigned int nHitsThreshold) const final
      {
        fillGivenTree(m_quadTree, lmdProcessor, nHitsThreshold, static_cast<typeY>(0), false);
      }

      /**
       * Fill in the items in the given vector. They are transformed to QuadTreeItems internally.
       */
      virtual void provideItemsSet(std::vector<typeData*>& itemsVector) final {
        std::vector<ItemType*>& quadtreeItemsVector = m_quadTree->getItemsVector();
        for (typeData* item : itemsVector)
        {
          quadtreeItemsVector.push_back(new ItemType(item));
        }
      }

    protected:
      /**
       * Return the parameter last level.
       */
      virtual unsigned int getLastLevel() const final
      {
        return m_lastLevel;
      }

    private:
      /**
       * Delete all the QuadTreeItems in the tree and clear the tree.
       */
      virtual void clear()
      {
        const std::vector<ItemType*>& quadtreeItemsVector = m_quadTree->getItemsVector();
        for (ItemType* item : quadtreeItemsVector) {
          delete item;
        }
        m_quadTree->clearTree();
      }

      /**
       * Create a quad tree with the given parameters ranges.
       */
      virtual void createQuadTree(const ChildRanges& ranges) final {
        const rangeX& x = ranges.first;
        const rangeY& y = ranges.second;
        m_quadTree = new QuadTree(x.first, x.second, y.first, y.second, 0, nullptr);
      }

      /**
       * Before making a new search we have to clean up the items that are already used from the items list.
       */
      virtual void cleanUpItems(std::vector<ItemType*>& items) const final
      {
        items.erase(std::remove_if(items.begin(), items.end(),
        [&](ItemType * hit) {
          return hit->isUsed();
        }),
        items.end());
      };

      /**
       * When a node is accepted as a result, we extract a vector with the items (back transformed to typeData*)
       * and pass it together with the result node to the given lambda function.
       */
      virtual void callResultFunction(QuadTree* node, CandidateProcessorLambda& lambda) const final
      {
        ReturnList resultItems;
        const std::vector<ItemType*> quadTreeItems = node->getItemsVector();
        resultItems.reserve(quadTreeItems.size());

        for (ItemType* quadTreeItem : quadTreeItems) {
          quadTreeItem->setUsedFlag();
          resultItems.push_back(quadTreeItem->getPointer());
        }

        lambda(resultItems, node);
      }

      /**
       * Internal function to do the real quad tree search: fill the nodes, check which of the n*m bins we need to
       * process further and go one level deeper.
       */
      virtual void fillGivenTree(QuadTree* node, CandidateProcessorLambda& lmdProcessor, unsigned int nItemsThreshold, typeY rThreshold,
                                 bool checkThreshold) const final
      {
        B2DEBUG(100, "startFillingTree with " << node->getItemsVector().size() << " hits at level " << static_cast<unsigned int>
                (node->getLevel()));
        if (node->getItemsVector().size() < nItemsThreshold)
          return;
        if (checkThreshold) {
          if ((node->getYMin() * node->getYMax() >= 0) && (fabs(node->getYMin()) > rThreshold)  && (fabs(node->getYMax()) > rThreshold))
            return;
        }

        // **** ???? *****
        unsigned char level_diff = 0;
        if (fabs(node->getYMean()) > 0.07) level_diff = 2;
        else if ((fabs(node->getYMean()) < 0.07) && (fabs(node->getYMean()) > 0.04))
          level_diff = 1;
        if (node->getLevel() >= (m_lastLevel - level_diff)) {
          callResultFunction(node, lmdProcessor);
          return;
        }
        // **** ???? *****

        if (node->getChildren() == nullptr)
          node->initializeChildren(*this);

        if (!node->checkFilled()) {
          fillChildren(node, node->getItemsVector());
          node->setFilled();
        }

        constexpr int m_nbins_theta = binCountX;
        constexpr int m_nbins_r = binCountY;

        bool binUsed[m_nbins_theta][m_nbins_r];
        for (int ii = 0; ii < m_nbins_theta; ii++)
          for (int jj = 0; jj < m_nbins_r; jj++)
            binUsed[ii][jj] = false;

        //Processing, which bins are further investigated
        for (int bin_loop = 0; bin_loop < m_nbins_theta * m_nbins_r; bin_loop++) {
          int t_index = 0;
          int r_index = 0;
          float max_value_temp = 0;
          for (int t_index_temp = 0; t_index_temp < m_nbins_theta; ++t_index_temp) {
            for (int r_index_temp = 0; r_index_temp < m_nbins_r; ++r_index_temp) {
              if ((max_value_temp < node->getChildren()->get(t_index_temp, r_index_temp)->getNItems())
                  && (!binUsed[t_index_temp][r_index_temp])) {
                max_value_temp = node->getChildren()->get(t_index_temp, r_index_temp)->getNItems();
                t_index = t_index_temp;
                r_index = r_index_temp;
              }
            }
          }

          binUsed[t_index][r_index] = true;

          node->getChildren()->get(t_index, r_index)->cleanUpItems(*this);
          this->fillGivenTree(node->getChildren()->get(t_index, r_index), lmdProcessor, nItemsThreshold, rThreshold, checkThreshold);
        }
      }

      /**
       * This function is called by fillGivenTree and fills the items into the corresponding children.
       * For this the user-defined method insertItemInNode is called.
       */
      virtual void fillChildren(QuadTree* node, std::vector<ItemType*>& items) const final
      {
        const size_t neededSize = 2 * items.size();
        node->getChildren()->apply([neededSize](QuadTree * qt) {qt->reserveHitsVector(neededSize);});

        //Voting within the four bins
        for (ItemType* item : items) {
          if (item->isUsed())
            continue;

          for (int t_index = 0; t_index < binCountX; ++t_index) {
            for (int r_index = 0; r_index < binCountY; ++r_index) {
              if (insertItemInNode(node->getChildren()->get(t_index, r_index), item->getPointer(), t_index, r_index)) {
                node->getChildren()->get(t_index, r_index)->insertItem(item);
              }
            }
          }
        }
      }

      /**
       * Creates the sub node of a given node. This function is called by fillGivenTree.
       * To calculate the ranges of the children nodes the user-defined function createChiildWithParent is used.
       */
      virtual void initializeChildren(QuadTree* node, QuadTreeChildren* m_children) const final
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

      // depricated
      virtual void provideItemsSet(const std::set<ItemType*>& items_set, std::vector<ItemType*>& items_vector) const final
      {
        items_vector.clear();
        items_vector.reserve(items_set.size());
        std::copy_if(items_set.begin(), items_set.end(), std::back_inserter(items_vector),
        [&](ItemType * item) {return not item->isUsed();});
      };

      unsigned int m_lastLevel; /**< The last level to be filled */
      QuadTree* m_quadTree; /**< The quad tree we work with */
    };
  }
}
