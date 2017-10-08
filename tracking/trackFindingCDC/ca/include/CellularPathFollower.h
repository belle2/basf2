/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/ca/Path.h>
#include <tracking/trackFindingCDC/ca/AutomatonCell.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <vector>
#include <cassert>

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
     *  Implements to pick up of the highest value path in neighborhood
     *  Following high value paths can be done two ways. First construct a single path that has the highest
     *  value of all. This carried out by follow single  which uses the highest cell returned by the
     *  cellular automaton.
     *  Second construct all paths which are maximal. A maximal path means that there is no longer path
     *  including this path. If there are many disjoint paths this is the way to get them. However you most
     *  certainly pick up a lot of elements twice if there are many start culminating into a common long
     *  path. This is carried out recursively by followAll over the start cells marked with start flag.
     */
    template<class ACellHolder>
    class CellularPathFollower {

    public:
      /// Follow paths from all start cells marked with the start flag
      std::vector<Path<ACellHolder>> followAll(
                                    const std::vector<ACellHolder*>& cellHolders,
                                    const std::vector<WeightedRelation<ACellHolder>>& cellHolderRelations,
                                    Weight minStateToFollow = -INFINITY) const
      {
        B2ASSERT("Expected the relations to be sorted",
                 std::is_sorted(cellHolderRelations.begin(), cellHolderRelations.end()));

        // Result
        std::vector<Path<ACellHolder> > paths;

        // Stack for back tracking
        Path<ACellHolder> path;

        for (ACellHolder* cellHolder : cellHolders) {
          const AutomatonCell& automatonCell = cellHolder->getAutomatonCell();

          if (validStartCell(automatonCell, minStateToFollow)) {
            // Cell marks a start point of a path

            // Start new path
            path.clear();

            // Insert a pointer to the cell into the path
            path.push_back(cellHolder);

            // Recursivly grow the path
            growAllPaths(path, cellHolderRelations, paths);
            path.pop_back();
          }
        }
        return paths;
      }

      /**
       *  Follows a single maximal path starting with the given start cell.
       *  If the start cell is nullptr or has a state lower than the minimum state to follow
       *  an empty vector is returned.
       */
      Path<ACellHolder> followSingle(ACellHolder* startCellHolder,
                                     const std::vector<WeightedRelation<ACellHolder>>& cellHolderRelations,
                                     Weight minStateToFollow = -INFINITY) const
      {
        assert(std::is_sorted(cellHolderRelations.begin(), cellHolderRelations.end()));

        Path<ACellHolder> path;
        if (not startCellHolder) return path;
        const AutomatonCell& startCell = startCellHolder->getAutomatonCell();
        if (not validStartCell(startCell, minStateToFollow)) return path;

        // Start new path
        path.reserve(20); // Just a guess

        // Insert a pointer to the cell into the path
        path.push_back(startCellHolder);
        bool grew = true;
        while (grew) {
          grew = false;
          ACellHolder* cellHolder = path.back();

          auto continuations = asRange(std::equal_range(cellHolderRelations.begin(),
                                                        cellHolderRelations.end(),
                                                        cellHolder));

          for (const WeightedRelation<ACellHolder>& relation : continuations) {
            if (isHighestContinuation(relation)) {
              ACellHolder* neighbor = relation.getTo();
              path.push_back(neighbor);
              grew = true;
              break;
            }
          }
        }
        return path;
      }

    private:
      /**
       *  Helper function for recursively growing paths.
       *  @param[in]  path                    Current path to be extended
       *  @param[in]  cellHolderNeighborhood  Considered relations to follow to extend the path
       *  @param[out] paths                   Longest paths generated
       */
      void growAllPaths(Path<ACellHolder>& path,
                        const std::vector<WeightedRelation<ACellHolder>>& cellHolderRelations,
                        std::vector<Path<ACellHolder> >& paths) const
      {
        auto growPathByRelation = [&](const WeightedRelation<ACellHolder>& neighborRelation) {
          if (not this->isHighestContinuation(neighborRelation)) return false;
          ACellHolder* neighbor(neighborRelation.getTo());
          path.push_back(neighbor);
          this->growAllPaths(path, cellHolderRelations, paths);
          path.pop_back();
          return true;
        };

        ACellHolder* lastCellHolder = path.back();

        auto continuations = asRange(std::equal_range(cellHolderRelations.begin(),
                                                      cellHolderRelations.end(),
                                                      lastCellHolder));
        int nRelationsUsed = std::count_if(continuations.begin(),
                                           continuations.end(),
                                           growPathByRelation);

        if (nRelationsUsed == 0) {
          // end point of the recursion copy maximal path to the vector.
          paths.push_back(path);
        }
      }

      /**
       *  Helper function to determine, if the cell has all flags
       *  indicating to be a start cell and that its state exceeds the minimal requirement.
       */
      static bool validStartCell(const AutomatonCell& automatonCell,
                                 Weight minStateToFollow)
      {
        return
          automatonCell.hasStartFlag() and
          not automatonCell.hasMaskedFlag() and
          not automatonCell.hasCycleFlag() and
          minStateToFollow <= automatonCell.getCellState();
      }

      /**
       *  Helper function determining if the given neighbor is one of the best to be followed.
       *  Since this is an algebraic property on comparision to the other alternatives is necessary.
       */
      static bool isHighestContinuation(const WeightedRelation<ACellHolder>& relation)
      {
        ACellHolder* cellHolderPtr(relation.getFrom());
        ACellHolder* neighborCellHolderPtr(relation.getTo());

        if (not cellHolderPtr or not neighborCellHolderPtr) return false;

        ACellHolder& cellHolder = *cellHolderPtr;
        Weight relationWeight = relation.getWeight();
        ACellHolder& neighborCellHolder = *neighborCellHolderPtr;

        return isHighestContinuation(cellHolder, relationWeight, neighborCellHolder);
      }

      /**
       *  Helper function determining if the given neighbor is one of the best to be followed.
       *  Since this is an algebraic property no comparision to the other alternatives is necessary.
       */
      static bool isHighestContinuation(ACellHolder& cellHolder,
                                        Weight relationWeight,
                                        ACellHolder& neighborCellHolder)
      {
        const AutomatonCell& automatonCell = cellHolder.getAutomatonCell();
        const AutomatonCell& neighborAutomatonCell = neighborCellHolder.getAutomatonCell();

        return not neighborAutomatonCell.hasCycleFlag() and not neighborAutomatonCell.hasMaskedFlag() and
               (automatonCell.getCellState() ==
                (neighborAutomatonCell.getCellState() + relationWeight + automatonCell.getCellWeight()));
      }
    };
  }
}
