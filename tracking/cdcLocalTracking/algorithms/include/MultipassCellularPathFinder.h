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

    template<class Item>
    class  MultipassCellularPathFinder {

    private:
      typedef WeightedNeighborhood<const Item> Neighborhood; // Type of the neighborhood of elements
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
        // one segment is created at a time denying all the wire hits it picked up
        // and apply the cellular automat again
        // and so on
        // no best candidate analysis needed
        // (only makes sense with minimal clusters to avoid reweighting of uncommon paths )

        bool created = false;
        B2DEBUG(100, "Apply multipass cellular automat");
        do {
          //apply the cellular automation
          //B2DEBUG(100,"Apply cellular automat");
          const Item* highestCell
            = m_cellularAutomaton.applyTo(itemRange, neighborhood);

          paths.push_back(Path());
          Path& newPath = paths.back();
          created = m_cellularPathFollower.followSingle(highestCell, neighborhood, newPath, m_minStateToFollow);

          //Block the used items
          for (const Item * item :  newPath) {
            item->setDoNotUse();
          }

          //Block the items that have already used components
          for (const Item & item :  itemRange) {
            item.receiveDoNotUse();
          }

        } while (created);

        // drop last oath because it was not filled
        paths.pop_back();

      }

    private:
      CellState m_minStateToFollow;
      CellularAutomaton<Item> m_cellularAutomaton;
      CellularPathFollower<Item> m_cellularPathFollower;

    }; // end class MultipassCellularPathFinder

  } //end namespace CDCLocalTracking

} //end namespace Belle2

#endif //MULTIPASSCELLULARPATHFINDER_H
