/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOFACET_H
#define CDCRECOFACET_H

#include <utility>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/algorithms/AutomatonCell.h>

#include "CDCRecoHit2D.h"
#include "CDCRecoTangent.h"


namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representing a triple of neighboring wire hits
    class CDCRecoFacet : public UsedTObject {

    public:

      enum Shape { ILLSHAPED = 0 , ORTHO = 1, META = 2, PARA = 3, };

    private:
      typedef std::pair<const Belle2::CDCLocalTracking::CDCWireHit*, RightLeftInfo> OrientedWireHit;  //should eventually become a full class

      static bool orientedWireHitIsLess(const OrientedWireHit& lhs,  const OrientedWireHit& rhs)
      { return (*(lhs.first) < * (rhs.first) or (*(lhs.first) == *(rhs.first) and lhs.second <  rhs.second));}

      static bool orientedWireHitIsEqual(const OrientedWireHit& lhs,  const OrientedWireHit& rhs)
      { return (*(lhs.first) == *(rhs.first) and lhs.second == rhs.second);}

      /** Constructor. */
      /** This is as well the parameter free I/O constructor.
       */
    public:
      CDCRecoFacet();

      CDCRecoFacet(
        const CDCWireHit* startWireHit,  const RightLeftInfo& startRLInfo,
        const CDCWireHit* middleWireHit, const RightLeftInfo& middleRLInfo,
        const CDCWireHit* endWireHit,    const RightLeftInfo& endRLInfo
      );

      CDCRecoFacet(
        const CDCWireHit* startWireHit,  const RightLeftInfo& startRLInfo,
        const CDCWireHit* middleWireHit, const RightLeftInfo& middleRLInfo,
        const CDCWireHit* endWireHit,    const RightLeftInfo& endRLInfo,
        const ParameterLine2D& startToMiddle,
        const ParameterLine2D& startToEnd = ParameterLine2D(),
        const ParameterLine2D& middleToEnd = ParameterLine2D()
      );

      /** Destructor. */
      ~CDCRecoFacet() {;}

      bool operator==(CDCRecoFacet const& rhs) const
      { return orientedWireHitIsEqual(m_start, rhs.m_start) and orientedWireHitIsEqual(m_middle, rhs.m_middle) and orientedWireHitIsEqual(m_end, rhs.m_end); }

      /** Establish a total ordering  based on the two recohits. First compare the two wirehits and secondary the additional information of the 2D recohits in order to have tangets based on the same wirehits always close together. */
      bool operator< (CDCRecoFacet const& rhs) const {

        return *(getStartWireHit()) < *(rhs.getStartWireHit()) or (
                 *(getStartWireHit()) == *(rhs.getStartWireHit()) and (
                   getStartRLInfo() < getStartRLInfo() or (
                     getStartRLInfo() == getStartRLInfo() and (

                       *(getMiddleWireHit()) < * (rhs.getMiddleWireHit()) or (
                         *(getMiddleWireHit()) == *(rhs.getMiddleWireHit()) and (
                           getMiddleRLInfo() < getMiddleRLInfo() or (
                             getMiddleRLInfo() == getMiddleRLInfo() and (

                               *(getEndWireHit()) < * (rhs.getEndWireHit()) or (
                                 *(getEndWireHit()) == *(rhs.getEndWireHit()) and (
                                   getEndRLInfo() < getEndRLInfo()
                                 )
                               )

                             )
                           )
                         )
                       )

                     )
                   )
                 )
               );
      }


      /** Equality comparision based in the two recohits */
      bool IsEqual(const CDCRecoFacet* const& other) const
      { return other == nullptr ? false : operator==(*other); }

      /** Establish a total ordering  based on the two recohits */
      bool IsLessThan(const CDCRecoFacet* const& other) const
      { return other == nullptr ? false : operator<(*other); }

      // lowest CDCRecoFacet creators
      static CDCRecoFacet getLowest() {
        return CDCRecoFacet(&(CDCWireHit::getLowest()), LEFT,
                            &(CDCWireHit::getLowest()), LEFT,
                            &(CDCWireHit::getLowest()), LEFT,
                            ParameterLine2D());
      }

      // lowerBound CDCRecoFacet creators for CDCWireHit
      static CDCRecoFacet getLowerBound(const CDCWireHit* startWireHit) {
        return CDCRecoFacet(startWireHit, LEFT,
                            &(CDCWireHit::getLowest()), LEFT,
                            &(CDCWireHit::getLowest()), LEFT,
                            ParameterLine2D());
      }

      // lowerBound CDCRecoFacet creators for CDCWireHit
      static CDCRecoFacet getLowerBound(const CDCWireHit* startWireHit, const RightLeftInfo& startRLInfo) {
        return CDCRecoFacet(startWireHit, startRLInfo,
                            &(CDCWireHit::getLowest()), LEFT,
                            &(CDCWireHit::getLowest()), LEFT,
                            ParameterLine2D());
      }

      static CDCRecoFacet getLowerBound(
        const CDCWireHit* startWireHit,
        const RightLeftInfo& startRLInfo,
        const CDCWireHit* middleWireHit,
        const RightLeftInfo& middleRLInfo
      ) {
        return CDCRecoFacet(startWireHit, startRLInfo,
                            middleWireHit, middleRLInfo,
                            &(CDCWireHit::getLowest()), LEFT,
                            ParameterLine2D());
      }


      const RightLeftInfo& getStartRLInfo() const
      { return m_start.second; }

      const RightLeftInfo& getMiddleRLInfo() const
      { return m_middle.second; }

      const RightLeftInfo& getEndRLInfo() const
      { return m_end.second; }



      const CDCWire& getStartWire() const
      { return getStartWireHit()->getWire(); }

      const CDCWire& getMiddleWire() const
      { return getMiddleWireHit()->getWire(); }

      const CDCWire& getEndWire() const
      { return getEndWireHit()->getWire(); }

      Shape getShape() const
      { return getShape(getStartWire(), getMiddleWire(), getEndWire()); }

      /// Gives the type of the resulting facet if the given wires were the start, middle and end point */
      static Shape getShape(
        const CDCWire& startWire,
        const CDCWire& middleWire,
        const CDCWire& endWire
      );

      const CDCWireHit* getStartWireHit() const
      { return m_start.first; }

      const CDCWireHit* getMiddleWireHit() const
      { return  m_middle.first; }

      const CDCWireHit* getEndWireHit() const
      { return m_end.first; }



      const ParameterLine2D& getStartToMiddleLine() const
      { return m_startToMiddle; }

      const ParameterLine2D& getStartToEndLine() const
      { return m_startToEnd; }

      const ParameterLine2D& getMiddleToEndLine() const
      { return m_middleToEnd; }

      void adjustLines() const;


      Vector2D getStartRefPos2D() const
      { return Vector2D::average(getStartToMiddleLine().at(0), getStartToEndLine().at(0)); }

      Vector2D getMiddleRefPos2D() const
      { return Vector2D::average(getStartToMiddleLine().at(1), getMiddleToEndLine().at(0)); }

      Vector2D getEndRefPos2D() const
      { return Vector2D::average(getStartToEndLine().at(1), getMiddleToEndLine().at(1)); }

      Vector2D getCenterOfMass2D() const
      { return Vector2D::average(getStartRefPos2D(), getMiddleRefPos2D(), getEndRefPos2D()); }



      CDCRecoHit2D getStartRecoHit2D() const
      { return CDCRecoHit2D::fromAbsPos2D(getStartWireHit(), getStartRLInfo(), getStartRefPos2D()); }

      CDCRecoHit2D getMiddleRecoHit2D() const
      { return CDCRecoHit2D::fromAbsPos2D(getMiddleWireHit(), getMiddleRLInfo(), getMiddleRefPos2D()); }

      CDCRecoHit2D getEndRecoHit2D() const
      { return CDCRecoHit2D::fromAbsPos2D(getEndWireHit(), getEndRLInfo(), getEndRefPos2D()); }


      CDCRecoTangent getStartToMiddle() const
      { return CDCRecoTangent(getStartWireHit(), getStartRLInfo(), getMiddleWireHit(), getMiddleRLInfo(), getStartToMiddleLine()); }

      CDCRecoTangent getStartToEnd() const
      { return CDCRecoTangent(getStartWireHit(), getStartRLInfo(), getEndWireHit(), getEndRLInfo(), getStartToEndLine()); }

      CDCRecoTangent getMiddleToEnd() const
      { return CDCRecoTangent(getMiddleWireHit(), getMiddleRLInfo(), getEndWireHit(), getEndRLInfo(), getMiddleToEndLine()); }

      void reversed() {
        std::swap(m_start, m_end);
        m_start.second = CDCLocalTracking::reversed(m_start.second);
        m_middle.second = CDCLocalTracking::reversed(m_middle.second);
        m_end.second = CDCLocalTracking::reversed(m_end.second);
      }


      CDCRecoFacet reversed() const {
        return  CDCRecoFacet(m_end.first,    CDCLocalTracking::reversed(m_end.second) ,
                             m_middle.first, CDCLocalTracking::reversed(m_middle.second) ,
                             m_start.first,  CDCLocalTracking::reversed(m_start.second));
      }

      /* making the interface between pointers and object more exchangeable */
      const CDCRecoFacet* operator->() const { return this; }

      bool hasWire(const CDCWire& wire) const {
        return getStartWireHit()->hasWire(wire) or
               getMiddleWireHit()->hasWire(wire) or
               getEndWireHit()->hasWire(wire);
      }

      bool hasWireHit(const CDCWireHit& wirehit) const {
        return getStartWireHit()->hasWireHit(wirehit) or
               getMiddleWireHit()->hasWireHit(wirehit) or
               getEndWireHit()->hasWireHit(wirehit);
      }

      AxialType getAxialType() const
      { return getISuperLayer() == INVALIDSUPERLAYER ? INVALID_AXIALTYPE : getStartToMiddle().getAxialType(); }

      ILayerType getISuperLayer() const;

      FloatType getStartPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getStartRefPos2D()); }

      FloatType getEndPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getEndRefPos2D()); }

      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const;

      friend std::ostream& operator<<(std::ostream& output, const CDCRecoFacet& facet) {
        return output << "Start : "  << facet.getStartWireHit() << " with rl " << facet.getStartRLInfo() <<
               " Middle : " << facet.getMiddleWireHit() << " with rl " << facet.getMiddleRLInfo() <<
               " End : "    << facet.getEndWireHit()  << " with rl " << facet.getEndRLInfo() ;
      }


      /// Sets the do not use flag of the facet's automaton cell and of the three contained wire hits
      void setDoNotUse() const {
        getAutomatonCell().setFlags(DO_NOT_USE);
        forwardDoNotUse();
      }

      /// Sets the do not use flag of the three contained wire hits
      void forwardDoNotUse() const {
        getStartWireHit()->getAutomatonCell().setFlags(DO_NOT_USE);
        getMiddleWireHit()->getAutomatonCell().setFlags(DO_NOT_USE);
        getEndWireHit()->getAutomatonCell().setFlags(DO_NOT_USE);
      }

      /// If one of the contained wire hits is marked as do not use this facet is set be not usable as well
      void receiveDoNotUse() const {

        if (getStartWireHit()->getAutomatonCell().hasAnyFlags(DO_NOT_USE) or
            getMiddleWireHit()->getAutomatonCell().hasAnyFlags(DO_NOT_USE) or
            getEndWireHit()->getAutomatonCell().hasAnyFlags(DO_NOT_USE)) {

          getAutomatonCell().setFlags(DO_NOT_USE);
        }
      }

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell() { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      const AutomatonCell& getAutomatonCell() const { return m_automatonCell; }

    private:

      OrientedWireHit m_start;
      OrientedWireHit m_middle;
      OrientedWireHit m_end;

      mutable ParameterLine2D m_startToMiddle;
      mutable ParameterLine2D m_startToEnd;
      mutable ParameterLine2D m_middleToEnd;

      AutomatonCell m_automatonCell;


      /// ROOT Macro to make CDCRecoFacet a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoFacet, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRECOFACET_H
