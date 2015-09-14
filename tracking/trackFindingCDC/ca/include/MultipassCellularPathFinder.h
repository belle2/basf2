/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/ca/CellularAutomaton.h>
#include <tracking/trackFindingCDC/ca/CellularPathFollower.h>
#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/numerics/BasicTypes.h>
#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {


    /** Class to combine the run of the cellular automaton and the repeated path extraction.
     *  Execute the cellular automaton and extracting paths interatively blocking the already used
     *  knots until there is no more path fullfilling the minimal length / energy requirement given
     *  as minStateToFollow to the constructor
     */
    template<class AItem>
    class  MultipassCellularPathFinder {

    private:
      /// Type for the neighborhood of elements in the algorithm
      typedef WeightedNeighborhood<const AItem> Neighborhood;

      /// Type of the resulting
      typedef std::vector<const AItem*> Path;

    public:
      /// Empty constructor
      explicit MultipassCellularPathFinder(const CellState& minStateToFollow = -std::numeric_limits<CellState>::infinity()):
        m_minStateToFollow(minStateToFollow) {}

      /// Applies the cellular automaton to the collection and its neighborhood
      template<class AItemRange>
      void apply(const AItemRange& itemRange,
                 const Neighborhood& neighborhood,
                 std::vector<Path>& paths) const
      {

        // multiple passes of the cellular automat
        // one segment is created at a time denying all knots it picked up,
        // applying the cellular automaton again
        // and so on
        // no best candidate analysis needed
        // (only makes sense with minimal clusters to avoid evaluating of uncommon paths)


        for (const AItem& item : itemRange) {
          item.unsetAndForwardMaskedFlag();
        }

        bool created = false;
        B2DEBUG(100, "Apply multipass cellular automat");
        do {
          //apply the cellular automation
          //B2DEBUG(100,"Apply cellular automat");
          const AItem* highestCell = m_cellularAutomaton.applyTo(itemRange, neighborhood);

          Path newPath = m_cellularPathFollower.followSingle(highestCell, neighborhood, m_minStateToFollow);

          if (newPath.empty()) {
            created = false;
          } else {

            //Block the used items
            for (const AItem* item : newPath) {
              item->setAndForwardMaskedFlag();
            }

            //Block the items that have already used components
            for (const AItem& item : itemRange) {
              item.receiveMaskedFlag();
            }

            paths.push_back(std::move(newPath));
            created = true;

          }

        } while (created);

      }

    private:
      /// The minimal path length / energy to be followed.
      CellState m_minStateToFollow;

      /// The cellular automaton to be used.
      CellularAutomaton<AItem> m_cellularAutomaton;

      /// The path follower used to extract the path from the graph processed by the cellular automaton.
      CellularPathFollower<AItem> m_cellularPathFollower;

    }; // end class MultipassCellularPathFinder

  } //end namespace TrackFindingCDC

} //end namespace Belle2

