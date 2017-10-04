/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHitPair.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

#include <tracking/trackFindingCDC/numerics/ERightLeft.h>
#include <tracking/trackFindingCDC/topology/EStereoKind.h>
#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <iosfwd>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;
    class CDCWire;

    /**
     *  Class representing a triple of neighboring wire hits.
     *  The first is taken as a single, while the second and the third are grouped into a pair.
     *  In this way we can save some object creations during lookups of a triple following this one,
     *  since the projection to the last pair can be done by reference.
     */
    class CDCRLWireHitTriple {
    public:
      /// Type for the different shapes of a triple of neighboring wire hits
      class Shape {

      public:
        /// Default constructor for an invalid shape
        Shape();

        /// Constructor from cell extend and o'clock direction change
        Shape(const short startToMiddleCellDistance,
              const short middleToEndCellDistance,
              const short oClockDelta);

        /// Getter for the start to middle cell distance
        short getStartToMiddleCellDistance() const;

        /// Getter for the middle to end cell distance
        short getMiddleToEndCellDistance() const;

        /// Getter for the sum of cell distances from start to middle and middle to end.
        short getCellExtend() const;

        /// Getter for the o'clock direction difference from start to middle compared to middle to end.
        short getOClockDelta() const;

        /// Check if the shape is considered valid.
        bool isValid() const;

      private:
        /// The cell distances from start to middle.
        char m_startToMiddleCellDistance;

        /// The cell distances from middle to end.
        char m_middleToEndCellDistance;

        /// The o'clock direction difference from start to middle compared to middle to end.
        short m_oClockDelta;
      };

      /// Default constructor for ROOT
      CDCRLWireHitTriple() = default;

      /// Constructor taking three oriented wire hits.
      CDCRLWireHitTriple(const CDCRLWireHit& startRLWireHit,
                         const CDCRLWireHit& middleRLWireHit,
                         const CDCRLWireHit& endRLWireHit,
                         int iCluster = 0);

      /// Constructs the reverse tiple from this one.
      CDCRLWireHitTriple reversed() const;

      /// Reverses the triple inplace.
      void reverse();

      /// Returns the aliased version of this oriented wire hit triple
      CDCRLWireHitTriple getAlias() const;

      /// Equality comparision considering all three oriented wire hits.
      bool operator==(const CDCRLWireHitTriple& other) const
      {
        return getStartRLWireHit() == other.getStartRLWireHit() and
               getRearRLWireHitPair() == other.getRearRLWireHitPair();
      }


      /// Establish a total ordering  based on the three oriented wire hits.
      bool operator< (const CDCRLWireHitTriple& other) const
      {
        return getICluster() < other.getICluster() or
               (getICluster() == other.getICluster() and
                (getStartRLWireHit() <  other.getStartRLWireHit() or
                 (getStartRLWireHit() == other.getStartRLWireHit() and
                  getRearRLWireHitPair() < other.getRearRLWireHitPair())));
      }

      /**
       *  Define oriented wire hit pairs to be coaligned with oriented wire hit
       *  triples on the first two oriented wire hits.
       */
      friend bool operator< (const CDCRLWireHitTriple& rlWireHitTriple,
                             const CDCRLWireHitPair& rlWireHitPair)
      {
        return rlWireHitTriple.getICluster() < rlWireHitPair.getICluster() or
               (rlWireHitTriple.getICluster() == rlWireHitPair.getICluster() and
                (rlWireHitTriple.getStartRLWireHit() < rlWireHitPair.getFromRLWireHit() or
                 (rlWireHitTriple.getStartRLWireHit() == rlWireHitPair.getFromRLWireHit() and
                  rlWireHitTriple.getMiddleRLWireHit() < rlWireHitPair.getToRLWireHit())));
      }

      /// Define oriented wire hit pairs to be coaligned with orient wire hit triples
      /// on the first two oriented wire hits.
      friend bool operator< (const CDCRLWireHitPair& rlWireHitPair,
                             const CDCRLWireHitTriple& rlWireHitTriple)
      {
        return rlWireHitPair.getICluster() < rlWireHitTriple.getICluster() or
               (rlWireHitPair.getICluster() == rlWireHitTriple.getICluster() and
                (rlWireHitPair.getFromRLWireHit() < rlWireHitTriple.getStartRLWireHit() or
                 (rlWireHitPair.getFromRLWireHit() == rlWireHitTriple.getStartRLWireHit() and
                  rlWireHitPair.getToRLWireHit() < rlWireHitTriple.getMiddleRLWireHit())));
      }

      /// Getter for the shape of this tiple if all three oriented wire hits are neighbors. Else ILLSHAPE
      Shape getShape() const;

      /// Getter for the common superlayer id of the pair
      ISuperLayer getISuperLayer() const
      {
        assert(getStartRLWireHit().getISuperLayer() == getRearRLWireHitPair().getISuperLayer());
        return getStartRLWireHit().getISuperLayer();
      }

      /// Getter for the common stereo type of the superlayer of the pair.
      EStereoKind getStereoKind() const
      {
        return ISuperLayerUtil::getStereoKind(getISuperLayer());
      }

      /// Indicator if the underlying wires are axial.
      bool isAxial() const
      {
        return getStereoKind() == EStereoKind::c_Axial;
      }

      /// Getter for the wire the first oriented wire hit is based on.
      const CDCWire& getStartWire() const
      {
        return getStartRLWireHit().getWire();
      }

      /// Getter for the wire the second oriented wire hit is based on.
      const CDCWire& getMiddleWire() const
      {
        return getMiddleRLWireHit().getWire();
      }

      /// Getter for the wire the third oriented wire hit is based on.
      const CDCWire& getEndWire() const
      {
        return getEndRLWireHit().getWire();
      }

      /// Indicator if any of the three oriented wire hits is based on the given wire.
      bool hasWire(const CDCWire& wire) const
      {
        return getStartRLWireHit().isOnWire(wire) or getRearRLWireHitPair().hasWire(wire);
      }

      /// Getter for the right left passage information of the first oriented wire hit.
      ERightLeft getStartRLInfo() const
      {
        return getStartRLWireHit().getRLInfo();
      }

      /// Getter for the right left passage information of the second oriented wire hit.
      ERightLeft getMiddleRLInfo() const
      {
        return getMiddleRLWireHit().getRLInfo();
      }

      /// Getter for the right left passage information of the third oriented wire hit.
      ERightLeft getEndRLInfo() const
      {
        return getEndRLWireHit().getRLInfo();
      }

      /// Getter for the hit wire of the first oriented wire hit.
      const CDCWireHit& getStartWireHit() const
      {
        return getStartRLWireHit().getWireHit();
      }

      /// Getter for the hit wire of the second oriented wire hit.
      const CDCWireHit& getMiddleWireHit() const
      {
        return getMiddleRLWireHit().getWireHit();
      }

      /// Getter for the hit wire of the third oriented wire hit.
      const CDCWireHit& getEndWireHit() const
      {
        return getEndRLWireHit().getWireHit();
      }

      /// Indicator if any of the three oriented wire hits is based on the given wire hit.
      bool hasWireHit(const CDCWireHit& wirehit) const
      {
        return getStartRLWireHit().hasWireHit(wirehit) or
               getRearRLWireHitPair().hasWireHit(wirehit);
      }

      /// Getter for the first oriented wire hit.
      CDCRLWireHit& getStartRLWireHit()
      {
        return m_startRLWireHit;
      }

      /// Getter for the second oriented wire hit.
      CDCRLWireHit& getMiddleRLWireHit()
      {
        return getRearRLWireHitPair().getFromRLWireHit();
      }

      /// Getter for the third oriented wire hit.
      CDCRLWireHit& getEndRLWireHit()
      {
        return getRearRLWireHitPair().getToRLWireHit();
      }

      /// Constant getter for the first oriented wire hit.
      const CDCRLWireHit& getStartRLWireHit() const
      {
        return m_startRLWireHit;
      }

      /// Constant getter for the second oriented wire hit.
      const CDCRLWireHit& getMiddleRLWireHit() const
      {
        return getRearRLWireHitPair().getFromRLWireHit();
      }

      /// Constant getter for the third oriented wire hit.
      const CDCRLWireHit& getEndRLWireHit() const
      {
        return getRearRLWireHitPair().getToRLWireHit();
      }

      /// Getter for the pair of second and third oriented wire hit.
      CDCRLWireHitPair& getRearRLWireHitPair()
      {
        return m_rearRLWireHitPair;
      }

      /// Constant getter for the pair of second and third oriented wire hit.
      const CDCRLWireHitPair& getRearRLWireHitPair() const
      {
        return m_rearRLWireHitPair;
      }

      /// Setter for the first oriented wire hit.
      void setStartRLWireHit(const CDCRLWireHit& startRLWireHit)
      {
        m_startRLWireHit = startRLWireHit;
      }

      /// Setter for the second oriented wire hit.
      void setMiddleRLWireHit(const CDCRLWireHit& middleRLWireHit)
      {
        m_rearRLWireHitPair.setFromRLWireHit(middleRLWireHit);
      }

      /// Setter for the third oriented wire hit.
      void setEndRLWireHit(const CDCRLWireHit& endRLWireHit)
      {
        m_rearRLWireHitPair.setToRLWireHit(endRLWireHit);
      }

      /// Setter for the right left passage information of the first oriented wire hit.
      void setStartRLInfo(const ERightLeft startRLInfo)
      {
        m_startRLWireHit.setRLInfo(startRLInfo);
      }

      /// Setter for the right left passage information of the second oriented wire hit.
      void setMiddleRLInfo(const ERightLeft middleRLInfo)
      {
        getRearRLWireHitPair().setFromRLInfo(middleRLInfo);
      }

      /// Setter for the right left passage information of the third oriented wire hit.
      void setEndRLInfo(const ERightLeft endRLInfo)
      {
        getRearRLWireHitPair().setToRLInfo(endRLInfo);
      }

      /// Getter for the cluster id
      int getICluster() const
      {
        return m_rearRLWireHitPair.getICluster();
      }

      /// Setter for the cluster id
      void setICluster(int iCluster)
      {
        m_rearRLWireHitPair.setICluster(iCluster);
      }

    protected:
      /// Memory for the start oriented wire hit.
      CDCRLWireHit m_startRLWireHit;

      /// Memory for the second and third wire hits.
      CDCRLWireHitPair m_rearRLWireHitPair;
    };

    std::ostream& operator<<(std::ostream& output, const CDCRLWireHitTriple& rlWireHitTriple);
  }
}
