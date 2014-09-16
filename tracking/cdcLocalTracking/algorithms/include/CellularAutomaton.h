/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CELLULARAUTOMATON_H
#define CELLULARAUTOMATON_H

#include <vector>
#include <boost/foreach.hpp>
#include <limits>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <framework/logging/Logger.h>


#include <tracking/cdcLocalTracking/algorithms/CellWeight.h>
#include <tracking/cdcLocalTracking/algorithms/NeighborWeight.h>
#include <tracking/cdcLocalTracking/algorithms/AutomatonCell.h>
#include <tracking/cdcLocalTracking/algorithms/WeightedNeighborhood.h>



namespace Belle2 {

  namespace CDCLocalTracking {
    ///Implemetation of the cellular automaton
    /** Implements the weighted cellular automaton algorithm respecting the DO_NOT_USE flags of the cells
     *  not to be traversed. */
    template<class Item>
    class CellularAutomaton {

    private:
      /// Type for the neighborhood of elements in the algorithm
      typedef WeightedNeighborhood<const Item> Neighborhood;

      /// Type for the very basic exception signal used in the detection of cycles.
      typedef int CycleDetectionMarker;

      /// Instance for the very basic exception signal used in the detection of cycles.
      const int CYCLE_DETECTED = -999;

    public:

      /// Empty constructor
      CellularAutomaton() {;}

      /// Empty deconstructor
      ~CellularAutomaton() {;}

      /// Applies the cellular automaton to the collection and its neighborhood
      /** Applies the cellular automaton algorithm to the collection where the connections are given by
       *  the neighborhood.
       *  @param itemRange the range based iterable containing the items that should acquire the cell states
       *  @param neighborhood the weighted neighborhood of type WeightedNeighborhood<const Item> */
      template<class ItemRange>
      const Item*  applyTo(const ItemRange& itemRange, const Neighborhood& neighborhood) const {
        return applyWithRecursion(itemRange, neighborhood);
      }


      /// Applies the cellular automaton to the collection and its neighborhood
      /** Applies the cellular automaton algorithm to the collection where the connections are given by
       *  the neighborhood.
       *  @param itemRange the range based iterable containing the items that should acquire the cell states
       *  @param neighborhood the weighted neighborhood of type WeightedNeighborhood<const Item> */
      template<class ItemRange>
      const Item* applyWithRecursion(const ItemRange& itemRange,
                                     const Neighborhood& neighborhood) const {
        // Set all cell states to -inf and the non permanent flags to unset.
        prepareCellFlags(itemRange);

        const Item* ptrHighestItem = nullptr;

        for (const Item & item : itemRange) {
          const AutomatonCell& automatonCell = item.getAutomatonCell();
          if (automatonCell.hasDoNotUseFlag()) continue;
          if (automatonCell.hasCycleFlag()) continue;

          if (not automatonCell.hasAssignedFlag()) {
            // Mark this cell as a start point of a long path since we encountered it in
            // at the top level of the recursion.
            automatonCell.setStartFlag();
          }

          try {
            const CellState& cellState = getFinalCellState(item, neighborhood);

            if (not ptrHighestItem) {
              // First assignment of a valid item
              ptrHighestItem = &item;
            } else if (ptrHighestItem->getAutomatonCell().getCellState() < cellState) {
              // Replacement of the highest item
              ptrHighestItem = &item;
            }

          } catch (CycleDetectionMarker) {
            // TODO: Come up with some handeling for cycles.
            // For now we continue to look for long paths in the graph
            // hoping to find a part that does not enter the cycle.

          }
        }

        return ptrHighestItem;

      }

    private:

      /// Gets the cell state of the item. Determines it if necessary traversing the graph. Throws CYCLE_DETECTED if it encounters a cycle in the graph.
      const CellState& getFinalCellState(const Item& item, const Neighborhood& neighborhood) const {
        const AutomatonCell& automatonCell = item.getAutomatonCell();
        // Throw if this cell has already been traversed in this recursion cycle
        if (automatonCell.hasCycleFlag()) {
          B2WARNING("Cycle detected");
          throw (CYCLE_DETECTED);

        }

        if (automatonCell.hasAssignedFlag()) {
          return automatonCell.getCellState();

        } else {
          // Mark cell in order to detect if it was already traversed in this recursion cycle
          item.getAutomatonCell().setCycleFlag();

          const CellState& finalCellState = updateState(item, neighborhood);

          // Unmark the cell
          item.getAutomatonCell().unsetCycleFlag();
          return finalCellState;
        }
      }



