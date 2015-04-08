#pragma once

#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeItem.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<typename typeX, typename typeY, class typeData, int binCountX, int binCountY>
    class QuadTreeProcessorTemplate {

    public:
      typedef QuadTreeItem<typeData> QuadTreeItem;
      typedef QuadTreeTemplate<typeX, typeY, QuadTreeItem, binCountX, binCountY> QuadTree;
      typedef typename QuadTree::CandidateProcessorLambda CandidateProcessorLambda;
      typedef typename QuadTree::Children QuadTreeChildren;

      typedef std::pair<typeX, typeX> rangeX;
      typedef std::pair<typeY, typeY> rangeY;
      typedef std::pair<rangeX, rangeY> ChildRanges;

    public:
      QuadTreeProcessorTemplate(unsigned char lastLevel) : m_lastLevel(lastLevel) {}
      virtual ~QuadTreeProcessorTemplate() { }

      virtual ChildRanges createChildWithParent(QuadTree* node, unsigned int i, unsigned int j) const = 0;
      virtual bool insertItemInNode(QuadTree* node, typeData* item, unsigned int xIndex, unsigned int yIndex) const = 0;

      virtual void cleanUpItems(std::vector<QuadTreeItem*>& items) const final
      {
        items.erase(std::remove_if(items.begin(), items.end(),
        [&](QuadTreeItem * hit) {return hit->isUsed();}),
        items.end());
      };

      // version with a threshold on y
      virtual void fillGivenTree(QuadTree* node, CandidateProcessorLambda& lmdProcessor,
                                 unsigned int nHitsThreshold, typeY rThreshold) const final
      {
        fillGivenTree(node, lmdProcessor, nHitsThreshold, rThreshold, true);
      }

      // version without a threshold on y
      virtual void fillGivenTree(QuadTree* node, CandidateProcessorLambda& lmdProcessor,
                                 unsigned int nHitsThreshold) const final
      {
        fillGivenTree(node, lmdProcessor, nHitsThreshold, static_cast<typeY>(0), false);
      }

    private:

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
          lmdProcessor(node);
          return;
        }
        // **** ???? *****

        if (node->getChildren() == nullptr)
          node->initializeChildren(*this);

        if (!node->checkFilled()) {
          fillChildren(node, node->getItemsVector());
          node->setFilled();
        }

        int m_nbins_theta = node->getXNbins();
        int m_nbins_r = node->getYNbins();

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

    public:
      virtual void fillChildren(QuadTree* node, std::vector<QuadTreeItem*>& items) const final
      {
        const size_t neededSize = 2 * items.size();
        node->getChildren()->apply([neededSize](QuadTree * qt) {qt->reserveHitsVector(neededSize);});

        //Voting within the four bins
        for (QuadTreeItem* item : items) {
          if (item->isUsed())
            continue;

          for (int t_index = 0; t_index < binCountX; ++t_index) {
            for (int r_index = 0; r_index < binCountY; ++r_index) {
              if (insertItemInNode(node->getChildren()->get(t_index, r_index), item->getPointer(), t_index, r_index)) {
                B2DEBUG(110, "Inserting new item in node (" << t_index << ", " << r_index << ")")
                node->getChildren()->get(t_index, r_index)->insertItem(item);
              }
            }
          }
        }
      }

      virtual void initializeChildren(QuadTree* node, QuadTreeChildren* m_children) const final
      {
        for (unsigned int i = 0; i < binCountX; ++i) {
          for (unsigned int j = 0; j < binCountY; ++j) {
            const ChildRanges& childRanges = createChildWithParent(node, i, j);
            const rangeX& rangeX = childRanges.first;
            const rangeY& rangeY = childRanges.second;

            B2DEBUG(110, "Creating new child on level " << static_cast<unsigned int>(node->getLevel()) << " with ranges ("
                    << rangeX.first << ", " << rangeX.second << ") - (" << rangeY.first << ", " << rangeY.second << ")")
            m_children->set(i, j, new QuadTree(rangeX.first, rangeX.second, rangeY.first, rangeY.second, node->getLevel() + 1, node));
          }
        }
      }


      virtual void provideItemsSet(const std::set<QuadTreeItem*>& items_set, std::vector<QuadTreeItem*>& items_vector) const final
      {
        items_vector.clear();
        items_vector.reserve(items_set.size());
        std::copy_if(items_set.begin(), items_set.end(), std::back_inserter(items_vector),
        [&](QuadTreeItem * item) {return not item->isUsed();});
      };

      virtual unsigned int getLastLevel() const final
      {
        return m_lastLevel;
      }

    private:
      unsigned int m_lastLevel;
    };
  }
}
