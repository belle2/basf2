/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

/*
 * Object which can store pointers to hits while processing FastHogh algorithm
 *
 * TODO: check if it's possible to store in each hit list of nodes in which we can meet it.
 *
 */


#pragma once

#include "tracking/trackFindingCDC/legendre/quadtree/TrigonometricalLookupTable.h"

#include <tracking/trackFindingCDC/legendre/CDCLegendreFastHough.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreConformalPosition.h>

#include <framework/logging/Logger.h>

#include <boost/math/constants/constants.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>
#include <boost/multi_array.hpp>

#include <set>
#include <vector>
#include <array>
#include <algorithm>

#include <cstdlib>
#include <iomanip>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cmath>
#include <functional>

namespace Belle2 {
  namespace TrackFindingCDC {

    class QuadTreeNeighborFinder;


    template<typename typeX, typename typeY, class typeData>
    class QuadTreeTemplate;

    template<typename typeX, typename typeY, class typeData>
    class QuadChildrenTemplate {
    public:
      QuadChildrenTemplate() {
        // initialize to null
        for (size_t t_index = 0; t_index < m_sizeX; ++t_index) {
          for (size_t r_index = 0; r_index < m_sizeY; ++r_index) {
            m_children[t_index][r_index] = nullptr;
          }
        }
      };

      void apply(std::function<void(QuadTreeTemplate<typeX, typeY, typeData>*)> lmd) {
        for (size_t t_index = 0; t_index < m_sizeX; ++t_index) {
          for (size_t r_index = 0; r_index < m_sizeY; ++r_index) {
            if (m_children[t_index][r_index]) {
              lmd(m_children[t_index][r_index]);
            }
          }
        }
      };

      ~QuadChildrenTemplate() {
        this->apply([](QuadTreeTemplate<typeX, typeY, typeData>* qt) { delete qt;});
      };

      inline void set(const size_t x, const size_t y, QuadTreeTemplate<typeX, typeY, typeData>* qt) {
        m_children[x][y] = qt;
      }

      inline QuadTreeTemplate<typeX, typeY, typeData>* get(const size_t x, const size_t y) {
        return m_children[x][y];
      }
      static constexpr size_t m_sizeX = 2;
      static constexpr size_t m_sizeY = 2;

    private:

      QuadTreeTemplate<typeX, typeY, typeData>* m_children[m_sizeX][m_sizeY];
    };


//    class QuadTreeProcessor;

    template<typename typeX, typename typeY, class typeData>
    class QuadTreeTemplate {

    public:

      // a lambda expression with this signature is used to process each of the
      // found candidates during the startFillingTree call
      typedef std::function< void(QuadTreeTemplate<typeX, typeY, typeData>*) > CandidateProcessorLambda;

      typedef std::array<typeX, 3> XBinTuple;
      typedef std::array<typeY, 3> YBinTuple;
      typedef std::vector<QuadTreeTemplate<typeX, typeY, typeData>*> NodeList;
      //typedef boost::multi_array< QuadTreeTemplate *, 2> ChildrenArray;

      QuadTreeTemplate() : m_xMin(0), m_xMax(0), m_yMin(0), m_yMax(0), m_level(0) {;};

      QuadTreeTemplate(typeX xMin, typeX xMax, typeY yMin, typeY yMax, int level, QuadTreeTemplate<typeX, typeY, typeData>* parent) :
        m_xMin(xMin), m_xMax(xMax), m_yMin(yMin), m_yMax(yMax), m_level(level) {
        m_filled = false;
        m_neighborsDefined = false;

        if (m_level > 0) {
          m_parent = parent;
        } else {
          m_parent = nullptr;
        }

        m_nBinsX = 2;
        m_nBinsY = 2;

        m_xBins[0] = m_xMin;
        m_xBins[1] = static_cast<typeX>(m_xMin + 0.5 * (m_xMax - m_xMin));
        m_xBins[2] = m_xMax;

        m_yBins[0] = m_yMin;
        m_yBins[1] = static_cast<typeY>(m_yMin + 0.5 * (m_yMax - m_yMin));
        m_yBins[2] = m_yMax;
      };

      void setParameters(typeX xMin, typeX xMax, typeY yMin, typeY yMax, int level, QuadTreeTemplate<typeX, typeY, typeData>* parent) {
        m_xMin = xMin;
        m_xMax = xMax;
        m_yMin = yMin;
        m_yMax = yMax;
        m_level = level;

        if (m_level > 0) {
          m_parent = parent;
        } else {
          m_parent = nullptr;
        }

        m_nBinsX = 2;
        m_nBinsY = 2;

        m_xBins[0] = m_xMin;
        m_xBins[1] = static_cast<typeX>(m_xMin + 0.5 * (m_xMax - m_xMin));
        m_xBins[2] = m_xMax;

        m_yBins[0] = m_yMin;
        m_yBins[1] = static_cast<typeY>(m_yMin + 0.5 * (m_yMax - m_yMin));
        m_yBins[2] = m_yMax;
      }

