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
    template<class Item>
    class CellularPathFollower {

    private:
      typedef WeightedNeighborhood<const Item> Neighborhood;
      typedef std::vector<const Item*> Path;

    public:
      /// Empty constructor
      CellularPathFollower() {;}

      /// Empty destructor
      ~CellularPathFollower() {;}
    public:

      /// Follow paths from all start items marked with IS_START

      template<class ItemRange>
      inline size_t followAll(
        const ItemRange& itemRange,
        const WeightedNeighborhood<const Item>& neighborhood,
        std::vector<Path>& paths,
        CellState minStateToFollow = -std::numeric_limits<CellState>::infinity()
      ) const {

        size_t nPathsBefore = paths.size();

        //segment to work on
        Path path;

        for (const Item & item : itemRange) {

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
      inline bool followSingle(
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

            if (isHighestNeighbor(currentItem, relationWeight, neighborItem)) {

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
          const Weight& relationWeight = itNeighbor.getWeight();
          const Item* neighbor = itNeighbor.getNeighbor();

          if (isHighestNeighbor(item, relationWeight, neighbor)) {

            path.push_back(neighbor);
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

      bool isHighestNeighbor(const Item* item, const Weight& relationWeight, const Item* neighbor) const {

        return (not neighbor->getAutomatonCell().hasAnyFlags(DO_NOT_USE + IS_CYCLE) and
                (item->getAutomatonCell().getCellState() ==
                 item->getAutomatonCell().getCellWeight() + relationWeight + neighbor->getAutomatonCell().getCellState())
               );
      }


    }; // end class CellularPathFollower

  } //end namespace CDCLocalTracking

} //end namespace Belle2

#endif //CELLULARPATHFOLLOWER_H_





