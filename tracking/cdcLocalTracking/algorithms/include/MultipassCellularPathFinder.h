/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MULTIPASSCELLULARPATHFINDER_H
#define MULTIPASSCELLULARPATHFINDER_H

#include <vector>
#include <boost/foreach.hpp>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <framework/logging/Logger.h>

#include <tracking/cdcLocalTracking/algorithms/WeightedNeighborhood.h>
#include <tracking/cdcLocalTracking/algorithms/CellularAutomaton.h>
#include <tracking/cdcLocalTracking/algorithms/CellularPathFollower.h>

namespace Belle2 {

  namespace CDCLocalTracking {


    /** Class to combine the run of the cellular automaton and the repeated path extraction.
     *  Execute the cellular automaton and extracting paths interatively blocking the already used
     *  knots until there is no more path fullfilling the minimal length / energy requirement given
     *  as minStateToFollow to the constructor
     */
    template<class Item>
    class  MultipassCellularPathFinder {

    private:
      /// Type for the neighborhood of elements in the algorithm
      typedef WeightedNeighborhood<const Item> Neighborhood;

      /// Type of the resulting
      typedef std::vector<const Item*> Path;

    public:
      /// Empty constructor
      MultipassCellularPathFinder(const CellState& minStateToFollow = -std::numeric_limits<CellState>::infinity()):
        m_minStateToFollow(minStateToFollow) {;}

      /// Empty destructor
      ~MultipassCellularPathFinder() {;}

      /// Applies the cellular automaton to the collection and its neighborhood
      template<class ItemRange>
      void apply(
        const ItemRange& itemRange,
        const Neighborhood& neighborhood,
        std::vector<Path>& paths
      ) const {

        // multiple passes of the cellular automat
        // one segment is created at a time denying all knots it picked up,
        // applying the cellular automaton again
        // and so on
        // no best candidate analysis needed
        // (only makes sense with minimal clusters to avoid evaluating of uncommon paths)

        bool created = false;
        B2DEBUG(100, "Apply multipass cellular automat");
        do {
          //apply the cellular automation
          //B2DEBUG(100,"Apply cellular automat");
          const Item* highestCell
            = m_cellularAutomaton.applyTo(itemRange, neighborhood);

          Path&& newPath = m_cellularPathFollower.followSingle(highestCell, neighborhood, m_minStateToFollow);

          if (newPath.empty()) {
            created = false;
          } else {

            //Block the used items
            for (const Item * item :  newPath) {
              item->setDoNotUse();
            }

            //Block the items that have already used components
            for (const Item & item :  itemRange) {
              item.receiveDoNotUse();
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
      CellularAutomaton<Item> m_cellularAutomaton;

      /// The path follower used to extract the path from the graph processed by the cellular automaton.
      CellularPathFollower<Item> m_cellularPathFollower;

    }; // end class MultipassCellularPathFinder

  } //end namespace CDCLocalTracking

} //end namespace Belle2

#endif //MULTIPASSCELLULARPATHFINDER_H
