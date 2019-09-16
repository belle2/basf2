/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>

#include <framework/logging/Logger.h>

#include <tracking/trackFindingVXD/algorithms/TrackerAlgorithmBase.h>


namespace Belle2 {
  /** The CellularAutomaton class
   * This class serves as a functor for the algorithm itself
   */
  template<class ContainerType, class ValidatorType>
  class CellularAutomaton final : public TrackerAlgorithmBase<ContainerType, ValidatorType> {
  public:
    /// typedef for the baseClass to get rid of the template arguments
    using BaseClass = TrackerAlgorithmBase<ContainerType, ValidatorType>;


    /** constructor */
    CellularAutomaton() : BaseClass() {}


    /** aborts CA after stopInRound iterations - mainly for debugging purposes: */
    unsigned int stopInRound = BaseClass::m_validator.nMaxIterations + 2;

    /** actual algorithm of Cellular Automaton, returns number of rounds needed to finish or -1 if CA was aborted */
    // TODO: Check where the -1 is returned, for bad cases as described above.
    int apply(ContainerType& aNetworkContainer) override final
    {
      /** REDESIGNCOMMENT CELLULARAUTOMATON:
       * optimization tip:
       * - at the moment for each round the number of active cells is reduced, but the loop runs over all nodes/cells.
       * TODO: Test Jakob's idea: Maybe its faster to recreate/refill an ActiveCell-List each round.
       */
      unsigned int activeCells = 1, // is set 1 because of following while loop.
                   deadCells = 0,
                   caRound = 1,
                   goodNeighbours = 0,
                   highestCellState = 0;

      // each iteration of following while loop is one CA-time-step
      while (activeCells != 0 and caRound < stopInRound) {
        activeCells = 0;

        /// CAstep:
        // compare cells with inner neighbours:
        for (auto* aNode : aNetworkContainer) {
          auto& currentCell = aNode->getMetaInfo();
          if (currentCell.isActivated() == false) { continue; }
          goodNeighbours = 0;

          for (auto* aNeighbour :  aNode->getInnerNodes()) {
            // skip if neighbour has not the same state (NOTE if one wants to improve the versatility of the code,
            // this should actually become a member of the cell-class, which then can add some extra
            // stuff like checking for loops.
            if (currentCell != aNeighbour->getMetaInfo()) continue;

            goodNeighbours++;
          }
          if (goodNeighbours != 0) {
            currentCell.setStateUpgrade(true);
            activeCells++;
          } else {
            // WARNING setActivationState does provoke unintended behavior, since sometimes states can not be
            // upgraded in one round, but can in the next round!
            /*currentCell.setActivationState(false);*/
            deadCells++;
          }
        }

        /// Updatestep:
        for (auto* aNode : aNetworkContainer) {
          auto& currentCell = aNode->getMetaInfo();
          if (currentCell.isActivated() == false or currentCell.isUpgradeAllowed() == false) { continue; }

          currentCell.setStateUpgrade(false);
          currentCell.increaseState();
          if (currentCell.getState() > highestCellState) { highestCellState = currentCell.getState(); }
        }

        /// catch bad case
        if (BaseClass::m_validator.isValidRound(caRound) == false) {
          break;
        }

        caRound++;
      } // CA main-loop

      return caRound;
    }


    /** checks network given for seeds, returns number of seeds found (if strictSeeding is set to true,
     * no subset of paths are stored, only unique ones).
     * WARNING: requires outerNodes to be set! (bidirectional network, not only directed to inner!). */
    unsigned int findSeeds(ContainerType& aNetworkContainer,  bool strictSeeding = false) override final
    {
      unsigned int nSeeds = 0;
      for (auto* aNode : aNetworkContainer) {
        if (strictSeeding && !(aNode->getOuterNodes().empty())) continue;

        if (BaseClass::m_validator.checkSeed(aNode->getMetaInfo()) == true) {
          aNode->getMetaInfo().setSeed(true);
          nSeeds++;
        }
      }
      return nSeeds;
    }
  };
}
