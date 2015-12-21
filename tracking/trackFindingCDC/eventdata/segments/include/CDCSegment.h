/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/topology/CDCWireSuperLayer.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>
#include <tracking/trackFindingCDC/topology/EStereoKind.h>

#include <vector>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A sequence of hits limited to one superlayer.
    template<class T>
    class CDCSegment : public std::vector<T> {

    public:
      /// Defines segments and superlayers to be coaligned.
      friend bool operator<(const CDCSegment<T>& segment, const CDCWireSuperLayer& wireSuperLayer)
      { return segment.getISuperLayer() < wireSuperLayer.getISuperLayer(); }

      /// Defines segments and superlayers to be coaligned.
      friend bool operator<(const CDCWireSuperLayer& wireSuperLayer, const CDCSegment<T>& segment)
      { return wireSuperLayer.getISuperLayer() < segment.getISuperLayer(); }

      /// Returns the common stereo type of all tracking hits.
      /** This checks if all tracking hits are located in the same superlayer and \n
       *  returns the stereo type of the later. Returns EStereoKind::c_Invalid if the superlayer \n
       *  is not shared among the tracking hits. */
      EStereoKind getStereoKind() const
      { return TrackFindingCDC::getStereoKind(*this); }

      /// Indicator if the underlying wires are axial.
      bool isAxial() const
      { return getStereoKind() == EStereoKind::c_Axial; }

      /** Returns the common super layer id of all stored tracking hits
       *  This checks if all items are located in the same superlayer and
       *  returns the superlayer id of the later. Returns ISuperLayerUtil::c_Invalid if the superlayer
       *  is not shared among the tracking hits. */
      ISuperLayer getISuperLayer() const
      { return ISuperLayerUtil::getCommon(*this); }

      /// Legacy accessor for the items of the segments, still used in some corners of python scripts.
      const std::vector<T>& items() const
      { return *this; }

      /// Getter for the two dimensional trajectory fitted to the segment
      CDCTrajectory2D& getTrajectory2D() const
      { return m_trajectory2D; }

      /// Setter for the two dimensional trajectory fitted to the segment
      void setTrajectory2D(const CDCTrajectory2D& trajectory2D) const
      { m_trajectory2D =  trajectory2D; }

    private:
      /// Memory for the two dimensional trajectory fitted to this segment
      mutable CDCTrajectory2D m_trajectory2D;

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2

