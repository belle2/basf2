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

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeChildren.h>

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

#include <framework/logging/Logger.h>

#include <array>
#include <vector>
#include <memory>
#include <functional>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Class which holds quadtree structure.
     * @tparam AX type of the X-axis variable
     * @tparam AY type of the Y-axis variable
     * @tparam AItem type of the objects which are filled into QuadTree
     */
    template<typename AX, typename AY, class AItem>
    class QuadTreeTemplate {

    public:
      /// Type of this class
      using ThisType = QuadTreeTemplate<AX, AY, AItem>;

      /// store the minimum, center and maximum of this bin in X direction (in this order in the tuple)
      using XBinTuple = std::array<AX, 3>;

      /// store the minimum, center and maximum of this bin in Y direction (in this order in the tuple)
      using YBinTuple = std::array<AY, 3>;

      /// type of the child nodes hold by this node
      using Children = QuadChildrenTemplate<ThisType>;

      /// Constructor
      QuadTreeTemplate()
        : m_level(0)
        , m_parent(nullptr)
        , m_filled(false)
      {
        m_xBins.fill(0);
        m_yBins.fill(0);
      }

      /// Constructor
      QuadTreeTemplate(AX xMin, AX xMax, AY yMin, AY yMax, int level, ThisType* parent)
        : m_level(level)
        , m_parent(level > 0 ? parent : nullptr)
        , m_filled(false)
      {
        // ensure the level value fits into unsigned char
        B2ASSERT("QuadTree datastructure only supports levels < 255", level < 255);
        computeBins(xMin, xMax, yMin, yMax);
      }

      /** Insert item into node */
      void insertItem(AItem* item)
      {
        m_items.push_back(item);
      };

      /** Reserve memory for holding items */
      void reserveItems(int nItems)
      {
        m_items.reserve(nItems);
      };

      /** Get items from node */
      std::vector<AItem*>& getItems()
      {
        return m_items;
      };

      /** Check if the node passes threshold on number of hits */
      size_t getNItems() const
      {
        return m_items.size();
      };

      /** Clear items which the node holds */
      void clearItems()
      {
        m_items.clear();
      };

      /** Create vector with children of current node */
      void createChildren()
      {
        m_children = makeUnique<Children>();
      }

      /** Returns the children structure of this node */
      Children* getChildren() const
      {
        return m_children.get();
      };

      /**
       *  Clear items which the node holds and destroy all children below this node.
       *  This method must only be called on the root node, for fast QuadTreeTemplate reusage
       */
      void clearChildren()
      {
        // can only be called on root item
        B2ASSERT("ClearTree can only be called on tree of level one", m_level == 0);

        // automatically removes all underlying objects
        m_children.reset(nullptr);
        m_filled = false;
      };

      /** Returns level of the node in tree (i.e., how much ancestors the node has) */
      unsigned char getLevel() const
      {
        return m_level;
      };

      /** Check whether node has been processed, i.e. children nodes has been filled */
      bool checkFilled() const
      {
        return m_filled;
      };

      /** Set status of node to "filled" (children nodes has been filled) */
      void setFilled()
      {
        m_filled = true;
      };

      /** Return pointer to the parent of the node */
      ThisType* getParent() const
      {
        return m_parent;
      };

      /** Get number of bins in "r" direction */
      constexpr int getXNbins() const
      {
        return m_xBins.size() - 1;
      }

      /** Get minimal "r" value of the node */
      AX getXMin() const
      {
        return m_xBins[0];
      };

      /** Get mean value of r */
      AX getXMean() const
      {
        return m_xBins[1];
      };

      /** Get maximal "r" value of the node */
      AX getXMax() const
      {
        return m_xBins[2];
      };

      /** Get "r" value of given bin border */
      AX getXBin(int bin) const
      {
        return m_xBins[bin];
      };

      /** Get number of bins in "Theta" direction */
      constexpr int getYNbins() const
      {
        return m_yBins.size() - 1;
      }

      /** Get minimal "Theta" value of the node */
      AY getYMin() const
      {
        return m_yBins[0];
      };

      /** Get mean value of theta */
      AY getYMean() const
      {
        return m_yBins[1];
      };

      /** Get maximal "Theta" value of the node */
      AY getYMax() const
      {
        return m_yBins[2];
      };

      /** Get "r" value of given bin border */
      AY getYBin(int bin) const
      {
        return m_yBins[bin];
      };

    private:
      /// sets the x and y bin values and computes the bin centers
      void computeBins(AX xMin, AX xMax, AY yMin, AY yMax)
      {
        m_xBins[0] = xMin;
        m_xBins[1] = xMin + (xMax - xMin) / 2;
        m_xBins[2] = xMax;

        m_yBins[0] = yMin;
        m_yBins[1] = yMin + (yMax - yMin) / 2;
        m_yBins[2] = yMax;
      }

    private:
      /// bins range on r
      XBinTuple m_xBins;

      /// bins range on theta
      YBinTuple m_yBins;

      /// Level of node in the tree
      unsigned char m_level;

      /// Pointer to the parent node
      ThisType* m_parent;

      /// Vector of items which belongs to the node
      std::vector<AItem*> m_items;

      /// Pointers to the children nodes
      std::unique_ptr<Children> m_children;

      /// Is the node has been filled with items
      bool m_filled;
    };
  }
}
