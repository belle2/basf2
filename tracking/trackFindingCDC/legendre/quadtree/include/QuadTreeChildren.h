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

#include <functional>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Class which holds pointer to the children nodes of QuadTree
     * @tparam ANode type of the QuadTree
     */
    template<typename ANode>
    class QuadChildrenTemplate {
    public:
      /// Number of children in X direction
      static constexpr const size_t c_nXBins = 2;

      /// Number of children in Y direction
      static constexpr const size_t c_nYBins = 2;

      /// Constructor
      QuadChildrenTemplate()
      {
        // initialize to null
        for (size_t iXBin = 0; iXBin < c_nXBins; ++iXBin) {
          for (size_t iYBin = 0; iYBin < c_nYBins; ++iYBin) {
            m_children[iXBin][iYBin] = nullptr;
          }
        }
      }

      /// Destructor destroying the owned pointers
      ~QuadChildrenTemplate()
      {
        for (size_t iXBin = 0; iXBin < c_nXBins; ++iXBin) {
          for (size_t iYBin = 0; iYBin < c_nYBins; ++iYBin) {
            delete m_children[iXBin][iYBin];
          }
        }
      }

      /// This method will apply a lambda function to every child of the quad tree
      void apply(std::function<void(ANode*)> lmd)
      {
        for (size_t iXBin = 0; iXBin < c_nXBins; ++iXBin) {
          for (size_t iYBin = 0; iYBin < c_nYBins; ++iYBin) {
            lmd(m_children[iXBin][iYBin]);
          }
        }
      }

      /**
       * Set child with indexes iXBin, iYBin
       *
       * Takes ownership of the node.
       *
       * @param qt pointer to QuadTree child to set
       */
      void set(size_t iXBin, size_t iYBin, ANode* qt)
      {
        // Delete owned node
        delete m_children[iXBin][iYBin];

        // Reassign it
        m_children[iXBin][iYBin] = qt;
      }

      /// Get pointer to the child with indexes iXBin, iYBin
      ANode* get(size_t iXBin, size_t iYBin)
      {
        return m_children[iXBin][iYBin];
      }

    private:
      /// Array of children
      ANode* m_children[c_nXBins][c_nYBins];
    };
  }
}
