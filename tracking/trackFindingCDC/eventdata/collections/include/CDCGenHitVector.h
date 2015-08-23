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

#include <vector>
#include <algorithm>


#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>

#include <tracking/trackFindingCDC/ca/SortableVector.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A generic vector used for different hit aggregates.
    /** details */

    template<class T>
    class CDCGenHitVector : public SortableVector<T> {

    public:
      /// Default constructor for ROOT compatibility.
      CDCGenHitVector() {;}

      /// Empty deconstructor
      ~CDCGenHitVector() {;}

      ///Implements the standard swap idiom
      friend void swap(CDCGenHitVector<T>& lhs, CDCGenHitVector<T>& rhs)
      {
        SortableVector<T>& rawLHS = lhs;
        SortableVector<T>& rawRHS = rhs;
        rawLHS.swap(rawRHS);
        B2DEBUG(200, "CDCGenHitVector::swap");
      }

    private:

      /// Helper class for STL algorithms searching for specific wire.
      class HasWirePredicate {
      public:
        /// Constructor of the helper class taking the sought wire.
        explicit HasWirePredicate(const Belle2::TrackFindingCDC::CDCWire& wire) : m_wire(wire) {;}

        /// Evaluates if the given item has the sought wire.
        bool operator()(const T& item) { return item->hasWire(m_wire); }

      private:
        /// Reference to the sought wire.
        const CDCWire& m_wire;
      };
      /// Helper class for STL algorithms searching for specific wire hit.
      class HasWireHitPredicate {
      public:
        /// Constructor of the helper class taking the sought wire hit.
        explicit HasWireHitPredicate(const Belle2::TrackFindingCDC::CDCWireHit& wirehit) : m_wirehit(wirehit) {;}

        /// Evaluates if the given item has the sought wire hit.
        bool operator()(const T& item) { return item->hasWireHit(m_wirehit); }

      private:
        /// Reference to the sought wire hit.
        const Belle2::TrackFindingCDC::CDCWireHit& m_wirehit;
      };

    public:
      /// Erases all tracking entities assoziated with specific wire from the vector.
      void eraseAll(const Belle2::TrackFindingCDC::CDCWire& wire)
      {
        this->erase(remove_if(this->begin(), this->end(), HasWirePredicate(wire)), this->end());
        //remove erase idiom
      }
      /// Erases all tracking entities assoziated with specific wire hit from the vector.
      void eraseAll(const Belle2::TrackFindingCDC::CDCWireHit& wirehit)
      {
        this->erase(remove_if(this->begin(), this->end(), HasWireHitPredicate(wirehit)), this->end());
        //remove erase idiom
      }

      /// Checks if any stored tracking entity is assoziated with a specific wire.
      bool hasWire(const Belle2::TrackFindingCDC::CDCWire& wire) const
      {
        auto found = std::find_if(this->begin(), this->end(), HasWirePredicate(wire));
        return found != this->end();
      }

      /// Checks if any stored tracking entity is assoziated with a specific wire hit.
      bool hasWireHit(const Belle2::TrackFindingCDC::CDCWireHit& wirehit) const
      {
        auto found = std::find_if(this->begin(), this->end(), HasWireHitPredicate(wirehit));
        return found != this->end();
      }

      /// Returns the common stereo type of all tracking entities.
      /** This checks if all tracking entities are located in the same superlayer and \n
       *  returns the stereo type of the later. Returns INVALID_STEREOTYPE if the superlayer \n
       *  is not shared among the tracking entities. */
      StereoType getStereoType() const
      { return isValidISuperLayer(getISuperLayer()) ? this->front()->getStereoType() : INVALID_STEREOTYPE; }

      /// Returns the common super layer id of all stored tracking entities
      /** This checks if all tracking entities are located in the same superlayer and \n
       *  returns the superlayer id of the later. Returns INVALID_ISUPERLAYER if the superlayer \n
       *  is not shared among the tracking entities. */
      ILayerType getISuperLayer() const
      {
        if (this->empty()) return INVALID_ISUPERLAYER;
        const T& firstItem = this->front();
        const ILayerType iSuperLayer = firstItem->getISuperLayer();
        auto sameSL = [iSuperLayer](const T & item) { return item->getISuperLayer() == iSuperLayer; };
        return  std::all_of(this->begin(), this->end(), sameSL) ? iSuperLayer : INVALID_ISUPERLAYER;
      }

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2

