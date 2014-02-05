/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOHITTANGENT_H
#define CDCRECOHITTANGENT_H

#include <vector>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/geometry/ParameterLine2D.h>

#include "CDCWireHit.h"
#include "CDCRecoHit2D.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representating a linear track piece between two wire hits \n
    /** A reconstructed tangent is an approximation of the possible trajectory between two wire hits. \n
     *  The approximation is obtained by constructing a tangent between two drift circles of the wire hits  \n
     *  ( in the reference xy projection ) \n
     *  Generally maximal four tangents are possible. So to uniquely define a tangent we have \n
     *  to give additional information how it passes relativ to the drift circles. This right left \n
     *  passage inforamtion indicates if the related wire hit should lie to the right of to the left \n
     *  of the tangent. The four possible combinations are then ( RIGHT , RIGHT ), ( RIGHT , LEFT ), \n
     *  ( LEFT , RIGHT ), ( LEFT , LEFT ). \n
     *  To represent the tangent this class uses a ParameterLine2D. The touch points to the two drift circles \n
     *  are located at(0) for the first and at(1) for the second. \n
     *  The tangent has therefor a sense of what is forward and can be reversed if necessary \n
     *  Generally tangents are only a good approximation between neighboring wire hits.*/
    class CDCRecoTangent : public CDCLocalTracking::UsedTObject {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoTangent();

      /// Construct a tangent from two wire hits and specified right left passage information.
      /** Constructs the tangent stretching from the from wire hit to the to wire hit with the right left passage information as given. This initializes the line to the correct touching points.*/
      CDCRecoTangent(const CDCWireHit* fromWireHit, const RightLeftInfo& fromRL,
                     const CDCWireHit* toWireHit  , const RightLeftInfo& toRL);
      /// Construct a tangent from two wire hits and specified right left passage information taking the line as is.
      /** Constructs the tangent stretching from the from wire hit to the to wire hit with the right left passage information as given. This initalizes the line to the line given. It does not change or checks the touch point of the tangent for their correctness. Use if the line is not need anyways, for instance for look up.*/
      CDCRecoTangent(const CDCWireHit* fromWireHit, const RightLeftInfo& fromRL,
                     const CDCWireHit* toWireHit  , const RightLeftInfo& toRL  ,
                     const ParameterLine2D& line);

      /// Construct a tangent from two recohits.
      /** Takes the wire hits and right left passage from the recohits. The line is initalize to pass through the reconstructed positions. Does not check if the points are actually touching the drift circles. */
      CDCRecoTangent(const CDCRecoHit2D& fromRecoHit, const CDCRecoHit2D& toRecoHit);

      /// Empty deconstructor
      ~CDCRecoTangent();

      /// A pair of wire hits for storing the two wire hits of the tangents.
      typedef std::pair < const Belle2::CDCLocalTracking::CDCWireHit*,
              const Belle2::CDCLocalTracking::CDCWireHit* > CDCWireHitPair;


      /// Getter for the first wire hit
      const CDCWireHit* getFromWireHit() const
      { return m_wirehits.first; }

      /// Getter for the second wire hit
      const CDCWireHit* getToWireHit() const
      { return m_wirehits.second; }

      /// Getter for the pair of wirehits the tangent is based on in the order from -> to
      const CDCWireHitPair& getWireHitPair() const { return m_wirehits; }

      /// Getter for the right left passage information relative to the first wire hit
      /** Indicates if the first wire hit lies to the right or to the left of the tangent*/
      const RightLeftInfo& getFromRLInfo() const
      { return m_fromRLInfo; }

      /// Getter for the right left passage information relative to the first wire hit
      /** Indicates if the second wire hit lies to the right or to the left of the tangent*/
      const RightLeftInfo& getToRLInfo() const
      { return m_toRLInfo; }

      /// Getter for the touching point of the tangent to the first drift circle
      const Vector2D& getFromRefTouch2D() const
      { return getLine().support(); }

      /// Getter for displacement of the touching point from the first wire in the reference plane
      /** Returns the difference vector from the reference wire position to the touching point of the tangent to the drift circle  for the first wire and touching point*/
      Vector2D getFromRefDisp2D() const
      { return getFromRefTouch2D() - getFromWireHit()->getRefPos2D(); }

      /// Getter for the touching point of the tangent to the second drift circle
      Vector2D getToRefTouch2D() const
      { return  getLine().at(1); }

      /// Getter for displacement of the touching point from the second wire in the reference plane
      /** Returns the difference vector from the reference wire position to the touching point of the tangent to the drift circle  for the first wire and touching point*/
      Vector2D getToRefDisp2D() const
      { return getToRefTouch2D() - getToWireHit()->getRefPos2D(); }

      /// Get for the line representation of the line
      /** The line stretchs between the two the touch point. The first touch point is at(0) the second at(1). */
      const ParameterLine2D& getLine() const
      { return m_line; }

      /// Getter for the direction of flight.
      /** Returns the vector from the first to the second touch point.*/
      const Vector2D& getRefFlightVec2D() const
      { return getLine().tangential(); }

      /// Returns the cosine of the angle between the two flight directions of the tangents.
      double getCosFlightDifference(const CDCRecoTangent& tangent) const
      { return getRefFlightVec2D().cosWith(tangent.getRefFlightVec2D()); }

      /// Creates the reconstructed hit of the first wire hit with the touch point as position
      CDCRecoHit2D getFromRecoHit() const
      { return CDCRecoHit2D(getFromWireHit(), getFromRefDisp2D(), getFromRLInfo()) ; }

      /// Creates the reconstructed hit of the second wire hit with the touch point as position
      CDCRecoHit2D getToRecoHit()   const
      { return CDCRecoHit2D(getToWireHit(),   getToRefDisp2D(),   getToRLInfo()) ; }

      /** @name Equality comparision
       *  Based on the equality of the two wire hits and the two left right passage information */
      /**@{*/

      /// Equality comparision based on the two wire hits and the two left right passage information.
      bool operator==(CDCRecoTangent const& rhs) const {
        return getFromWireHit() == rhs.getFromWireHit() and
               getFromRLInfo() == getFromRLInfo() and
               getToWireHit() == rhs.getToWireHit()  and
               getFromRLInfo() == getFromRLInfo() ;
      }

      /// Equality comparision based on the two wire hits and the two left right passage information usable with pointer.
      /** Equality comparision of reconstructed tangents based on the two wire hits and the two left right passage information.
       *  This is still usable if a nullptr is given. The nullptr is always different to an actual wire object.
       *  Compatible for use with ROOT containers.
       */
      bool IsEqual(const CDCRecoTangent* const& other) const
      { return other == nullptr ? false : operator==(*other); }

      /// Equality comparision based on the two wire hits and the two left right passage information usable with two pointers.
      /** Equality comparision of reconstructed tangents based on their two wire hits \n
       *  and the two left right passage information. \n
       *  This is still usable, if two nullptrs are given to the function. \n
       *  The nullptr is always different to any actual wire object, but a nullptr is equal to itself. \n
       *  Since there is no object in the case of two nullptrs we have to relie on a static method */
      static inline bool ptrIsEqual(const CDCRecoTangent* lhs , const CDCRecoTangent* rhs)
      { return lhs == nullptr ? (rhs == nullptr ? true : false) : lhs->IsEqual(rhs); }
      /**@}*/

      /** @name Total ordering
       * Establish a total ordering based on wire hits first and left right passage info second. */
      /**@{*/
      /// Total ordering for tangents
      /** Establish a total ordering based on wire hits first and left right passage info second. \n
       *  First compare the two wirehits (first then second) and \n
       *  secondary the right left passage information (first then second) \n
       *  in order to have tangets based on the same wirehits close together. */
      bool operator<(CDCRecoTangent const& rhs) const {
        return getFromWireHit() <  rhs.getFromWireHit() or (
                 getFromWireHit() == rhs.getFromWireHit() and (
                   getFromRLInfo() < rhs.getFromRLInfo()  or (
                     getFromRLInfo() == rhs.getFromRLInfo() and (

                       getToWireHit() <  rhs.getToWireHit() or (
                         getToWireHit() == rhs.getToWireHit()  and (
                           getToRLInfo() < rhs.getToRLInfo())))))) ;
      }


      /// Defines wire hits and reconstructed tangents as coaligned on the first wire hit
      friend bool operator<(const CDCRecoTangent& recoTangent, const CDCWireHit& wireHit) { return *(recoTangent.getFromWireHit()) < wireHit; }

      /// Defines wire hits and reconstructed tangents as coaligned on the first wire hit
      friend bool operator<(const CDCWireHit& wireHit, const CDCRecoTangent& recoTangent) { return  wireHit <  *(recoTangent.getFromWireHit()); }

      /// Total ordering for tangents usable with pointers.
      /** Establish a total ordering based wire hits first and left right passage info second,
       *  but introduces the special nullptr case to the ordering.
       *  The nullptr is always smallest. Therefore it forms a lower bound for the tangent pointers.
       *  This also enables compatibility with all sorts of ROOT containers*/
      bool IsLessThan(const CDCRecoTangent* const& other) const
      { return other == nullptr ? false : operator<(*other); }

      /// Total ordering relation based on the wire and the hit id usable with two pointer
      /** Retains the total ordering sheme for tangent objects,
       *  but introduces the special nullptr case to the ordering.
       *  The nullptr is always smallest. Therefor it forms a lower bound for the tangent objects.
       *  This is for completeness if we want to take the nullptr as a valid member of the range.
       *  Since there is no object in the case of two nullptrs we have to relie on a static method */
      static inline bool ptrIsLessThan(const CDCRecoTangent* lhs , const CDCRecoTangent* rhs)
      { return rhs == nullptr ? false : (lhs == nullptr ? true : *lhs < *rhs); }


      /// Lower bound for all possible tangent objects. For look up only.
      /** Returns the lowest possible tangent object. \n
       *  It contains the lowest wire hits two times. \n
       *  Both passage information are set to left. \n
       *  The tangent line however is not set correctly!
       *  It is involved in the look up into sorted ranges of higher order objects ( facets ). */
      static const CDCRecoTangent getLowest()
      { return CDCRecoTangent(&(CDCWireHit::getLowest()), LEFT, &(CDCWireHit::getLowest()), LEFT, ParameterLine2D()); }

      /// Lower bound for all possible tangent objects with the first wire hit as given. For look up only.
      /** Returns the lowest possible tangent object with a specific first wire hit. \n
       *  It contains the two lowest wire hits as second wirehit. Both passage information are set to left. \n
       *  The tangent line however is not set correctly! \n
       *  It is used to find actual tangents starting from a wire hit in a sorted range of wire hits. */
      static const CDCRecoTangent getLowerBound(const CDCWireHit* wirehit)
      { return CDCRecoTangent(wirehit, LEFT, &(CDCWireHit::getLowest()), LEFT, ParameterLine2D()); }

      /// Lower bound for all possible tangent objects with the first and second wire hit as given. For look up only.
      /** Returns the lowest possible tangent object based on the two given wire hits. \n
       *  Both passage information are set to left. \n
       *  The tangent line however is not set correctly! \n
       *  It is used to find actual tangents starting from a wire hit in a sorted range of wire hits. */
      //static const CDCRecoTangent getLowerBound(const CDCWireHit* fromWireHit, const CDCWireHit* toWireHit)
      //{ return CDCRecoTangent(fromWireHit, LEFT, toWireHit, LEFT, ParameterLine2D()); }
      /**@}*/

      /** @name Mimic pointer
        */
      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference there is no way to tell \n
       *  if one should use the dot '.' or operator '->' for method look up. \n
       *  So this function defines the -> operator for the object. \n
       *  No matter you have a pointer or an object access is given with '->'*/
      /**@{*/
      const CDCRecoTangent* operator->() const { return this; }

      /**@}*/
      /** @name Methods common to all tracking entities
       *  All entities ( track parts contained in a single superlayer ) share this interface to help the definition of collections of them. */
      /**@{*/
      /// Checks if one end of the tangent is next to the give wire
      bool hasWire(const CDCWire& wire) const
      { return getFromWireHit()->hasWire(wire) or getToWireHit()->hasWire(wire); }

      /// Checks if the tangent stretches from or to the given wire hit
      bool hasWireHit(const CDCWireHit& wirehit) const
      { return getFromWireHit()->hasWireHit(wirehit) or getToWireHit()->hasWireHit(wirehit); }

      /// Gets the center of the tangent half way from one touch point to the other
      Vector2D getCenterOfMass2D() const
      { return getLine().at(0.5) ; }

      /// Get the axial type of the superlayer the tangent is contained in. Details.
      /** Gets the axial type of the superlayer the tangent is contained in, \n
       *  if both wire hits are actually in the same superlayer. Else it returns INVALID.
       */
      AxialType getAxialType() const
      { return getISuperLayer() == INVALIDSUPERLAYER ? INVALID_AXIALTYPE : getFromWireHit()->getAxialType(); }

      /// Gets the superlayer id of the tangent if it exists.
      /** Gets the superlayer id of the tangent, \n
       *  if both wire hits are actually in the same superlayer. Else it returns INVALIDSUPERLAYER
       */
      ILayerType getISuperLayer() const {
        return getFromWireHit()->getISuperLayer() == getToWireHit()->getISuperLayer() ?
               getFromWireHit()->getISuperLayer() : INVALIDSUPERLAYER;
      }

      /// Estimate the transvers travel distance on the given circle of the first touch point
      /** Uses the point of closest approach on the circle
       *  to the first touch point and
       *  calculates the arc length from the reference point on the circle.
       *  @return The arc length on the circle from the reference */
      FloatType getStartPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getFromRefTouch2D()); }

      /// Estimate the transvers travel distance on the given circle of the second touch point
      /** Uses the point of closest approach on the circle
       *  to the second touch point and
       *  calculates the arc length from the reference point on the circle.
       *  @return The arc length on the circle from the reference */
      FloatType getEndPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getToRefTouch2D()); }

      /// Calculates the squared distance of the tangent to a circle as see from the transvers plane.
      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const;

      /**@}*/

    public:

      /// Adjusts the line to touch the drift circles with the correct right left passage information
      void adjustLine();

      /// Adjusts the right left passage information of the hits to the tangent's direction of flight */
      void adjustRLInfo();

      /// Reverses the tangent inplace.
      /** Swaps the to wire hits, reverses the left right informations and exchanges the two touch points of the line */
      void reverse();

      /// Same as reverse but returns a copy.
      CDCRecoTangent reversed() const;

      /// Returns the wirehit *this* tangent and the given tangent have in common. Nullptr for no common wire.
      const CDCWireHit* commonWireHit(const CDCRecoTangent& recoTangent) const;

      /// Print tangent for debugging
      friend ostream& operator<<(ostream& output, const CDCRecoTangent& tangent) {
        output << "RecoTangent" << std::endl;
        output << "From : " << tangent.getFromWireHit()->getWire() << " " <<  tangent.getFromRefDisp2D() << std::endl;
        output << "To : " << tangent.getToWireHit()->getWire() << " " <<  tangent.getToRefDisp2D()  << std::endl;
        return output;
      }

      /// Constructs a line touching two circles in one point each.
      /** @param fromCenter first circle center
       *  @param signedFromRadius radius of the first circle multiplied with the right left passage information
       *  @param toCenter second circle center
       *  @param signedToRadius radius of the first circle multiplied with the right left passage information.
       *  @return the line being tangential to both circles.
       *  Note : the touch points reside at(0) for the first and at(1) for the second */
      static ParameterLine2D constructTouchingLine(
        const Vector2D& fromCenter,
        const FloatType& fromSignedRadius,
        const Vector2D& toCenter,
        const FloatType& toSignedRadius
      );

    private:
      /// Memory for the two wirehits the tangent is based on
      std::pair < const Belle2::CDCLocalTracking::CDCWireHit*,
          const Belle2::CDCLocalTracking::CDCWireHit* > m_wirehits;
      RightLeftInfo m_fromRLInfo; ///< Memory for the right left passage information relativ to the first wirehit
      RightLeftInfo m_toRLInfo; ///< Memory for the right left passage information relativ to the first wirehit


      ParameterLine2D m_line;///< Memory for the line between the two touching points. The first touch point at(0), second at(1).

      /// ROOT Macro to make CDCRecoTangent a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoTangent, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRECOHITTANGENT_H
