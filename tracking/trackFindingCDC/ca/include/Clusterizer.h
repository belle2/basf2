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

#include <tracking/trackFindingCDC/utilities/GetValueType.h>
#include <tracking/trackFindingCDC/utilities/GetIterator.h>
#include <tracking/trackFindingCDC/utilities/Range.h>

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
    template<class ACellHolder, class ACluster = std::vector<ACellHolder*> >
    class Clusterizer {

      /// Pointer type used to refer to the objects to be clustered.
      using CellHolderPtr = ACellHolder*;

    public:
      /**
       *  Creates the clusters.
       *  Repeatly expands a neighborhood of reference objects that have an AutomatonCell.
       *  The CellState after the clusterization is the index of the generated cluster.
       *  The CellWeight is set to the total number of relations each cell has with in the neighborhood.
       *  * ACellHolderRange is required to be a range of cells convertable to ACellHolder*
       *  * ACellHolderNeighborhood is required to have a .equal_range() method
       *    that yields a range of pairs which .second elements are the sought neighbors.
       *  @param cellHolderPtrs         Pointers to objects that should be clustered.
       *  @param cellHolderNeighborhood  Relations between the objects that should be clustered
       *  @param[out] clusters          Groups of connected objects in the neighborhood.
       */
      template<class ACellHolderPtrRange, class ACellHolderNeighborhood>
      void createFromPointers(const ACellHolderPtrRange& cellHolderPtrs,
                              const ACellHolderNeighborhood& cellHolderNeighborhood,
                              std::vector<ACluster>& clusters) const
      {
        clusters.reserve(30);

        // Prepare states
        for (CellHolderPtr cellHolderPtr : cellHolderPtrs) {
          setCellState(cellHolderPtr, -1);
        }

        int iCluster = -1;
        for (CellHolderPtr cellHolderPtr : cellHolderPtrs) {
          if (getCellState(cellHolderPtr) != -1) continue;

          clusters.push_back(ACluster());
          ACluster& newCluster = clusters.back();
          ++iCluster;
          startCluster(cellHolderPtr, cellHolderNeighborhood, iCluster, newCluster);
        }
      }

    private:
      /// Helper function. Starting a new cluster and iterativelly (not recursively) expands it.
      template<class ACellHolderNeighborhood>
      inline void startCluster(CellHolderPtr seedCellHolderPtr,
                               const ACellHolderNeighborhood& cellHolderNeighborhood,
                               int iCluster,
                               ACluster& newCluster) const
      {
        setCellState(seedCellHolderPtr, iCluster);
        newCluster.insert(newCluster.end(), seedCellHolderPtr);

        // Grow the cluster iterativelly
        std::vector<CellHolderPtr> checkNow;
        std::vector<CellHolderPtr> checkNext;

        checkNow.reserve(10);
        checkNext.reserve(10);

        checkNext.push_back(seedCellHolderPtr);

        while (not checkNext.empty()) {

          checkNow.swap(checkNext);
          checkNext.clear();

          for (CellHolderPtr cellHolderPtr : checkNow) {
            size_t nNeighbors = 0;

            auto cellHolderRelations = cellHolderNeighborhood.equal_range(cellHolderPtr);
            using CellHolderRelationIterator = GetIterator<const ACellHolderNeighborhood>;
            using CellHolderRelation = GetValueType<const ACellHolderNeighborhood>;
            for (const CellHolderRelation & cellHolderRelation
                 : Range<CellHolderRelationIterator>(cellHolderRelations)) {
              CellHolderPtr neighborCellHolderPtr = cellHolderRelation.second;
              ++nNeighbors;

              CellState neighborICluster = getCellState(neighborCellHolderPtr);
              if (neighborICluster == -1) {
                // Neighbor not yet in cluster
                setCellState(neighborCellHolderPtr, iCluster);
                newCluster.insert(newCluster.end(), neighborCellHolderPtr);

                // Register neighbor for further expansion
                checkNext.push_back(neighborCellHolderPtr);

              } else if (neighborICluster != iCluster) {
                B2WARNING("Clusterizer: Neighboring item was already assigned to different cluster. Check if the neighborhood is symmetric.");

              } else {

              }
            }

            // Setting the cell weight to the number of neighbors
            setCellWeight(cellHolderPtr, nNeighbors);

          } // end (CellHolderPtr cellHolderPtr : checkNow)
        } // end while(not checkNext.empty())
      }

      /// Setter for the cell state of a pointed object that holds an AutomatonCell
      void setCellState(CellHolderPtr cellHolderPtr, CellState cellState) const
      {
        AutomatonCell& automatonCell = cellHolderPtr->getAutomatonCell();
        automatonCell.setCellState(cellState);
      }

      /// Getter for the cell state of a pointed object that holds an AutomatonCell
      CellState getCellState(CellHolderPtr cellHolderPtr) const
      {
        const AutomatonCell& automatonCell = cellHolderPtr->getAutomatonCell();
        return automatonCell.getCellState();
      }

      /// Setter for the cell weight of a pointed object that holds an AutomatonCell
      void setCellWeight(CellHolderPtr cellHolderPtr, CellWeight cellWeight) const
      {
        AutomatonCell& automatonCell = cellHolderPtr->getAutomatonCell();
        automatonCell.setCellWeight(cellWeight);
      }

    };
  }
}
