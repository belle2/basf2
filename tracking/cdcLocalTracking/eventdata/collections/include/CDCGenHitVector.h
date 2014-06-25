/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCGENHITVECTOR_H
#define CDCGENHITVECTOR_H

#include <vector>
#include <algorithm>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>

#include <tracking/cdcLocalTracking/topology/CDCWire.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCWireHit.h>

#include <tracking/cdcLocalTracking/algorithms/SortableVector.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// A generic vector used for different hit aggregates.
    /** details */

    template<class T>
    class CDCGenHitVector : public SortableVector<T> {
    public:
      typedef typename SortableVector<T>::iterator iterator; ///< Iterator type of this container
      typedef typename SortableVector<T>::const_iterator const_iterator;    ///< Constant iterator type of this container

      typedef typename SortableVector<T>::reverse_iterator reverse_iterator; ///< Reversed iterator type of this container
      typedef typename SortableVector<T>::const_reverse_iterator const_reverse_iterator;  ///< Constant reversed iterator type of this container


      typedef typename SortableVector<T>::range range; ///< Iterator type of this container
      typedef typename SortableVector<T>::const_range const_range; ///< Iterator type of this container

      typedef typename SortableVector<T>::input_iterator input_iterator;    ///< Constant iterator type of this container


    public:
      /// Default constructor for ROOT compatibility.
      CDCGenHitVector() {;}

      /// Empty deconstructor
      ~CDCGenHitVector() {;}

      ///Implements the standard swap idiom
      friend void swap(CDCGenHitVector<T>& lhs, CDCGenHitVector<T>& rhs) {
        SortableVector<T>& rawLHS = lhs;
        SortableVector<T>& rawRHS = rhs;
        rawLHS.swap(rawRHS);
        B2DEBUG(200, "CDCGenHitVector::swap");
      }

    private:

      /// Helper class for STL algorithms searching for specific wire
      class HasWirePredicate {
      public:
        HasWirePredicate(const Belle2::CDCLocalTracking::CDCWire& wire) : m_wire(wire) {;}
        bool operator()(const T& item) { return item->hasWire(m_wire); }
      private:
        const CDCWire& m_wire;
      };
      /// Helper class for STL algorithms searching for specific wire hit
      class HasWireHitPredicate {
      public:
        HasWireHitPredicate(const Belle2::CDCLocalTracking::CDCWireHit& wirehit) : m_wirehit(wirehit) {;}
        bool operator()(const T& item) { return item->hasWireHit(m_wirehit); }
      private:
        const Belle2::CDCLocalTracking::CDCWireHit& m_wirehit;
      };

    public:
      /// Erases all tracking entities assoziated with specific wire from the vector.
      void eraseAll(const CDCWire& wire) {
        this->erase(remove_if(this->begin(), this->end(), HasWirePredicate(wire)), this->end());
        //remove erase idiom
      }
      /// Erases all tracking entities assoziated with specific wire hit from the vector.
      void eraseAll(const Belle2::CDCLocalTracking::CDCWireHit& wirehit) {
        this->erase(remove_if(this->begin(), this->end(), HasWireHitPredicate(wirehit)), this->end());
        //remove erase idiom
      }

      /// Checks if any stored tracking entity is assoziated with a specific wire.
      bool hasWire(const CDCWire& wire) const {
        const_iterator found = std::find_if(this->begin(), this->end(), HasWirePredicate(wire));
        return found != this->end();
      }

      /// Checks if any stored tracking entity is assoziated with a specific wire hit.
      bool hasWireHit(const Belle2::CDCLocalTracking::CDCWireHit& wirehit) const {
        const_iterator found = std::find_if(this->begin(), this->end(), HasWireHitPredicate(wirehit));
        return found != this->end();
      }

      /// Copy all entities in this collection assoziated with a specific wire to the given collection.
      void collectForWire(const CDCWire& wire, CDCGenHitVector<T>& collect) const {
        input_iterator inputTo = collect.input();
        for (const_iterator itItem = this->begin(); itItem != this->end(); ++itItem) {
          if ((*itItem)->hasWire(wire)) inputTo = *itItem;
        }
      }
      /// Copy all entities in this collection assoziated with a specific wire hit to the given collection.
      void collectForWireHit(const Belle2::CDCLocalTracking::CDCWireHit& wirehit, CDCGenHitVector<T>& collect) const {
        input_iterator inputTo = collect.input();
        for (const_iterator itItem = this->begin(); itItem != this->end(); ++itItem) {
          if ((*itItem)->hasWireHit(wirehit)) inputTo = *itItem;
        }
      }

      /// Calculates the average center of mass of all stored tracking entities.
      Vector2D getCenterOfMass2D() const {
        Vector2D accumulate(0.0, 0.0);
        //B2DEBUG(100,"getCenterOfMass");
        for (const_iterator itItem = this->begin(); itItem != this->end(); ++itItem) {
          //B2DEBUG(100,(*itItem)->getCenterOfMass2D());
          accumulate.add((*itItem)->getCenterOfMass2D());
        }
        accumulate.divide(this->size());
        return accumulate;
      }

      /// Returns the common axial type of all tracking entities.
      /** This checks if all tracking entities are located in the same superlayer and \n
       *  returns the axial type of the later. Returns INVALID_AXIALTYPE if the superlayer \n
       *  is not shared among the tracking entities. */
      AxialType getAxialType() const
      { return getISuperLayer() == INVALIDSUPERLAYER ? INVALID_AXIALTYPE : this->front()->getAxialType(); }


      /// Returns the common super layer id of all stored tracking entities
      /** This checks if all tracking entities are located in the same superlayer and \n
       *  returns the superlayer id of the later. Returns INVALIDSUPERLAYER if the superlayer \n
       *  is not shared among the tracking entities. */
      ILayerType getISuperLayer() const {
        if (this->empty()) return INVALIDSUPERLAYER;
        const_iterator itItem = this->begin();
        const T& firstItem =  *itItem;
        ILayerType iSuperlayer = firstItem->getISuperLayer();
        for (++itItem; itItem != this->end(); ++itItem) {
          if (iSuperlayer != (*itItem)->getISuperLayer()) return INVALIDSUPERLAYER ;
        }
        return iSuperlayer;
      }

      /// Calculates the sum of all squared distances of the stored tracking entities to the circle as see from the transvers plane.
      FloatType getSquaredDist2D(const CDCTrajectory2D& trajectory2D) const {

        FloatType accumulate = 0;
        for (const_iterator itItem = this->begin(); itItem != this->end(); ++itItem) {
          const T& item = *itItem;
          accumulate += item->getSquaredDist2D(trajectory2D);
        }
        return accumulate;

      }

      // Methodes that explicitly use the order of the items as a feature of the storing vector.
      // meaning things that can not be done with the set

      /// Getter for the first observed position projected to the given trajectory
      Vector2D getFrontRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      { return this->front()->getFrontRecoPos2D(trajectory2D); }

      /// Getter for the last observed position projected to the given trajectory
      Vector2D getBackRecoPos2D(const CDCTrajectory2D& trajectory2D) const
      { return this->back()->getBackRecoPos2D(trajectory2D); }

      /// Calculates the travel distance of the first track entity in the vector
      FloatType getFrontPerpS(const CDCTrajectory2D& trajectory2D) const
      { return this->front()->getFrontPerpS(trajectory2D); }

      /// Calculates the travel distance of the last track entity in the vector
      FloatType getBackPerpS(const CDCTrajectory2D& trajectory2D) const
      { return this->back()->getBackPerpS(trajectory2D); }

      /// Calculate the total transvers travel distance traversed by this ordered sequence of hit entities
      FloatType getTotalPerpS(const CDCTrajectory2D& trajectory2D) const
      { return trajectory2D.calcPerpSBetween(getFrontRecoPos2D(trajectory2D), getBackRecoPos2D(trajectory2D)); }

      /// Checks if the last entity in the vector lies at greater travel distance than the first entity
      bool isForwardTrajectory(const CDCTrajectory2D& trajectory2D) const
      { return getTotalPerpS(trajectory2D) > 0.0; }

      /// Checks if the last entity in the vector lies greater or lower travel distance than the last entity.
      /** Returns:
       *  * FORWARD if the last entity lies behind the first.
       *  * BACKWARD if the last entity lies before the first.
       */
      ForwardBackwardInfo isCoaligned(const CDCTrajectory2D& trajectory2D) const
      { return sign(getTotalPerpS(trajectory2D)); }

    private:
      /// ROOT Macro to make CDCGenHitVector a ROOT class.
      ClassDefInCDCLocalTracking(CDCGenHitVector, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2

#endif // CDCGENHITVECTOR_H
