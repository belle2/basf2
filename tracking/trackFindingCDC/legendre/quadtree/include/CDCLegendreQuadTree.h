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
#include <tracking/trackFindingCDC/legendre/TrackHit.h>

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

    template<typename quadChild>
    class QuadChildrenTemplate {
    public:

      QuadChildrenTemplate()
      {
        // initialize to null
        for (size_t t_index = 0; t_index < m_sizeX; ++t_index) {
          for (size_t r_index = 0; r_index < m_sizeY; ++r_index) {
            m_children[t_index][r_index] = nullptr;
          }
        }
      };

      void apply(std::function<void(quadChild*)> lmd)
      {
        for (size_t t_index = 0; t_index < m_sizeX; ++t_index) {
          for (size_t r_index = 0; r_index < m_sizeY; ++r_index) {
            if (m_children[t_index][r_index]) {
              lmd(m_children[t_index][r_index]);
            }
          }
        }
      };

      ~QuadChildrenTemplate()
      {
        this->apply([](quadChild * qt) { delete qt;});
      };

      inline void set(const size_t x, const size_t y, quadChild* qt)
      {
        m_children[x][y] = qt;
      }

      inline quadChild* get(const size_t x, const size_t y)
      {
        return m_children[x][y];
      }
      static constexpr size_t m_sizeX = 2;
      static constexpr size_t m_sizeY = 2;

    private:

      quadChild* m_children[m_sizeX][m_sizeY];
    };


    template<typename typeX, typename typeY, class typeData, int binCountX = 2, int binCountY = 2>
    class QuadTreeTemplate {

    public:
      typedef QuadTreeTemplate<typeX, typeY, typeData, binCountX, binCountY> ThisType;

      // a lambda expression with this signature is used to process each of the
      // found candidates during the startFillingTree call
      typedef std::function< void(ThisType*) > CandidateProcessorLambda;

      // store the minimum, center and maximum of this bin (in this order in the tuple)
      typedef std::array<typeX, 3> XBinTuple;
      typedef std::array<typeY, 3> YBinTuple;
      typedef std::vector<ThisType*> NodeList;

      // type of the child nodes hold by this node
      typedef QuadChildrenTemplate<ThisType> Children;

      QuadTreeTemplate() : m_parent(nullptr), m_level(0), m_filled(false)
      {
        // initialize all bin settings to zero
        for (int i = 0; i < 3; i++) {
          m_xBins[i] = 0;
          m_yBins[i] = 0;
        }
      }

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

      void setParameters(typeX xMin, typeX xMax, typeY yMin, typeY yMax, int level, ThisType* parent)
      {
        m_level = level;

        if (m_level > 0) {
          m_parent = parent;
        } else {
          m_parent = nullptr;
        }

        computeBins(xMin, xMax, yMin, yMax);
      }

      void terminate() { clearTree();}

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

        if (m_level > 3)
          this->findNeighbors();
      }

      /** Returns level of the node in tree (i.e., how much ancestors the node has) */
      inline Children* getChildren() const {return m_children.get();};

      /** Returns level of the node in tree (i.e., how much ancestors the node has) */
      inline unsigned char getLevel() const {return m_level;};

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
      inline std::vector<typeData*>& getItemsVector() {return m_items;};

      /** Check if the node passes threshold on number of hits */
      inline size_t getNItems() const {return m_items.size();};

      /** Removing used or bad hits */
      template<class processor>
      void cleanUpItems(processor const& proc)
      {
        proc.cleanUpItems(m_items);
      } ;

      /** Check whether node has been processed, i.e. children nodes has been filled */
      inline bool checkFilled() const {return m_filled; };

      /** Set status of node to "filled" (children nodes has been filled) */
      void setFilled() {m_filled = true; };

      /** Get mean value of theta */
      inline typeY getYMean() const {return m_yBins[1];};

      /** Get mean value of r */
      inline typeX getXMean() const {return m_xBins[1];};

      /** Get number of bins in "r" direction */
      inline constexpr int getXNbins() const {return binCountX;}

      /** Get minimal "r" value of the node */
      inline typeX getXMin() const {return m_xBins[0];};

      /** Get maximal "r" value of the node */
      inline typeX getXMax() const {return m_xBins[2];};

      /** Get "r" value of given bin border */
      inline typeX getXBin(int bin) const { return static_cast<typeX>(m_xBins[bin]); };

      /** Get number of bins in "Theta" direction */
      inline constexpr int getYNbins() const {return binCountY;}

      /** Get minimal "Theta" value of the node */
      inline typeY getYMin() const {return m_yBins[0];};

      /** Get maximal "Theta" value of the node */
      inline typeY getYMax() const {return m_yBins[2];};

      /** Get "r" value of given bin border */
      inline typeY getYBin(int bin) const { return static_cast<typeY>(m_yBins[bin]); };

      /** Return pointer to the parent of the node */
      inline QuadTreeTemplate<typeX, typeY, typeData>* getParent() const {return m_parent;};

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

      // sets the x and y bin values and computes the bin centers
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
      void applyToChildren(std::function<void(QuadTreeTemplate<typeX, typeY, typeData>*)> lmd)
      {
        if (!m_children)
          return;
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


    // a special instance
    typedef TrackFindingCDC::QuadTreeTemplate<int, float, TrackFindingCDC::TrackHit> QuadTreeLegendre;
  }
}
