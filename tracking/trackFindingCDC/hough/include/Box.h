/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Oliver Frost                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/numerics.h>
#include <tracking/trackFindingCDC/utilities/GenIndices.h>

#include <framework/logging/Logger.h>

#include <tuple>
#include <algorithm>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class... Types>
    class Box {
    public:
      /// Tuple of the types of each coordinates
      typedef std::tuple<Types...> Point;

      /// Tuple of differencs in each coordinate
      typedef std::tuple < decltype(Types() - Types())... > Delta;

      /// Accessor for the individual coordinate types.
      template<std::size_t I>
      using Type = typename std::tuple_element<I, Point>::type;

      /// Accessor for the individual coordinate difference types.
      template<std::size_t I>
      using Width = typename std::tuple_element<I, Delta>::type;

      /// Get the number of coorinates
      static std::size_t getNCoordinates()
      { return std::tuple_size<Point>(); }

      /// Initialise the box with bound in each dimension.
      Box(std::pair<Types, Types>... bounds) :
        m_lowerBounds{std::min(bounds.first, bounds.second)...},
        m_upperBounds{std::max(bounds.first, bounds.second)...}
      {;}

      /// Set bounds of the box to new values
      void setBounds(std::pair<Types, Types>... bounds)
      {
        m_lowerBounds = {std::min(bounds.first, bounds.second)...};
        m_upperBounds = {std::max(bounds.first, bounds.second)...};
      }

      /// Get the lower bound of the box in the coordinate I
      template<std::size_t I>
      const Type<I>& getLowerBound() const
      { return std::get<I>(m_lowerBounds); }

      /// Get the upper bound of the box in the coordinate I
      template<std::size_t I>
      const Type<I>& getUpperBound() const
      { return std::get<I>(m_upperBounds); }

      /// Get the difference of upper and lower bound
      template<std::size_t I>
      Width<I> getWidth() const
      { return getUpperBound<I>() - getLowerBound<I>(); }

    public:
      /// Get for break point in the range of coordinate I out of nDivsions equal spaced points
      template<std::size_t I>
      Type<I> getDivision(size_t nDivisions, size_t iDivision) const
      {
        assert(nDivisions >= iDivision);
        return getLowerBound<I>() + getWidth<I>() * iDivision / nDivisions;
      }

      /// Get the lower partition in the coordinate I
      template<std::size_t I>
      std::pair<Type<I>, Type<I> >
      getDivisionBounds(size_t nDivisions, size_t iDivision) const
      {
        assert(nDivisions > iDivision);
        return std::make_pair(getDivision<I>(nDivisions, iDivision),
                              getDivision<I>(nDivisions, iDivision + 1));
      }

      /// Get for the distance between two division bounds with overlap.
      template<std::size_t I>
      Width<I> getDivisionWidthWithOverlap(const Width<I>& overlap, size_t nDivisions) const
      { return (getWidth<I>() + (nDivisions - 1) * overlap) / nDivisions; }

      template<std::size_t I>
      Type<I>
      getLowerDivisionBoundWithOverlap(const Width<I>& overlap,
                                       size_t nDivisions,
                                       size_t iDivision) const
      {
        assert(nDivisions >= iDivision);
        return getLowerBound<I>() + (getWidth<I>() - overlap) * iDivision / nDivisions;
      }

      template<std::size_t I>
      Type<I>
      getUpperDivisionBoundWithOverlap(const Width<I>& overlap,
                                       size_t nDivisions,
                                       size_t iDivision) const
      {
        assert(nDivisions >= iDivision);
        return getUpperBound<I>() - (getWidth<I>() - overlap) * (nDivisions - iDivision - 1) / nDivisions;
      }

      /// Get the lower partition in the coordinate I
      template<std::size_t I>
      std::pair<Type<I>, Type<I> >
      getDivisionBoundsWithOverlap(const Width<I>& overlap,
                                   size_t nDivisions,
                                   size_t iDivision) const
      {
        assert(nDivisions > iDivision);
        return std::make_pair(getLowerDivisionBoundWithOverlap<I>(overlap, nDivisions, iDivision),
                              getUpperDivisionBoundWithOverlap<I>(overlap, nDivisions, iDivision));
      }

      /// Get the center of the box in the coordinate I
      template<std::size_t I>
      Type<I> getCenter() const
      { return getDivision<I>(2, 1); }

      /// Get the bounds of the box in the coordinate I
      template<std::size_t I>
      std::pair<Type<I>, Type<I> > getBounds() const
      { return std::make_pair(getLowerBound<I>(), getUpperBound<I>()); }

      /// Get the lower partition in the coordinate I
      template<std::size_t I>
      std::pair<Type<I>, Type<I> > getLowerHalfBounds() const
      { return getDivisionBounds(2, 0); }

      /// Get the upper partition in the coordinate I
      template<std::size_t I>
      std::pair<Type<I>, Type<I> > getUpperHalfBounds() const
      { return getDivisionBounds(2, 1); }

      /// Indicate if the given value is in the range of the coordinate I
      template<std::size_t I, class OtherValue>
      bool isIn(const OtherValue& value) const
      { return getLowerBound<I>() < value and not(getUpperBound<I>() < value); }

    private:
      /// Indicates if any of the boolean values is true.
      inline static bool any(const std::initializer_list<bool>& booleans)
      {
        return std::accumulate(std::begin(booleans), std::end(booleans),
        false, [](const bool & lhs, const bool & rhs) {return lhs or rhs;});

      }

      /// Indicates if all of the boolean values are true.
      inline static bool all(const std::initializer_list<bool>& booleans)
      {
        return std::accumulate(std::begin(booleans), std::end(booleans),
        true, [](const bool & lhs, const bool & rhs) {return lhs and rhs;});
      }

      /** Indicates if this box intersects with the other box
       *  Implementation unroling the individual coordinates.
       *
       *  Checks if the lower bound of one box is higher than the upper bound in the other box
       *  for every coordinate.
       *  If no such bound can be found there is no intersection.
       *  Negation yields the desired result.
       */
      template<size_t ... Is>
      bool intersectsImpl(const Box<Types...>& box, IndexSequence<Is...>) const
      {
        return not(any({box.getUpperBound<Is>() < getLowerBound<Is>()... }) or
                   any({getUpperBound<Is>() < box.getLowerBound<Is>()... }));
      }

      /** Indicate if all given values are in the range of their coordinates.
       *  Implementation unroling the individual coordinates.
       */
      template<size_t ... Is>
      bool isInImpl(const Types& ... values, IndexSequence<Is...>) const
      { return not all({isIn<Is>(values) ... }); }

    public:
      /// Indicates if this box intersects with the other box
      bool intersects(const Box<Types...>& box) const
      { return intersectsImpl(box, GenIndices<sizeof...(Types)>());  }

      /// Indicate if all given values are in the range of their coordinates.
      bool isIn(const Types& ... values) const
      { return isInImpl(values..., GenIndices<sizeof...(Types)>()); }

    private:
      /// Lower bounds of the box in each direction
      Point m_lowerBounds;

      /// Upper bounds of the box in each direction
      Point m_upperBounds;
    };

  }
}
