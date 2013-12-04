/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCWIREHIT_H
#define CDCWIREHIT_H

#include <TVector2.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/translators/SimpleTDCCountTranslator.h>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/algorithms/AutomatonCell.h>

#include <tracking/cdcLocalTracking/topology/CDCWire.h>

#include <tracking/cdcLocalTracking/eventdata/trajectories/CDCTrajectory2D.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representing a hit wire in the central drift chamber
    /** This class combines the measurement informations from a CDCHit with the static geometry informations. \n
     *  It forms the basis of all other higher level tracking objects.
     *  It inherits from AutomataCell since we want to use it with the Clusterizer which makes use of the cell state. \n
     *  Additionally contains references to both the CDCWire instance from the CDCTopology and the CDCHit from the StoreArray of the event. \n
     *  Also it keeps the index of the CDCHit in the StoreArray since we will need it for later output to genfit::TrackCands \n
     */

    class CDCWireHit : public AutomatonCell {

    public:

      /// Default constructor for ROOT compatibility.
      CDCWireHit();

    private:
      /// Constructor for interal use
      /** Construct for a dummy CDCWireHit. The wire hit is still assoziated with a wire but has no hit attached to it.
          The getHit() will yield nullptr. The getStoreIHit() yields -1 for this case. */
      CDCWireHit(const CDCWire* wire);

    public:
      /// Constructor for augmenting the CDCHit with the geometry information of the CDCWire.
      /** Binds to the CDCHit and the corresponding wire together. Also takes the index of the StoreArray
        * from which the hit has been taken. Necessary for later translation to genfit.*/
      CDCWireHit(const CDCHit* hit, int iHit);

      /// Constructor to taking a wire ID and a driftlength at the reference. For testing only!
      CDCWireHit(const WireID& wireID, const FloatType& driftLength);

      /// Empty deconstructor
      ~CDCWireHit();

    public:

      /** @name Equality comparision
       *  Based on the equality of the wire and the hit id */
      /**@{*/
      /// Equality comparision based the wire and the hit id.
      /** Equality comparision based in wire ids and hit id. \n
          Hit id is taken into account in case we want to distiguish double hits at some point.\n
          Depending on the double hit implementation this might have to change though.*/
      bool operator==(const CDCWireHit& other) const
      { return getWire() == other.getWire() and getStoreIHit() == other.getStoreIHit(); }

      /// Equality comparision based on the wire and the hit id usable with pointer.
      /** Equality comparision of wire hits based on the wire and the hit id.
       *  This is still usable if a nullptr is given. The nullptr is always different to an actual wire object.
       *  Compatible for use with ROOT containers.
       */
      bool inline IsEqual(const CDCWireHit* const& other) const
      {return other == nullptr ? false : operator==(*other); }

      /// Equality comparision based on on the wire and the hit id usable with two pointer.
      /** Equality comparision of wires based on their on the wire and their hit id.
       *  This is still usable, if two nullptrs are given to the function.
       *  The nullptr is always different to any actual wire object, but a nullptr is equal to itself.
       *  Since there is no object in the case of two nullptrs we have to relie on a static method */
      static inline bool ptrIsEqual(const CDCWireHit* lhs , const CDCWireHit* rhs)
      { return lhs == nullptr ? (rhs == nullptr ? true : false) : lhs->IsEqual(rhs); }
      /**@}*/

      /** @name Total ordering
       *  Comparing the wire first and the hit id second. Hence wire hits are coaligned with the wires.*/
      /**@{*/
      /// Total ordering relation based on the wire and the hit id.
      /** Defines a total ordering sheme for wire hit objects based on the wire first and the hit id second.
       *  It needs be present for the wire to work with all kinds of stl algorithms and containers */
      bool operator<(const CDCWireHit& other) const {
        return (getWire() < other.getWire() or (
                  getWire() == other.getWire() and getStoreIHit() < other.getStoreIHit()));
      }

      /// Total ordering relation based on the wire and the hit id usable with pointers.
      /** Retains the total ordering sheme for wire hit objects, but introduces the special nullptr case to the ordering.
       *  The nullptr is always smallest. Therefore it forms a lower bound for the wire hit pointers.
       *  This also enables compatibility with all sorts of ROOT containers*/
      bool inline IsLessThan(const CDCWireHit* const& other) const
      { return other == nullptr ? false : operator<(*other); }

      /// Total ordering relation based on the wire and the hit id usable with two pointer
      /** Retains the total ordering sheme for wire hit objects,
       *  but introduces the special nullptr case to the ordering.
       *  The nullptr is always smallest. Therefor it forms a lower bound for the wire hit objects.
       *  This is for completeness if we want to take the nullptr as a valid member of the range.
       *  Since there is no object in the case of two nullptrs we have to relie on a static method */
      static inline bool ptrIsLessThan(const CDCWireHit* lhs , const CDCWireHit* rhs)
      { return rhs == nullptr ? false : (lhs == nullptr ? true : *lhs < *rhs); }

      /// Getter for the possible lowest wire hit. For lookup purpose only.
      /** Get the lowest wire hit static instance. The lowest wire hit compares less than all other wire hit instances possible. \n
       *  It serves as a sentinal in all kinds of look ups in ordered sequences of higher order tracking entities. \n
       *  Since we need a certain storage place for the wire to go and we do not want to recreate it all the time \n
       *  we keep it in a static variable.
       *  However it has to be initialized before the first look up and after each switch of the geometry
       *  This should happen on the initialize() or beginRun() of the tracking module!! */
      static const CDCWireHit& getLowest()
      { return *s_lowest; }

      /// Getter for the possible lowest wire hit on a certain wire. It contains no CDCHit though. For lookup purpose only. See details.
      /** Important : make sure that the CDCWire lives longer than the CDCWireHit \n
       *  return by this function since its keeping the reference to it. */
      static const CDCWireHit getLowerBound(const CDCWire* wire)
      { return CDCWireHit(wire); }
      /**@}*/

      /// Getter for the CDCHit pointer into the StoreArray
      const CDCHit* getHit() const { return m_hit; }

      /// Getter for the CDCWire the hit is located on.
      const CDCWire& getWire() const { return *m_wire; }

      /// The two dimensional reference position of the underlying wire
      const Vector2D& getRefPos2D() const { return getWire().getRefPos2D(); }

      /// Connection vector from the center of this wire hit to the other.
      Vector2D getRefVecTo2D(const CDCWireHit& to) const
      { return to.getRefPos2D() - getRefPos2D(); }

      /// Getter for the  drift length at the reference position of the wire
      FloatType getRefDriftLength() const
      { return m_refDriftLength; }

      /// Getter for the skew of the wire.
      FloatType getSkew() const { return getWire().getSkew(); }

      /// Estimate the transvers travel distance on the given circle.
      /** Uses the point of closest approach on the circle and
       *  calculates the arc length from the reference on the circle.
       *  @return The arc length on the circle from the reference */
      FloatType getPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpS(getRefPos2D()); }

      /// Getter for the index  of the hit in the StoreArray holding this hit.
      int getStoreIHit() const { return m_iHit; }


      /** @name Mimic pointer
        */
      /// Access the object methods and methods from a pointer in the same way.
      /** In situations where the type is not known to be a pointer or a reference there is no way to tell \n
       *  if one should use the dot '.' or operator '->' for method look up. \n
       *  So this function defines the -> operator for the object. \n
       *  No matter you have a pointer or an object access is given with '->'*/
      /**@{*/
      const CDCWireHit* operator->() const { return this; }
      /**@}*/


      /** @name Methods common to all tracking entities
       *  All entities ( track parts contained in a single superlayer ) share this interface to help the definition of collections of them. */
      /**@{*/
      /// Checks of the wire hit is base on the wire given
      bool hasWire(const CDCWire& wire) const
      { return getWire() == wire; }

      /// Checks if the wire hit is equal to the wire hit given.
      bool hasWireHit(const CDCWireHit& wirehit) const
      { return operator==(wirehit); }

      /// Center of mass is just the refernce position for wire hits.
      const Vector2D& getCenterOfMass2D() const { return getRefPos2D(); }

      /// Getter for the axial type of the underlying wire.
      AxialType getAxialType() const
      { return getWire().getAxialType(); }

      /// Getter for the super layer id
      ILayerType getISuperLayer() const
      { return getWire().getISuperLayer(); }

      /// Same as getPerpS().
      FloatType getStartPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Same as getPerpS().
      FloatType getEndPerpS(const CDCTrajectory2D& trajectory2D) const
      { return getPerpS(trajectory2D); }

      /// Calculates the squared distance of the wire hit to a circle as see from the transvers plane
      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const {
        FloatType distance = trajectory2D.getDist2D(getRefPos2D());
        distance = distance > 0 ? distance - getRefDriftLength() : distance + getRefDriftLength();
        return distance * distance;
      }
      /**@}*/

      /// Sting output operator for wire hit objects to help debugging
      friend std::ostream& operator<<(std::ostream& output, const CDCWireHit& wirehit) {
        return output << "CDCWireHit(" << wirehit.getWire() << ",dl=" << wirehit.getRefDriftLength() << ")";
      }

      /// Sting output operator for wire hit pointers to help debugging
      friend std::ostream& operator<<(std::ostream& output, const CDCWireHit* wirehit)
      { return wirehit == nullptr ? output << "nullptr" : output << *wirehit; }

    private:
      const CDCWire* m_wire;  ///< Memory for the CDCWire reference

      const CDCHit* m_hit;   ///< Memory for the CDCWire reference
      int m_iHit; ///< Memory for the index into the storing StoreArray<CDCHit>
      FloatType m_refDriftLength; ///< Memory for the drift length at the wire reference point

    private:
      /// Static variable for lowest possible wire hit
      /** For all kinds of look up involving high order tracking entitie we need a sentinal wire hit
       *  which is lower than any kind of actual wire hit. Since we do not want to recreate it all the
       *  time we create it as a static variable. The default parameterless constructor constructs the
       *  lowest wire hit. So no specifc initialisation is needed.
       *  However the lowest wire has to be updated for each switch of geometry.
       */
      static const CDCWireHit* s_lowest;
    public:

      /// Initializes the lowest wire hit possible
      /** This creates the lowest wire hit possible stored as a static instance.
       *  Call after each geometry switch */
      static void initializeLowest() {
        if (s_lowest) delete s_lowest;
        s_lowest = new CDCWireHit(&(CDCWire::getLowest()));
      }

      //setup instance for the tdc count translation
    public:
      /// Helper object type for translating the information stored in the CDCHit to drift lengths
      typedef CDC::SimpleTDCCountTranslator TDCCountTranslator;
    private:
      /// Helper object instance for translating the information stored in the CDCHit to drift lengths
      static TDCCountTranslator* m_tdcCountTranslator;
    public:
      /// Helper object instance getter for translating the information stored in the CDCHit to drift lengths
      static TDCCountTranslator& TDCCountTranslatorInstance() {
        if (!m_tdcCountTranslator) m_tdcCountTranslator = new TDCCountTranslator();
        return *m_tdcCountTranslator;
      }

      /// ROOT Macro to make CDCWireHit a ROOT class.
      ClassDefInCDCLocalTracking(CDCWireHit, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCWIREHIT_H
