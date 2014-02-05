/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CELLULARPATHFOLLOWER_H_
#define CELLULARPATHFOLLOWER_H_

#include<vector>

#include <framework/logging/Logger.h>

#include <tracking/cdcLocalTracking/algorithms/WeightedNeighborhood.h>


namespace Belle2 {

  namespace CDCLocalTracking {

    /// Implements to pick up of the highest value path in neighborhood
    /** Following high value paths can be done two ways. First construct a single path that has the highest
     *  value of all. This carried out by follow single  which uses the highest item returned by the
     *  cellular automaton.
     *  Second construct all paths which are maximal. A maximal path means that there is no longer path
     *  including this path. If there are many disjoint paths this is the way to get them. However you most
     *  certainly pick up a lot of elements twice if there are many start culminating into a common long
     *  path segment. This is carried out recursively by followAll over the start items marked with IS_START.*/
    template<class Collection>
    class CellularPathFollower {
      //Collections can be
      // const_iterables of const Item or of const Item *

      //Segments can be
      //  ordered collections of const Item * that has push_back(const Item *)
      //  vector<const Item *>
      //  CDCGenericHitVector<const Item *>

    private:
      typedef typename Collection::value_type Item;
      typedef WeightedNeighborhood<const Item> Neighborhood;
      typedef std::vector<const Item*> Path;

    public:
      /// Empty constructor
      CellularPathFollower() {;}

      /// Empty destructor
      ~CellularPathFollower() {;}
    public:

      /// Follow paths from all start items marked with IS_START
      inline
      size_t
      followAll(
        const Collection& collection,
        const WeightedNeighborhood<const Item>& neighborhood,
        std::vector<Path>& paths,
        CellState minStateToFollow = -std::numeric_limits<CellState>::infinity()
      ) const {

        size_t nPathsBefore = paths.size();

        //segment to work on
        Path path;

        for (typename Collection::const_iterator itItem = collection.begin();
             itItem != collection.end(); ++itItem) {

          const Item& item = *itItem;

          if (item.getAutomatonCell().hasAnyFlags(IS_START) and
              not item.getAutomatonCell().hasAnyFlags(DO_NOT_USE + IS_CYCLE) and
              minStateToFollow <= item.getAutomatonCell().getCellState()) {

            //item marks a start point of a path

            //start new segment
            path.clear();

            //insert a pointer to the item into the segment
            path.push_back(&item);

            //recursivly grow the segment
            growPath(path, neighborhood, paths);
            path.pop_back();

          }
        }

        return paths.size() - nPathsBefore;
      }

      /// Follows a single maximal path starting with the given start item.
      inline
      bool
      followSingle(
        const Item* startItem,
        const WeightedNeighborhood<const Item>& neighborhood,
        Path& path,
        CellState minStateToFollow = -std::numeric_limits<CellState>::infinity()
      ) const {

        if (startItem != nullptr and
            startItem->getAutomatonCell().hasAnyFlags(IS_START) and
            not startItem-> getAutomatonCell().hasAnyFlags(DO_NOT_USE + IS_CYCLE) and
            minStateToFollow <= startItem->getAutomatonCell().getCellState()) {

          //start new segment
          path.clear();

          //insert a pointer to the item into the segment
          path.push_back(startItem);

          const Item* currentItem = startItem;

          // Search for the highest neighbor
          // Get the range of neighbors
          typename Neighborhood::range currentNeighborRange = neighborhood.equal_range(currentItem);

          while (currentNeighborRange.first != currentNeighborRange.second) {
            //const Item * currentItem = itNeighbor.getItem();
            const Item* neighborItem = currentNeighborRange.first.getNeighbor();
            Weight relationWeight = currentNeighborRange.first.getWeight();

            if (not neighborItem->getAutomatonCell().hasAnyFlags(DO_NOT_USE + IS_CYCLE) and
                currentItem->getAutomatonCell().getCellState() == currentItem->getAutomatonCell().getCellWeight() +
                relationWeight +
                neighborItem->getAutomatonCell().getCellState()
                /* detected that this is the highest continuation */) {

              // Best continuation detected
              // Append the neighbor item
              path.push_back(neighborItem);

              // Get the neighbors neighbors to be considered next. Changes iteration range.
              currentNeighborRange = neighborhood.equal_range(neighborItem);

              // The neighbor item becomes the new first postion
              currentItem = neighborItem;

            } else {
              ++(currentNeighborRange.first);
            }
          }
          // return if a path has been grown
          return true;

        } else {

          // return that no path has been grown
          return false;

        }
      }


    private:
      /// Helper function for recursively growing a path.
      inline
      void
      growPath(
        Path& path,
        const Neighborhood& neighborhood,
        std::vector<Path>& paths
      ) const {

        const Item* lastItem = path.back();
        typename Neighborhood::range neighborRange = neighborhood.equal_range(lastItem);

        bool grew = false;
        for (typename Neighborhood::iterator itNeighbor = neighborRange.first;
             itNeighbor != neighborRange.second;  ++itNeighbor) {

          const Item* item = itNeighbor.getItem();
          const Item* neighborItem = itNeighbor.getNeighbor();

          if (not neighborItem-> getAutomatonCell().hasAnyFlags(DO_NOT_USE + IS_CYCLE) and
              lastItem->getAutomatonCell().getCellState() == item->getAutomatonCell().getCellWeight() +
              itNeighbor.getWeight() +
              neighborItem->getAutomatonCell().getCellState()
              /* detect if this is the highest continuation */) {

            path.push_back(neighborItem);
            grew = true;
            growPath(path, neighborhood, paths);
            path.pop_back();

          }
        }
        if (not grew) {
          // end point of the recursion copy maximal path to the vector
          paths.push_back(path);
        }

      }
    }; // end class CellularPathFollower

  } //end namespace CDCLocalTracking

} //end namespace Belle2

#endif //CELLULARPATHFOLLOWER_H_





