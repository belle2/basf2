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

#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

#include <tracking/trackFindingCDC/ca/AutomatonCell.h>
#include <tracking/trackFindingCDC/ca/CellWeight.h>
#include <tracking/trackFindingCDC/ca/NeighborWeight.h>

#include <framework/logging/Logger.h>

#include <vector>
#include <limits>

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
       *  @param cellHolderNeighborhood  The weighted neighborhood between the cells.
       *  @return                        The cell holder with the highest cell state found.
       */
      template<class ACellHolderRange>
      ACellHolder* applyTo(ACellHolderRange& cellHolders,
                           const WeightedNeighborhood<ACellHolder>& cellHolderNeighborhood) const
      {
        // Set all cell states to -inf and the non permanent flags to unset.
        prepareCellFlags(cellHolders);

        ACellHolder* ptrHighestCellHolder = nullptr;
        CellState highestCellState = NAN;

        for (ACellHolder& cellHolder : cellHolders) {
          if (cellHolder.getAutomatonCell().hasMaskedFlag()) continue;
          if (cellHolder.getAutomatonCell().hasCycleFlag()) continue;

          if (not cellHolder.getAutomatonCell().hasAssignedFlag()) {
            // Mark this cell as a start point of a long path since we encountered it in
            // at the top level of the recursion.
            cellHolder.getAutomatonCell().setStartFlag();
            // The flag will be unset when it appears on the _to_ side of a relation.
          }

          try {
            CellState cellState = getFinalCellState(cellHolder, cellHolderNeighborhood);
            if (std::isnan(highestCellState) or highestCellState < cellState) {
              // We have a new best start point.
              ptrHighestCellHolder = &cellHolder;
              highestCellState = cellState;
            }
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
        return ptrHighestCellHolder;
      }

    private:
      /**
       *  Gets the cell state of the cell holder.
       *  Determines it if necessary traversing the graph.
       *  Throws CycleException if it encounters a cycle in the graph.
       */
      CellState getFinalCellState(ACellHolder& cellHolder,
                                  const WeightedNeighborhood<ACellHolder>& cellHolderNeighborhood) const
      {
        // Throw if this cell has already been traversed in this recursion cycle
        if (cellHolder.getAutomatonCell().hasCycleFlag()) {
          B2DEBUG(100, "Cycle detected");
          throw (CycleException());
        }

        if (cellHolder.getAutomatonCell().hasAssignedFlag()) {
          return cellHolder.getAutomatonCell().getCellState();

        } else {
          // Mark cell in order to detect if it was already traversed in this recursion cycle
          cellHolder.getAutomatonCell().setCycleFlag();

          CellState finalCellState = updateState(cellHolder, cellHolderNeighborhood);

          // Unmark the cell
          cellHolder.getAutomatonCell().unsetCycleFlag();
          return finalCellState;
        }
      }

      /// Updates the state of a cell considering all continuations recursively
      CellState updateState(ACellHolder& cellHolder,
                            const WeightedNeighborhood<ACellHolder>& cellHolderNeighborhood) const
      {
        //--- blocked cells do not contribute a continuation ---
        if (cellHolder.getAutomatonCell().hasMaskedFlag()) {
          cellHolder.getAutomatonCell().setCellState(NAN);
          cellHolder.getAutomatonCell().setAssignedFlag();
          return cellHolder.getAutomatonCell().getCellState();
        }

        //--- Search for neighbors ---
        CellState maxStateWithContinuation = NAN;

        // Check neighbors now
        for (const WeightedRelation<ACellHolder>& relation
             : cellHolderNeighborhood.equal_range(&cellHolder)) {
          // Advance to valid neighbor
          ACellHolder* neighborCellHolderPtr = relation.getTo();
          if (neighborCellHolderPtr and not neighborCellHolderPtr->getAutomatonCell().hasMaskedFlag()) {

            ACellHolder& neighborCellHolder = *neighborCellHolderPtr;

            // Invalidate a possible start flag since the neighbor has an ancestors
            neighborCellHolder.getAutomatonCell().unsetStartFlag();

            // Get the value of the neighbor
            CellState neighborCellState = getFinalCellState(neighborCellHolder, cellHolderNeighborhood);

            // Add the value of the connetion to the gain value
            CellState stateWithContinuation = neighborCellState + relation.getWeight();

            // Remember only the maximum value of all neighbors
            if (std::isnan(maxStateWithContinuation) or maxStateWithContinuation < stateWithContinuation) {
              maxStateWithContinuation = stateWithContinuation;
            }

          } // end if hasMaskedFlag()
        } // end for relations

        if (std::isnan(maxStateWithContinuation)) {
          // No valid neighbor contributed a connection to the cell
          maxStateWithContinuation = 0;
        }

        // The value of this cell is only its own weight
        maxStateWithContinuation += cellHolder.getAutomatonCell().getCellWeight();

        // Set the value
        cellHolder.getAutomatonCell().setCellState(maxStateWithContinuation);

        // Mark this cell as having its final value
        cellHolder.getAutomatonCell().setAssignedFlag();

        // Return the just determined value
        return cellHolder.getAutomatonCell().getCellState();
      }

    private:
      /**
       *  Helper function to prepare the stats.
       *  Clears all temporary cell flags and sets the cell state to minus infinity.
       */
      template<class ACellHolderRange>
      void prepareCellFlags(ACellHolderRange& cellHolders) const
      {
        for (ACellHolder& cellHolder : cellHolders) {
          cellHolder.getAutomatonCell().unsetTemporaryFlags();
          cellHolder.getAutomatonCell().setCellState(NAN);
        }
      }

    }; // end class CellularAutomaton

  } //end namespace TrackFindingCDC

} //end namespace Belle2
