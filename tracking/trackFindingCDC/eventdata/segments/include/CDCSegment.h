/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>
#include <tracking/trackFindingCDC/topology/EStereoKind.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A sequence of hits limited to one superlayer.
    template <class T>
    class CDCSegment : public std::vector<T> {

    public:
      /// Default constructor for ROOT
      CDCSegment() = default;

      /**
       * Returns the common stereo type of all hits.
       *
       * This checks if all tracking hits are located in the same superlayer and \n
       * returns the stereo type of the later. Returns EStereoKind::c_Invalid if the superlayer \n
       * is not shared among the tracking hits.
       */
      EStereoKind getStereoKind() const
      {
        return ISuperLayerUtil::getStereoKind(getISuperLayer());
      }

      /// Indicator if the underlying wires are axial.
      bool isAxial() const
      {
        return getStereoKind() == EStereoKind::c_Axial;
      }

      /**
       * Returns the common super layer id of all stored tracking hits
       *
       * This checks if all items are located in the same superlayer and
       * returns the superlayer id of the later. Returns ISuperLayerUtil::c_Invalid,
       * if the superlayer is not shared among the hits.
       */
      ISuperLayer getISuperLayer() const
      {
        return ISuperLayerUtil::getFrom(this->front());
      }

      /// Legacy accessor for the items of the segments, still used in some corners
      const std::vector<T>& items() const
      {
        return *this;
      }

      /// Getter for the two dimensional trajectory fitted to the segment
      CDCTrajectory2D& getTrajectory2D() const
      {
        return m_trajectory2D;
      }

      /// Setter for the two dimensional trajectory fitted to the segment
      void setTrajectory2D(const CDCTrajectory2D& trajectory2D) const
      {
        m_trajectory2D = trajectory2D;
      }

      /// Sort the recoHits according to their perpS information
      void sortByArcLength2D()
      {
        // cppcheck-suppress constParameter
        std::stable_sort(this->begin(), this->end(), [](const T & recoHit, const T & otherRecoHit) {
          return recoHit.getArcLength2D() < otherRecoHit.getArcLength2D();
        });
      }

      /// Getter for the flag that this segment may have an aliased version
      double getAliasScore() const
      {
        return m_aliasScore;
      }

      /// Setter for the flag that this segment may have an aliased version
      void setAliasScore(double aliasScore)
      {
        m_aliasScore = aliasScore;
      }

    private:
      /// Memory for the two dimensional trajectory fitted to this segment
      mutable CDCTrajectory2D m_trajectory2D;

      /// Boolean flag to indicate that this segment has a valid alias version
      double m_aliasScore;

    };

  }
}
