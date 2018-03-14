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

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Implementation of the clustering
     *  Clusters elements of a given collection using the relations presented by a neighorhood.
     *  A cluster is essentially a connected subset of all cells
     *  that can reach each other by one more relations in a neighborhood.
     *  The algorithm is essentially an iterative expansion of the neighborhood relations keeping
     *  track of the already used cells by using the CellState of the AutomatonCell.
     *
     *  * ACellHolder must therefore provide an AutomatonCell accessable by a getAutomatonCell() method.
     *    In case the objects you what to cluster do not contain an automaton cell already you may adopt it
     *    by using the WithAutomatonCell mixin.
     *  * ACluster can be anything that is default constructable and supports .insert(end(), ACellHolder*).
     */
    template <class ACellHolder, class ACluster = std::vector<ACellHolder*> >
    class Clusterizer
      : public Findlet<ACellHolder* const, WeightedRelation<ACellHolder> const, ACluster> {

    public:
      /**
       *  Creates the clusters.
       *  Repeatly expands a neighborhood of referenced objects that have an AutomatonCell.
       *  The CellState after the clusterization is the index of the generated cluster.
       *  The CellWeight is set to the total number of neighbors of each cell.
       *
       *  @param cellHolders          Pointers to objects that should be clustered.
       *  @param cellHolderRelations  Relations between the objects that should be clustered
       *  @param[out] clusters        Groups of connected objects in the neighborhood.
       */
      void apply(std::vector<ACellHolder*> const& cellHolders,
                 std::vector<WeightedRelation<ACellHolder> > const& cellHolderRelations,
                 std::vector<ACluster>& clusters) override
      {
        // Expect the relations to be sorted for lookup
        assert(std::is_sorted(cellHolderRelations.begin(),
                              cellHolderRelations.end()));

        // Prepare some output clusters
        clusters.reserve(30);

        // Prepare states
        for (ACellHolder* cellHolder : cellHolders) {
          setCellState(cellHolder, -1);
        }

        // Work horse cluster
        std::vector<ACellHolder*> cluster;

        // Go through each cell holder and start a cluster on each that is unassigned yet
        int iCluster = -1;
        for (ACellHolder* cellHolder : cellHolders) {
          if (getCellState(cellHolder) != -1) continue;

          cluster.clear();

          ++iCluster;
          setCellState(cellHolder, iCluster);
          cluster.push_back(cellHolder);

          expandCluster(cellHolderRelations, cluster);

          clusters.emplace_back(std::move(cluster));
          cluster.clear();
        }
      }

    private:
      /// Helper function. Starting a new cluster and iterativelly expands it.
      void expandCluster(std::vector<WeightedRelation<ACellHolder>> const& cellHolderRelations,
                         std::vector<ACellHolder*>& cluster) const
      {
        ACellHolder* seedCellHolder = cluster.front();
        int iCluster = getCellState(seedCellHolder);

        // Grow the cluster iterativelly
        std::vector<ACellHolder*> checkNow;
        std::vector<ACellHolder*> checkNext;

        checkNow.reserve(10);
        checkNext.reserve(10);

        checkNext.push_back(seedCellHolder);

        while (not checkNext.empty()) {

          checkNow.swap(checkNext);
          checkNext.clear();

          for (ACellHolder* cellHolder : checkNow) {

            ConstVectorRange<WeightedRelation<ACellHolder> > neighborRelations(
              std::equal_range(cellHolderRelations.begin(),
                               cellHolderRelations.end(),
                               cellHolder));

            // Setting the cell weight to the number of neighbors
            size_t nNeighbors = neighborRelations.size();
            setCellWeight(cellHolder, nNeighbors);

            for (const WeightedRelation<ACellHolder>& neighborRelation : neighborRelations) {
              ACellHolder* neighborCellHolder = neighborRelation.getTo();

              Weight neighborICluster = getCellState(neighborCellHolder);
              if (neighborICluster == -1) {
                // Neighbor not yet in cluster
                setCellState(neighborCellHolder, iCluster);
                cluster.push_back(neighborCellHolder);

                // Register neighbor for further expansion
                checkNext.push_back(neighborCellHolder);
                continue;
              }

              if (neighborICluster != iCluster) {
                B2WARNING("Clusterizer: Neighboring item was already assigned to different "
                          "cluster. Check if the neighborhood is symmetric.");
                continue;
              }
            }
          }
        }
      }

      /// Setter for the cell state of a pointed object that holds an AutomatonCell
      void setCellState(ACellHolder* cellHolder, Weight cellState) const
      {
        AutomatonCell& automatonCell = cellHolder->getAutomatonCell();
        automatonCell.setCellState(cellState);
      }

      /// Getter for the cell state of a pointed object that holds an AutomatonCell
      Weight getCellState(ACellHolder* cellHolder) const
      {
        const AutomatonCell& automatonCell = cellHolder->getAutomatonCell();
        return automatonCell.getCellState();
      }

      /// Setter for the cell weight of a pointed object that holds an AutomatonCell
      void setCellWeight(ACellHolder* cellHolder, Weight cellWeight) const
      {
        AutomatonCell& automatonCell = cellHolder->getAutomatonCell();
        automatonCell.setCellWeight(cellWeight);
      }

    };
  }
}
