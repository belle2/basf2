/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <array>
#include <iterator>
#include <functional>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Class which holds pointer to the children nodes of QuadTree
     * @tparam ANode type of the QuadTree
     */
    template<typename ANode>
    class QuadTreeChildren {
    public:
      /// Number of children in X direction
      static constexpr const size_t c_nXBins = 2;

      /// Number of children in Y direction
      static constexpr const size_t c_nYBins = 2;

      /// Iterator type to iterate over the child node
      using iterator = typename std::array<ANode*, c_nYBins* c_nXBins>::const_iterator;

      /// Constructor
      QuadTreeChildren()
        : m_children{} // initialize to nullptr
      {
      }

      /// Destructor destroying the owned pointers
      ~QuadTreeChildren()
      {
        for (ANode* node : m_children) {
          delete node;
        }
      }

      /// Begin iterator over the children for the range based for loop
      iterator begin() const
      {
        return std::begin(m_children);
      }

      /// End iterator over the children for the range based for loop
      iterator end() const
      {
        return std::end(m_children);
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
        delete m_children[iXBin * c_nYBins + iYBin];

        // Reassign it
        m_children[iXBin * c_nYBins + iYBin] = qt;
      }

      /// Get pointer to the child with indexes iXBin, iYBin
      ANode* get(size_t iXBin, size_t iYBin)
      {
        return m_children[iXBin * c_nYBins + iYBin];
      }

    private:
      /// Array for the owned children
      std::array<ANode*, c_nYBins* c_nXBins> m_children;
    };
  }
}
