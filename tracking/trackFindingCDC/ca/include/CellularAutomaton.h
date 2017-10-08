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

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>
#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <framework/logging/Logger.h>

#include <cmath>
#include <cassert>

namespace Belle2 {

  namespace TrackFindingCDC {
    /**
     *  Implements the weighted cellular automaton algorithm
     */
    template<class ACellHolder>
    class CellularAutomaton {

    private:
      /// Type for the very basic exception signal used in the detection of cycles.
      class CycleException {};

    public:
      /**
       *  Applies the cellular automaton to the collection of cells and its neighborhood
       *  @param cellHolders             The range based iterable containing the cells.
       *  @param cellHolderRelations     The weighted relations between the cells.
       *  @return                        The cell holder with the highest cell state found.
       */
      ACellHolder* applyTo(const std::vector<ACellHolder*>& cellHolders,
                           const std::vector<WeightedRelation<ACellHolder>>& cellHolderRelations) const
      {
        B2ASSERT("Expected the relations to be sorted",
                 std::is_sorted(cellHolderRelations.begin(), cellHolderRelations.end()));

        // Set all cell states to -inf and the non permanent flags to unset.
        prepareCellFlags(cellHolders);

        for (ACellHolder* cellHolder : cellHolders) {
          AutomatonCell& cell = cellHolder->getAutomatonCell();
          if (cell.hasMaskedFlag()) continue;
          if (cell.hasCycleFlag()) continue;

          if (not cell.hasAssignedFlag()) {
            // Mark this cell as a start point of a long path since we encountered it in
            // at the top level of the recursion.
            cell.setStartFlag();
            // The flag will be unset when it appears on the _to_ side of a relation.
          }

          try {
            // Assignes flags and the cell state
            getFinalCellState(cellHolder, cellHolderRelations);
          } catch (CycleException) {
            // TODO: Come up with some handeling for cycles.
            // For now we continue to look for long paths in the graph
            // hoping to find a part that does not enter the cycle.

            // Thoughts:
            // If there is a single cycle in the graph we might be able to break it at some point.
            // However, if there are multiple cycles intertwined things get very tricky.
            // But can we actually distinguish the two situations?
          }
        }

        auto lessStartCellState = [](ACellHolder * lhs, ACellHolder * rhs) {
          AutomatonCell& lhsCell = lhs->getAutomatonCell();
          AutomatonCell& rhsCell = rhs->getAutomatonCell();
          return (std::make_pair(lhsCell.hasStartFlag(), lhsCell.getCellState()) <
                  std::make_pair(rhsCell.hasStartFlag(), rhsCell.getCellState()));
        };

        auto itStartCellHolder =
          std::max_element(cellHolders.begin(), cellHolders.end(), lessStartCellState);
        if (itStartCellHolder == cellHolders.end()) return nullptr;
        if (not(*itStartCellHolder)->getAutomatonCell().hasStartFlag()) return nullptr;
        return *itStartCellHolder;
      }

    private:
      /**
       *  Gets the cell state of the cell holder.
       *  Determines it if necessary traversing the graph.
       *  Throws CycleException if it encounters a cycle in the graph.
       */
      Weight getFinalCellState(ACellHolder* cellHolder,
                               const std::vector<WeightedRelation<ACellHolder>>& cellHolderRelations) const
      {
        AutomatonCell& cell = cellHolder->getAutomatonCell();

        // Throw if this cell has already been traversed in this recursion cycle
        if (cell.hasCycleFlag()) {
          B2DEBUG(100, "Cycle detected");
          throw (CycleException());
        }

        if (cell.hasAssignedFlag()) {
          return cell.getCellState();

        } else {
          // Mark cell in order to detect if it was already traversed in this recursion cycle
          cell.setCycleFlag();

          Weight finalCellState = updateState(cellHolder, cellHolderRelations);

          // Unmark the cell
          cell.unsetCycleFlag();
          return finalCellState;
        }
      }

      /// Updates the state of a cell considering all continuations recursively
      Weight updateState(ACellHolder* cellHolder,
                         const std::vector<WeightedRelation<ACellHolder>>& cellHolderRelations) const
      {
        AutomatonCell& cell = cellHolder->getAutomatonCell();

        // --- blocked cells do not contribute a continuation ---
        // Redundant check.
        if (cell.hasMaskedFlag()) {
          cell.setAssignedFlag();
          return NAN;
        }

        //--- Search for neighbors ---
        Weight maxStateWithContinuation = NAN;

        // Flag to keep track whether the best continuation lies on a prioriy path
        bool isPriorityPath = false;

        auto continuations = asRange(
                               std::equal_range(cellHolderRelations.begin(), cellHolderRelations.end(), cellHolder));

        // Check neighbors now
        for (const WeightedRelation<ACellHolder>& relation : continuations) {
          // Advance to valid neighbor
          ACellHolder* neighborCellHolder = relation.getTo();

          // Skip dead ends (should not happen)
          if (not neighborCellHolder) continue;

          AutomatonCell& neighborCell = neighborCellHolder->getAutomatonCell();
          // Skip masked continuations
          if (neighborCell.hasMaskedFlag()) continue;

          // Invalidate a possible start flag since the neighbor has an ancestors
          neighborCell.unsetStartFlag();

          // Get the value of the neighbor
          Weight neighborCellState = getFinalCellState(neighborCellHolder, cellHolderRelations);

          // Add the value of the connetion to the gain value
          Weight stateWithContinuation = neighborCellState + relation.getWeight();

          // Remember only the maximum value of all neighbors
          if (std::isnan(maxStateWithContinuation) or maxStateWithContinuation < stateWithContinuation) {
            maxStateWithContinuation = stateWithContinuation;
            // Remember whether the best continuation marks a priorty path
            // construction ensures that priority paths have at least two elements
            isPriorityPath = neighborCell.hasPriorityFlag() or neighborCell.hasPriorityPathFlag();
          }
        } // end for relations

        if (std::isnan(maxStateWithContinuation)) {
          // No valid neighbor contributed a connection to the cell
          maxStateWithContinuation = 0;
        }

        // The value of this cell is only its own weight
        maxStateWithContinuation += cell.getCellWeight();

        // Set the value
        cell.setCellState(maxStateWithContinuation);
        cell.setPriorityPathFlag(isPriorityPath);

        // Mark this cell as having its final value
        cell.setAssignedFlag();

        // Return the just determined value
        return cell.getCellState();
      }

    private:
      /**
       *  Helper function to prepare the stats.
       *  Clears all temporary cell flags and sets the cell state to minus infinity.
       */
      void prepareCellFlags(const std::vector<ACellHolder*>& cellHolders) const
      {
        for (ACellHolder* cellHolder : cellHolders) {
          AutomatonCell& cell = cellHolder->getAutomatonCell();
          cell.unsetTemporaryFlags();
          if (cell.hasMaskedFlag()) continue;
          cell.setCellState(NAN);
        }
      }
    };
  }
}
