/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;

    /// An aggregation of CDCWireHits.
    class CDCWireHitCluster : public std::vector<CDCWireHit*> {

      /// Type of the base class
      using Super = std::vector<CDCWireHit*>;

    public:
      /// Default constructor
      CDCWireHitCluster() = default;

      /// Default copy constructor
      CDCWireHitCluster(const CDCWireHitCluster& wireHitCluster) = default;

      /// Constructor from a bunch of wire hits
      // cppcheck-suppress passedByValue
      explicit CDCWireHitCluster(std::vector<CDCWireHit*> wireHits)
        : Super(std::move(wireHits))
      {
      }

      /// Get the super cluster id of this cluster
      int getISuperCluster() const
      {
        return m_iSuperCluster;
      }

      /// Set the super cluster id of this cluster
      void setISuperCluster(int iSuperCluster)
      {
        m_iSuperCluster = iSuperCluster;
      }

      /// Return if this was set as background
      bool getBackgroundFlag() const
      {
        return m_backgroundFlag;
      }

      /// Set whether this cluster is consider as made of background
      void setBackgroundFlag(bool backgroundFlag = true)
      {
        m_backgroundFlag = backgroundFlag;
      }

    private:
      /// Memory for the super cluster id
      int m_iSuperCluster = -1;

      /// Memory for the flag marking this cluster as background
      bool m_backgroundFlag = false;
    };
  }
}
