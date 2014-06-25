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
      const Item*
      applyTo(
        const ItemRange& itemRange,
        const Neighborhood& neighborhood
      ) const {

        return applyWithRecursion(itemRange, neighborhood);

      }


      /// Applies the cellular automaton to the collection and its neighborhood
      /** Applies the cellular automaton algorithm to the collection where the connections are given by
       *  the neighborhood.
       *  @param itemRange the range based iterable containing the items that should acquire the cell states
       *  @param neighborhood the weighted neighborhood of type WeightedNeighborhood<const Item> */
      template<class ItemRange>
      const Item*
      applyWithRecursion(
        const ItemRange& itemRange,
        const Neighborhood& neighborhood
      ) const {
        //set all cell states to -inf and the flags to unset
        prepareCellFlags(itemRange);


        typedef decltype(std::begin(itemRange)) ItemIterator;

        ItemIterator itHighestItem = std::begin(itemRange);

        // Advance the iterator to a valid item in order to to have a valid highest item.
        while (itHighestItem != std::end(itemRange) and
               itHighestItem->getAutomatonCell().hasDoNotUseFlag()) {

          ++itHighestItem;

        }

        // apply cellular automation
        // uses recursion and a single pass through
        for (ItemIterator itItem = itHighestItem; itItem != std::end(itemRange); ++itItem) {

          const Item& item = *itItem;

          const AutomatonCell& itemCell = item.getAutomatonCell();

          if (not itemCell.hasAssignedFlag() and
              not itemCell.hasCycleFlag() and
              not itemCell.hasDoNotUseFlag()) {

            const CellState& state = updateState(item, neighborhood);

            // Mark this cell as a start point of a long path since we encountered it in
            // a top level recursion
            item.getAutomatonCell().setStartFlag();
            itHighestItem = itHighestItem->getAutomatonCell().getCellState() > state ?  itHighestItem : itItem;

          }
        }

        // Return the element with the highst cell value as a good start point for a segment/track
        if (itHighestItem == std::end(itemRange) or
            itHighestItem->getAutomatonCell().hasCycleFlag() or
            itHighestItem->getAutomatonCell().hasDoNotUseFlag() or
            not itHighestItem->getAutomatonCell().hasStartFlag()) {

          return nullptr;

        } else {

          return &(*itHighestItem);

        }
      }
      /// Updates the state of a cell considering all continuations recursively
      inline const CellState& updateState(const Item& item, const Neighborhood& neighborhood) const {

        // since we encounter this cell in a recursion it is not the start point of track
        item.getAutomatonCell().unsetStartFlag();

        // check if the cell is valid to continue on
        if (item.getAutomatonCell().hasCycleFlag()) {
          // if not invalidate this cell and return
          item.getAutomatonCell().setCellState(NO_CONTINUATION);
          return item.getAutomatonCell().getCellState();
        }
        // We check the do not use flag before going into the recursion
        // We check the IS_SET flag before going into the recursion
        // so now need to check here again for the IS_SET flag
        /*
        else if ( item.hasFlags(IS_SET) ){
          //assuming this has already been assigned correctly
          return item.getCellState();
        }*/


        // at this point cellState has not been set before and
        // the cell also does not carry the do not use flag

        // search for neighbors
        typename Neighborhood::range neighborRange = neighborhood.equal_range(&item);

        //advance to a valid neighbor
        while (neighborRange.first != neighborRange.second and
               neighborRange.first.getNeighbor()->getAutomatonCell().hasDoNotUseFlag()) {
          ++(neighborRange.first);
        }

        CellState maxStateWithContinuation = 0;

        if (neighborRange.first == neighborRange.second) {
          // No further neighbors
          // No additional point can be pick up for the continuation
          maxStateWithContinuation = 0;

        } else {
          // we have at least on valid neighbor now in neighborRange.first
          // so this initial value will only last the initial loop
          maxStateWithContinuation = -std::numeric_limits<CellState>::infinity();

          //mark cell in order to detect if it was already traversed in this recursion cycle
          item.getAutomatonCell().setCycleFlag();

          // consider all neighbors as possible continuations and
          // ask each how much value they have to offer
          for (typename Neighborhood::iterator itNeighbor = neighborRange.first;
               itNeighbor != neighborRange.second; ++itNeighbor) {

            const Item* neighbor = itNeighbor.getNeighbor();

            if (not neighbor->getAutomatonCell().hasDoNotUseFlag()) {

              // Invalidate a possible start flag since the neighbor has an ancestors
              neighbor->getAutomatonCell().unsetStartFlag();

              // Check if the neighbor was already marked in this recursion cycle
              // Preventing an infinit loop
              if (neighbor->getAutomatonCell().hasCycleFlag()) {
                // encountered cycle
                // do not unset IS_CYCLE of this item
                item.getAutomatonCell().setCellState(NO_CONTINUATION);
                B2WARNING("Cycle detected");
                return item.getAutomatonCell().getCellState();

              }

              // Get the value of the neighbor
              // If it was set just get it
              // If is was not set go into the recursion
              const CellState& stateWithoutContinuation =
                neighbor->getAutomatonCell().hasAssignedFlag() ?
                neighbor->getAutomatonCell().getCellState() :
                updateState(*neighbor, neighborhood);


              // Add the value of the connetion to the gain value
              CellState stateWithContinuation = stateWithoutContinuation + itNeighbor.getWeight();

              // Remember only the maximum value of all neighbors
              maxStateWithContinuation = std::max(maxStateWithContinuation, stateWithContinuation);

            }
          }

          // no cycle encountered, unset flag
          item.getAutomatonCell().unsetCycleFlag();

        }

        // Add the value of this cell to the value of the best neighbor continuation
        // to get the total value since this cell shall also be part of the path
        maxStateWithContinuation += item.getAutomatonCell().getCellWeight();

        // Set this cell has a correct value
        item.getAutomatonCell().setAssignedFlag();
        // Set the value
        item.getAutomatonCell().setCellState(maxStateWithContinuation);

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
