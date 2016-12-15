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
 * Possible runtime improvements:
 * - use a specific allocator for the QuadTrees which can be wiped without calling
 *   each QuadTree constructor once the search is complete.
 */


#pragma once

#include <framework/logging/Logger.h>
#include <tracking/trackFindingCDC/legendre/quadtree/TrigonometricalLookupTable.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeChildren.h>

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

    /**
     * Class which holds quadtree structure.
     * @tparam typeX type of the X-axis variable
     * @tparam typeY type of the Y-axis variable
     * @tparam typeData type of the objects which are filled into QuadTree
     * @tparam binCountX number of divisions of the node along X axis (default is 2)
     * @tparam binCountY number of divisions of the node along Y axis (default is 2)
     */
    template<typename typeX, typename typeY, class typeData, int binCountX = 2, int binCountY = 2>
    class QuadTreeTemplate {

    public:

      /// Type of this class
      using ThisType = QuadTreeTemplate<typeX, typeY, typeData, binCountX, binCountY>;

      /**
       * a lambda expression with this signature is used to process each of the
       * found candidates during the startFillingTree call
       */
      using CandidateProcessorLambda = std::function< void(ThisType*) >;

      /// store the minimum, center and maximum of this bin in X direction (in this order in the tuple)
      using XBinTuple = std::array<typeX, 3>;

      /// store the minimum, center and maximum of this bin in Y direction (in this order in the tuple)
      using YBinTuple = std::array<typeY, 3>;

      /// Vector of the sub node QuadTree pointers
      using NodeList = std::vector<ThisType*>;

      /// type of the child nodes hold by this node
      using Children = QuadChildrenTemplate<ThisType>;

      /// Constructor
      QuadTreeTemplate() : m_parent(nullptr), m_level(0), m_filled(false)
      {
        // initialize all bin settings to zero
        for (int i = 0; i < 3; i++) {
          m_xBins[i] = 0;
          m_yBins[i] = 0;
        }
      }

      /// Constructor
      QuadTreeTemplate(typeX xMin, typeX xMax, typeY yMin, typeY yMax, unsigned char level, ThisType* parent) :
        m_level(level)
      {
        // ensure the level value fits into unsigned char
        B2ASSERT("QuadTree datastructure only supports levels < 255", level < 255);
        m_filled = false;
        //m_neighborsDefined = false;

        if (m_level > 0) {
          m_parent = parent;
        } else {
          m_parent = nullptr;
        }

        computeBins(xMin, xMax, yMin, yMax);
      }

      /// Set parameters of the QuadTree
      void setParameters(typeX xMin, typeX xMax, typeY yMin, typeY yMax, int level, ThisType* parent)
      {
        m_level = level;
        m_filled = false;

        if (m_level > 0) {
          m_parent = parent;
        } else {
          m_parent = nullptr;
        }

        computeBins(xMin, xMax, yMin, yMax);
      }

      /// This method should be called every time quadtree runs out of the scope/cleaned up
      void terminate()
      {
        clearTree();
      }

      /** Create vector with children of current node */
      template<class processor>
      void initializeChildren(processor const& proc)
      {
        m_children.reset(new Children());

        proc.initializeChildren(this, m_children.get());

      }

      /** Build neighborhood for leafs */
      void buildNeighborhood(int levelNeighborhood)
      {
        if (m_level < 13) {
          applyToChildren([levelNeighborhood](ThisType * qt) { qt->buildNeighborhood(levelNeighborhood);});
        }

        if (m_level > 3) {
          this->findNeighbors();
        }
      }

      /** Returns level of the node in tree (i.e., how much ancestors the node has) */
      Children* getChildren() const
      {
        return m_children.get();
      };

      /** Returns level of the node in tree (i.e., how much ancestors the node has) */
      unsigned char getLevel() const
      {
        return m_level;
      };

      /** Copy information about hits into member of class (node at level 0 should be used  because other levels fills by parents) */
      template<class processor>
      void provideItemsSet(processor const& proc, const std::set<typeData*>& hits_set)
      {
        proc.provideItemsSet(hits_set, m_items);
      }

      /** Forced filling of tree, skipping limitation on number of hits
       * Filling nodes which are parents to the current one
       */
      template<class processor>
      void fillChildrenForced(processor const& proc)
      {
        if (!m_parent->checkFilled()) {
          m_parent->fillChildrenForced<processor>(proc);
          proc.fillChildren(this, m_items);
          setFilled();
        }
      }

      /** Insert hit into node */
      void insertItem(typeData* hit) {m_items.push_back(hit); };

      /** Reserve memory for holding hits */
      void reserveHitsVector(int nHits) {m_items.reserve(nHits); };

      /** Get hits from node */
      std::vector<typeData*>& getItemsVector() {return m_items;};

      /** Check if the node passes threshold on number of hits */
      size_t getNItems() const {return m_items.size();};

      /** Removing used or bad hits */
      template<class processor>
      void cleanUpItems(processor const& proc)
      {
        B2ASSERT("Deleting items on a the top level will lead to memory leaks! If you want to safely delete all items, use the methods provided by the processor.",
                 m_level != 0);
        proc.cleanUpItems(m_items);
      } ;

      /** Check whether node has been processed, i.e. children nodes has been filled */
      bool checkFilled() const {return m_filled; };

      /** Set status of node to "filled" (children nodes has been filled) */
      void setFilled() {m_filled = true; };

      /** Get mean value of theta */
      typeY getYMean() const {return m_yBins[1];};

      /** Get mean value of r */
      typeX getXMean() const {return m_xBins[1];};

      /** Get number of bins in "r" direction */
      constexpr int getXNbins() const {return binCountX;}

      /** Get minimal "r" value of the node */
      typeX getXMin() const {return m_xBins[0];};

      /** Get maximal "r" value of the node */
      typeX getXMax() const {return m_xBins[2];};

      /** Get "r" value of given bin border */
      typeX getXBin(int bin) const { return static_cast<typeX>(m_xBins[bin]); };

      /** Get number of bins in "Theta" direction */
      constexpr int getYNbins() const {return binCountY;}

      /** Get minimal "Theta" value of the node */
      typeY getYMin() const {return m_yBins[0];};

      /** Get maximal "Theta" value of the node */
      typeY getYMax() const {return m_yBins[2];};

      /** Get "r" value of given bin border */
      typeY getYBin(int bin) const { return static_cast<typeY>(m_yBins[bin]); };

      /** Return pointer to the parent of the node */
      ThisType* getParent() const {return m_parent;};

      /** Clear hits which the node holds */
      void clearNode() {m_items.clear(); };

      /** Clear hits which the node holds and destroy all children below this node.
       * This method must only be called on the root node, for fast QuadTreeTemplate reusage */
      void clearTree()
      {
        // can only be called on root item
        B2ASSERT("ClearTree can only be called on tree of level one", m_level == 0);

        // automatically removes all underlying objects
        m_children.reset(nullptr);
        m_filled = false;
      };

    private:

      /// sets the x and y bin values and computes the bin centers
      void computeBins(typeX xMin, typeX xMax, typeY yMin, typeY yMax)
      {
        m_xBins[0] = xMin;
        m_xBins[1] = static_cast<typeX>(xMin + 0.5 * (xMax - xMin));
        m_xBins[2] = xMax;

        m_yBins[0] = yMin;
        m_yBins[1] = static_cast<typeY>(yMin + 0.5 * (yMax - yMin));
        m_yBins[2] = yMax;
      }

      /** apply a lambda expression to all children of this tree node */
      void applyToChildren(std::function<void(ThisType*)> lmd)
      {
        if (!m_children) {
          return;
        }
        m_children->apply(lmd);
      };

      std::vector<typeData*> m_items;  /**< Vector of hits which belongs to the node */

      ThisType* m_parent;  /**< Pointer to the parent node */

      /** Pointers to the children nodes */
      std::unique_ptr< Children > m_children;

      XBinTuple m_xBins;          /**< bins range on r */
      YBinTuple m_yBins;      /**< bins range on theta */
      unsigned char m_level;  /**< Level of node in the tree */
      bool m_filled; /**< Is the node has been filled with hits */
    };
  }
}