      void terminate() { clearTree();} ;

      /** Create vector with children of current node */
      template<class processor>
      void initializeChildren() {
        m_children.reset(new QuadChildrenTemplate<typeX, typeY, typeData>());

        processor::initializeChildren(this, m_children.get());

      } ;

      /** Build neighborhood for leafs */
      void buildNeighborhood(int levelNeighborhood) {
        if (m_level < 13) {
          applyToChildren([levelNeighborhood](QuadTreeTemplate<typeX, typeY, typeData>* qt) { qt->buildNeighborhood(levelNeighborhood);});
        }

        if (m_level > 3)
          this->findNeighbors();
      };

      /** Returns level of the node in tree (i.e., how much ancestors the node has) */
      inline QuadChildrenTemplate<typeX, typeY, typeData>* getChildren() const {return m_children.get();};

      /** Returns level of the node in tree (i.e., how much ancestors the node has) */
      inline int getLevel() const {return m_level;};

      /** Set threshold on number of items in the node */
      void setNItemsThreshold(unsigned int hitsThreshold) {m_itemsThreshold = hitsThreshold;};

      /** Get threshold on number of items in the node */
      inline unsigned int getNItemsThreshold() const {return m_itemsThreshold;};

      /** Sets threshold on pt of candidates */
      void setLastLevel(double lastLevel) {m_lastLevel = static_cast<float>(lastLevel);};

      /** Sets threshold on pt of candidates */
      int getLastLevel() const {return m_lastLevel;};

      /** Copy information about hits into member of class (node at level 0 should be used  because other levels fills by parents) */
      template<class processor>
      void provideItemsSet(const std::set<typeData*>& hits_set) {
        processor::provideItemsSet(hits_set, m_items);
      };

      // legacy interface, remove
      /*      void startFillingTree() {
              CandidateProcessorLambda lmd = [](QuadTreeTemplate*)->void {};
              startFillingTree(lmd);
            }*/
      /** legacy interface, remove */
      /*      void startFillingTree(bool returnResult, std::vector<QuadTreeTemplate*>& nodeList) {
              CandidateProcessorLambda lmd = [](QuadTreeTemplate*)->void {};
              startFillingTree(returnResult, nodeList, lmd);
            }*/

      /** Fill the tree structure */
      template<class processor>
      void startFillingTree(CandidateProcessorLambda& lmdProcessor) {
        processor::fillTree(this, lmdProcessor);
      } ;

      /** Forced filling of tree, skipping limitation on number of hits
       * Filling nodes which are parents to the current one
       */
      template<class processor>
      void fillChildrenForced() {
        if (!m_parent->checkFilled()) {
          m_parent->fillChildrenForced<processor>();
          processor::fillChildren(m_children.get(), m_items);
          setFilled();
        }
      } ;

      /** Insert hit into node */
      void insertItem(typeData* hit) {m_items.push_back(hit); };

      /** Reserve memory for holding hits */
      void reserveHitsVector(int nHits) {m_items.reserve(nHits); };

      /** Check if the node passes threshold on number of hits */
      bool checkNode() const {return m_items.size() >= m_itemsThreshold;};

      /** Get hits from node */
      inline std::vector<typeData*>& getItemsVector() {return m_items;};

      /** Check if the node passes threshold on number of hits */
      inline int getNItems() const {return m_items.size();};

      /** Removing used or bad hits */
      template<class processor>
      void cleanUpItems() {
        processor::cleanUpItems(m_items);
      } ;

      //TODO: IMPORTANT! check functionality of this method and impact of uncommenting _m_level_ ! */
      /** Check whether node is leaf (lowest node in the tree) */
      bool isLeaf() const {return m_level >= m_lastLevel;};

      /** Check whether node has been processed, i.e. children nodes has been filled */
      inline bool checkFilled() const {return m_filled; };

      /** Set status of node to "filled" (children nodes has been filled) */
      void setFilled() {m_filled = true; };

      /** Get mean value of theta */
      inline typeY getYMean() const {return static_cast<typeY>((m_yMin + m_yMax) / 2. /* boost::math::constants::pi<double>() / TrigonometricalLookupTable::Instance().getNBinsTheta()*/);};

      /** Get mean value of r */
      inline typeX getXMean() const {return static_cast<typeX>((m_xMin + m_xMax) / 2.);};

      /** Get number of bins in "r" direction */
      inline int getXNbins() const {return m_nBinsX;};

      /** Get minimal "r" value of the node */
      inline typeX getXMin() const {return static_cast<typeX>(m_xMin);};

