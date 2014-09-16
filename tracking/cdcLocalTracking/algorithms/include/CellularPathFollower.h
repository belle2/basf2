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

#include <tracking/cdcLocalTracking/algorithms/WeightedNeighborhood.h>
#include <tracking/cdcLocalTracking/algorithms/AutomatonCell.h>

#include <tracking/cdcLocalTracking/algorithms/CellWeight.h>
#include <tracking/cdcLocalTracking/algorithms/NeighborWeight.h>

#include <framework/logging/Logger.h>

#include <vector>
#include <limits>

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
      /// Neighborhood type for the graph edges
      typedef WeightedNeighborhood<const Item> Neighborhood;

      /// Resulting path type to be generated
      typedef std::vector<const Item*> Path;



    public:
      /// Empty constructor
      CellularPathFollower() {;}

      /// Empty destructor
      ~CellularPathFollower() {;}



    public:

      /// Follow paths from all start items marked with the start flag
      template<class ItemRange>
      std::vector<Path>&&  followAll(const ItemRange& itemRange,
                                     const WeightedNeighborhood<const Item>& neighborhood,
                                     CellState minStateToFollow = -std::numeric_limits<CellState>::infinity()) const {

        m_paths.clear();

        // Segment to work on
        Path path;

        for (const Item & item : itemRange) {
          const AutomatonCell& cell = item.getAutomatonCell();

          if (validStartCell(cell, minStateToFollow)) {
            //item marks a start point of a path

            //start new segment
            path.clear();

            //insert a pointer to the item into the segment
            path.push_back(&item);

            bool growMany = true;

            //recursivly grow the segment
            growPath(path, neighborhood, growMany);
            path.pop_back();

          }
        }

        // Take out the actual path contents without copying it.
        return std::move(m_paths);

      }



      /// Follows a single maximal path starting with the given start item. If the start item is nullptr or has a state lower than the minimum state to follow an empty vector is returned.
      Path followSingle(const Item* ptrStartItem,
                        const WeightedNeighborhood<const Item>& neighborhood,
                        CellState minStateToFollow = -std::numeric_limits<CellState>::infinity()) const {

        Path path;

        if (ptrStartItem == nullptr) return path;

        const Item& startItem = *ptrStartItem;
        const AutomatonCell& startCell = startItem.getAutomatonCell();

        if (validStartCell(startCell, minStateToFollow)) {
          // Start new segment
          path.reserve(20); //Just a guess

          // Insert a pointer to the item into the segment
          path.push_back(ptrStartItem);

          bool growMany = false;

          growPath(path, neighborhood, growMany);

        }

        return path;

      }



    private:
      /// Helper function for recursively growing paths. If growMany is true it will follow all highest continuations and store the paths in the m_paths class member.
      void growPath(Path& path,
                    const Neighborhood& neighborhood,
                    bool growMany = false) const {

        const Item* lastItem = path.back();

        bool grew = false;
        for (const typename Neighborhood::WeightedRelation & relation : neighborhood.equal_range(lastItem)) {
          if (isHighestContinuation(relation)) {

            const Item* ptrNeighbor = getNeighbor(relation);
            path.push_back(ptrNeighbor);

            growPath(path, neighborhood, growMany);
            if (growMany) {
              grew = true;
              path.pop_back();
            } else {
              return;
            }
          }
        }

        if (not grew) {
          // end point of the recursion copy maximal path to the vector.
          m_paths.push_back(path);
        }

      }



      /// Helper function to determine, if the cell has all flags
      /// indicating to be a start cell and that its state exceeds the minimal requirement.
      static bool validStartCell(const AutomatonCell& cell, const CellState& minStateToFollow) {
        return
          cell.hasStartFlag() and
          not cell.hasDoNotUseFlag() and
          not cell.hasCycleFlag() and
          minStateToFollow <= cell.getCellState();
      }



      /// Helper function determining if the given neighbor is one of the best to be followed. Since this is an algebraic property on comparision to the other alternatives is necessary.
      static bool isHighestContinuation(const typename Neighborhood::WeightedRelation& relation) {
        const Item* ptrItem = getItem(relation);
        const Item* ptrNeighbor = getNeighbor(relation);

        if (not ptrItem or not ptrNeighbor) return false;

        const NeighborWeight& weight = getWeight(relation);

        const Item& item = *ptrItem;
        const Item& neighbor = *ptrNeighbor;

        return isHighestContinuation(item, weight, neighbor);
      }



      /// Helper function determining if the given neighbor is one of the best to be followed. Since this is an algebraic property on comparision to the other alternatives is necessary.
      static bool isHighestContinuation(const Item& item, const NeighborWeight& weight, const Item& neighbor) {
        const AutomatonCell& itemCell = item.getAutomatonCell();
        const AutomatonCell& neighborCell = neighbor.getAutomatonCell();

        return (
                 not neighborCell.hasCycleFlag() and
                 not neighborCell.hasDoNotUseFlag() and
                 (itemCell.getCellState() == (neighborCell.getCellState() + weight + itemCell.getCellWeight()))
               );
      }

    private:
      /// Temporal storage of all the generated paths on tree traversal.
      mutable std::vector<Path> m_paths;

    }; // end class CellularPathFollower

  } //end namespace CDCLocalTracking

} //end namespace Belle2

#endif //CELLULARPATHFOLLOWER_H_