      /// Updates the state of a cell considering all continuations recursively
      const CellState& updateState(const Item& item, const Neighborhood& neighborhood) const {
        //--- blocked cells do not contribute a continuation ---
        if (item.getAutomatonCell().hasDoNotUseFlag()) {
          item.getAutomatonCell().setCellState(NAN);
          item.getAutomatonCell().setAssignedFlag();
          return item.getAutomatonCell().getCellState();

        }

        //--- Search for neighbors ---
        CellState maxStateWithContinuation = -std::numeric_limits<CellState>::infinity();

        // Check neighbors now
        for (const typename Neighborhood::WeightedRelation & relation : neighborhood.equal_range(&item)) {
          //advance to valid neighbor
          const Item* ptrNeighbor = getNeighbor(relation);
          if (ptrNeighbor and not ptrNeighbor->getAutomatonCell().hasDoNotUseFlag()) {

            const Item& neighbor = *ptrNeighbor;

            // Invalidate a possible start flag since the neighbor has an ancestors
            neighbor.getAutomatonCell().unsetStartFlag();

            // Get the value of the neighbor
            const CellState& stateWithoutContinuation = getFinalCellState(neighbor, neighborhood);

            // Add the value of the connetion to the gain value
            CellState stateWithContinuation = stateWithoutContinuation + getWeight(relation);

            // Remember only the maximum value of all neighbors
            maxStateWithContinuation = std::max(maxStateWithContinuation, stateWithContinuation);

          } //end if hasDoNotUse()
        } // end for relations

        if (maxStateWithContinuation == -std::numeric_limits<CellState>::infinity()) {
          // No valid neighbor contributed a connection to the cell
          maxStateWithContinuation = 0;
        }

        // Add the value of this cell to the value of the best neighbor continuation
        // to get the total value since this cell shall also be part of the path
        maxStateWithContinuation += item.getAutomatonCell().getCellWeight();

        // Set the value
        item.getAutomatonCell().setCellState(maxStateWithContinuation);

        // Mark this cell as having its final value
        item.getAutomatonCell().setAssignedFlag();

        // Return the just determined value
        return item.getAutomatonCell().getCellState();

      }


    private:

      //non recusive version not yet ported to support cell flags
      /*
      void CellularAutomat::applyToNoRecursion(const Collection & collection, const Neighborhood & neighborhood) const{
        //the algorithm works with out recursive function calls by keeping a stack of relevant information on its own
        //it sets the cell states in the following way

        // cell state > 0  -> cell has a chain of state-1 descendence and has anscestor
        // cell state < 0  -> cell has a chain of (-state)-1 descendence but no anscestor
        //by keeping the sign indicating the presence of ascestors we know, where to start from to have the longest paths

        // cell state == 0 -> the should not occure since all cells are traversed at least once

        //set all cell states to zero
        prepareCellStates(collection);

        vector<Neighborhood::NeighborRange> stack;
        vector<int> highestState;
        for ( Collection::const_iterator itItem = collection.begin();
            itItem != collection.end(); ++itItem){

          if ( itItem->getCellState() == 0) {
            //assumes the cell has been assigned a state correctly

            Neighborhood::NeighborRange rangeForItem = neighborhood.getNeighborRange(&(*itItem));

            //set up recursion
            stack.push_back(rangeForItem);
            highestState.push_back(0);

            int highestFromRecursion = 0;

            while ( not stack.empty() ){
              Neighborhood::NeighborRange & currentRange = stack.back();
              if ( currentRange.first != currentRange.second){

                const Collection::Item * currentItem = *(currentRange.first);
                int currentCellState = currentItem->getCellState();
                if ( currentCellState > 0 ){

                  //assume the state as already been assigned correctly
                  //remember the highest state in this recursion level
                  highestState.back() = max(highestState.back() , currentCellState);
                  //next item in the current recursion level
                  ++(currentRange.first);

                } else if ( currentCellState < 0 ){
                  //assume the state as already been assigned correctly
                  //but still thinks it has no ascestors
                  //correct that making it aware of having ascestors

                  //swap cell state to a positiv value
                  currentCellState = abs(currentCellState);
                  currentItem->setCellState(currentCellState);

                  //remember the highest state in this recursion level
                  highestState.back() = max(highestState.back() , currentCellState);
                  //next item in the current recursion level
                  ++(currentRange.first);

                }  else {
                  //no cell state assigned yet check neighbors first
                  //check neighbors first
                  Neighborhood::NeighborRange neighborsForCurrentItem = neighborhood.getNeighborRange(currentItem);

                  //one recursion level down
                  stack.push_back(neighborsForCurrentItem);
                  highestState.push_back(0);
                }
              } else {

                //recursion level exhausted
                highestFromRecursion = highestState.back(); //keep "return value" from recursion level
                stack.pop_back();
                highestState.pop_back();
                //one level up


                if( not stack.empty() ){
                  //resume former level
                  Neighborhood::NeighborRange & resumeRange = stack.back();
                  const Collection::Item * currentItem = *(resumeRange.first);

                  //update the cell state with the information form the former recursion level
                  currentItem->setCellState(highestFromRecursion + 1);

                  //remember the highest state in this recursion level
                  highestState.back() = max(highestState.back() , highestFromRecursion + 1);
                  //next item in the resumed recursion level
                  ++(resumeRange.first);
                } else {
                  // the stack is now empty and the highestFromRecursion has to be assigned to the itItem
                }
              } //end if ( currentRange.first != currentRange.second)
            } //end while not stack.empty()


            //assign the top level element a negativ state because it has no ancestors so far
            //if it had that have been checked anscestors it would have aquired a non zero cell state already
            //else the recursion will correct the state to a positiv value as soon as it is encountered
            itItem->setCellState(-(highestFromRecursion + 1));


          } //end if itItem->getCellState() == 0
        } //end itItem

      }*/


    private:
      /// Helper function to prepare the stats
      /** Clears all flags but DO_NOT_USE and sets the cell state to minus infinity. */
      template<class ItemRange>
      void prepareCellFlags(const ItemRange& itemRange) const {
        for (const Item & item : itemRange) {

          item.getAutomatonCell().unsetTemporaryFlags();
          item.getAutomatonCell().setCellState(-std::numeric_limits<CellState>::infinity());

        }

      }




    }; // end class CellularAutomaton

  } //end namespace CDCLocalTracking

} //end namespace Belle2

#endif //CELLULARAUTOMATON_H