      /** Get maximal "r" value of the node */
      inline typeX getXMax() const {return static_cast<typeX>(m_xMax);};

      /** Get "r" value of given bin border */
      inline typeX getXBin(unsigned int bin) const { return static_cast<typeX>(m_xBins[bin]); };

      /** Get number of bins in "Theta" direction */
      inline int getYNbins() const {return m_nBinsY;};

      /** Get minimal "Theta" value of the node */
      inline typeY getYMin() const {return m_yMin;};

      /** Get maximal "Theta" value of the node */
      inline typeY getYMax() const {return m_yMax;};

      /** Get "r" value of given bin border */
      inline typeX getYBin(unsigned int bin) const { return static_cast<typeX>(m_yBins[bin]); };

      /** Return pointer to the parent of the node */
      inline QuadTreeTemplate<typeX, typeY, typeData>* getParent() const {return m_parent;};

      /** Add pointer to some node to list of neighbors of current node */
      void addNeighbor(QuadTreeTemplate<typeX, typeY, typeData>* node) {m_neighbors.push_back(node);};

      /** Get number of neighbors of the current node (used mostly for debugging purposes) */
      int getNneighbors() const {return m_neighbors.size();};

      /** Check whether neighbors of the node has been found */
      inline bool isNeighborsDefined() const {return m_neighborsDefined;};

      /** Find and store neighbors of the node */
      void findNeighbors() {
        if (not m_neighborsDefined) {
          //    QuadTreeNeighborFinder::Instance().controller(this, this, m_parent);
          m_neighborsDefined = true;
        }
        //  B2INFO("Number of neighbors: " << m_neighbors.size());

        //  B2INFO("Number of hits in node: " << m_items.size());
        /*  for(CDCLegendreQuadTree* node: m_neighbors){
         if(node->getNItems() == 0) node->fillChildrenForced();
         node->cleanUpItems();
         */
        //    B2INFO("Number of hits in neighbor: " << node->getNItems());
        /*    if(node->checkNode()){
         for(CDCLegendreTrackHit *hit: node->getItemsVector()){
         m_items.push_back(hit);
         }
         */
        //  }
      };

      /** Get list of neighbors of the current node */
      std::vector<QuadTreeTemplate<typeX, typeY, typeData>*>& getNeighborsVector() {
        if (not m_neighborsDefined) {
          B2WARNING(
            "Trying to get neighbors of the node which are not defined. Starting neighbors finding");
          //    findNeighbors();
        }
        return m_neighbors;
      }

      /** Clear hits which the node holds */
      void clearNode() {m_items.clear(); };

      /** Clear hits which the node holds and destroy all children below this node.
       * This method must only be called on the root node, for fast QuadTreeTemplate reusage */
      void clearTree() {
        // can only be called on root item
        assert(m_level == 0);

        // automatically removes all underlying objects
        m_children.reset(nullptr);
        m_filled = false;
      };

    private:

      /** apply a lambda expression to all children of this tree node */
      void applyToChildren(std::function<void(QuadTreeTemplate<typeX, typeY, typeData>*)> lmd) {
        if (!m_children)
          return;
        m_children->apply(lmd);
      };

      /** Check if we reach limitation on dr and dtheta; returns true when reached limit */
      bool checkLimitsR() {
        if (m_level == 13)
          return true;

        //  if (fabs(m_rMax - m_rMin) < m_deltaR) {
        //    return true;
        //  }

        return false;
      };

      unsigned int m_itemsThreshold;
      int m_lastLevel;

      typeX m_xMin; /**< Lower border of the node (R) */
      typeX m_xMax; /**< Upper border of the node (R) */
      typeY m_yMin;  /**< Lower border of the node (Theta) */
      typeY m_yMax;  /**< Upper border of the node (Theta) */
      int m_level;  /**< Level of node in the tree */

      std::vector<typeData*> m_items;  /**< Vector of hits which belongs to the node */

      QuadTreeTemplate<typeX, typeY, typeData>* m_parent;  /**< Pointer to the parent node */

      std::vector<QuadTreeTemplate<typeX, typeY, typeData>*> m_neighbors; /**< 8 neighbours of each node (or 5 at borders) */

      /** Pointers to the children nodes */
      std::unique_ptr< QuadChildrenTemplate<typeX, typeY, typeData> > m_children;

      XBinTuple m_xBins;          /**< bins range on r */
      YBinTuple m_yBins;      /**< bins range on theta */
      int m_nBinsX;        /**< number of r bins */
      int m_nBinsY;    /**< number of theta bins */
      bool m_filled; /**< Is the node has been filled with hits */
      bool m_neighborsDefined; /**< Checks whether neighbors of current node has been defined */

    };

  }
}
