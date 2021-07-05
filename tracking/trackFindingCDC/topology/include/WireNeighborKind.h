/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/Modulo.h>
#include <tracking/trackFindingCDC/numerics/ESign.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
     *  Type for the neighbor relationship from one wire to another.
     *  Representations is tailored to cover the primary and secondary neighborhood only.
     */
    class WireNeighborKind {

    public:
      /// Default constructor - constructs an invalid neighbor kind.
      WireNeighborKind() = default;

      /// Useful constructor from cell distance and direction to reach the designated neighbor
      WireNeighborKind(short cellDistance, short oClockDirection)
        : m_cellDistance(cellDistance)
        , m_oClockDirection(moduloFast(oClockDirection, 12))
      {}

      /// Calculate the distance between the current and the layer of the designated neighbor
      short getILayerDifference() const
      {
        return getILayerDifferenceSign() * getAbsILayerDifference();
      }

      /// Calculate the absolute distance between the current and the layer of the designated neighbor
      short getAbsILayerDifference() const
      {
        short slope = abs(symmetricModuloFast(getOClockDirection() + 3, 6));
        short iAbsLayerDifference = slope * getCellDistance() / 2;
        if (iAbsLayerDifference > 2) iAbsLayerDifference = 2;
        return iAbsLayerDifference;
      }

      ///  Calculate if the designated neighbor is more outside.
      short getILayerDifferenceSign() const
      {
        return sign(symmetricModuloFast(getOClockDirection() + 3, 12));
      }

      /**
       *  Get the distance to the wire neighbor counted in number of cells.
       *  This is typically 1 for the primary neighbors and 2 for the secondary neighbors
       */
      short getCellDistance() const
      { return m_cellDistance; }

      /// Get the direction on the clock to follow to reach the neighbor
      short getOClockDirection() const
      { return m_oClockDirection; }

      /// Check if the neighbor kind is in principle valid.
      bool isValid() const
      {
        return
          m_cellDistance >= 1 and
          m_cellDistance <= 2 and
          m_oClockDirection >= 0 and
          m_oClockDirection < 12;
      }

    private:
      /// Memory for the distance between to wires counted in numbers of cells
      short m_cellDistance = 0;

      /// Memory for the direction the neighbor is located. Range 1 - 12 imaged on a regular clock
      short m_oClockDirection = 0;

    };

  }
}
